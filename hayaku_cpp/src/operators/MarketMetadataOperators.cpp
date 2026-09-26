#include "MarketOperators.h"

/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-17
 *      Author: fasiondog
 */

#include "Indicator.h"

namespace hayaku {

// Cumulative backward adjustment ratio factor
class IAdjFactor : public IndicatorImp {
  INDICATOR_IMP(IAdjFactor)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IAdjFactor();
  virtual ~IAdjFactor() override;

  virtual bool supportIncrementCalculate() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-17
 *      Author: fasiondog
 */

#include <algorithm>
#include <vector>

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IAdjFactor)
#endif

namespace hayaku {

IAdjFactor::IAdjFactor() : IndicatorImp("ADJ_FACTOR", 1) {
  need_context_ = true;
}

IAdjFactor::~IAdjFactor() {}

// Calculate the daily-line adjustment factor (the listing date is not strict,
// the calculation starts from the data contained in in_kdata). Note that as an
// internal function it returns an empty result directly when there is no
// ex-rights/ex-dividend data. base_factor: the base factor value used for the
// incremental calculation (1.0 by default, which means calculating from the
// beginning). Refer to the implementation of
// KDataPrivatedBufferImp::_recoverEqualBackward().
static vector<std::pair<Datetime, Indicator::value_t>> cum_adj_factor(
    const KData& in_kdata, price_t base_factor = 1.0) {
  // Get the K-line data of the corresponding daily-line range
  KData kdata = in_kdata.getKData(KQuery::DAY);

  size_t total = kdata.size();
  vector<std::pair<Datetime, Indicator::value_t>> result;
  result.reserve(total);
  HAYAKU_IF_RETURN(total == 0, result);

  auto* krecords = kdata.data();

  // Get all the ex-rights/ex-dividend data (already sorted by date)
  Datetime start_date = krecords[0].datetime;
  Datetime end_date = krecords[total - 1].datetime + Days(1);
  StockWeightList sw_list = kdata.getStock().getWeight(start_date, end_date);

  // if (sw_list.empty()) {
  //     // There is no ex-rights/ex-dividend data, return an empty result
  //     return result;
  // }

  // Initialize all the factors to the base factor
  vector<price_t> factors(total, base_factor);

  StockWeightList::const_reverse_iterator weightIter = sw_list.rbegin();
  size_t pre_pos = total - 1;

  for (; weightIter != sw_list.rend(); ++weightIter) {
    // Find the position of the ex-rights date in the K-lines
    size_t i = pre_pos;
    while (i > 0 && krecords[i].datetime > weightIter->datetime()) {
      i--;
    }

    pre_pos = i;  // The ex-rights date position

    // Get the close price of the record date (the day before the ex-rights
    // date)
    if (pre_pos == 0) {
      continue;  // There is no data of the previous day, it cannot be
                 // calculated
    }

    price_t closePrice = krecords[pre_pos - 1].closePrice;
    if (closePrice <= 0.0) {
      continue;
    }

    price_t denominator = 0.0, temp = closePrice;
    if (weightIter->suogu() != 0.0) {
      denominator = weightIter->suogu();
    } else {
      // The change ratio of the outstanding shares = 0.1 * (bonus shares +
      // rights shares + capitalized shares)
      price_t change =
          0.1 * (weightIter->countAsGift() + weightIter->countForSell() +
                 weightIter->increasement());
      denominator = 1.0 + change;
      temp = closePrice + weightIter->priceForSell() * change -
             0.1 * weightIter->bonus();
    }

    if (temp == 0.0 || denominator == 0.0) {
      continue;
    }

    // Calculate the backward adjustment coefficient
    price_t k = (denominator * closePrice) / temp;

    // Multiply all the factors from the ex-rights date to the latest date by k
    for (size_t j = pre_pos; j < total; ++j) {
      factors[j] *= k;
    }
  }

  // Build the result
  for (size_t i = 0; i < total; ++i) {
    result.emplace_back(krecords[i].datetime, factors[i]);
  }

  return result;
}

void IAdjFactor::_calculate(const Indicator& ind) {
  HAYAKU_WARN_IF(!isLeaf() && !ind.empty(),
                 "The input is ignored because {} depends on the context!",
                 name_);

  const KData& k = getContext();
  size_t total = k.size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 1);
  discard_ = 0;

