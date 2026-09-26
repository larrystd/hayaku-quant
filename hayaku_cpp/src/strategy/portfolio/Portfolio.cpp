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

HAYAKU_API std::ostream& operator<<(std::ostream& os, const Portfolio& pf) {
  os << pf.str();
  return os;
}

HAYAKU_API std::ostream& operator<<(std::ostream& os, const PortfolioPtr& pf) {
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
    : m_name("Portfolio"), m_query(Null<KQuery>()), m_need_calculate(true) {
  initParam();
}

Portfolio::Portfolio(const string& name) : m_name(name) { initParam(); }

Portfolio::Portfolio(const string& name,
                     const internal::PortfolioAccountPortPtr& account,
                     const SelectorPtr& se, const AFPtr& af)
    : m_name(name),
      m_account(account),
      m_se(se),
      m_af(af),
      m_query(Null<KQuery>()),
      m_need_calculate(true) {
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
      HAYAKU_THROW("{}", htr("You can't trace in jupyter!"));
    }
  }
}

void Portfolio::paramChanged() { m_need_calculate = true; }

void Portfolio::reset() {
  if (m_account) m_account->reset();
  if (m_cashAccount) m_cashAccount->reset();
  if (m_se) m_se->reset();
  if (m_af) m_af->reset();
  m_need_calculate = true;
  m_real_sys_list.clear();
  m_running_sys_set.clear();
  m_dates.clear();
  m_adjust_flags.clear();
  m_cycle_end_dates.clear();
  m_adjust_turnover.clear();
  _reset();
}

PortfolioPtr Portfolio::clone() {
  PortfolioPtr p = _clone();
  p->m_params = m_params;
  p->m_name = m_name;
  p->m_is_python_object = m_is_python_object;
  p->m_query = m_query;
  p->m_need_calculate = true;
  if (m_se) p->m_se = m_se->clone();
  if (m_af) p->m_af = m_af->clone();
  if (m_account) p->m_account = m_account->cloneAccount();
  if (m_cashAccount) p->m_cashAccount = m_cashAccount->cloneAccount();
  return p;
}

void Portfolio::readyForRun() {
  HAYAKU_CHECK(m_se, "m_se is null!");
  HAYAKU_CHECK(m_account, "m_account is null!");
  reset();
  _calculateAdjustDate();
  m_se->setPF(shared_from_this());
  _readyForRun();
}

DatetimeList Portfolio::getAdjustDates() const {
  DatetimeList ret;
  for (size_t i = 0, total = m_dates.size(); i < total; i++) {
    if (m_adjust_flags[i]) {
      ret.push_back(m_dates[i]);
    }
  }
  return ret;
}

DatetimeList Portfolio::getCycleEndDates() const {
  DatetimeList ret;
  for (size_t i = 0, total = m_dates.size(); i < total; i++) {
    if (m_adjust_flags[i]) {
      ret.push_back(m_cycle_end_dates[i]);
    }
  }
  return ret;
}

