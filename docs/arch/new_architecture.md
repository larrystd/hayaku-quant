# Hayaku C++ 架构总览

> 本文描述当前 C++ 核心。重构前的结构见[旧架构](old_architecture.md)，目录迁移记录见[第 6C 步](refactor/step-6c-progress.md)。

## 1. 核心结构

`hayaku_cpp/src` 按业务分成八个目录。C++ 应用从 `HayakuSession` 进入数据运行时；需要账户和单策略回测时，再显式创建相应对象。如使用 Python，pybind11 绑定层调用同一套 C++ 实现。

```text
┌──────────────────────────────────────────────────────────────────┐
│ C++ application / pybind11 caller                                │
└──────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
┌──────────────────────────────────────────────────────────────────┐
│ application: HayakuSession / process runtime / plugin assembly   │
└──────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
┌──────────────────────────────────────────────────────────────────┐
│ data:       DataEngine -> DataRuntime -> storage drivers         │
│ strategy:   StrategyEngine / StrategyRuntime / Portfolio         │
│ execution:  ExecutionEngine -> ExecutionRuntime -> Ledger        │
│ operators:  Indicator / Factor                                   │
│ metrics:    Performance        common: shared infrastructure     │
│ extensions: realtime / ingest / optional adapters                │
└──────────────────────────────────────────────────────────────────┘
```

图中的 `data → strategy` 是 `Stock`、`KData` 等数据输入；`strategy → execution` 通过账户端口提交订单。`metrics` 消费账户记录做统计，不参与下单。目录是**逻辑领域边界**：当前 [`BUILD.bazel`](../../hayaku_cpp/src/BUILD.bazel) 把大部分 C++ 源码编进一个 `:core` 目标，并没有为八个目录各建独立库；导入和实时前端才另有目标。

| C++ 目录 | 主要职责 | 代表代码 |
| --- | --- | --- |
| [`application`](../../hayaku_cpp/src/application/) | 会话、进程资源、插件加载与运行时装配 | [`HayakuSession`](../../hayaku_cpp/src/application/HayakuSession.h)、[`GlobalInitializer`](../../hayaku_cpp/src/application/GlobalInitializer.cpp)、[`DataRuntimeAssembly`](../../hayaku_cpp/src/application/DataRuntimeAssembly.cpp) |
| [`data`](../../hayaku_cpp/src/data/) | 数据查询门面、进程共享运行时、证券和 K 线模型 | [`DataEngine`](../../hayaku_cpp/src/data/DataEngine.h)、[`DataRuntime`](../../hayaku_cpp/src/data/DataRuntime.h)、[`Stock`](../../hayaku_cpp/src/data/Stock.h) |
| [`data/storage`](../../hayaku_cpp/src/data/storage/) | 基础信息、板块、K 线驱动的选择与连接管理 | [`DataDriverFactory`](../../hayaku_cpp/src/data/storage/DataDriverFactory.h)、[`KDataDriver`](../../hayaku_cpp/src/data/storage/KDataDriver.h) |
| [`operators`](../../hayaku_cpp/src/operators/) | 指标表达式、指标实现、因子计算及持久化接口 | [`Indicator`](../../hayaku_cpp/src/operators/Indicator.h)、[`IndicatorImp`](../../hayaku_cpp/src/operators/IndicatorImp.h)、[`FactorStore`](../../hayaku_cpp/src/operators/FactorStore.h) |
| [`strategy`](../../hayaku_cpp/src/strategy/) | 单策略定义/运行、决策与风险组件、选股和组合 | [`StrategyDefinition`](../../hayaku_cpp/src/strategy/StrategyDefinition.h)、[`StrategyRuntime`](../../hayaku_cpp/src/strategy/StrategyRuntime.h)、[`Portfolio`](../../hayaku_cpp/src/strategy/portfolio/Portfolio.h) |
| [`execution`](../../hayaku_cpp/src/execution/) | 订单处理、账户状态、账本、交易记录及账户端口 | [`ExecutionEngine`](../../hayaku_cpp/src/execution/ExecutionEngine.h)、[`ExecutionRuntime`](../../hayaku_cpp/src/execution/ExecutionRuntime.h)、[`Ledger`](../../hayaku_cpp/src/execution/Ledger.h) |
| [`metrics`](../../hayaku_cpp/src/metrics/) | 账户绩效和批量统计 | [`Performance`](../../hayaku_cpp/src/metrics/Performance.h)、[`BatchMetrics`](../../hayaku_cpp/src/metrics/BatchMetrics.h) |
| [`common`](../../hayaku_cpp/src/common/)、[`extensions`](../../hayaku_cpp/src/extensions/) | 公共基础设施；可选原生能力 | 见各子目录 |

