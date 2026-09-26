#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Strongly typed order input for ExecutionEngine.
 */


#include <cstdint>
#include <utility>

#include "TradeRecord.h"
#include "OrderOrigin.h"

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
 * ExecutionEngine intentionally accepts the already resolved execution price. Signal evaluation,
 * slippage and market-data lookup remain outside the execution boundary.
 */
class HAYAKU_API OrderRequest {
public:
    OrderRequest(OrderSide side, Datetime datetime, Stock stock, price_t realPrice, double number,
                 price_t stoploss = 0.0, price_t goalPrice = 0.0, price_t planPrice = 0.0,
                 OrderOrigin origin = OrderOrigin::UNSPECIFIED, string remark = "")
    : m_side(side),
      m_datetime(std::move(datetime)),
      m_stock(std::move(stock)),
      m_realPrice(realPrice),
      m_number(number),
      m_stoploss(stoploss),
      m_goalPrice(goalPrice),
      m_planPrice(planPrice),
      m_origin(origin),
      m_remark(std::move(remark)) {}

    [[nodiscard]] OrderSide side() const noexcept {
        return m_side;
    }

    [[nodiscard]] const Datetime& datetime() const noexcept {
        return m_datetime;
    }

    [[nodiscard]] const Stock& stock() const noexcept {
        return m_stock;
    }

    [[nodiscard]] price_t realPrice() const noexcept {
        return m_realPrice;
    }

    [[nodiscard]] double number() const noexcept {
        return m_number;
    }

    [[nodiscard]] price_t stoploss() const noexcept {
        return m_stoploss;
    }

    [[nodiscard]] price_t goalPrice() const noexcept {
        return m_goalPrice;
    }

    [[nodiscard]] price_t planPrice() const noexcept {
        return m_planPrice;
    }

    [[nodiscard]] OrderOrigin origin() const noexcept {
        return m_origin;
    }

    [[nodiscard]] const string& remark() const noexcept {
        return m_remark;
    }

private:
    OrderSide m_side;
    Datetime m_datetime;
    Stock m_stock;
    price_t m_realPrice;
    double m_number;
    price_t m_stoploss;
    price_t m_goalPrice;
    price_t m_planPrice;
    OrderOrigin m_origin;
    string m_remark;
};

}  // namespace hayaku
