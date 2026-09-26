/*
 * test_MM_FixedCount.cpp
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#include <data/DataRuntime.h>
#include <execution/pricing/TradeCosts.h>
#include <strategy/risk/MoneyManagers.h>

#include "../selection/create_test_strategy.h"
#include "doctest/doctest.h"

using namespace hayaku;

/**
 * @defgroup test_MM_FixedCount test_MM_FixedCount
 * @ingroup test_hayaku_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MM_FixedCount") {
  DataRuntime& sm = getDataRuntime();
  Stock stock = sm.getStock("sh600000");
  internal::PortfolioAccountPortPtr tm = create_test_account(
      AccountConfig(Datetime(199001010000LL), 0.0, TC_FixedA()));

  /** @arg n < 1 */
  CHECK_THROWS_AS(MM_FixedCount(0), std::exception);

  /** @arg n = 100; a trade account with the initial capital 0 can execute a buy
   */
  tm = create_test_account(
      AccountConfig(Datetime(199001010000LL), 0.0, TC_FixedA()));
  CHECK_EQ(tm->initCash(), 0.0);
  auto mm = MM_FixedCount(100);
  mm->setAccount(tm);
  mm->setParam<bool>("auto-checkin", true);
  mm->getBuyNumber(Datetime(200001200000), stock, 24.11, 24.11,
                   OrderOrigin::SIGNAL);
  CHECK_EQ(tm->cash(Datetime(200001200000)), 2417.02);
}

/** @} */