主要调用关系是：`application` 负责组装和释放运行时；`strategy` 消费 `data` 的 K 线及 `operators` 的计算结果，经 `execution` 定义的账户端口下单；`execution` 处理订单和账本，不运行信号判断；`metrics` 读取账户记录。可选实现通过核心侧接口接入。这些是代码职责边界，目前尚非 Bazel 强制的逐目录依赖规则。

## 2. 生命周期与所有权

### 2.1 打开和关闭会话

[`HayakuSession::open()`](../../hayaku_cpp/src/application/HayakuSession.cpp) 先调用 `acquireProcessRuntime()`。第一次获取时，进程层初始化指标引擎、日志、TA-Lib（启用时）、任务组及 `DataDriverFactory`。第一个会话再创建 `DataRuntime`，装配插件/数据驱动，加载基础数据并启动需要的预加载。后续并存会话复用同一个数据运行时，且 `SessionOptions` 必须完全相同。`ready()` 和 `waitReady()` 用于观察、等待数据准备完成。

```text
┌──────────────────────────────────────────────────────────────┐
│ First HayakuSession::open(options)                           │
└──────────────────────────────────────────────────────────────┘
                               │
                               ▼
┌──────────────────────────────────────────────────────────────┐
│ acquireProcessRuntime(): process-wide initialization         │
│ DataRuntimeAssembly: plugin / optional adapter wiring        │
│ DataRuntime::init(): drivers, securities and preload         │
└──────────────────────────────────────────────────────────────┘
                               │
                               ▼
┌──────────────────────────────────────────────────────────────┐
│ N sessions share one DataRuntime and identical options       │
│ Each may own an ExecutionEngine and a bound StrategyEngine   │
└──────────────────────────────────────────────────────────────┘
                               │
                               ▼
┌──────────────────────────────────────────────────────────────┐
│ Last close: stop realtime, scheduler and preload             │
│ Release DataRuntime and adapter wiring                       │
│ releaseProcessRuntime(): process-wide cleanup                │
└──────────────────────────────────────────────────────────────┘
```

`close()` 会先令该会话失效，并等待它**通过 `bindStrategy()` 持有**的策略停止。最后一个会话关闭才释放共享数据运行时。`close()` 不能在 `SpotAgent` 回调内部调用；应在回调结束后从持有会话的线程调用。插件库由进程级 [`PluginRuntime`](../../hayaku_cpp/src/application/PluginRuntime.h) 管理，关闭会话不热卸载插件库。

### 2.2 对象所有权

| 对象 | 谁持有 | 生命周期边界 |
| --- | --- | --- |
| `DataRuntime` | 进程级数据运行时状态 | 首个会话创建；最后一个会话关闭时释放。 |
| `DataEngine` | `HayakuSession` 的值成员 | 保存 `DataRuntime*` 作为借用引用；每次查询检查会话活动状态。 |
| `ExecutionEngine` | 可由会话的 `unique_ptr` 持有，也可在 C++ 中独立构造 | 内部以 `shared_ptr` 持有该账户的 `ExecutionRuntime`；会话绑定的门面会检查关闭状态。 |
| `StrategyEngine` | `HayakuSession::bindStrategy()` 的 `unique_ptr`，或由 C++ 调用方独立持有 | 保存定义、`StrategyRuntime` 和账户端口；同一引擎不允许并发执行两次 `run()`。 |
| `Ledger` | `ExecutionRuntime` 的内部成员 | 现金、持仓、借贷、成交等可变账户状态的存储位置。 |

`HayakuSession::open(options, accountConfig)` 才安装会话执行账户。C++ 的 `bindStrategy(definition)` 要求该账户存在；同一会话再次绑定相同定义会复用现有引擎，改绑不同定义会报错。直接构造 `StrategyEngine(definition, execution)` 是另一种 C++ 用法：引擎共享底层账户端口，调用方自行管理该引擎的停止与运行所需数据的有效期。相关检查见 [`StrategyEngine.cpp`](../../hayaku_cpp/src/strategy/StrategyEngine.cpp) 和 [`ExecutionEngine.cpp`](../../hayaku_cpp/src/execution/ExecutionEngine.cpp)。

