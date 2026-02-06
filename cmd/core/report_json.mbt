pub fn severity_to_string(s : Severity) -> String {
  match s {
    Severity::Info => "Info"
    Severity::Warn => "Warn"
    Severity::Error => "Error"
  }
}

fn escape_str(s : String) -> String {
  let mut out = ""
  for ch in s {
    match ch {
      '"' => out = out + "\\\""
      '\\' => out = out + "\\\\"
      '\n' => out = out + "\\n"
      '\r' => out = out + "\\r"
      '\t' => out = out + "\\t"
      _ => out = out + ch.to_string()
    }
  }
  out
}

fn quote(s : String) -> String { "\"" + escape_str(s) + "\"" }

pub fn location_to_json(loc : Location) -> String {
  "{"+ 
  "\"page\":" + loc.page.to_string() + 
  ","+ 
  "\"span_start\":" + loc.span_start.to_string() + 
  ","+ 
  "\"span_end\":" + loc.span_end.to_string()+ 
  "}"
}

pub fn issue_to_json(it : Issue) -> String {
  let loc_json =
    match it.location {
      None => "null"
      Some(loc) => location_to_json(loc)
    }
  "{"+ 
  "\"rule_id\":" + quote(it.rule_id) + 
  ","+ 
  "\"severity\":" + quote(severity_to_string(it.severity)) + 
  ","+ 
  "\"message\":" + quote(it.message) + 
  ","+ 
  "\"location\":" + loc_json+ 
  "}"
}

pub fn issues_to_json(issues : Array[Issue]) -> String {
  let parts : Array[String] = []
  for it in issues {
    parts.push(issue_to_json(it))
  }
  "{"+ 
  "\"issues\":[" + parts.join(",") + 
  "]"+ 
  "}"
}
