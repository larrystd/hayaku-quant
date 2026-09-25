# 第 1 步进度：DataEngine 与三引擎边界

> 状态：待执行（当前执行阶段：DataEngine）
>
> 前置阶段：[第一阶段完成报告：API 清单与行为基线](step-0-progress.md)
>
> 本文是第 1 步 DataEngine 的执行与进度文档，同时固定后续 ExecutionEngine、StrategyEngine 和 Python API 阶段的边界。原总方案中的第 1～8 步作为技术参考，不再作为独立交付阶段。

## 1. 重构目标

把当前三个职责过大的入口收敛为三个名称直接、职责单一的 Engine：

| 当前入口 | 新入口 | 唯一核心职责 |
| --- | --- | --- |
| `StockManager` | `DataEngine` | 提供证券、行情、板块和财务数据 |
| `TradeManagerBase`、`OrderBrokerBase` | `ExecutionEngine` | 执行订单并维护账户账本 |
| `System` | `StrategyEngine` | 编排策略组件并产生订单请求 |

`HikyuuSession` 只负责三个 Engine 的创建、持有、访问和关闭，不承载业务逻辑。Python/pybind11 是最外层的接口适配层，不是第四个业务引擎。

目标结构：

```text
┌─────────────────────────────────────────────────────────────┐
│ HikyuuSession                                               │
│ 生命周期、配置、Engine 所有权                               │
└───────────────┬──────────────────┬──────────────────────────┘
                │                  │
                ▼                  ▼
┌──────────────────────┐   ┌──────────────────────┐
│ DataEngine           │   │ ExecutionEngine     │
│ 证券与行情数据       │   │ 订单、成交与账户     │
└──────────┬───────────┘   └──────────▲───────────┘
           │ MarketData               │ OrderRequest /
           │                          │ ExecutionReport
           ▼                          │
        ┌─────────────────────────────┴──┐
        │ StrategyEngine                 │
        │ 策略组件编排、逐 Bar 调度      │
        └────────────────────────────────┘
```

依赖规则：

```text
StrategyEngine ──────► DataEngine
       │
       └─────────────► ExecutionEngine

DataEngine       不依赖 StrategyEngine 或 ExecutionEngine
ExecutionEngine  不依赖 StrategyEngine
Python/pybind11  只能依赖三个 Engine 的公共门面和稳定领域类型
```

## 2. 当前进度

| 项目 | 当前值 |
| --- | --- |
| 当前阶段 | 第 1 步：DataEngine |
| 阶段状态 | 未开始 |
| 整体进度 | 0% |
| 工作分支 | `refactor/api-boundary` |
| 前置基线 | 第 0 步已完成并验收 |
| 当前阻塞 | 无 |
| 下一动作 | 固定 DataEngine 最小接口和性能基线 |

| 编号 | 工作项 | 状态 | 主要产物 |
| --- | --- | --- | --- |
| 1.1 | 固定接口、调用方和性能基线 | 未开始 | 接口决策、基准数据 |
| 1.2 | 建立 `SessionOptions`、`HikyuuSession` 和 `DataEngine` 骨架 | 未开始 | 可编译的最小门面 |
| 1.3 | 迁移初始化、关闭和加载生命周期 | 未开始 | Session 持有 DataEngine |
| 1.4 | 迁移数据查询和内部 DataRuntime | 未开始 | DataEngine 完整数据能力 |
| 1.5 | 将 `StockManager` 改为兼容转发门面 | 未开始 | 无新增业务逻辑的旧入口 |
| 1.6 | 接入 C++/Python 测试和最小绑定 | 未开始 | 生命周期、查询和兼容测试 |
| 1.7 | 执行全量回归、性能对比和阶段验收 | 未开始 | 前后对比与验收结果 |

进度更新规则：

- 开始实际代码修改时将阶段状态改为“进行中”；
- 每个工作项只有在代码、测试和证据齐全后才能标为“已完成”；
- 阶段完成时必须补充执行前后对比、逐项验收结果、实际耗时和性能数据；
- 未覆盖或推迟的内容必须明确列入遗留项，不能计入完成进度。

## 3. 核心设计原则

### 3.1 只有三个业务门面

普通 C++/Python 用户主要看见：

