# ZSeanYves/Doclint
## Doclint WASM Engine (MVP)

This project compiles a **MoonBit** rule-checking core into a **WASM (wasm-gc) engine**, and ships a **Node host demo** that takes a document JSON as input and produces both:

* `report.json` (machine-readable)
* `report.html` (human-readable)

### Stable WASM Exports

* `check_json_with_ruleset_wasm(ruleset: String, doc_json: String) -> String`
* `check_html_with_ruleset_wasm(ruleset: String, doc_json: String) -> String`

### Ruleset Switching

Supported rulesets (demo defaults to `thesis`):

* `thesis`
* `contract`

---

## Requirements

* MoonBit toolchain installed (`moon` available in PATH)
* Node.js (a recent version is recommended)

  * The demo uses **imported strings**, so Node must be run with the corresponding experimental flag.

> Note: The WASM engine is **pure** (string in → string out) and does **not** rely on a filesystem. All file I/O is handled by the host (Node).

---

## Quick Run (Recommended)

If the repository provides a `Makefile`, run:

```bash
make demo
```

Outputs will be generated at:

* `examples/node/report.json`
* `examples/node/report.html`

---

## Manual Run (Without Makefile)

### 1) Build WASM

```bash
moon build --target wasm-gc
```

It is recommended to copy the build artifact to a stable location:

* `dist/doclint.wasm`

(If you use a script/Makefile, the copy step can be automated.)

### 2) Run the Node Demo (Enable imported strings)

```bash
node --experimental-wasm-imported-strings examples/node/run.mjs
```

### 3) Check Outputs

* `examples/node/report.json`
* `examples/node/report.html`

---

## Input / Output

* Input file: `examples/node/input.case1.json`

  * You can replace it with other inputs as long as they follow the project’s document schema (`meta` + `pages[]`).
* Output files:

  * `examples/node/report.json`: issues list (machine-readable)
  * `examples/node/report.html`: HTML report (summary + table)

---

## Project Layout (Key Parts)

```text
dist/
  doclint.wasm
examples/
  node/
    run.mjs
    input.case1.json
    report.json        # generated after running
    report.html        # generated after running
src/
  ... (MoonBit core and rulesets)
```

---

## Troubleshooting

### Node reports errors related to imported strings

Make sure you are running:

```bash
node --experimental-wasm-imported-strings examples/node/run.mjs
```

Also ensure your Node version is recent enough. Older versions may not support the required experimental WASM features.

### Copying the `.wasm` into `dist/` is annoying

Use a `Makefile` or a small script to automate “build + copy”, f