## 3. 数据与指标

### 3.1 行情读取

`DataEngine` 是只读查询门面，提供证券、K 线、市场、板块、交易日历、权息和历史财务查询。它通过共享 `DataRuntime` 访问缓存与驱动；例如 `getKData(code, query)` 先取得 `Stock`，再调用 `Stock::getKData(query)`。`DataRuntime` 初始化时从 `DataDriverFactory` 取得基础信息、板块、K 线驱动；工厂按配置选实现，K 线驱动可走连接池。预加载是缓存路径，驱动仍是数据来源。

```text
┌──────────────────────────────────────────────┐
│ DataEngine::getKData(code, query)            │
└──────────────────────────────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────┐
│ DataRuntime::getStock(code)                  │
│ Stock::getKData(query)                       │
└──────────────────────────────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────┐
│ Cache / KDataDriver connection pool          │
│ Local or configured proxy driver             │
└──────────────────────────────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────┐
│ KData / KRecord                              │
└──────────────────────────────────────────────┘
```

驱动接口在 `data/storage`，具体实现分布在 `data/storage` 和 `extensions`。共享内存客户端可通过 [`RealtimeDataSource`](../../hayaku_cpp/src/data/RealtimeDataSource.h) 接入代理驱动；是否使用由会话配置与可选插件决定。默认 Bazel 配置包含 HDF5、SQLite、TDX；ClickHouse 依赖相应插件与配置，MySQL 不在当前 Bazel 构建配置中。

### 3.2 指标和因子

`Indicator` 是用户持有的句柄，其 `impl_` 指向 `IndicatorImp`。表达式由算子和子指标组合，计算实现、参数与结果缓冲主要在 `IndicatorImp` 及其派生实现中；`KData` 可以作为计算上下文。`Factor` 在指标公式之上组织按证券计算的因子；需要保存因子或让后端计算时，经 [`FactorStore`](../../hayaku_cpp/src/operators/FactorStore.h) 接口接入相应实现。纯序列指标计算不要求先打开数据会话，依赖证券行情的计算则需要可用的数据运行时。

## 4. 策略、执行与统计

### 4.1 单策略回测

`StrategyDefinition` 在构造时保存资金管理、信号以及可选的环境、条件、止损、止盈、盈利目标、滑点组件；资金管理和信号必填。定义对象没有公开的组件 setter，但组件本身由指针引用，组件对象也可能有内部状态；运行状态机和挂单等状态在 `StrategyRuntime`。`BacktestRequest` 提供本次 `KData` 与重置选项。

`StrategyEngine` 创建 `StrategyRuntime`，同步运行 `run(request)`，并返回含证券、查询区间、成交记录的 `BacktestResult` 值快照。内部运行时逐条处理 K 线，判断组件条件和待处理订单；可用 `stop()` 请求协作式停止。结果快照不包含完整账户资金/持仓，后者从 `ExecutionEngine` 查询。见 [`StrategyRuntime.cpp`](../../hayaku_cpp/src/strategy/StrategyRuntime.cpp)、[`BacktestResult.h`](../../hayaku_cpp/src/strategy/BacktestResult.h)。

### 4.2 账户边界

```text
┌──────────────────────────────────────────────────────────────────┐
│ StrategyRuntime -> StrategyExecutionPort                         │
│ C++ direct order caller -> ExecutionEngine                       │
└──────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
┌──────────────────────────────────────────────────────────────────┐
│ ExecutionAccountPort -> ExecutionRuntime -> Ledger               │
│ OrderRequest (resolved price) -> ExecutionReport / TradeRecord   │
└──────────────────────────────────────────────────────────────────┘
```

`ExecutionEngine` 负责同步提交订单与提供 `AccountSnapshot`、`AccountView`、历史记录；[`OrderRequest`](../../hayaku_cpp/src/execution/OrderRequest.h) 已包含确定的执行价，信号判断、行情查询和滑点计算留在策略一侧。内部 `ExecutionRuntime` 实现 [`ExecutionAccountPort`](../../hayaku_cpp/src/execution/ExecutionAccountPort.h)，使策略只依赖需要的账户操作。组合调仓需要额外的 [`PortfolioAccountPort`](../../hayaku_cpp/src/execution/PortfolioAccountPort.h)；券商同步另走 [`ExecutionBrokerPort`](../../hayaku_cpp/src/execution/ExecutionBrokerPort.h)。绩效统计由 [`Performance`](../../hayaku_cpp/src/metrics/Performance.h) 等读取账户数据完成。

