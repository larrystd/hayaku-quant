# 第 3 步：最终架构冻结与旧体系退役

> 状态：已完成（旧体系删除、物理目录迁移、全量功能验收完成）
>
> 前置阶段：[第 2 步：三引擎窄门面与兼容适配](step-2-progress.md)
>
> 决策日期：2026-09-25
>
> 开始执行：2026-09-25 18:39 CST
>
> 首轮并行合流完成：2026-09-25 18:58 CST（19 分钟）
>
> 第二轮收口与全量验收：2026-09-25 19:12 CST（累计 33 分钟）
>
> 兼容壳硬删除批次完成：2026-09-25 19:30 CST（累计 51 分钟）
>
> 最终目录迁移与验收完成：2026-09-25 20:30 CST

## 1. 先确定最终结果

第 2 步建立了 `DataEngine`、`ExecutionEngine` 和 `StrategyEngine`，但
`StockManager`、`TradeManager`、`System` 仍是实际或兼容后端。第 3 步不是再增加一层门面，
而是把真实实现迁入三个 Engine，并在完成时删除旧体系、兼容转发和旧目录。

本阶段接受破坏性 API 变更。兼容代码可以在中间提交中临时存在，**不得作为第 3 步的最终产物**。

### 1.1 最终运行结构

```text
┌─────────────────────────────────────────────────────────────────────┐
│ Python 稳定模块                                                     │
│ hayaku.data │ hayaku.execution │ hayaku.strategy │ hayaku.analysis │
└───────────────────────────────┬─────────────────────────────────────┘
                                ▼
┌─────────────────────────────────────────────────────────────────────┐
│ pybind11：按业务模块绑定稳定 API 和明确的扩展接口                   │
└───────────────────────────────┬─────────────────────────────────────┘
                                ▼
┌─────────────────────────────────────────────────────────────────────┐
│ HayakuSession                                                       │
│ 配置、生命周期、Data/Execution/Strategy Engine 所有权              │
└──────────────┬────────────────────┬──────────────────────┬───────────┘
               ▼                    ▼                      ▼
     ┌────────────────┐   ┌──────────────────┐   ┌──────────────────┐
     │ DataEngine     │   │ ExecutionEngine  │   │ StrategyEngine   │
     │ 只读市场数据   │◄──┼──────────────────┼───│ 策略编排         │
     └───────┬────────┘   │ 订单和账户       │◄──┤                  │
             │            └────────┬─────────┘   └────────┬─────────┘
             └─────────────────────┬──────────────────────┘
                                   ▼
     ┌──────────────────────────────────────────────────────────────┐
     │ common：时间、配置、日志、序列化、线程、网络等基础设施      │
     │ 不包含行情、账户、订单、策略等任何业务类型                  │
     └──────────────────────────────────────────────────────────────┘
```

依赖规则只有以下几条：

```text
StrategyEngine  ──► DataEngine
StrategyEngine  ──► ExecutionEngine
Analysis        ──► Execution/Strategy 的只读结果
app/data/execution/strategy/analysis ──► common
common          ──► 不依赖任何业务模块
```

Engine 之间不得通过 `StockManager`、`TradeManagerPtr`、`SystemPtr` 或全局单例通信。
Driver、Broker、TradeCost 和策略扩展接口跟随所属业务模块，不建立全局 `spi` 或 `adapters` 目录。

### 1.2 三个 Engine 和 Session 的最终职责

| 类型 | 最终职责 | 明确不负责 |
| --- | --- | --- |
| `HayakuSession` | 解析配置、创建并持有三个 Engine、统一关闭 | 数据查询、记账、逐 Bar 策略逻辑 |
| `DataEngine` | 证券、行情、板块、日历、财务等只读查询 | Driver 管理、插件控制、线程/IPC 运维、可变全局状态 |
| `ExecutionEngine` | 账户创建、订单校验与执行、唯一账本写入、账户快照 | 信号判断、行情查询、策略组件编排 |
| `StrategyEngine` | 策略定义执行、逐 Bar 调度、延迟订单、停止和结果快照 | 直接改账本、直接调 Broker/Driver、暴露 Portfolio 内部钩子 |

最终稳定接口的形态如下，具体字段可在编码前用编译测试进一步固定，但旧类型不得重新进入签名：

