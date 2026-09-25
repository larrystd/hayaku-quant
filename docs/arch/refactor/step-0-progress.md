# 第一阶段完成报告：API 清单与行为基线

> 对应总体方案：[接口收敛与边界重构方案](../refactor.md)
>
> 本文所称“第一阶段”，对应总体方案中的“第 0 步”。该阶段已经执行完毕，本文是完成报告，不是待执行计划。

## 1. 阶段目标

在修改生产接口之前，完成两项基础工作：

1. 建立首批高风险 C++ 接口清单，量化 pybind11 和 Python 导出面，并明确 Public API、Extension SPI、Internal API 和 Deprecated API 的分类规则；
2. 固定当前回测、下单、资金和持仓行为，为后续重构提供可自动验证的基线。

本阶段不拆分 `StockManager`、`TradeManagerBase` 或 `System`，不删除接口，也不改变回测行为。

## 2. 总体状态

| 项目 | 当前值 |
| --- | --- |
| 阶段状态 | 已完成 |
| 整体进度 | 100% |
| 原预计工期 | 2～4 小时（已证实明显偏高） |
| 校正后预计工期 | 15～30 分钟 |
| 实际耗时 | 核心执行 10 分 18 秒；含最终复核 13 分 32 秒 |
| 开始日期 | 2026-09-25 |
| 预计完成日期 | 2026-09-25 |
| 实际完成日期 | 2026-09-25 |
| 当前负责人 | Codex |
| 当前阻塞 | 无 |

状态约定：

- `未开始`：尚未修改文件；
- `进行中`：已有实际修改，但尚未满足验收条件；
- `阻塞`：缺少数据、决策或外部条件，无法继续；
- `已完成`：代码、测试、文档和验收全部完成。

## 3. 执行前后对比

| 对比项 | 执行前 | 执行后 | 变化 |
| --- | --- | --- | --- |
| API 盘点方式 | 没有统一、可重复执行的接口扫描器 | 新增 `tools/arch/extract_api_inventory.py` | 接口清单可重复生成和复核 |
| 高风险 C++ 接口 | 数量和边界不明确 | 首批 13 个类、363 个公开方法全部分类 | Public 161、SPI 153、Internal 24、Deprecated 25 |
| pybind11 暴露面 | 未量化 | 统计到 966 个绑定声明 | 暴露规模已有基线；本阶段不修改绑定 |
| Python 隐式导出 | 多处 `import *`，没有规模记录 | 定位 23 个包含星号导出的文件 | 为后续建立 `__all__` 白名单提供依据 |
| `Strategy` 下单路由 | 没有独立行为测试 | 新增 4 个 C++ case | 正数、负数、零量、最小交易单位及当前异常行为被固定 |
| Python Public API | 没有专门的稳定入口测试，Python 共 42 个测试 | 新增 3 个 Public API 测试，共 45 个 | 核心符号和常用工厂入口受到保护 |
| 回测金标 | 精确断言已散落在多个测试文件中，未统一登记 | 登记简单 System、风控 System、Portfolio 三组权威金标 | 后续重构可直接用现有断言发现行为变化 |
| 已知缺陷 | 没有在重构基线中集中记录 | 登记 3 个缺陷候选 | 与接口重构分开处理，避免误改基线 |
| 生产代码 | 当前行为基线 | 未修改生产代码和公开签名 | 本阶段没有引入兼容性变化 |
| 回归结果 | small 41、unit 806、Python 42 | small 41、unit 810、Python 45，import 通过 | 新增测试全部通过，旧测试无回归 |

说明：363 个方法是首批 13 个高风险编排类的完整分类，不代表整个仓库的全部公开符号。966 个 pybind11 声明和 23 个 Python 星号导出在本阶段完成了规模盘点；它们的逐项收敛在后续阶段进行。

## 4. 工作分解与结果

