/*
 * WithoutAFPortfolio.cpp
 *
 *  Created on: 2016-2-21
 *      Author: fasiondog
 */

#include "WithoutAFPortfolio.h"

#include "strategy/engine/internal/StrategyRuntime.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::WithoutAFPortfolio)
#endif

namespace hku {

WithoutAFPortfolio::WithoutAFPortfolio() : Portfolio("PF_WithoutAF") {
    initParam();
}

WithoutAFPortfolio::WithoutAFPortfolio(const internal::PortfolioAccountPortPtr& account,
                                       const SelectorPtr& se)
: Portfolio("PF_WithoutAF", account, se, AFPtr()) {
    initParam();
}

WithoutAFPortfolio::~WithoutAFPortfolio() {}

void WithoutAFPortfolio::initParam() {
    setParam<bool>("trade_on_close", true);         // Trade at the close
    setParam<bool>("strategy_use_own_account", false);
    setParam<bool>("sell_at_not_selected", false);  // Force selling the unselected positions
}

void WithoutAFPortfolio::_reset() {
    m_force_sell_sys_list.clear();
    m_running_sys_list.clear();
    m_selected_list.clear();
    m_se_sys_to_pf_sys_dict.clear();
}

void WithoutAFPortfolio::_readyForRun() {
    // Get the prototype system list from se
    auto pro_sys_list = m_se->getProtoSystemList();
    HKU_WARN_IF_RETURN(pro_sys_list.empty(), void(), "Can't fetch proto_sys_lsit from Selector!");

    // Only the modes where all are delayed or all are immediate are supported
    bool trade_on_close = getParam<bool>("trade_on_close");
    bool strategy_use_own_account = getParam<bool>("strategy_use_own_account");

    size_t total = pro_sys_list.size();
    m_real_sys_list.reserve(total);
    internal::StrategyRuntimeList se_sys_list;
    se_sys_list.reserve(total);
    for (size_t i = 0; i < total; i++) {
        internal::StrategyRuntimePtr& pro_sys = pro_sys_list[i];
        if (pro_sys) {
            auto sys = pro_sys->clone();
            sys->setParam<bool>("buy_delay", !trade_on_close);
            sys->setParam<bool>("sell_delay", !trade_on_close);
            sys->setParam<bool>("shared_account", true);

            auto se_sys = sys->clone();
            se_sys->setParam<bool>("shared_account", false);
            if (!strategy_use_own_account || !se_sys->getAccount()) {
                // When using its own tm or having no tm of its own, the pf tm is copied and used
                se_sys->setAccount(m_account->cloneAccount());
                se_sys_list.emplace_back(se_sys);
            }

            m_se->bindRealToProto(se_sys, pro_sys);
            m_real_sys_list.emplace_back(sys);
            m_se_sys_to_pf_sys_dict[se_sys] = sys;

            KData k = sys->getStock().getKData(m_query);
            se_sys->prepare();
            se_sys->bind(k);

            sys->setAccount(m_account);
            string sys_name = fmt::format("{}_{}_{}", sys->name(), sys->getStock().market_code(),
                                          sys->getStock().name());
            sys->name(fmt::format("PF_{}", sys_name));
            sys->prepare();
            sys->bind(k);
        }
    }

    // Tell se to calculate
    m_se->calculate(se_sys_list, m_query);
}

void WithoutAFPortfolio::_runMomentOnOpen(const Datetime& date, const Datetime& nextCycle,
                                          bool adjust) {
    // m_force_sell_sys_list is used here to cache the systems not selected in this round but still
    // running At the open, process the systems not selected in this round but still holding a
    // position; they are removed when there is no position left
    bool trace = getParam<bool>("trace");
    for (auto iter = m_force_sell_sys_list.begin(); iter != m_force_sell_sys_list.end();) {
        auto& sys = *iter;
        auto stk = sys->getStock();
        auto num = m_account->getHoldNumber(date, stk);
        if (iszero(num)) {
            HKU_INFO_IF(trace, htr("[PF] removed system {}", sys->name()));
            iter = m_force_sell_sys_list.erase(iter);
        } else {
            if (getParam<bool>("sell_at_not_selected")) {
                auto tr = sys->sellForceOnOpen(date, num, OrderOrigin::PORTFOLIO);
                HKU_INFO_IF(trace && !tr.isNull(),
                            htr("[PF] force sell not selected sys {}", sys->name()));
            }
            ++iter;
        }
    }

    if (!adjust) {
        // Run the systems outside the selected system pool first, letting them perform the possible
        // sell Note: a buy issued by some system that buys multiple times cannot be avoided for now
        if (getParam<bool>("sell_at_not_selected")) {
            for (auto& sys : m_force_sell_sys_list) {
                static_cast<void>(sys->runMomentOnOpen(date));
            }
        }

        for (auto& sys : m_running_sys_list) {
            static_cast<void>(sys->runMomentOnOpen(date));
        }

        return;
    }
}

void WithoutAFPortfolio::_runMomentOnClose(const Datetime& date, const Datetime& nextCycle,
                                           bool adjust) {
    bool trace = getParam<bool>("trace");
    //---------------------------------------------------
    // A non-adjustment day:
    // 1. Run the systems outside the selected system pool first, letting them perform the possible
    // sell
    // 2. Then run the currently running systems in turn
    //---------------------------------------------------
    if (!adjust) {
        // Run the systems outside the selected system pool first, letting them perform the possible
        // sell Note: a buy issued by some system that buys multiple times cannot be avoided for now
        if (getParam<bool>("sell_at_not_selected")) {
            for (auto& sys : m_force_sell_sys_list) {
                static_cast<void>(sys->runMomentOnClose(date));
            }
        }

        for (auto& sys : m_running_sys_list) {
            static_cast<void>(sys->runMomentOnClose(date));
        }

        return;
    }

    //---------------------------------------------------
    // On the adjustment day, re-select the system pool
    //---------------------------------------------------
    bool trade_on_close = getParam<bool>("trade_on_close");
    auto current_selected_list = m_se->getSelected(date);
    HKU_INFO_IF(trace, "[PF] {}: {}", htr("current select system count"),
                current_selected_list.size());

    m_selected_list.clear();
    for (auto& sw : current_selected_list) {
        m_selected_list.push_back(m_se_sys_to_pf_sys_dict[sw.strategy]);
    }

    // Remove the systems outside the selected system pool from the current running pool
    std::unordered_set<internal::StrategyRuntimePtr> tmp_selected_set;
    for (auto& sys : m_selected_list) {
        tmp_selected_set.insert(sys);
    }

    std::unordered_set<internal::StrategyRuntimePtr> will_remove_sys_list;
    for (auto& sys : m_running_sys_set) {
        if (tmp_selected_set.find(sys) == tmp_selected_set.end()) {
            will_remove_sys_list.insert(sys);
        }
    }

    size_t running_sys_count = m_running_sys_list.size();
    size_t out_sys_count = will_remove_sys_list.size();
    size_t in_sys_count = 0;

    for (auto& sys : will_remove_sys_list) {
        HKU_INFO_IF(trace, htr("[PF] will remove system: {}", sys->name()));
        m_running_sys_set.erase(sys);
        m_running_sys_list.remove(sys);
    }

    // When a removed system has a position, run it once to let it sell and add it to
    // m_force_sell_sys_list
    if (!trade_on_close) {
        for (auto& sys : will_remove_sys_list) {
            auto stk = sys->getStock();
            auto num = m_account->getHoldNumber(date, stk);
            if (!iszero(num)) {
                static_cast<void>(sys->runMomentOnClose(date));
            }
            m_force_sell_sys_list.emplace_back(sys);
        }
    }

    // Add it into the current running system set and set the position adjustment cycle
    for (auto& sys : m_selected_list) {
        auto [it, ok] = m_running_sys_set.insert(sys);
        if (ok) {
            m_running_sys_list.emplace_back(sys);
            auto sg = sys->getSG();
            sg->startCycle(date, nextCycle);
            in_sys_count++;
        }
    }

    //----------------------------------------------------------------------------
    // Run all the running systems in turn
    //----------------------------------------------------------------------------
    for (auto& sys : m_running_sys_list) {
        static_cast<void>(sys->runMomentOnClose(date));
    }

    // In the execute-at-close mode, sell the positions of the systems to be removed
    if (trade_on_close) {
        for (auto& sys : will_remove_sys_list) {
            auto stk = sys->getStock();
            auto num = m_account->getHoldNumber(date, stk);
            if (!iszero(num)) {
                if (getParam<bool>("sell_at_not_selected")) {
                    auto _ = sys->sellForceOnClose(date, num, OrderOrigin::PORTFOLIO);
                } else {
                    static_cast<void>(sys->runMomentOnClose(date));
                }
            }
            if (sys->getAccount()->have(stk)) {
                m_force_sell_sys_list.emplace_back(sys);
            }
        }
    }

    // Calculate the position adjustment turnover
    if (running_sys_count > 0) {
        m_adjust_turnover.emplace_back(
          date, static_cast<double>(in_sys_count + out_sys_count) / running_sys_count);
    }
}

void WithoutAFPortfolio::_runMomentWithoutAFNotForceSell(const Datetime& date,
                                                         const Datetime& nextCycle, bool adjust) {
    // m_force_sell_sys_list is used here to cache the systems not selected in this round but still
    // running At the open, process the systems not selected in this round but still holding a
    // position; they are removed when there is no position left
    bool trace = getParam<bool>("trace");
    for (auto iter = m_force_sell_sys_list.begin(); iter != m_force_sell_sys_list.end();) {
        auto& sys = *iter;
        auto stk = sys->getStock();
        auto num = m_account->getHoldNumber(date, stk);
        if (iszero(num)) {
            HKU_INFO_IF(trace, htr("[PF] removed system {}", sys->name()));
            iter = m_force_sell_sys_list.erase(iter);
        } else {
            ++iter;
        }
    }

    //---------------------------------------------------
    // A non-adjustment day:
    // 1. Run the systems outside the selected system pool first, letting them perform the possible
    // sell
    // 2. Then run the currently running systems in turn
    //---------------------------------------------------
    if (!adjust) {
        // Run the systems outside the selected system pool first, letting them perform the possible
        // sell Note: a buy issued by some system that buys multiple times cannot be avoided for now
        for (auto& sys : m_force_sell_sys_list) {
            static_cast<void>(sys->runMoment(date));
        }

        for (auto& sys : m_running_sys_list) {
            static_cast<void>(sys->runMoment(date));
        }
        return;
    }

    //---------------------------------------------------
    // On the adjustment day, re-select the system pool
    //---------------------------------------------------
    bool trade_on_close = getParam<bool>("trade_on_close");
    auto current_selected_list = m_se->getSelected(date);
    HKU_INFO_IF(trace, "[PF] {}: {}", htr("current select system count"),
                current_selected_list.size());

    m_selected_list.clear();
    for (auto& sw : current_selected_list) {
        m_selected_list.push_back(m_se_sys_to_pf_sys_dict[sw.strategy]);
    }

    // Remove the systems outside the selected system pool from the current running pool
    std::unordered_set<internal::StrategyRuntimePtr> tmp_selected_set;
    for (auto& sys : m_selected_list) {
        tmp_selected_set.insert(sys);
    }

    std::unordered_set<internal::StrategyRuntimePtr> will_remove_sys_list;
    for (auto& sys : m_running_sys_set) {
        if (tmp_selected_set.find(sys) == tmp_selected_set.end()) {
            will_remove_sys_list.insert(sys);
        }
    }

    for (auto& sys : will_remove_sys_list) {
        HKU_INFO_IF(trace, htr("[PF] will remove system: {}", sys->name()));
        m_running_sys_set.erase(sys);
        m_running_sys_list.remove(sys);
    }

    // When a removed system has a position, run it once to let it sell and add it to
    // m_force_sell_sys_list
    if (!trade_on_close) {
        for (auto& sys : will_remove_sys_list) {
            auto stk = sys->getStock();
            auto num = m_account->getHoldNumber(date, stk);
            if (!iszero(num)) {
                static_cast<void>(sys->runMoment(date));
            }
            m_force_sell_sys_list.emplace_back(sys);
        }
    }

    // Add it into the current running system set and set the position adjustment cycle
    for (auto& sys : m_selected_list) {
        auto [it, ok] = m_running_sys_set.insert(sys);
        if (ok) {
            m_running_sys_list.emplace_back(sys);
            auto sg = sys->getSG();
            sg->startCycle(date, nextCycle);
        }
    }

    //----------------------------------------------------------------------------
    // Run all the running systems in turn
    //----------------------------------------------------------------------------
    for (auto& sys : m_running_sys_list) {
        static_cast<void>(sys->runMoment(date));
    }

    // In the execute-at-close mode, sell the positions of the systems to be removed
    if (trade_on_close) {
        for (auto& sys : will_remove_sys_list) {
            auto stk = sys->getStock();
            auto num = m_account->getHoldNumber(date, stk);
            if (!iszero(num)) {
                static_cast<void>(sys->runMoment(date));
            }
            m_force_sell_sys_list.emplace_back(sys);
        }
    }
}

void WithoutAFPortfolio::_runMomentWithoutAFForceSell(const Datetime& date,
                                                      const Datetime& nextCycle, bool adjust) {
    bool trace = getParam<bool>("trace");
    // At the open, process the forced sell system list; a system without a position is removed from
    // the list, otherwise the sell is executed and it is kept (whether it still has a position is
    // judged on the next trading day)
    for (auto iter = m_force_sell_sys_list.begin(); iter != m_force_sell_sys_list.end();) {
        auto& sys = *iter;
        auto stk = sys->getStock();
        auto num = m_account->getHoldNumber(date, stk);
        if (iszero(num)) {
            iter = m_force_sell_sys_list.erase(iter);
        } else {
            auto tr = sys->sellForceOnOpen(date, num, OrderOrigin::PORTFOLIO);
            HKU_INFO_IF(trace && !tr.isNull(),
                        htr("[PF] force sell not selected sys {}", sys->name()));
            ++iter;
        }
    }

    //---------------------------------------------------
    // On a non-adjustment day, just run the currently running systems in turn
    //---------------------------------------------------
    if (!adjust) {
        for (auto& sys : m_running_sys_list) {
            static_cast<void>(sys->runMoment(date));
        }
        return;
    }

    //---------------------------------------------------
    // On the adjustment day, re-select the system pool
    //---------------------------------------------------
    bool trade_on_close = getParam<bool>("trade_on_close");
    auto current_selected_list = m_se->getSelected(date);
    HKU_INFO_IF(trace, "[PF] {}: {}", htr("current selected system count"),
                current_selected_list.size());

    m_selected_list.clear();
    for (auto& sw : current_selected_list) {
        m_selected_list.push_back(m_se_sys_to_pf_sys_dict[sw.strategy]);
    }

    // Remove the systems outside the selected system pool from the current running pool
    std::unordered_set<internal::StrategyRuntimePtr> tmp_selected_set;
    for (auto& sys : m_selected_list) {
        tmp_selected_set.insert(sys);
    }

    std::unordered_set<internal::StrategyRuntimePtr> will_remove_sys_list;
    for (auto& sys : m_running_sys_set) {
        if (tmp_selected_set.find(sys) == tmp_selected_set.end()) {
            will_remove_sys_list.insert(sys);
        }
    }

    for (auto& sys : will_remove_sys_list) {
        HKU_INFO_IF(trace, htr("[PF] remove system: {}", sys->name()));
        m_running_sys_set.erase(sys);
        m_running_sys_list.remove(sys);
    }

    // When a removed system has a position, force an immediate sell executed at the open
    if (!trade_on_close) {
        for (auto& sys : will_remove_sys_list) {
            auto stk = sys->getStock();
            auto num = m_account->getHoldNumber(date, stk);
            auto _ = sys->sellForceOnOpen(date, num, OrderOrigin::PORTFOLIO);
            m_force_sell_sys_list.emplace_back(sys);
        }
    }

    // Add it into the current running system set and set the position adjustment cycle
    for (auto& sys : m_selected_list) {
        auto [it, ok] = m_running_sys_set.insert(sys);
        if (ok) {
            m_running_sys_list.emplace_back(sys);
            auto sg = sys->getSG();
            sg->startCycle(date, nextCycle);
        }
    }
    //----------------------------------------------------------------------------
    // Run all the running systems in turn
    //----------------------------------------------------------------------------
    for (auto& sys : m_running_sys_list) {
        static_cast<void>(sys->runMoment(date));
    }

    // In the execute-at-close mode, sell the positions of the systems to be removed
    if (trade_on_close) {
        for (auto& sys : will_remove_sys_list) {
            auto stk = sys->getStock();
            auto num = m_account->getHoldNumber(date, stk);
            if (!iszero(num)) {
                auto tr = sys->sellForceOnClose(date, num, OrderOrigin::PORTFOLIO);
                HKU_INFO_IF(trace && !tr.isNull(),
                            htr("[PF] force sell not selected sys {}", sys->name()));
                m_force_sell_sys_list.emplace_back(sys);
            }
        }
    }
}

json WithoutAFPortfolio::lastSuggestion() const {
    json sys_json_list = json::array();
    for (const auto& sys : m_force_sell_sys_list) {
        sys_json_list.emplace_back(sys->lastSuggestion());
    }
    for (const auto& sys : m_running_sys_set) {
        sys_json_list.emplace_back(sys->lastSuggestion());
    }

    json ret;
    ret["name"] = name();
    ret["sys_list"] = sys_json_list;
    return ret;
}

PortfolioPtr HKU_API PF_WithoutAF(const internal::PortfolioAccountPortPtr& tm, const SEPtr& se, int adjust_cycle,
                                  const string& adjust_mode, bool delay_to_trading_day,
                                  bool trade_on_close, bool strategy_use_own_account,
                                  bool sell_at_not_selected) {
    PortfolioPtr ret = make_shared<WithoutAFPortfolio>(tm, se);
    ret->setParam<int>("adjust_cycle", adjust_cycle);
    ret->setParam<string>("adjust_mode", adjust_mode);
    ret->setParam<bool>("delay_to_trading_day", delay_to_trading_day);
    ret->setParam<bool>("trade_on_close", trade_on_close);
    ret->setParam<bool>("strategy_use_own_account", strategy_use_own_account);
    ret->setParam<bool>("sell_at_not_selected", sell_at_not_selected);
    return ret;
}

} /* namespace hku */
