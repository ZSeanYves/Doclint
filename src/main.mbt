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
  // TODO：之后再接 argv
  let rules_name = "thesis"
  let in_path = "src/test_golden/case1.in.json"
  let out_dir = "src/out"
  ensure_dir(out_dir)
  let out_json_path = join_path(out_dir, "report.json")
  let out_html_path = join_path(out_dir, "report.html")
  let doc_json = match read_utf8(in_path) {
    Err(msg) => {
      println(msg)
      return ()
    }
    Ok(s) => s
  }

  // 核心：全部交给纯函数 API
  let json_out = @wasm.check_json_with_ruleset(rules_name, doc_json)
  let html_out = @wasm.check_html_with_ruleset(rules_name, doc_json)
  match write_utf8(out_json_path, json_out) {
    Ok(_) => ()
    Err(msg) => println(msg)
  }
  match write_utf8(out_html_path, html_out) {
    Ok(_) => ()
    Err(msg) => println(msg)
  }
}
