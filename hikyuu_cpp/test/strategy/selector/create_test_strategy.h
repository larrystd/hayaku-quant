/*
 * Copyright (c) 2026 hikyuu.org
 */

#pragma once

#include "data/indicator/crt/KDATA.h"
#include "data/indicator/crt/MA.h"
#include "data/indicator/crt/NOT.h"
#include "execution/AccountConfig.h"
#include "execution/internal/ExecutionAccountFactory.h"
#include "execution/internal/PortfolioAccountPort.h"
#include "strategy/engine/StrategyDefinition.h"
#include "strategy/engine/internal/StrategyRuntime.h"
#include "strategy/moneymanager/crt/MM_Nothing.h"
#include "strategy/signal/crt/SG_Bool.h"

using namespace hku;

inline internal::PortfolioAccountPortPtr create_test_account(
  const AccountConfig& config = AccountConfig()) {
    auto account = internal::makeExecutionAccount(config);
    auto portfolio_account = std::dynamic_pointer_cast<internal::PortfolioAccountPort>(account);
    HKU_CHECK(portfolio_account, "Test execution account must implement PortfolioAccountPort");
    return portfolio_account;
}

inline internal::StrategyRuntimePtr create_test_strategy(
  MoneyManagerPtr money_manager, SignalPtr signal, string name = "test_strategy",
  const AccountConfig& account_config = AccountConfig()) {
    auto definition =
      StrategyDefinition(std::move(money_manager), std::move(signal), std::move(name));
    return std::make_shared<internal::StrategyRuntime>(definition,
                                                       create_test_account(account_config));
}

inline internal::StrategyRuntimePtr create_test_strategy(int fast_n, int slow_n) {
    auto ind = MA(CLOSE(), fast_n) > MA(CLOSE(), slow_n);
    return create_test_strategy(MM_Nothing(), SG_Bool(ind, NOT(ind)),
                                fmt::format("test_sys_{}_{}", fast_n, slow_n));
}
