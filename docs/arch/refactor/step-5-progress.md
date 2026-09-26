# 第 5 步：源码文件与目录组织收敛

> 状态：批次 0～6 已完成，第 5 步已通过结构、构建、API 与测试验收
>
> 前置条件：第 4 步完成并验收，工作区恢复到可稳定编译和测试的状态
>
> 决策日期：2026-09-25
>
> 文件级复核日期：2026-09-26
>
> 本步范围：`hayaku_cpp/src`、对应的 `hayaku_cpp/test`、构建文件，以及因路径变化必须调整的 pybind11 include
>
> 历史说明：第 5 步验收后，项目品牌已按后续用户决定提前原子切换为 `hayaku`；本文路径已同步到当前名称

## 1. 核心目标

本步骤只解决一个问题：源码文件和目录组织混乱。

当前主要问题不是缺少更多抽象，而是：

- `src` 有 92 个目录，很多目录只是 `crt`、`imp`、`internal`、`logic` 等实现标签。
- 同一业务能力被拆成大量很薄的头文件、工厂文件和实现文件。
- `data`、`app`、`common` 混入指标、插件、实时服务和具体后端。
- 目录名称不能直接说明文件属于哪个业务模块。
- 测试、pybind11 和 xmake 跟随旧路径，导致移动一个文件需要修改多个地方。

本步要得到的是一套稳定、容易浏览的物理结构。它不负责重新设计业务接口，也不通过增加类来“拆职责”。

## 2. 本步的边界

### 2.1 现在必须做

1. 将源码归入八个明确的一级目录：
   `common`、`data`、`operators`、`execution`、`metrics`、`strategy`、`application`、`extensions`。
2. 删除只表达实现方式的目录层级：
   `internal`、`imp`、`crt`、`logic`、`optimal`、`support`、`utils`。
3. 将可选后端和运行服务移入 `extensions`，不再与默认核心源码混放。
4. 合并同一职责下只有少量转发、构造或模板样板代码的薄文件。
5. 修正含糊、缩写、错误拼写或与内容不符的文件名；如果需要修改公共类型名，则延期处理。
6. 同步调整 include 路径、xmake 源文件列表、安装头路径和测试路径。
7. 让 `hayaku_cpp/test` 镜像新的核心模块，而不是镜像旧的实现目录。
8. pybind11 只修改因文件移动产生的 include 和构建路径，不修改 Python 可见行为。
9. 每完成一个模块立即删除旧路径，不建立长期转发目录。
10. 纯移动版本完整通过后，将本步范围内的生产头统一为单一 `#pragma once`，删除最外层传统 include guard；不得删除文件内部的功能条件编译块。

### 2.2 本步允许的代码变化

- 文件移动和文件改名。
- include 路径变化。
- 一个类的成员函数实现在多个 `.cpp` 之间重新归组。
- 将同一类族的无状态工厂函数或很薄实现合并到分类文件。
- 删除已确认没有生产调用、没有构建引用且测试无覆盖需求的死文件。
- 为保持移动前行为而进行的最小构建修正。
- 将传统头文件保护宏机械替换为单一 `#pragma once`。

除 include 路径外，公共类、函数签名、序列化标识和运行行为必须保持不变。

### 2.3 这一步先不做

| 延期内容 | 本步不做的原因 | 后续处理 |
|---|---|---|
| 拆分或合并 `DataRuntime`、`ExecutionRuntime`、`StrategyRuntime` | 这是对象职责和生命周期设计，不是目录整理 | 第 8 步统一评估是否直接并入三个 Engine |
| 重写 `StockManager`、`TradeManagerBase`、`System` | 会改变核心接口和调用关系 | 第 8 步接口收敛 |
| 新建 `DataCatalog`、`Ledger`、`OrderProcessor` 等类 | 当前目标是减少文件和接口，不能用新抽象完成目录整理 | 原则上不建；仅在第 8 步有明确独立生命周期时再评估 |
| `Stock -> Instrument`、`KData -> BarSeries` 等类型改名 | 会同时影响序列化、绑定、文档和用户代码 | 第 8 步公共 API 整理 |
| 品牌、顶层目录、namespace、宏和发布物统一改名 | 约涉及数千处文本修改，会掩盖物理移动差异 | 第 5 步验收后作为独立原子批次完成 |
| Python 包内部重新设计 | 不属于 C++ 源码物理组织 | 第 6 步 Python 暴露层与包结构收敛 |
| 插件接口、配置模型、序列化格式和 ABI 重设计 | 属于接口与兼容策略 | 后续独立步骤 |
| 指标公式、策略规则、成交、手续费、滑点和复权语义 | 属于业务行为 | 本轮禁止修改 |
| 无关的 C++20 现代化和性能优化 | 会扩大 diff，难以判断目录迁移是否等价 | 后续按热点和收益单独处理 |

第 5 步本身保持当时名称不动，以便单独验证物理迁移；后续品牌批次已一次性切换到
`hayaku`，且没有为旧名称增加兼容层。

## 3. 执行前基线

统计口径为当前 `hayaku_cpp/src` 工作区快照：

| 指标 | 当前值 |
|---|---:|
| 一级业务目录 | 6 |
| `src` 目录总数 | 92 |
| `.h/.cpp` 文件数 | 1,126 |
| 代码行数 | 121,094 |
| 不超过 30 行的 `.h/.cpp` | 288 |
| `strategy` 目录数 | 43 |
| `data/indicator` 文件数 | 401 |
| 禁止目录数 | 34 |
| 空目录数 | 0 |

以上统计已于 2026-09-26 在 `poc@2cada065d7ae9d86c31d07a9dcd46a903974c1f3` 重新确认。文件数只用于发现碎片，不设“必须压缩到某个数字”的硬指标，避免为了数量合并不相关代码。

### 3.1 批次 0 实际基线

| 检查项 | 结果 |
|---|---|
| 工作区 | 源码无修改；仅本文档为未跟踪文件 |
| configure | 通过 |
| core + Python extension 构建 | 通过 |
| `small-test` | 51/51 用例、3356/3356 断言通过 |
| `unit-test` | 804/805 用例、208437/208438 断言通过；唯一失败为执行前已有的 SpotAgent TCP 端口 0 夹具问题 |
| plugin ABI | 7/7 用例、56/56 断言通过 |
| Python | 66/66 通过 |
| import | Python 3.10.21、hayaku 2.8.2 正常加载 |
| 架构扫描 | 547 个 C/C++ 文件，无 `data/analysis -> app` 依赖 |
| API inventory | 274 个 C++ 方法、888 个 pybind 导出；基线保存到忽略目录 `build/step5-api-inventory-before.md` |

`extract_api_inventory.py` 当前在输出到仓库外绝对路径时会在日志打印阶段抛出 `ValueError`；文件本身已成功写出。批次 5 修复该工具，批次 0 暂改用仓库内已忽略的 `build/` 路径。SpotAgent 基线失败在任何 Step 5 源码修改前可稳定复现，迁移到 `extensions/realtime` 时修正测试夹具；其他 804 个用例不得回退。

## 4. 本步目标目录与文件

本步使用当前项目名。以下是文件级目标，不是只画目录框。

记号约定：

- `Name.{h,cpp}` 表示最终同时存在 `Name.h` 和 `Name.cpp`。
- 标注“合并”的文件承接列出的现有类型或函数，不代表新增同名公共类。
- 公共函数和类型名本步保持不变；变化的是文件名、include 路径和编译单元组织。
- 复杂、有状态且有独立测试价值的类继续单独成文件；只有同职责的薄包装才合并。

### 4.1 根目录与 `common`

```text
hayaku_cpp/
├── src/
│   ├── LICENSE.txt
│   ├── doc.h
│   ├── hayaku.h                         当前项目总入口
│   ├── xmake.lua
│   │
│   ├── common/
│   │   ├── Arithmetic.{h,cpp}           原 arithmetic
│   │   ├── Base64.{h,cpp}
│   │   ├── Config.h                     原 common/config.h
│   │   ├── CppDef.h
│   │   ├── Debug.h
│   │   ├── DllLoader.h
│   │   ├── Exception.{h,cpp}
│   │   ├── Expected.h
│   │   ├── FileLock.{h,cpp}
│   │   ├── Lang.{h,cpp}
│   │   ├── Log.{h,cpp}
│   │   ├── Md5.{h,cpp}
│   │   ├── MoFileReader.h
│   │   ├── Net.h
│   │   ├── Null.h
│   │   ├── OmpMacro.h
│   │   ├── Os.{h,cpp}
│   │   ├── OsDef.h
│   │   ├── Parameter.{h,cpp}
│   │   ├── ResourceVersionTraits.h         供 database 与 telemetry 资源池共同使用
│   │   ├── RuntimeInfo.h
│   │   ├── CommonSerialization.{h,cpp}   Datetime/TimeDelta 等基础值序列化
│   │   ├── StringConversion.h           原 any_to_string.h
│   │   ├── StringView.h                 第 8 步再决定是否由 std::string_view 替代
│   │   │
│   │   ├── time/
│   │   │   ├── Datetime.{h,cpp}
│   │   │   ├── TimeDelta.{h,cpp}
│   │   │   └── SpendTimer.{h,cpp}
│   │   │
│   │   ├── concurrency/
│   │   │   ├── FuncWrapper.h
│   │   │   ├── GlobalStealThreadPool.{h,cpp}
│   │   │   ├── InterruptFlag.h
│   │   │   ├── ParallelAlgorithms.{h,cpp}
│   │   │   ├── MQStealQueue.h
│   │   │   ├── MQStealThreadPool.h
│   │   │   ├── MQThreadPool.h
│   │   │   ├── StealThreadPool.h
│   │   │   ├── ThreadPool.h
│   │   │   ├── ThreadSafeQueue.h
│   │   │   └── WorkStealQueue.h
│   │   │
│   │   └── database/
│   │       ├── AsyncDBConnectBase.h
│   │       ├── AsyncSQLResultSet.h
│   │       ├── AsyncSQLStatementBase.h
│   │       ├── DBCondition.{h,cpp}
│   │       ├── DBConnect.h
│   │       ├── DBConnectBase.h
│   │       ├── DBUpgrade.{h,cpp}
│   │       ├── ResourcePool.h
│   │       ├── SQLException.h
│   │       ├── SQLResultSet.h
│   │       ├── SQLStatementBase.h
│   │       ├── SQLiteConnect.{h,cpp}
│   │       ├── SQLiteStatement.{h,cpp}
│   │       ├── AsyncSQLiteConnect.{h,cpp}
│   │       ├── AsyncSQLiteStatement.{h,cpp}
│   │       ├── SQLiteUtil.{h,cpp}
│   │       ├── TableMacro.h
│   │       └── Transactions.h           合并 AutoTransAction/AsyncTransAction
```

