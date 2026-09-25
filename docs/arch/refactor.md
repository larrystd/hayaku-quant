# Hikyuu 接口收敛与边界重构方案

本文档针对 Hikyuu 当前“公开接口过多、层次边界不清晰、C++ 内部能力几乎原样暴露到 Python”的问题，给出一套可分步实施、可回滚、可验证的重构方案。

目标不是重写算法，也不是减少策略组件的组合能力，而是：

- 缩小稳定公开 API；
- 分开用户 API、扩展接口（SPI）和内部接口；
- 把初始化、数据访问、回测、交易执行的边界明确下来；
- 降低 `StockManager`、`TradeManagerBase`、`System` 等“大接口”的职责；
- 保持现有回测结果和数据语义；兼容代码只在迁移中间态存在，最终删除旧 API 和旧目录。

## 1. 当前主要问题

### 1.1 接口没有分级

当前 C++ 头文件中只要是 `public`，通常就会被调用方认为是稳定 API；pybind11 又进一步把许多继承钩子、底层驱动和内部调度方法暴露给 Python。

典型例子：

- `StockManager` 同时负责生命周期、配置、证券查询、板块读写、交易日历、财务数据、预加载、插件和 IPC；
- `TradeManagerBase` 同时承担账户查询、账本写入、成本计算、券商通知、绩效统计、CSV 导出和子类扩展；
- `System` 同时承担组件装配、回测编排、单 Bar 执行、延迟订单状态机和 Portfolio 内部协作；
- Python 顶层包通过多处 `import *` 暴露了大量底层类和辅助函数；
- `_calculate`、`_reset`、`_add_valid` 等扩展钩子和普通用户接口混在同一个 Python 类上。

结果是调用方可以绕过正常流程直接修改中间状态，任何内部调整都可能成为破坏性变更。

### 1.2 依赖方向不稳定

当前主要依赖关系近似如下：

```text
┌─────────────────────┐
│ Python 顶层 API     │
└──────────┬──────────┘
           │ import *，直接暴露大量符号
           ▼
┌─────────────────────┐
│ pybind11 绑定        │
└──────────┬──────────┘
           │ 几乎逐方法映射 C++ 类型
           ▼
┌─────────────────────────────────────────────────────────────┐
│ StockManager │ System │ TradeManager │ 各类 Base            │
│ 生命周期、领域规则、调度、存储、插件和基础设施互相可见      │
└─────────────────────────────────────────────────────────────┘
```

目标依赖关系：

```text
┌─────────────────────────────────────────────────────────────┐
│ 稳定用户 API                                                │
│ Session │ DataService │ BacktestRunner │ AccountView         │
└──────────────────────────────┬──────────────────────────────┘
                               ▼
┌─────────────────────────────────────────────────────────────┐
│ 应用层                                                      │
│ 初始化用例 │ 数据查询用例 │ 回测用例 │ 订单执行用例          │
└──────────────────────────────┬──────────────────────────────┘
                               ▼
┌─────────────────────────────────────────────────────────────┐
│ 领域层                                                      │
│ Stock/KData │ Indicator │ System 组件 │ Order/Account/Ledger │
└──────────────────────────────┬──────────────────────────────┘
                               ▼
┌─────────────────────────────────────────────────────────────┐
│ Port（少量必要的扩展接口）                                  │
│ MarketDataPort │ BrokerPort │ CostModel │ StrategyPart SPI   │
└──────────────────────────────┬──────────────────────────────┘
                               ▼
┌─────────────────────────────────────────────────────────────┐
│ Adapter / Infrastructure                                    │
│ HDF5/MySQL/SQLite/TDX │ IPC/SHM │ pybind11 │ 券商适配器       │
└─────────────────────────────────────────────────────────────┘
```

依赖只能向下。领域层不能依赖 pybind11、Python、具体数据库或进程级单例。

## 2. 接口分类规则

每个现有接口都应被归入以下三类，而不是简单地判断“保留或删除”。

| 类型 | 面向对象 | 稳定性 | 例子 |
| --- | --- | --- | --- |
| Public API | 普通 C++/Python 用户 | 保持兼容、版本化弃用 | `get_stock`、指标工厂、运行回测 |
| Extension SPI | 自定义指标、策略部件、数据源、券商插件作者 | 文档化协议，允许少量纯虚方法 | `_calculate` 对应的受控扩展协议 |
| Internal API | Hikyuu 自身模块 | 不保证兼容，不直接绑定到 Python | 预加载任务、延迟订单处理、PF 内部强平 |

新增抽象接口必须满足至少一个条件：

1. 已经存在两个及以上实现；
2. 是明确的插件边界；
3. 是需要隔离测试的外部系统边界；
4. 是跨动态库或跨进程协议边界。

不满足以上条件时优先使用普通类、组合或自由函数，避免“每个类再套一个接口”。

## 3. 总体优先级

