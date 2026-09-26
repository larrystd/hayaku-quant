/*
 * test_Portfolio.cpp
 *
 *  Created on: 2013-4-20
 *      Author: fasiondog
 */

#include <data/DataRuntime.h>
#include <operators/SeriesOperators.h>
#include <operators/WindowOperators.h>
#include <strategy/decision/Signals.h>
#include <strategy/portfolio/AllocationPolicies.h>
#include <strategy/portfolio/Portfolios.h>
#include <strategy/risk/MoneyManagers.h>
#include <strategy/selection/Selectors.h>

#include "../selection/create_test_strategy.h"
#include "doctest/doctest.h"

using namespace hayaku;

/**
 * @defgroup test_Portfolio test_Portfolio
 * @ingroup test_hayaku_trade_sys_suite
 * @{
 */

/** @par Test point: all the instances are in the delay mode and the positions
 * are not adjusted */
TEST_CASE("test_PF_for_delay_and_no_adjust") {
  DataRuntime& sm = getDataRuntime();

  auto sys = create_test_strategy(
      MM_FixedCount(100), SG_CrossGold(EMA(CLOSE(), 12), EMA(CLOSE(), 26)));

  auto account =
      create_test_account(AccountConfig(Datetime(199001010000L), 500000));
  SEPtr se = SE_Fixed();
  se->addStockList({sm["sz000001"], sm["sz000063"], sm["sz000651"]}, sys);
  AFPtr af = AF_EqualWeight();
  PFPtr pf = PF_Simple(account, se, af);

  KQuery query =
      KQueryByDate(Datetime(201101010000L), Null<Datetime>(), KQuery::DAY);
  pf->run(query);

  /** @arg */
}

/** @} */