```cpp
class HayakuSession {
public:
    static HayakuSession open(const SessionOptions&);
    DataEngine& data();
    ExecutionEngine& execution();
    StrategyEngine& strategy();
    void close() noexcept;
};

class ExecutionEngine {
public:
    AccountId createAccount(const AccountConfig&);
    ExecutionReport submit(AccountId, const OrderRequest&);
    AccountSnapshot snapshot(AccountId) const;
    TradeRecordList history(AccountId) const;
};

class StrategyEngine {
public:
    BacktestResult run(const BacktestRequest&);
    void stop() noexcept;
    bool running() const noexcept;
};
```

其中：

- `AccountConfig` 明确初始资金、费用策略、Broker 和融资融券能力；
- `OrderRequest` 使用执行域自己的 `OrderOrigin`，不再依赖 `SystemPart`；
- `StrategyDefinition` 是不可变的组件组合，不包含账户或 `TradeManager`；
- `BacktestRequest` 包含 `StrategyDefinition`、证券/查询和 `AccountId`；
- `BacktestResult` 至少包含运行状态、成交快照和最终 `AccountSnapshot`；
- Strategy 内部所需的历史账户视图、重置和资金划转属于 internal port，不扩进普通公共 API。

### 1.3 最终目录结构

最终目录按业务能力组织。顶层目录必须能用一句话说明职责，不建立全局
`domain/components/spi/adapters/support` 等横向技术分层。基础设施统一放入 `common`：

```text
hayaku_cpp/hayaku/      ──► hayaku_cpp/src/
hayaku_cpp/unit_test/   ──► hayaku_cpp/test/
```

`src` 只放生产代码，`test` 只放测试代码；`test` 内部目录与 `src` 的模块结构保持镜像。

```text
hayaku_cpp/src/
├── app/
│   ├── HayakuSession.*
│   └── SessionOptions.*
├── data/                       # 获取、查询和计算市场数据
│   ├── DataEngine.*
│   ├── Stock.*
│   ├── KData.*
│   ├── KQuery.*
│   ├── Block.*
│   ├── indicator/              # K 线序列转换为指标序列
│   ├── factor/                 # 多证券数据计算因子结果
│   ├── driver/                 # 数据扩展接口及 HDF5/MySQL/SQLite/TDX 实现
│   └── internal/
│       ├── DataRuntime.*
│       └── DataCache.*
├── execution/                  # 执行订单并维护账户状态
│   ├── ExecutionEngine.*
│   ├── AccountConfig.*
│   ├── AccountSnapshot.*
│   ├── OrderRequest.*
│   ├── ExecutionReport.*
│   ├── TradeRecord.*
│   ├── PositionRecord.*
│   ├── broker/                 # Broker 扩展接口及实盘适配器
│   ├── cost/                   # 交易费用接口及实现
│   └── internal/
│       ├── ExecutionRuntime.*
│       ├── Ledger.*
│       ├── OrderValidator.*
│       └── AccountTransferService.*
├── strategy/                   # 产生并编排交易决策
│   ├── StrategyEngine.*
│   ├── StrategyDefinition.*
│   ├── BacktestRequest.*
│   ├── BacktestResult.*
│   ├── signal/
│   ├── risk/                   # 仓位、止损、止盈和目标价
│   ├── portfolio/
│   ├── selector/
│   └── internal/
│       ├── StrategyRuntime.*
│       ├── ComponentContext.*
│       ├── PendingOrderState.*
│       └── WalkForwardRuntime.*
├── analysis/                   # 只读分析交易和回测结果
│   ├── Performance.*
│   └── Report.*
└── common/                     # 与量化业务无关的基础设施
    ├── datetime/
    ├── config/
    ├── logging/
    ├── serialization/
    ├── threading/
    └── networking/

hayaku_cpp/test/
├── app/
├── data/
├── execution/
├── strategy/
├── analysis/
└── common/
```

绑定层和 Python 层按相同模块镜像：

```text
hayaku_pywrap/                  hayaku/
├── app/                       ├── session.py
├── data/                      ├── data/
├── execution/                 ├── execution/
├── strategy/                  ├── strategy/
├── analysis/                  ├── analysis/
├── common/                    ├── common/
└── advanced/                  ├── advanced/
                               └── interactive/
```

物理移动只在职责迁移和金标通过后进行；最终不能留下 `compat/`、转发头或旧 Python 包。

`common` 的准入规则：代码必须与证券、行情、账户、订单和策略概念无关，并且至少被两个业务模块
复用。数据库 Driver、Broker、TradeCost、指标和策略组件都不属于基础设施，必须留在所属业务模块。

