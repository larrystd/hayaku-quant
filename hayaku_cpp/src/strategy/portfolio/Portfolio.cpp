/*
 * Portfolio.cpp
 *
 *  Created on: 2016-2-21
 *      Author: fasiondog
 */

#include "Portfolio.h"

#include "application/SystemInfo.h"
#include "data/DataRuntime.h"
#include "strategy/selection/OptimalSelectorBase.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::Portfolio)
#endif

namespace hayaku {

std::ostream& operator<<(std::ostream& os, const Portfolio& pf) {
  os << pf.str();
  return os;
}

std::ostream& operator<<(std::ostream& os, const PortfolioPtr& pf) {
  if (pf) {
    os << pf->str();
  } else {
    os << "Portfolio(NULL)";
  }

  return os;
}

string Portfolio::str() const {
  std::stringstream os;
  string strip(",\n");
  string space("  ");
  os << "Portfolio{\n"
     << space << name() << strip << space << getParameter() << strip << space
     << getQuery() << strip << space << getAF() << strip << space << getSE()
     << strip << space
     << (getAccount()
             ? fmt::format("Account({})", getAccount()->accountId().value())
             : "Account(NULL)")
     << strip << "}";
  return os.str();
}

Portfolio::Portfolio()
    : name_("Portfolio"), query_(Null<KQuery>()), need_calculate_(true) {
  initParam();
}

Portfolio::Portfolio(const string& name) : name_(name) { initParam(); }

Portfolio::Portfolio(const string& name,
                     const internal::PortfolioAccountPortPtr& account,
                     const SelectorPtr& se, const AFPtr& af)
    : name_(name),
      account_(account),
      se_(se),
      af_(af),
      query_(Null<KQuery>()),
      need_calculate_(true) {
  initParam();
}

Portfolio::~Portfolio() {}

void Portfolio::initParam() {
  setParam<int>("adjust_cycle", 1);          // Position adjustment cycle
  setParam<string>("adjust_mode", "query");  // Position adjustment mode

  // Delay to a trading day: when the adjustment day is not a trading day it is
  // delayed automatically to the next trading day
  setParam<bool>("delay_to_trading_day", true);

  setParam<bool>("trace", false);  // Print the trace
  setParam<int>("trace_max_num",
                10);  // Max held securities shown when printing the trace
}

void Portfolio::baseCheckParam(const string& name) const {
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

  } else if ("trace" == name) {
    if (getParam<bool>("trace") && pythonInJupyter()) {
      HAYAKU_THROW("{}", "You can't trace in jupyter!");
    }
  }
}

void Portfolio::paramChanged() { need_calculate_ = true; }

void Portfolio::reset() {
  if (account_) account_->reset();
  if (cash_account_) cash_account_->reset();
  if (se_) se_->reset();
  if (af_) af_->reset();
  need_calculate_ = true;
  real_sys_list_.clear();
  running_sys_set_.clear();
  dates_.clear();
  adjust_flags_.clear();
  cycle_end_dates_.clear();
  adjust_turnover_.clear();
  _reset();
}

PortfolioPtr Portfolio::clone() {
  PortfolioPtr p = _clone();
  p->params_ = params_;
  p->name_ = name_;
  p->is_python_object_ = is_python_object_;
  p->query_ = query_;
  p->need_calculate_ = true;
  if (se_) p->se_ = se_->clone();
  if (af_) p->af_ = af_->clone();
  if (account_) p->account_ = account_->cloneAccount();
  if (cash_account_) p->cash_account_ = cash_account_->cloneAccount();
  return p;
}

void Portfolio::readyForRun() {
  HAYAKU_CHECK(se_, "m_se is null!");
  HAYAKU_CHECK(account_, "m_account is null!");
  reset();
  _calculateAdjustDate();
  se_->setPF(shared_from_this());
  _readyForRun();
}

DatetimeList Portfolio::getAdjustDates() const {
  DatetimeList ret;
  for (size_t i = 0, total = dates_.size(); i < total; i++) {
    if (adjust_flags_[i]) {
      ret.push_back(dates_[i]);
    }
  }
  return ret;
}

