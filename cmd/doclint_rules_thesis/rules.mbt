pub(all) struct PageLimitRule {
  min : Int
  max : Int
}

///|
impl @cor.Rule for PageLimitRule with id(_self) -> String {
  "thesis.page_limit"
}

///|
impl @cor.Rule for PageLimitRule with check(
  self,
  _ctx : @cor.RuleContext,
  doc : @cor.Document,
) -> Array[@cor.Issue] {
  let n = @cor.page_count(doc)
  if n < self.min || n > self.max {
    [
      @cor.mk_issue(
        self.id(),
        @cor.Severity::Error,
        "页数不符合要求",
        None,
      ),
    ]
  } else {
    []
  }
}


pub(all) struct RequireKeywordRule {
  rid : String
  keyword : String
  sev : @cor.Severity
}

///|
impl @cor.Rule for RequireKeywordRule with id(self) -> String {
  self.rid
}

///|
impl @cor.Rule for RequireKeywordRule with check(
  self,
  _ctx : @cor.RuleContext,
  doc : @cor.Document,
) -> Array[@cor.Issue] {
  let mut all_text = ""
  for p in doc.pages {
    all_text = all_text + "\n" + p.text
  }
  if all_text.contains(self.keyword) {
    []
  } else {
    [
      @cor.mk_issue(
        self.id(),
        self.sev,
        "缺少必需内容：" + self.keyword,
        None,
      ),
    ]
  }
}


///|
test "basic rules should work" {
  let doc : @cor.Document = {
    meta: { title: "t", author: "a", created_at: "x" },
    pages: [{ index: 0, text: "正文\n参考文献\n[1] ..." }],
  }
  let page_rule : PageLimitRule = { min: 2, max: 50 }
  let abs_rule : RequireKeywordRule = {
    rid: "basic.require_abstract",
    keyword: "摘要",
    sev: @cor.Severity::Error,
  }
  let rules : Array[&@cor.Rule] = [page_rule, abs_rule]
  let issues = @cor.run_rules(@cor.RuleContext::{  }, doc, rules)

  // 应该缺摘要
  let mut found = false
  for it in issues {
    if it.rule_id == "basic.require_abstract" {
      found = true
    }
  }
  assert_eq(found, true)
}