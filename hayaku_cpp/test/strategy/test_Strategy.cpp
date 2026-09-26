/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Strategy public-order routing baselines. These tests intentionally avoid
 * starting the live strategy runtime; they verify the stable command boundary
 * only.
 */

#include <strategy/Strategy.h>

#include "doctest/doctest.h"

using namespace hayaku;

namespace {

class CaptureStrategy final : public Strategy {
 public:
  CaptureStrategy() : Strategy("CaptureStrategy") {}

  TradeRecord buy(const Stock&, price_t price, double num, double stoploss,
                  double goalPrice, OrderOrigin origin,
                  const string& remark) override {
    ++buyCount;
    lastPrice = price;
    lastNum = num;
    lastStoploss = stoploss;
    lastGoalPrice = goalPrice;
    lastOrigin = origin;
    lastRemark = remark;
    return TradeRecord();
  }

  TradeRecord sell(const Stock&, price_t price, double num, price_t stoploss,
                   price_t goalPrice, OrderOrigin origin,
                   const string& remark) override {
    ++sellCount;
    lastPrice = price;
    lastNum = num;
    lastStoploss = stoploss;
    lastGoalPrice = goalPrice;
    lastOrigin = origin;
    lastRemark = remark;
    return TradeRecord();
  }

  size_t buyCount{0};
  size_t sellCount{0};
  price_t lastPrice{Null<price_t>()};
  double lastNum{0.0};
  price_t lastStoploss{Null<price_t>()};
  price_t lastGoalPrice{Null<price_t>()};
  OrderOrigin lastOrigin{OrderOrigin::UNSPECIFIED};
  string lastRemark;
};

Stock makeTestStock() {
  return Stock("TS", "000001", "Strategy order test", 0, true,
               Datetime(20000101), Null<Datetime>(), 0.01, 0.01, 2, 100, 1000);
}

}  // namespace

/**
 * @defgroup test_Strategy test_Strategy
 * @ingroup test_hayaku_strategy_suite
 * @{
 */

TEST_CASE("test_Strategy_order_routes_buy") {
  CaptureStrategy strategy;
  Stock stock = makeTestStock();

  /** @arg A positive quantity routes to buy with the public order metadata. */
  strategy.order(stock, 200.0, "buy-route");
  CHECK_EQ(strategy.buyCount, 1);
  CHECK_EQ(strategy.sellCount, 0);
  CHECK_EQ(strategy.lastPrice, 0.0);
  CHECK_EQ(strategy.lastNum, 200.0);
  CHECK_EQ(strategy.lastStoploss, 0.0);
  CHECK_EQ(strategy.lastGoalPrice, 0.0);
  CHECK_EQ(strategy.lastOrigin, OrderOrigin::SIGNAL);
  CHECK_EQ(strategy.lastRemark, "buy-route");
}

TEST_CASE("test_Strategy_order_captures_current_negative_quantity_behavior") {
  CaptureStrategy strategy;
  Stock stock = makeTestStock();

  /**
   * @arg Record the current behavior: even an exact negative lot is converted
   * to the sell-all sentinel. This is a known defect candidate, not the desired
   * future contract.
   */
  strategy.order(stock, -200.0, "sell-route");
  CHECK_EQ(strategy.buyCount, 0);
  CHECK_EQ(strategy.sellCount, 1);
  CHECK_EQ(strategy.lastPrice, 0.0);
  CHECK_EQ(strategy.lastNum, MAX_DOUBLE);
  CHECK_EQ(strategy.lastOrigin, OrderOrigin::SIGNAL);
  CHECK_EQ(strategy.lastRemark, "sell-route");
}

TEST_CASE("test_Strategy_order_rejects_non_buy_quantity") {
  CaptureStrategy strategy;
  Stock stock = makeTestStock();

  /** @arg Zero does not reach either execution method. */
  strategy.order(stock, 0.0);
  CHECK_EQ(strategy.buyCount, 0);
  CHECK_EQ(strategy.sellCount, 0);

  /** @arg A positive quantity below the minimum trade unit does not reach buy.
   */
  strategy.order(stock, 99.0);
  CHECK_EQ(strategy.buyCount, 0);
  CHECK_EQ(strategy.sellCount, 0);
}

TEST_CASE("test_Strategy_order_sell_all_sentinel") {
  CaptureStrategy strategy;
  Stock stock = makeTestStock();

  /** @arg The negative MAX_DOUBLE sentinel is normalized to a sell-all request.
   */
  strategy.order(stock, -MAX_DOUBLE, "sell-all");
  CHECK_EQ(strategy.buyCount, 0);
  CHECK_EQ(strategy.sellCount, 1);
  CHECK_EQ(strategy.lastNum, MAX_DOUBLE);
  CHECK_EQ(strategy.lastRemark, "sell-all");
}

/** @} */