DatetimeList Portfolio::getCycleEndDates() const {
  DatetimeList ret;
  for (size_t i = 0, total = dates_.size(); i < total; i++) {
    if (adjust_flags_[i]) {
      ret.push_back(cycle_end_dates_[i]);
    }
  }
  return ret;
}

void Portfolio::_calculateAdjustDate() {
  int adjust_cycle = getParam<int>("adjust_cycle");
  string mode = getParam<string>("adjust_mode");
  bool delay_to_trading_day = getParam<bool>("delay_to_trading_day");
  to_lower(mode);

  dates_ = getDataRuntime().getTradingCalendar(query_);
  HAYAKU_IF_RETURN(dates_.empty(), void());

  adjust_flags_.resize(dates_.size(), 0);
  cycle_end_dates_.resize(dates_.size(), Null<Datetime>());

  if ("query" == mode || "day" == mode) {
    size_t cur_adjust_ix = 0;
    Datetime cur_cycle_end;
    for (size_t i = 0, total = dates_.size(); i < total; i++) {
      if (i == cur_adjust_ix) {
        cur_adjust_ix += adjust_cycle;
        cur_cycle_end = cur_adjust_ix < total ? dates_[cur_adjust_ix]
                                              : dates_.back() + Minutes(1);
        adjust_flags_[i] = 1;
        cycle_end_dates_[i] = cur_cycle_end;
      }
    }

  } else if (delay_to_trading_day) {
    _calculateAdjustDateOnModeDelayToTradingDay(adjust_cycle, mode);
  } else {
    _calculateAdjustDateOnMode(adjust_cycle, mode);
  }
}

void Portfolio::_calculateAdjustDateOnMode(int adjust_cycle,
                                           const string& mode) {
  if ("week" == mode) {
    Datetime cur_cycle_end = dates_.front().nextWeek();
    for (size_t i = 0, total = dates_.size(); i < total; i++) {
      const auto& date = dates_[i];
      bool adjust = (date.dayOfWeek() == adjust_cycle);
      if (adjust) {
        cur_cycle_end = date.nextWeek();
        if (cur_cycle_end >= dates_.back()) {
          cur_cycle_end = dates_.back() + Minutes(1);
        }
        adjust_flags_[i] = 1;
        cycle_end_dates_[i] = cur_cycle_end;
      }
    }

  } else if ("month" == mode) {
    Datetime cur_cycle_end = dates_.front().nextMonth();
    for (size_t i = 0, total = dates_.size(); i < total; i++) {
      const auto& date = dates_[i];
      bool adjust = (date.day() == adjust_cycle);
      if (adjust) {
        cur_cycle_end = date.nextMonth();
        if (cur_cycle_end >= dates_.back()) {
          cur_cycle_end = dates_.back() + Minutes(1);
        }
        adjust_flags_[i] = 1;
        cycle_end_dates_[i] = cur_cycle_end;
      }
    }

  } else if ("quarter" == mode) {
    Datetime cur_cycle_end = dates_.front().nextQuarter();
    for (size_t i = 0, total = dates_.size(); i < total; i++) {
      const auto& date = dates_[i];
      bool adjust = (date.day() == adjust_cycle);
      if (adjust) {
        cur_cycle_end = date.nextQuarter();
        if (cur_cycle_end >= dates_.back()) {
          cur_cycle_end = dates_.back() + Minutes(1);
        }
        adjust_flags_[i] = 1;
        cycle_end_dates_[i] = cur_cycle_end;
      }
    }
  } else if ("year" == mode) {
    Datetime cur_cycle_end = dates_.front().nextYear();
    for (size_t i = 0, total = dates_.size(); i < total; i++) {
      const auto& date = dates_[i];
      bool adjust = (date.dayOfYear() == adjust_cycle);
      if (adjust) {
        cur_cycle_end = date.nextYear();
        if (cur_cycle_end >= dates_.back()) {
          cur_cycle_end = dates_.back() + Minutes(1);
        }
        adjust_flags_[i] = 1;
        cycle_end_dates_[i] = cur_cycle_end;
      }
    }
  }
}

