# Step 6C：Python 顶层与 C++ 八域命名对齐

> 状态：已完成（2026-09-26）
>
> 前置：[Step 6B 顶层目录整理](step-6b-progress.md)

## 当前结构

```text
hayaku/
├── common/       ↔ hayaku_cpp/src/common、hayaku_pywrap/common
├── data/         ↔ data
├── operators/    ↔ operators
├── execution/    ↔ execution
├── metrics/      ↔ metrics
├── strategy/     ↔ strategy
├── application/  ↔ application；含 session、interactive、GUI、CLI、配置和 hub
├── extensions/   ↔ extensions；含 ingest、realtime、visualization、spi
├── _support/     私有 Python 基础设施
├── cpp/、plugin/ 原生模块与插件产物落点
└── __init__.py、_public_api.py、core.py
```

`core.py`、`cpp/` 和 `plugin/` 保持原生加载位置，以维持模块路径、类型 identity、
pickle 路径及动态库查找。`hayaku.__all__` 的 30 个顶层名称不变。

## 导入路径迁移

| Step 6B 路径 | 当前路径 |
|---|---|
| `hayaku.indicator` | `hayaku.operators` |
| `hayaku.analysis` | `hayaku.metrics` |
| `hayaku.apps`、`hayaku.session` | `hayaku.application`、`hayaku.application.session` |
| `hayaku.ingest` | `hayaku.extensions.ingest` |
| `hayaku.realtime` | `hayaku.extensions.realtime` |
| `hayaku.visualization` | `hayaku.extensions.visualization` |
| `hayaku.spi` | `hayaku.extensions.spi` |

旧子模块路径已移除，不留转发包。`hayaku.application` 与 `hayaku.extensions` 的普通导入
保持轻量；交互会话、GUI、绘图和可选原生模块仍由明确的子模块导入触发。

## 验收记录

- Python 3.10：unittest **67/67**、pytest **43/43**；迁移前后 11 个公共模块的
  `__all__` 快照和 29 个顶层原生对象 identity 一致。
- 三个 wheel 构建及隔离安装通过。core wheel 有 **71 个 SQL**、UI、图标、i18n 和
  `core310.so`，不含已移除的顶层包、测试、示例、缓存或可选原生模块。四个 console
  script 从安装包加载成功。
- 中英文 Sphinx 构建、文档结构与 README 对齐检查通过；完整资源对齐检查仍报告
  英中 `_static/10002-function-arc.png` 哈希不同，该差异在本批次前已存在。

执行期间未暂存或提交文件；工作区原有的其它修改予以保留。
