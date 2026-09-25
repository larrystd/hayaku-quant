/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "doctest/doctest.h"
#include <data/internal/DataRuntime.h>
#include <app/HikyuuSession.h>
#include <app/SessionOptions.h>

using namespace hku;

/**
 * @defgroup test_dataengine_suite test_dataengine_suite
 * @ingroup test_hikyuu_data_suite
 * @{
 */

namespace {

SessionOptions makeDataEngineOptions() {
    auto& sm = getDataRuntime();
    return SessionOptions(sm.getBaseInfoDriverParameter(), sm.getBlockDriverParameter(),
                          sm.getKDataDriverParameter(), sm.getPreloadParameter(),
                          sm.getHikyuuParameter(), sm.getStrategyContext());
}

}  // namespace

TEST_CASE("test_DataEngine_stock_queries") {
    auto session = HikyuuSession::open(makeDataEngineOptions());
    const auto& data = session.data();
    auto& sm = getDataRuntime();

    /** @arg DataEngine preserves the existing security count. */
    CHECK_EQ(data.size(), sm.size());

    /** @arg Existing and missing security queries match the compatibility backend. */
    CHECK_EQ(data.getStock("sh000001"), sm.getStock("sh000001"));
    CHECK_EQ(data.getStock("missing"), sm.getStock("missing"));

    /** @arg Market metadata remains unchanged. */
    CHECK_EQ(data.getMarketInfo("SH"), sm.getMarketInfo("SH"));
    CHECK_EQ(data.getMarketStock("SH"), sm.getMarketStock("SH"));
    CHECK_EQ(data.getMarketList(), sm.getAllMarket());
}

TEST_CASE("test_DataEngine_market_data_queries") {
    auto session = HikyuuSession::open(makeDataEngineOptions());
    const auto& data = session.data();
    auto query = KQuery(-20);

    /** @arg KData results match direct DataRuntime access. */
    auto actual = data.getKData("sh000001", query);
    auto expected = getDataRuntime().getStock("sh000001").getKData(query);
    CHECK_EQ(actual.size(), expected.size());
    CHECK_EQ(actual.getDatetimeList(), expected.getDatetimeList());

    /** @arg Trading calendars match the compatibility backend. */
    CHECK_EQ(data.getTradingCalendar(query), getDataRuntime().getTradingCalendar(query));

    /** @arg Historical-finance field metadata remains available without driver access. */
    CHECK_EQ(data.getHistoryFinanceAllFields(),
             getDataRuntime().getHistoryFinanceAllFields());
}

TEST_CASE("test_DataEngine_block_queries") {
    auto session = HikyuuSession::open(makeDataEngineOptions());
    const auto& data = session.data();
    auto& sm = getDataRuntime();

    /** @arg Block category and block lookup results remain unchanged. */
    CHECK_EQ(data.getBlockCategoryList(), sm.getAllCategory());
    CHECK_EQ(data.getBlock("指数板块", "上证50"), sm.getBlock("指数板块", "上证50"));
}

/** @} */