```text
DataEngine
ExecutionEngine
StrategyEngine
```

`InstrumentRepository`、`DriverRegistry`、`Ledger`、`OrderValidator`、`SystemEngine` 等是实现细节，默认放在 `internal` 命名空间或私有实现中，不因为拆分类就自动成为公共接口。

### 3.2 新旧接口并存迁移

每个阶段采用相同迁移方式：

```text
新 Engine 完成能力
       │
       ├── 新代码直接使用 Engine
       │
       └── 旧类保留兼容转发
                 │
                 └── 经过弃用期后再删除
```

一个提交内不同时进行“大规模移动文件、改变行为、删除旧接口”。行为修复必须有独立测试和独立提交。

### 3.3 每个阶段独立闭环

一个阶段只允许修改自己的实现、必要的兼容适配器、构建文件和测试：

| 阶段 | 允许修改的核心区域 | 禁止顺手修改 |
| --- | --- | --- |
| DataEngine | `StockManager`、数据驱动访问、Session | 交易、策略算法、System 状态机 |
| ExecutionEngine | TradeManager、Broker、账户、费用调用链 | 数据加载、策略组件算法、System 编排 |
| StrategyEngine | System、策略组件上下文和运行入口 | DataEngine/ExecutionEngine 内部实现 |
| Python API | Python 导出与 pybind11 绑定 | C++ 领域行为和算法 |

跨阶段依赖只能通过已经验收的公共接口或临时兼容适配器完成。

## 4. C++ 开发规范

### 4.1 语言标准

- 构建目标使用 C++20；
- 新代码只使用标准 C++17/C++20 语言和标准库能力，不依赖 GNU/Clang 私有扩展；
- 优先使用已经被项目工具链稳定支持的 C++20 能力；需要兼容性时使用等价的 C++17 写法；
- 不引入 C++23 API，例如 `std::expected`；需要时定义项目内明确的结果类型；
- 暂不引入 C++ Modules 和 Coroutines，除非单独验证构建兼容性和实际收益。

优先采用：

- `std::unique_ptr` 表达唯一所有权；
- `std::string_view`、`std::span` 表达非拥有参数；
- `std::optional`、强类型 `enum class` 表达可选值和状态；
- `[[nodiscard]]`、`explicit`、`const`、`noexcept` 明确接口契约；
- RAII 管理线程、连接、缓存和 Session 生命周期；
- concepts 仅用于确实需要约束的公共模板，不为普通类增加模板复杂度。

避免：

- 裸 `new/delete`；
- 无明确所有权的裸指针；
- 默认使用 `shared_ptr`；
- 通过字符串或 JSON 在 C++ 核心路径传递订单和账户状态；
- 在每根 K 线或每笔订单路径中反复分配对象；
- 为只有一个实现的类创建无意义虚接口；
- 为了使用新语法而使用新语法。

### 4.2 接口风格

- Engine 公共接口使用值对象、只读引用和明确结果类型；
- 普通拒单、无信号和数据缺失使用状态值，不使用异常控制高频流程；
- 配置错误、初始化失败等边界错误可以抛出项目既有异常；
- Engine 默认不可复制；只有语义明确时允许移动；
- 生命周期由 `HikyuuSession` 管理，不增加新的全局单例；
- Public API、Extension SPI、Internal API 必须在头文件和 inventory 中明确分类；
- 新增接口必须说明调用方，不能仅为“以后可能用到”而暴露。

### 4.3 性能约束

重构不得用明显的运行时成本换取目录整齐：

- `Stock`、`KData`、`Indicator` 和账户快照避免不必要的深拷贝；
- 逐 Bar 热路径不新增堆分配、字符串拼接、JSON 转换或锁竞争；
- `StrategyEngine` 在运行前完成组件引用和参数准备，循环内不重复查找组件；
- `ExecutionEngine` 使用强类型值对象传递订单和成交，不在核心路径使用字典式参数；
- `Ledger` 保持顺序追加，查询侧使用只读视图或批量结果；
- Python 长时间回测入口应在安全范围内释放 GIL；
- 抽象层默认使用组合和静态调用；只有 Driver、Broker、策略组件等真实扩展点保留虚调用；
- 性能优化必须以基准结果为依据，不进行不可验证的微优化。