不进入目标树的已知候选包括 `FilterNode.h`、`LRUCache11.h`、`snowflake.h` 和无调用的 Hybrid/TLS 资源池变体。只有生产引用扫描、构建矩阵和测试都证明无使用后才删除。`LruCache.h` 当前只服务 MySQL，随 MySQL 扩展移动；`TimerManager.h` 随实时扩展移动。

### 4.2 `data` 与 `operators`

```text
│   ├── data/
│   │   ├── Block.{h,cpp}
│   │   ├── DataEngine.{h,cpp}
│   │   ├── DataRuntime.{h,cpp}           只从 internal 移出，本步不拆不并
│   │   ├── MarketTypes.h                 原 DataType.h，仅改文件名和 include
│   │   ├── HistoryFinanceInfo.{h,cpp}
│   │   ├── KData.{h,cpp}
│   │   ├── KDataExtension.{h,cpp}
│   │   ├── KDataImp.{h,cpp}
│   │   ├── KDataPrivatedBufferImp.{h,cpp} 类型名拼写问题登记到第 8 步
│   │   ├── KDataSharedBufferImp.{h,cpp}
│   │   ├── KQuery.{h,cpp}
│   │   ├── MarketInfo.{h,cpp}
│   │   ├── KRecord.{h,cpp}
│   │   ├── TimeLineRecord.{h,cpp}
│   │   ├── TransRecord.{h,cpp}
│   │   ├── RealtimeDataSource.{h,cpp}
│   │   ├── Stock.{h,cpp}
│   │   ├── StockMapIterator.h
│   │   ├── StockTypeInfo.{h,cpp}
│   │   ├── StockWeight.{h,cpp}
│   │   ├── StrategyContext.{h,cpp}
│   │   ├── ZhBond10.{h,cpp}
│   │   ├── DataSerialization.{h,cpp}     合并 data 类型序列化适配，格式不变
│   │   └── storage/
│   │       ├── BaseInfoDriver.{h,cpp}
│   │       ├── BlockInfoDriver.{h,cpp}
│   │       ├── DataDriverFactory.{h,cpp}
│   │       ├── DriverConnectPool.h
│   │       ├── HistoryFinanceReader.{h,cpp}
│   │       ├── KDataDriver.{h,cpp}
│   │       ├── DoNothingKDataDriver.h
│   │       ├── BaseInfoTables.h          合并八个只描述表结构的头文件
│   │       ├── SQLiteBaseInfoDriver.{h,cpp}
│   │       ├── SQLiteBlockInfoDriver.{h,cpp}
│   │       └── SQLiteKDataDriver.{h,cpp}
│   │
│   ├── operators/
│   │   ├── IndParam.{h,cpp}
│   │   ├── Indicator.{h,cpp}
│   │   ├── IndicatorImp.{h,cpp}
│   │   ├── Indicator2InImp.{h,cpp}
│   │   ├── IndicatorImpBuffer.{h,cpp}
│   │   ├── IndicatorSupport.{h,cpp}      原 utils/WildcardMatch
│   │   ├── Factor.{h,cpp}
│   │   ├── FactorSet.{h,cpp}
│   │   ├── FactorStore.{h,cpp}
│   │   ├── CompiledFactorPlan.{h,cpp}
│   │   ├── ScalarMathOperators.{h,cpp}   ABS/CEILING/FLOOR/INTPART/MOD/EXP/LN/LOG/
│   │   │                                  POW/SQRT/SGN/SIGNED_POWER/ROUND 系列
│   │   ├── TrigonometricOperators.{h,cpp} ACOS/ASIN/ATAN/COS/SIN/TAN
│   │   ├── SeriesOperators.h             序列来源、引用和变换的公共工厂声明
│   │   ├── SeriesSourceOperators.cpp     CVAL/KDATA/PRICELIST/CONTEXT/RESULT
│   │   ├── SeriesReferenceOperators.cpp  RECOVER/REF/REFX/LASTVALUE
│   │   ├── SeriesTransformOperators.cpp  ALIGN/BACKSET/DISCARD/DROPNA/REPLACE/
│   │   │                                  REVERSE/SLICE
│   │   ├── BooleanOperators.h            布尔判断与事件判断的公共工厂声明
│   │   ├── PredicateOperators.cpp        BETWEEN/EVERY/EXIST/FILTER/NOT/IS*
│   │   ├── EventOperators.cpp            CROSS/NDAY/UPNDAY/DOWNNDAY/JUMP*/LONGCROSS
│   │   ├── WindowOperators.h             窗口位置、聚合和移动平均声明
│   │   ├── PositionOperators.cpp         BARSCOUNT/BARSLAST*/BARSSINCE/LAST/SUMBARS
│   │   ├── RollingExtremaOperators.cpp   HHV/HHVBARS/LLV/LLVBARS/MAX/MIN
│   │   ├── RollingAggregateOperators.cpp COUNT/SUM/AVEDEV/DEVSQ
│   │   ├── MovingAverageOperators.cpp    EMA/MA/SMA/WMA
│   │   ├── AdaptiveAverageOperators.cpp  AMA/DMA/KALMAN
│   │   ├── StatisticsOperators.h         统计、相关性和回归声明
│   │   ├── StatisticalMoments.cpp        KURT/MRR/SKEW/STDEV/STDP/VAR/VARP/ZSCORE
│   │   ├── StatisticalRelations.cpp      BETA/CORR/COV/SPEARMAN/QUANTILE_TRUNC
│   │   ├── RegressionOperators.cpp       IC/ICIR/IR/SLOPE/TS_RANK
│   │   ├── MomentumOperators.h           趋势和振荡指标声明
│   │   ├── TrendOperators.cpp            ADX/ADX2/ATR/DIFF/MACD/TR
│   │   ├── OscillatorOperators.cpp       ROC*/RSI/VIGOR
│   │   ├── RiskOperators.{h,cpp}         MDD/MDD_CURRENT/RSRS_*/SAFTYLOSS
│   │   ├── OperatorCombinations.{h,cpp}  原 analysis/combinate
│   │   ├── MarketOperators.h             行情与标的相关工厂声明
│   │   ├── MarketBreadthOperators.cpp    ADVANCE/DECLINE/BLOCKSETNUM/INBLOCK/INSUM
│   │   ├── MarketMetadataOperators.cpp   ADJ_FACTOR/CODELIKE/CYCLE/FACTOR/FINANCE/
│   │   │                                  LIUTONGPAN/NAMELIKE/STKTYPE/ZHBOND10/ZONGGUBEN
│   │   └── MarketPriceVolumeOperators.cpp AD/COST/HSL/INDEX/TIME/TIMELINE*/TURNOVER/WINNER
```

`operators` 不再平铺约 400 个文件，也不再保留一份 `crt/X.h` 加一份 `imp/IX.{h,cpp}` 的三文件模板。公共工厂按能力进入少量 `*Operators.h`，实现按算法族分布到上面列出的 `.cpp`，避免一个类别对应一个巨型实现文件。执行前需要按实际行数再次校验；超过约 1,000 行时继续按同一算法族拆实现 `.cpp`，但不新增公共类型或 `crt/imp` 目录。

### 4.3 `execution` 与 `metrics`

