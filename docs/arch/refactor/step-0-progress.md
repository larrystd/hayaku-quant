# 第 0 步进度：API 清单与行为基线

> 对应总体方案：[接口收敛与边界重构方案](../refactor.md)

## 1. 阶段目标

在修改生产接口之前，完成两项基础工作：

1. 建立 C++、pybind11 和 Python 对外接口清单，明确 Public API、Extension SPI、Internal API 和 Deprecated API；
2. 固定当前回测、下单、资金和持仓行为，为后续重构提供可自动验证的基线。

本阶段不拆分 `StockManager`、`TradeManagerBase` 或 `System`，不删除接口，也不改变回测行为。

## 2. 总体状态

| 项目 | 当前值 |
| --- | --- |
| 阶段状态 | 未开始 |
| 整体进度 | 0% |
| 预计工期 | 4～6 个工作日 |
| 开始日期 | 待定 |
| 预计完成日期 | 待定 |
| 实际完成日期 | — |
| 当前负责人 | 待定 |
| 当前阻塞 | 无 |

状态约定：

- `未开始`：尚未修改文件；
- `进行中`：已有实际修改，但尚未满足验收条件；
- `阻塞`：缺少数据、决策或外部条件，无法继续；
- `已完成`：代码、测试、文档和验收全部完成。

## 3. 工作分解与进度

| 编号 | 工作项 | 状态 | 预计耗时 | 实际耗时 | 产物 |
| --- | --- | --- | ---: | ---: | --- |
| 0.1 | 自动提取 C++ 和 Python 暴露接口 | 未开始 | 0.5 天 | — | 原始接口清单 |
| 0.2 | 核对仓库内部、文档和示例的接口使用 | 未开始 | 0.5～1 天 | — | 使用位置与兼容性记录 |
| 0.3 | 完成接口分类和迁移建议 | 未开始 | 0.5 天 | — | `docs/arch/api-inventory.md` |
| 0.4 | 补充 `StockManager` 行为测试 | 未开始 | 0.5 天 | — | C++ 单元测试 |
| 0.5 | 新增 `System` 行为测试 | 未开始 | 1～1.5 天 | — | C++ 单元测试和回测结果 |
| 0.6 | 新增 `Strategy` 下单路由测试 | 未开始 | 0.5～1 天 | — | C++ 单元测试 |
| 0.7 | 建立三组 Python 回测金标 | 未开始 | 1～1.5 天 | — | JSON/CSV 金标和 Python 测试 |
| 0.8 | 建立 Python Public API 白名单测试 | 未开始 | 0.5 天 | — | Python API 测试 |
| 0.9 | 接入统一测试入口并执行全量回归 | 未开始 | 0.5 天 | — | 测试记录 |
| 0.10 | 完成阶段评审和下一阶段准入判断 | 未开始 | 0.5 天 | — | 评审结论 |

## 4. 具体任务

### 0.1 自动提取接口

需要扫描：

- `hikyuu_cpp/hikyuu/**/*.h` 中的公开类、函数和方法；
- `hikyuu_pywrap/**/*.cpp` 中的 `m.def`、`.def`、`.def_static`、`.def_property*`；
- `hikyuu/__init__.py`、`hikyuu/core.py`、`hikyuu/extend.py` 中的导入和导出；
- 各子包 `__init__.py` 中的星号导入。

重点文件：

- `hikyuu_cpp/hikyuu/StockManager.h`；
- `hikyuu_cpp/hikyuu/trade_manage/TradeManagerBase.h`；
- `hikyuu_cpp/hikyuu/trade_sys/system/System.h`；
- `hikyuu_cpp/hikyuu/strategy/Strategy.h`；
- `hikyuu_pywrap/_StockManager.cpp`；
- `hikyuu_pywrap/trade_manage/_TradeManager.cpp`；
- `hikyuu_pywrap/trade_sys/_System.cpp`；
- `hikyuu_pywrap/strategy/_Strategy.cpp`。

完成条件：

- 每个公开符号可以追溯到定义文件和绑定文件；
- C++ 接口和 Python 名称之间建立对应关系；
- 自动提取结果可重复生成，避免完全依赖手工维护。

### 0.2 核对接口使用情况

对每个候选接口检查：

- C++ 核心内部是否调用；
- Python 包内部是否调用；
- 单元测试是否调用；
- `docs/` 和 `hikyuu/examples/` 是否使用；
- 是否仅为 pybind11 子类扩展而存在；
- 是否标注了“internal”“test only”或类似说明；
- 是否存在同义、重复或可以组合的入口。

完成条件：

- 不能仅根据方法名称判断是否删除；
- 每个计划迁移或废弃的接口至少有一条使用证据；
- 无法确认的接口单独标记为“待确认”，不能直接归为可删除。

