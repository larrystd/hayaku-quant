# Hayaku Quant

Hayaku Quant 是一个以 C++20 为核心、通过 pybind11 提供 Python API 的量化研究与回测引擎。

项目源自 [Hikyuu](https://github.com/fasiondog/hikyuu)，当前正在进行破坏式架构重构：删除过宽的
全局管理器和兼容接口，将数据、执行、策略三个核心职责收口到明确的 Engine 边界。

> 当前状态：**架构重构 POC，尚未发布稳定版本**。
>
> 当前 Python 导入名仍是 `hikyuu`，C++ 库名仍是 `hikyuu`。仓库更名已经完成，包、命名空间和
> 发布物的更名将在边界稳定后单独进行。本项目目前不是上游 Hikyuu 的即插即用替代品。

## 为什么重构

原有代码通过 `StockManager`、`TradeManager`、`System` 和大量顶层接口暴露内部状态，导致数据、
交易、策略、插件及进程生命周期互相耦合。Hayaku Quant 的目标是：

- 用 `HikyuuSession` 显式管理初始化、资源所有权和关闭；
- 用 `DataEngine` 提供稳定、只读的市场数据查询；
- 用 `ExecutionEngine` 统一订单、账户、持仓和账本写入；
- 用 `StrategyEngine` 负责编排策略组件和回测运行；
- Python、pybind11 和 C++ 使用一致的业务模块边界；
- 删除旧接口，而不是长期维护两套 API；
- 优先使用 C++20/C++17 标准能力，并在边界清晰的前提下保持计算性能。

## 当前架构

```text
┌──────────────────────────────────────────────────────────────────────┐
│ 用户代码                                                             │
│ Python / C++                                                         │
└───────────────────────────────┬──────────────────────────────────────┘
                                ▼
┌──────────────────────────────────────────────────────────────────────┐
│ Python API                                                           │
│ hikyuu.data │ hikyuu.execution │ hikyuu.strategy │ hikyuu.analysis │
└───────────────────────────────┬──────────────────────────────────────┘
                                ▼
┌──────────────────────────────────────────────────────────────────────┐
│ pybind11：按业务模块导出稳定 API                                    │
└───────────────────────────────┬──────────────────────────────────────┘
                                ▼
┌──────────────────────────────────────────────────────────────────────┐
│ HikyuuSession                                                        │
│ 配置解析、Engine 所有权、初始化和关闭                               │
└───────────────┬───────────────────┬───────────────────┬──────────────┘
                ▼                   ▼                   ▼
       ┌────────────────┐  ┌──────────────────┐  ┌──────────────────┐
       │ DataEngine     │  │ ExecutionEngine  │  │ StrategyEngine   │
       │ 只读市场数据   │  │ 订单与账户账本   │  │ 策略与回测编排   │
       └───────┬────────┘  └────────┬─────────┘  └────────┬─────────┘
               └────────────────────┴─────────────────────┘
                                    ▼
┌──────────────────────────────────────────────────────────────────────┐
│ common：时间、配置、日志、序列化、线程、网络等非业务基础设施       │
└──────────────────────────────────────────────────────────────────────┘
```

核心边界：

| 模块 | 负责 | 不负责 |
| --- | --- | --- |
| `HikyuuSession` | 配置、组合三个 Engine、生命周期 | 数据查询、记账、逐 Bar 策略逻辑 |
| `DataEngine` | 证券、K 线、板块、日历、财务等只读查询 | 订单、账户和策略状态 |
| `ExecutionEngine` | 订单提交、账户、持仓、资金和成交记录 | 信号判断和行情查询 |
| `StrategyEngine` | 策略组件编排、回测、停止和结果快照 | 直接修改账本或 Driver |
| `analysis` | 对账户和回测结果做只读分析 | 运行策略和写入交易状态 |
| `common` | 无业务含义的通用基础设施 | 证券、行情、账户、订单和策略类型 |

`StockManager`、`TradeManager`、`System` 及对应 Python 旧入口已经从当前 POC 删除。

## 仓库结构

```text
hayaku-quant/
├── hikyuu_cpp/
│   ├── src/                 # C++ 核心实现
│   │   ├── app/             # Session 与应用生命周期
│   │   ├── data/            # 数据领域、DataEngine、Driver、指标和因子
│   │   ├── execution/       # ExecutionEngine、账户、账本、Broker 和费用
│   │   ├── strategy/        # StrategyEngine、策略组件和组合
│   │   ├── analysis/        # 只读绩效与结果分析
│   │   └── common/          # 通用基础设施
│   └── test/                # 与 src 模块对应的 C++ 测试
├── hikyuu_pywrap/           # 按业务模块组织的 pybind11 绑定
├── hikyuu/                  # Python API、研究工具、绘图和数据工具
├── docs/arch/               # 架构分析、重构方案和阶段验收记录
├── test_data/               # 轻量测试数据；大型夹具不进入 Git
├── op.sh                    # 本地构建、测试和诊断入口
└── xmake.lua                # xmake 工程配置
```

## 构建环境

当前 POC 的主要验证环境：

- macOS / Apple Silicon；
- C++20 编译器；
- [xmake](https://xmake.io/) 3.0+；
- Python 3.10；
- Homebrew Python 默认路径 `/opt/homebrew/opt/python@3.10`。

Linux 和 Windows 构建配置继承自上游，但当前重构结果仍需要在 CI 中重新完成跨平台验证。

## 快速开始

克隆并切换到当前开发分支：

```bash
git clone git@github.com:larrystd/hayaku-quant.git
cd hayaku-quant
git switch poc
```

检查本机工具链：

```bash
./op.sh doctor
```

配置并编译 C++ 核心及 Python 3.10 扩展：

```bash
./op.sh configure shared
./op.sh build
./op.sh import-test
```

补齐[大型测试夹具](#测试数据)后，也可以一次完成配置、编译和全部测试：

```bash
./op.sh all shared
```

常用命令：

| 命令 | 用途 |
| --- | --- |
| `./op.sh configure [shared\|static]` | 配置 Release 构建 |
| `./op.sh build` | 编译 C++ 核心和 Python 扩展 |
| `./op.sh small-test` | 构建并运行小型 C++ 测试集 |
| `./op.sh unit-test` | 构建并运行完整 C++ 测试集 |
| `./op.sh python-test` | 使用 Python 3.10 运行 Python 测试 |
| `./op.sh test` | 运行全部测试 |
| `./op.sh clean` | 清理 xmake 构建输出 |

在 macOS/arm64 下，主要产物位于：

```text
build/release/macosx/arm64/lib/libhikyuu.dylib
build/release/macosx/arm64/lib/small-test
build/release/macosx/arm64/lib/unit-test
hikyuu/cpp/core310.so
```

共享库构建下不要直接启动 `small-test` 或 `unit-test`，否则 macOS 可能找不到 OpenSSL 等动态库。
请使用：

```bash
./op.sh run-small-binary
./op.sh run-unit-binary
```

如 Python 或 xmake 不在默认位置，可以覆盖环境变量：

```bash
PYTHON_BIN=/path/to/python3.10 XMAKE_BIN=/path/to/xmake ./op.sh build
```

## Python API 示例

运行前需要准备可用的数据配置；`open_session()` 默认读取 `~/.hikyuu/hikyuu.ini`。

```python
from hikyuu import Query, open_session
from hikyuu.execution import AccountConfig

account = AccountConfig(name="research", initial_cash=300_000.0)

with open_session(account_config=account) as session:
    session.wait_ready()

    stock = session.data.get_stock("sh600000")
    bars = session.data.get_kdata("sh600000", Query(-150))
    account_view = session.execution.view()

    print(stock.market_code)
    print(len(bars))
    print(account_view.funds)
```

Session 关闭后，其 Data、Execution 和 Strategy Engine 引用都会失效。业务代码不应保存跨 Session
的可变运行期对象。

更完整的订单示例见
[`hikyuu/examples/execution_engine.py`](hikyuu/examples/execution_engine.py)。

## 测试数据

为了把 Git 数据控制在 10MB 以内，以下大型集成测试夹具不进入仓库历史：

```text
test_data/sh_1min.h5
test_data/sz_1min.h5
test_data/sh_5min.h5
test_data/sz_5min.h5
test_data/stock.db
test_data/downloads/finance/gpcw20110930.dat
test_data/test_min_data.csv
```

轻量测试数据仍随仓库提供。全量数据相关测试需要在本地补齐上述文件；这些路径已经加入
`.gitignore`，不会被误提交。

## 分支说明

```text
legacy-snapshot
       │
       └── poc
```

- `poc`：默认分支，当前架构重构主线；
- `legacy-snapshot`：重构开始前的单提交代码快照；
- 当前不使用 `main`；边界稳定、跨平台构建和发布流程完成后再建立正式主分支；
- 上游完整 Git 历史不复制到本仓库，仍可从原 Hikyuu 仓库查询。

## 当前进度

已完成：

- 架构与公共 API 清点；
- 三个 Engine 的窄门面；
- `StockManager`、`TradeManager`、`System` 旧体系删除；
- C++、pybind11、Python 目录按业务能力重排；
- C++ 和 Python 接口边界测试；
- Git 历史及大型测试数据瘦身。

下一阶段：

- 消除 `data -> app` 的反向依赖；
- 让 `DataRuntime` 只管理核心数据状态、Driver、缓存和预加载；
- 将数据导入收口为可选的 `ingest` 能力；
- 将实时行情和 IPC 收口为可选的 `realtime` 能力；
- 让 MySQL、ClickHouse 成为明确的可选存储适配器；
- 移除默认初始化中的插件、网络、遥测和商业能力副作用。

## 架构文档

- [原架构分析](docs/arch/old_architecture.md)
- [总体重构方案](docs/arch/refactor.md)
- [Step 0：基线与接口清点](docs/arch/refactor/step-0-progress.md)
- [Step 1：三引擎设计](docs/arch/refactor/step-1-progress.md)
- [Step 2：窄门面实现](docs/arch/refactor/step-2-progress.md)
- [Step 3：旧体系退役与目录重构](docs/arch/refactor/step-3-progress.md)
- [Step 4：数据层、应用层与可选能力收口](docs/arch/refactor/step-4-progress.md)

## 许可证与来源

本项目保留上游 Hikyuu 的版权声明，并继续遵循仓库中的
[Apache License 2.0](LICENSE) 及 [第三方许可证说明](THIRD_PARTY_LICENSES.md)。

Hayaku Quant 是独立的重构实验仓库，与上游项目的正式发布和支持渠道无关。修改或分发代码时，
请同时遵守相关依赖、数据源和交易接口的许可证与合规要求。

## 风险声明

本项目仅用于软件工程、量化研究和回测实验，不构成投资建议，也不提供证券交易服务。策略回测
结果不代表未来收益。使用者应自行承担数据质量、模型偏差、交易接入和实际资金操作带来的风险。