```text
│   ├── execution/
│   │   ├── ExecutionEngine.{h,cpp}
│   │   ├── ExecutionRuntime.{h,cpp}      只从 internal 移出，本步不拆不并
│   │   ├── ExecutionAccountFactory.{h,cpp}
│   │   ├── ExecutionAccountPort.h
│   │   ├── ExecutionBrokerPort.h
│   │   ├── PortfolioAccountPort.h
│   │   ├── Ledger.h
│   │   ├── AccountConfig.h
│   │   ├── AccountId.h
│   │   ├── AccountSnapshot.h
│   │   ├── AccountView.h
│   │   ├── PositionExtInfo.h
│   │   ├── OrderOrigin.{h,cpp}
│   │   ├── OrderRequest.h
│   │   ├── ExecutionReport.h
│   │   ├── BorrowRecord.{h,cpp}
│   │   ├── CostRecord.{h,cpp}
│   │   ├── FundsRecord.{h,cpp}
│   │   ├── LoanRecord.{h,cpp}
│   │   ├── PositionRecord.{h,cpp}
│   │   ├── TradeRecord.{h,cpp}
│   │   ├── broker/
│   │   │   └── OrderBrokerBase.{h,cpp}
│   │   └── pricing/
│   │       ├── TradeCostBase.{h,cpp}
│   │       ├── TradeCosts.{h,cpp}        FixedA*/ETF/Zero/Stub 与 TC_* 工厂
│   │       ├── SlippageBase.{h,cpp}
│   │       └── SlippageModels.{h,cpp}    Fixed/Normal/LogNormal/TruncNormal/Uniform
│   │
│   ├── metrics/
│   │   ├── Performance.{h,cpp}
│   │   ├── ReportExtension.{h,cpp}
│   │   └── BatchMetrics.{h,cpp}          原 misc 中的批量统计函数
```

`AccountSnapshot` 与 `AccountView`、多个 TradeCost 类型是否应进一步合并，属于第 8 步接口决策。本步只允许把现有声明归入同一文件，不删除类型、不改变调用语义。

### 4.4 `strategy`

```text
│   ├── strategy/
│   │   ├── Strategy.{h,cpp}
│   │   ├── StrategyEngine.{h,cpp}
│   │   ├── StrategyRuntime.{h,cpp}       只从 internal 移出，本步不拆不并
│   │   ├── StrategyExecutionPort.{h,cpp}
│   │   ├── StrategyDefinition.{h,cpp}
│   │   ├── BacktestRequest.h
│   │   ├── BacktestResult.{h,cpp}
│   │   ├── ComponentContext.{h,cpp}
│   │   ├── PendingOrderState.h
│   │   └── StrategyRunners.{h,cpp}       合并 RunSystemInStrategy/RunPortfolioInStrategy
│   │
│   │   ├── decision/
│   │   │   ├── SignalBase.{h,cpp}
│   │   │   ├── Signals.{h,cpp}           合并 SG_* 工厂声明与 SG_Flex 实现
│   │   │   ├── AllwaysBuySignal.{h,cpp}
│   │   │   ├── BandSignal.{h,cpp}
│   │   │   ├── BandSignal2.{h,cpp}
│   │   │   ├── BoolSignal.{h,cpp}
│   │   │   ├── CrossGoldSignal.{h,cpp}
│   │   │   ├── CrossSignal.{h,cpp}
│   │   │   ├── CycleSignal.{h,cpp}
│   │   │   ├── ManualSignal.{h,cpp}
│   │   │   ├── OneSideSignal.{h,cpp}
│   │   │   ├── SingleSignal.{h,cpp}
│   │   │   ├── SingleSignal2.{h,cpp}
│   │   │   ├── SignalExpressions.h       Add/Sub/Mul/Div/And/Or 与 value 组合声明
│   │   │   ├── SignalArithmeticExpressions.cpp
│   │   │   ├── SignalBooleanExpressions.cpp
│   │   │   ├── ConditionBase.{h,cpp}
│   │   │   ├── Conditions.{h,cpp}        合并 CN_* 工厂
│   │   │   ├── BoolCondition.{h,cpp}
│   │   │   ├── ManualCondition.{h,cpp}
│   │   │   ├── OPLineCondition.{h,cpp}
│   │   │   ├── ConditionExpressions.{h,cpp}
│   │   │   ├── EnvironmentBase.{h,cpp}
│   │   │   ├── Environments.{h,cpp}      合并 EV_* 工厂
│   │   │   ├── BoolEnvironment.{h,cpp}
│   │   │   ├── ManualEnvironment.{h,cpp}
│   │   │   ├── TwoLineEnvironment.{h,cpp}
│   │   │   └── EnvironmentExpressions.{h,cpp}
│   │
│   │   ├── risk/
│   │   │   ├── MoneyManagerBase.{h,cpp}
│   │   │   ├── MoneyManagers.{h,cpp}     合并 MM_* 工厂
│   │   │   ├── FixedCapitalFundsMM.{h,cpp}
│   │   │   ├── FixedCapitalMoneyManager.{h,cpp}
│   │   │   ├── FixedCountMoneyManager.{h,cpp}
│   │   │   ├── FixedCountTpsMM.{h,cpp}
│   │   │   ├── FixedPercentMoneyManager.{h,cpp}
│   │   │   ├── FixedRiskMoneyManager.{h,cpp}
│   │   │   ├── FixedUnitsMoneyManager.{h,cpp}
│   │   │   ├── NotMoneyManager.{h,cpp}
│   │   │   ├── WilliamsFixedRiskMoneyManager.{h,cpp}
│   │   │   ├── StoplossBase.{h,cpp}
│   │   │   ├── StoplossRules.{h,cpp}     合并 ST_* 工厂和 Saftyloss 工厂实现
│   │   │   ├── FixedPercentStoploss.{h,cpp}
│   │   │   ├── IndicatorStoploss.{h,cpp}
│   │   │   ├── ProfitGoalBase.{h,cpp}
│   │   │   ├── ProfitGoals.{h,cpp}       合并 PG_* 工厂
│   │   │   ├── FixedHoldDays.{h,cpp}
│   │   │   ├── FixedPercentProfitGoal.{h,cpp}
│   │   │   └── NoGoalProfitGoal.{h,cpp}
│   │
│   │   ├── selection/
│   │   │   ├── SelectorBase.{h,cpp}
│   │   │   ├── StrategyWeight.{h,cpp}
│   │   │   ├── Selectors.{h,cpp}         合并 SE_* 工厂
│   │   │   ├── FixedSelector.{h,cpp}
│   │   │   ├── MultiFactorSelector.{h,cpp}
│   │   │   ├── MultiFactorSelector2.{h,cpp}
│   │   │   ├── SignalSelector.{h,cpp}
│   │   │   ├── OperatorSelector.{h,cpp}
│   │   │   ├── OperatorValueSelector.{h,cpp} 独立有状态类，不并入表达式文件
│   │   │   ├── SelectorExpressions.{h,cpp} 合并其余薄算术包装
│   │   │   ├── OptimalSelectorBase.{h,cpp}
│   │   │   ├── MaxFundsOptimalSelector.{h,cpp}
│   │   │   ├── OptimalEvaluateSelector.{h,cpp}
│   │   │   ├── PerformanceOptimalSelector.{h,cpp}
│   │   │   ├── MultiFactorBase.{h,cpp}
│   │   │   ├── MultiFactors.{h,cpp}      合并 MF_* 工厂
│   │   │   ├── EqualWeightMultiFactor.{h,cpp}
│   │   │   ├── ICMultiFactor.{h,cpp}
│   │   │   ├── ICIRMultiFactor.{h,cpp}
│   │   │   ├── WeightMultiFactor.{h,cpp}
│   │   │   ├── NormalizeBase.{h,cpp}
│   │   │   ├── Normalizers.h             合并 buildin_norm 工厂声明
│   │   │   ├── NormMinMax.{h,cpp}
│   │   │   ├── NormQuantile.{h,cpp}
│   │   │   ├── NormQuantileUniform.{h,cpp}
│   │   │   ├── NormZScore.{h,cpp}
│   │   │   ├── QuantileTrunc.{h,cpp}
│   │   │   ├── ScoresFilterBase.{h,cpp}
│   │   │   ├── ScoreFilters.h            合并 buildin_scfilter 工厂声明
│   │   │   ├── GroupSCFilter.{h,cpp}
│   │   │   ├── IgnoreLessOrEqualValueSCFilter.{h,cpp}
│   │   │   ├── IgnoreNanSCFilter.{h,cpp}
│   │   │   ├── MinAmountPercentSCFilter.{h,cpp}
│   │   │   ├── PriceSCFilter.{h,cpp}
│   │   │   ├── TopNSCFilter.{h,cpp}
│   │   │   ├── ScoreRecord.{h,cpp}
│   │   │   ├── StyleRegression.{h,cpp}
│   │   │   └── IndustryNeutralize.h
│   │
│   │   └── portfolio/
│   │       ├── Portfolio.{h,cpp}
│   │       ├── Portfolios.{h,cpp}        合并 PF_* 工厂
│   │       ├── SimplePortfolio.{h,cpp}
│   │       ├── WithoutAFPortfolio.{h,cpp}
│   │       ├── AllocateFundsBase.{h,cpp}
│   │       ├── AllocationPolicies.{h,cpp} 合并 AF_* 工厂
│   │       ├── EqualWeightAllocateFunds.{h,cpp}
│   │       ├── FixAmountFunds.{h,cpp}
│   │       ├── FixedWeightAllocateFunds.{h,cpp}
│   │       ├── FixedWeightListAllocateFunds.{h,cpp}
│   │       └── MultiFactorAllocaterFunds.{h,cpp}
```

这里保留现有 Base 类型，是因为删除或改名会改变公共接口；它们将在第 8 步统一处理。本步只消除围绕每个 Base 再建立 `crt/imp/logic` 三层目录和一批一两行工厂头的做法。`AllwaysBuySignal`、`Saftyloss`、`MultiFactorAllocaterFunds` 等拼写问题也暂时保持文件与公共类型一致，并登记到第 8 步原子改名，避免出现“文件名已修、类型名未修”的半迁移状态。

### 4.5 `application` 与 `extensions`