### 1.4 最终删除清单

以下对象在第 3 步完成时必须删除，而不是只标记 deprecated：

| 旧对象 | 最终替代 | 最终删除范围 |
| --- | --- | --- |
| `StockManager`、全局 `sm`、`hayaku_init` | `HayakuSession` + `DataEngine` | `StockManager.h/.cpp`、绑定、顶层 Python 名称和兼容测试 |
| `TradeManagerBase`、`TradeManager`、`TradeManagerPtr/TMPtr`、`crtTM` | `ExecutionRuntime` + `Ledger` + `AccountConfig/AccountId` | 继承体系、工厂、pybind trampoline、Python `trade_manage` |
| `TradeManagerExecutionAdapter` | `ExecutionEngine` 直接持有 `ExecutionRuntime` | `trade/internal/TradeManagerExecutionAdapter.*` |
| `System`、`SystemPtr`、`SYS_Simple`、`SYS_WalkForward` | `StrategyDefinition` + `StrategyRuntime` | `trade_sys/system/**`、System 绑定、旧工厂和 Python `trade_sys` 入口 |
| `ExecutionEngine(TradeManagerPtr)`、`bindExecution` | Session 创建账户/ExecutionEngine | C++、pybind11、Python 全部删除 |
| `StrategyEngine(SystemPtr)`、`bindStrategy` | Session 固有 StrategyEngine + `BacktestRequest` | C++、pybind11、Python 全部删除 |
| Python 顶层通配导出 | 约 30 个稳定高频符号 | `import *` 聚合、790 项旧白名单和隐式导出 |

删除旧目录不等于删除仍有价值的能力。以下内容先迁入新目录再删除旧目录：

- `TradeRecord`、`CostRecord`、`PositionRecord` 等值类型迁入 `execution`；
- 费用模型迁入 `execution/cost`；
- `OrderBrokerBase` 的能力收窄后迁入 `execution/broker`；
- EV/CN/SG/MM/ST/TP/PG/SP/SE/AF/MF 等策略组件按职责迁入 `strategy`；
- Portfolio、WalkForward 保留能力，但只通过 internal coordinator 使用；
- 旧序列化数据若需要读取，使用一次性离线转换工具，不在生产库保留旧运行时类。

### 1.5 Python 最终公共面

`import hayaku` 只提供版本、异常、Session、三个 Engine 和少量高频领域值类型；目标为
`len(hayaku.__all__) <= 30`。指标、策略工厂、扩展协议和运维能力使用显式模块导入。

```text
hayaku                  Session、Engine、Datetime、Stock、KData、Query 等高频类型
hayaku.data             数据查询和值类型
hayaku.execution        Order/Execution/Account API
hayaku.strategy         StrategyDefinition/Backtest API 和组件工厂
hayaku.analysis         Performance 和回测结果分析
hayaku.common           Datetime、配置和其他必要基础类型
hayaku.advanced         IPC、数据服务、导入和运维控制
hayaku.interactive      允许研究场景使用的宽导入集合
```

最终还必须满足：

- `import hayaku` 不访问网络、不初始化 Hub、不加载绘图库；
- `dir(hayaku)` 不出现 Driver、Broker、Importer、IPC 和旧大类；
- 不存在 `hayaku.trade_manage`、`hayaku.trade_sys` 和 `hayaku.compat`；
- pybind11 不绑定 internal 方法、可变账本接口或 Portfolio 调度钩子；
- 发布时重新生成 `.pyi`，不保留旧类占位声明。

## 2. 当前与最终目标的差距

截至第 2 步完成后的静态盘点：

| 项目 | 当前 | 最终 |
| --- | --- | --- |
| 数据实现 | `DataRuntime` 已承接实现，`StockManager` 仍转发 | 仓内调用全部使用 DataEngine，删除 StockManager |
| 执行实现 | Engine → Adapter → TradeManager | Engine → ExecutionRuntime → Ledger |
| 策略实现 | Engine → `System::run()` → TradeManager | StrategyRuntime → ExecutionEngine |
| Session 装配 | `bindExecution(TradeManagerPtr)`、`bindStrategy(SystemPtr)` | Session 原生拥有三个 Engine，无旧类型输入 |
| Python 顶层 | `__all__` 790 项 | 不超过 30 项 |
| 旧符号扫描 | 约 1,423 个匹配行、220 个生产文件 | 禁止旧符号为 0 |
| 目录 | `hayaku_cpp/hayaku`、`unit_test` 及历史业务目录 | `hayaku_cpp/src`、`test`，内部为 app/data/execution/strategy/analysis/common |

