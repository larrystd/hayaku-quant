# 版本发布说明

## 2.8.3（开发中）

`Performance.names()` 返回英文统计项 key。旧中文 key 暂可使用，但已废弃并会输出告警。
中文环境下，`Performance.report()` 仍通过 `i18n/zh_CN.po` 输出中文。
`SE_PerformanceOptimal` 的默认 key 改为 `Account Avg Annual Return %`，
`hayaku.analysis` 和 `hayaku.draw` 的默认 key 已同步。

| 旧中文 key | 新英文 key |
| --- | --- |
| 帐户初始金额 | Account Initial Capital |
| 累计投入本金 | Total Invested Principal |
| 累计投入资产 | Total Invested Assets |
| 累计借入现金 | Total Borrowed Cash |
| 累计借入资产 | Total Borrowed Assets |
| 累计红利 | Total Dividends |
| 现金余额 | Cash Balance |
| 未平仓头寸净值 | Open Position Net Value |
| 当前总资产 | Current Total Assets |
| 已平仓交易总成本 | Total Cost of Closed Trades |
| 已平仓净利润总额 | Total Net Profit of Closed Trades |
| 单笔交易最大占用现金比例% | Max Cash Usage per Trade % |
| 交易平均占用现金比例% | Avg Cash Usage per Trade % |
| 未平仓帐户收益率% | Open Position Account Return % |
| 已平仓帐户收益率% | Closed Trade Account Return % |
| 帐户年复合收益率% | Account CAGR % |
| 帐户平均年收益率% | Account Avg Annual Return % |
| 赢利交易赢利总额 | Total Profit of Winning Trades |
| 亏损交易亏损总额 | Total Loss of Losing Trades |
| 已平仓交易总数 | Total Closed Trades |
| 赢利交易数 | Number of Winning Trades |
| 亏损交易数 | Number of Losing Trades |
| 赢利交易比例% | Win Rate % |
| 赢利期望值 | Profit Expectancy |
| 赢利交易平均赢利 | Avg Profit per Winning Trade |
| 亏损交易平均亏损 | Avg Loss per Losing Trade |
| 平均赢利/平均亏损比例 | Avg Win / Avg Loss Ratio |
| 净赢利/亏损比例 | Profit Factor |
| 最大单笔赢利 | Largest Single Win |
| 最大单笔盈利百分比% | Largest Single Win % |
| 最大单笔亏损 | Largest Single Loss |
| 最大单笔亏损百分比% | Largest Single Loss % |
| 赢利交易平均持仓时间 | Avg Holding Period of Winning Trades |
| 赢利交易最大持仓时间 | Max Holding Period of Winning Trades |
| 亏损交易平均持仓时间 | Avg Holding Period of Losing Trades |
| 亏损交易最大持仓时间 | Max Holding Period of Losing Trades |
| 空仓总时间 | Total Time Flat |
| 空仓时间/总时间% | Time Flat / Total Time % |
| 平均空仓时间 | Avg Time Flat |
| 最长空仓时间 | Max Time Flat |
| 最大连续赢利笔数 | Max Consecutive Wins |
| 最大连续亏损笔数 | Max Consecutive Losses |
| 最大连续赢利金额 | Max Consecutive Win Amount |
| 最大连续亏损金额 | Max Consecutive Loss Amount |
| R乘数期望值 | R-Multiple Expectancy |
| 交易机会频率/年 | Trade Opportunities per Year |
| 年度期望R乘数 | Annual Expected R-Multiple |
| 赢利交易平均R乘数 | Avg R-Multiple of Winning Trades |
| 亏损交易平均R乘数 | Avg R-Multiple of Losing Trades |
| 最大单笔赢利R乘数 | Max Single Win R-Multiple |
| 最大单笔亏损R乘数 | Max Single Loss R-Multiple |
| 最大连续赢利R乘数 | Max Consecutive Win R-Multiple |
| 最大连续亏损R乘数 | Max Consecutive Loss R-Multiple |

旧版本的发布记录可在 Git 历史中查阅。
