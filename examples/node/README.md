# Node Host Demo (WASM)

This demo shows how to call the Doclint WASM engine from Node.js:

- Input: `input.case1.json`
- Output: `report.json` and `report.html`

## Requirements

- Node.js (recent)
- Run with imported strings enabled:
  - `--experimental-wasm-imported-strings`

## Run

From repo root:

```bash
node --experimental-wasm-imported-strings examples/node/run.mjs
