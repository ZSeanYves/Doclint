# Doclint

Doclint is a **MoonBit-powered rule engine** compiled to **WASM (wasm-gc)**. A Node host demo is provided to feed document JSON into the engine and generate:

* `report.json` (machine-readable issues)
* `report.html` (human-readable report)

---

## What you get (MVP)

* **WASM engine artifact**: `dist/doclint.wasm`
* **Stable WASM exports** (string in → string out):

  * `check_json_with_ruleset_wasm(ruleset: String, doc_json: String) -> String`
  * `check_html_with_ruleset_wasm(ruleset: String, doc_json: String) -> String`
* **Ruleset switching**: `thesis` / `contract`

---

## Quickstart

### Option A: One-command demo (recommended)

If a `Makefile` is available at repo root:

```bash
make demo
```

Outputs:

* `examples/node/report.json`
* `examples/node/report.html`

### Option B: Manual run

1. Build WASM:

```bash
moon build --target wasm-gc
```

2. Run Node demo (enable imported strings):

```bash
node --experimental-wasm-imported-strings examples/node/run.mjs
```

3. Check outputs:

* `examples/node/report.json`
* `examples/node/report.html`

---

## Where to look next

* **Run & host integration (Node demo)**: `examples/node/README.md`
* **Engine core (API, schema, rulesets, tests)**: `src/README.md`
* **Rulesets**:

  * `thesis`: `src/doclint_rules_thesis/`
  * `contract`: `src/doclint_rules_contract/`
* **Golden tests**: `src/test_golden/` and `src/Doclint_test.mbt`

---

## Requirements

* MoonBit toolchain (`moon`)
* Node.js (recent version recommended)

  * Run with: `--experimental-wasm-imported-strings`

> Note: The WASM engine itself is pure (no filesystem). File I/O is handled by the host.
