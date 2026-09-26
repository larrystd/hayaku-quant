/*
 * SimplePortfolio.cpp
 *
 *  Created on: 2016-2-21
 *      Author: fasiondog
 */

#include "SimplePortfolio.h"

#include "application/SystemInfo.h"
#include "strategy/StrategyRuntime.h"
#include "strategy/selection/OptimalSelectorBase.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::SimplePortfolio)
#endif

namespace hayaku {

SimplePortfolio::SimplePortfolio() : Portfolio("PF_Simple") {}

SimplePortfolio::SimplePortfolio(
    const internal::PortfolioAccountPortPtr& account, const SelectorPtr& se,
    const AFPtr& af)
    : Portfolio("PF_Simple", account, se, af) {}

SimplePortfolio::~SimplePortfolio() {}

void SimplePortfolio::_reset() {
  dlist_sys_list_.clear();
  delay_adjust_sys_list_.clear();
  tmp_selected_list_.clear();
  tmp_will_remove_sys_.clear();
}

void SimplePortfolio::_readyForRun() {
  HAYAKU_CHECK(af_, "m_af is null!");

  // The se algorithm and the af algorithm do not match
  HAYAKU_CHECK(se_->isMatchAF(af_), "The current SE and AF do not match!");

  // Check whether the account has the initial assets
  FundsRecord funds = account_->getFunds(Null<Datetime>());
  HAYAKU_CHECK(funds.total_assets() > 0.0, "The current tm is zero assets!");

  // Get the prototype system list from se
  const auto& pro_sys_list = se_->getProtoSystemList();
  HAYAKU_WARN_IF_RETURN(pro_sys_list.empty(), void(),
                        "Can't fetch proto_sys_lsit from Selector!");

  // Create the cash account
  cash_account_ = account_->cloneAccount();

  // Configure the asset allocator
  af_->setAccount(account_);
  af_->setCashAccount(cash_account_);
  af_->setQuery(query_);

  // Get all the candidate subsystems, assign sub accounts to those without an
  // associated account and prepare every subsystem for the startup
  internal::PortfolioAccountPortPtr prototypeAccount =
      account_->createChildAccount("TM_SUB");
  size_t total = pro_sys_list.size();
  real_sys_list_.reserve(total);
  for (size_t i = 0; i < total; i++) {
    const internal::StrategyRuntimePtr& pro_sys = pro_sys_list[i];
    if (pro_sys) {
      internal::StrategyRuntimePtr sys = pro_sys->clone();
      se_->bindRealToProto(sys, pro_sys);
      real_sys_list_.emplace_back(sys);

      // Create sub accounts with an initial capital of 0 for the systems
      // actually executed internally
      sys->setAccount(prototypeAccount->cloneAccount());
      string sys_name =
          fmt::format("{}_{}_{}", sys->name(), sys->getStock().market_code(),
                      sys->getStock().name());
      sys->name(fmt::format("PF_{}", sys_name));

      KData k = sys->getStock().getKData(query_);
      sys->prepare();
      sys->bind(k);
    }
  }

  // Tell se the list of the systems actually running
  se_->calculate(real_sys_list_, query_);
}

void SimplePortfolio::_runMomentOnOpen(const Datetime& date,
                                       const Datetime& nextCycle, bool adjust) {
  //---------------------------------------------------
  // Check whether there is a delisted security among the running systems
  //---------------------------------------------------
  for (auto iter = running_sys_set_.begin(); iter != running_sys_set_.end();
       /*++iter*/) {
    auto& sys = *iter;
    if (sys->getStock().getMarketValue(date, query_.kType()) == 0.0) {
      auto subAccount = sys->getAccount();
      auto sub_cash = subAccount->currentCash();
      if (sub_cash > 0.0 && subAccount->checkout(date, sub_cash)) {
        cash_account_->checkin(date, sub_cash);
      }
      dlist_sys_list_.emplace_back(sys);
      running_sys_set_.erase(iter++);
    } else {
      ++iter;
    }
  }

  //---------------------------------------------------
  // Handle the possible deviation among the sub accounts, the cash account and
  // the total account before the open
  //---------------------------------------------------
  int precision = account_->precision();

  // Update the ex-rights/ex-dividend data of all the running systems
  price_t sum_cash = 0.0;
  for (auto& running_sys : running_sys_set_) {
    internal::PortfolioAccountPortPtr subAccount = running_sys->getAccount();
    subAccount->updateWithWeight(date);
    sum_cash += subAccount->currentCash();
  }

  // Do the netting before the open (balancing the deviation among sub_sys,
  // cashAccount and tm)
  bool trace = getParam<bool>("trace");
  HAYAKU_INFO_IF(trace, "[PF] {}: {}, {}: {}, {}: {}",
                 htr("The sum cash of subAccount"), sum_cash, htr("cash tm"),
                 cash_account_->currentCash(), htr("tm cash"),
                 account_->currentCash());
  sum_cash += cash_account_->currentCash();

  price_t diff =
      roundEx(std::abs(account_->currentCash() - sum_cash), precision);
  if (diff > 0.) {
    if (account_->currentCash() > sum_cash) {
      cash_account_->checkin(date, diff);
    } else if (account_->currentCash() < sum_cash) {
      if (cash_account_->currentCash() > diff) {
        cash_account_->checkout(date, cash_account_->currentCash() - diff);
      }
    }
    HAYAKU_INFO_IF(trace, "[PF] {}: {}, {}: {}, {}: {}",
                   htr("After compensate: the sum cash of subAccount"),
                   sum_cash, htr("cash tm"), cash_account_->currentCash(),
                   htr("tm cash"), account_->currentCash());
  }

  //----------------------------------------------------------------------
  // Print the assets before the position adjustment for the trace
  //----------------------------------------------------------------------
  if (trace) {
    auto funds = account_->getFunds(date, query_.kType());
    HAYAKU_INFO("[PF] [{}] - {}: {},  {}: {}, {}: {}", htr("before rebalance"),
                htr("total funds"), funds.cash + funds.market_value,
                htr("cash"), funds.cash, htr("market_value"),
                funds.market_value);
  }

  //----------------------------------------------------------------------
  // At the open, handle first the systems whose position adjustment sell failed
  // on the previous trading day
  //----------------------------------------------------------------------
  HAYAKU_INFO_IF(trace, "[PF] {}: {}", htr("process delay adjust sys, size"),
                 delay_adjust_sys_list_.size());
  StrategyWeightList tmp_continue_adjust_sys_list;
  for (auto& sys : delay_adjust_sys_list_) {
    auto tr =
        sys.strategy->sellForceOnOpen(date, sys.weight, OrderOrigin::PORTFOLIO);
    if (!tr.isNull()) {
      HAYAKU_INFO_IF(trace, htr("[PF] Delay adjust sell: {}", tr));
      account_->addTradeRecord(tr);

      // After the sell, try to withdraw the funds and transfer them to the
      // shadow total account
      internal::PortfolioAccountPortPtr subAccount = sys.strategy->getAccount();
      auto sub_cash = subAccount->currentCash();
      if (sub_cash > 0.0 && subAccount->checkout(date, sub_cash)) {
        cash_account_->checkin(date, sub_cash);
      }

    } else {
      // When a forced sell fails and there is still a position, the processing
      // continues on the next trading day
      PositionRecord position = sys.strategy->getAccount()->getPosition(
          date, sys.strategy->getStock());
      if (position.number > 0.0) {
        HAYAKU_INFO_IF(
            trace, htr("[{}] failed to force sell, delay to next day", name()));
        tmp_continue_adjust_sys_list.emplace_back(sys);
      }
    }
  }

  delay_adjust_sys_list_.swap(tmp_continue_adjust_sys_list);

  //---------------------------------------------------
  // Check whether any running system has a delayed buy / sell signal (i.e. a
  // system that trades at the open)
  //---------------------------------------------------
  for (auto& sys : running_sys_set_) {
    auto tr = sys->processPendingSell(date);
    if (!tr.isNull()) {
      HAYAKU_INFO_IF(trace, htr("[PF] sell delay on open {}", tr));
      account_->addTradeRecord(tr);
    }
    tr = sys->processPendingBuy(date);
    if (!tr.isNull()) {
      HAYAKU_INFO_IF(trace, htr("[PF] buy delay on open {}", tr));
      account_->addTradeRecord(tr);
    }
  }

  traceMomentTMAfterRunAtOpen(date);
}

void SimplePortfolio::_runMomentOnClose(const Datetime& date,
                                        const Datetime& nextCycle,
                                        bool adjust) {
  bool trace = getParam<bool>("trace");
  //---------------------------------------------------
  // On the adjustment day, adjust the funds allocation
  //---------------------------------------------------
  if (adjust) {
    // Remove the systems without a position and without a delayed buy / sell
    // signal from the running system list immediately and recall the funds
    tmp_will_remove_sys_.clear();
    for (auto& sys : running_sys_set_) {
      auto subAccount = sys->getAccount();
      const auto& pending = sys->pendingOrders();
      // There is no position
      if (0 == subAccount->getHoldNumber(date, sys->getStock()) &&
          ((sys->getParam<bool>("buy_delay") && !pending.buy().valid) &&
           (sys->getParam<bool>("sell_delay") && !pending.buy().valid))) {
        // There is no delayed buy / sell signal
        HAYAKU_INFO_IF(trace,
                       htr("[PF] remove no signal delay sys: {}", sys->name()));
        tmp_will_remove_sys_.emplace_back(sys, 0.0);

        auto sub_cash = subAccount->currentCash();
        if (sub_cash > 0.0 && subAccount->checkout(date, sub_cash)) {
          cash_account_->checkin(date, sub_cash);
        }
      }
    }

    size_t running_sys_count = running_sys_set_.size();
    size_t out_sys_count = tmp_will_remove_sys_.size();
    size_t in_sys_count = 0;

    for (auto& sw : tmp_will_remove_sys_) {
      running_sys_set_.erase(sw.strategy);
    }

    // Get the selected system list from the selection strategy
    tmp_selected_list_ = se_->getSelected(date);

    // When AF adjusts the weights of the held systems, process the delayed
    // requests of the unselected running systems otherwise the running systems
    // are considered to control the selling themselves, unaffected by the
    // current selection
    if (af_->getParam<bool>("adjust_running_sys")) {
      // When a selected system is not in the existing list, clear its delayed
      // buy operation first, preventing a future signal on the adjustment day
      for (auto& sw : tmp_selected_list_) {
        if (sw.strategy) {
          if (running_sys_set_.find(sw.strategy) == running_sys_set_.end()) {
            HAYAKU_INFO_IF(trace,
                           htr("[PF] clear delay buy request(future): {}",
                               sw.strategy->name()));
            sw.strategy->clearPendingBuy();
          }
        }
      }
    }

    if (trace && !tmp_selected_list_.empty()) {
      for (auto& sys : tmp_selected_list_) {
        HAYAKU_INFO_IF(sys.strategy, htr("[PF] select: {}, score: {:<.4f}",
                                         sys.strategy->name(), sys.weight));
      }
    }

    // The asset allocation algorithm adjusts the asset allocation of every
    // subsystem; AF adjusts the positions uniformly at the close and returns
    // the systems whose close adjustment failed (they need to be processed at
    // the next open)
    auto tmp_continue_adjust_sys_list =
        af_->adjustFunds(date, tmp_selected_list_, running_sys_set_);

    if (delay_adjust_sys_list_.empty()) {
      delay_adjust_sys_list_.swap(tmp_continue_adjust_sys_list);
    } else {
      for (auto& sw : tmp_continue_adjust_sys_list) {
        delay_adjust_sys_list_.emplace_back(sw);
      }
    }

    // When a selected system is not in the existing list and funds have been
    // allocated to its account, add it to the running system list
    for (auto& sys : tmp_selected_list_) {
      if (sys.strategy) {
        if (running_sys_set_.find(sys.strategy) == running_sys_set_.end()) {
          if (sys.strategy->getAccount()->cash(date, query_.kType()) > 0.0) {
            running_sys_set_.insert(sys.strategy);
            in_sys_count++;
          }
        }
      }
    }

    // Remove immediately from the running system list the systems without a
    // position and without funds, and the systems without a position and
    // without a delayed buy / sell signal
    tmp_will_remove_sys_.clear();
    for (auto& sys : running_sys_set_) {
      auto subAccount = sys->getAccount();
      // There is no position
      if (subAccount->currentCash() < 1.0 &&
          0 == subAccount->getHoldNumber(date, sys->getStock())) {
        // There is no cash
        HAYAKU_INFO_IF(trace, htr("[PF] remove sys: {}", sys->name()));
        tmp_will_remove_sys_.emplace_back(sys, 0.0);
      }
    }

    out_sys_count += tmp_will_remove_sys_.size();
    for (auto& sw : tmp_will_remove_sys_) {
      running_sys_set_.erase(sw.strategy);
    }

    // Calculate the position adjustment turnover
    if (running_sys_count > 0) {
      adjust_turnover_.emplace_back(
          date, static_cast<double>(in_sys_count + out_sys_count) /
                    running_sys_count);
    }
  }

  //----------------------------------------------------------------------
  // Print the assets after the position adjustment for the trace
  //----------------------------------------------------------------------
  if (trace) {
    auto funds = account_->getFunds(date, query_.kType());
    HAYAKU_INFO("[PF] [{}] - {}: {}, {}: {}, {}: {}", htr("after adjust"),
                htr("total assets"), funds.total_assets(), htr("cash"),
                funds.cash, htr("market_value"), funds.market_value);
  }

  //----------------------------------------------------------------------------
  // Run all the running systems; whether delayed or not, every running system
  // must be run once a day
  //----------------------------------------------------------------------------
  std::unordered_set<internal::StrategyRuntime*> delay_adjust_sys_set;
  for (auto& sw : delay_adjust_sys_list_) {
    delay_adjust_sys_set.insert(sw.strategy.get());
  }

  for (auto& sub_sys : running_sys_set_) {
    // HAYAKU_INFO_IF(trace, "[PF] run: {}", sub_sys->name());
    if (adjust) {
      auto sg = sub_sys->getSG();
      sg->startCycle(date, nextCycle);
      if (trace) {
        if (delay_adjust_sys_set.find(sub_sys.get()) ==
            delay_adjust_sys_set.end()) {
          if (sub_sys->getParam<bool>("buy_delay")) {
            HAYAKU_INFO_IF(
                sg->shouldBuy(date),
                htr("[PF] {} sg will buy on next open", sub_sys->name()));
          } else {
            HAYAKU_INFO_IF(
                sg->shouldBuy(date),
                htr("[PF] {} sg will buy on current close", sub_sys->name()));
          }
        } else {
          HAYAKU_INFO(
              htr("[PF] {} will adjust sell on next open", sub_sys->name()));
        }
      }
    }

    auto tr = sub_sys->runMoment(date);
    if (!tr.isNull()) {
      HAYAKU_INFO_IF(trace, "[PF] {}", tr);
      account_->addTradeRecord(tr);
    }
  }

  //----------------------------------------------------------------------
  // Print the assets of every subsystem after the execution for the trace
  //----------------------------------------------------------------------
  if (trace) {
    auto funds = account_->getFunds(date, query_.kType());
    HAYAKU_INFO("[PF] [{}] - {}: {}, {}: {}, {}: {}", htr("after run at close"),
                htr("total assets"), funds.total_assets(), htr("cash"),
                funds.cash, htr("market_value"), funds.market_value);
  }
}

json SimplePortfolio::lastSuggestion() const {
  json sys_json_list = json::array();
  for (const auto& sys : running_sys_set_) {
    sys_json_list.emplace_back(sys->lastSuggestion());
  }

  for (const auto& sw : delay_adjust_sys_list_) {
    sys_json_list.emplace_back(sw.strategy->lastSuggestion());
  }

  json ret;
  ret["name"] = name();
  ret["sys_list"] = sys_json_list;
  return ret;
}

PortfolioPtr HAYAKU_API PF_Simple(const internal::PortfolioAccountPortPtr& tm,
                                  const SEPtr& st, const AFPtr& af,
                                  int adjust_cycle, const string& adjust_mode,
                                  bool delay_to_trading_day) {
  PortfolioPtr ret = make_shared<SimplePortfolio>(tm, st, af);
  ret->setParam<int>("adjust_cycle", adjust_cycle);
  ret->setParam<string>("adjust_mode", adjust_mode);
  ret->setParam<bool>("delay_to_trading_day", delay_to_trading_day);
  return ret;
}

} /* namespace hayaku */