| 优先级 | 重构面 | 原因 |
| --- | --- | --- |
| P0 | API 清单与行为基线 | 没有基线就无法安全删除接口 |
| P1 | 初始化与全局状态 | `StockManager` 单例和 Python import 副作用影响所有模块 |
| P1 | 订单、执行、账户边界 | 回测和实盘语义混杂，风险最高 |
| P1 | Python 导出面 | 用户最直接感受到的“接口太多” |
| P2 | `StockManager` 职责拆分 | 数据域的扇出最大，但应在 Session 门面稳定后拆分 |
| P2 | `System` 编排和组件 SPI | 保留组合能力，隐藏内部状态机 |
| P3 | 目录物理迁移 | 只在逻辑边界稳定后进行 |
| 暂缓 | 指标算法、具体策略算法、序列化格式 | 改动收益低、回归面大 |

## 4. 分阶段实施步骤

> 执行状态更新：第 0～3 步已完成。旧 `StockManager/TradeManager/System` 体系、
> 兼容转发和旧目录已删除，生产代码和测试已迁移到 `hikyuu_cpp/src`和
> `hikyuu_cpp/test`。最终结果、验收数据和目录树以
> [第 3 步：最终架构冻结与旧体系退役](refactor/step-3-progress.md)为准。
> 下述原第 1～8 步保留为技术拆分参考，不再作为最终交付定义。

## 第 0 步：冻结行为并建立 API 清单

这一阶段不改变生产逻辑，只建立后续删接口所需的证据。

### 需要修改或新增的文件

| 文件 | 动作 |
| --- | --- |
| `docs/arch/api-inventory.md` | 新增；记录 C++ Public API、Python Public API、SPI、Internal 的分类 |
| `hikyuu_cpp/unit_test/hikyuu/hikyuu/test_StockManager.cpp` | 补充初始化、查询、临时证券和生命周期基线测试 |
| `hikyuu_cpp/unit_test/hikyuu/trade_manage/test_TradeManager.cpp` | 补充订单后资金、持仓、费用、交易记录的金标测试 |
| `hikyuu_cpp/unit_test/hikyuu/trade_sys/system/test_System.cpp` | 新增；固定简单策略逐 Bar 的交易结果和延迟请求行为 |
| `hikyuu_cpp/unit_test/hikyuu/strategy/test_Strategy.cpp` | 新增；固定 `order/orderValue/buy/sell` 的路由行为 |
| `hikyuu/test/test_public_api.py` | 新增；记录稳定顶层符号和弃用符号 |
| `hikyuu/test/test_backtest_golden.py` | 新增；固定关键回测的成交、资金和持仓结果 |
| `hikyuu/test/test.py` | 将新增 Python 测试加入统一入口 |

### 需要盘点的接口

- `hikyuu_cpp/hikyuu/StockManager.h` 的全部 `public` 方法；
- `hikyuu_cpp/hikyuu/trade_manage/TradeManagerBase.h` 的全部虚方法；
- `hikyuu_cpp/hikyuu/trade_sys/system/System.h` 的公开方法和 “For internal use by PF/AF only” 方法；
- `hikyuu_cpp/hikyuu/strategy/Strategy.h` 的行情查询及下单方法；
- `hikyuu_pywrap/**/*.cpp` 中所有 `m.def`、`.def` 和 `.def_property`；
- `hikyuu/__init__.py`、`hikyuu/core.py`、`hikyuu/extend.py` 的隐式导出符号。

### 这一阶段先不动

- 不改任何方法签名；
- 不移动目录；
- 不改变 `core310.so` 模块名；
- 不改序列化字段；
- 不改指标和交易系统算法。

### 验收标准

- 当前 `small-test`、`unit-test` 和 Python 测试全部通过；
- 至少选 3 个代表性策略保存逐笔 `TradeRecord`、每日资金、最终持仓金标；
- 每个对外符号被标为 Public、SPI、Internal 或 Deprecated；
- 后续每次移除接口都能在清单中找到替代接口和弃用版本。

## 第 1 步：引入显式 Session，停止扩散全局生命周期

先增加窄门面，不立即删除 `StockManager::instance()`。这是后续拆分的支点。

### 新增文件

| 文件 | 职责 |
| --- | --- |
| `hikyuu_cpp/hikyuu/application/HikyuuSession.h` | 进程级运行会话；负责初始化、关闭和服务访问 |
| `hikyuu_cpp/hikyuu/application/HikyuuSession.cpp` | 把配置解析和 `StockManager` 生命周期包装起来 |
| `hikyuu_cpp/hikyuu/application/SessionOptions.h` | 强类型初始化配置，替代五个无结构 `Parameter` 参数 |
| `hikyuu_pywrap/application/_HikyuuSession.cpp` | 只绑定用户需要的 Session 方法 |
| `hikyuu_pywrap/application/application_main.cpp` | 应用层绑定入口 |
| `hikyuu/session.py` | Python 上下文管理器和默认 Session 兼容入口 |
| `hikyuu_cpp/unit_test/hikyuu/application/test_HikyuuSession.cpp` | Session 生命周期测试 |