```text
│   ├── application/
│   │   ├── HayakuSession.{h,cpp}         品牌批次已完成类型改名
│   │   ├── SessionOptions.{h,cpp}
│   │   ├── GlobalInitializer.{h,cpp}
│   │   ├── DataRuntimeAssembly.{h,cpp}
│   │   ├── PluginRuntime.{h,cpp}
│   │   ├── ConfigLoader.{h,cpp}
│   │   ├── IniParser.{h,cpp}
│   │   ├── SystemInfo.{h,cpp}            原 sysinfo 的版本与 Python 运行状态
│   │   └── plugins/
│   │       ├── PluginBase.h
│   │       ├── PluginClient.h
│   │       ├── PluginLoader.h
│   │       ├── PluginManager.h
│   │       ├── PluginIds.h
│   │       ├── CheckDataPluginInterface.h
│   │       ├── DataDriverPluginInterface.h
│   │       ├── DataServerPluginInterface.h
│   │       ├── DevicePluginInterface.h
│   │       ├── ExtendIndicatorsPluginInterface.h
│   │       ├── HayakuExtraPluginInterface.h
│   │       ├── ImportKDataToClickHousePluginInterface.h
│   │       ├── ImportKDataToHdf5PluginInterface.h
│   │       ├── ImportKDataToMySQLPluginInterface.h
│   │       ├── ShmServerPluginInterface.h
│   │       ├── TMReportPluginInterface.h
│   │       ├── DevicePlugin.{h,cpp}
│   │       ├── FactorPlugin.{h,cpp}
│   │       ├── ExtendIndicatorsPlugin.{h,cpp}
│   │       └── ExtraPlugins.{h,cpp}      hayakuextra/TMReport 等薄装配
│   │
│   └── extensions/
│       ├── ingest/
│       │   ├── IngestExport.h
│       │   ├── CheckData.{h,cpp}
│       │   ├── KDataTempCsvDriver.{h,cpp}  保持公共类名不变
│       │   ├── TdxKDataDriver.{h,cpp}
│       │   ├── QLBlockInfoDriver.{h,cpp}
│       │   ├── KDataToClickHouseImporter.{h,cpp}
│       │   ├── KDataToHdf5Importer.{h,cpp}
│       │   └── KDataToMySQLImporter.{h,cpp}
│       ├── realtime/
│       │   ├── GlobalSpotAgent.{h,cpp}
│       │   ├── RealtimePort.{h,cpp}
│       │   ├── RealtimeExport.h
│       │   ├── SpotAgent.{h,cpp}
│       │   ├── SpotRecord.h
│       │   ├── Scheduler.{h,cpp}
│       │   ├── ScheduledTasks.{h,cpp}
│       │   ├── TimerManager.h
│       │   ├── NodeClient.h
│       │   ├── NodeServer.h
│       │   ├── NodeMessage.h
│       │   ├── NodeError.h
│       │   ├── ShmClientHook.{h,cpp}
│       │   ├── ShmMirrorSink.{h,cpp}
│       │   ├── KDataShmBufferImp.{h,cpp}
│       │   ├── DataServerPlugin.{h,cpp}
│       │   ├── ShmServerPlugin.{h,cpp}
│       │   └── spot.fbs                  spot_generated.h 只生成到 build 目录
│       ├── mysql/
│       │   ├── MySQLConnect.h
│       │   ├── MySQLConnectBoost.cpp
│       │   ├── MySQLConnectNative.cpp
│       │   ├── MySQLStatement.h
│       │   ├── MySQLStatementBoost.cpp
│       │   ├── MySQLStatementNative.cpp
│       │   ├── AsyncMySQLConnect.{h,cpp}
│       │   ├── AsyncMySQLStatement.{h,cpp}
│       │   ├── MySQLBaseInfoDriver.{h,cpp}
│       │   ├── MySQLBlockInfoDriver.{h,cpp}
│       │   ├── MySQLKDataDriver.{h,cpp}
│       │   ├── KRecordTable.h
│       │   ├── LruCache.h
│       │   └── MySQLSupport.cpp          原 mysql_imp
│       ├── hdf5/
│       │   ├── H5KDataDriver.{h,cpp}
│       │   └── H5Record.h
│       ├── talib/
│       │   ├── TalibOperators.h          保留全部 TA_* 工厂签名
│       │   ├── TalibMovingAverageOperators.cpp
│       │   ├── TalibPriceOscillatorOperators.cpp
│       │   ├── TalibStatisticsOperators.cpp
│       │   └── TalibSupport.{h,cpp}      原 ta_defines/ta_imp
│       └── telemetry/
│           ├── Telemetry.{h,cpp}         CanUpgrade/反馈/授权提醒，公共函数名不变
│           ├── HttpClient.{h,cpp}        保留现有 AsioHttpClient 类型
│           ├── HttpException.h
│           ├── ResourceAsioPool.h
│           └── Url.{h,cpp}
```

当前没有足够独立源码支撑单独的 `clickhouse/` 目录：ClickHouse 目前只有导入适配，因此放入 `ingest`。`sysinfo` 则可以在不改变函数签名的前提下按文件拆分：版本和 Python 运行状态留在 `application/SystemInfo`，升级检查、反馈和授权提醒连同其唯一使用的 HTTP 客户端进入 `telemetry`。这属于文件归位，不引入新业务接口。

`spot_generated.h` 不再作为手写源码保留；Integration owner 在 xmake 中建立 `spot.fbs -> spot_generated.h` 的 `flatc` 生成依赖，输出到 target 的 build/autogen 目录。生成头不参与 `#pragma once` 机械整理。`RealtimePort`、Scheduler、IPC hook 和 `KDataShmBufferImp` 虽位于 `extensions/realtime`，仍属于 core target；只有 SpotAgent、GlobalSpotAgent 和两个实时插件属于可选 `hayaku-realtime` target。

### 4.6 测试目录

```text
└── test/
    ├── test_main.cpp
    ├── xmake.lua
    ├── common/                       按公共工具主题保留 test_<subject>.cpp
    ├── data/                         对应 data 与 storage
    ├── operators/                    指标测试仍可一算子一测试，不随生产文件强行合并
    ├── execution/                    对应 execution/broker/pricing
    ├── metrics/
    ├── strategy/                     对应 decision/risk/selection/portfolio
    ├── application/
    │   └── plugin_fixtures/              测试动态库夹具
    └── extensions/                       对应 ingest/realtime/mysql/hdf5/talib/telemetry
```

测试文件不以“减少数量”为目标。一项算子或策略规则拥有独立边界用例时，可以继续保留独立测试文件；需要消除的是测试路径对 `crt/imp/internal` 等实现目录的镜像。

### 4.7 合理性结论

| 检查项 | 结论 |
|---|---|
| 一级边界 | 八个一级模块合理，名称都能说明职责，没有新增 `support/utils/internal` 杂物箱 |
| 目录数量 | 若上述目录全部存在，生产树约 26 个目录，较当前 92 个明显收敛；不以 26 作为强制指标 |
| `operators` | 原先简单地放到一个目录不合理；按算法族合并工厂和实现后，既避免约 400 文件平铺，也不制造十几层目录 |
| `strategy` | `decision/risk/selection/portfolio` 四组合理；保留现有业务类型，但删除每类重复的 `crt/imp/logic` 结构 |
| `application` | 增加 `plugins` 是必要的，因为当前确有一组装配文件；它是业务装配边界，不是通用插件框架 |
| `extensions` | 只为当前确有完整源码的能力建目录；HTTP 客户端仅被 telemetry 使用，因此随其移动；不创建只有导入适配的 ClickHouse 空模块 |
| Runtime | 三个 Runtime 只移动到模块根，不拆分、不新增协作者，符合本步“组织文件而非重写对象模型”的范围 |
| 文件合并 | 合并目标主要是约 289 个薄文件和重复工厂样板；大类、复杂算法和测试继续独立，避免形成巨型文件 |
| 公共类型文件 | KRecord、TradeRecord、线程池、策略实现和插件接口等独立概念继续单独成文件；本步不为减少数字强行拼接公共声明 |
| 现有大文件 | `IndicatorImp.cpp`、`ExecutionRuntime.cpp`、`Stock.cpp`、`StrategyRuntime.cpp`、`DataRuntime.cpp` 和 `TableMacro.h` 本步不因行数强拆；它们属于第 8 步职责/API 债务 |
| 风险 | 最大风险是公开 include 路径变化和 unity build；必须逐模块迁移、编译和测试，不能一次移动全部文件 |

因此，这个文件级结构可以作为第 5 步目标，但其中的“合并文件”仍必须经过调用扫描和单个编译单元行数检查。若合并需要改变类职责、方法签名或序列化标识，应立即停止并登记到第 8 步，而不是在本步顺手完成。

## 5. 当前目录到目标目录的物理映射

### 5.1 直接移动

