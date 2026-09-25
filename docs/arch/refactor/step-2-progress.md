# 第 2 步完成报告：ExecutionEngine、StrategyEngine 与 Python API 收口

> 状态：已完成（窄接口、兼容适配、Session 生命周期、Python 命名空间和全量回归均已验收）
>
> 前置阶段：[第 1 步：DataEngine 与三引擎边界](step-1-progress.md)
>
> 执行日期：2026-09-25；核心实现与验收约 24 分钟（17:49～18:13），三条工作线并行开发、统一集成。

> 后续决策（2026-09-25）：本阶段是过渡架构，不是最终架构。最终必须迁移真实实现、删除兼容层和
> 重组目录，详见[第 3 步：最终架构冻结与旧体系退役](step-3-progress.md)。

## 1. 阶段结论

原计划剩余的三项工作已合并为一个阶段完成：

```text
┌────────────────────────────────────────────────────────────┐
│ HikyuuSession                                              │
│ DataEngine / ExecutionEngine / StrategyEngine 生命周期    │
└───────┬────────────────────┬───────────────────────┬───────┘
        ▼                    ▼                       ▼
┌──────────────┐    ┌──────────────────┐    ┌──────────────────┐
│ DataEngine   │    │ StrategyEngine   │    │ ExecutionEngine  │
│ 只读市场数据 │    │ run/stop/result  │    │ submit/snapshot  │
└──────────────┘    └────────┬─────────┘    └────────┬─────────┘
                             ▼                       ▼
                    ┌─────────────────┐     ┌─────────────────┐
                    │ System          │     │ TM Adapter      │
                    │ 兼容运行后端    │     │ 兼容执行适配    │
                    └────────┬────────┘     └────────┬────────┘
                             └──────────┬─────────────┘
                                        ▼
                              ┌──────────────────┐
                              │ TradeManager     │
                              │ 成熟账本/Broker  │
                              └──────────────────┘

Python：hikyuu.data / hikyuu.execution / hikyuu.strategy
兼容层：StockManager / TradeManagerBase / System 继续可用
```

完成后的边界：

- `DataEngine` 负责只读市场数据；
- `ExecutionEngine` 只负责订单执行、账户快照和成交历史；
- `StrategyEngine` 只负责一次策略运行、结果快照和协作停止；
- `HikyuuSession` 负责三个 Engine 的所有权和关闭失效，不承载交易或策略逻辑；
- Python 普通入口按领域模块组织，SPI 和低层运维接口分别进入 `hikyuu.spi` 和
  `hikyuu.advanced`；
- 旧类保留为构造期依赖和兼容后端；绑定完成后的运行期代码只调用 Engine 的窄接口。

## 2. 工作项完成情况

| 编号 | 工作项 | 状态 | 验收证据 |
| --- | --- | --- | --- |
| 2.1 | 固定三个 Engine 的最小接口和所有权 | 已完成 | 三个窄门面、强类型输入输出、依赖扫描 |
| 2.2 | 实现 ExecutionEngine 和兼容适配 | 已完成 | 5 个专项 C++ case，逐笔交易与账户状态对照 |
| 2.3 | 实现 StrategyEngine 和 System 兼容适配 | 已完成 | 4 个专项 C++ case，旧 System 金标逐笔一致 |
| 2.4 | 建立 Python 稳定命名空间和绑定 | 已完成 | 8 个 API boundary 测试、三个领域模块 |
| 2.5 | 接入 HikyuuSession 三引擎生命周期 | 已完成 | C++/Python 打开、绑定、幂等、关闭失效测试 |
| 2.6 | 更新 API inventory 和双语文档 | 已完成 | inventory 可重复生成，中英文文档同步 |
| 2.7 | 全量构建、测试、性能和阶段验收 | 已完成 | Release、small、unit、Python 3.10、import 全通过 |

## 3. 执行前后对比

