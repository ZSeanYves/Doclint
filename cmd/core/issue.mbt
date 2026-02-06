pub(all) enum Severity {
  Info
  Warn
  Error
}

pub(all) struct Location {
  page : Int
  span_start : Int
  span_end : Int
}

pub(all) struct Issue {
  rule_id : String
  severity : Severity
  message : String
  location : Option[Location]
}

pub fn mk_issue(
  rule_id : String,
  severity : Severity,
  message : String,
  location : Option[Location]
) -> Issue {
  { rule_id, severity, message, location }
}