| 当前路径 | 本步目标 | 本步处理 |
|---|---|---|
| `analysis/Performance*`、`ReportExtension*`、`misc*` | `metrics/` | 只移动或按现有职责合并文件，不重写计算接口 |
| `analysis/combinate*` | `operators/` | 只归位，不重写组合模型 |
| `app` 中 Session、初始化和装配文件 | `application/` | 保留现有类型和行为 |
| `common/config`、`common/ini_parser` | `application/` | 归入会话配置所有方，不设计新配置模型 |
| `common/datetime` | `common/time/` | 物理归类 |
| `common/thread` | `common/concurrency/` | 物理归类，重复实现只在有调用证据时删除 |
| `common/db_connect/sqlite` | `common/database/` | SQLite 连接基础设施留在 common；行情 SQLite 驱动进入 `data/storage` |
| `common/serialization` 中的数据类型适配 | 对应 `data/` 或 `execution/` | 序列化文件跟随类型所有方，格式不变 |
| `data/driver` 中默认本地存储 | `data/storage/` | 只移动驱动文件和构建引用 |
| `data/indicator` | `operators/` | 去掉 `crt/imp` 层，按现有算子族归组 |
| `data/factor` | `operators/` | 与时间序列算子归入同一一级模块，接口不变 |
| `data/internal` | `data/` | 文件回到模块根；不创建新的 `runtime` 子目录 |
| `execution/internal` | `execution/` | 文件回到模块根，不拆出新公共类 |
| `execution/cost` | `execution/pricing/` | 只改变所有权路径 |
| `strategy/engine/internal` | `strategy/` | 去掉实现标签目录，保持现有运行逻辑 |
| `strategy/signal`、`condition`、`environment` | `strategy/decision/` | 去掉各自的 `crt/imp` 层 |
| `strategy/moneymanager`、`stoploss`、`profitgoal` | `strategy/risk/` | 去掉各自的 `crt/imp` 层 |
| `strategy/selector`、`multifactor` | `strategy/selection/` | 去掉 `crt/imp/filter/normalize` 的无效层级 |
| `strategy/portfolio`、`allocatefunds` | `strategy/portfolio/` | 合并到同一组合职责目录 |
| `strategy/slippage` | `execution/pricing/` | 只移动现有实现，不改变定价语义 |

### 5.2 移入扩展

| 当前内容 | 本步目标 |
|---|---|
| TDX、Qianlong、CSV 等导入代码 | `extensions/ingest/` |
| IPC、共享内存、数据服务、SpotAgent 和调度服务 | `extensions/realtime/` |
| MySQL 实现 | `extensions/mysql/` |
| ClickHouse/MySQL/HDF5 导入适配 | `extensions/ingest/` |
| HDF5 实现 | `extensions/hdf5/` |
| `data/indicator_talib` | `extensions/talib/` |
| `common/node`、共享内存和实时调度 | `extensions/realtime/` |
| `sysinfo` 中升级/反馈/授权提醒及其专用 HTTP 客户端 | `extensions/telemetry/` |

扩展移动只改变源码位置、target 文件列表和 include。是否删除插件接口、是否改成静态注册、是否改变 ABI 都不在本步处理。

### 5.3 必须先审计再移动

以下目录已有目标方向，但执行前仍要用调用者确认文件级拆分：

- `common/http_client`：当前生产调用者只有 sysinfo 网络功能，整体进入 `extensions/telemetry`
- `common/node`：仅实时调用的文件进入 `extensions/realtime`
- `common/plugin`：进入 `application/plugins`
- `common/serialization`
- `common/db_connect`
- `app/plugin`：通用装配桥进入 `application/plugins`，导入/实时入口进入相应扩展
- `app/runtime`：Agent、调度、IPC 进入 realtime；sysinfo 按本地状态与网络遥测拆成两个文件组

处理规则：先列出生产调用者和构建开关，再按唯一真实所有方移动。若同时被两个核心模块使用，才允许保留在 `common`；若只服务一个扩展，必须随扩展移动。不得为避免判断而复制文件或创建新的 `support/utils` 目录。

## 6. 文件收拢规则

目录收敛后还需要处理文件碎片，但不做接口重设计：

1. 有独立状态、复杂算法或独立测试价值的类，继续单独保留 `.h/.cpp`。
2. 同一类族中仅做对象构造、参数转发的函数，合并到一个复数分类文件。
3. 只有几行模板样板、clone 或注册代码的实现，合并到其业务类族文件。
4. POD、枚举、请求和结果等小型值对象可以按同一业务概念放在一个头文件中。
5. 不把无关算法塞进巨型 `Builtins.cpp`；合并依据是共同职责，不是行数。
6. 一个类很大时，可以按成员函数主题拆多个 `.cpp`，但仍保持一个公共头和一个公共类型。
7. 删除 `all.h`、`build_in.h`、`plugins.h` 等仅做全量包含的聚合头；项目保留单一总入口头 `hayaku.h`。
8. 文件名必须描述内容；`misc`、`inner_tasks`、`build_in` 和已知错误拼写不得进入目标结构。
9. 不因为整理目录新增 `Manager`、`Base`、`Port`、`Context`、`Catalog`、`Processor` 或 `Runtime` 类型。
10. 不设置文件数硬上限；每次合并都要能说明共同职责和测试边界。

## 7. 并行执行方案

本节是第 5 步的实际执行约束。批次 0 已完成，当前正在执行批次 1。

### 7.1 当前真实状态与主要风险

第 4 步已经提交到 `poc` 分支，批次 0 开始时源码工作区干净，仅本文档未跟踪；`application`、`extensions`、`operators`、`metrics` 四个目标一级目录尚不存在。任何 agent 都不得从旧分支取文件覆盖当前成果。

已确认的热点如下：

| 热点 | 当前事实 | 对执行的约束 |
|---|---|---|
| Operators | `data/indicator` 约 401 个文件，`data/factor` 8 个文件，TA-Lib 40 个文件 | 是最大工作量；Indicator 与 Factor 必须协调迁移，不能拆给互不沟通的两个 owner |
| Strategy | 约 332 个生产文件，仍有大量 `crt/imp/logic/optimal` 目录 | 先机械移动，验证后再合并薄工厂，不能把两类变化混在一个 diff 中 |
| `DataRuntime` | 旧路径约有 278 个生产和测试引用 | 是全局路径切换屏障；由一个 owner 移动，各消费方只修改自己目录中的 include |
| `common` | `Parameter`、序列化和部分数据库代码仍依赖 data | 本步只整理物理归属，不能宣称已经完成严格的 `common -> data` 单向依赖 |
| Factor/Indicator | `Factor` 使用 `Indicator`，`FACTOR` 算子又使用 `Factor` | 放在同一个 Operators 总 lane，避免半迁移状态 |
| Execution/Strategy | Strategy 依赖 Execution，Slippage 要从 Strategy 移到 Execution | 两者由同一个 agent 按“Execution 先、Strategy 后”收拢，不让两个 agent 交叉改 pricing |
| 构建系统 | 根 xmake、C++ core/test xmake、pywrap xmake 都引用旧路径 | 只允许 Integration owner 修改；并行 agent 不运行共享构建 |
| 架构检查 | `check_app_dependencies.py`、API inventory 和 CI 仍识别旧目录 | 必须与最终路径原子更新，否则可能构建通过但架构检查失效 |

另外有两个已经定位、应随物理整理处理的问题：

- `KData.cpp` 中未实际使用的 `data/indicator/crt/KDATA.h` include 应在 Operators 迁移时删除，避免形成 `data -> operators` 的假反向依赖。
- `strategy/all.h` 仍引用已经不存在的 `system/build_in.h`；目标结构不保留该聚合头，不能继续转发这个失效入口。

### 7.2 并行模型

使用三个执行 agent 加一个 Integration owner。所有 agent 共享同一个工作区，因此并行依据是**文件所有权互斥**，不是让多个 agent 同时修改同一个目录。并行写入期间禁止运行 xmake；所有构建和测试只在屏障点由 Integration owner 串行执行。

```text
                         ┌──────────────────────────┐
                         │ Integration：共享文件与闸门 │
                         └────────────┬─────────────┘
                                      │
              ┌───────────────────────┼───────────────────────┐
              │                       │                       │
     ┌────────▼────────┐     ┌────────▼────────┐     ┌────────▼────────┐
     │ Agent A          │     │ Agent B          │     │ Agent C          │
     │ Common/Data/算子 │     │ 度量/执行/策略   │     │ 应用/扩展/绑定   │
     └────────┬────────┘     └────────┬────────┘     └────────┬────────┘
              └───────────────────────┼───────────────────────┘
                                      │
                         ┌────────────▼─────────────┐
                         │ 串行构建、测试、结构验收 │
                         └──────────────────────────┘
```

共同规则：

1. agent 只编辑分配给自己的文件；发现目标文件已有其他 agent 修改时立即停止并报告。
2. 移动者负责目标文件，include 由“消费文件的 owner”修改，不能跨 lane 批量替换。
3. 先做纯移动和改 include，通过构建后再合并薄文件；不能一边移动一边重写实现。
4. 不创建临时兼容头、旧目录 shim 或新旧两份实现。
5. 不改公共签名、序列化标识、Python 名称、策略规则、指标公式和执行语义。
6. agent 不自行 `commit`、`push`、rebase 或清理现有修改；每个闸门是否提交由用户明确决定。
7. 对第 4 步已修改的文件，在现有内容上移动和修正，不从 `HEAD` 重新取文件。
8. 传统 include guard 到 `#pragma once` 的转换只在纯移动版本完整通过后进行，避免降低 Git 重命名识别率。

### 7.3 文件所有权

#### Agent A：`common`、`data`、`operators`

独占以下来源和目标：

- `common` 根目录通用工具、`datetime`、`thread`、`db_connect` 根部与 SQLite 实现。
- `data` 根目录、`data/internal`、默认 Driver 契约、Factory 和 SQLite 存储实现。
- `data/indicator`、`data/factor`、`analysis/combinate`，但不含 `data/indicator_talib`。
- 对应的 `test/common`、`test/data`、最终 `test/operators`。
- data 类型的序列化适配。