每个阶段记录重构前后相同数据、相同参数下的：

1. 初始化耗时；
2. 代表性数据查询耗时；
3. 三组金标回测耗时；
4. 峰值内存；
5. 交易记录和最终资产一致性。

相同 Release 构建下，代表性回测中位耗时回退超过 5% 必须分析原因，超过 10% 不得验收，除非有明确、记录在案的功能收益和人工批准。

性能测试沿用仓库已有机制：使用 `hikyuu_cpp/unit_test/hikyuu/test_config.h` 的 `ENABLE_BENCHMARK_TEST` 开关、`BENCHMARK_TIME_MSG` 和 `SpendTimer`。正式对比使用 Release 构建，先预热，再至少运行 10 次并记录中位数；macOS 峰值内存使用 `/usr/bin/time -l` 记录。Debug 构建结果不得用于阶段性能验收。

## 5. 第一阶段：DataEngine

### 5.1 职责

`DataEngine` 负责：

- 证券、市场和证券类型查询；
- K 线、交易日历、复权权重和债券数据查询；
- 板块查询与明确的写入操作；
- 财务数据查询；
- 数据初始化、缓存和加载状态；
- 在内部管理 Driver、插件、预加载线程及 IPC/SHM。

它不负责订单、账户、策略组件或回测调度。

建议的最小公共接口：

```cpp
class DataEngine {
public:
    DataEngine(const DataEngine&) = delete;
    DataEngine& operator=(const DataEngine&) = delete;

    [[nodiscard]] bool ready() const noexcept;
    void waitReady() const;

    [[nodiscard]] Stock getStock(std::string_view code) const;
    [[nodiscard]] KData getKData(const Stock& stock, const KQuery& query) const;
    [[nodiscard]] DatetimeList getTradingCalendar(const KQuery& query,
                                                   std::string_view market) const;
};
```

完整数据能力可以继续通过成员方法或窄的结果对象提供，但 Driver、线程、IPC 和测试钩子不得进入普通 Public API。

### 5.2 文件范围

计划新增：

```text
hikyuu_cpp/hikyuu/application/HikyuuSession.h
hikyuu_cpp/hikyuu/application/HikyuuSession.cpp
hikyuu_cpp/hikyuu/application/SessionOptions.h
hikyuu_cpp/hikyuu/data/DataEngine.h
hikyuu_cpp/hikyuu/data/DataEngine.cpp
hikyuu_cpp/hikyuu/data/internal/DataEngineImpl.h
hikyuu_cpp/hikyuu/data/internal/DataEngineImpl.cpp
hikyuu_cpp/unit_test/hikyuu/application/test_HikyuuSession.cpp
hikyuu_cpp/unit_test/hikyuu/data/test_DataEngine.cpp
hikyuu_pywrap/application/_HikyuuSession.cpp
hikyuu_pywrap/data/_DataEngine.cpp
hikyuu/session.py
```

计划修改：

```text
hikyuu_cpp/hikyuu/hikyuu.h
hikyuu_cpp/hikyuu/hikyuu.cpp
hikyuu_cpp/hikyuu/StockManager.h
hikyuu_cpp/hikyuu/StockManager.cpp
hikyuu_cpp/hikyuu/data_driver/DataDriverFactory.h
hikyuu_cpp/hikyuu/data_driver/DataDriverFactory.cpp
hikyuu_cpp/hikyuu/xmake.lua
hikyuu_cpp/unit_test/xmake.lua
hikyuu_pywrap/main.cpp
hikyuu_pywrap/_StockManager.cpp
hikyuu_pywrap/xmake.lua
hikyuu/__init__.py
hikyuu/test/test.py
```

本阶段不修改：

```text
hikyuu_cpp/hikyuu/trade_manage/**
hikyuu_cpp/hikyuu/trade_sys/**
hikyuu_cpp/hikyuu/strategy/Strategy.*
hikyuu_cpp/hikyuu/indicator/**
hikyuu_cpp/hikyuu/serialization/**
```

### 5.3 实施顺序

