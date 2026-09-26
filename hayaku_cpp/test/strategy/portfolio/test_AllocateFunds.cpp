/*
 * test_AllocateFunds.cpp
 *
 *  Created on: 2018-2-11
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <data/DataRuntime.h>
#include "../selection/create_test_strategy.h"
#include <strategy/selection/Selectors.h>
#include <strategy/portfolio/AllocationPolicies.h>

using namespace hayaku;

/**
 * @defgroup test_AllocateFunds test_AllocateFunds
 * @ingroup test_hayaku_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_AllocateFunds") {
    AFPtr af = AF_EqualWeight();
    auto account = create_test_account(AccountConfig(Datetime(200101010000L), 100000));

    /** @arg Test account binding */
    CHECK_UNARY(!af->getAccount());
    af->setAccount(account);
    CHECK_EQ(af->getAccount(), account);

    /** @arg Test clone */
    AFPtr af2 = af->clone();
    CHECK_NE(af2.get(), af.get());
    CHECK_EQ(af2->name(), af->name());

    //----------------------------------------------------
    // Test the adjustment of the planned weights
    //----------------------------------------------------
    /** @arg No automatic adjustment with auto_adjust = false */
    StrategyWeightList sw(10);
    StrategyWeightList expect_sw(10);
    for (size_t i = 0; i < 10; i++) {
        sw[i].weight = i;
        expect_sw[i].weight = 9 - i;
    }

    AllocateFundsBase::adjustWeight(sw, 1.0, false, false);
    CHECK_EQ(sw.size(), 9);
    for (size_t i = 0; i < 9; i++) {
        CHECK_EQ(sw[i].weight, expect_sw[i].weight);
    }

    /** @arg auto_adjust = true, ignore_zero = true*/
    sw.clear();
    sw.emplace_back(internal::StrategyRuntimePtr(), 0.0);
    sw.emplace_back(internal::StrategyRuntimePtr(), 2.0);
    sw.emplace_back(internal::StrategyRuntimePtr(), Null<price_t>());
    sw.emplace_back(internal::StrategyRuntimePtr(), 3.0);
    expect_sw.clear();
    expect_sw.emplace_back(internal::StrategyRuntimePtr(), 3.0 / 5.0 * 0.8);
    expect_sw.emplace_back(internal::StrategyRuntimePtr(), 2.0 / 5.0 * 0.8);

    AllocateFundsBase::adjustWeight(sw, 0.8, true, true);
    CHECK_EQ(sw.size(), expect_sw.size());
    CHECK_EQ(sw[0].weight, doctest::Approx(expect_sw[0].weight));
    CHECK_EQ(sw[1].weight, doctest::Approx(expect_sw[1].weight));

    /** @arg auto_adjust = true, ignore_zero = false*/
    sw.clear();
    sw.emplace_back(internal::StrategyRuntimePtr(), 0.0);
    sw.emplace_back(internal::StrategyRuntimePtr(), 2.0);
    sw.emplace_back(internal::StrategyRuntimePtr(), Null<price_t>());
    sw.emplace_back(internal::StrategyRuntimePtr(), 3.0);
    expect_sw.clear();
    expect_sw.emplace_back(internal::StrategyRuntimePtr(), 3.0 / 5.0 * (2.0 / 4.0) * 0.8);
    expect_sw.emplace_back(internal::StrategyRuntimePtr(), 2.0 / 5.0 * (2.0 / 4.0) * 0.8);

    AllocateFundsBase::adjustWeight(sw, 0.8, true, false);
    CHECK_EQ(sw.size(), expect_sw.size());
    CHECK_EQ(sw[0].weight, doctest::Approx(expect_sw[0].weight));
    CHECK_EQ(sw[1].weight, doctest::Approx(expect_sw[1].weight));
}

/** @} */
