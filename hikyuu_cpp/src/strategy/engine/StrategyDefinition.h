/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Explicit, System-independent strategy component graph.
 */

#pragma once
#ifndef HIKYUU_TRADE_SYS_ENGINE_STRATEGYDEFINITION_H
#define HIKYUU_TRADE_SYS_ENGINE_STRATEGYDEFINITION_H

#include "strategy/condition/ConditionBase.h"
#include "strategy/environment/EnvironmentBase.h"
#include "strategy/moneymanager/MoneyManagerBase.h"
#include "strategy/profitgoal/ProfitGoalBase.h"
#include "strategy/signal/SignalBase.h"
#include "strategy/slippage/SlippageBase.h"
#include "strategy/stoploss/StoplossBase.h"
#include "common/Parameter.h"

namespace hku {

/** Immutable ownership of the components needed to run one strategy. */
class HKU_API StrategyDefinition {
public:
    StrategyDefinition(MoneyManagerPtr moneyManager, SignalPtr signal, string name = "Strategy",
                       EnvironmentPtr environment = {},
                       ConditionPtr condition = {}, StoplossPtr stoploss = {},
                       StoplossPtr takeProfit = {}, ProfitGoalPtr profitGoal = {},
                       SlippagePtr slippage = {}, Parameter parameters = {});

    [[nodiscard]] const string& name() const noexcept;
    [[nodiscard]] const MoneyManagerPtr& moneyManager() const noexcept;
    [[nodiscard]] const SignalPtr& signal() const noexcept;
    [[nodiscard]] const EnvironmentPtr& environment() const noexcept;
    [[nodiscard]] const ConditionPtr& condition() const noexcept;
    [[nodiscard]] const StoplossPtr& stoploss() const noexcept;
    [[nodiscard]] const StoplossPtr& takeProfit() const noexcept;
    [[nodiscard]] const ProfitGoalPtr& profitGoal() const noexcept;
    [[nodiscard]] const SlippagePtr& slippage() const noexcept;
    [[nodiscard]] const Parameter& parameters() const noexcept;

    [[nodiscard]] bool operator==(const StrategyDefinition& other) const;
    [[nodiscard]] bool operator!=(const StrategyDefinition& other) const;

private:
    MoneyManagerPtr m_moneyManager;
    SignalPtr m_signal;
    string m_name;
    EnvironmentPtr m_environment;
    ConditionPtr m_condition;
    StoplossPtr m_stoploss;
    StoplossPtr m_takeProfit;
    ProfitGoalPtr m_profitGoal;
    SlippagePtr m_slippage;
    Parameter m_parameters;
};

inline const string& StrategyDefinition::name() const noexcept {
    return m_name;
}

inline const MoneyManagerPtr& StrategyDefinition::moneyManager() const noexcept {
    return m_moneyManager;
}

inline const SignalPtr& StrategyDefinition::signal() const noexcept {
    return m_signal;
}

inline const EnvironmentPtr& StrategyDefinition::environment() const noexcept {
    return m_environment;
}

inline const ConditionPtr& StrategyDefinition::condition() const noexcept {
    return m_condition;
}

inline const StoplossPtr& StrategyDefinition::stoploss() const noexcept {
    return m_stoploss;
}

inline const StoplossPtr& StrategyDefinition::takeProfit() const noexcept {
    return m_takeProfit;
}

inline const ProfitGoalPtr& StrategyDefinition::profitGoal() const noexcept {
    return m_profitGoal;
}

inline const SlippagePtr& StrategyDefinition::slippage() const noexcept {
    return m_slippage;
}

inline const Parameter& StrategyDefinition::parameters() const noexcept {
    return m_parameters;
}

inline bool StrategyDefinition::operator!=(const StrategyDefinition& other) const {
    return !(*this == other);
}

}  // namespace hku

#endif /* HIKYUU_TRADE_SYS_ENGINE_STRATEGYDEFINITION_H */