`System.h/.cpp` 仍包含约 2300 行状态机，`TradeManagerBase` 仍有约 75 个公开方法，
`System` 仍有约 53 个公开方法。继续只包门面不会缩小内部耦合，必须迁移所有权和调用方。

当前进度：

| 编号 | 交付项 | 状态 |
| --- | --- | --- |
| 3.0 | 冻结最终架构、删除清单、并行边界和验收标准 | 已完成 |
| 3.1 | 用编译测试冻结四个内部契约和迁移金标 | 已完成 |
| 3.2 | DataEngine 调用迁移并删除 StockManager | 已完成 |
| 3.3 | ExecutionRuntime/Ledger 迁移并删除 TradeManager | 已完成 |
| 3.4 | StrategyRuntime 迁移并删除 System | 已完成 |
| 3.5 | Python/pybind11 收口并删除旧包 | 已完成 |
| 3.6 | 物理目录迁移和构建脚本整理 | 已完成 |
| 3.7 | 全量功能、性能、内存和 API 验收 | 已完成 |

### 2.1 2026-09-25 两轮执行结果

本轮完成的是实现所有权迁移和边界收口，不把仍存在的兼容壳误报为最终完成：

| 工作线 | 执行前 | 当前结果 | 本轮验收 |
| --- | --- | --- | --- |
| Data | `StockManager` 同时保存状态并被生产代码直接调用 | 状态和实现迁入 `DataRuntime`；除兼容壳自身外，C++ 生产代码的 `StockManager` include/实例调用为 0 | Core 编译、small-test、unit-test 通过 |
| Execution | `ExecutionEngine -> Adapter -> TradeManager`，账户字段散落在旧实现 | `ExecutionEngine -> ExecutionRuntime -> Ledger`；Adapter 已删除；`TradeManager` 缩为 53/27 行兼容壳 | 新增 Runtime 直连、AccountId/Config/View/OrderOrigin 测试；unit-test 通过 |
| Strategy | `StrategyEngine -> System::run()`，逐 Bar 状态机和四组延迟订单位于 `System` | 普通策略由 `StrategyRuntime` 执行；延迟订单集中到 `PendingOrderState`；下单经 `StrategyExecutionPort` | WalkForward 的非 Runtime 账户回退缺陷已在全量测试中发现并修复；unit-test 通过 |
| Python | 顶层公开集合约 790 项且导入绘图相关模块 | `__all__` 固定为 30 项；删除顶层 `StockManager/TradeManager/System/hayaku_init/sm`；顶层不再导入 Hub/绘图；pandas 改为按需导入 | Python 3.10 共 57 项测试通过；隔离进程副作用测试和 import-test 通过 |

本轮首次全量 unit-test 暴露 3 个 WalkForward 回归：兼容构造错误地拒绝
`WalkForwardTradeManager`。修复 `StrategyExecutionPort` 的精确 legacy 回退后，结果为
`832/832` 用例、`209250/209250` 断言通过。第二轮进一步完成：

- `ExecutionEngine` 删除内部 `m_compatManager` 和 `_manager()`，直接持有 `ExecutionRuntime`；
- `HayakuSession::open` 原生接收 `AccountConfig`，Python 不再暴露旧 `bind_execution`；
- StockManager Python 绑定被删除，C++ 生产代码对 StockManager 的直接引用降为 0；
- Strategy、Portfolio、Selector、AllocateFunds 和 WalkForward 的正常运行路径改经
  `StrategyRuntime`/`StrategyExecutionPort`，保留的 legacy 回退仅服务尚未迁移的旧协议；
- 顶层 Python API 固定为 30 项，导入副作用使用独立 Python 进程验证。

已验证命令：

```text
./op.sh configure   PASS
./op.sh build       PASS
./op.sh small-test  PASS：41/41，3288/3288
./op.sh unit-test   PASS：832/832，209250/209250
./op.sh python-test PASS：57/57（Python 3.10.21）
./op.sh import-test PASS
git diff --check    PASS
```

当前生产代码残留旧符号的文件数（不是匹配行数）为：

```text
StockManager 17        TradeManagerBase 18   TradeManagerPtr 39
TMPtr 16               SystemPtr 35          hayaku_init 14
TradeManagerExecutionAdapter 0
```

