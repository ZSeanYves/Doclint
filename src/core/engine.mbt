pub fn check_doc(ctx : RuleContext, doc : Document, rules : Array[&Rule]) -> Array[Issue] {
  run_rules(ctx, doc, rules)
}

///|
pub fn check_json(
  doc_json : String,
  rules : Array[&Rule],
  parse_doc : (String) -> Result[Document, String],
) -> String {
  match parse_doc(doc_json) {
    Err(_e) =>
      issues_to_json([
        mk_issue(
          "core.parse",
          Severity::Error,
          "文档 JSON 解析失败",
          None,
        ),
      ])
    Ok(doc) => {
      let ctx : RuleContext = RuleContext::{  }
      let issues = check_doc(ctx, doc, rules)
      issues_to_json(issues)
    }
  }
}