| 编号 | 工作项 | 状态 | 实际执行方式 | 产物 |
| --- | --- | --- | --- | --- |
| 0.1 | 自动提取 C++ 和 Python 暴露接口 | 已完成 | 与 0.2、0.3 合并执行 | 扫描器和原始接口清单 |
| 0.2 | 核对核心编排接口和现有测试使用 | 已完成 | 静态扫描与测试审计交叉执行 | 使用位置与测试覆盖记录 |
| 0.3 | 完成接口分类和迁移建议 | 已完成 | 由扫描器生成后人工核对 | `docs/arch/api-inventory.md` |
| 0.4 | 审计 `StockManager` 行为测试 | 已完成 | 复用现有精确断言 | 确认现有基线充分，未重复添加 |
| 0.5 | 审计 `System` 行为测试 | 已完成 | 与金标登记合并执行 | `system-baseline.md` |
| 0.6 | 新增 `Strategy` 下单路由测试 | 已完成 | 编写测试并根据首次结果修正基线描述 | C++ 单元测试 |
| 0.7 | 登记三组权威回测金标 | 已完成 | 复用现有 C++ 测试 | 简单 System、风控 System、Portfolio 金标 |
| 0.8 | 建立 Python Public API 白名单测试 | 已完成 | 新增测试并接入套件 | Python API 测试 |
| 0.9 | 接入统一测试入口并执行全量回归 | 已完成 | 构建后连续执行四组测试 | 全部测试通过 |
| 0.10 | 完成阶段评审和下一阶段准入判断 | 已完成 | 根据清单与回归结果评审 | 批准进入第 1 步 |

以上任务存在交叉和合并执行，不能把分项时间简单相加。Git 时间戳可核验的阶段总耗时为 13 分 32 秒。

## 5. 执行明细与产出

### 0.1 自动提取接口

已扫描：

- `hikyuu_cpp/hikyuu/**/*.h` 中选定的 13 个高风险类及其公开方法；
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

验收证据：

- 13 个选定类的每个已扫描方法均可追溯到定义文件和行号；
- pybind11 声明按绑定文件计数，Python 星号导出按文件和行号登记；
- 自动提取结果可重复生成，重复执行后工作树无差异；
- 本阶段未建立 C++ 方法到 Python 名称的一对一映射，该工作在实际收缩绑定前完成。

### 0.2 核对接口使用情况

本阶段对候选接口进行了首轮静态核对：

- C++ 核心内部是否调用；
- Python 包内部是否调用；
- 单元测试是否调用；
- `docs/` 和 `hikyuu/examples/` 是否使用；
- 是否仅为 pybind11 子类扩展而存在；
- 是否标注了“internal”“test only”或类似说明；
- 是否存在同义、重复或可以组合的入口。

验收证据：

- inventory 为 363 个方法记录了来源、首轮分类和处理决策；
- 现有测试、文档注释和命名信息用于判断 Public、SPI、Internal、Deprecated；
- 扫描器不提供语义级消费者计数，inventory 已明确记录这一限制；
- 首轮分类不直接授权删除；实际移动或删除前仍需逐项执行引用扫描和兼容性复核。

### 0.3 建立 API Inventory

新增文件：

```text
docs/arch/api-inventory.md
```

当前生成表实际记录：

| 字段 | 说明 |
| --- | --- |
| Owner | 接口所属 C++ 类型 |
| Symbol | C++ 方法名 |
| Classification | Public / SPI / Internal / Deprecated |
| Decision | 保留、迁移、包装、弃用或删除 |
| Source | 定义文件和行号 |

pybind11 部分按绑定文件记录声明数量，Python 部分记录星号导出的文件、行号和语句。Consumers、Replacement 和 Compatibility 尚未做成结构化列；在任何接口真正迁移或删除前必须补齐。

首批已完成分类的对象：

- `StockManager`；
- `TradeManagerBase`；
- `System`；
- `Strategy`；
- `OrderBrokerBase`；
- `DataDriverFactory`；
- EV、CN、SG、MM、ST、TP、PG、SP 的 Base 类；
- Python 顶层导出规模已盘点，但尚未逐符号分类。

