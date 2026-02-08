pub(all) struct PageLimitRule {
  min : Int
  max : Int
}

///|
pub impl @cor.Rule for PageLimitRule with id(_self) -> String {
  "thesis.page_limit"
}

///|
pub impl @cor.Rule for PageLimitRule with check(
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
pub impl @cor.Rule for RequireKeywordRule with id(self) -> String {
  self.rid
}

///|
pub impl @cor.Rule for RequireKeywordRule with check(
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
pub(all) struct ThesisRuleset {
  page_limit : PageLimitRule
  require_abs : RequireKeywordRule
}

pub fn build_ruleset() -> ThesisRuleset {
  {
    page_limit: { min: 2, max: 50 },
    require_abs: {
      rid: "basic.require_abstract",
      keyword: "摘要",
      sev: @cor.Severity::Error,
    },
  }
}

///|
pub fn as_rules_ref(rs : ThesisRuleset) -> Array[&@cor.Rule] {
  [rs.page_limit, rs.require_abs]
}
