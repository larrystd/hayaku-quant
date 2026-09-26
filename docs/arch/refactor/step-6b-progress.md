# Step 6B：`hayaku` 顶层目录整理

> 状态：已完成（2026-09-26）
>
> 前置：[Step 6 绑定层与包内结构收敛](step-6-progress.md)
>
> 后续八域命名调整见 [Step 6C](step-6c-progress.md)；本文件记录 Step 6B 完成时的结构。

## 目标结构

```text
hayaku/
├── __init__.py、_public_api.py、core.py、session.py
├── common/、data/、indicator/、execution/、analysis/、strategy/、spi/、ingest/
├── realtime/                   可选实时控制
├── visualization/              绘图、研究图表与图标
├── apps/
│   ├── gui/、cli/              GUI 与命令行程序
│   ├── config/                 默认配置和板块资源
│   └── interactive.py、hub.py  显式使用的应用能力
├── _support/                   私有 Python 工具
└── cpp/、plugin/               原生模块和插件产物落点

tests/python/、examples/python/  仓库级测试与示例，不进入 hayaku wheel
```

`core.py`、`cpp/` 和 `plugin/` 的位置保留，以维持原生模块导入路径、类型 identity、
pickle 路径及动态库查找；`session.py` 保留显式会话入口。`apps/__init__.py` 不导入 GUI、
数据库、绘图或交互会话。

## 路径迁移

| 旧路径 | 新路径 |
|---|---|
| `hayaku.advanced` | `hayaku.realtime` |
| `hayaku.draw` | `hayaku.visualization` |
| `hayaku.gui` | `hayaku.apps.gui` |
| `hayaku.shell` | `hayaku.apps.cli` |
| `hayaku.interactive` | `hayaku.apps.interactive` |
| `hayaku.hub` | `hayaku.apps.hub` |
| `hayaku.config` | `hayaku.apps.config` |
| `hayaku.test` | `tests/python/`（仓库内测试目录） |
| `hayaku.examples` | `examples/python/`（仓库内示例目录） |

本批次直接移除旧子模块路径，不留转发包。`hayaku.__all__` 的 30 项公共名称不变；
各领域模块的 `__all__` 与迁移前安装包快照一致。仓库内 import、console scripts、
Notebook、资源路径、CI、构建脚本和中英文文档同步到新路径。

## 验收记录

- 顶层目录从 18 个收敛到 14 个（不计 `__pycache__`）。新包发现结果为 40 个
  `hayaku` 包，旧子模块均不在包清单中。
- `hayaku` 顶层 30 个导出、29 个与 `hayaku.core` 对应的对象 identity、原有领域
  `__all__` 均与 Step 6 安装包快照一致。`realtime` 为 9 项，`visualization` 为 29 项。
- Python 3.10 的 unittest 67 例、pytest 43 例通过；14 个 Notebook 可解析。
  四个 console script 的目标函数从隔离安装的 wheel 成功加载。
- `core`、`ingest`、`realtime` 原生模块构建通过；三个 wheel 完成隔离安装与导入。
  core wheel 含 71 个 SQL、UI、图标、i18n 和 `core310.so`，没有旧顶层包、
  测试、示例、缓存文件或可选原生模块。
- C++ small-test 51 例和 plugin-abi-test 7 例通过；架构扫描器测试、应用依赖检查、
  中英文文档与 README 对齐检查、`git diff --check` 均通过。
- 中英文 Sphinx 文档构建通过；两棵树各有两条原有的未引用参考文献警告。

`hayaku.apps.interactive` 保留显式导入时打开默认数据会话的既有语义；运行该入口需要
有效的数据配置。执行期间没有暂存或提交文件，工作区原有修改予以保留。