建议的窄接口：

```cpp
class HikyuuSession {
public:
    static HikyuuSession open(const SessionOptions&);
    void close();
    bool ready() const;
    void waitReady() const;

    DataService& data();
    BacktestService& backtest();
};
```

### 需要重构的现有文件和接口

| 文件 | 当前接口 | 处理方式 |
| --- | --- | --- |
| `hikyuu_cpp/hikyuu/hikyuu.h/.cpp` | 两组 `hikyuu_init`、`getConfigFromIni` | 实现改为委托 Session；旧函数保留并标记弃用 |
| `hikyuu_cpp/hikyuu/StockManager.h/.cpp` | `init/reload/reloadWith/quit` | 生命周期最终移到 Session；迁移期保留转发 |
| `hikyuu_cpp/hikyuu/StockManager.h` | `waitDataReady/cancelLoad/initializing/joinPreloadThread` | 降为 Session 内部生命周期接口，不再新增 Python 暴露 |
| `hikyuu_cpp/hikyuu/StockManager.h` | `getLoadTaskGroup/thread_id/clearPlugin/releaseShmServerBaseInfoCache` | 标为 Internal；移出公开段或放入内部协作者 |
| `hikyuu_pywrap/main.cpp` | `hikyuu_init` | 绑定到默认 Session 兼容函数，并发出弃用提示 |
| `hikyuu_pywrap/_StockManager.cpp` | `init/reload/reload_with/wait_data_ready/cancel_load` | 新代码转向 Session；旧绑定暂时保留 |
| `hikyuu/__init__.py` | import 时创建全局 `sm`、注册 cleanup、设置插件路径 | 改为惰性默认 Session；import 不应启动或操作数据加载 |

### 这一阶段保留不动

- `Stock`、`KData`、`KQuery` 的使用方式；
- `StockManager::getStock` 等查询行为；
- 原配置文件格式；
- 默认全局 `sm`，但只作为兼容代理存在。

### 验收标准

- `import hikyuu` 不初始化数据、不启动预加载线程；
- `with HikyuuSession.open(options) as session:` 可完整初始化和释放资源；
- 旧的 `hikyuu_init()` 和 `sm` 仍可运行现有示例；
- 同一进程重复 open/close 行为有明确测试；第一阶段不承诺多 Session 并行。

## 第 2 步：拆分 `StockManager`，建立数据域边界

`StockManager` 当前包含太多不相关能力，应从“万能管理器”变成兼容门面。

### 新增文件

| 文件 | 新职责 |
| --- | --- |
| `hikyuu_cpp/hikyuu/data/InstrumentRepository.h/.cpp` | 证券、市场、证券类型的只读查询 |
| `hikyuu_cpp/hikyuu/data/MarketDataService.h/.cpp` | K 线、交易日历、复权权重和行情查询 |
| `hikyuu_cpp/hikyuu/data/BlockRepository.h/.cpp` | 板块查询及显式的板块写入操作 |
| `hikyuu_cpp/hikyuu/data/FinanceDataService.h/.cpp` | 历史财务字段和财务记录查询 |
| `hikyuu_cpp/hikyuu/data/internal/DataRuntime.h/.cpp` | 驱动、缓存、预加载、IPC/SHM 的内部编排 |

服务边界建议：

```text
HikyuuSession
    │
    ├── InstrumentRepository  证券和市场元数据
    ├── MarketDataService     K线、日历、权重
    ├── BlockRepository       板块
    ├── FinanceDataService    财务数据
    └── DataRuntime           驱动、缓存、预加载、IPC（内部）
```

### `StockManager` 接口迁移表

| 当前接口组 | 目标位置 | 最终可见性 |
| --- | --- | --- |
| `getStock/getStockList/size/begin/end` | `InstrumentRepository` | Public |
| `getMarketInfo/getMarketStock/getAllMarket` | `InstrumentRepository` | Public |
| `getStockTypeInfo/getStockTypeInfoList` | `InstrumentRepository` | Public |
| `getTradingCalendar/isHoliday/isTradingHours` | `MarketDataService` | Public |
| `getStockWeightList/getZhBond10` | `MarketDataService` | Public |
| `getAllCategory/getBlock/getBlockList/getStockBelongs` | `BlockRepository` | Public |
| `addBlock/saveBlock/removeBlock` | `BlockRepository` 的 Writer 能力 | Public，但和只读接口分开 |
| `getHistoryFinance*` | `FinanceDataService` | Public |
| `addStock/removeStock/addTempCsvStock/removeTempCsvStock` | `TemporaryInstrumentStore` 或测试辅助类 | Advanced，不放顶层 |
| `get*DriverParameter/getBaseInfoDriver` | `DataRuntime` | Internal |
| `getPlugin/setPluginPath/getPluginPath/clearPlugin` | Session 的插件运行时 | Internal/Advanced |
| `isIpcClientMode/releaseShmServerBaseInfoCache` | `DataRuntime` | Internal |
| `_testingSetIpcClientMode` | 测试 fixture | Test-only，不进入发布头文件公开区 |