void Portfolio::_calculateAdjustDate() {
  int adjust_cycle = getParam<int>("adjust_cycle");
  string mode = getParam<string>("adjust_mode");
  bool delay_to_trading_day = getParam<bool>("delay_to_trading_day");
  to_lower(mode);

  m_dates = getDataRuntime().getTradingCalendar(m_query);
  HAYAKU_IF_RETURN(m_dates.empty(), void());

  m_adjust_flags.resize(m_dates.size(), 0);
  m_cycle_end_dates.resize(m_dates.size(), Null<Datetime>());

  if ("query" == mode || "day" == mode) {
    size_t cur_adjust_ix = 0;
    Datetime cur_cycle_end;
    for (size_t i = 0, total = m_dates.size(); i < total; i++) {
      if (i == cur_adjust_ix) {
        cur_adjust_ix += adjust_cycle;
        cur_cycle_end = cur_adjust_ix < total ? m_dates[cur_adjust_ix]
                                              : m_dates.back() + Minutes(1);
        m_adjust_flags[i] = 1;
        m_cycle_end_dates[i] = cur_cycle_end;
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
    Datetime cur_cycle_end = m_dates.front().nextWeek();
    for (size_t i = 0, total = m_dates.size(); i < total; i++) {
      const auto& date = m_dates[i];
      bool adjust = (date.dayOfWeek() == adjust_cycle);
      if (adjust) {
        cur_cycle_end = date.nextWeek();
        if (cur_cycle_end >= m_dates.back()) {
          cur_cycle_end = m_dates.back() + Minutes(1);
        }
        m_adjust_flags[i] = 1;
        m_cycle_end_dates[i] = cur_cycle_end;
      }
    }

  } else if ("month" == mode) {
    Datetime cur_cycle_end = m_dates.front().nextMonth();
    for (size_t i = 0, total = m_dates.size(); i < total; i++) {
      const auto& date = m_dates[i];
      bool adjust = (date.day() == adjust_cycle);
      if (adjust) {
        cur_cycle_end = date.nextMonth();
        if (cur_cycle_end >= m_dates.back()) {
          cur_cycle_end = m_dates.back() + Minutes(1);
        }
        m_adjust_flags[i] = 1;
        m_cycle_end_dates[i] = cur_cycle_end;
      }
    }

  } else if ("quarter" == mode) {
    Datetime cur_cycle_end = m_dates.front().nextQuarter();
    for (size_t i = 0, total = m_dates.size(); i < total; i++) {
      const auto& date = m_dates[i];
      bool adjust = (date.day() == adjust_cycle);
      if (adjust) {
        cur_cycle_end = date.nextQuarter();
        if (cur_cycle_end >= m_dates.back()) {
          cur_cycle_end = m_dates.back() + Minutes(1);
        }
        m_adjust_flags[i] = 1;
        m_cycle_end_dates[i] = cur_cycle_end;
      }
    }
  } else if ("year" == mode) {
    Datetime cur_cycle_end = m_dates.front().nextYear();
    for (size_t i = 0, total = m_dates.size(); i < total; i++) {
      const auto& date = m_dates[i];
      bool adjust = (date.dayOfYear() == adjust_cycle);
      if (adjust) {
        cur_cycle_end = date.nextYear();
        if (cur_cycle_end >= m_dates.back()) {
          cur_cycle_end = m_dates.back() + Minutes(1);
        }
        m_adjust_flags[i] = 1;
        m_cycle_end_dates[i] = cur_cycle_end;
      }
    }
  }
}

void Portfolio::_calculateAdjustDateOnModeDelayToTradingDay(
    int adjust_cycle, const string& mode) {
  std::set<Datetime> adjust_date_set;
  if ("week" == mode) {
    Datetime cur_cycle_end = m_dates.front().nextWeek();
    for (size_t i = 0, total = m_dates.size(); i < total; i++) {
      const auto& date = m_dates[i];
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
        if (cur_cycle_end >= m_dates.back()) {
          cur_cycle_end = m_dates.back() + Minutes(1);
        }
        m_adjust_flags[i] = 1;
        m_cycle_end_dates[i] = cur_cycle_end;
      }
    }

  } else if ("month" == mode) {
    Datetime cur_cycle_end = m_dates.front().nextMonth();
    for (size_t i = 0, total = m_dates.size(); i < total; i++) {
      const auto& date = m_dates[i];
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
        if (cur_cycle_end >= m_dates.back()) {
          cur_cycle_end = m_dates.back() + Minutes(1);
        }
        m_adjust_flags[i] = 1;
        m_cycle_end_dates[i] = cur_cycle_end;
      }
    }

  } else if ("quarter" == mode) {
    Datetime cur_cycle_end = m_dates.front().nextQuarter();
    for (size_t i = 0, total = m_dates.size(); i < total; i++) {
      const auto& date = m_dates[i];
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
        if (cur_cycle_end >= m_dates.back()) {
          cur_cycle_end = m_dates.back() + Minutes(1);
        }
        m_adjust_flags[i] = 1;
        m_cycle_end_dates[i] = cur_cycle_end;
      }
    }

  } else if ("year" == mode) {
    Datetime cur_cycle_end = m_dates.front().nextYear();
    for (size_t i = 0, total = m_dates.size(); i < total; i++) {
      const auto& date = m_dates[i];
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
        if (cur_cycle_end >= m_dates.back()) {
          cur_cycle_end = m_dates.back() + Minutes(1);
        }
        m_adjust_flags[i] = 1;
        m_cycle_end_dates[i] = cur_cycle_end;
      }
    }
  }
}

