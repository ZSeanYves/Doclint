import fs from "node:fs";

const wasmBytes = fs.readFileSync(new URL("../../dist/doclint.wasm", import.meta.url));
const docJson = fs.readFileSync(new URL("./input.case1.json", import.meta.url), "utf8");

const { instance } = await WebAssembly.instantiate(
  wasmBytes,
  {},
  {
    builtins: ["js-string"],
    importedStringConstants: "_",
  }
);

const jsonOut = instance.exports.check_json_with_ruleset_wasm("thesis", docJson);
const htmlOut = instance.exports.check_html_with_ruleset_wasm("thesis", docJson);

fs.writeFileSync(new URL("./report.json", import.meta.url), jsonOut);
fs.writeFileSync(new URL("./report.html", import.meta.url), htmlOut);

console.log("ok: wrote examples/node/report.json and report.html");