### 4.3 仍在 C++ 中的其他运行路径

[`strategy/portfolio`](../../hayaku_cpp/src/strategy/portfolio/) 仍包含 `Portfolio`、选股和资金分配实现；[`Strategy`](../../hayaku_cpp/src/strategy/Strategy.h) 与 [`StrategyRunners`](../../hayaku_cpp/src/strategy/StrategyRunners.h) 仍用于调度及实时相关路径。它们与上面的单策略 `StrategyDefinition` / `StrategyEngine` 路径同时存在，不能把 Python 已删除的旧 `System` / `Portfolio` 绑定理解成 C++ 代码也已移除。后续 C++ 文档应分别说明这两类入口和它们的适用场景。

## 5. 扩展接口与构建边界

可选能力通过核心定义的接口或端口接入。应用装配层负责找插件和注册实现，核心运行时使用接口调用，避免把插件加载逻辑放进 `DataRuntime`。

| 边界 | 核心侧接口 | 当前用途 |
| --- | --- | --- |
| 数据驱动 | [`BaseInfoDriver`](../../hayaku_cpp/src/data/storage/BaseInfoDriver.h)、[`KDataDriver`](../../hayaku_cpp/src/data/storage/KDataDriver.h)、`DataDriverFactory` | 根据 `SessionOptions` 选择存储实现。 |
| 实时/共享内存 | [`RealtimePort`](../../hayaku_cpp/src/extensions/realtime/RealtimePort.h)、[`RealtimeDataSource`](../../hayaku_cpp/src/data/RealtimeDataSource.h) | 注册实时服务、共享内存客户端数据源。 |
| 因子存储 | [`FactorStore`](../../hayaku_cpp/src/operators/FactorStore.h) | 可选持久化与后端因子计算。 |
| 扩展报表 | [`ReportExtension`](../../hayaku_cpp/src/metrics/ReportExtension.h) | 可选绩效报告。 |

Bazel 的 `:core_shared` 产出 `libhayaku.so`；`:ingest_shared` 和 `:realtime_shared` 分别产出可选共享库，并动态依赖核心库。Python 绑定层另构建三个 Python 3.10 扩展。实时 `spot_generated.h` 从 [`spot.fbs`](../../hayaku_cpp/src/extensions/realtime/spot.fbs) 生成。构建目标目前只配置 macOS/Linux；依赖版本固定在 [`MODULE.bazel`](../../MODULE.bazel) 与锁文件。命令、产物路径与特性开关见 [`BAZEL.md`](../../BAZEL.md)。

## 6. 文档计划

本文件作为 C++ 架构总览，后续文档按下面顺序补充。每篇以实际头文件、调用链和 C++ 示例为准，再同步中文使用文档。

| 顺序 | 交付物 | 内容与验收 |
| --- | --- | --- |
| P0 | 新增 `docs/arch/cpp/developer.md` | 写可编译的 C++ 最小示例：打开/关闭 Session、等待数据、安装账户、绑定策略；示例对应当前头文件与 Bazel 目标。 |
| P1 | `docs/arch/cpp/session-and-data.md` | 画清进程资源、共享 `DataRuntime`、会话状态、预加载和驱动选择；覆盖多会话相同配置、关闭顺序及回调限制。 |
| P1 | `docs/arch/cpp/strategy-and-execution.md` | 分开单策略、组合、实时 `Strategy` 路径；列出定义、运行状态、账户端口、账本、订单和结果快照的所有权与调用顺序。 |
| P1 | `docs/arch/cpp/extensions-and-build.md` | 说明数据驱动、因子、实时、报表四类扩展接口，以及 core/ingest/realtime 的 Bazel 目标和生成文件；示例链接到实际实现。 |
| P2 | Markdown API/使用说明与 README | 在 README 链接 C++ 架构页；校对 Python 与 C++ 入口差异，清理已删除的导入路径。 |

完成每项后检查本地链接、示例编译或运行结果；接口或构建变动时更新本架构基线。