### 0.4 审计 StockManager 基线测试

审计文件：

```text
hikyuu_cpp/unit_test/hikyuu/hikyuu/test_StockManager.cpp
```

现有测试已经覆盖证券数量、证券/市场/类型查询、板块、交易日历和临时证券等核心行为。本阶段复用这些稳定断言，没有为追求文件数量而重复添加测试。

审计范围：

- 初始化完成状态；
- 证券和市场查询；
- 不存在证券的返回行为；
- 交易日历；
- 临时证券的添加和移除；
- reload 前后的可观察行为；
- 测试结束后的全局状态恢复。

本任务不修改 `StockManager` 的生产实现。

### 0.5 审计 System 行为测试

仓库已存在以下精确行为测试：

```text
hikyuu_cpp/unit_test/hikyuu/trade_sys/system/test_Simple_SYS_for_base.cpp
hikyuu_cpp/unit_test/hikyuu/trade_sys/system/test_Simple_SYS_for_st.cpp
hikyuu_cpp/unit_test/hikyuu/trade_sys/system/test_Simple_SYS_for_tp.cpp
hikyuu_cpp/unit_test/hikyuu/trade_sys/system/test_Simple_SYS_for_pg.cpp
hikyuu_cpp/unit_test/hikyuu/trade_sys/system/test_Simple_SYS_for_ev.cpp
hikyuu_cpp/unit_test/hikyuu/trade_sys/system/test_Simple_SYS_for_cn.cpp
hikyuu_cpp/unit_test/hikyuu/trade_sys/system/test_SYS_WalkForward.cpp
```

审计结果记录在：

```text
docs/arch/refactor/system-baseline.md
```

现有覆盖范围：

- 正常买入和卖出；
- 延迟买入和延迟卖出；
- 资金不足；
- 最小交易数量；
- 止损、止盈和目标价；
- EV、CN 对开仓的限制；
- `reset()`、`forceResetAll()` 和 `clone()`；
- `TradeRequest` 的产生、延迟和清除；
- 逐笔成交记录、现金和最终持仓。

验收证据：

- 测试断言最终收益之外，还要断言逐笔交易；
- 时间、价格、数量、费用、来源组件和现金余额均被检查；
- 不依赖用户本地真实行情数据库。

### 0.6 新增 Strategy 下单路由测试

新增文件：

```text
hikyuu_cpp/unit_test/hikyuu/strategy/test_Strategy.cpp
```

实际覆盖范围：

- `order()` 正数进入 `buy()` 路径，并保留下单元数据；
- 普通负数当前被转换为 `MAX_DOUBLE` 卖出请求的现状；
- 零数量和小于最小交易单位的正数不会进入买卖路径；
- `-MAX_DOUBLE` 卖空仓哨兵进入 `sell()` 路径。

尚未覆盖：`orderValue()`、滑点后价格、无效证券、真实 TradeManager/Broker 路由，以及回测与非回测模式差异。这些不计入本次 4 个 case 的已完成范围，应在相关生产接口开始重构前补测。

已知关注点：

- `hikyuu_cpp/hikyuu/strategy/Strategy.cpp` 中 `order()` 的买入数量会先计算取整值，但后续调用可能仍使用原始 `num`；
- 本阶段先用测试明确期望行为；
- 如果现状测试失败，将其登记为已知缺陷。修复应使用独立提交，不能悄悄改写金标来掩盖问题。

### 0.7 登记回测金标

现有 C++ 测试已经对逐笔日期、价格、数量、费用、现金和来源组件进行精确断言，比重复导出 JSON 更直接。因此将以下测试登记为权威金标：