void Portfolio::runMoment(const Datetime& date, const Datetime& nextCycle,
                          bool adjust) {
  // The current date is earlier than the account creation date, ignore it
  // directly
  HAYAKU_IF_RETURN(date < m_account->initDatetime(), void());

  bool trace = getParam<bool>("trace");
  if (trace) {
    HAYAKU_INFO(
        "{} ===========================================================", date);
    if (adjust) {
      HAYAKU_INFO("****************************************************");
      HAYAKU_INFO("**                                                **");
      HAYAKU_INFO(htr("**  [PF] Position adjustment will be made today.  **"));
      HAYAKU_INFO("**                                                **");
      HAYAKU_INFO("****************************************************");
    }
    HAYAKU_INFO("{}: {}", htr("[PF] current running system size"),
                m_running_sys_set.size());
  }

  // Adjust the ex-rights/ex-dividend data of the account before the open
  m_account->updateWithWeight(date);

  _runMomentOnOpen(date, nextCycle, adjust);
  traceMomentTMAfterRunAtOpen(date);

  _runMomentOnClose(date, nextCycle, adjust);
  traceMomentTMAfterRunAtClose(date);

  // Print the current account assets for the trace
  if (trace) {
    FundsRecord funds = m_account->getFunds(date, m_query.kType());
    HAYAKU_INFO("[PF] {}: {:.2f}, {}: {:<.2f}, {}: {:<.2f}", htr("total asset"),
                funds.total_assets(), htr("current cash"), funds.cash,
                htr("market value"), funds.market_value);
  }
}

void Portfolio::run(const KQuery& query, bool force) {
  SPEND_TIME(Portfolio_run);

  string mode = getParam<string>("adjust_mode");
  to_lower(mode);
  if (mode != "query") {
    HAYAKU_CHECK(
        query.kType() == KQuery::DAY, "{} {}", query.kType(),
        htr("kType of query must be DAY when adjust_mode is not \"query\"!"));
  }

  setQuery(query);

  if (force) {
    m_need_calculate = true;
  }
  HAYAKU_IF_RETURN(!m_need_calculate, void());

  readyForRun();

  if (m_real_sys_list.empty()) {
    HAYAKU_WARN(htr("There is no system in portfolio!"));
    m_need_calculate = true;
    return;
  }

  for (size_t i = 0; i < m_dates.size(); i++) {
    runMoment(m_dates[i], m_cycle_end_dates[i], m_adjust_flags[i]);
  }

  m_need_calculate = false;
}

void Portfolio::traceMomentTMAfterRunAtOpen(const Datetime& date) {
  HAYAKU_IF_RETURN(!getParam<bool>("trace") || m_running_sys_set.empty(),
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
  for (const auto& sys : m_running_sys_set) {
    Stock stk = sys->getStock();
    size_t position = sys->getAccount()->getHoldNumber(date, stk);
    KRecord krecord = stk.getKRecord(date, m_query.kType());
    auto stk_name = stk.name();
    HAYAKU_INFO("| {:<11}| {:<11}| {:<11}| {:<13.2f}| {:<12.2f}|",
                stk.market_code(), stk_name, position,
                position * krecord.openPrice, krecord.openPrice);
    // clang-format off
        HAYAKU_INFO("+------------+------------+------------+--------------+--------------+");
        count++;
        int trace_max_num = getParam<int>("trace_max_num");
        if (count >= trace_max_num) {
            if (m_running_sys_set.size() > trace_max_num) {
                HAYAKU_INFO("+ ... ... more                                                        +");
                HAYAKU_INFO("+------------+------------+------------+--------------+--------------++");
            }
            break;
        }
    // clang-format on
  }
}

void Portfolio::traceMomentTMAfterRunAtClose(const Datetime& date) {
  HAYAKU_IF_RETURN(!getParam<bool>("trace") || m_running_sys_set.empty(),
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
  for (const auto& sys : m_running_sys_set) {
    Stock stk = sys->getStock();
    auto funds = sys->getAccount()->getFunds(date, m_query.kType());
    size_t position = sys->getAccount()->getHoldNumber(date, stk);
    KRecord krecord = stk.getKRecord(date, m_query.kType());
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
            if (m_running_sys_set.size() > trace_max_num) {
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
  for (const auto& sys : m_running_sys_set) {
    sys_json_list.emplace_back(sys->lastSuggestion());
  }

  json ret;
  ret["name"] = name();
  ret["sys_list"] = sys_json_list;
  return ret;
}

} /* namespace hayaku */
