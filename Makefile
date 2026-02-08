.PHONY: wasm demo clean

WASM_OUT := dist/doclint.wasm
DEMO_DIR := examples/node

wasm:
	moon build --target wasm-gc
	mkdir -p dist
	@WASM=$$(find . -name "*.wasm" -not -path "./dist/*" -not -path "./node_modules/*" | xargs -I{} stat -f "%m %N" "{}" | sort -nr | head -n 1 | cut -d' ' -f2-); \
	if [ -z "$$WASM" ]; then echo "error: cannot find wasm output"; exit 1; fi; \
	echo "found wasm: $$WASM"; \
	cp "$$WASM" $(WASM_OUT); \
	echo "ok: wrote $(WASM_OUT)"

demo: wasm
	node --experimental-wasm-imported-strings $(DEMO_DIR)/run.mjs
	@echo "ok: outputs in $(DEMO_DIR)/report.json and report.html"

clean:
	rm -f $(WASM_OUT)
	rm -f $(DEMO_DIR)/report.json $(DEMO_DIR)/report.html