1. 增加 `SessionOptions`、`HikyuuSession` 和 `DataEngine`，先通过现有 `StockManager` 实现兼容能力；
2. 把生命周期、查询和内部运行时职责从 `StockManager` 逐组迁入 `DataEngineImpl`；
3. 将仓库内部数据调用逐步切换到 `DataEngine`；
4. 将 `StockManager` 改为无新增业务逻辑的兼容转发门面；
5. 增加最小 Python Session/DataEngine 绑定，但不在此阶段全面整理 Python 命名空间；
6. 更新 API inventory、前后对比、性能数据和验收记录。

### 5.4 验收标准

- 新代码可以只通过 `HikyuuSession::data()` 完成常用数据查询；
- `StockManager` 不再负责 Engine 生命周期，也不再增加业务逻辑；
- Driver、插件、线程和 IPC 控制不出现在 `DataEngine` 普通公共接口；
- `import hikyuu` 不启动数据加载；
- 旧 `hikyuu_init()`、`StockManager::instance()` 和 Python `sm` 仍可工作；
- 数据查询结果与当前基线一致；
- 全量测试和性能门槛通过。

执行进度、前后对比和验收结果直接更新在本文：

```text
docs/arch/refactor/step-1-progress.md
```

## 6. 第二阶段：ExecutionEngine

### 6.1 职责

`ExecutionEngine` 负责：

- 接收和校验 `OrderRequest`；
- 回测撮合或调用实盘 Broker；
- 计算执行价格、滑点和费用；
- 产生 `ExecutionReport`；
- 通过内部 `Ledger` 更新现金、持仓、负债和交易记录；
- 提供只读 `AccountSnapshot` 和交易历史。

它不读取行情数据库，不判断买卖信号，不运行逐 Bar 策略状态机。

建议的最小公共接口：

```cpp
class ExecutionEngine {
public:
    ExecutionEngine(const ExecutionEngine&) = delete;
    ExecutionEngine& operator=(const ExecutionEngine&) = delete;

    [[nodiscard]] ExecutionReport submit(const OrderRequest& request);
    [[nodiscard]] bool cancel(const OrderId& order_id);
    [[nodiscard]] AccountSnapshot snapshot() const;
    [[nodiscard]] TradeRecordList history() const;
};
```

`Ledger` 是唯一可以改变账户状态的对象，但保持为 ExecutionEngine 内部能力。

### 6.2 文件范围

计划新增：

```text
hikyuu_cpp/hikyuu/trade/OrderRequest.h
hikyuu_cpp/hikyuu/trade/ExecutionReport.h
hikyuu_cpp/hikyuu/trade/AccountSnapshot.h
hikyuu_cpp/hikyuu/trade/ExecutionEngine.h
hikyuu_cpp/hikyuu/trade/ExecutionEngine.cpp
hikyuu_cpp/hikyuu/trade/internal/Ledger.h
hikyuu_cpp/hikyuu/trade/internal/Ledger.cpp
hikyuu_cpp/hikyuu/trade/internal/BacktestMatcher.h
hikyuu_cpp/hikyuu/trade/internal/BacktestMatcher.cpp
hikyuu_cpp/hikyuu/trade/BrokerAdapter.h
hikyuu_cpp/hikyuu/trade/BrokerAdapter.cpp
hikyuu_cpp/unit_test/hikyuu/trade/test_ExecutionEngine.cpp
hikyuu_pywrap/trade/_ExecutionEngine.cpp
```

计划修改：

```text
hikyuu_cpp/hikyuu/trade_manage/TradeManagerBase.h
hikyuu_cpp/hikyuu/trade_manage/TradeManagerBase.cpp
hikyuu_cpp/hikyuu/trade_manage/TradeManager.h
hikyuu_cpp/hikyuu/trade_manage/TradeManager.cpp
hikyuu_cpp/hikyuu/trade_manage/OrderBrokerBase.h
hikyuu_cpp/hikyuu/trade_manage/OrderBrokerBase.cpp
hikyuu_cpp/hikyuu/strategy/Strategy.h
hikyuu_cpp/hikyuu/strategy/Strategy.cpp
hikyuu_cpp/hikyuu/xmake.lua
hikyuu_cpp/unit_test/xmake.lua
hikyuu_pywrap/trade_manage/_TradeManager.cpp
hikyuu_pywrap/trade_manage/_OrderBroker.cpp
hikyuu_pywrap/xmake.lua
hikyuu/test/test.py
```