其中 StockManager 的 17 个文件是兼容类、绑定清单或其他旧入口；排除
`StockManager.h/.cpp` 后，C++ 生产调用方的直接 include/实例访问已经为 0。

第二轮结束时仍不是 Step 3 完成态，当时的剩余关键路径如下；其中前 3 项已在
2.2 的硬删除批次中完成或改为新签名：

1. 将 `StrategyDefinition` 从 `SystemPtr` 解耦，并迁移 Portfolio/WalkForward/实时 Strategy
   和 Python 自定义交易账户的宽 `TMPtr` 协议；
2. 删除 `TradeManagerBase/TradeManager`、`System` 和 `StockManager` 兼容壳及剩余绑定；
3. 删除 C++ `ExecutionEngine(TradeManagerBase)`、`bindExecution/bindStrategy` 等兼容签名；
4. 迁移 GUI、示例和文档后，删除 `trade_manage/trade_sys` Python 包及旧测试入口；
5. 最后执行 `hayaku_cpp/hayaku -> src`、`unit_test -> test` 的物理迁移，并完成性能、
   峰值内存和 `.pyi` 验收。

### 2.2 兼容壳硬删除批次结果

本批继续执行了可独立完成的硬删除，没有等待最终目录搬迁：

- 物理删除 `StockManager.h/.cpp` 和 `_StockManager.cpp`；生产代码、263 个 C++ 测试文件、
  GUI、示例和绘图调用全部迁到 `DataRuntime`、`DataEngine` 或 `HayakuSession`；
- 删除 C++/Python `hayaku_init`；实时 `Strategy` 改为显式持有自己的 `HayakuSession`；
- 删除公开 `ExecutionEngine(TradeManagerPtr)` 和 `HayakuSession::bindExecution`，执行公共头、
  pybind 和测试不再依赖 `TradeManagerBase/TradeManagerPtr/TMPtr`；
- `StrategyDefinition` 不再持有 `SystemPtr`，删除 `StrategyEngine(SystemPtr)`，Session 改为
  `bindStrategy(const StrategyDefinition&)` 并注入同账户 `ExecutionEngine`；
- 新 Engine、Session 和相应 pybind 公共入口中的 `SystemPtr`、旧 Execution 构造均为 0。

本批全量验证结果：

```text
./op.sh build       PASS
./op.sh small-test  PASS：41/41，3288/3288
./op.sh unit-test   PASS：830/830，209243/209243
./op.sh python-test PASS：57/57（Python 3.10.21）
./op.sh import-test PASS
git diff --check    PASS
```

unit-test 从上一批的 832 项变为 830 项，是因为删除了两个只验证旧 Engine/System
兼容构造的测试；业务断言基线仍为 209243 项并全部通过。长耗时自定义组件的协作取消覆盖，
需要后续以独立可取消组件协议补回，不能继续依靠派生 `System` 的测试钩子。

当前剩余旧符号所在生产文件数为：

```text
TradeManagerBase 15    TradeManagerPtr 44    TMPtr 16
SystemPtr 26           hayaku_init 0         TradeManagerExecutionAdapter 0
```

`StockManager` 和 `hayaku_init` 的可执行引用已经为 0；仅 Python API 禁止名单和“旧名称
不可见”测试中保留字符串字面量。下一批不能再完全并行：必须先把
StrategyDefinition/组件对 TM 的依赖替换为
`AccountId + AccountView + StrategyExecutionPort`，再解除 `ExecutionRuntime : TradeManagerBase`，
最后才能删除 TradeManager/System 旧绑定与 Python 旧包。

## 3. 改进方向与实施顺序

### 3.1 先冻结四个内部契约

以下契约由集成线先定稿，其他工作线只能消费，不能各自定义一套：

1. `AccountConfig/AccountId/AccountView`：账户创建、标识和只读查询；
2. `ExecutionCommand/ExecutionResult`：唯一账本写入协议；
3. `StrategyDefinition/BacktestRequest`：不可变策略定义和运行输入；
4. `ComponentContext/PendingOrder`：策略组件上下文和延迟订单内部状态。

同时冻结旧金标和序列化样本。只迁移行为，不顺便修正
`Strategy.cpp` 数量归一化、负数卖出以及 `open_spend_time` 等已知独立缺陷。

### 3.2 数据侧：删除 StockManager

目标调用链：

```text
Session ──► DataEngine ──► DataRuntime ──► data/driver
```