| 对比项 | 执行前 | 执行后 | 结论 |
| --- | --- | --- | --- |
| 普通交易入口 | 面向 `TradeManagerBase` 的 75 个方法 | `ExecutionEngine` 仅 `submit/snapshot/history` | 新业务代码不再依赖账本内部操作集合 |
| 普通策略入口 | 面向 `System` 的 53 个方法 | `StrategyEngine` 仅 `run/stop/running` | 编排入口和组件装配接口分离 |
| 运行输入输出 | 多参数调用和可变内部对象 | `OrderRequest/ExecutionReport`、`StrategyConfig/BacktestResult` | 边界采用强类型和值快照 |
| Session | 只有 DataEngine | 持有并约束三个 Engine | 关闭后所有 scoped Engine 统一失效 |
| Engine 地址 | 重绑定语义不明确 | 同对象重绑幂等，不同对象拒绝 | 不会因替换 `unique_ptr` 产生悬空引用 |
| Python 领域入口 | 主要依赖顶层和旧子包 | 新增 `data/execution/strategy/spi/advanced` | 新代码有明确导入路径 |
| Python wildcard | 无顶层白名单，辅助名可能意外泄漏 | `__all__` 为冻结白名单，`dir()` 同步使用 | 新绑定不会自动成为公共 API |
| Python 当前名称 | 非下划线全局名 856 个 | 白名单批准 790 个，隐藏 66 个内部/辅助名 | 保留兼容面的同时阻止继续失控 |
| API inventory | 16 类、404 方法、998 个绑定声明 | 23 类、449 方法、1033 个绑定声明 | 新增 7 个领域类型并纳入审计；不是旧大类继续膨胀 |
| C++ unit-test | 815 case、209153 assertion | 826 case、209218 assertion | 新增 11 case、65 assertion |
| Python 测试 | 47 个 | 56 个 | 新增领域导入、白名单和三引擎生命周期测试 |

inventory 总数增加的原因是把 Engine 及其值类型正式纳入扫描。真正面向普通代码的三个核心门面
合计为 `DataEngine` 26 个、`ExecutionEngine` 4 个、`StrategyEngine` 4 个已扫描公开成员；旧大类
没有新增业务方法。

## 4. 三条工作线结果

### 4.1 ExecutionEngine

新增：

```text
hikyuu_cpp/hikyuu/trade/OrderRequest.h
hikyuu_cpp/hikyuu/trade/ExecutionReport.h
hikyuu_cpp/hikyuu/trade/AccountSnapshot.h
hikyuu_cpp/hikyuu/trade/ExecutionEngine.h/.cpp
hikyuu_cpp/hikyuu/trade/internal/TradeManagerExecutionAdapter.h/.cpp
hikyuu_pywrap/trade/_ExecutionEngine.cpp
hikyuu_pywrap/trade/trade_main.cpp
hikyuu_cpp/unit_test/hikyuu/trade/test_ExecutionEngine.cpp
```

设计结果：

- 构造时必须显式提供 `TradeManagerPtr`，不偷偷创建默认账户；
- `OrderSide` 覆盖买入、卖出、卖空和平空；
- 正常拒单使用 `ExecutionStatus::REJECTED`，参数或生命周期错误仍使用异常；
- 账户查询返回值拥有的 `AccountSnapshot`，不暴露内部容器；
- 适配器复用现有校验、费用、Broker 和记账语义，专项测试与 `TradeManager` 结果一致；
- ExecutionEngine 不依赖 StrategyEngine 或具体策略组件。

### 4.2 StrategyEngine

新增：

```text
hikyuu_cpp/hikyuu/trade_sys/engine/StrategyConfig.h
hikyuu_cpp/hikyuu/trade_sys/engine/BacktestResult.h/.cpp
hikyuu_cpp/hikyuu/trade_sys/engine/StrategyEngine.h/.cpp
hikyuu_pywrap/trade_sys/_StrategyEngine.cpp
hikyuu_cpp/unit_test/hikyuu/trade_sys/engine/test_StrategyEngine.cpp
```

设计结果：

- `StrategyConfig` 只包含 `KData` 和 reset 选项；
- `BacktestResult` 复制证券、查询和成交记录，后续 reset 不会改变已有结果；
- `run` 保持同步语义，`stop` 通过轻量原子标记在 Bar/WalkForward 安全边界协作停止；
- 取消标记只在 StrategyEngine 包装的线程内生效，直接 `System::run` 行为不变；
- 新旧路径逐笔成交记录完全一致，原 System、Portfolio 和风控金标继续通过。

### 4.3 Python API 与 Session

新增或收口：