### 需要修改的文件

- `hikyuu_cpp/hikyuu/StockManager.h/.cpp`：逐步改成旧接口转发门面；
- `hikyuu_cpp/hikyuu/Stock.cpp`：不再直接知道 StockManager 的生命周期和 IPC 细节，通过 MarketDataService/DataRuntime 内部端口取数；
- `hikyuu_cpp/hikyuu/KData*.h/.cpp`：只在确有直接单例依赖的位置改为数据服务依赖，不改值对象语义；
- `hikyuu_cpp/hikyuu/data_driver/DataDriverFactory.h/.cpp`：从全局静态工厂改成 `DataRuntime` 持有的驱动注册表；
- `hikyuu_pywrap/_StockManager.cpp`：只保留兼容绑定；
- `hikyuu_pywrap/data_driver/_DataDriverFactory.cpp`：移出普通用户 API，放入 advanced/plugin 命名空间；
- `hikyuu_pywrap/data_driver/_BaseInfoDriver.cpp`、`_BlockInfoDriver.cpp`、`_KDataDriver.cpp`：只保留实现自定义驱动所需的 SPI，不绑定内部便捷方法。

### 这一阶段先不动

- 具体 HDF5、MySQL、SQLite、TDX、ClickHouse 驱动实现；
- `Stock`、`KData` 的公开值语义和 Python 操作方式；
- 数据表结构与配置文件格式；
- 预加载算法本身。

### 验收标准

- 普通行情查询不再需要知道 `DataDriverFactory`；
- 只读证券查询不能调用板块写操作或插件控制接口；
- `StockManager` 只剩兼容转发代码，不再新增业务逻辑；
- 新数据服务可使用内存 fake 独立测试，不依赖真实数据库。

## 第 3 步：明确“策略决策 → 订单 → 执行 → 账户”边界

当前 `Strategy::order/buy/sell`、`System`、`TradeManagerBase::buy/sell` 和 `OrderBrokerBase` 之间同时传递下单意图与成交结果，回测和实盘边界模糊。

目标流程：

```text
┌──────────────────┐
│ Strategy/System  │ 只产生交易意图
└────────┬─────────┘
         ▼
┌──────────────────┐
│ OrderRequest     │ 标的、方向、数量、价格约束、来源
└────────┬─────────┘
         ▼
┌──────────────────┐
│ ExecutionService │ 回测撮合或实盘 BrokerAdapter
└────────┬─────────┘
         ▼
┌──────────────────┐
│ ExecutionReport  │ 成交、拒绝、部分成交、费用
└────────┬─────────┘
         ▼
┌──────────────────┐
│ Account/Ledger   │ 唯一负责资金、持仓和账本变更
└──────────────────┘
```

### 新增文件

| 文件 | 职责 |
| --- | --- |
| `hikyuu_cpp/hikyuu/trade/OrderRequest.h` | 不可变下单请求 |
| `hikyuu_cpp/hikyuu/trade/ExecutionReport.h` | 执行结果；支持拒绝和部分成交语义 |
| `hikyuu_cpp/hikyuu/trade/ExecutionPort.h` | 回测撮合与实盘券商共同的最小端口 |
| `hikyuu_cpp/hikyuu/trade/BacktestExecution.h/.cpp` | 回测价格、滑点、费用和成交生成 |
| `hikyuu_cpp/hikyuu/trade/BrokerExecutionAdapter.h/.cpp` | 将新端口适配到现有 Broker |
| `hikyuu_cpp/hikyuu/trade/Account.h/.cpp` | 面向应用层的账户命令入口 |
| `hikyuu_cpp/hikyuu/trade/AccountSnapshot.h` | 只读资金、持仓、负债快照 |
| `hikyuu_cpp/hikyuu/trade/Ledger.h/.cpp` | 交易记录和资金/持仓变更的唯一写入者 |

### 需要重构的现有文件和接口

| 文件 | 当前接口 | 处理方式 |
| --- | --- | --- |
| `hikyuu_cpp/hikyuu/strategy/Strategy.h/.cpp` | `order/orderValue/buy/sell` | 用户保留 `order/orderValue`；`buy/sell` 下沉为 Execution 适配细节或 Advanced API |
| `hikyuu_cpp/hikyuu/trade_sys/system/TradeRequest.h/.cpp` | 延迟操作内部记录 | 与公开 `OrderRequest` 分开命名和职责；不直接作为成交订单 |
| `hikyuu_cpp/hikyuu/trade_manage/OrderBrokerBase.h/.cpp` | `_buy/_sell/_getAssetInfo` 和 JSON 资产字符串 | 迁移为 `ExecutionPort::submit/cancel/query` 与强类型快照；旧 Broker 用 adapter 保留 |
| `hikyuu_cpp/hikyuu/strategy/BrokerTradeManager.h/.cpp` | 同步券商资产与本地 TM | 改成 Broker adapter 和账户对账服务，避免继承扩大接口 |
| `hikyuu_cpp/hikyuu/trade_manage/TradeManagerBase.h/.cpp` | 交易和账户写入 | 实现改为调用 Ledger；旧签名先转发 |
| `hikyuu_cpp/hikyuu/trade_sys/system/System.cpp` | `_buyNow/_sellNow/...` | 只构造 OrderRequest，不直接同时修改账户和通知 Broker |
| `hikyuu_pywrap/strategy/_Strategy.cpp` | 绑定多套下单入口 | 顶层只暴露 `order/order_value` |
| `hikyuu_pywrap/trade_manage/_OrderBroker.cpp` | 暴露 `_buy/_sell/_get_asset_info` | 移到明确的 Python Broker SPI 类，不放普通 Broker 对象上 |