主要工作：

- 将 `indicator/**`、`factor/**`、`analysis/**`、`global/**`、`plugin/**` 中的
  `StockManager::instance()` 改为显式 DataEngine/数据上下文；
- 将 Driver 注册和数据写操作移入 `data/driver` 或 `advanced`，插件路径移入 Session 配置；
- 通用网络、IPC 和共享内存设施放入 `common/networking`，数据协议实现仍归 `data/driver`；
- 删除 `hayaku_cpp/hayaku/StockManager.*`、`export_StockManager`、Python `sm` 和 `hayaku_init`；
- `DataRuntime` 保持 internal，不进入 pybind11。

暂不改动行情算法、数据库 Schema、具体 HDF5/MySQL/SQLite/TDX 实现。

### 3.3 执行侧：把账本迁入 ExecutionEngine

目标调用链：

```text
ExecutionEngine
      └── ExecutionRuntime
            ├── OrderValidator
            ├── Ledger：唯一账户写入点
            ├── BrokerRouter
            ├── TradeCost
            └── CorporateActionProcessor
```

主要工作：

- 从 `trade_manage/TradeManager.*` 提取现金、持仓、负债和成交历史到 `Ledger`；
- `ExecutionEngine` 直接持有 Runtime，移除 Adapter 和 `TradeManagerPtr`；
- 策略组件只读取 `AccountView`，不得获得账本写权限；
- Portfolio/AllocateFunds 的资金划转使用 internal `AccountTransferService`，不把
  `checkin/checkout/addTradeRecord` 重新暴露为公共接口；
- 将 Broker、费用、报告和领域记录迁入各自的新目录，最后删除 `trade_manage/**`。

暂不虚构尚不存在的异步撤单协议；Broker SPI 扩展必须由真实实盘需求和专项测试驱动。

### 3.4 策略侧：把状态机迁入 StrategyEngine

目标调用链：

```text
StrategyDefinition
        ▼
StrategyRuntime ──► DataEngine
        │
        └─────────► ExecutionEngine.submit()
```

主要工作：

- 将 `System.h/.cpp` 的初始化、逐 Bar 循环、组件状态和结果收集迁入 `StrategyRuntime`；
- 将 `TradeRequest` 和四类延迟请求迁入 internal `PendingOrderState`；
- 使用 `ComponentContext` 替代组件的 `setTM/setSG/setTO/setQuery` 反向注入；
- 将 Portfolio/AllocateFunds、WalkForward/Selector 和实时 Strategy 分别接入窄 internal coordinator；
- 所有订单统一转为 `OrderRequest`，只通过 ExecutionEngine 执行；
- 最后删除 `System`、`DelegateSystem`、旧 WalkForward System 和 `trade_sys/system/**`。

暂不修改 EV/CN/SG/MM/ST/TP/PG/SP 等组件算法和回测语义。

### 3.5 暴露侧：删除旧 Python/pybind11 API

主要工作：

- pybind11 按 `app/data/execution/strategy/analysis/common/advanced` 重组注册文件；
- Python 顶层改为显式、轻量、无副作用导入；
- 示例、notebook、测试和双语文档全部改用 Session/Engine；
- 删除 StockManager/TradeManager/System 的 trampoline、pickle 和旧工厂绑定；
- 删除 `hayaku/trade_manage`、`hayaku/trade_sys` 及旧顶层属性；
- 最终重新生成 `.pyi` 并用 API inventory 固定公开面。

### 3.6 C++ 与性能约束

- 工程继续使用 C++20；新增实现只使用工具链稳定支持的 C++17/C++20 标准能力；
- 所有权优先使用值、`std::unique_ptr` 和稳定 ID，不用 `shared_ptr` 掩盖生命周期；
- 公共头只包含所属模块的值类型和必要扩展接口，Runtime/Ledger 使用 internal 头或 pImpl 隔离；
- 只有存在多个实现或明确插件边界时才新增虚接口；
- 逐 Bar 和逐订单热路径不新增锁、字符串协议、重复查表或无必要堆分配；
- 搬迁先保持行为，再以独立提交做算法修复或性能优化。

## 4. 并行执行方案

先完成 3.1 的契约冻结，再启动四条工作线：