### 0.3 建立 API Inventory

新增文件：

```text
docs/arch/api-inventory.md
```

每条接口至少记录：

| 字段 | 说明 |
| --- | --- |
| Symbol | C++ 或 Python 完整符号名 |
| Defined in | 定义文件 |
| Bound/Exported by | pybind11 或 Python 导出文件 |
| Consumers | 核心、测试、示例和文档的主要调用位置 |
| Classification | Public / SPI / Internal / Deprecated |
| Decision | 保留、迁移、包装、弃用或删除 |
| Replacement | 建议的新接口；没有则写 `—` |
| Compatibility | 兼容期限和风险 |

第一批必须完成分类的对象：

- `StockManager`；
- `TradeManagerBase`；
- `System`；
- `Strategy`；
- `OrderBrokerBase`；
- `DataDriverFactory`；
- EV、CN、SG、MM、ST、TP、PG、SP 的 Base 类；
- Python 顶层导出符号。

### 0.4 补充 StockManager 基线测试

修改文件：

```text
hikyuu_cpp/unit_test/hikyuu/hikyuu/test_StockManager.cpp
```

覆盖范围：

- 初始化完成状态；
- 证券和市场查询；
- 不存在证券的返回行为；
- 交易日历；
- 临时证券的添加和移除；
- reload 前后的可观察行为；
- 测试结束后的全局状态恢复。

本任务不修改 `StockManager` 的生产实现。

### 0.5 新增 System 行为测试

新增文件：

```text
hikyuu_cpp/unit_test/hikyuu/trade_sys/system/test_System.cpp
```

如测试构建脚本未自动收集新文件，则同步修改：

```text
hikyuu_cpp/unit_test/xmake.lua
```

覆盖范围：

- 正常买入和卖出；
- 延迟买入和延迟卖出；
- 资金不足；
- 最小交易数量；
- 止损、止盈和目标价；
- EV、CN 对开仓的限制；
- `reset()`、`forceResetAll()` 和 `clone()`；
- `TradeRequest` 的产生、延迟和清除；
- 逐笔成交记录、现金和最终持仓。

完成条件：

- 测试断言最终收益之外，还要断言逐笔交易；
- 时间、价格、数量、费用、来源组件和现金余额均被检查；
- 不依赖用户本地真实行情数据库。

### 0.6 新增 Strategy 下单路由测试

新增文件：

```text
hikyuu_cpp/unit_test/hikyuu/strategy/test_Strategy.cpp
```

覆盖范围：

- `order()` 正数进入买入路径；
- `order()` 负数进入卖出路径；
- `orderValue()` 的数量换算；
- 最小交易单位和数量取整；
- 滑点后的实际价格；
- 无效价格、零数量和无效证券；
- `buy()`、`sell()` 与 TradeManager/Broker 的路由关系；
- 回测模式与非回测模式的差异。

已知关注点：

- `hikyuu_cpp/hikyuu/strategy/Strategy.cpp` 中 `order()` 的买入数量会先计算取整值，但后续调用可能仍使用原始 `num`；
- 本阶段先用测试明确期望行为；
- 如果现状测试失败，将其登记为已知缺陷。修复应使用独立提交，不能悄悄改写金标来掩盖问题。

### 0.7 建立回测金标

新增文件：

```text
hikyuu/test/test_backtest_golden.py
hikyuu/test/golden/simple_system.json
hikyuu/test/golden/risk_control_system.json
hikyuu/test/golden/simple_portfolio.json
```

三组建议场景：

1. 简单均线交叉：验证基本买卖流程；
2. 风控系统：包含手续费、滑点、止损和止盈；
3. 简单 Portfolio：包含多证券、Selector 和 AllocateFunds。

每组金标至少记录：

- 策略和数据参数；
- 数据时间范围；
- 每一笔交易；
- 每日现金和总资产；
- 最终持仓；
- 最终收益和最大回撤；
- 尚未执行的延迟请求。

约束：

- 优先使用 JSON 或 CSV，不使用绑定内部对象布局的 pickle；
- 明确浮点容差；
- 使用仓库内 `test_data/`，不依赖联网和用户真实数据；
- 金标更新必须由人工检查差异，不能在普通测试命令中自动覆盖。

### 0.8 建立 Python Public API 测试

新增文件：

```text
hikyuu/test/test_public_api.py
```

测试内容：

- 稳定核心符号可以从规定路径导入；
- 计划保留的兼容入口仍然存在；
- API inventory 中的 Python 名称与运行时导出一致；
- 后续引入 `__all__` 后，顶层只暴露白名单符号；
- SPI 和 Internal 接口不会被误记为稳定 Public API。

