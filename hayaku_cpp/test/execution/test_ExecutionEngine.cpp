/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "doctest/doctest.h"
#include <data/DataRuntime.h>
#include <execution/ExecutionEngine.h>
#include <execution/pricing/TradeCosts.h>
#include <execution/ExecutionAccountFactory.h>
#include <execution/ExecutionBrokerPort.h>
#include <execution/PortfolioAccountPort.h>

using namespace hayaku;

namespace {

class TestOrderBroker final : public OrderBrokerBase {
public:
    explicit TestOrderBroker(string assetInfo) : m_assetInfo(std::move(assetInfo)) {}

    void _buy(Datetime, const string&, const string&, price_t, double, price_t, price_t,
              OrderOrigin, const string&) override {}

    void _sell(Datetime, const string&, const string&, price_t, double, price_t, price_t,
               OrderOrigin, const string&) override {}

    string _getAssetInfo() override {
        return m_assetInfo;
    }

private:
    string m_assetInfo;
};

}  // namespace

/**
 * @defgroup test_executionengine_suite test_executionengine_suite
 * @ingroup test_hayaku_trade_suite
 * @{
 */

TEST_CASE("test_ExecutionEngine_constructor") {
    /** @arg account construction is explicit and independent of the removed legacy facade. */
    ExecutionEngine engine(AccountConfig(Datetime(199901010000), 100000.0, TC_Zero(),
                                         "EXECUTION"));
    CHECK_UNARY(engine.accountId().valid());
    CHECK_EQ(engine.history().size(), 1);
}

TEST_CASE("test_OrderOrigin_conversion") {
    /** @arg execution origins preserve the legacy stable wire names */
    CHECK_EQ(getOrderOriginName(OrderOrigin::SIGNAL), "SG");
    CHECK_EQ(getOrderOriginName(OrderOrigin::ALLOCATION), "AF");
    CHECK_EQ(getOrderOriginEnum("pf"), OrderOrigin::PORTFOLIO);

    /** @arg an unknown origin maps to the explicit unspecified value */
    CHECK_EQ(getOrderOriginEnum("unknown"), OrderOrigin::UNSPECIFIED);
    CHECK_EQ(getOrderOriginName(OrderOrigin::UNSPECIFIED), "--");
}

TEST_CASE("test_ExecutionEngine_owns_runtime") {
    /** @arg the new construction path owns a real account runtime without a compatibility facade */
    const AccountId requestedId(42);
    ExecutionEngine engine(AccountConfig(Datetime(199901010000), 100000.0, TC_Zero(), "OWNED",
                                         2, false, false, requestedId));
    CHECK_EQ(engine.accountId(), requestedId);
    CHECK_EQ(engine.history().size(), 1);

    const AccountView initial = engine.view();
    CHECK_EQ(initial.id(), requestedId);
    CHECK_EQ(initial.initDatetime(), Datetime(199901010000));
    CHECK_EQ(initial.funds().cash, 100000.0);
    CHECK_UNARY(initial.positions().empty());

    Stock stock = getDataRuntime().getStock("sh600000");
    ExecutionReport report = engine.submit(OrderRequest(
      OrderSide::BUY, Datetime(199911170000), stock, 27.18, 100, 20.0, 35.0, 27.0,
      OrderOrigin::SIGNAL, "runtime-owned buy"));
    REQUIRE_UNARY(report.filled());
    CHECK_EQ(report.trade().from, OrderOrigin::SIGNAL);

    const AccountView afterBuy = engine.view();
    CHECK_EQ(afterBuy.positions().size(), 1);
    CHECK_EQ(afterBuy.funds(), engine.snapshot().funds());
}

TEST_CASE("test_ExecutionEngine_submit_long_order") {
    Stock stock = getDataRuntime().getStock("sh600000");
    ExecutionEngine engine(AccountConfig(Datetime(199901010000), 100000.0, TC_Zero(),
                                         "EXECUTION"));

    /** @arg a valid buy uses the extracted runtime accounting semantics */
    OrderRequest buyRequest(OrderSide::BUY, Datetime(199911170000), stock, 27.18, 100, 20.0, 35.0,
                            27.0, OrderOrigin::SIGNAL, "engine buy");
    ExecutionReport buyReport = engine.submit(buyRequest);
    CHECK_UNARY(buyReport.filled());
    CHECK_EQ(buyReport.status(), ExecutionStatus::FILLED);
    CHECK_EQ(buyReport.trade().business, BUSINESS_BUY);
    CHECK_EQ(buyReport.trade(), engine.history().back());
    CHECK_EQ(buyReport.trade().remark, "engine buy");

    /** @arg a valid sell updates the same bound account */
    OrderRequest sellRequest(OrderSide::SELL, Datetime(199911180000), stock, 27.20, MAX_DOUBLE, 0.0,
                             0.0, 27.20, OrderOrigin::STOP_LOSS, "engine sell");
    ExecutionReport sellReport = engine.submit(sellRequest);
    CHECK_UNARY(sellReport.filled());
    CHECK_EQ(sellReport.trade().business, BUSINESS_SELL);
    CHECK_UNARY(engine.view().positions().empty());
    CHECK_EQ(engine.history().back(), sellReport.trade());
}