本阶段不修改：

```text
hikyuu_cpp/hikyuu/StockManager.*
hikyuu_cpp/hikyuu/data/**
hikyuu_cpp/hikyuu/data_driver/**
hikyuu_cpp/hikyuu/trade_sys/*/imp/**
hikyuu_cpp/hikyuu/indicator/**
```

`System` 只允许增加调用新 ExecutionEngine 的兼容适配点，不在本阶段重写逐 Bar 编排。

### 6.3 实施顺序

1. 补齐 `Strategy::order/orderValue`、拒单、最小交易单位、资金不足、手续费和滑点测试；
2. 定义不可变 `OrderRequest`、明确状态的 `ExecutionReport` 和只读 `AccountSnapshot`；
3. 实现回测撮合、Broker 适配和内部 Ledger；
4. 让旧 `TradeManagerBase` 方法转发到 ExecutionEngine；
5. 修复已登记的下单数量缺陷，使用独立 bugfix 提交；
6. 对比新旧路径的逐笔成交、现金、持仓和性能；
7. 更新 API inventory 和完成报告。

### 6.4 验收标准

- 只有 Ledger 能修改账户状态；
- `ExecutionEngine` 不依赖 `StrategyEngine` 或具体策略组件；
- 回测和实盘适配器返回相同结构的 `ExecutionReport`；
- `TradeManagerBase` 只承担兼容转发，不再增加职责；
- `Strategy::order/orderValue` 的数量和价格行为有完整测试；
- 三组金标的新旧路径结果一致；
- 全量测试和性能门槛通过。

完成记录写入：

```text
docs/arch/refactor/step-2-execution-engine-progress.md
```

## 7. 第三阶段：StrategyEngine

### 7.1 职责

`StrategyEngine` 是当前 `System` 的明确替代名称，负责：

- 持有不可随意变更的 `StrategyConfig`；
- 从 DataEngine 获取回测数据；
- 编排 EV、CN、SG、MM、ST、TP、PG、SP 等组件；
- 运行逐 Bar 状态机和延迟订单状态；
- 产生 `OrderRequest` 并交给 ExecutionEngine；
- 接收 `ExecutionReport`，形成回测结果。

它不直接访问 Driver、Broker 或写入账户。

建议的最小公共接口：

```cpp
class StrategyEngine {
public:
    StrategyEngine(const StrategyEngine&) = delete;
    StrategyEngine& operator=(const StrategyEngine&) = delete;

    [[nodiscard]] BacktestResult run(const StrategyConfig& config);
    void stop() noexcept;
    [[nodiscard]] bool running() const noexcept;
};
```

逐 Bar、延迟订单、Portfolio/AllocateFunds 协作方法属于内部实现，不进入普通公共接口。

### 7.2 文件范围

计划新增：

```text
hikyuu_cpp/hikyuu/trade_sys/engine/StrategyEngine.h
hikyuu_cpp/hikyuu/trade_sys/engine/StrategyEngine.cpp
hikyuu_cpp/hikyuu/trade_sys/engine/StrategyConfig.h
hikyuu_cpp/hikyuu/trade_sys/engine/BacktestResult.h
hikyuu_cpp/hikyuu/trade_sys/engine/internal/StrategyRuntime.h
hikyuu_cpp/hikyuu/trade_sys/engine/internal/StrategyRuntime.cpp
hikyuu_cpp/hikyuu/trade_sys/component/ComponentContext.h
hikyuu_cpp/unit_test/hikyuu/trade_sys/engine/test_StrategyEngine.cpp
hikyuu_pywrap/trade_sys/_StrategyEngine.cpp
```

计划修改：

