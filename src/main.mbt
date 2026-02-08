///|
type IoResult[T] = Result[T, String]

///|
fn read_utf8(path : String) -> IoResult[String] {
  let s = @fs.read_file_to_string(path, encoding="utf8") catch {
    _ => return Err("read failed: " + path)
  }
  Ok(s)
}

///|
fn write_utf8(path : String, content : String) -> IoResult[Unit] {
  @fs.write_string_to_file(path, content, encoding="utf8") catch {
    _ => return Err("write failed: " + path)
  }
  Ok(())
}

///|
fn ensure_dir(path : String) -> Unit {
  if @fs.path_exists(path) {
    if !(@fs.is_dir(path) catch { _ => false }) {
      println("path exists but not a dir: " + path)
    }
  } else {
    @fs.create_dir(path) catch {
      _ => println("failed to create dir: " + path)
    }
  }
}

///|
fn join_path(dir : String, file : String) -> String {
  // 简化：dir 不以 / 结尾
  dir + "/" + file
}

fn drop1(xs : Array[String]) -> Array[String] {
  let out : Array[String] = []
  let mut i = 0
  for x in xs {
    if i > 0 { out.push(x) }
    i = i + 1
  }
  out
}



///|
fn main {
  let args = drop1(@sys.get_cli_args())
  let mut rules_name = "thesis"
  let mut in_path = "test_golden/case2.in.json"
  let mut out_dir = "out"
  let mut emit_html = true
  let mut i = 0
  while i < args.length() {
    let a = args[i]
    if a == "--help" || a == "-h" {
      print_help()
      @sys.exit(0)
    } else if a == "--rules" {
      if i + 1 >= args.length() {
        println("missing value for --rules")
        print_help()
        @sys.exit(1)
      }
      rules_name = args[i + 1]
      i = i + 2
      continue
    } else if a == "--in" {
      if i + 1 >= args.length() {
        println("missing value for --in")
        print_help()
        @sys.exit(1)
      }
      in_path = args[i + 1]
      i = i + 2
      continue
    } else if a == "--outdir" {
      if i + 1 >= args.length() {
        println("missing value for --outdir")
        print_help()
        @sys.exit(1)
      }
      out_dir = args[i + 1]
      i = i + 2
      continue
    } else if a == "--no-html" {
      emit_html = false
      i = i + 1
      continue
    } else {
      println("unknown arg: " + a)
      print_help()
      @sys.exit(1)
    }
  }
  ensure_dir(out_dir)
  let out_json_path = join_path(out_dir, "report.json")
  let out_html_path = join_path(out_dir, "report.html")
  let doc_json_res = read_utf8(in_path)
  let doc_json = match doc_json_res {
    Err(msg) => {
      println(msg)
      @sys.exit(1)
      ""
    }
    Ok(s) => s
  }
  let doc_res = @cor.parse_doc_json(doc_json)
  let doc = match doc_res {
    Err(e) => {
      let issues : Array[@cor.Issue] = [
        @cor.mk_issue(
          "core.parse",
          @cor.Severity::Error,
          "文档 JSON 解析失败: " + e,
          None,
        ),
      ]
      write_out(out_json_path, out_html_path, issues, emit_html)
      @sys.exit(1)
      return ()
    }
    Ok(d) => d
  }
  let ctx : @cor.RuleContext = @cor.RuleContext::{  }
  let issues = if rules_name == "contract" {
    let rs = @con.build_ruleset()
    let rules = @con.as_rules_ref(rs)
    @cor.check_doc(ctx, doc, rules)
  } else {
    let rs = @the.build_ruleset()
    let rules = @the.as_rules_ref(rs)
    @cor.check_doc(ctx, doc, rules)
  }
  write_out(out_json_path, out_html_path, issues, emit_html)
  @sys.exit(0)
}

///|
fn write_out(
  out_json_path : String,
  out_html_path : String,
  issues : Array[@cor.Issue],
  emit_html : Bool,
) -> Unit {
  match write_utf8(out_json_path, @cor.issues_to_json(issues)) {
    Ok(_) => println("written: " + out_json_path)
    Err(msg) => println(msg)
  }
  if emit_html {
    match write_utf8(out_html_path, @cor.issues_to_html(issues)) {
      Ok(_) => println("written: " + out_html_path)
      Err(msg) => println(msg)
    }
  }
}

///|
fn print_help() -> Unit {
  println(
    "Usage:\n" +
    "  doclint --rules thesis|contract --in <file> --outdir <dir> [--no-html]\n" +
    "\n" +
    "Examples:\n" +
    "  doclint --rules thesis --in test_golden/case1.in.json\n" +
    "  doclint --rules contract --in test_golden/case2.in.json --outdir out\n" +
    "\n" +
    "Defaults:\n" +
    "  --rules thesis\n" +
    "  --in test_golden/case2.in.json\n" +
    "  --outdir out\n",
  )
}