TEST_CASE("test_ExecutionEngine_submit_rejected_order") {
    Stock stock = getDataRuntime().getStock("sh600000");
    ExecutionEngine engine(
      AccountConfig(Datetime(199901010000), 0.0, TC_Zero(), "EXECUTION"));

    /** @arg normal business rejection is represented by status, not a facade exception */
    ExecutionReport report =
      engine.submit(OrderRequest(OrderSide::BUY, Datetime(199911170000), stock, 27.18, 100));
    CHECK_UNARY(report.rejected());
    CHECK_EQ(report.status(), ExecutionStatus::REJECTED);
    CHECK_UNARY(report.trade().isNull());
    CHECK_EQ(engine.history().size(), 1);
}

TEST_CASE("test_ExecutionEngine_submit_short_order") {
    Stock stock = getDataRuntime().getStock("sh600000");
    ExecutionEngine engine(AccountConfig(Datetime(199901010000), 100000.0, TC_Zero(),
                                         "EXECUTION", 2, false, true));

    /** @arg enabled securities lending is handled inside the owned runtime */
    ExecutionReport sellReport =
      engine.submit(OrderRequest(OrderSide::SELL_SHORT, Datetime(199911170000), stock, 10.0, 100));
    CHECK_UNARY(sellReport.filled());
    CHECK_EQ(sellReport.trade().business, BUSINESS_SELL_SHORT);
    CHECK_EQ(sellReport.trade().number, 100.0);

    /** @arg buy-to-cover preserves MAX_DOUBLE whole-position behavior */
    ExecutionReport buyReport = engine.submit(
      OrderRequest(OrderSide::BUY_SHORT, Datetime(199911180000), stock, 10.0, MAX_DOUBLE));
    CHECK_UNARY(buyReport.filled());
    CHECK_EQ(buyReport.trade().business, BUSINESS_BUY_SHORT);
    CHECK_EQ(buyReport.trade().number, 100.0);
    CHECK_UNARY(engine.view().shortPositions().empty());
}

TEST_CASE("test_ExecutionEngine_snapshot") {
    Stock stock = getDataRuntime().getStock("sh600000");
    ExecutionEngine engine(AccountConfig(Datetime(199901010000), 100000.0, TC_Zero(),
                                         "EXECUTION"));
    ExecutionReport report =
      engine.submit(OrderRequest(OrderSide::BUY, Datetime(199911170000), stock, 27.18, 100));
    REQUIRE_UNARY(report.filled());

    /** @arg snapshot returns value-owned funds and position collections */
    AccountSnapshot snapshot = engine.snapshot();
    const AccountView view = engine.view();
    CHECK_EQ(snapshot.funds(), view.funds());
    CHECK_EQ(snapshot.positions(), view.positions());
    CHECK_EQ(snapshot.shortPositions(), view.shortPositions());
    CHECK_EQ(snapshot.positions().size(), 1);
}

TEST_CASE("test_makeExecutionAccount_port_boundaries") {
    auto account = internal::makeExecutionAccount(
      AccountConfig(Datetime(199901010000), 100000.0, TC_Zero(), "PORT"));

    /** @arg the internal factory exposes the core account contract only */
    REQUIRE_UNARY(account);
    CHECK_EQ(account->currentCash(), 100000.0);
    CHECK_EQ(account->getProfitCurve({Datetime(199901020000)}).size(), 1);

    /** @arg portfolio capabilities are available through the explicit extension */
    auto portfolio = std::dynamic_pointer_cast<internal::PortfolioAccountPort>(account);
    REQUIRE_UNARY(portfolio);
    auto child = portfolio->createChildAccount("CHILD", 1234.0);
    REQUIRE_UNARY(child);
    CHECK_EQ(child->currentCash(), 1234.0);
    auto cloned = portfolio->cloneAccount();
    REQUIRE_UNARY(cloned);
    CHECK_EQ(cloned->currentCash(), account->currentCash());
    CHECK_NE(cloned->accountId(), account->accountId());
}

TEST_CASE("test_ExecutionBrokerPort_fetchAssetInfoFromBroker") {
    auto account = internal::makeExecutionAccount(AccountConfig(Datetime(199901010000), 0.0));
    auto brokerPort = std::dynamic_pointer_cast<internal::ExecutionBrokerPort>(account);
    REQUIRE_UNARY(brokerPort);

    auto broker = std::make_shared<TestOrderBroker>(R"json({
        "datetime": "1999-11-17 00:00:00",
        "cash": 8765.0,
        "positions": [{
            "market": "SH", "code": "600000", "number": 100.0,
            "stoploss": 20.0, "goal_price": 35.0, "cost_price": 27.18
        }]
    })json");

    /** @arg a live broker snapshot replaces cash and positions in the owned ledger */
    brokerPort->fetchAssetInfoFromBroker(broker);
    CHECK_EQ(account->currentCash(), 8765.0);
    CHECK_EQ(account->getStockNumber(), 1);
    CHECK_UNARY(account->have(getDataRuntime().getStock("sh600000")));
}

/** @} */