void Portfolio::_calculateAdjustDateOnModeDelayToTradingDay(
    int adjust_cycle, const string& mode) {
  std::set<Datetime> adjust_date_set;
  if ("week" == mode) {
    Datetime cur_cycle_end = dates_.front().nextWeek();
    for (size_t i = 0, total = dates_.size(); i < total; i++) {
      const auto& date = dates_[i];
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
        if (cur_cycle_end >= dates_.back()) {
          cur_cycle_end = dates_.back() + Minutes(1);
        }
        adjust_flags_[i] = 1;
        cycle_end_dates_[i] = cur_cycle_end;
      }
    }

  } else if ("month" == mode) {
    Datetime cur_cycle_end = dates_.front().nextMonth();
    for (size_t i = 0, total = dates_.size(); i < total; i++) {
      const auto& date = dates_[i];
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
        if (cur_cycle_end >= dates_.back()) {
          cur_cycle_end = dates_.back() + Minutes(1);
        }
        adjust_flags_[i] = 1;
        cycle_end_dates_[i] = cur_cycle_end;
      }
    }

  } else if ("quarter" == mode) {
    Datetime cur_cycle_end = dates_.front().nextQuarter();
    for (size_t i = 0, total = dates_.size(); i < total; i++) {
      const auto& date = dates_[i];
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
        if (cur_cycle_end >= dates_.back()) {
          cur_cycle_end = dates_.back() + Minutes(1);
        }
        adjust_flags_[i] = 1;
        cycle_end_dates_[i] = cur_cycle_end;
      }
    }

  } else if ("year" == mode) {
    Datetime cur_cycle_end = dates_.front().nextYear();
    for (size_t i = 0, total = dates_.size(); i < total; i++) {
      const auto& date = dates_[i];
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
        if (cur_cycle_end >= dates_.back()) {
          cur_cycle_end = dates_.back() + Minutes(1);
        }
        adjust_flags_[i] = 1;
        cycle_end_dates_[i] = cur_cycle_end;
      }
    }
  }
}

void Portfolio::runMoment(const Datetime& date, const Datetime& nextCycle,
                          bool adjust) {
  // The current date is earlier than the account creation date, ignore it
  // directly
  HAYAKU_IF_RETURN(date < account_->initDatetime(), void());

  bool trace = getParam<bool>("trace");
  if (trace) {
    HAYAKU_INFO(
        "{} ===========================================================", date);
    if (adjust) {
      HAYAKU_INFO("****************************************************");
      HAYAKU_INFO("**                                                **");
      HAYAKU_INFO("**  [PF] Position adjustment will be made today.  **");
      HAYAKU_INFO("**                                                **");
      HAYAKU_INFO("****************************************************");
    }
    HAYAKU_INFO("{}: {}", "[PF] current running system size",
                running_sys_set_.size());
  }

  // Adjust the ex-rights/ex-dividend data of the account before the open
  account_->updateWithWeight(date);

  _runMomentOnOpen(date, nextCycle, adjust);
  traceMomentTMAfterRunAtOpen(date);

  _runMomentOnClose(date, nextCycle, adjust);
  traceMomentTMAfterRunAtClose(date);

  // Print the current account assets for the trace
  if (trace) {
    FundsRecord funds = account_->getFunds(date, query_.kType());
    HAYAKU_INFO("[PF] {}: {:.2f}, {}: {:<.2f}, {}: {:<.2f}", "total asset",
                funds.total_assets(), "current cash", funds.cash,
                "market value", funds.market_value);
  }
}

void Portfolio::run(const KQuery& query, bool force) {
  SPEND_TIME(Portfolio_run);

  string mode = getParam<string>("adjust_mode");
  to_lower(mode);
  if (mode != "query") {
    HAYAKU_CHECK(
        query.kType() == KQuery::DAY, "{} {}", query.kType(),
        "kType of query must be DAY when adjust_mode is not \"query\"!");
  }

  setQuery(query);

  if (force) {
    need_calculate_ = true;
  }
  HAYAKU_IF_RETURN(!need_calculate_, void());

  readyForRun();

  if (real_sys_list_.empty()) {
    HAYAKU_WARN("There is no system in portfolio!");
    need_calculate_ = true;
    return;
  }

  for (size_t i = 0; i < dates_.size(); i++) {
    runMoment(dates_[i], cycle_end_dates_[i], adjust_flags_[i]);
  }

  need_calculate_ = false;
}

