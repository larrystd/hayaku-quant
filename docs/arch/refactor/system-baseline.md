# System、Strategy 与 Portfolio 行为基线

本文档登记第 0 步直接复用或新增的行为基线。Hikyuu 已有大量逐笔精确断言，因此不再复制一套内容相同的 JSON 金标；现有 C++ 测试即为权威金标，Python 测试负责稳定导入面。

## 1. 三组核心金标

| 金标 | 权威测试 | 固定行为 |
| --- | --- | --- |
| 简单 System | `hikyuu_cpp/unit_test/hikyuu/trade_sys/system/test_Simple_SYS_for_base.cpp` | MA 交叉、非延迟/延迟买卖、逐笔日期、计划价、成交价、数量、费用、现金和来源组件 |
| 风控 System | `test_Simple_SYS_for_st.cpp`、`test_Simple_SYS_for_tp.cpp`、`test_Simple_SYS_for_pg.cpp`、`test_Simple_SYS_for_ev.cpp`、`test_Simple_SYS_for_cn.cpp` | 止损、止盈、目标价、环境和条件对开仓/平仓的影响 |
| Portfolio | `hikyuu_cpp/unit_test/hikyuu/trade_sys/portfolio/test_PF_for_base.cpp` 及同目录 PF 测试 | 多 System 调度、Selector、AllocateFunds、组合账户和延迟交易 |

这些测试使用仓库内 `test_data/`，不依赖用户真实行情库或网络。其断言直接比较领域对象和数值，避免 JSON 序列化格式变化造成无意义差异。

## 2. System 覆盖矩阵

| 行为 | 当前覆盖 | 结论 |
| --- | --- | --- |
| 缺少 TM/SG/MM 时拒绝运行 | `test_Simple_SYS_for_base.cpp` | 已覆盖 |
| 非延迟买卖 | `test_Simple_SYS_for_base.cpp` | 已覆盖逐笔结果 |
| 延迟买卖 | `test_Simple_SYS_for_base.cpp` | 已覆盖逐笔结果 |
| Stoploss | `test_Simple_SYS_for_st.cpp` | 已覆盖 |
| Takeprofit | `test_Simple_SYS_for_tp.cpp` | 已覆盖 |
| ProfitGoal | `test_Simple_SYS_for_pg.cpp` | 已覆盖 |
| Environment | `test_Simple_SYS_for_ev.cpp` | 已覆盖 |
| Condition | `test_Simple_SYS_for_cn.cpp` | 已覆盖 |
| Walk Forward | `test_SYS_WalkForward.cpp` | 已覆盖资金及延迟请求 |
| TradeRequest 序列化 | `test_export.cpp` | 已覆盖 |
| System 序列化 | `test_export.cpp` | 已覆盖 |
| SystemPart 名称和枚举 | `test_SystemPart.cpp` | 已覆盖 |
| Strategy 数量路由 | `hikyuu_cpp/unit_test/hikyuu/strategy/test_Strategy.cpp` | 本阶段新增 |
| Python 稳定入口 | `hikyuu/test/test_public_api.py` | 本阶段新增 |

## 3. Strategy 新增基线

新增的 `test_Strategy.cpp` 不启动实时行情线程，只测试公开订单命令边界：

- 正数订单进入 `buy()`；
- 零数量和小于最小买入单位的数量不会进入执行方法；
- 负数订单进入 `sell()`；
- `-MAX_DOUBLE` 保持卖出全部的哨兵语义；
- remark 和来源组件能够原样传递。

## 4. 已发现的缺陷候选

### 4.1 正数订单没有使用归一化数量

文件：`hikyuu_cpp/hikyuu/strategy/Strategy.cpp`

当前代码计算了 `buy_num`：

```cpp
double buy_num = int64_t(num / min_trade_num) * min_trade_num;
```

但调用 `buy()` 时传递的是原始 `num`，不是 `buy_num`。因此最小交易单位取整和最大交易数量限制实际上没有生效。

本阶段只记录该问题，不修改生产行为。后续应使用独立 bugfix 提交修复，并增加以下断言：

- `250`、最小交易单位 `100` 时买入 `200`；
- 超过 `maxTradeNumber` 时截断为最大数量。

### 4.2 普通负数订单会变成卖出全部

文件：`hikyuu_cpp/hikyuu/strategy/Strategy.cpp`

当前判断：

```cpp
else if ((sell_num + num) < min_trade_num) {
    sell_num = MAX_DOUBLE;
}
```

因为 `num` 为负数，即使输入 `-200` 且最小交易单位为 `100`，`sell_num + num` 也为 `0`，最终被转换成 `MAX_DOUBLE`。

新增测试明确记录了这一现状，但在测试注释中标明它是“缺陷候选”，不是目标契约。修复前需要确认原意究竟是：

- 卖出指定数量；还是
- 当卖出后剩余持仓不足一个交易单位时卖出全部。

后一种语义需要查询当前持仓，不能仅通过订单参数 `num` 推断。

### 4.3 `open_spend_time` 绑定到错误函数

文件：`hikyuu_pywrap/main.cpp`

`open_spend_time` 当前绑定到了 `close_spend_time`。这是独立的绑定错误，后续应单独修复并增加 Python 测试。

## 5. 验证结果

环境：macOS arm64、Python 3.10.21、xmake 3.0.8、shared build。

| 命令 | 结果 |
| --- | --- |
| `./op.sh build` | 通过，生成 `hikyuu/cpp/core310.so` |
| `./op.sh small-test` | 41/41 cases，3288/3288 assertions |
| `./op.sh unit-test` | 810/810 cases，209129/209129 assertions |
| `./op.sh python-test` | 45/45 tests |
| `./op.sh import-test` | Python 3.10.21、Hikyuu 2.8.2 导入通过 |

## 6. 后续使用规则

- 修改 `System`、`TradeManager`、`Strategy` 或 Portfolio 调度后必须运行全量 `unit-test`；
- 修改 Python 导出后必须运行 `test_public_api.py`；
- 金标差异必须人工检查，不能直接用新输出覆盖旧断言；
- 确认并修复上述缺陷时，行为变更与接口重构应使用不同提交。
