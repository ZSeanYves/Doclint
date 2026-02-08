pub(all) struct RuleContext {
  // 未来可放：语言、时区、配置等
}

pub(open) trait Rule {
  id(Self) -> String
  check(Self, ctx : RuleContext, doc : Document) -> Array[Issue]
}

pub fn run_rules(
  ctx : RuleContext,
  doc : Document,
  rules : Array[&Rule]
) -> Array[Issue] {
  let all : Array[Issue] = []
  for r in rules {
    let issues = r.check(ctx, doc)
    all.push_iter(issues.iter())
  }
  all
}

