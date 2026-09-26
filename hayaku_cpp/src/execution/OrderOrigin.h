#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <cstdint>

#include "data/MarketTypes.h"

namespace hayaku {

/** Execution-domain order origin, independent of strategy component
 * implementation classes. */
enum class OrderOrigin : std::uint8_t {
  ENVIRONMENT = 0,
  CONDITION = 1,
  SIGNAL = 2,
  STOP_LOSS = 3,
  TAKE_PROFIT = 4,
  MONEY_MANAGEMENT = 5,
  PROFIT_GOAL = 6,
  SLIPPAGE = 7,
  ALLOCATION = 8,
  PORTFOLIO = 9,
  UNSPECIFIED = 10,
};

[[nodiscard]] string HAYAKU_API getOrderOriginName(OrderOrigin origin);
[[nodiscard]] OrderOrigin HAYAKU_API getOrderOriginEnum(const string& name);

}  // namespace hayaku
