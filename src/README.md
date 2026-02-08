# Doclint 引擎核心（src/）

本目录包含 Doclint 的 **MoonBit 规则引擎核心**、两套可切换规则集（thesis/contract）、WASM 导出面（string in → string out）以及 golden 测试数据。其目标是将“文档检查”做成可嵌入的、可复用的规则引擎：宿主（Node/Web/后端）提供输入 JSON，WASM 返回 JSON/HTML 报告。

---

## 1. 模块总览（Module Map）

- `core/`：引擎内核与基础数据模型
  - `Document / Page / Issue / Rule` 等核心类型
  - `parse_doc_json`：最小可用 schema 解析
  - `run_rules`：规则执行循环
  - `issues_to_json / issues_to_html`：输出渲染

- `doclint_rules_thesis/`：论文场景规则集（示例）
  - `PageLimitRule`：页数范围校验
  - `RequireKeywordRule`：必须包含关键段落（如“摘要”）

- `doclint_rules_contract/`：合同场景规则集（示例）
  - 当前复用 thesis 的两个规则类型，通过 ruleset 组合装配

- `wasm/`：WASM 引擎导出面（**纯函数**）
  - `check_json_with_ruleset(_wasm)`：返回 JSON 字符串
  - `check_html_with_ruleset(_wasm)`：返回 HTML 字符串

- `test_golden/`：golden 测试输入输出（端到端）
- `Doclint_test.mbt`：测试入口

---

## 2. 核心数据模型（core）

### 2.1 文档结构

引擎处理的文档采用简化 schema：

- `Document`
  - `meta: DocMeta`
  - `pages: Array[Page]`

- `DocMeta`
  - `title: String`
  - `author: String`
  - `created_at: String`

- `Page`
  - `index: Int`
  - `text: String`

> 注：`pages[].text` 是规则检查的主要输入，规则可按页扫描关键字、结构段落等。

### 2.2 诊断结果（Issues）

- `Issue`
  - `rule_id: String`：规则唯一 ID（稳定输出）
  - `severity: Severity`：`Info | Warn | Error`
  - `message: String`：人类可读说明
  - `location: Location?`：可选定位信息

- `Location`
  - `page: Int`
  - `span_start: Int`
  - `span_end: Int`

> MVP 阶段 location 允许为空（`null`），后续可逐步补齐定位能力（页内跨度、段落等）。

### 2.3 Rule Trait（可扩展规则接口）

规则实现 `Rule` trait：

- `id(Self) -> String`
- `check(Self, RuleContext, Document) -> Array[Issue]`

其中 `RuleContext` 目前为空结构体，为后续扩展预留（例如语言、配置、时区、项目约束等）。

---

## 3. 引擎执行流程（core）

引擎的最小执行链路如下：

1) 解析输入 JSON：`parse_doc_json(doc_json) -> Result[Document, String]`  
2) 选择规则集（thesis/contract）并装配成 `Array[&Rule]`  
3) 执行规则：`run_rules(ctx, doc, rules) -> Array[Issue]`  
4) 渲染输出：
   - `issues_to_json(issues) -> String`
   - `issues_to_html(issues) -> String`

### 3.1 关键 API（对齐 moon info）

- `parse_doc_json(String) -> Result[Document, String]`  
  解析项目约定 schema（非通用 JSON 解析器，MVP 有意保持小而可控）。

- `run_rules(RuleContext, Document, Array[&Rule]) -> Array[Issue]`  
  按顺序运行规则，并收集所有 Issue。

- `check_doc(RuleContext, Document, Array[&Rule]) -> Array[Issue]`  
  对外更语义化的入口（内部调用 `run_rules`）。

- `issues_to_json(Array[Issue]) -> String`  
- `issues_to_html(Array[Issue]) -> String`  
  输出渲染：JSON 便于机器处理；HTML 便于展示与验收。

> 兼容性提示：WASM 侧强烈建议只使用 `src/wasm` 的导出 API（见第 5 节），避免宿主与内部结构耦合。

