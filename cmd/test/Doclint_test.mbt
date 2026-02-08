///|
/// 批量 golden：扫描 cmd/test/golden 下所有 *.in.json，逐个比对同名 *.out.json
test "golden all cases (file-based)" {
  let dir = "cmd/test/golden"
  let files = @fs.read_dir(dir)

  for f in files {
    if ends_with(f, ".in.json") {
      let in_path = dir + "/" + f
      let out_path = dir + "/" + replace_suffix(f, ".in.json", ".out.json")

      assert_eq(@fs.path_exists(out_path), true)

      let input_json = @fs.read_file_to_string(in_path, encoding="utf8")
      let expected_json = @fs.read_file_to_string(out_path, encoding="utf8")

      let parse_doc = fn (s : String) -> Result[@cor.Document, String] {
        parse_doc_json_for_test(s)
      }

      let page_rule : @the.PageLimitRule = { min: 2, max: 50 }
      let abs_rule : @the.RequireKeywordRule = {
        rid: "basic.require_abstract",
        keyword: "摘要",
        sev: @cor.Severity::Error,
      }
      let rules : Array[&@cor.Rule] = [page_rule, abs_rule]

      let out = @cor.check_json(input_json, rules, parse_doc)

      assert_eq(canon(out), canon(expected_json))
    }
  }
}


// Test Tool
fn canon(s : String) -> String {
  let mut t = ""
  for ch in s {
    if ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t' { () }
    else { t = t + ch.to_string() }
  }
  t
}

fn ends_with(s : String, suf : String) -> Bool {
  let n = s.length()
  let m = suf.length()
  if n < m { false } else { slice(s,n - m, n) == suf }
}

fn replace_suffix(s : String, from : String, to : String) -> String {
  let n = s.length()
  let m = from.length()
  slice(s,0, n - m) + to
}

/// 极简 JSON 解析：只支持你的 schema：
/// { "meta": {...}, "pages": [ {"index": <int>, "text": "<string>"} ... ] }
fn parse_doc_json_for_test(s : String) -> Result[@cor.Document, String] {
  match parse_meta_and_pages(s) {
    None => Err("bad json")
    Some(doc) => Ok(doc)
  }
}

///|
fn parse_meta_and_pages(s : String) -> @cor.Document? {
  let title = match find_string_field(s, "title") {
    None => ""
    Some(t) => t
  }
  let author = match find_string_field(s, "author") {
    None => ""
    Some(a) => a
  }
  let created_at = match find_string_field(s, "created_at") {
    None => ""
    Some(c) => c
  }
  let pages = match parse_pages(s) {
    None => []
    Some(p) => p
  }
  Some({ meta: { title, author, created_at }, pages })
}

///|
fn find_string_field(s : String, key : String) -> String? {
  let pat = "\"" + key + "\""
  let kpos = match find_sub(s, pat, 0) {
    None => return None
    Some(p) => p
  }
  let colon = match find_char(s, ':', kpos + pat.length()) {
    None => return None
    Some(p) => p
  }
  let q1 = match find_char(s, '"', colon + 1) {
    None => return None
    Some(p) => p
  }
  let (val, _next) = match read_json_string(s, q1 + 1) {
    None => return None
    Some(pair) => pair
  }
  Some(val)
}


///|
fn parse_pages(s : String) -> Array[@cor.Page]? {
  let pat = "\"pages\""
  let ppos = match find_sub(s, pat, 0) {
    None => return None
    Some(p) => p
  }
  let colon = match find_char(s, ':', ppos + pat.length()) {
    None => return None
    Some(p) => p
  }
  let lb = match find_char(s, '[', colon + 1) {
    None => return None
    Some(p) => p
  }
  let mut i = lb + 1
  let pages : Array[@cor.Page] = []

  // 循环找 {"index":..., "text":"..."}
  while true {
    // 跳过空白与逗号
    i = skip_ws_comma(s, i)
    // 结束 ]
    if i >= s.length() {
      return None
    }
    if slice(s, i, i + 1) == "]" {
      break
    }
    // 找到 { 起始
    let ob = match find_char(s, '{', i) {
      None => return None
      Some(p) => p
    }
    // 解析 index
    let idx = match find_int_field_from(s, "index", ob) {
      None => return None
      Some(p) => p
    }
    // 解析 tex
    let txt = match find_string_field_from(s, "text", ob) {
      None => return None
      Some(p) => p
    }
    pages.push({ index: idx, text: txt })

    // 移动到下一个对象或 ]
    let cb = match find_char(s, '}', ob + 1) {
      None => return None
      Some(p) => p
    }
    i = cb + 1
  }
  Some(pages)
}

