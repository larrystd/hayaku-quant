/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "StrategyDefinition.h"

#include <utility>

namespace hayaku {

StrategyDefinition::StrategyDefinition(
    MoneyManagerPtr moneyManager, SignalPtr signal, string name,
    EnvironmentPtr environment, ConditionPtr condition, StoplossPtr stoploss,
    StoplossPtr takeProfit, ProfitGoalPtr profitGoal, SlippagePtr slippage,
    Parameter parameters)
    : m_moneyManager(std::move(moneyManager)),
      m_signal(std::move(signal)),
      m_name(std::move(name)),
      m_environment(std::move(environment)),
      m_condition(std::move(condition)),
      m_stoploss(std::move(stoploss)),
      m_takeProfit(std::move(takeProfit)),
      m_profitGoal(std::move(profitGoal)),
      m_slippage(std::move(slippage)),
      m_parameters(std::move(parameters)) {
  HAYAKU_CHECK(m_moneyManager, "StrategyDefinition requires a MoneyManager");
  HAYAKU_CHECK(m_signal, "StrategyDefinition requires a Signal");
}

bool StrategyDefinition::operator==(const StrategyDefinition& other) const {
  return m_moneyManager == other.m_moneyManager && m_signal == other.m_signal &&
         m_name == other.m_name && m_environment == other.m_environment &&
         m_condition == other.m_condition && m_stoploss == other.m_stoploss &&
         m_takeProfit == other.m_takeProfit &&
         m_profitGoal == other.m_profitGoal && m_slippage == other.m_slippage &&
         m_parameters == other.m_parameters;
}

}  // namespace hayaku