```text
hikyuu_cpp/hikyuu/trade_sys/system/System.h
hikyuu_cpp/hikyuu/trade_sys/system/System.cpp
hikyuu_cpp/hikyuu/trade_sys/system/TradeRequest.h
hikyuu_cpp/hikyuu/trade_sys/system/TradeRequest.cpp
hikyuu_cpp/hikyuu/trade_sys/environment/EnvironmentBase.h
hikyuu_cpp/hikyuu/trade_sys/condition/ConditionBase.h
hikyuu_cpp/hikyuu/trade_sys/signal/SignalBase.h
hikyuu_cpp/hikyuu/trade_sys/moneymanager/MoneyManagerBase.h
hikyuu_cpp/hikyuu/trade_sys/stoploss/StoplossBase.h
hikyuu_cpp/hikyuu/trade_sys/profitgoal/ProfitGoalBase.h
hikyuu_cpp/hikyuu/trade_sys/slippage/SlippageBase.h
hikyuu_cpp/hikyuu/xmake.lua
hikyuu_cpp/unit_test/xmake.lua
hikyuu_pywrap/trade_sys/_System.cpp
hikyuu_pywrap/xmake.lua
hikyuu/test/test.py
```

本阶段不修改：

```text
hikyuu_cpp/hikyuu/data/**
hikyuu_cpp/hikyuu/data_driver/**
hikyuu_cpp/hikyuu/trade/internal/**
hikyuu_cpp/hikyuu/trade_sys/*/imp/**
hikyuu_cpp/hikyuu/indicator/**
hikyuu_cpp/hikyuu/serialization/**
```

### 7.3 实施顺序

1. 定义 `StrategyConfig`、`BacktestResult` 和只读 `ComponentContext`；
2. 把逐 Bar 和延迟订单状态机迁入内部 `StrategyRuntime`；
3. 使用 DataEngine 获取数据，使用 ExecutionEngine 提交订单；
4. 将现有 `System` 改为 StrategyEngine 的兼容门面；
5. 将 `_calculate/_reset/_clone` 等扩展钩子和普通用户接口分级，但暂不全面移动 Python 命名空间；
6. 对所有内置组件执行金标回归和性能比较；
7. 更新 API inventory 和完成报告。

### 7.4 验收标准

- 普通用户只需要配置并调用 `StrategyEngine::run()`；
- StrategyEngine 不直接访问数据库 Driver 或 Broker 实现；
- 只有 ExecutionEngine 能提交执行并修改账户；
- `runMoment*`、`pfProcessDelay*`、强制卖出和延迟状态接口不在普通公共门面；
- 旧 `System` 和 `SYS_*` 工厂在兼容期内继续工作；
- 所有现有内置策略组件的金标结果一致；
- 全量测试和性能门槛通过。

完成记录写入：

```text
docs/arch/refactor/step-3-strategy-engine-progress.md
```

## 8. 第四阶段：Python/pybind11 接口收口

### 8.1 职责

这一阶段只决定用户能看见什么，不改变三个 Engine 的 C++ 行为。

目标命名空间：

```text
hikyuu                 高频稳定类型、工厂和 open_session
├── data               DataEngine 及数据查询辅助
├── strategy           StrategyEngine、配置和回测结果
├── execution          ExecutionEngine、订单和账户快照
├── advanced           临时证券、低层控制和低频工具
└── spi                Driver、Broker 和策略组件扩展协议
```

### 8.2 文件范围

计划修改或新增：

```text
hikyuu/__init__.py
hikyuu/core.py
hikyuu/extend.py
hikyuu/session.py
hikyuu/data/__init__.py
hikyuu/strategy/__init__.py
hikyuu/execution/__init__.py
hikyuu/advanced/__init__.py
hikyuu/spi/__init__.py
hikyuu/indicator/__init__.py
hikyuu/trade_manage/__init__.py
hikyuu/trade_sys/__init__.py
hikyuu_pywrap/main.cpp
hikyuu_pywrap/application/**
hikyuu_pywrap/data/**
hikyuu_pywrap/trade/**
hikyuu_pywrap/trade_manage/**
hikyuu_pywrap/trade_sys/**
hikyuu/test/test_public_api.py
hikyuu/test/test.py
```

本阶段不修改：

```text
hikyuu_cpp/hikyuu/data/** 的业务实现
hikyuu_cpp/hikyuu/trade/** 的业务实现
hikyuu_cpp/hikyuu/trade_sys/engine/** 的业务实现
hikyuu_cpp/hikyuu/trade_sys/*/imp/**
hikyuu_cpp/hikyuu/indicator/**
```

