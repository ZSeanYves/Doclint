pub struct ContractRuleset {
  page_limit : @the.PageLimitRule
  // TODO：MVP暂时复用
  require_abs : @the.RequireKeywordRule
}

pub fn build_ruleset() -> ContractRuleset {
  {
    page_limit: { min: 1, max: 200 }, // 合同允许更长
    require_abs: {
      rid: "contract.require_title",   // 合同规则可以检查“标题/甲方乙方/签署日期”等
      keyword: "合同",
      sev: @cor.Severity::Warn,
    },
  }
}

pub fn as_rules_ref(rs : ContractRuleset) -> Array[&@cor.Rule] {
  [rs.page_limit, rs.require_abs]
}