### 需要立即修正并锁定测试的问题

- `hikyuu_cpp/hikyuu/strategy/Strategy.cpp` 中 `order()` 对买入数量做了取整，但后续买入调用仍传原始 `num`；应先用测试固定期望，再在本阶段修复；
- 新增拒单、零数量、最小交易单位、部分成交、手续费、资金不足和重复回报测试；
- 回测执行和实盘执行必须返回同一种 `ExecutionReport`，但允许能力不同。

### 这一阶段先不动

- `Signal`、`MoneyManager`、`Stoploss` 等策略算法；
- `TradeRecord` 的旧序列化格式；
- 各类交易费用算法实现；
- 首轮不实现完整实盘 OMS，只定义清晰端口并适配现有 Broker。

### 验收标准

- 只有 Ledger 能改变账户资金和持仓；
- 策略不直接调用券商实现；
- 回测与实盘共用 `OrderRequest/ExecutionReport` 语义；
- 同一金标策略重构前后的逐笔成交和最终资产一致。

## 第 4 步：拆分 `TradeManagerBase` 大接口

`TradeManagerBase` 当前是最明显的接口膨胀点。不要再创建一个同样大的新基类，而应按调用目的拆成小能力。

### 目标能力

| 能力 | 只包含 |
| --- | --- |
| `AccountView` | 当前资金、持仓、负债、快照查询 |
| `LedgerWriter` | 应用成交回报、入金、出金、借还操作 |
| `TradeHistory` | 交易记录和历史持仓查询 |
| `CostModel` | 各类交易成本计算 |
| `PerformanceService` | 曲线、回撤、月度/年度收益计算 |
| `AccountExporter` | CSV 等输出，放外围工具层 |

### `TradeManagerBase` 接口迁移表

| 当前接口组 | 目标位置 | 处理 |
| --- | --- | --- |
| `currentCash/cash/have/getHoldNumber/getFunds/getPosition*` | `AccountView` | 稳定只读 API |
| `getTradeList/getHistoryPositionList/getBorrowStockList` | `TradeHistory` | 稳定查询 API |
| `buy/sell/sellShort/buyShort` | `LedgerWriter::apply(ExecutionReport)` | 旧接口弃用并转发 |
| `checkin/checkout/checkinStock/checkoutStock/borrow*/return*` | `AccountCommandService` | 命令接口，不与查询混合 |
| `addTradeRecord/addPosition` | `Ledger` | Internal；禁止普通用户直接写账 |
| `getBuyCost/getSellCost/getBorrow*Cost/getReturn*Cost` | `CostModel` | SPI/Public Advanced |
| `regBroker/clearBroker/getBrokerLastDatetime/setBrokerLastDatetime` | `ExecutionService` | 从账户中移除 |
| `getPerformance/getMaxPullBack/getProfitPercent*` | `PerformanceService` | 派生计算，不属于账户核心 |
| `getFundsList/getFundsCurve/getProfitCurve/getBaseAssetsCurve` | `PerformanceService` | 派生计算 |
| `tocsv` | `AccountExporter` | 外围工具，不放基类 |
| `fetchAssetInfoFromBroker` | `ReconciliationService` | 应用层用例 |
| `_reset/_clone/isPythonObject` | SPI/内部机制 | 不属于普通 Public API |

### 需要修改的文件

- `hikyuu_cpp/hikyuu/trade_manage/TradeManagerBase.h/.cpp`；
- `hikyuu_cpp/hikyuu/trade_manage/TradeManager.h/.cpp`；
- `hikyuu_cpp/hikyuu/trade_manage/Performance.h/.cpp`；
- `hikyuu_cpp/hikyuu/trade_manage/OrderBrokerBase.h/.cpp`；
- `hikyuu_cpp/hikyuu/trade_manage/TradeCostBase.h/.cpp`；
- `hikyuu_pywrap/trade_manage/_TradeManager.cpp`；
- `hikyuu_pywrap/trade_manage/_Performance.cpp`；
- `hikyuu_pywrap/trade_manage/_OrderBroker.cpp`；
- `hikyuu/trade_manage/trade.py`。

### 这一阶段先不动

