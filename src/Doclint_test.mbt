///|
/// 批量 golden：扫描 cmd/test/golden 下所有 *.in.json，逐个比对同名 *.out.json
test "golden all cases (file-based)" {
  let dir = "src/test_golden"
  let files = @fs.read_dir(dir)
  for f in files {
    if ends_with(f, ".in.json") {
      let in_path = dir + "/" + f
      let out_path = dir + "/" + replace_suffix(f, ".in.json", ".out.json")
      assert_eq(@fs.path_exists(out_path), true)
      let input_json = @fs.read_file_to_string(in_path, encoding="utf8")
      let expected_json = @fs.read_file_to_string(out_path, encoding="utf8")
      let page_rule : @the.PageLimitRule = { min: 2, max: 50 }
      let abs_rule : @the.RequireKeywordRule = {
        rid: "basic.require_abstract",
        keyword: "摘要",
        sev: @cor.Severity::Error,
      }
      let rules : Array[&@cor.Rule] = [page_rule, abs_rule]
      let out = @cor.check_json(input_json, rules, @cor.parse_doc_json)
      assert_eq(canon(out), canon(expected_json))
    }
  }
}

test "ruleset switch should change behavior" {
  let doc : @cor.Document = {
    meta: { title: "t", author: "a", created_at: "x" },
    pages: [
      { index: 0, text: "正文\n参考文献\n[1] ..." },
      { index: 1, text: "更多正文" }
    ],
  }

  // thesis
  let ts = @the.build_ruleset()
  let thesis_issues = @cor.check_doc(@cor.RuleContext::{}, doc, @the.as_rules_ref(ts))

  // contract
  let cs = @con.build_ruleset()
  let contract_issues = @cor.check_doc(@cor.RuleContext::{}, doc, @con.as_rules_ref(cs))

  // 只要规则集不同，至少 rule_id 集合应不同（最弱断言）
  let thesis_json = @cor.issues_to_json(thesis_issues)
  let contract_json = @cor.issues_to_json(contract_issues)
  assert_eq(thesis_json == contract_json, false)
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
  if n < m { false } else { @cor.slice(s,n - m, n) == suf }
}

fn replace_suffix(s : String, from : String, to : String) -> String {
  let n = s.length()
  let m = from.length()
  @cor.slice(s,0, n - m) + to
}

