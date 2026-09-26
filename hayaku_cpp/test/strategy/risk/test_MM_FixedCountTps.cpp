/*
 * test_MM_FixedCountTps.cpp
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <data/DataRuntime.h>
#include "../selection/create_test_strategy.h"
#include <execution/pricing/TradeCosts.h>
#include <strategy/risk/MoneyManagers.h>

using namespace hayaku;

/**
 * @defgroup test_MM_FixedCountTps test_MM_FixedCountTps
 * @ingroup test_hayaku_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MM_FixedCountTpsTps") {
    DataRuntime& sm = getDataRuntime();
    Stock stock = sm.getStock("sh600000");
    internal::PortfolioAccountPortPtr tm = create_test_account(AccountConfig(Datetime(199001010000LL), 0.0, TC_FixedA()));

    /** @arg The buy / sell quantity contains a value less than 0 */
    CHECK_THROWS_AS(MM_FixedCountTps({100., -1}, {100., 100.}), std::exception);
    CHECK_THROWS_AS(MM_FixedCountTps({100., 100.}, {100., -100.}), std::exception);

    /** @arg The total buy and sell quantities differ (a warning is printed only) */
    auto _ = MM_FixedCountTps({100., 100.}, {200., 200.});

    /** @arg n = 100; a trade account with the initial capital 0 auto-fills and can buy */
    tm = create_test_account(AccountConfig(Datetime(199001010000LL), 0.0, TC_FixedA()));
    CHECK_EQ(tm->initCash(), 0.0);
    auto mm = MM_FixedCountTps({100., 200.}, {200., 100.});
    mm->setAccount(tm);
    mm->setParam<bool>("auto-checkin", true);
    CHECK_EQ(mm->currentBuyCount(stock), 0);
    double buy_num = mm->getBuyNumber(Datetime(200001200000), stock, 24.11, 24.11, OrderOrigin::SIGNAL);
    CHECK_EQ(buy_num, 100);
    CHECK_EQ(tm->cash(Datetime(200001200000)), 2417.02);
    auto tr = tm
                ->submit(OrderRequest(OrderSide::BUY, Datetime(200001200000), stock, 24.11, 100,
                                      0, 0, 24.11, OrderOrigin::SIGNAL))
                .trade();
    mm->buyNotify(tr);
    CHECK_EQ(mm->currentBuyCount(stock), 1);

    buy_num = mm->getBuyNumber(Datetime(200001200000), stock, 24.11, 24.11, OrderOrigin::SIGNAL);
    CHECK_EQ(buy_num, 200);
    tr = tm
           ->submit(OrderRequest(OrderSide::BUY, Datetime(200001200000), stock, 24.11, 100, 0, 0,
                                 24.11, OrderOrigin::SIGNAL))
           .trade();
    mm->buyNotify(tr);
    CHECK_EQ(mm->currentBuyCount(stock), 2);

    buy_num = mm->getBuyNumber(Datetime(200001200000), stock, 24.11, 24.11, OrderOrigin::SIGNAL);
    CHECK_EQ(buy_num, 0);

    double sell_num = mm->getSellNumber(Datetime(200001210000), stock, 24.11, 24.11, OrderOrigin::SIGNAL);
    CHECK_EQ(sell_num, 200);
    tr = tm
           ->submit(OrderRequest(OrderSide::SELL, Datetime(200001200000), stock, 24.11, 100, 0, 0,
                                 24.11, OrderOrigin::SIGNAL))
           .trade();
    mm->sellNotify(tr);
    CHECK_EQ(mm->currentBuyCount(stock), 0);
    CHECK_EQ(mm->currentSellCount(stock), 1);

    sell_num = mm->getSellNumber(Datetime(200001210000), stock, 24.11, 24.11, OrderOrigin::SIGNAL);
    CHECK_EQ(sell_num, 100);
    tr = tm
           ->submit(OrderRequest(OrderSide::SELL, Datetime(200001200000), stock, 24.11, 100, 0, 0,
                                 24.11, OrderOrigin::SIGNAL))
           .trade();
    mm->sellNotify(tr);
    CHECK_EQ(mm->currentBuyCount(stock), 0);
    CHECK_EQ(mm->currentSellCount(stock), 2);

    buy_num = mm->getBuyNumber(Datetime(200001202000), stock, 24.11, 24.11, OrderOrigin::SIGNAL);
    CHECK_EQ(buy_num, 100);
    tr = tm
           ->submit(OrderRequest(OrderSide::BUY, Datetime(200001202000), stock, 24.11, 100, 0, 0,
                                 24.11, OrderOrigin::SIGNAL))
           .trade();
    mm->buyNotify(tr);
    CHECK_EQ(mm->currentBuyCount(stock), 1);
    CHECK_EQ(mm->currentSellCount(stock), 0);
}

/** @} */
