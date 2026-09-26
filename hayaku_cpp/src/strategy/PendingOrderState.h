#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Internal delayed-order state owned by StrategyRuntime.
 */

#include "data/KRecord.h"
#include "execution/TradeRecord.h"

namespace hayaku::internal {

struct PendingOrder {
  void clear() noexcept {
    valid = false;
    business = BUSINESS_INVALID;
    datetime = Datetime();
    stoploss = 0.0;
    goal = 0.0;
    number = 0.0;
    origin = OrderOrigin::UNSPECIFIED;
    remark.clear();
    count = 0;
    krecord = KRecord();
  }

  bool valid{false};
  BUSINESS business{BUSINESS_INVALID};
  Datetime datetime;
  price_t stoploss{0.0};
  price_t goal{0.0};
  double number{0.0};
  OrderOrigin origin{OrderOrigin::UNSPECIFIED};
  string remark;
  int count{0};
  KRecord krecord;
};

class PendingOrderState {
 public:
  [[nodiscard]] PendingOrder& buy() noexcept { return buy_; }

  [[nodiscard]] const PendingOrder& buy() const noexcept { return buy_; }

  [[nodiscard]] PendingOrder& sell() noexcept { return sell_; }

  [[nodiscard]] const PendingOrder& sell() const noexcept { return sell_; }

  [[nodiscard]] PendingOrder& sellShort() noexcept { return sell_short_; }

  [[nodiscard]] const PendingOrder& sellShort() const noexcept {
    return sell_short_;
  }

  [[nodiscard]] PendingOrder& buyShort() noexcept { return buy_short_; }

  [[nodiscard]] const PendingOrder& buyShort() const noexcept {
    return buy_short_;
  }

  void clear() noexcept {
    buy_.clear();
    sell_.clear();
    sell_short_.clear();
    buy_short_.clear();
  }

 private:
  PendingOrder buy_;
  PendingOrder sell_;
  PendingOrder sell_short_;
  PendingOrder buy_short_;
};

}  // namespace hayaku::internal
