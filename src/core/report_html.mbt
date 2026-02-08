///|
/// 将 Issue 列表渲染成可读 HTML 报告
pub fn issues_to_html(issues : Array[Issue]) -> String {
  let mut err = 0
  let mut warn = 0
  let mut info = 0
  for it in issues {
    match it.severity {
      Severity::Error => err = err + 1
      Severity::Warn => warn = warn + 1
      Severity::Info => info = info + 1
    }
  }
  let mut err_rows = ""
  let mut warn_rows = ""
  let mut info_rows = ""
  for it in issues {
    let row = issue_row_html(it)
    match it.severity {
      Severity::Error => err_rows = err_rows + row
      Severity::Warn => warn_rows = warn_rows + row
      Severity::Info => info_rows = info_rows + row
    }
  }
  let total = issues.length().to_string()
  "<!doctype html>" +
  "<html><head><meta charset=\"utf-8\">" +
  "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">" +
  "<title>Doclint Report</title>" +
  "<style>" +
  "body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial; margin:0; padding:24px; background:#fafafa;}" +
  ".card{background:#fff; border:1px solid #eee; border-radius:12px; padding:16px; margin-bottom:16px; box-shadow:0 1px 2px rgba(0,0,0,.04);}" +
  ".kpi{display:flex; gap:12px; flex-wrap:wrap;}" +
  ".kpi .box{padding:10px 12px; border:1px solid #eee; border-radius:10px; background:#fff;}" +
  "h1{font-size:20px; margin:0 0 12px 0;}" +
  "h2{font-size:16px; margin:16px 0 8px 0;}" +
  "table{width:100%; border-collapse:collapse; background:#fff; border:1px solid #eee; border-radius:12px; overflow:hidden;}" +
  "th,td{padding:10px 12px; border-bottom:1px solid #eee; text-align:left; vertical-align:top;}" +
  "th{background:#f6f7f9; font-weight:600;}" +
  "tr:last-child td{border-bottom:none;}" +
  ".badge{display:inline-block; padding:2px 8px; border-radius:999px; font-size:12px; border:1px solid #eee; background:#fff;}" +
  ".badge.error{border-color:#f3c2c2; background:#fff5f5;}" +
  ".badge.warn{border-color:#f2d6a5; background:#fffaf2;}" +
  ".badge.info{border-color:#b8d4ff; background:#f3f8ff;}" +
  ".muted{color:#666; font-size:12px;}" +
  "</style>" +
  "</head><body>" +
  "<div class=\"card\">" +
  "<h1>Doclint Report</h1>" +
  "<div class=\"kpi\">" +
  "<div class=\"box\"><div class=\"muted\">Total</div><div>" +
  total +
  "</div></div>" +
  "<div class=\"box\"><div class=\"muted\">Error</div><div>" +
  err.to_string() +
  "</div></div>" +
  "<div class=\"box\"><div class=\"muted\">Warn</div><div>" +
  warn.to_string() +
  "</div></div>" +
  "<div class=\"box\"><div class=\"muted\">Info</div><div>" +
  info.to_string() +
  "</div></div>" +
  "</div>" +
  "</div>" +
  section_table("Errors", err_rows) +
  section_table("Warnings", warn_rows) +
  section_table("Infos", info_rows) +
  "</body></html>"
}

///|
fn section_table(title : String, rows : String) -> String {
  if rows == "" {
    "<div class=\"card\">" +
    "<h2>" +
    escape_html(title) +
    "</h2>" +
    "<div class=\"muted\">(none)</div>" +
    "</div>"
  } else {
    "<div class=\"card\">" +
    "<h2>" +
    escape_html(title) +
    "</h2>" +
    "<table>" +
    "<thead><tr>" +
    "<th>rule_id</th><th>severity</th><th>message</th><th>location</th>" +
    "</tr></thead>" +
    "<tbody>" +
    rows +
    "</tbody>" +
    "</table>" +
    "</div>"
  }
}

///|
fn issue_row_html(it : Issue) -> String {
  let badge = severity_badge(it.severity)
  let loc = match it.location {
    None => ""
    Some(l) =>
      "page " +
      l.page.to_string() +
      " [" +
      l.span_start.to_string() +
      "," +
      l.span_end.to_string() +
      ")"
  }
  "<tr>" +
  "<td><code>" +
  escape_html(it.rule_id) +
  "</code></td>" +
  "<td>" +
  badge +
  "</td>" +
  "<td>" +
  escape_html(it.message) +
  "</td>" +
  "<td>" +
  escape_html(loc) +
  "</td>" +
  "</tr>"
}

///|
fn severity_badge(sev : Severity) -> String {
  match sev {
    Severity::Error => "<span class=\"badge error\">Error</span>"
    Severity::Warn => "<span class=\"badge warn\">Warn</span>"
    Severity::Info => "<span class=\"badge info\">Info</span>"
  }
}

///|
/// HTML 最小转义，避免注入/破坏结构
fn escape_html(s : String) -> String {
  let mut out = ""
  for ch in s {
    match ch {
      '<' => out = out + "&lt;"
      '>' => out = out + "&gt;"
      '&' => out = out + "&amp;"
      '"' => out = out + "&quot;"
      '\'' => out = out + "&#39;"
      _ => out = out + ch.to_string()
    }
  }
  out
}