明确排除并交给 Agent C：

- `common/config`、`common/ini_parser`、`common/plugin`。
- `common/http_client`、`common/node`、`TimerManager.h`。
- `common/db_connect/mysql`、`LruCache.h`。
- IPC、MySQL、HDF5、TDX、CSV、Qianlong、TA-Lib 等扩展实现。
- `KDataShmBufferImp.*`。

`DataDriverFactory.*` 始终由 Agent A 单独持有。Agent C 只能提交扩展后端的新路径清单，不能同时编辑 Factory。

#### Agent B：`metrics`、`execution`、`strategy`

独占以下来源和目标：

- `analysis/Performance*`、`ReportExtension*`、`misc*` 到 `metrics`。
- `execution/**` 到目标 `execution`、`broker`、`pricing`。
- `strategy/slippage/**` 到 `execution/pricing`。
- 其余 `strategy/**` 到模块根、`decision`、`risk`、`selection`、`portfolio`。
- 对应的 `test/analysis`、`test/execution`、`test/strategy`。
- execution 类型的序列化适配。

执行顺序固定为：

```text
execution/internal + execution/cost
    -> strategy/slippage
    -> strategy engine/runtime
    -> decision/risk/selection/portfolio
```

`analysis/combinate.*` 不属于 Metrics，始终由 Agent A 移入 Operators。`StyleRegression.cpp` 继续单独编译，不因目录整理并入 unity build。

#### Agent C：`application`、`extensions`、pybind11/Python 边界

独占以下来源和目标：

- `app/**` 中非 Integration 保留文件。
- `common/config`、`ini_parser`、`plugin` 到 `application` 或 `application/plugins`。
- realtime：`app/runtime`、IPC、共享内存、`common/node`、`TimerManager.h`。
- ingest：CSV、TDX、Qianlong 及现有导入适配。
- mysql：MySQL Driver、`common/db_connect/mysql`、`LruCache.h`。
- hdf5、talib、telemetry 的现有实现和对应测试。
- `hayaku_pywrap/**` 中非 Integration 保留文件，以及仅为验证边界所需的 `hayaku/**` 测试调整。
- `sub_setup.py`、`ingest_setup.py`、`realtime_setup.py` 的边界验收；它们当前没有 C++ 物理路径引用，只要 target/库/包名不变就不修改。

约束：

- 插件接口头和 `PluginIds.h` 由 Agent C 统一归入 `application/plugins`，其他 lane 只更新自己的消费者 include。
- Python 包目录、import 名、`__all__`、绑定名和 wheel 模块名不变。
- `hayaku.ingest` 继续加载 `hayaku_ingest_native.ingest310` 等当前模块名。
- TA-Lib 源码必须等待 Agent A 冻结新的 Indicator 基础路径后再移动，但可与 native operator 算法族合并并行进行。

#### Integration owner：共享文件

下列文件只允许 Integration owner 修改：

- 根目录 `xmake.lua`。
- `hayaku_cpp/src/xmake.lua`、`hayaku_cpp/test/xmake.lua`、`hayaku_pywrap/xmake.lua`。
- `hayaku_cpp/src/hayaku.h`、`hayaku_cpp/src/doc.h`。
- `hayaku_cpp/test/test_main.cpp`、`hayaku_pywrap/main.cpp`。
- `app/HayakuSession.*`、`app/DataRuntimeAssembly.*` 及其直接测试。
- `app/plugin/interface/TMReportPluginInterface.h` 等同时连接 application、metrics、execution 的桥接文件。
- `tools/arch/check_app_dependencies.py`、对应测试、`extract_api_inventory.py`。
- `.github/workflows/**`、`op.sh` 和本文档。
- `hayaku_cpp/demo/**`，因其同时消费多个 lane 的公共头。

各 agent 在屏障点只向 Integration owner 提交三类信息：旧路径到新路径映射、应加入或删除的源文件列表、仍存在的跨 lane include。Integration owner 不顺手重写各 lane 的业务实现。

### 7.4 执行批次与屏障

#### 批次 0：冻结第 4 步基线，串行

开始任何移动前必须完成：

1. 明确第 4 步尚未验收的 realtime target、后端矩阵等项目，是先补齐还是作为已登记债务带入第 5 步。
2. 保存 `git status --short` 和完整 diff，确认所有现有修改的归属。
3. 重新记录目录、文件、行数和薄文件数量。
4. 冻结 C++/Python API inventory、序列化结果、代表性回测结果和 Python import 副作用。
5. 完成一次共享构建和当前测试；基线失败时，第 5 步不开始。

基线命令：

```bash
./op.sh configure shared
./op.sh build
./op.sh small-test
./op.sh unit-test
${HOME}/.local/bin/xmake -b plugin-abi-test
${HOME}/.local/bin/xmake r plugin-abi-test
./op.sh import-test
./op.sh python-test
python3.10 -m unittest discover -s tools/arch -p 'test_*.py'
python3.10 tools/arch/check_app_dependencies.py
python3.10 tools/arch/extract_api_inventory.py --output /tmp/step5-api-inventory-before.md
git diff --check
```

当前最近一次可参考的验收下限为：`unit-test` 805/805、208450 条断言，plugin ABI 7 个用例/56 条断言，SpotAgent 1 个用例/14 条断言，Python 主入口 64/64，ingest 边界 5/5。正式执行时以批次 0 的重新运行结果为准。

#### 批次 1：只读映射，三个 agent 并行

- Agent A 生成 Common/Data/Operators 的逐文件映射。
- Agent B 生成 Metrics/Execution/Strategy 的逐文件映射。
- Agent C 生成 Application/Extensions/pybind11 的逐文件映射。
- Integration owner 解决同名目标、交叉序列化文件、插件桥和构建开关冲突。

退出条件：

- 每个现有生产文件恰好有一个 owner 和一个目标路径。
- 不存在两个源文件覆盖同一目标文件。
- 每个计划合并的文件都列出原文件、目标文件、共同职责和对应测试。
- 所有公共头移动都列出消费 lane；不存在“移动者顺手全局替换”的隐含工作。

逐文件映射和执行结果追加在本文档第 10 节；不再新增另一份进度文档。

#### 批次 2：基础与执行路径的纯移动，三个 agent 并行

Agent A：

1. `datetime -> common/time` 与 `thread -> common/concurrency`。
2. 在 time/concurrency 路径稳定后整理 `common/database`。
3. 将 `DataRuntime` 移到 data 根，并更新自己拥有的消费文件。
4. 整理 data 核心和默认 `data/storage`；暂不移动 operators。

Agent B：

1. `Performance/ReportExtension/misc -> metrics`，`combinate` 留给 Agent A。
2. `execution/internal -> execution`。
3. `execution/cost` 与 `strategy/slippage -> execution/pricing`。
4. 只修正 Agent B 文件中的 Common/Data/Execution 新路径。

Agent C：

1. 移动 application 核心、配置和 plugins 基础设施。
2. 建立 ingest、realtime、mysql、hdf5、telemetry 的目标目录并移动已确认文件。
3. TA-Lib 和 pybind11 暂不切换，等待 Operators 与所有公共路径稳定。
4. 只修正 Agent C 文件中的 Common/Data/Application 新路径。

到达屏障后所有 agent 停止写入。Integration owner 更新共享入口、xmake、架构扫描器和 CI，然后重新 configure：

```bash
./op.sh configure shared
./op.sh build
./op.sh small-test
python3.10 -m unittest discover -s tools/arch -p 'test_*.py'
git diff --check
```

构建未恢复前不得进入批次 3，也不得通过增加兼容头绕过失败。

#### 批次 3：Operators、Strategy 与 Extensions 的纯移动，三个 agent 并行

Agent A：

1. 先移动 Indicator 核心类型、Factor、FactorSet、FactorStore 和 `analysis/combinate`。
2. Factor 与 `FACTOR` 算子由同一 owner 连续处理。
3. 再按目标算法族移动 native operator 源码和测试，但暂不合并实现。

Agent B：

1. 将 Strategy engine/runtime 移到模块根。
2. 按 `decision/risk/selection/portfolio` 移动生产文件和测试。
3. 处理同名 `test_export.cpp`，按业务重命名，禁止相互覆盖。
4. 删除目标结构不再保留的 `all.h/build_in.h/buildin_*.h` 引用。

Agent C：

1. 在 Indicator 新基础路径稳定后移动 TA-Lib。
2. 完成 extension 消费方 include 和对应测试路径。
3. 更新非共享 pybind11 文件中的 include；Python 可见注册保持不变。

到达第二个屏障后，由 Integration owner 更新共享文件并运行：

```bash
./op.sh configure shared
./op.sh build
./op.sh small-test
./op.sh unit-test
${HOME}/.local/bin/xmake -b plugin-abi-test
${HOME}/.local/bin/xmake r plugin-abi-test
./op.sh import-test
```

此时要求“纯移动版本”完整通过。未通过之前，禁止开始薄文件合并。

#### 批次 4：薄文件收拢，按目标文件互斥并行

三个 agent 先完成自己模块内第 4 节已批准的合并；谁先完成，谁可以在 Integration owner 重新分配后帮助 Operators，但不得临时自行接管文件。

Operators 的再分工固定到目标文件族：