void Portfolio::traceMomentTMAfterRunAtOpen(const Datetime& date) {
  HAYAKU_IF_RETURN(!getParam<bool>("trace") || running_sys_set_.empty(),
                   void());

  //----------------------------------------------------------------------
  // Print the position for the trace
  //----------------------------------------------------------------------
  // clang-format off
    HAYAKU_INFO("+------------+------------+------------+--------------+--------------+");
    HAYAKU_INFO("| code       | name       | position   | market value |  open price  |");
    HAYAKU_INFO("+------------+------------+------------+--------------+--------------+");
  // clang-format on

  size_t count = 0;
  for (const auto& sys : running_sys_set_) {
    Stock stk = sys->getStock();
    size_t position = sys->getAccount()->getHoldNumber(date, stk);
    KRecord krecord = stk.getKRecord(date, query_.kType());
    auto stk_name = stk.name();
    HAYAKU_INFO("| {:<11}| {:<11}| {:<11}| {:<13.2f}| {:<12.2f}|",
                stk.market_code(), stk_name, position,
                position * krecord.openPrice, krecord.openPrice);
    // clang-format off
        HAYAKU_INFO("+------------+------------+------------+--------------+--------------+");
        count++;
        int trace_max_num = getParam<int>("trace_max_num");
        if (count >= trace_max_num) {
            if (running_sys_set_.size() > trace_max_num) {
                HAYAKU_INFO("+ ... ... more                                                        +");
                HAYAKU_INFO("+------------+------------+------------+--------------+--------------++");
            }
            break;
        }
    // clang-format on
  }
}

void Portfolio::traceMomentTMAfterRunAtClose(const Datetime& date) {
  HAYAKU_IF_RETURN(!getParam<bool>("trace") || running_sys_set_.empty(),
                   void());

  //----------------------------------------------------------------------
  // Print the position for the trace
  //----------------------------------------------------------------------
  // clang-format off
    HAYAKU_INFO("+------------+------------+------------+--------------+--------------+-------------+-------------+");
    HAYAKU_INFO("| code       | name       | position   | market value | remain cash  | open price  | close price  |");
    HAYAKU_INFO("+------------+------------+------------+--------------+--------------+-------------+-------------+");
  // clang-format on

  size_t count = 0;
  for (const auto& sys : running_sys_set_) {
    Stock stk = sys->getStock();
    auto funds = sys->getAccount()->getFunds(date, query_.kType());
    size_t position = sys->getAccount()->getHoldNumber(date, stk);
    KRecord krecord = stk.getKRecord(date, query_.kType());
    auto stk_name = stk.name();
    HAYAKU_INFO(
        "| {:<11}| {:<11}| {:<11}| {:<13.2f}| {:<13.2f}| {:<12.2f}| {:<12.2f}|",
        stk.market_code(), stk_name, position, funds.market_value, funds.cash,
        krecord.openPrice, krecord.closePrice);
    // clang-format off
        HAYAKU_INFO("+------------+------------+------------+--------------+--------------+-------------+-------------+");
        count++;
        int trace_max_num = getParam<int>("trace_max_num");
        if (count >= trace_max_num) {
            if (running_sys_set_.size() > trace_max_num) {
                HAYAKU_INFO("+ ... ... more                                                                                   +");
                HAYAKU_INFO("+------------+------------+------------+--------------+--------------+-------------+-------------+");
            }
            break;
        }
    // clang-format on
  }
}

json Portfolio::lastSuggestion() const {
  json sys_json_list = json::array();
  for (const auto& sys : running_sys_set_) {
    sys_json_list.emplace_back(sys->lastSuggestion());
  }

  json ret;
  ret["name"] = name();
  ret["sys_list"] = sys_json_list;
  return ret;
}

} /* namespace hayaku */