### 8.3 实施顺序

1. 明确顶层稳定白名单并建立 `__all__`；
2. 显式导入替代层层 `import *`；
3. 将扩展协议移入 `hikyuu.spi`；
4. 将低层控制移入 `hikyuu.advanced`；
5. 旧路径保留兼容转发并发出 `DeprecationWarning`；
6. 修复 `open_spend_time` 的错误绑定；
7. 更新文档示例，执行 notebook/示例冒烟测试；
8. 更新 API inventory 和完成报告。

### 8.4 验收标准

- `dir(hikyuu)` 和 `hikyuu.__all__` 只包含明确批准的稳定接口；
- 普通用户类不出现 Driver、线程、IPC 和 `_calculate/_reset/_clone` 等内部入口；
- `DataEngine`、`ExecutionEngine`、`StrategyEngine` 从规定路径稳定导入；
- 所有迁移符号都有新路径、兼容转发和弃用警告；
- Python 3.10 测试、示例和 import 测试通过；
- 不因包装层增加大对象复制或阻塞 GIL 的回测开销。

完成记录写入：

```text
docs/arch/refactor/step-4-python-api-progress.md
```

## 9. 兼容与清理策略

每个旧入口只有在同时满足以下条件时才能删除：

1. 新 Engine 已提供功能等价入口；
2. 仓库内部调用已经迁移；
3. C++ 和 Python 旧入口已至少经过一个小版本弃用期；
4. API inventory 已记录替代入口和删除版本；
5. `rg` 确认没有非兼容层调用；
6. release notes 已说明迁移方式；
7. 全量测试、金标和性能门槛全部通过。

物理移动文件和删除兼容层不是独立业务阶段，应在所属 Engine 稳定后用单独 cleanup 提交执行。

## 10. 每阶段统一验证

功能验证：

```bash
./op.sh build
./op.sh small-test
./op.sh unit-test
./op.sh python-test
./op.sh import-test
```

额外验证：

```text
1. 重复生成 API inventory，确认差异符合预期；
2. 运行三组回测金标，比较逐笔交易、现金和最终持仓；
3. 使用相同 Release 构建和数据执行性能基准；
4. 记录执行前后接口数量、测试数量、耗时和峰值内存；
5. 检查本阶段是否修改了禁止区域；
6. 确认工作树只包含本阶段文件。
```

阶段完成报告必须包含：

| 内容 | 要求 |
| --- | --- |
| 执行前 | 当前接口数量、依赖、测试和性能基线 |
| 执行后 | 新接口、兼容层、文件变化和测试结果 |
| 差异 | 删除、迁移、新增及行为变化 |
| 验收 | 每条标准的证据和通过/不通过结论 |
| 性能 | 同数据同参数的重构前后结果 |
| 遗留项 | 明确说明未完成范围，不能用“全部完成”掩盖 |

## 11. 提交边界

推荐提交序列：

```text
docs: define three-engine refactoring boundaries

refactor: introduce data engine facade
refactor: move data runtime behind data engine
test: validate data engine compatibility and performance

refactor: introduce execution contracts
refactor: move account mutations behind execution engine
fix: normalize strategy order quantities
test: validate execution engine compatibility and performance

refactor: introduce strategy engine
refactor: move system runtime behind strategy engine
test: validate strategy engine compatibility and performance

refactor: separate Python public API from extension SPI
docs: publish engine API migration guide
```

每个提交只做一种变化，保持可审查、可二分、可回滚。

## 12. 完成定义

三引擎重构完成时必须同时满足：

- 普通用户主要通过 `HikyuuSession` 和三个 Engine 完成任务；
- `StockManager`、`TradeManagerBase`、`System` 不再承担新业务逻辑；
- 三个旧类只剩兼容转发，并有明确弃用计划；
- DataEngine、ExecutionEngine、StrategyEngine 的依赖方向无循环；
- Internal API 不出现在普通 Python 自动补全和稳定 C++ 聚合头中；
- 新增功能不再要求给三个旧大类添加方法；
- 所有行为金标保持一致，已批准的 bugfix 除外；
- 全量测试通过，性能回退在验收门槛内；
- 每个阶段都有执行前后对比和验收报告。
