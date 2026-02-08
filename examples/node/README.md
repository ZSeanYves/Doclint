# 1) build wasm
moon build --target wasm-gc

# 2) run node demo
node --experimental-wasm-imported-strings examples/node/run.mjs

# 3) check outputs
# examples/node/report.json
# examples/node/report.html