| Owner | 独占目标文件族 |
|---|---|
| Agent A | Indicator/Factor 核心、`ScalarMathOperators`、`TrigonometricOperators`、Series、Boolean/Event、`OperatorCombinations` |
| Agent B | Window/Position/Rolling、Moving/Adaptive Average、Statistics/Relations/Regression |
| Agent C | Momentum/Trend/Oscillator、Risk、Market、TA-Lib 适配 |

规则：

- 每个 `*Operators.h` 和配套 `.cpp` 只允许一个 owner。
- 不建立全局 `Builtins.cpp`；超过约 1,000 行时按同一算法族拆 `.cpp`。
- Strategy 中仅合并第 4 节指定的工厂、表达式和 runners；有状态策略实现继续单独成文件。
- Execution 仅合并 TradeCost、Slippage 的薄工厂/样板；不合并 Runtime、Engine、Port 或 Record 类型。
- Application 的插件接口继续按独立职责保留；不为了减少文件数合并成一个 `Plugins.h`。
- 每完成一个文件族，先编译再处理下一个文件族，便于定位等价性问题。
- 所有纯移动和文件族合并通过后，机械移除生产头最外层 `#ifndef/#define/#endif`，统一为单一 `#pragma once`；单独执行构建和测试后再进入批次 5。

#### 批次 5：构建、测试与 Python 边界收口

并行写入结束后，Integration owner 串行完成：

1. 统一更新四份 xmake、安装头排除项、unity 分组和 coverage 排除路径。
2. 更新 `hayaku.h`、`doc.h`、`test_main.cpp`、pywrap `main.cpp`。
3. 更新架构检查器、API inventory 路径和 CI。
4. 确认 `small-test` 没有因显式 glob 失效而漏收测试。
5. 确认 optional ingest、TA-Lib 和 plugin fixture 没有意外进入 core。
6. Agent C 只做最后的 pybind include 修正和 Python 边界测试，不移动 Python 包。

#### 批次 6：删除旧结构并完整验收

- 删除已经为空的旧目录、聚合头、旧路径和任何临时文件。
- 扫描 `internal/imp/crt/logic/optimal/support/utils` 残留。
- 扫描源码、测试、demo 和 pywrap 的旧 include 前缀。
- 对比 API inventory、序列化结果、代表性回测结果和 Python import 副作用。
- 填写本文档第 10 节的实际结果；不为达到目录或文件数字继续强行合并。

结构扫描：

```bash
find hayaku_cpp/src -mindepth 1 -maxdepth 1 -type d | sort

find hayaku_cpp/src hayaku_cpp/test -type d \
  \( -name internal -o -name imp -o -name crt -o -name logic \
     -o -name optimal -o -name support -o -name utils \) -print

grep -RInE '#[[:space:]]*include.*(app/|analysis/|data/internal/|data/indicator/|data/factor/|execution/internal/|execution/cost/|strategy/engine/internal/|strategy/slippage/)' \
  hayaku_cpp/src hayaku_cpp/test hayaku_cpp/demo hayaku_pywrap

python3.10 tools/arch/extract_api_inventory.py --output /tmp/step5-api-inventory-after.md
git diff --check
```

第二条和第三条命令最终必须无输出；第一条必须只列出第 5 步规定的八个一级模块。

完整回归：

```bash
./op.sh configure shared
./op.sh build
./op.sh small-test
./op.sh unit-test
${HOME}/.local/bin/xmake -b hayaku-ingest
${HOME}/.local/bin/xmake -b ingest
${HOME}/.local/bin/xmake -b plugin-abi-test
${HOME}/.local/bin/xmake r plugin-abi-test
./op.sh import-test

PYTHON_BIN=/opt/homebrew/opt/python@3.10/bin/python3.10
${PYTHON_BIN} -m unittest \
  hayaku.test.test_api_boundary \
  hayaku.test.test_ingest_boundary \
  hayaku.test.test_session
./op.sh python-test
```

最小构建矩阵：

```bash
${HOME}/.local/bin/xmake f -c -k shared -y --feedback=n \
  --mysql=n --hdf5=y --sqlite=y --tdx=y --ta_lib=y
${HOME}/.local/bin/xmake -b core
${HOME}/.local/bin/xmake r small-test

${HOME}/.local/bin/xmake f -c -k shared -y --feedback=n \
  --mysql=n --hdf5=n --sqlite=y --tdx=n --ta_lib=n
${HOME}/.local/bin/xmake -b core
${HOME}/.local/bin/xmake r small-test
```

本机具备 MySQL 依赖时再补 `mysql=y`；否则交给 CI。每组矩阵必须重新执行 `xmake f -c`，不能复用含旧源路径的缓存。

### 7.5 失败处理与回退

1. 每个屏障保留可审查 diff 和“已移动文件清单”；是否创建 checkpoint commit 由用户决定。
2. 纯移动阶段失败，只修路径、文件列表和 include，不顺手改算法或对象职责。
3. 合并阶段失败，回退当前文件族的合并，保留已经验证通过的物理移动。
4. optional backend 失败时先验证构建开关和源文件归属，不能把扩展实现重新塞回 core。
5. API、序列化或回测结果变化时立即停止，该变化转入第 8 步设计，不能解释成“目录整理副作用”。
6. 一个 lane 未通过自己的结构扫描，不进入共享构建闸门。

### 7.6 预计时间

以下为三个 agent 并行、当前依赖已在本机缓存且第 4 步基线可通过时的墙钟时间，不是三个 agent 工时相加：

| 阶段 | 预计时间 |
|---|---:|
| 批次 0：基线冻结 | 30～60 分钟 |
| 批次 1：逐文件映射与冲突消解 | 30～60 分钟 |
| 批次 2：基础、Data、Execution、Application 纯移动 | 1.5～2.5 小时 |
| 批次 3：Operators、Strategy、Extensions 纯移动 | 1.5～3 小时 |
| 批次 4：有依据的薄文件收拢 | 2～4 小时 |
| 批次 5～6：集成、矩阵与完整验收 | 1.5～2.5 小时 |

目标关键路径为 **7～13 小时**。如果只做到目标目录、暂不合并约 400 个 indicator/factor 文件，可在约 **4～7 小时**得到第一个完整可构建版本；但这不算第 5 步全部完成。最可能拉长时间的是 Operators 合并后的编译错误、optional backend 构建矩阵和全量测试，而不是目录创建本身。

## 8. 验收标准

### 8.1 结构

- [x] `src` 一级目录只有八个：`common`、`data`、`operators`、`execution`、`metrics`、`strategy`、`application`、`extensions`。
- [x] 不存在只表达实现手法的目录：`internal`、`imp`、`crt`、`logic`、`optimal`、`support`、`utils`。
- [x] 目录深度不超过 `src/<module>/<submodule>/<file>`。
- [x] 没有空目录、长期转发目录或仅包含一个聚合头的目录。
- [x] 每个保留目录都能用一句话说明唯一职责。
- [x] `test` 按八个一级模块镜像组织，不镜像实现细节。
- [x] 可选后端和运行服务全部位于 `extensions`。

### 8.2 文件

- [x] `misc`、`inner_tasks`、`build_in` 等含糊文件名已消失。
- [x] 已确认的拼写错误文件名已修正；涉及公共类型的改名已明确登记到第 8 步，而不是半改。
- [x] 薄文件仅在职责一致时合并，没有产生新的巨型杂物文件。
- [x] 有独立状态、算法和测试价值的类型没有为了减少数量被强行合并。
- [x] 所有移动文件都有唯一目标，不存在新旧两份实现。
- [x] 本步范围内的生产头只使用单一 `#pragma once`，不存在最外层传统 include guard。

### 8.3 行为与构建

- [x] 除 include 路径外，公共 C++ 类型和函数签名保持不变。
- [x] Python 包仍使用当前 import 名，Python 可见 API 保持不变。
- [x] 序列化标识和已有数据格式保持不变。
- [x] 指标、策略、交易和复权黄金结果与基线一致。
- [x] 当前支持的核心与扩展构建组合通过。
- [x] `small-test`、相关 C++ 单元测试和 Python 3.10 核心测试通过。
- [x] `git diff --check` 通过。

## 9. 后续步骤

第 5 步完成后再依次处理：

### 第 6 步：Python 绑定层与包结构收敛

- `hayaku_pywrap` 按八个 C++ 业务域重组，保留 `core/ingest/realtime` 三个原生模块边界。
- `hayaku` 按 Python 用户接口重组，清除 `data` 中导入实现与公共数据门面的职责冲突。
- 冻结 30 项顶层 API 和 pybind 导出集合，不把物理移动变成接口变更。
- 详细目标结构与边界见 `docs/arch/refactor/step-6-progress.md`。

### 第 7 步：C++ 质量基线与无效文件清理

- 一次性对齐 Google 格式，建立 clang-tidy 和 ASan 的可重复检查入口。
- 审计无效配置与文件，删除前核对构建、打包和文档消费者。
- 目标、边界和验收条件见 [第 7 步](step-7-progress.md)。

### 第 8 步：接口收敛

- 评估 `StockManager`、`DataRuntime` 是否直接收拢到 `DataEngine`。
- 评估 `TradeManagerBase`、`ExecutionRuntime` 是否直接收拢到 `ExecutionEngine`。
- 评估 `System`、`StrategyRuntime` 是否直接收拢到 `StrategyEngine`。
- 删除重复接口和无独立价值的 Base/Manager/Port，而不是增加一批内部服务类。
- 处理 `Stock/KData/KQuery` 等公共类型的最终职责命名。