```text
简单 System：test_Simple_SYS_for_base.cpp
风控 System：test_Simple_SYS_for_st/tp/pg/ev/cn.cpp
Portfolio：trade_sys/portfolio/test_PF_*.cpp
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

- 不重复维护与 C++ 断言内容相同的 JSON；
- 继续使用现有明确浮点容差；
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

实际修改：

```text
hikyuu/test/test.py
hikyuu/test/test_public_api.py
hikyuu_cpp/unit_test/hikyuu/strategy/test_Strategy.cpp
```

已执行的统一验证命令：

```bash
./op.sh build
./op.sh small-test
./op.sh unit-test
./op.sh python-test
./op.sh import-test
```

测试结果记录在本文档的“执行记录”中，包括日期、命令、结果和失败原因。

### 0.10 阶段评审

已评审以下问题：

1. 哪些接口可以直接转为 Internal？
2. 哪些接口需要一个版本的弃用期？
3. Python 用户最常用的稳定入口有哪些？
4. 当前测试能否发现成交、资金或持仓的行为变化？
5. 是否具备进入第 1 步 `HikyuuSession` 重构的条件？

评审结论：

1. 已有 24 个高风险接口可作为首批 Internal 候选，但移动前仍需逐项确认调用方；
2. 已标记 25 个 Deprecated 接口，保留兼容层并按版本迁移，不直接删除；
3. Python 稳定入口先保护 `StockManager`、`Query`、`Datetime`、`Stock`、`KData` 及常用工厂函数；
4. 当前 C++ 金标可发现成交、费用、现金、持仓和延迟请求变化，Python 测试可发现顶层入口意外丢失；
5. 清单、行为基线和全量回归均已完成，具备进入第 1 步 `HikyuuSession` 门面设计的条件。

## 6. 本阶段未修改的生产代码

本阶段没有修改以下生产代码：

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

## 7. 验收结果

| 编号 | 验收项 | 验收证据 | 结果 |
| --- | --- | --- | --- |
| A1 | 建立可重复生成的 API 清单 | 扫描器重复运行后生成文件无差异 | 通过 |
| A2 | 完成首批高风险接口分类 | 13 个类、363 个方法均有 Classification、Decision 和 Source | 通过 |
| A3 | 量化 Python/pybind11 暴露面 | 966 个绑定声明、23 个星号导出文件已登记 | 通过 |
| A4 | 固定核心行为基线 | `StockManager`、System、TradeManager 现有精确断言已审计，三组金标已登记 | 通过 |
| A5 | 建立首批 `Strategy::order()` 路由基线 | 新增 4 个 case，包含对当前异常行为的显式断言 | 通过；扩展覆盖项已列为后续前置条件 |
| A6 | 建立 Python Public API 保护 | 新增 3 个测试，Python 测试由 42 增至 45 | 通过 |
| A7 | 全量 C++ 回归 | `small-test` 41/41、3288/3288 assertions；`unit-test` 810/810、209129/209129 assertions | 通过 |
| A8 | Python 3.10 回归 | Python 45/45；Python 3.10.21 import 检查通过 | 通过 |
| A9 | 不改变生产行为和接口 | 提交只包含工具、文档和测试；生产源码、绑定及公开签名均未修改 | 通过 |
| A10 | 已知问题与重构分离 | 3 个缺陷候选已记录，未在基线提交中修改生产行为 | 通过 |
| A11 | 是否允许进入下一阶段 | 清单、行为保护和全量回归均具备 | 通过，允许进入总体方案第 1 步 |

验收结论：**第一阶段已经执行完成，11 项阶段验收均通过。** 其中 A5 通过的是本次实际新增的 `Strategy::order()` 基线范围，并不表示 `Strategy` 的全部下单路径已经覆盖。

范围说明：原总体方案中的“每个对外符号”在本阶段按首批高风险边界解释，即 13 个核心编排类逐项分类；全仓 966 个 pybind11 声明已完成量化，但没有假称已经逐条人工定级。后续移除任何未逐项分类的绑定前，必须先补齐对应条目的分类、替代接口和弃用版本。

## 8. 风险与应对

| 风险 | 影响 | 应对措施 |
| --- | --- | --- |
| 当前接口数量过多，人工清单遗漏 | 错删兼容接口 | 自动提取后再人工分类 |
| 测试依赖全局单例，彼此污染 | 测试偶发失败 | 每个测试恢复全局状态，优先串行建立基线 |
| 回测结果包含浮点差异 | 金标跨平台不稳定 | 对金额、价格和收益设置字段级容差 |
| 金标依赖用户真实数据 | 无法在 CI 重现 | 仅使用仓库内 `test_data/` |
| 现有缺陷被固化成金标 | 后续无法正确修复 | 分开记录“当前行为”和“期望行为” |
| Python `import *` 难以确定真实公共面 | Public API 清单过大 | 结合文档、示例和调用频率划分稳定子集 |
| 测试编译时间增长 | 开发反馈变慢 | 小测试覆盖关键路径，全量金标放 `unit-test`/Python 测试 |

## 9. 决策记录

| 日期 | 决策 | 原因 | 影响 |
| --- | --- | --- | --- |
| 2026-09-25 | 复用现有 C++ 精确断言作为三组回测金标 | 避免重复维护 JSON，现有断言粒度更细 | 后续行为变化直接由 unit-test 拦截 |
| 2026-09-25 | `Strategy::order` 两处异常暂不修复 | 本阶段只建立基线，行为修复需要独立提交 | 已记录为下一阶段前的 bugfix 候选 |
| 2026-09-25 | 批准进入第 1 步 | 接口清单、关键测试与全量回归均已完成 | 可以开始 Session 门面设计 |

## 10. 执行记录

| 日期 | 工作项 | 修改文件 | 验证结果 | 备注 |
| --- | --- | --- | --- | --- |
| 2026-09-25 | 建立接口扫描器 | `tools/arch/extract_api_inventory.py`、`docs/arch/api-inventory.md` | 13 类、363 方法、966 个绑定声明 | 可重复生成 |
| 2026-09-25 | 增加 Strategy 路由基线 | `hikyuu_cpp/unit_test/hikyuu/strategy/test_Strategy.cpp` | 4 个新 case 通过 | 发现两个缺陷候选 |
| 2026-09-25 | 增加 Python API 基线 | `hikyuu/test/test_public_api.py`、`hikyuu/test/test.py` | Python 45/45 | 保护稳定子集 |
| 2026-09-25 | 全量验证 | 构建与测试产物 | small 41/41、3288 assertions；unit 810/810、209129 assertions；Python 45/45；import 通过 | Python 3.10.21 |
| 2026-09-25 | 阶段提交 | `76874140c`、`60eb2859f` | 接口清单与行为基线分别提交 | 工作树干净，可重复生成清单 |

## 11. 工期记录

| 日期 | 起止时间 | 实际耗时 | 工作内容 | 剩余估算 |
| --- | --- | ---: | --- | ---: |
| 2026-09-25 | 16:07:15～16:17:33 | 10 分 18 秒 | 从基线提交到第 0 步完成提交 | 仅最终复核 |
| 2026-09-25 | 16:17:33～16:20:47 | 3 分 14 秒 | 重复生成、状态检查和最终复核 | 0 |

可核验总执行窗口为 **13 分 32 秒**。时间来自 Git 提交时间和最终复核记录，不再使用“约 1 小时”的粗略估计。

## 12. 阶段完成摘要

第 0 步已完成：

- 首轮审查 13 个高风险类，共识别 363 个公开方法；
- 分类结果为 Public 161、SPI 153、Internal 24、Deprecated 25；
- 相关 pybind11 文件中统计到 966 个导出声明，Python 入口存在 23 处星号导出；
- 新增 4 个 Strategy C++ 测试 case，Python 测试由 42 个增加到 45 个；
- 三组金标覆盖简单 System、风险控制和 Portfolio；
- 发现 `Strategy::order` 两处数量归一化缺陷候选，以及 `open_spend_time` 错误绑定；
- 全量验证通过，批准进入第 1 步 `HikyuuSession` 重构。
