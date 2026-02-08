///|
/// WASM stable exports (thin wrapper)
pub fn check_json_with_ruleset_wasm(ruleset : String, doc_json : String) -> String {
  check_json_with_ruleset(ruleset, doc_json)
}

///|
pub fn check_html_with_ruleset_wasm(ruleset : String, doc_json : String) -> String {
  check_html_with_ruleset(ruleset, doc_json)
}


///|
/// 纯函数：根据 ruleset + doc_json 返回 issues 的 JSON 字符串
pub fn check_json_with_ruleset(ruleset : String, doc_json : String) -> String {
  let doc_res = @cor.parse_doc_json(doc_json)
  let doc = match doc_res {
    Ok(d) => d
    Err(e) => {
      let issues : Array[@cor.Issue] = [
        @cor.mk_issue(
          "core.parse",
          @cor.Severity::Error,
          "文档 JSON 解析失败: " + e,
          None,
        ),
      ]
      return @cor.issues_to_json(issues)
    }
  }
  let ctx : @cor.RuleContext = @cor.RuleContext::{  }
  let issues = run_ruleset(ruleset, ctx, doc)
  @cor.issues_to_json(issues)
}

///|
/// 纯函数：根据 ruleset + doc_json 返回 HTML 报告
pub fn check_html_with_ruleset(ruleset : String, doc_json : String) -> String {
  let doc_res = @cor.parse_doc_json(doc_json)
  let doc = match doc_res {
    Ok(d) => d
    Err(e) => {
      let issues : Array[@cor.Issue] = [
        @cor.mk_issue(
          "core.parse",
          @cor.Severity::Error,
          "文档 JSON 解析失败: " + e,
          None,
        ),
      ]
      return @cor.issues_to_html(issues)
    }
  }
  let ctx : @cor.RuleContext = @cor.RuleContext::{  }
  let issues = run_ruleset(ruleset, ctx, doc)
  @cor.issues_to_html(issues)
}

///|
fn run_ruleset(
  ruleset : String,
  ctx : @cor.RuleContext,
  doc : @cor.Document,
) -> Array[@cor.Issue] {
  if ruleset == "contract" {
    let rs = @con.build_ruleset()
    let rules = @con.as_rules_ref(rs)
    @cor.check_doc(ctx, doc, rules)
  } else {
    // 默认 thesis
    let rs = @the.build_ruleset()
    let rules = @the.as_rules_ref(rs)
    @cor.check_doc(ctx, doc, rules)
  }
}