本阶段只记录和保护已经确认的稳定子集，不立即收缩 `hikyuu` 顶层导出。

### 0.9 接入测试并执行回归

可能修改：

```text
hikyuu/test/test.py
hikyuu_cpp/unit_test/xmake.lua
```

统一验证命令：

```bash
./op.sh build
./op.sh small-test
./op.sh unit-test
./op.sh python-test
./op.sh import-test
```

测试结果记录在本文档的“执行记录”中，包括日期、命令、结果和失败原因。

### 0.10 阶段评审

评审时回答：

1. 哪些接口可以直接转为 Internal？
2. 哪些接口需要一个版本的弃用期？
3. Python 用户最常用的稳定入口有哪些？
4. 当前测试能否发现成交、资金或持仓的行为变化？
5. 是否具备进入第 1 步 `HikyuuSession` 重构的条件？

## 5. 本阶段不修改的生产代码

除非发现必须单独修复的明确缺陷，本阶段不修改：

- `hikyuu_cpp/hikyuu/StockManager.h/.cpp` 的接口和实现；
- `hikyuu_cpp/hikyuu/trade_manage/TradeManagerBase.h/.cpp`；
- `hikyuu_cpp/hikyuu/trade_sys/system/System.h/.cpp`；
- `hikyuu_cpp/hikyuu/strategy/Strategy.h/.cpp`；
- `hikyuu_pywrap/` 的绑定内容；
- `hikyuu/__init__.py` 的导出行为；
- 指标、策略和交易费用算法；
- 序列化格式及 Python pickle 格式；
- 目录结构和动态库名称。

如果基线测试暴露已有缺陷，采用以下流程：

```text
发现缺陷
   │
   ├── 记录当前实际行为
   ├── 明确正确行为
   ├── 新增可复现的失败测试
   └── 使用独立 bugfix 提交修复
```

缺陷修复不得与接口重构放在同一个提交中。

## 6. 验收清单

- [ ] 已生成完整的原始接口列表；
- [ ] `StockManager`、`TradeManagerBase`、`System`、`Strategy` 已逐项分类；
- [ ] Python 顶层导出已分类；
- [ ] 每个 Deprecated 接口都有替代方案和兼容期限；
- [ ] `StockManager` 基线测试已补充；
- [ ] `System` C++ 行为测试已建立；
- [ ] `Strategy` 下单路由测试已建立；
- [ ] 三组代表性回测金标已建立；
- [ ] Python Public API 测试已建立；
- [ ] `small-test` 通过；
- [ ] `unit-test` 通过；
- [ ] Python 测试通过；
- [ ] Python 3.10 import 测试通过；
- [ ] 已记录已有缺陷，但未将缺陷行为误当成正确金标；
- [ ] 已完成进入第 1 步的评审。

只有以上项目全部完成，阶段状态才能改为“已完成”。

## 7. 风险与应对

| 风险 | 影响 | 应对措施 |
| --- | --- | --- |
| 当前接口数量过多，人工清单遗漏 | 错删兼容接口 | 自动提取后再人工分类 |
| 测试依赖全局单例，彼此污染 | 测试偶发失败 | 每个测试恢复全局状态，优先串行建立基线 |
| 回测结果包含浮点差异 | 金标跨平台不稳定 | 对金额、价格和收益设置字段级容差 |
| 金标依赖用户真实数据 | 无法在 CI 重现 | 仅使用仓库内 `test_data/` |
| 现有缺陷被固化成金标 | 后续无法正确修复 | 分开记录“当前行为”和“期望行为” |
| Python `import *` 难以确定真实公共面 | Public API 清单过大 | 结合文档、示例和调用频率划分稳定子集 |
| 测试编译时间增长 | 开发反馈变慢 | 小测试覆盖关键路径，全量金标放 `unit-test`/Python 测试 |

## 8. 决策记录

| 日期 | 决策 | 原因 | 影响 |
| --- | --- | --- | --- |
| 待定 | — | — | — |

## 9. 执行记录

| 日期 | 工作项 | 修改文件 | 验证结果 | 备注 |
| --- | --- | --- | --- | --- |
| 待定 | — | — | — | — |

## 10. 工期记录

| 日期 | 投入时间 | 工作内容 | 剩余估算 |
| --- | ---: | --- | ---: |
| 待定 | — | — | 4～6 天 |

## 11. 阶段完成摘要

当前尚未开始实施。完成后在此记录：

- 最终接口数量和各分类数量；
- 新增及修改的测试文件；
- 金标覆盖的策略场景；
- 发现的已有缺陷；
- 未解决风险；
- 是否批准进入第 1 步 `HikyuuSession` 重构。
