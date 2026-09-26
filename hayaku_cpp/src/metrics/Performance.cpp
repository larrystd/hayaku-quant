/*
 * Performance.cpp
 *
 *  Created on: 2013-4-23
 *      Author: fasiondog
 */

#include "metrics/Performance.h"

namespace hayaku {

namespace {

/** Legacy Chinese key aliases retained for backward compatibility. */
const std::map<string, string>& legacyKeyMap() {
  static const std::map<string, string> keys = {
      {"帐户初始金额", "Account Initial Capital"},
      {"累计投入本金", "Total Invested Principal"},
      {"累计投入资产", "Total Invested Assets"},
      {"累计借入现金", "Total Borrowed Cash"},
      {"累计借入资产", "Total Borrowed Assets"},
      {"累计红利", "Total Dividends"},
      {"现金余额", "Cash Balance"},
      {"未平仓头寸净值", "Open Position Net Value"},
      {"当前总资产", "Current Total Assets"},
      {"已平仓交易总成本", "Total Cost of Closed Trades"},
      {"已平仓净利润总额", "Total Net Profit of Closed Trades"},
      {"单笔交易最大占用现金比例%", "Max Cash Usage per Trade %"},
      {"交易平均占用现金比例%", "Avg Cash Usage per Trade %"},
      {"未平仓帐户收益率%", "Open Position Account Return %"},
      {"已平仓帐户收益率%", "Closed Trade Account Return %"},
      {"帐户年复合收益率%", "Account CAGR %"},
      {"帐户平均年收益率%", "Account Avg Annual Return %"},
      {"赢利交易赢利总额", "Total Profit of Winning Trades"},
      {"亏损交易亏损总额", "Total Loss of Losing Trades"},
      {"已平仓交易总数", "Total Closed Trades"},
      {"赢利交易数", "Number of Winning Trades"},
      {"亏损交易数", "Number of Losing Trades"},
      {"赢利交易比例%", "Win Rate %"},
      {"赢利期望值", "Profit Expectancy"},
      {"赢利交易平均赢利", "Avg Profit per Winning Trade"},
      {"亏损交易平均亏损", "Avg Loss per Losing Trade"},
      {"平均赢利/平均亏损比例", "Avg Win / Avg Loss Ratio"},
      {"净赢利/亏损比例", "Profit Factor"},
      {"最大单笔赢利", "Largest Single Win"},
      {"最大单笔盈利百分比%", "Largest Single Win %"},
      {"最大单笔亏损", "Largest Single Loss"},
      {"最大单笔亏损百分比%", "Largest Single Loss %"},
      {"赢利交易平均持仓时间", "Avg Holding Period of Winning Trades"},
      {"赢利交易最大持仓时间", "Max Holding Period of Winning Trades"},
      {"亏损交易平均持仓时间", "Avg Holding Period of Losing Trades"},
      {"亏损交易最大持仓时间", "Max Holding Period of Losing Trades"},
      {"空仓总时间", "Total Time Flat"},
      {"空仓时间/总时间%", "Time Flat / Total Time %"},
      {"平均空仓时间", "Avg Time Flat"},
      {"最长空仓时间", "Max Time Flat"},
      {"最大连续赢利笔数", "Max Consecutive Wins"},
      {"最大连续亏损笔数", "Max Consecutive Losses"},
      {"最大连续赢利金额", "Max Consecutive Win Amount"},
      {"最大连续亏损金额", "Max Consecutive Loss Amount"},
      {"R乘数期望值", "R-Multiple Expectancy"},
      {"交易机会频率/年", "Trade Opportunities per Year"},
      {"年度期望R乘数", "Annual Expected R-Multiple"},
      {"赢利交易平均R乘数", "Avg R-Multiple of Winning Trades"},
      {"亏损交易平均R乘数", "Avg R-Multiple of Losing Trades"},
      {"最大单笔赢利R乘数", "Max Single Win R-Multiple"},
      {"最大单笔亏损R乘数", "Max Single Loss R-Multiple"},
      {"最大连续赢利R乘数", "Max Consecutive Win R-Multiple"},
      {"最大连续亏损R乘数", "Max Consecutive Loss R-Multiple"},
  };
  return keys;
}

/** English keys and their legacy Chinese aliases, including addKey aliases. */
std::map<string, string>& chineseNameMap() {
  static std::map<string, string> names = [] {
    std::map<string, string> ret;
    for (const auto& [chinese, english] : legacyKeyMap()) {
      ret[english] = chinese;
    }
    return ret;
  }();
  return names;
}

/** Get the current English key of the given legacy Chinese key; it is kept for
 * backward compatibility only, do not use it in the new code. Returns an empty
 * string if not found. */
const string& lookupLegacyKey(const string& key) {
  static const string empty;
  for (const auto& [english, chinese] : chineseNameMap()) {
    if (chinese == key) {
      return english;
    }
  }
  return empty;
}

/** Check whether the given key is an English key, i.e. it consists of the
 * printable ASCII characters only. Non-English keys are accepted only as
 * deprecated aliases for lookups.
 */
bool isEnglishKey(const string& key) {
  for (unsigned char ch : key) {
    if (ch < 0x20 || ch > 0x7E) {
      return false;
    }
  }
  return true;
}

}  // namespace

Performance::Performance()
    : keys_({"Account Initial Capital",
             "Total Invested Principal",
             "Total Invested Assets",
             "Total Borrowed Cash",
             "Total Borrowed Assets",
             "Total Dividends",
             "Cash Balance",
             "Open Position Net Value",
             "Current Total Assets",
             "Total Cost of Closed Trades",
             "Total Net Profit of Closed Trades",
             "Max Cash Usage per Trade %",
             "Avg Cash Usage per Trade %",
             "Open Position Account Return %",
             "Closed Trade Account Return %",
             "Account CAGR %",
             "Account Avg Annual Return %",
             "Total Profit of Winning Trades",
             "Total Loss of Losing Trades",
             "Total Closed Trades",
             "Number of Winning Trades",
             "Number of Losing Trades",
             "Win Rate %",
             "Profit Expectancy",
             "Avg Profit per Winning Trade",
             "Avg Loss per Losing Trade",
             "Avg Win / Avg Loss Ratio",
             "Profit Factor",
             "Largest Single Win",
             "Largest Single Win %",
             "Largest Single Loss",
             "Largest Single Loss %",
             "Avg Holding Period of Winning Trades",
             "Max Holding Period of Winning Trades",
             "Avg Holding Period of Losing Trades",
             "Max Holding Period of Losing Trades",
             "Total Time Flat",
             "Time Flat / Total Time %",
             "Avg Time Flat",
             "Max Time Flat",
             "Max Consecutive Wins",
             "Max Consecutive Losses",
             "Max Consecutive Win Amount",
             "Max Consecutive Loss Amount",
             "R-Multiple Expectancy",
             "Trade Opportunities per Year",
             "Annual Expected R-Multiple",
             "Avg R-Multiple of Winning Trades",
             "Avg R-Multiple of Losing Trades",
             "Max Single Win R-Multiple",
             "Max Single Loss R-Multiple",
             "Max Consecutive Win R-Multiple",
             "Max Consecutive Loss R-Multiple"}) {
  for (const auto& key : keys_) {
    result_[key] = 0.0;
  }
}

Performance::~Performance() {}

bool Performance::exist(const string& key) {
  if (result_.count(key) != 0) {
    return true;
  }
  // Backward compatibility with the legacy Chinese key (deprecated)
  return !lookupLegacyKey(key).empty();
}

Performance& Performance::operator=(const Performance& other) noexcept {
  HAYAKU_IF_RETURN(this == &other, *this);
  result_ = other.result_;
  keys_ = other.keys_;
  return *this;
}

Performance& Performance::operator=(Performance&& other) noexcept {
  HAYAKU_IF_RETURN(this == &other, *this);
  result_ = std::move(other.result_);
  keys_ = std::move(other.keys_);
  return *this;
}

void Performance::reset() {
  map_type::iterator iter = result_.begin();
  for (; iter != result_.end(); ++iter) {
    if (!std::isnan(iter->second)) {
      iter->second = 0.0;
    }
  }
}

double Performance::get(const string& name) const {
  auto iter = result_.find(name);
  if (iter != result_.end()) {
    return iter->second;
  }
  // Backward compatibility with the legacy Chinese key (deprecated)
  const string& new_key = lookupLegacyKey(name);
  if (!new_key.empty()) {
    HAYAKU_WARN(
        "Performance - the legacy Chinese key(\"{}\") is deprecated, please "
        "use "
        "\"{}\" instead!",
        name, new_key);
    return result_.at(new_key);
  }
  HAYAKU_WARN("Performance - key({}) not exist!", name);
  return Null<double>();
}

PriceList Performance::values() const {
  PriceList result(result_.size());
  size_t i = 0;
  for (const auto& key : keys_) {
    result[i++] = result_.at(key);
  }
  return result;
}

void Performance::addKey(const string& key, const string& chinese) {
  HAYAKU_ERROR_IF_RETURN(!isEnglishKey(key), void(),
                         "Performance - addKey: only the English key is "
                         "supported, but got \"{}\"!",
                         key);
  if (!chinese.empty()) {
    chineseNameMap()[key] = chinese;
  }
  keys_.push_back(key);
  result_[key] = 0.0;
}

void Performance::setValue(const string& key, double value) {
  HAYAKU_ERROR_IF_RETURN(!isEnglishKey(key), void(),
                         "Performance - setValue: only the English key is "
                         "supported, but got \"{}\"!",
                         key);
  result_[key] = value;
}

string Performance::report() {
  std::stringstream buf;

  buf << std::fixed;
  buf.precision(2);

  buf.setf(std::ios_base::fixed);
  buf.precision(2);
  for (const auto& key : keys_) {
    buf << key << ": " << result_.at(key) << std::endl;
  }

  buf.unsetf(std::ostream::floatfield);
  (void)buf.precision();
  return buf.str();
}

void Performance::statistics(const internal::ExecutionAccountPortPtr& tm,
                             const Datetime& datetime) {
  // Clear the last statistics result
  reset();

  HAYAKU_INFO_IF_RETURN(!tm, void(), "Execution account is null!");
  HAYAKU_ERROR_IF_RETURN(!datetime.isNull() && datetime < tm->lastDatetime(),
                         void(), "datetime must >= tm->lastDatetime !");

  int precision = tm->precision();
  result_["Account Initial Capital"] = tm->initCash();
  FundsRecord funds = tm->getFunds(datetime, KQuery::DAY);
  result_["Cash Balance"] = funds.cash;
  result_["Total Invested Principal"] = funds.base_cash;
  result_["Total Invested Assets"] = funds.base_asset;
  result_["Total Borrowed Cash"] = funds.borrow_cash;
  result_["Total Borrowed Assets"] = funds.borrow_asset;
  result_["Open Position Net Value"] = funds.market_value;
  result_["Current Total Assets"] =
      funds.cash + funds.market_value - funds.borrow_cash - funds.borrow_asset;
  price_t total_money = funds.base_cash + funds.base_asset;

  const TradeRecordList& trade_list = tm->getTradeList();
  TradeRecordList::const_iterator trade_iter = trade_list.begin();
  for (; trade_iter != trade_list.end(); ++trade_iter) {
    if (trade_iter->business == BUSINESS_BONUS) {
      result_["Total Dividends"] += trade_iter->realPrice;
    }
  }

  struct CalData {
    CalData()
        : total_duration(0),
          continues(0),
          max_continues(0),
          continues_money(0.0),
          max_continues_money(0.0),
          total_r(0.0),
          max_continues_r(0.0) {}

    int total_duration;           // Total holding duration
    int continues;                // Current consecutive holding duration
    int max_continues;            // Maximum number of the consecutive holdings
    price_t continues_money;      // Current consecutive holding profit or loss
    price_t max_continues_money;  // Maximum consecutive holding profit or loss
    price_t total_r;              // Accumulated R multiple
    price_t max_continues_r;  // Sum of the R multiples of the max consecutive
                              // profits/losses
  };

  CalData earn, loss;

  bool pre_earn = true;
  const PositionRecordList& his_position = tm->getHistoryPositionList();
  price_t total_r = 0.0;
  result_["Total Closed Trades"] = (double)his_position.size();
  PositionRecordList::const_iterator his_iter = his_position.begin();
  for (; his_iter != his_position.end(); ++his_iter) {
    const PositionRecord& pos = *his_iter;
    result_["Total Cost of Closed Trades"] += pos.totalCost;

    price_t profit =
        roundEx(pos.sellMoney - pos.totalCost - pos.buyMoney, precision);
    result_["Total Net Profit of Closed Trades"] = roundEx(
        result_["Total Net Profit of Closed Trades"] + profit, precision);

    price_t profit_percent = profit / (pos.buyMoney + pos.totalCost) * 100.;

    price_t r = roundEx(profit / pos.totalRisk, precision);
    total_r += r;

    if (profit > 0.0) {
      result_["Number of Winning Trades"]++;
      result_["Total Profit of Winning Trades"] = roundEx(
          profit + result_["Total Profit of Winning Trades"], precision);
      if (profit > result_["Largest Single Win"]) {
        result_["Largest Single Win"] = profit;
      }

      if (profit_percent > result_["Largest Single Win %"]) {
        result_["Largest Single Win %"] = profit_percent;
      }

      int duration =
          (pos.cleanDatetime.date() - pos.takeDatetime.date()).days();
      earn.total_duration += duration;
      if (duration > result_["Max Holding Period of Winning Trades"]) {
        result_["Max Holding Period of Winning Trades"] = duration;
      }

      earn.total_r += r;
      if (r > result_["Max Single Win R-Multiple"]) {
        result_["Max Single Win R-Multiple"] = r;
      }

      // The last trade was a profitable trade
      if (pre_earn) {
        earn.continues++;
        earn.continues_money =
            roundEx(profit + earn.continues_money, precision);
        if (earn.continues >= earn.max_continues) {
          earn.max_continues = earn.continues;
          if (earn.continues_money > earn.max_continues_money) {
            earn.max_continues_money = earn.continues_money;
            earn.max_continues_r += r;
          }
        }
      } else {
        earn.continues = 1;
        earn.continues_money = profit;
        earn.max_continues = 1;
        if (profit > earn.max_continues_money) {
          earn.max_continues_money = profit;
          earn.max_continues_r = r;
        }
      }

      pre_earn = true;

    } else {
      // The one that made no money is recorded as a losing trade
      result_["Number of Losing Trades"]++;
      result_["Total Loss of Losing Trades"] =
          roundEx(profit + result_["Total Loss of Losing Trades"], precision);
      if (profit < result_["Largest Single Loss"]) {
        result_["Largest Single Loss"] = profit;
      }

      if (profit_percent < result_["Largest Single Loss %"]) {
        result_["Largest Single Loss %"] = profit_percent;
      }

      int duration =
          (pos.cleanDatetime.date() - pos.takeDatetime.date()).days();
      loss.total_duration += duration;
      if (duration > result_["Max Holding Period of Losing Trades"]) {
        result_["Max Holding Period of Losing Trades"] = duration;
      }

      loss.total_r += r;
      if (r < result_["Max Single Loss R-Multiple"]) {
        result_["Max Single Loss R-Multiple"] = r;
      }

      // The last one was a losing trade
      if (!pre_earn) {
        loss.continues++;
        loss.continues_money =
            roundEx(profit + loss.continues_money, precision);
        if (loss.continues >= loss.max_continues) {
          loss.max_continues = loss.continues;
          if (loss.continues_money < loss.max_continues_money) {
            loss.max_continues_money = loss.continues_money;
            loss.max_continues_r += r;
          }
        }

      } else {
        loss.continues = 1;
        loss.continues_money = profit;
        loss.max_continues = 1;
        if (profit < loss.max_continues_money) {
          loss.max_continues_money = profit;
          loss.max_continues_r = r;
        }
      }

      pre_earn = false;
    }
  }

  result_["Max Consecutive Wins"] = earn.max_continues;
  result_["Max Consecutive Win Amount"] = earn.max_continues_money;
  result_["Max Consecutive Losses"] = loss.max_continues;
  result_["Max Consecutive Loss Amount"] = loss.max_continues_money;

  if (result_["Max Consecutive Wins"] != 0.0) {
    result_["Max Consecutive Win R-Multiple"] = roundEx(
        earn.max_continues_r / result_["Max Consecutive Wins"], precision);
  }

  if (result_["Max Consecutive Losses"] != 0.0) {
    result_["Max Consecutive Loss R-Multiple"] = roundEx(
        loss.max_continues_r / result_["Max Consecutive Losses"], precision);
  }

  if (result_["Total Invested Principal"] != 0.0) {
    result_["Open Position Account Return %"] =
        100. *
        (result_["Current Total Assets"] / result_["Total Invested Principal"] -
         1.);
    result_["Closed Trade Account Return %"] =
        100. * result_["Total Net Profit of Closed Trades"] /
        result_["Total Invested Principal"];
  }

  if (result_["Number of Winning Trades"] != 0.0) {
    result_["Avg Profit per Winning Trade"] =
        roundEx(result_["Total Profit of Winning Trades"] /
                    result_["Number of Winning Trades"],
                precision);
    result_["Avg Holding Period of Winning Trades"] =
        earn.total_duration / result_["Number of Winning Trades"];
    result_["Avg R-Multiple of Winning Trades"] =
        roundEx(earn.total_r / result_["Number of Winning Trades"], precision);
  }

  if (result_["Number of Losing Trades"] != 0.0) {
    result_["Avg Loss per Losing Trade"] =
        roundEx(result_["Total Loss of Losing Trades"] /
                    result_["Number of Losing Trades"],
                precision);
    result_["Avg Holding Period of Losing Trades"] =
        loss.total_duration / result_["Number of Losing Trades"];
    result_["Avg R-Multiple of Losing Trades"] =
        roundEx(loss.total_r / result_["Number of Losing Trades"], precision);
  }

  if (result_["Avg Loss per Losing Trade"] != 0.0) {
    result_["Avg Win / Avg Loss Ratio"] =
        roundEx(result_["Avg Profit per Winning Trade"] /
                    std::fabs(result_["Avg Loss per Losing Trade"]),
                precision);
  }

  if (result_["Total Closed Trades"] != 0.0) {
    result_["Win Rate %"] = 100 * result_["Number of Winning Trades"] /
                            result_["Total Closed Trades"];
    result_["R-Multiple Expectancy"] =
        roundEx(total_r / result_["Total Closed Trades"], precision);
  }

  if (result_["Total Loss of Losing Trades"] != 0.0) {
    result_["Profit Factor"] =
        result_["Total Profit of Winning Trades"] /
        std::fabs(result_["Total Loss of Losing Trades"]);
  }

  result_["Profit Expectancy"] =
      0.01 * result_["Win Rate %"] * result_["Avg Profit per Winning Trade"] +
      (1 - 0.01 * result_["Win Rate %"]) * result_["Avg Loss per Losing Trade"];

  int64_t duration = 0;
  if (tm->firstDatetime() != Null<Datetime>()) {
    if (datetime == Null<Datetime>()) {
      duration = (Datetime::now() - tm->firstDatetime()).days() + 1;
    } else {
      duration = (datetime - tm->firstDatetime()).days() + 1;
    }
  }

  double years = duration / 365.0;

  if (duration > 1) {
    result_["Trade Opportunities per Year"] =
        result_["Total Closed Trades"] / years;
    result_["Annual Expected R-Multiple"] =
        roundEx(result_["R-Multiple Expectancy"] *
                    result_["Trade Opportunities per Year"],
                precision);
  }

  if (total_money != 0.0 && years != 0.0) {
    result_["Account Avg Annual Return %"] =
        100 * (((result_["Current Total Assets"] / total_money) - 1) / years);
    result_["Account CAGR %"] =
        100 *
        ((std::pow(10,
                   (std::log10(result_["Current Total Assets"] / total_money) /
                    years)) -
          1));
  }

  double max_percent = 0.0, sum_percent = 0.0;
  int trade_number = 0;
  trade_iter = trade_list.begin();
  for (; trade_iter != trade_list.end(); ++trade_iter) {
    if (trade_iter->business == BUSINESS_BUY) {
      trade_number++;
      const TradeRecord& record = *trade_iter;
      price_t hold_cash = roundEx(
          record.realPrice * record.number + record.cost.total, precision);
      price_t total_cash = roundEx(hold_cash + record.cash, precision);
      double percent = (total_cash != 0.0) ? hold_cash / total_cash : 0.0;
      sum_percent += percent;
      if (percent > max_percent) {
        max_percent = percent;
      }
    }
  }

  result_["Max Cash Usage per Trade %"] = 100 * max_percent;
  if (trade_number != 0) {
    result_["Avg Cash Usage per Trade %"] = 100 * sum_percent / trade_number;
  }

  PositionRecordList cur_position = tm->getPositionList();
  int total_short_days = 0;

  if (tm->firstDatetime() != Null<Datetime>()) {
    int short_number = 0;
    int short_days = 0;
    int max_short_days = 0;
    bool pre_short = false;
    Datetime end_day;
    if (datetime == Null<Datetime>()) {
      end_day = Datetime(tm->lastDatetime().date() + bd::days(1));
    } else {
      end_day = Datetime(datetime.date() + bd::days(1));
    }

    DatetimeList day_range = getDateRange(tm->firstDatetime(), end_day);
    DatetimeList::const_iterator day_iter = day_range.begin();
    for (; day_iter != day_range.end(); ++day_iter) {
      bool hold = false;
      his_iter = his_position.begin();
      for (; his_iter != his_position.end(); ++his_iter) {
        if (his_iter->takeDatetime <= *day_iter &&
            *day_iter < his_iter->cleanDatetime) {
          hold = true;
          break;
        }
      }

      if (hold) {
        if (pre_short) {
          short_days = 0;
          pre_short = false;
        }
        continue;
      }

      // It is currently an empty position
      total_short_days++;
      if (pre_short) {
        short_days++;
        if (short_days > max_short_days) {
          max_short_days = short_days;
        }
      } else {
        short_number++;
        pre_short = true;
      }
    }

    result_["Total Time Flat"] = total_short_days;
    result_["Max Time Flat"] = max_short_days;
    if (day_range.size() != 0) {
      result_["Time Flat / Total Time %"] =
          100 * total_short_days / day_range.size();
    }
    if (short_number != 0) {
      result_["Avg Time Flat"] = total_short_days / short_number;
    }
  }
}

} /* namespace hayaku */
