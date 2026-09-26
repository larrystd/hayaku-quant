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
    : money_manager_(std::move(moneyManager)),
      signal_(std::move(signal)),
      name_(std::move(name)),
      environment_(std::move(environment)),
      condition_(std::move(condition)),
      stoploss_(std::move(stoploss)),
      take_profit_(std::move(takeProfit)),
      profit_goal_(std::move(profitGoal)),
      slippage_(std::move(slippage)),
      parameters_(std::move(parameters)) {
  HAYAKU_CHECK(money_manager_, "StrategyDefinition requires a MoneyManager");
  HAYAKU_CHECK(signal_, "StrategyDefinition requires a Signal");
}

bool StrategyDefinition::operator==(const StrategyDefinition& other) const {
  return money_manager_ == other.money_manager_ && signal_ == other.signal_ &&
         name_ == other.name_ && environment_ == other.environment_ &&
         condition_ == other.condition_ && stoploss_ == other.stoploss_ &&
         take_profit_ == other.take_profit_ &&
         profit_goal_ == other.profit_goal_ && slippage_ == other.slippage_ &&
         parameters_ == other.parameters_;
}

}  // namespace hayaku