```text
                    ┌── A：DataEngine 调用迁移 ───────────────┐
契约冻结 ───────────┼── B：ExecutionRuntime / Ledger ────────┤
                    ├── C：StrategyRuntime / ComponentContext ┤──► 合流
                    └── D：Python/pybind11 新目录和 API 测试 ─┘
                                                                  │
                                                                  ▼
                           Portfolio/WalkForward/实盘接入 ──► 旧代码删除
                                                                  │
                                                                  ▼
                                      物理目录迁移 ──► 全量验收
```

### 4.1 工作线边界

| 工作线 | 独占修改范围 | 可独立完成内容 | 合流依赖 |
| --- | --- | --- | --- |
| A 数据 | `data/**`、`data_driver/**`、数据调用方中的限定文件 | DataEngine 显式依赖、Driver 收入 data 模块、StockManager 调用清零 | C 负责其目录内的数据调用；最终由集成线删除门面 |
| B 执行 | `trade/**`、`trade_manage/**`、执行测试 | Ledger、ExecutionRuntime、Broker/Cost SPI、执行金标 | 向 C 提供冻结的内部执行端口 |
| C 策略 | `trade_sys/**`、`strategy/**`、策略测试 | StrategyRuntime、组件上下文、延迟订单、状态机原样迁移 | 接入 B 的执行端口后才能删除 System/TM |
| D 暴露 | `hayaku_pywrap/**`、`hayaku/**`、Python 测试和 API 文档 | 新目录、显式导入、边界测试、stub 准备 | C++ 新接口稳定后切换绑定和删除旧包 |

共享文件只由集成线修改：

```text
hayaku_cpp/hayaku/hayaku.h → hayaku_cpp/src/hayaku.h
hayaku_cpp/hayaku/application/HayakuSession.* → hayaku_cpp/src/app/**
hayaku_cpp/hayaku/xmake.lua → hayaku_cpp/src/xmake.lua
hayaku_cpp/unit_test/xmake.lua → hayaku_cpp/test/xmake.lua
hayaku_pywrap/main.cpp
hayaku_pywrap/xmake.lua
hayaku/__init__.py
hayaku/test/test.py
docs/arch/api-inventory.md
```

并行工作使用独立 worktree 和独立 build 目录；不能共享会写入测试数据或序列化文件的构建目录。

### 4.2 合流顺序

1. 合入 B 的 ExecutionRuntime/Ledger 和只读账户视图；
2. 合入 C 的 StrategyRuntime，并切到 ExecutionEngine；
3. 合入 A，删除最后的 StockManager 内部调用；
4. 合入 Portfolio、WalkForward、实时 Strategy 和插件迁移；
5. 合入 D 的最终绑定和 Python API；
6. 删除三个旧体系及兼容代码；
7. 最后做物理目录移动和 include/build 脚本整理。

目录移动放在行为迁移之后，避免 `git mv` 与逻辑修改制造大面积冲突。每个中间提交必须可编译，
但只有旧实现、旧目录和兼容入口全部删除后才算 Step 3 完成。

## 5. 暂时不动的范围

为控制风险，本阶段不修改：

- Indicator、Factor 和策略组件的计算算法；
- HDF5/MySQL/SQLite/TDX 的存储格式和数据库 Schema；
- 成交价格、费用、复权、分红配股和融资融券规则；
- Boost 序列化现有文件的内容语义；需要旧文件迁移时提供离线转换；
- GUI、绘图和数据抓取业务逻辑，只迁移它们对公共入口的调用；
- 已记录的独立缺陷，另开 bugfix 提交处理。

## 6. 最终验收标准

### 6.1 静态结构

- 生产代码中 `StockManager`、`TradeManagerBase`、`TradeManagerPtr/TMPtr`、`SystemPtr`、
  `TradeManagerExecutionAdapter`、`hayaku_init` 和全局 `sm` 引用为 0；
- 生产代码全部位于 `hayaku_cpp/src`，测试全部位于 `hayaku_cpp/test`；
- 不再存在 `hayaku_cpp/hayaku` 和 `hayaku_cpp/unit_test`；
- 删除 `StockManager.*`、`trade_manage/**`、`trade_sys/system/**` 及对应绑定/Python 包；
- `ExecutionEngine.h` 不包含任何旧 TradeManager 头；
- `StrategyEngine.h` 不包含 `System.h`，运行期不调用 `System::run()`；
- 只有 `Ledger` 可以修改现金、持仓、负债和成交历史；
- StrategyRuntime 不直接调用 Broker、Driver 或账本写接口；
- internal 类型不被 pybind11 绑定，聚合头不包含 internal 或 `common` 实现头。