///|
fn parse_int_dec(s : String) -> Int? {
  // s 只包含 0-9
  let mut acc = 0
  let mut any = false
  for ch in s {
    if ch < '0' || ch > '9' {
      return None
    }
    acc = acc * 10 + (ch.to_int() - '0'.to_int()) // 如果 Char 没 to_int，你告诉我我再换写法
    any = true
  }
  if any {
    Some(acc)
  } else {
    None
  }
}

fn find_int_field_from(s : String, key : String, start : Int) -> Option[Int] {
  let pat = "\"" + key + "\""

  let kpos =
    match find_sub(s, pat, start) {
      None => return None
      Some(p) => p
    }

  let colon =
    match find_char(s, ':', kpos + pat.length()) {
      None => return None
      Some(p) => p
    }

  let mut i = skip_ws(s, colon + 1)

  let mut neg = false
  if i < s.length() && slice(s, i, i + 1) == "-" {
    neg = true
    i = i + 1
  }

  let mut j = i
  let n = s.length()
  while j < n {
    let c = slice(s, j, j + 1)
    if c >= "0" && c <= "9" { j = j + 1 } else { break }
  }
  if j == i { return None }

  let digits = slice(s, i, j)
  let num =
    match parse_int_dec(digits) {
      None => return None
      Some(v) => v
    }

  Some(if neg { -num } else { num })
}


///|
fn find_string_field_from(s : String, key : String, start : Int) -> String? {
  let pat = "\"" + key + "\""
  let kpos = match find_sub(s, pat, start) {
    None => return None
    Some(p) => p
  }
  let colon = match find_char(s, ':', kpos + pat.length()) {
    None => return None
    Some(p) => p
  }
  let q1 = match find_char(s, '"', colon + 1) {
    None => return None
    Some(p) => p
  }
  let (val, _next) = match read_json_string(s, q1 + 1) {
    None => return None
    Some(pair) => pair
  }
  Some(val)
}


fn read_json_string(s : String, start : Int) -> Option[(String, Int)] {
  // 从 start 开始读到下一个未转义的 "
  let mut out = ""
  let mut i = start
  while i < s.length() {
    let ch = slice(s,i, i + 1)
    if ch == "\"" {
      return Some((out, i + 1))
    } else if ch == "\\" {
      if i + 1 >= s.length() { return None }
      let esc = slice(s,i + 1, i + 2)
      match esc {
        "n" => out = out + "\n"
        "r" => out = out + "\r"
        "t" => out = out + "\t"
        "\"" => out = out + "\""
        "\\" => out = out + "\\"
        _ => out = out + esc
      }
      i = i + 2
    } else {
      out = out + ch
      i = i + 1
    }
  }
  None
}

fn find_sub(s : String, pat : String, start : Int) -> Option[Int] {
  let n = s.length()
  let m = pat.length()
  if m == 0 { return Some(start) }
  let mut i = start
  while i + m <= n {
    if slice(s,i, i + m) == pat {
      return Some(i)
    }
    i = i + 1
  }
  None
}

fn find_char(s : String, ch : Char, start : Int) -> Option[Int] {
  let mut i = start
  while i < s.length() {
    if slice(s,i, i + 1) == ch.to_string() {  // 若你的 String API 没 nth，就用 slice(i,i+1) 比较
      return Some(i)
    }
    i = i + 1
  }
  None
}

fn skip_ws(s : String, i : Int) -> Int {
  let mut n = i
  while n < s.length() {
    let c = slice(s,n, n + 1)
    if c == " " || c == "\n" || c == "\r" || c == "\t" { n = n + 1 } else { break }
  }
  n
}

fn skip_ws_comma(s : String, i : Int) -> Int {
  let mut n = i
  while n < s.length() {
    let c = slice(s,n, n + 1)
    if c == " " || c == "\n" || c == "\r" || c == "\t" || c == "," { n = n + 1 } else { break }
  }
  n
}

fn slice(s : String, start : Int, end_ : Int) -> String {
  let mut out = ""
  let mut i = 0
  for ch in s {
    if i >= start && i < end_ {
      out = out + ch.to_string()
    }
    i = i + 1
  }
  out
}