  // Reuse the incremental calculation method; start_pos = 0 means calculating
  // from the beginning
  _increment_calculate(ind, 0);
}

bool IAdjFactor::supportIncrementCalculate() const { return true; }

void IAdjFactor::_increment_calculate(const Indicator& ind, size_t start_pos) {
  HAYAKU_WARN_IF(!isLeaf() && !ind.empty(),
                 "The input is ignored because {} depends on the context!",
                 name_);

  const KData& k = getContext();
  size_t total = k.size();
  HAYAKU_IF_RETURN(total == 0 || start_pos >= total, void());

  auto* dst = data();
  auto* kdata = k.data();

  // Key: the adjustment factor must be calculated based on the daily line
  // Optimization strategy: use the already calculated base factor and calculate
  // only the ex-rights/ex-dividend influence of the newly added part

  // 1. Determine the base factor and the start date
  price_t base_factor = 1.0;
  Datetime calc_start_date;

  if (start_pos > 0) {
    // Use the factor value of the previous position as the base
    base_factor = dst[start_pos - 1];
    calc_start_date = kdata[start_pos - 1].datetime.startOfDay();
  } else if (!old_context_.empty()) {
    // The first incremental calculation with an old context: use the start date
    // of the old context
    base_factor = 1.0;  // Calculate from the beginning
    calc_start_date = old_context_[0].datetime.startOfDay();
  } else {
    // The real first calculation, starting from the first K-line
    calc_start_date = kdata[0].datetime.startOfDay();
  }

  Datetime calc_end_date =
      kdata[total - 1].datetime + k.getQuery().kTypeInSeconds();

  // 2. Get the K-line data corresponding to the incremental part (starting from
  // calc_start_date)
  KData inc_kdata =
      k.getStock().getKData(KQueryByDate(calc_start_date, calc_end_date));

  if (inc_kdata.empty()) {
    // There is no data, the whole incremental part uses the base factor
    for (size_t i = start_pos; i < total; ++i) {
      dst[i] = base_factor;
    }
    return;
  }

  // 3. Calculate the daily-line adjustment factor of the incremental part (the
  // base factor is passed in)
  auto daily_factors = cum_adj_factor(inc_kdata, base_factor);
  if (k.getQuery().kType() == KQuery::DAY) {
    for (size_t i = start_pos; i < total; ++i) {
      dst[i] = daily_factors[i - start_pos].second;
    }
    return;
  }

  if (daily_factors.empty()) {
    // There is no ex-rights/ex-dividend data, the whole incremental part uses
    // the base factor
    for (size_t i = start_pos; i < total; ++i) {
      dst[i] = base_factor;
    }
    return;
  }

  // 4. Align the daily-line adjustment factor to the current K-line period,
  // starting from start_pos
  size_t d_idx = 0;
  Datetime start_date = kdata[start_pos].datetime.startOfDay();

  // Find the index of the daily-line factor matching the date of start_pos
  while (d_idx < daily_factors.size() &&
         daily_factors[d_idx].first < start_date) {
    ++d_idx;
  }

  // 5. Fill the incremental part starting from start_pos
  price_t cumulative_factor = base_factor;

  for (size_t i = start_pos; i < total; ++i) {
    const Datetime& k_date = kdata[i].datetime.startOfDay();

    // Process all the daily-line factors earlier than or equal to the current
    // K-line date
    while (d_idx < daily_factors.size() &&
           daily_factors[d_idx].first <= k_date) {
      cumulative_factor = daily_factors[d_idx].second;
      ++d_idx;
    }

    // Set the adjustment factor of the current K-line
    dst[i] = cumulative_factor;
  }
}

Indicator HAYAKU_API ADJ_FACTOR() {
  return Indicator(make_shared<IAdjFactor>());
}

Indicator HAYAKU_API ADJ_FACTOR(const KData& k) {
  auto p = make_shared<IAdjFactor>();
  p->setContext(k);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-04-10
 *      Author: Jet
 */

namespace hayaku {

/* Return whether the security code matches the given pattern, in the form of an
 * indicator */
class ICodeLike : public IndicatorImp {
  INDICATOR_IMP(ICodeLike)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ICodeLike();
  virtual ~ICodeLike() override;
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-04-10
 *      Author: Jet
 */

#include "IndicatorSupport.h"
#include "data/Stock.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ICodeLike)
#endif

namespace hayaku {

ICodeLike::ICodeLike() : IndicatorImp("CODELIKE", 1) {
  need_context_ = true;
  setParam<string>("pattern", "");
}

ICodeLike::~ICodeLike() {}

void ICodeLike::_calculate(const Indicator& data) {
  HAYAKU_IF_RETURN(!isLeaf() && !data.empty(), void());

  const KData& k = getContext();
  size_t total = k.size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 1);

  string pattern = getParam<string>("pattern");
  string code = k.getStock().code();
  value_t match = wildcardMatch(code, pattern) ? 1.0 : 0.0;

  auto* dst = this->data();
  for (size_t i = 0; i < total; ++i) {
    dst[i] = match;
  }
}

void ICodeLike::_increment_calculate(const Indicator& data, size_t start_pos) {
  const KData& k = getContext();
  size_t total = k.size();

  string pattern = getParam<string>("pattern");
  string code = k.getStock().code();
  value_t match = wildcardMatch(code, pattern) ? 1.0 : 0.0;

  auto* dst = this->data();
  for (size_t i = start_pos; i < total; ++i) {
    dst[i] = match;
  }
}

Indicator HAYAKU_API CODELIKE(const string& pattern) {
  auto p = make_shared<ICodeLike>();
  p->setParam<string>("pattern", pattern);
  return Indicator(p);
}

Indicator HAYAKU_API CODELIKE(const KData& k, const string& pattern) {
  auto p = make_shared<ICodeLike>();
  p->setParam<string>("pattern", pattern);
  p->setContext(k);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */

namespace hayaku {

/* Get the turnover rate, it equals VOL(k) / CAPITAL(k) */
class ICycle : public IndicatorImp {
  INDICATOR_IMP(ICycle)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ICycle();
  virtual ~ICycle() override;
  virtual void _checkParam(const string& name) const override;

 private:
  void _initParams();
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */

#include "SeriesOperators.h"
#include "operators/SeriesOperators.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ICycle)
#endif

namespace hayaku {

ICycle::ICycle() : IndicatorImp("CYCLE", 1) {
  need_context_ = true;
  _initParams();
}

ICycle::~ICycle() {}

void ICycle::_initParams() {
  setParam<int>("adjust_cycle", 1);              // Position adjustment cycle
  setParam<string>("adjust_mode", "query");      // Position adjustment mode
  setParam<bool>("delay_to_trading_day", true);  // Delay to the trading day
}

void ICycle::_checkParam(const string& name) const {
  if ("adjust_mode" == name || "adjust_cycle" == name) {
    if (!haveParam("adjust_mode") || !haveParam("adjust_cycle")) {
      // When the two parameters are judged at the same time, one of them may
      // not have been set yet
      return;
    }
    string adjust_mode = getParam<string>("adjust_mode");
    to_lower(adjust_mode);
    int adjust_cycle = getParam<int>("adjust_cycle");
    if ("query" == adjust_mode) {
      HAYAKU_ASSERT(adjust_cycle >= 1);
    } else if ("day" == adjust_mode) {
      HAYAKU_ASSERT(adjust_cycle >= 1);
    } else if ("week" == adjust_mode) {
      HAYAKU_ASSERT(adjust_cycle >= 1 && adjust_cycle <= 5);
    } else if ("month" == adjust_mode) {
      HAYAKU_ASSERT(adjust_cycle >= 1 && adjust_cycle <= 31);
    } else if ("quarter" == adjust_mode) {
      HAYAKU_ASSERT(adjust_cycle >= 1 && adjust_cycle <= 92);
    } else if ("year" == adjust_mode) {
      HAYAKU_ASSERT(adjust_cycle >= 1 && adjust_cycle <= 366);
    } else {
      HAYAKU_THROW("Invalid adjust_mode: {}!", adjust_mode);
    }
  }
}

static void calculate_no_delay(const DatetimeList& datelist, int adjust_cycle,
                               const string& mode, PriceList& buf) {
  if ("week" == mode) {
    Datetime cur_cycle_end = datelist.front().nextWeek();
    for (size_t i = 0, total = datelist.size(); i < total; i++) {
      const auto& date = datelist[i];
      bool adjust = (date.dayOfWeek() == adjust_cycle);
      if (adjust) {
        cur_cycle_end = date.nextWeek();
      }
      if (cur_cycle_end >= datelist.back()) {
        cur_cycle_end = datelist.back() + Minutes(1);
      }
      buf[i] = adjust;
    }
  } else if ("month" == mode) {
    Datetime cur_cycle_end = datelist.front().nextMonth();
    for (size_t i = 0, total = datelist.size(); i < total; i++) {
      const auto& date = datelist[i];
      bool adjust = (date.day() == adjust_cycle);
      if (adjust) {
        cur_cycle_end = date.nextMonth();
      }
      if (cur_cycle_end >= datelist.back()) {
        cur_cycle_end = datelist.back() + Minutes(1);
      }
      buf[i] = adjust;
    }
  } else if ("quarter" == mode) {
    Datetime cur_cycle_end = datelist.front().nextQuarter();
    for (size_t i = 0, total = datelist.size(); i < total; i++) {
      const auto& date = datelist[i];
      bool adjust = (date.day() == adjust_cycle);
      if (adjust) {
        cur_cycle_end = date.nextQuarter();
      }
      if (cur_cycle_end >= datelist.back()) {
        cur_cycle_end = datelist.back() + Minutes(1);
      }
      buf[i] = adjust;
    }
  } else if ("year" == mode) {
    Datetime cur_cycle_end = datelist.front().nextYear();
    for (size_t i = 0, total = datelist.size(); i < total; i++) {
      const auto& date = datelist[i];
      bool adjust = (date.dayOfYear() == adjust_cycle);
      if (adjust) {
        cur_cycle_end = date.nextYear();
      }
      if (cur_cycle_end >= datelist.back()) {
        cur_cycle_end = datelist.back() + Minutes(1);
      }
      buf[i] = adjust;
    }
  }
}

static void calculate_delay(const DatetimeList& datelist, int adjust_cycle,
                            const string& mode, PriceList& buf) {
  std::set<Datetime> adjust_date_set;
  if ("week" == mode) {
    Datetime cur_cycle_end = datelist.front().nextWeek();
    for (size_t i = 0, total = datelist.size(); i < total; i++) {
      const auto& date = datelist[i];
      Datetime adjust_date = date.startOfWeek() + Days(adjust_cycle - 1);
      bool adjust = false;
      if (date == adjust_date) {
        adjust = true;
        adjust_date_set.emplace(adjust_date);
      } else if (adjust_date_set.find(adjust_date) == adjust_date_set.end() &&
                 date > adjust_date) {
        adjust = true;
        adjust_date_set.emplace(adjust_date);
      }

      if (adjust) {
        cur_cycle_end = date.nextWeek();
      }
      if (cur_cycle_end >= datelist.back()) {
        cur_cycle_end = datelist.back() + Minutes(1);
      }

      buf[i] = adjust;
    }

  } else if ("month" == mode) {
    Datetime cur_cycle_end = datelist.front().nextMonth();
    for (size_t i = 0, total = datelist.size(); i < total; i++) {
      const auto& date = datelist[i];
      Datetime adjust_date = date.startOfMonth() + Days(adjust_cycle - 1);
      bool adjust = false;
      if (date == adjust_date) {
        adjust = true;
        adjust_date_set.emplace(adjust_date);
      } else if (adjust_date_set.find(adjust_date) == adjust_date_set.end() &&
                 date > adjust_date) {
        adjust = true;
        adjust_date_set.emplace(adjust_date);
      }

      if (adjust) {
        cur_cycle_end = date.nextMonth();
      }
      if (cur_cycle_end >= datelist.back()) {
        cur_cycle_end = datelist.back() + Minutes(1);
      }

      buf[i] = adjust;
    }

  } else if ("quarter" == mode) {
    Datetime cur_cycle_end = datelist.front().nextQuarter();
    for (size_t i = 0, total = datelist.size(); i < total; i++) {
      const auto& date = datelist[i];
      Datetime adjust_date = date.startOfQuarter() + Days(adjust_cycle - 1);
      bool adjust = false;
      if (date == adjust_date) {
        adjust = true;
        adjust_date_set.emplace(adjust_date);
      } else if (adjust_date_set.find(adjust_date) == adjust_date_set.end() &&
                 date > adjust_date) {
        adjust = true;
        adjust_date_set.emplace(adjust_date);
      }

      if (adjust) {
        cur_cycle_end = date.nextQuarter();
      }
      if (cur_cycle_end >= datelist.back()) {
        cur_cycle_end = datelist.back() + Minutes(1);
      }

      buf[i] = adjust;
    }

  } else if ("year" == mode) {
    Datetime cur_cycle_end = datelist.front().nextYear();
    for (size_t i = 0, total = datelist.size(); i < total; i++) {
      const auto& date = datelist[i];
      Datetime adjust_date = date.startOfYear() + Days(adjust_cycle - 1);
      bool adjust = false;
      if (date == adjust_date) {
        adjust = true;
        adjust_date_set.emplace(adjust_date);
      } else if (adjust_date_set.find(adjust_date) == adjust_date_set.end() &&
                 date > adjust_date) {
        adjust = true;
        adjust_date_set.emplace(adjust_date);
      }

      if (adjust) {
        cur_cycle_end = date.nextYear();
      }
      if (cur_cycle_end >= datelist.back()) {
        cur_cycle_end = datelist.back() + Minutes(1);
      }

      buf[i] = adjust;
    }
  }
}

void ICycle::_calculate(const Indicator& data) {
  HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                 "The input is ignored because {} depends on the context!",
                 name_);

  const KData& k = getContext();
  size_t total = k.size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 1);

  DatetimeList datelist = k.getStock().getTradingCalendar(k.getQuery());
  HAYAKU_IF_RETURN(datelist.empty(), void());

  int adjust_cycle = getParam<int>("adjust_cycle");
  string adjust_mode = getParam<string>("adjust_mode");
  bool delay_to_trading_day = getParam<bool>("delay_to_trading_day");

  PriceList buf(datelist.size());
  if ("query" == adjust_mode || "day" == adjust_mode) {
    size_t cur_adjust_ix = 0;
    for (size_t i = 0, len = datelist.size(); i < len; i++) {
      bool adjust = false;
      if (i == cur_adjust_ix) {
        adjust = true;
        cur_adjust_ix += adjust_cycle;
      }
      buf[i] = adjust;
    }

  } else if (delay_to_trading_day) {
    calculate_delay(datelist, adjust_cycle, adjust_mode, buf);
  } else {
    calculate_no_delay(datelist, adjust_cycle, adjust_mode, buf);
  }

  Indicator tmpind = ALIGN(PRICELIST(buf, std::move(datelist)), k);
  const auto* src = tmpind.data();
  auto* dst = this->data();
  HAYAKU_ASSERT(tmpind.size() == total);
  memcpy(dst, src, sizeof(value_t) * total);
}

Indicator HAYAKU_API CYCLE(int adjust_cycle, const string& adjust_mode,
                           bool delay_to_trading_day) {
  auto p = make_shared<ICycle>();
  p->setParam<int>("adjust_cycle", adjust_cycle);
  p->setParam<string>("adjust_mode", adjust_mode);
  p->setParam<bool>("delay_to_trading_day", delay_to_trading_day);
  return Indicator(p);
}

Indicator HAYAKU_API CYCLE(const KData& k, int adjust_cycle,
                           const string& adjust_mode,
                           bool delay_to_trading_day) {
  auto p = make_shared<ICycle>();
  p->setParam<int>("adjust_cycle", adjust_cycle);
  p->setParam<string>("adjust_mode", adjust_mode);
  p->setParam<bool>("delay_to_trading_day", delay_to_trading_day);
  p->setContext(k);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-13
 *      Author: fasiondog
 */

namespace hayaku {

class IFinance : public IndicatorImp {
  INDICATOR_IMP(IFinance)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IFinance();
  virtual ~IFinance() override = default;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-13
 *      Author: fasiondog
 */

#include "data/DataRuntime.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IFinance)
#endif

namespace hayaku {

IFinance::IFinance() : IndicatorImp("FINANCE", 1) {
  need_context_ = true;
  setParam<int>("field_ix", 0);
  setParam<string>("field_name", "");

  // Some information such as the earnings per share is calculated with the
  // annual report only
  setParam<bool>("only_year_report", false);

  // Some information such as the earnings per share needs a dynamic
  // calculation; for example, when only the Q1 report exists, the annual
  // earnings is estimated with the Q1 report * 4
  setParam<bool>("dynamic", false);
}

void IFinance::_calculate(const Indicator& data) {
  HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                 "The input is ignored because {} depends on the context!",
                 name_);

  const KData& kdata = getContext();
  size_t total = kdata.size();
  if (total == 0) {
    return;
  }

  _readyBuffer(total, 1);
  _increment_calculate(data, 0);
}

void IFinance::_increment_calculate(const Indicator& data, size_t start_pos) {
  const KData& kdata = getContext();
  size_t total = kdata.size();
  auto finances = kdata.getStock().getHistoryFinance();

  if (getParam<bool>("only_year_report")) {
    vector<HistoryFinanceInfo> tmp_finances;
    for (auto&& finance : finances) {
      if (finance.fileDate.month() == 12L) {
        tmp_finances.emplace_back(std::move(finance));
      }
    }
    finances = std::move(tmp_finances);
  }

  if (finances.empty()) {
    discard_ = total;
    return;
  }

  int field_ix = getParam<int>("field_ix");
  string field_name = getParam<string>("field_name");
  if (field_ix < 0 && !field_name.empty()) {
    field_ix = static_cast<int>(getDataRuntime().getHistoryFinanceFieldIndex(
        getParam<string>("field_name")));
  }

  bool dynamic = getParam<bool>("dynamic");
  auto* dst = this->data();
  const auto* k = kdata.data();

  size_t finances_total = finances.size();

  // Fixed #25 (gitee): the quarterly report and the annual report may conflict
  // when published on the same day the problem that FINANCE(kdata, 231) and
  // FINANCE(kdata, 95) have no annual report data (231 is the fourth quarter)
  for (size_t i = finances_total - 1; i > 0; --i) {
    if (finances[i - 1].reportDate >= finances[i].reportDate) {
      finances[i - 1].reportDate = finances[i].reportDate - TimeDelta(1);
    }
  }

  size_t cur_kix = start_pos;
  size_t pos = 0;
  while (pos < finances_total && cur_kix < total) {
    auto value = finances[pos].values.at(field_ix);
    if (pos + 1 == finances_total) {
      while (cur_kix < total &&
             finances[pos].reportDate <= k[cur_kix].datetime) {
        if (dynamic) {
          long month = finances[pos].fileDate.month();
          if (3L == month) {
            // Q1 report
            dst[cur_kix] = value * 4;
          } else if (6L == month) {
            // Half-year report
            dst[cur_kix] = value * 2;
          } else if (9L == month) {
            // Q3 report
            dst[cur_kix] = value / 3.0 * 4.0;
          } else {
            // Annual report
            dst[cur_kix] = value;
          }
        } else {
          dst[cur_kix] = value;
        }
        cur_kix++;
      }
    } else {
      while (cur_kix < total &&
             finances[pos].reportDate <= k[cur_kix].datetime &&
             finances[pos + 1].reportDate > k[cur_kix].datetime) {
        if (dynamic) {
          long month = finances[pos].fileDate.month();
          if (3L == month) {
            // Q1 report
            dst[cur_kix] = value * 4;
          } else if (6L == month) {
            // Half-year report
            dst[cur_kix] = value * 2;
          } else if (9L == month) {
            // Q3 report
            dst[cur_kix] = value / 3.0 * 4.0;
          } else {
            // Annual report
            dst[cur_kix] = value;
          }
        } else {
          dst[cur_kix] = value;
        }
        cur_kix++;
      }
    }
    pos++;
  }
}

Indicator HAYAKU_API FINANCE(int field_ix) {
  auto p = make_shared<IFinance>();
  p->setParam<int>("field_ix", field_ix);
  return Indicator(p);
}

Indicator HAYAKU_API FINANCE(const KData& k, int field_ix) {
  auto p = make_shared<IFinance>();
  p->setParam<int>("field_ix", field_ix);
  p->setContext(k);
  return Indicator(p);
}

Indicator HAYAKU_API FINANCE(const string& field_name) {
  auto p = make_shared<IFinance>();
  p->setParam<int>("field_ix", -1);
  p->setParam<string>("field_name", field_name);
  return Indicator(p);
}

Indicator HAYAKU_API FINANCE(const KData& k, const string& field_name) {
  auto p = make_shared<IFinance>();
  p->setParam<int>("field_ix", -1);
  p->setParam<string>("field_name", field_name);
  p->setContext(k);
  return Indicator(p);
}

}  // namespace hayaku

/*
 * ILiuTongPang.h
 *
 *  Created on: 2019-3-6
 *      Author: fasiondog
 */

namespace hayaku {

class ILiuTongPan : public IndicatorImp {
  INDICATOR_IMP(ILiuTongPan)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ILiuTongPan();
  virtual ~ILiuTongPan() override;
};

} /* namespace hayaku */

/*
 * ILiuTongPang.cpp
 *
 *  Created on: 2019-3-6
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ILiuTongPan)
#endif

namespace hayaku {

ILiuTongPan::ILiuTongPan() : IndicatorImp("LIUTONGPAN", 1) {
  need_context_ = true;
}

ILiuTongPan::~ILiuTongPan() {}

void ILiuTongPan::_calculate(const Indicator& data) {
  HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                 "The input is ignored because {} depends on the context!",
                 name_);

  const KData& k = getContext();
  size_t total = k.size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 1);

  // Set the discard to everything first, it is updated later
  discard_ = total;

  Stock stock = k.getStock();
  auto* kdata = k.data();
  Datetime lastdate = kdata[total - 1].datetime.startOfDay();

  StockWeightList sw_list =
      stock.getWeight(Datetime::min(), lastdate + Days(1));
  HAYAKU_IF_RETURN(sw_list.empty(), void());

  // Find the first ex-rights/ex-dividend record whose outstanding shares are
  // not 0
  price_t pre_free_count = 0.0;
  Datetime pre_sw_date;
  auto sw_iter = sw_list.begin();
  for (; sw_iter != sw_list.end(); ++sw_iter) {
    if (sw_iter->freeCount() > 0) {
      pre_free_count = sw_iter->freeCount();
      pre_sw_date = sw_iter->datetime();
      break;
    }
  }

  // Return directly when there is no ex-rights/ex-dividend data with
  // outstanding shares, or when the date of that record is later than the last
  // K-line date
  HAYAKU_IF_RETURN(sw_iter == sw_list.end() || pre_sw_date > lastdate, void());

  auto* dst = this->data();
  size_t pos = 0;
  for (; sw_iter != sw_list.end(); ++sw_iter) {
    price_t free_count = sw_iter->freeCount();
    Datetime cur_sw_date = sw_iter->datetime();
    if (free_count <= 0.0) {
      continue;  // Ignore the ex-rights/ex-dividend record whose outstanding
                 // shares are 0
    }

    while (pos < total && kdata[pos].datetime < cur_sw_date) {
      if (kdata[pos].datetime >= pre_sw_date) {
        dst[pos] = pre_free_count;
      }
      pos++;
    }

    pre_free_count = free_count;
    pre_sw_date = cur_sw_date;
    if (pos >= total) {
      break;
    }
  }

  for (; pos < total; pos++) {
    dst[pos] = pre_free_count;
  }

  // Update the discard
  for (size_t i = 0; i < total; i++) {
    if (!std::isnan(dst[i])) {
      discard_ = i;
      break;
    }
  }
}

Indicator HAYAKU_API LIUTONGPAN() {
  return make_shared<ILiuTongPan>()->calculate();
}

Indicator HAYAKU_API LIUTONGPAN(const KData& k) {
  auto p = make_shared<ILiuTongPan>();
  p->setContext(k);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-04-10
 *      Author: Jet
 */

namespace hayaku {

/* Return whether the security name matches the given pattern, in the form of an
 * indicator */
class INameLike : public IndicatorImp {
  INDICATOR_IMP(INameLike)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  INameLike();
  virtual ~INameLike() override;
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-04-10
 *      Author: Jet
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::INameLike)
#endif

namespace hayaku {

INameLike::INameLike() : IndicatorImp("NAMELIKE", 1) {
  need_context_ = true;
  setParam<string>("pattern", "");
}

INameLike::~INameLike() {}

void INameLike::_calculate(const Indicator& data) {
  HAYAKU_IF_RETURN(!isLeaf() && !data.empty(), void());

  const KData& k = getContext();
  size_t total = k.size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 1);

  string pattern = getParam<string>("pattern");
  string name = k.getStock().name();
  value_t match = wildcardMatch(name, pattern) ? 1.0 : 0.0;

  auto* dst = this->data();
  for (size_t i = 0; i < total; ++i) {
    dst[i] = match;
  }
}

void INameLike::_increment_calculate(const Indicator& data, size_t start_pos) {
  const KData& k = getContext();
  size_t total = k.size();

  string pattern = getParam<string>("pattern");
  string name = k.getStock().name();
  value_t match = wildcardMatch(name, pattern) ? 1.0 : 0.0;

  auto* dst = this->data();
  for (size_t i = start_pos; i < total; ++i) {
    dst[i] = match;
  }
}

Indicator HAYAKU_API NAMELIKE(const string& pattern) {
  auto p = make_shared<INameLike>();
  p->setParam<string>("pattern", pattern);
  return Indicator(p);
}

Indicator HAYAKU_API NAMELIKE(const KData& k, const string& pattern) {
  auto p = make_shared<INameLike>();
  p->setParam<string>("pattern", pattern);
  p->setContext(k);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-04-17
 *      Author: fasiondog
 */

namespace hayaku {

class IStkType : public IndicatorImp {
  INDICATOR_IMP(IStkType)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IStkType();
  virtual ~IStkType() override;
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-04-17
 *      Author: fasiondog
 */

#include "data/StockTypeInfo.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IStkType)
#endif

namespace hayaku {

IStkType::IStkType() : IndicatorImp("STKTYPE", 1) { need_context_ = true; }

IStkType::~IStkType() {}

void IStkType::_calculate(const Indicator& ind) {
  HAYAKU_WARN_IF(!isLeaf() && !ind.empty(),
                 "The input is ignored because {} depends on the context!",
                 getParam<string>("kpart"));

  size_t total = getContext().size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 1);

  discard_ = 0;
  _increment_calculate(ind, 0);
}

void IStkType::_increment_calculate(const Indicator& data, size_t start_pos) {
  const KData& kdata = getContext();
  size_t total = kdata.size();
  const Stock& stock = kdata.getStock();
  value_t stktype = static_cast<value_t>(stock.type());
  auto* dst = this->data();
  for (size_t i = start_pos; i < total; i++) {
    dst[i] = stktype;
  }
}

Indicator HAYAKU_API STKTYPE() { return make_shared<IStkType>()->calculate(); }

Indicator HAYAKU_API STKTYPE(const KData& k) {
  auto p = make_shared<IStkType>();
  p->setContext(k);
  return Indicator(p);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-01
 *      Author: fasiondog
 */

namespace hayaku {

/*
 * Get the yield of the 10-year Chinese treasury bond according to the context
 * or the input parameters
 * 1. The context date has priority
 * 2. If the time is earlier than the case where no treasury bond data exists,
 * the default value is used instead
 * 3. If the time is later than the existing treasury bond data, the last
 * treasury bond data is used
 */
class IZhBond10 : public IndicatorImp {
  INDICATOR_IMP(IZhBond10)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IZhBond10();
  explicit IZhBond10(const DatetimeList& dates, double default_val = 4.0);
  virtual ~IZhBond10() override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-01
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IZhBond10)
#endif

namespace hayaku {

IZhBond10::IZhBond10() : IndicatorImp("ZHBOND10") {
  // This parameter is used when there is no context, otherwise the dates in the
  // context are used; the data should be sorted in ascending order
  setParam<DatetimeList>("dates", DatetimeList());
  setParam<double>("default", 4.0);
}

IZhBond10::IZhBond10(const DatetimeList& dates, double default_val)
    : IndicatorImp("ZHBOND10") {
  setParam<DatetimeList>("dates", dates);
  setParam<double>("default", default_val);
}

IZhBond10::~IZhBond10() {}

void IZhBond10::_calculate(const Indicator& data) {
  DatetimeList dates;
  auto k = data.getContext();
  if (!k.empty()) {
    dates = k.getDatetimeList();
  } else {
    k = this->getContext();
    if (k.empty()) {
      dates = getParam<DatetimeList>("dates");
    } else {
      dates = k.getDatetimeList();
    }
  }

  size_t total = dates.size();
  HAYAKU_IF_RETURN(0 == total, void());

  _readyBuffer(total, 1);

  value_t default_val = (value_t)getParam<double>("default");
  auto* dst = this->data();

  // All the data needs to be fetched; when the given date is later than the
  // existing data, the value of the last record is taken as the current value
  const auto& bonds = getDataRuntime().getZhBond10();
  size_t bonds_size = bonds.size();
  if (0 == bonds_size) {
    for (size_t i = 0; i < total; i++) {
      dst[i] = default_val;
    }
    return;
  }

  size_t bondix = 0;
  for (size_t i = 0; i < total; i++) {
    bool found = false;
    for (size_t j = bondix; j < bonds_size; j++) {
      if (bonds[j].date > dates[i]) {
        found = true;
        bondix = j;
        break;
      }
    }
    if (found) {
      if (bondix != 0) {
        dst[i] = bonds[bondix - 1].value;
      } else {
        dst[i] = default_val;
      }
    } else {
      dst[i] = bonds[bonds_size - 1].value;
    }
  }
}

Indicator HAYAKU_API ZHBOND10(double default_val) {
  auto p = make_shared<IZhBond10>();
  p->setParam<double>("default", default_val);
  return Indicator(p);
}

Indicator HAYAKU_API ZHBOND10(const DatetimeList& dates, double default_val) {
  auto p = make_shared<IZhBond10>(dates);
  p->setParam<double>("default", default_val);
  p->calculate();
  return Indicator(p);
}

Indicator HAYAKU_API ZHBOND10(const KData& k, double default_val) {
  auto p = make_shared<IZhBond10>();
  p->setParam<double>("default", default_val);
  p->setContext(k);
  return Indicator(p);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-18
 *      Author: fasiondog
 */

namespace hayaku {

class IZongGuBen : public IndicatorImp {
  INDICATOR_IMP(IZongGuBen)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IZongGuBen();
  virtual ~IZongGuBen() override;
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-18
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IZongGuBen)
#endif

namespace hayaku {

IZongGuBen::IZongGuBen() : IndicatorImp("ZONGGUBEN", 1) {
  need_context_ = true;
}

IZongGuBen::~IZongGuBen() {}

void IZongGuBen::_calculate(const Indicator& data) {
  HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                 "The input is ignored because {} depends on the context!",
                 name_);

  const KData& k = getContext();
  size_t total = k.size();
  if (total == 0) {
    return;
  }

  _readyBuffer(total, 1);

  _increment_calculate(data, 0);
}

void IZongGuBen::_increment_calculate(const Indicator& data, size_t start_pos) {
  const KData& k = getContext();
  Stock stock = k.getStock();
  size_t total = k.size();
  StockWeightList sw_list = stock.getWeight();
  if (sw_list.size() == 0) {
    return;
  }

  auto* dst = this->data();
  size_t pos = start_pos;
  auto sw_iter = sw_list.begin();
  price_t pre_total_count = sw_iter->totalCount();
  for (; sw_iter != sw_list.end(); ++sw_iter) {
    price_t total_count = sw_iter->totalCount();
    if (total_count == 0) {
      continue;  // Ignore the ex-rights/ex-dividend record whose outstanding
                 // shares are 0
    }

    while (pos < total && k[pos].datetime < sw_iter->datetime()) {
      dst[pos] = pre_total_count;
      pos++;
    }

    pre_total_count = total_count;
    if (pos >= total) {
      break;
    }
  }

  for (; pos < total; pos++) {
    dst[pos] = pre_total_count;
  }
}

Indicator HAYAKU_API ZONGGUBEN() {
  return make_shared<IZongGuBen>()->calculate();
}

Indicator HAYAKU_API ZONGGUBEN(const KData& k) {
  auto p = make_shared<IZongGuBen>();
  p->setContext(k);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-15
 *      Author: fasiondog
 */

#include "Indicator.h"
#include "operators/Factor.h"

namespace hayaku {

class IFactor : public IndicatorImp {
 public:
  IFactor();
  explicit IFactor(const Factor& factor);
  virtual ~IFactor() override;

  virtual string formula() const override;
  virtual void _calculate(const Indicator& data) override;
  virtual IndicatorImpPtr _clone() override;

  virtual bool selfAlike(const IndicatorImp& other) const noexcept override;

 private:
  Factor factor_;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
 private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(IndicatorImp);
    ar& boost::serialization::make_nvp("m_factor", factor_);
  }
#endif
};

} /* namespace hayaku */

/*
 * IFactor.cpp
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IFactor)
#endif

namespace hayaku {

IFactor::IFactor() : IndicatorImp("FACTOR", 1) {
  need_context_ = true;
  need_self_alike_compare_ = true;
}

IFactor::IFactor(const Factor& factor)
    : IndicatorImp("FACTOR", 1), factor_(factor) {
  need_context_ = true;
  need_self_alike_compare_ = true;
}

IFactor::~IFactor() {}

string IFactor::formula() const { return factor_.formula().formula(); };

IndicatorImpPtr IFactor::_clone() { return make_shared<IFactor>(factor_); }

bool IFactor::selfAlike(const IndicatorImp& other) const noexcept {
  // Factor uses the "name + K-line type" as the unique identifier (see the
  // Factor.h documentation) and deliberately does not compare the formula
  // content. Therefore two FACTOR nodes are treated as the same node by
  // CompiledFactorPlan for the CSE merge only when both the name and the K-line
  // type are the same. The upstream FactorSet::add has already verified the
  // K-line type and removed the duplicates by name (the later one overwrites
  // the earlier one), so a well-formed FactorSet would not feed two factors
  // with the same identifier but different formulas into the compiled path at
  // the same time. dynamic_cast guarantees that a non-IFactor node is not
  // judged equal.
  const auto* other_ctx = dynamic_cast<const IFactor*>(&other);
  HAYAKU_IF_RETURN(other_ctx == nullptr, false);
  return factor_.name() == other_ctx->factor_.name() &&
         factor_.ktype() == other_ctx->factor_.ktype();
}

void IFactor::_calculate(const Indicator& data) {
  HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                 "The input is ignored because {} depends on the context!",
                 name_);

  const KData& k = getContext();
  size_t total = k.size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 1);

  auto value = factor_.getValue(k);
  value.setContext(k);
  discard_ = value.discard();
  value.getImp()->swap(this);
}

Indicator HAYAKU_API FACTOR(const Factor& factor) {
  return Indicator(make_shared<IFactor>(factor));
}

} /* namespace hayaku */
