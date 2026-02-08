///|
type IoResult[T] = Result[T, String]

///|
fn read_utf8(path : String) -> IoResult[String] {
  let s = @fs.read_file_to_string(path, encoding="utf8") catch {
    _ => return Err("read failed: " + path)
  }
  Ok(s)
}

///|
fn write_utf8(path : String, content : String) -> IoResult[Unit] {
  @fs.write_string_to_file(path, content, encoding="utf8") catch {
    _ => return Err("write failed: " + path)
  }
  Ok(())
}

///|
fn ensure_dir(path : String) -> Unit {
  if @fs.path_exists(path) {
    if !(@fs.is_dir(path) catch { _ => false }) {
      println("path exists but not a dir: " + path)
    }
  } else {
    @fs.create_dir(path) catch {
      _ => println("failed to create dir: " + path)
    }
  }
}

///|
fn join_path(dir : String, file : String) -> String {
  // 简化：dir 不以 / 结尾
  dir + "/" + file
}


///|
fn main {
  let rules_name = "thesis"
  let in_path = "cmd/test/golden/case2.in.json"
  let out_dir = "cmd/out"
  ensure_dir(out_dir)
  let out_json_path = join_path(out_dir, "report.json")
  let out_html_path = join_path(out_dir, "report.html")
  let doc_json = match read_utf8(in_path) {
    Err(_msg) => return ()
    Ok(s) => s
  }
  let doc = match @cor.parse_doc_json(doc_json) {
    Err(e) => {
      let issues : Array[@cor.Issue] = [
        @cor.mk_issue(
          "core.parse",
          @cor.Severity::Error,
          "文档 JSON 解析失败: " + e,
          None,
        ),
      ]
      // parse 失败：仍然输出 json + html
      let _ = write_utf8(out_json_path, @cor.issues_to_json(issues))
      let _ = write_utf8(out_html_path, @cor.issues_to_html(issues))
      return ()
    }
    Ok(d) => d
  }
  let ctx : @cor.RuleContext = @cor.RuleContext::{  }
  let issues = if rules_name == "contract" {
    let rs = @con.build_ruleset()
    let rules = @con.as_rules_ref(rs)
    @cor.check_doc(ctx, doc, rules)
  } else {
    let rs = @the.build_ruleset()
    let rules = @the.as_rules_ref(rs)
    @cor.check_doc(ctx, doc, rules)
  }

  // 成功：输出 json + html
  let _ = write_utf8(out_json_path, @cor.issues_to_json(issues))
  let _ = write_utf8(out_html_path, @cor.issues_to_html(issues))

}



