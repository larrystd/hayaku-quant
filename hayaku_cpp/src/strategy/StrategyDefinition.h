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
class StrategyDefinition {
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
  MoneyManagerPtr money_manager_;
  SignalPtr signal_;
  string name_;
  EnvironmentPtr environment_;
  ConditionPtr condition_;
  StoplossPtr stoploss_;
  StoplossPtr take_profit_;
  ProfitGoalPtr profit_goal_;
  SlippagePtr slippage_;
  Parameter parameters_;
};

inline const string& StrategyDefinition::name() const noexcept { return name_; }

inline const MoneyManagerPtr& StrategyDefinition::moneyManager()
    const noexcept {
  return money_manager_;
}

inline const SignalPtr& StrategyDefinition::signal() const noexcept {
  return signal_;
}

inline const EnvironmentPtr& StrategyDefinition::environment() const noexcept {
  return environment_;
}

inline const ConditionPtr& StrategyDefinition::condition() const noexcept {
  return condition_;
}

inline const StoplossPtr& StrategyDefinition::stoploss() const noexcept {
  return stoploss_;
}

inline const StoplossPtr& StrategyDefinition::takeProfit() const noexcept {
  return take_profit_;
}

inline const ProfitGoalPtr& StrategyDefinition::profitGoal() const noexcept {
  return profit_goal_;
}

inline const SlippagePtr& StrategyDefinition::slippage() const noexcept {
  return slippage_;
}

inline const Parameter& StrategyDefinition::parameters() const noexcept {
  return parameters_;
}

inline bool StrategyDefinition::operator!=(
    const StrategyDefinition& other) const {
  return !(*this == other);
}

}  // namespace hayaku