### 品牌前置批次（已完成）

- 顶层源码目录、Python 包、C++ namespace、宏前缀、库、target、include、CLI、配置目录、
  插件 ABI 和文档已统一为 `hayaku`。
- 未保留旧 namespace alias、旧动态库、转发头或 Python shim package。
- 该批次在第 5 步完成验收后独立执行，没有混入第 5 步数百个文件的物理移动。

## 10. 完成后记录

### 10.1 批次 1 映射审计

- Agent A 覆盖 Common/Data/Operators；Agent B 覆盖 392 个生产文件和 48 个测试文件；Agent C 覆盖约 152 个生产文件及对应绑定消费者。
- 生产目标冲突只有聚合头重名和 macOS 大小写冲突：`Factor.h/FACTOR.h`、多个 `build_in.h`；已分别决定为 `FactorOperator.h` 中间名和最终复数分类头。
- 测试冲突为 `test_Factor.cpp/test_FACTOR.cpp` 以及 risk 下两个 `test_export.cpp`；纯移动时按业务重命名，不合并用例。
- 已补入目标树的现实文件：`ResourceVersionTraits.h`、`OperatorValueSelector.*`、`RealtimeExport.h`；删除不存在的 Backtest plugin 占位。
- `common/Config.h` 是 `config_utils.h.in` 生成物，不做 `git mv`；Integration owner 原子更新生成文件名及消费者。
- `spot_generated.h` 当前没有生成规则；纯移动闸门必须补齐 flatc 规则后才删除已提交的生成头。
- B 范围有 109 处 `BOOST_CLASS_EXPORT` 注册；合并时必须保证每个类型仅一处，且序列化字段和顺序不变。
- 确认可删候选：`FilterNode.h`、`LRUCache11.h`、`snowflake.h`、Hybrid/TLS 资源池变体；仅在最小构建矩阵通过后实际删除。

### 10.2 批次 2 执行记录

- Common 的时间、并发和数据库基础设施已分别归入 `common/time`、`common/concurrency`、`common/database`；DataRuntime 和默认存储驱动已归入 `data`、`data/storage`。
- Metrics 已独立；Execution runtime 已扁平到 `execution`，交易成本与滑点已统一归入 `execution/pricing`。
- Application、plugins、ingest、realtime、MySQL、HDF5 与 telemetry 已完成纯移动；TA-Lib 按屏障要求留到批次 3。
- `spot_generated.h` 已从源码树删除，xmake 使用 flatc 由 `spot.fbs` 生成到 target autogen 目录，并通过 public include 目录供 realtime 和测试消费。
- `common/Config.h` 的生成文件名和消费 include 已统一；macOS 大小写不敏感文件系统上的旧生成文件已规范化。
- 屏障结果：重新 configure 通过，core 与 Python extension 构建通过，架构扫描 525 个 C/C++ 文件无 application 反向依赖，`small-test` 51/51、3356/3356 通过，`git diff --check` 通过。
- 编译屏障发现 `KData.cpp` 仍直接使用 `OPEN/CLOSE` 工厂，故在 Operators 尚未迁移前保留 `KDATA.h` include；最终由批次 3 按新路径收口，不能提前删除。

### 10.3 批次 3 执行记录

- Operators 完成 411 个生产文件、149 个测试的一一纯移动；`Factor.h/FACTOR.h` 的大小写冲突通过中间名 `FactorOperator` 消除，`combinate` 改为 `OperatorCombinations`。
- Strategy 完成 311 个生产文件、39 个测试的纯移动，最终只保留根、`decision`、`risk`、`selection`、`portfolio`；四个 `test_export.cpp` 已按业务重命名，109 个 `BOOST_CLASS_EXPORT` 注册保持完整且无重复。
- TA-Lib 的 40 个生产文件与 41 个测试已归入 `extensions/talib`；pybind11 仅更新 include，Python 注册名和模块名未改。
- xmake 已切到八个一级模块和新测试镜像；`StyleRegression.cpp` 及其测试继续单独编译。
- `spot.fbs` 由 realtime target 在编译前生成到 `build/autogen/extensions/realtime`，源码树不再保留生成头。
- 第二屏障结果：configure、core 与 Python extension 构建通过；`small-test` 51/51、3356/3356；`unit-test` 805/805、208437/208437；plugin ABI 7/7、56/56；Python 3.10 import 正常。
- 当前沙箱禁止 NNG IPC/TCP listener（`NNG_EPERM`）；SpotAgent 回调测试只在该明确错误下记录 warning 并返回，允许 listener 的环境仍执行完整异步回调、停止、join 与重启断言。

### 10.4 批次 4 执行记录

- Operators 按 Series、Scalar、Trigonometric、Boolean/Event、Window、Statistics、Momentum、Risk、Market 等职责收拢；超过约 1,000 行的实现按同族拆分，最大常规聚合实现文件为 957 行，`StatisticalMoments.cpp` 作为单一统计族保留为 1,689 行。
- Execution 的交易成本和滑点薄工厂分别收拢到 `TradeCosts`、`SlippageModels`；Strategy 的 runners、表达式、工厂和薄策略族按 `decision/risk/selection/portfolio` 收拢，有状态实现、Base、Record、Engine、Runtime、Port 与 `StyleRegression` 保持独立。
- TA-Lib 适配最终收敛为 7 个生产文件；C API 调用显式使用全局命名空间，避免聚合后与 `hayaku::TA_*` 公共工厂重名。
- `SystemInfo` 只保留本地版本与 Python/Jupyter 状态；联网升级检查、反馈和授权提醒迁入 `extensions/telemetry/Telemetry`，公共函数名与签名不变。
- 109 个 Strategy/Execution 序列化导出注册全部保持唯一；全量 API inventory 的 363 个规范化条目与执行前完全一致。
- 339 个最终生产头均恰好包含一个 `#pragma once`；机械移除 276 组传统保护宏，保留当时仍需使用的功能条件块。

### 10.5 批次 5～6 执行记录

- 删除无引用的 `FilterNode.h`、`LRUCache11.h`、`snowflake.h` 和四个 Hybrid/TLS 资源池变体；删除 `operators/build_in.*`、`strategy/all.h`、`serialization/all.h`，消费者改用职责明确的分类头。
- 删除所有空旧目录；最终 `src` 只保留八个一级模块，`src/test` 中 `internal/imp/crt/logic/optimal/support/utils` 扫描为 0，旧 include 前缀扫描为 0，源码目录深度不超过三层。
- `spot_generated.h` 只由 flatc 生成到 `build/autogen`，不再作为源码或 `#pragma once` 整理对象。
- 完整功能矩阵（HDF5/SQLite/TDX/TA-Lib）通过 core 构建和 51 个 small-test；最小矩阵（SQLite-only）通过 core 构建和 7 个后端无关 small-test。为此移除了测试入口对 HDF5 的硬编码限制，并在 HDF5 关闭时只装配后端无关的小测试集。
- 最终默认配置已恢复并重建；native ingest、Python ingest、plugin ABI、Python import、边界测试和完整 Python 测试均通过。

### 10.6 最终验收结果

- `small-test`：51/51 用例、3356/3356 断言。
- `unit-test`：805/805 用例、208437/208437 断言；受限沙箱中的 SpotAgent listener 仅对 `NNG_EPERM` 记录已登记 warning。
- plugin ABI：7/7 用例、56/56 断言。
- Python 3.10 边界测试：21/21；完整 Python 测试：66/66；import 为 Python 3.10.21 / hayaku 2.8.2。
- API inventory：执行前后均为 21 个 reviewed classes、274 个 C++ public methods、888 个 pybind exports、12 个包含 star export 的 Python 文件；去除源码路径/行号后的 363 个条目完全一致。
- 现有指标、策略、执行、序列化与组合回归均包含在 805 个 C++ 用例和 66 个 Python 用例中并通过；本步骤未建立额外的独立回测快照。
- `git diff --check`、架构依赖检查、结构扫描和传统头保护宏扫描均通过；工作区未暂存、未提交。

最终统计：

| 指标 | 执行前 | 执行后 | 结论 |
|---|---:|---:|---|
| `src` 一级目录数 | 6 | 8 | 达到目标 |
| `src` 总目录数 | 92 | 27（含 `src` 根） | 只保留业务模块和必要子模块 |
| `.h/.cpp` 文件数 | 1,126 | 590 | 基于职责合并，无按数字强并 |
| 不超过 30 行的文件数 | 288 | 96 | 薄文件明显减少 |
| 禁止目录数 | 34 | 0 | 达到目标 |
| 空目录/转发目录 | 0 | 0 | 达到目标 |
| 构建结果 | configure、core、Python extension 通过 | 默认配置、两组矩阵、ingest 与 Python extension 全部通过 | 通过 |
| 测试结果 | small 51/51；unit 804/805（已登记 1 个 realtime 夹具基线失败）；plugin ABI 7/7；Python 66/66 | small 51/51；unit 805/805；plugin ABI 7/7；Python 边界 21/21、完整 66/66 | 无新增失败，修复基线夹具 |
| 回测结果 | 待冻结 | 现有指标/策略/执行/组合回归全部通过；无独立快照 | 本步无行为差异证据 |

各批次的变更范围、删除文件、测试命令和失败处理已记录；本次未获授权创建 checkpoint commit，因此保持工作区未暂存、未提交。