```text
hikyuu/data/__init__.py
hikyuu/execution/__init__.py
hikyuu/strategy/__init__.py
hikyuu/spi/__init__.py
hikyuu/advanced/__init__.py
hikyuu/_public_api.py
hikyuu/test/test_api_boundary.py
docs/zh/python_api.rst
docs/en/python_api.rst
```

`HikyuuSession` 新增 `bindExecution/bindStrategy`、`execution/strategy` 和状态查询。绑定采用一次绑定
规则：同一个账户或 System 重复绑定幂等，不同对象会被拒绝；关闭时请求策略停止并等待退出，再
使三个 Engine 句柄失效。Python 对应使用 snake_case 名称。

顶层显式旧属性暂不删除，保证 `hikyuu.MA`、`hikyuu.System` 等既有代码继续运行；但
`from hikyuu import *` 和交互补全现在受明确白名单约束。测试中原来依赖顶层意外泄漏的 `np`
已改为显式 `import numpy as np`。

## 5. 验收结果

环境：macOS arm64、Release shared build、Python 3.10.21、xmake 3.0.8。

| 验证项 | 命令/方法 | 结果 |
| --- | --- | --- |
| Release 构建 | `./op.sh build` | 通过，生成 `libhikyuu.dylib` 和 `core310.so` |
| small-test | `./op.sh small-test` | 41/41 case，3288/3288 assertion |
| unit-test | `./op.sh unit-test` | 826/826 case，209218/209218 assertion |
| Python 3.10 | `./op.sh python-test` | 56/56 通过 |
| import | `./op.sh import-test` | Python 3.10.21、Hikyuu 2.8.2 通过 |
| API inventory | `python3.10 tools/arch/extract_api_inventory.py` | 23 类、449 方法、1033 个绑定声明 |
| 策略金标 | StrategyEngine 与直接 System 对照 | 证券、查询和逐笔成交记录完全一致 |
| 执行金标 | ExecutionEngine 与绑定 TradeManager 对照 | 买卖、卖空、拒单、资金、持仓和历史一致 |
| Session | C++/Python 生命周期用例 | 绑定幂等、异对象拒绝、关闭后访问拒绝 |
| 格式检查 | `clang-format`、`git diff --check` | 通过 |

### 5.1 性能

Release 模式、Python 3.10，预热后取 21 组中位数：

| 热路径 | 直接旧入口 | 新 Engine | 变化 |
| --- | ---: | ---: | ---: |
| 2836 Bar 策略运行 | 1.165875 ms | 1.173375 ms | +0.64% |
| 每笔同步买单，合计 2100 笔 | 1.885 us | 1.705 us | -9.53% |

两条代表性路径均满足“不回退超过 5%”。下单对照中 Engine 复用了预构造 `OrderRequest`，因此该
数据用于确认门面没有额外热路径负担，不表示底层记账算法本身加速了 9.53%。

## 6. 兼容限制与未做事项

本阶段完成的是公共边界收口，不是一次性重写成熟内核：

- `TradeManagerBase` 的物理账本尚未迁入全新的 Ledger；当前由内部适配器复用成熟实现；
- `System` 的逐 Bar 状态机尚未物理搬入 StrategyEngine；当前作为经过金标验证的兼容运行后端；
- 旧 C++/Python 显式入口尚未删除，也没有立即发出弃用警告；
- pybind11 `.pyi` 按仓库约定应在发布流程使用 `pybind11-stubgen` 统一重新生成，本阶段不手写生成文件；
- 未修改指标算法、策略组件决策、序列化格式、数据库 Schema 和 Broker 协议。

这些是第 2 步的兼容迁移策略，不影响新边界使用。后续决策已确定最终版本不保留兼容层；旧能力
必须先迁入新 Runtime，随后删除旧类型、旧绑定和旧目录。删除顺序与验收条件以第 3 步文档为准。

## 7. 阶段判定

三条并行工作线均已完成并通过统一验收。运行期调用入口已经收敛为：

```text
HikyuuSession
├── DataEngine
├── ExecutionEngine
└── StrategyEngine
```

创建账户和装配策略时仍需通过现有工厂提供 `TradeManagerPtr` 与 `SystemPtr`；这两个成熟实现当前是
Engine 的兼容后端，而不是普通运行期调用入口。

**第 2 步已完成，但它只是窄门面过渡阶段。第 3 步将完成真实实现迁移、兼容层删除和目录重组，
达到最终三引擎架构。**