- `TradeRecord`、`PositionRecord`、`FundsRecord` 等数据结构的字段；
- `TC_FixedA*`、`TC_FixedETF` 等费用实现；
- Boost serialization 版本与既有 pickle 兼容性。

### 验收标准

- 普通调用方只依赖 `AccountView`，无法调用内部写账接口；
- 自定义费用模型只实现 CostModel，不再继承整个 TradeManager；
- Performance 可针对任意 AccountSnapshot/TradeHistory 独立计算；
- `TradeManagerBase` 进入弃用期，不再接受新方法。

## 第 5 步：收窄 `System`，统一策略部件 SPI

EV、CN、SG、MM、ST、TP、PG、SP 是 Hikyuu 的核心组合能力，应保留；需要减少的是重复的生命周期接口和外泄的内部调度接口。

### 新增文件

| 文件 | 职责 |
| --- | --- |
| `hikyuu_cpp/hikyuu/trade_sys/component/ComponentContext.h` | 向组件提供只读 Stock/KData/AccountView/时间上下文 |
| `hikyuu_cpp/hikyuu/trade_sys/component/ComponentLifecycle.h` | 统一 `reset/prepare/clone` 的最小约定 |
| `hikyuu_cpp/hikyuu/trade_sys/system/SystemConfig.h` | 组件装配和运行参数，替代大量 setter |
| `hikyuu_cpp/hikyuu/trade_sys/system/BacktestRunner.h/.cpp` | 对外提供稳定的回测运行入口 |
| `hikyuu_cpp/hikyuu/trade_sys/system/internal/SystemEngine.h/.cpp` | 逐 Bar 状态机、延迟请求和通知编排 |

### 需要重构的文件和接口

| 文件 | 当前接口 | 处理方式 |
| --- | --- | --- |
| `trade_sys/system/System.h/.cpp` | `setTM/setMM/setEV/.../setSP` | 构造阶段使用 `SystemConfig`；旧 setter 进入弃用期 |
| `trade_sys/system/System.h/.cpp` | 三组 `run`、`runMoment*`、`readyForRun` | Public 只保留 BacktestRunner 的 `run`；逐时点执行降为 Internal |
| `trade_sys/system/System.h/.cpp` | `sellForceOnOpen/Close`、`pfProcessDelay*`、`clearDelayBuyRequest` | 移入 `SystemEngine` 的 PF/AF 内部端口 |
| `trade_sys/system/System.h/.cpp` | `get*TradeRequest/haveDelay*` | 对用户暴露只读 suggestion/snapshot，不暴露可变状态机细节 |
| `trade_sys/system/System.h` | `_reset/_forceResetAll/_clone/isPythonObject` | 归入 SPI 或内部桥接，不作为普通用户 API |
| 各 `*Base.h/.cpp` | 重复 `name/param/reset/clone/_calculate` | 统一公共生命周期协议，但保留每类算法的最小专有 SPI |
| `hikyuu_pywrap/trade_sys/_System.cpp` | `ready`、多重 run、全部可写组件属性 | 顶层收窄为 config + run + result；旧属性暂时兼容 |
| `hikyuu_pywrap/trade_sys/_Signal.cpp` 等 | 同时暴露普通方法和 `_calculate/_reset/_add_*` | 拆成用户类与 `hikyuu.spi` 扩展类 |

涉及的组件基类文件：

- `trade_sys/environment/EnvironmentBase.h/.cpp`；
- `trade_sys/condition/ConditionBase.h/.cpp`；
- `trade_sys/signal/SignalBase.h/.cpp`；
- `trade_sys/moneymanager/MoneyManagerBase.h/.cpp`；
- `trade_sys/stoploss/StoplossBase.h/.cpp`；
- `trade_sys/profitgoal/ProfitGoalBase.h/.cpp`；
- `trade_sys/slippage/SlippageBase.h/.cpp`；
- `trade_sys/selector/SelectorBase.h/.cpp`；
- `trade_sys/allocatefunds/AllocateFundsBase.h/.cpp`。

### 这一阶段先不动

- `crt/` 下的工厂函数命名，如 `SG_Cross`、`MM_FixedPercent`；
- `imp/` 下各策略算法；
- 指标表达式运算符；
- Portfolio 的选股和资金分配算法。

### 验收标准

- 普通用户不能直接调用逐 Bar 内部调度和 PF 强制卖出接口；
- Python 子类扩展仍然可用，但扩展入口集中在 `hikyuu.spi`；
- System 装配完成后关键依赖不可随意改动；
- 所有现有内置策略组件在金标回测中结果一致。

## 第 6 步：收敛 Python 顶层 API 和 pybind11 绑定

Python 接口是用户最直观的边界。C++ 类有一个 public 方法，不代表 Python 必须绑定它。

### 需要修改的文件