---

## 4. 规则集（Rulesets）

### 4.1 Thesis ruleset（doclint_rules_thesis）

对外提供两组装配函数：

- `build_ruleset() -> ThesisRuleset`
- `as_rules_ref(ThesisRuleset) -> Array[&@core.Rule]`

当前包含规则：

- `PageLimitRule { min, max }`
  - 典型 rule_id：`thesis.page_limit`
  - 示例行为：当页数不在 [min, max] 时输出 Error

- `RequireKeywordRule { rid, keyword, sev }`
  - `rid` 作为 rule_id
  - 示例：必须出现 “摘要”，否则输出 Error

### 4.2 Contract ruleset（doclint_rules_contract）

对外函数：

- `build_ruleset() -> ContractRuleset`
- `as_rules_ref(ContractRuleset) -> Array[&@core.Rule]`

当前 `ContractRuleset` 复用 thesis 的两条规则类型作为字段（组合装配），用来证明“同一引擎、不同规则包”的可扩展性。

---

## 5. WASM 引擎导出面（src/wasm）

WASM 包提供 **纯函数** API：输入为字符串，输出为字符串，适合被 Node/Web/后端嵌入。

### 5.1 API 列表（对齐 moon info）

- `check_json_with_ruleset(ruleset: String, doc_json: String) -> String`
- `check_html_with_ruleset(ruleset: String, doc_json: String) -> String`

稳定导出面（推荐宿主使用）：

- `check_json_with_ruleset_wasm(ruleset: String, doc_json: String) -> String`
- `check_html_with_ruleset_wasm(ruleset: String, doc_json: String) -> String`

### 5.2 统一错误策略（parse 失败也可输出报告）

当 `parse_doc_json` 失败时，WASM API 不抛异常，而是返回一份“可渲染报告”：

- JSON：包含 `core.parse` 的 Error Issue
- HTML：同样可读（Error 计数 + 列表）

> 这样宿主无需区分“引擎错误 vs 检查错误”，统一按报告展示/记录。

### 5.3 ruleset 分发策略

当前 MVP 使用字符串分发：

- `ruleset == "contract"` → contract
- 否则默认 thesis

后续可以演进为：
- `match ruleset` + 更明确的错误提示（unknown ruleset）
- 支持自定义规则集注册（插件式）

---

## 6. 测试（Golden）

- golden 输入输出目录：`src/test_golden/`
- 测试入口：`src/Doclint_test.mbt`

golden 测试的目标是端到端保证：

`in.json -> parse_doc_json -> check_json_with_ruleset -> out.json`

建议每新增一条规则，都补充至少一条 golden case，确保行为可回归、输出稳定。

---

## 7. 如何扩展（开发指南）

### 7.1 新增一条规则

1) 在某个 ruleset 包中新增 `struct MyRule { ... }`
2) `pub impl @core.Rule for MyRule`
3) 在 `build_ruleset()` 中实例化并放入 ruleset 结构体
4) 在 `as_rules_ref()` 中把该规则加入返回数组

### 7.2 新增一个 ruleset 包（例如 `doclint_rules_resume`）

1) 新建包目录 `src/doclint_rules_resume/`
2) 提供：
   - `build_ruleset() -> ResumeRuleset`
   - `as_rules_ref(ResumeRuleset) -> Array[&@core.Rule]`
3) 在 `src/wasm` 的 ruleset 分发处加一个分支

---

## 8. 当前限制与后续方向（Roadmap）

MVP 限制（刻意为之）：
- `parse_doc_json` 只支持项目 schema（非通用 JSON）
- 规则数量少（用于验证规则引擎形态与可切换 ruleset）
- location 可为空（后续可逐步增强定位）

可演进方向：
- 更强的定位：按页/按段落/按行列
- 更多规则类型：结构段落、引用格式、标题层级、敏感词等
- 规则配置化：从 JSON/TOML 配置生成 ruleset
- 报告增强：severity 过滤、分组、排序、摘要统计