### 6.2 API

- `len(hayaku.__all__) <= 30`；
- `import hayaku` 无网络、Hub、GUI、绘图和数据加载副作用；
- C++/Python 普通用户只通过 Session 和三个 Engine 完成数据查询、账户执行和策略运行；
- SPI、advanced 与稳定用户 API 分离，所有公开方法都有归属和测试；
- API inventory 中不存在兼容/deprecated 分组。

### 6.3 行为和性能

- small-test、unit-test、Python 3.10 测试和 import-test 全部通过；
- 简单 System、风控 System、Portfolio、WalkForward 和实时 Strategy 金标迁移后完全一致；
- 买、卖、卖空、平空、费用、资金、持仓、融资融券、Broker 回调和复权行为一致；
- Session 关闭、并发 stop、结果快照和异常路径通过生命周期测试；
- Release 模式代表性回测和逐笔执行中位耗时回退不超过 5%；
- 记录峰值内存，无未解释增长；
- `git diff --check`、clang-format、双语文档和 `.pyi` 校验通过。

## 7. 阶段完成定义

以下四项必须同时成立：

```text
真实实现已经进入三个 Engine
        +
仓库内部调用已迁移
        +
旧类、旧目录、旧绑定已经删除
        +
行为、性能和 API 数量通过验收
```

只完成新 Runtime、只把旧类改成转发、只收窄 Python `__all__`，都不能把第 3 步标记为完成。

## 8. 最终执行结果

### 8.1 旧体系已物理删除

- `StockManager`、`TradeManagerBase/TradeManager`、`crtTM`、`System`、
  `TradeManagerExecutionAdapter` 和 `StrategyConfig` 的生产实现、绑定及旧测试已删除；
- C++ 生产代码与测试中的 `TradeManagerPtr/TMPtr`、`SystemPtr/SYSPtr`、
  `SYS_Simple/SYS_WalkForward` 静态扫描为 0；
- Python 的 `hayaku.trade_manage`、`hayaku.trade_sys` 已删除；旧名称只保留在拒绝列表和
  负向边界测试中，用于保证它们不会重新进入公共 API；
- 顶层 `hayaku.__all__` 为 30 项，普通导入不初始化数据、不加载绘图或网络模块。
- 仓库当前没有 `.pyi` 产物或生成流程，因此不存在需要保留的旧类占位声明；
  公共面由 API inventory 和 Python 边界测试固定。

### 8.2 最终物理目录

```text
hayaku_cpp/
├── src/
│   ├── app/                    # Session、进程运行期与应用插件
│   ├── data/                   # 行情值类型、DataEngine、driver、factor、indicator
│   ├── execution/              # ExecutionEngine、Ledger、账户、Broker、费用
│   ├── strategy/               # StrategyEngine、Runtime、策略组件、组合与选择器
│   ├── analysis/               # 只读绩效和组合分析
│   └── common/                 # 配置、时间、日志、序列化、线程、网络、数据库基础设施
└── test/
    ├── app/
    ├── data/
    ├── execution/
    ├── strategy/
    ├── analysis/
    └── common/

hayaku_pywrap/
├── app/
├── data/
├── execution/
├── strategy/
├── analysis/
├── common/
└── advanced/
```

`hayaku_cpp/hayaku`、`hayaku_cpp/unit_test`、`trade_manage` 和
`trade_sys/system` 均已不存在。xmake、安装头文件复制逻辑和 API inventory 已切换到新路径。

### 8.3 最终验收

```text
./op.sh configure shared  PASS
./op.sh build             PASS
./op.sh small-test        PASS：44/44 cases，3300/3300 assertions
./op.sh unit-test         PASS：797/797 cases，208380/208380 assertions
./op.sh python-test       PASS：59/59（Python 3.10.21）
./op.sh import-test       PASS
git diff --check          PASS
```

测试数量下降来自删除只验证旧 TradeManager/System 协议的测试；对应 Engine、Runtime、
账户端口、策略组件和生命周期测试已保留或重写。small-test 新增了 DataEngine 生命周期用例，
因此从 41 项变为 44 项。

Release `unit-test` 在同机各运行 10 次：原工作树中位数 `2.901s`，重构后中位数
`2.846s`（约快 1.9%）；观测到的最大 RSS 分别为 `307,068,928` 与 `304,365,568`
字节（约降低 0.9%）。该数据用于确认没有明显回退，不替代独立微基准。