| 文件 | 修改方向 |
| --- | --- |
| `hikyuu/__init__.py` | 去除层层 `import *`；显式导出最常用稳定 API |
| `hikyuu/core.py` | 仅负责加载对应 Python 版本的二进制模块，不再作为所有符号的公共门面 |
| `hikyuu/extend.py` | 去除 `from .core import *`，改为显式导入 |
| `hikyuu/indicator/__init__.py` | 明确指标用户 API；生成器和实现细节分包 |
| `hikyuu/trade_manage/__init__.py` | 只导出账户视图、记录类型和常用费用工厂 |
| `hikyuu/trade_sys/__init__.py` | 只导出 System 配置、Runner 和内置组件工厂 |
| `hikyuu_pywrap/main.cpp` | 只注册基础类型及应用入口；修复错误的 `open_spend_time` 绑定 |
| `hikyuu_pywrap/*` | 按 Public/SPI/Internal 清单删减绑定 |
| Python `.pyi` 类型存根（当前工作树未包含） | 若恢复发布类型存根，则按新的稳定 API 生成，不提前手写不存在的文件 |
| `hikyuu/cpp/core3xx.pyi`（发布生成物） | 发布时通过 pybind11-stubgen 重新生成，不手改 |

### 建议的 Python 命名空间

```text
hikyuu                 最常用稳定 API
├── data               数据查询和导入工具
├── indicator          指标及内置指标工厂
├── backtest           SystemConfig、Runner、Result
├── trade              Order、AccountView、记录对象
├── analysis           分析
├── draw               绘图
├── advanced           驱动注册、临时证券、低层控制
└── spi                Python 自定义组件、数据源和 Broker 的扩展协议
```

### 顶层保留

- `Stock`、`KData`、`KQuery`、`Datetime`、`Indicator` 等高频领域类型；
- 常用指标工厂；
- 常用策略组件工厂，如 `SG_*`、`MM_*`；
- `open_session/load_hikyuu` 的兼容入口；
- 回测的单一入口。

### 移出顶层但不立即删除

- `DataDriverFactory`、各具体 Driver 基类；
- `_calculate/_reset/_clone/_add_*` 等 SPI 方法；
- IPC/SHM、插件加载、预加载线程控制；
- `runMoment*`、`pfProcessDelay*`、强制卖出等内部调度；
- 数据转换和调试辅助函数中的低频接口。

这些符号先迁移到 `hikyuu.advanced` 或 `hikyuu.spi`，旧路径至少保留一个小版本周期并发出 `DeprecationWarning`。

### 这一阶段先不动

- 常用工厂函数名称；
- `Stock/KData/Indicator` 的常用 Python 运算符；
- notebook 和文档示例所依赖的短别名，在完整迁移前不删除；
- 二进制模块的 Python 版本命名规则。

### 验收标准

- `dir(hikyuu)` 中只包含明确列入 `__all__` 的稳定符号；
- SPI 方法不出现在普通用户类的自动补全中；
- 所有被移动的接口有明确的新路径和警告；
- 文档示例、notebook 冒烟测试和 Python 单元测试通过。

## 第 7 步：收敛 C++ 聚合头文件和编译边界

完成逻辑迁移后再处理 C++ include 面，避免用户通过聚合头继续依赖内部实现。

### 需要修改的文件

| 文件 | 处理 |
| --- | --- |
| `hikyuu_cpp/hikyuu/hikyuu.h` | 只包含稳定用户 API，不再包含所有 trade/manage/strategy 实现 |
| `hikyuu_cpp/hikyuu/trade_sys/all.h` | 标为 legacy 聚合头；拆成 `public.h` 与内部 include |
| 各模块 `build_in.h` | 只聚合内置工厂，不聚合 Base 实现细节 |
| `hikyuu_cpp/hikyuu/xmake.lua` | 增加 application/data/trade 新目录；明确 public/private headers |
| `hikyuu_pywrap/xmake.lua` | 加入新绑定入口，内部头不对外安装 |

建议最终头文件布局：

```text
hikyuu/
├── api/                 稳定 Public API
├── spi/                 扩展协议
├── domain/              领域实现，不保证全部公开
├── application/         用例和编排
└── infrastructure/      driver、IPC、插件和适配器
```

目录迁移可使用转发头保持源码兼容，不能在同一提交中同时“大规模移动文件 + 改行为”。

### 这一阶段先不动

- 第三方依赖和 xmake 技术选型；
- 动态库名称和 ABI 版本策略，除非单独立项；
- 具体算法源文件的内部目录。

### 验收标准

- 只包含稳定 API 的最小 C++ 示例可以编译；
- Public 头不 include pybind11、数据库具体实现、IPC 或线程池头；
- include-what-you-use 或等价检查不再依赖 `hikyuu.h` 的传递包含；
- clean build、shared build 和 Python extension build 全部通过。

## 第 8 步：弃用、删除和物理清理

删除接口必须是最后一步，而不是重构的起点。

### 删除条件

同时满足以下条件才可删除旧接口：

