#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Strongly typed order input for ExecutionEngine.
 */

#include <cstdint>
#include <utility>

#include "OrderOrigin.h"
#include "TradeRecord.h"

namespace hayaku {

enum class OrderSide : std::uint8_t {
  BUY,
  SELL,
  SELL_SHORT,
  BUY_SHORT,
};

/**
 * Immutable input for one synchronous execution request.
 *
 * ExecutionEngine intentionally accepts the already resolved execution price.
 * Signal evaluation, slippage and market-data lookup remain outside the
 * execution boundary.
 */
class OrderRequest {
 public:
  OrderRequest(OrderSide side, Datetime datetime, Stock stock,
               price_t realPrice, double number, price_t stoploss = 0.0,
               price_t goalPrice = 0.0, price_t planPrice = 0.0,
               OrderOrigin origin = OrderOrigin::UNSPECIFIED,
               string remark = "")
      : side_(side),
        datetime_(std::move(datetime)),
        stock_(std::move(stock)),
        real_price_(realPrice),
        number_(number),
        stoploss_(stoploss),
        goal_price_(goalPrice),
        plan_price_(planPrice),
        origin_(origin),
        remark_(std::move(remark)) {}

  [[nodiscard]] OrderSide side() const noexcept { return side_; }

  [[nodiscard]] const Datetime& datetime() const noexcept { return datetime_; }

  [[nodiscard]] const Stock& stock() const noexcept { return stock_; }

  [[nodiscard]] price_t realPrice() const noexcept { return real_price_; }

  [[nodiscard]] double number() const noexcept { return number_; }

  [[nodiscard]] price_t stoploss() const noexcept { return stoploss_; }

  [[nodiscard]] price_t goalPrice() const noexcept { return goal_price_; }

  [[nodiscard]] price_t planPrice() const noexcept { return plan_price_; }

  [[nodiscard]] OrderOrigin origin() const noexcept { return origin_; }

  [[nodiscard]] const string& remark() const noexcept { return remark_; }

 private:
  OrderSide side_;
  Datetime datetime_;
  Stock stock_;
  price_t real_price_;
  double number_;
  price_t stoploss_;
  price_t goal_price_;
  price_t plan_price_;
  OrderOrigin origin_;
  string remark_;
};

}  // namespace hayaku
