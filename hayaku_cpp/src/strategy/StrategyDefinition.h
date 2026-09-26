#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Explicit, System-independent strategy component graph.
 */

#include "common/Parameter.h"
#include "execution/pricing/SlippageBase.h"
#include "strategy/decision/ConditionBase.h"
#include "strategy/decision/EnvironmentBase.h"
#include "strategy/decision/SignalBase.h"
#include "strategy/risk/MoneyManagerBase.h"
#include "strategy/risk/ProfitGoalBase.h"
#include "strategy/risk/StoplossBase.h"

namespace hayaku {

/** Immutable ownership of the components needed to run one strategy. */
class HAYAKU_API StrategyDefinition {
 public:
  StrategyDefinition(MoneyManagerPtr moneyManager, SignalPtr signal,
                     string name = "Strategy", EnvironmentPtr environment = {},
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

inline const MoneyManagerPtr& StrategyDefinition::moneyManager()
    const noexcept {
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

inline bool StrategyDefinition::operator!=(
    const StrategyDefinition& other) const {
  return !(*this == other);
}

}  // namespace hayaku