1. 已存在功能等价的新入口；
2. 旧入口至少经过一个小版本的弃用期；
3. 仓库内调用、文档和示例已经迁移；
4. `rg` 确认没有内部使用；
5. release notes 已列出迁移方式；
6. 金标回测和完整测试通过。

### 最后删除或降级的主要接口

- `StockManager` 上的生命周期、驱动、插件和线程控制接口；
- `TradeManagerBase` 上的 Broker、Performance、CSV 和直接写账接口；
- `System` 上标注为 PF/AF internal 的公开方法；
- Python 普通类上的 `_calculate/_reset/_clone/_add_*` SPI 方法；
- `hikyuu/__init__.py` 的隐式星号导出；
- `DataDriverFactory` 的普通用户级 Python 入口。

## 5. 明确暂时不重构的区域

以下部分虽然也存在历史包袱，但不应进入第一轮，否则回归范围会失控。

| 区域 | 文件范围 | 暂缓原因 |
| --- | --- | --- |
| 指标计算核心 | `indicator/Indicator*.h/.cpp`、`indicator/imp/` | 性能敏感且已形成稳定表达式模型 |
| TA-Lib 指标 | `indicator_talib/` | 独立、边界相对清晰 |
| 内置策略算法 | `trade_sys/*/imp/` | 主要问题不在算法，而在基类和编排接口 |
| 工厂函数 | `trade_sys/*/crt/` | 是用户高频 DSL，先保持兼容 |
| 记录值对象 | `TradeRecord`、`PositionRecord`、`FundsRecord` 等 | 涉及序列化和 Python pickle 兼容 |
| 序列化 | `serialization/` | 应在边界稳定后单独版本化 |
| 绘图与 GUI | `hikyuu/draw/`、`hikyuu/gui/` | 不是当前接口膨胀根因 |
| 数据导入脚本 | `hikyuu/data/`、`hikyuu/fetcher/` | 可作为数据层调用方逐步迁移 |
| 具体数据驱动 | driver 的 HDF5/MySQL/SQLite/TDX 实现 | 先改注册和访问边界，不改实现 |

## 6. 推荐提交顺序

每个提交只做一种变化，便于 review 和二分回归。

1. `test: add public API inventory and backtest golden baselines`
2. `refactor: add explicit hikyuu session facade`
3. `refactor: split stock manager query services`
4. `refactor: introduce order and execution contracts`
5. `refactor: isolate account ledger from trade manager`
6. `refactor: move system runtime internals behind runner`
7. `refactor: separate Python public API from extension SPI`
8. `refactor: narrow aggregate headers and module exports`
9. `cleanup: remove deprecated compatibility interfaces`

不要把上述步骤压成一个大提交。尤其是第 3、4、5 步，每一步都应独立通过 C++ 和 Python 全量测试。

## 7. 每一步的统一验证命令

项目根目录已有 `op.sh`，每个阶段至少执行：

```bash
./op.sh build
./op.sh small-test
./op.sh unit-test
./op.sh python-test
./op.sh import-test
```

涉及 Python 导出时额外检查：

```bash
/opt/homebrew/bin/python3.10 -c "import hikyuu; print(sorted(hikyuu.__all__))"
```

涉及绑定修改后，在发布准备阶段重新生成并检查 `.pyi`；不要手工维护生成的 `core3xx.pyi`。

## 8. 第一轮建议范围

第一轮不要直接拆完整个项目，建议只交付下面三个可闭环结果：

### 里程碑 A：有基线

- 完成 API inventory；
- 增加 System/Strategy 测试；
- 建立三组代表性回测金标。

### 里程碑 B：有唯一应用入口

- 引入 `HikyuuSession`；
- Python import 去掉数据加载副作用；
- 旧 `hikyuu_init/sm` 通过兼容层工作。

### 里程碑 C：跑通一条新的交易纵切面

- 一个简单 Signal 产生 `OrderRequest`；
- `BacktestExecution` 产生 `ExecutionReport`；
- `Ledger` 更新账户；
- 新旧路径对同一个策略输出完全一致。

完成这三个里程碑后，再决定是否全面拆分 StockManager 和 TradeManager。这样可以先验证新边界是否真的更简单，而不是一次性把历史接口换成另一套同样复杂的接口。

## 9. 判断重构是否成功

不能只用“目录更整齐”判断成功，应使用以下指标：

- Python 顶层公开符号数量显著减少；
- `StockManager` 和 `TradeManagerBase` 的公开方法数量持续下降；
- Internal 方法不再出现在 pybind11 普通用户绑定中；
- 新功能不需要继续给上述大类增加方法；
- 数据驱动、Broker 和 Python 策略扩展分别只依赖各自 SPI；
- 单元测试可以使用内存 fake 测试应用层，不需要启动完整全局环境；
- 旧策略的交易记录和资金曲线在允许的数值误差内保持一致；
- 模块依赖方向可以稳定描述为“API → Application → Domain → Port → Adapter”。

最重要的约束是：减少接口不等于简单地删除方法，而是让每种调用方只看见完成自己职责所需的最小接口。
