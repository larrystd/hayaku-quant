#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <utility>
#include <vector>

#include "AccountId.h"
#include "broker/OrderBrokerBase.h"
#include "pricing/TradeCostBase.h"
#include "pricing/TradeCosts.h"

namespace hayaku {

/** Immutable construction parameters for an execution account. */
class HAYAKU_API AccountConfig {
 public:
  explicit AccountConfig(Datetime initDatetime = Datetime(199001010000LL),
                         price_t initialCash = 100000.0,
                         TradeCostPtr costPolicy = TC_Zero(),
                         string name = "SYS", int precision = 2,
                         bool supportBorrowCash = false,
                         bool supportBorrowStock = false,
                         AccountId accountId = {},
                         std::vector<OrderBrokerPtr> brokers = {})
      : init_datetime_(initDatetime),
        initial_cash_(initialCash),
        cost_policy_(std::move(costPolicy)),
        name_(std::move(name)),
        precision_(precision),
        support_borrow_cash_(supportBorrowCash),
        support_borrow_stock_(supportBorrowStock),
        account_id_(accountId),
        brokers_(std::move(brokers)) {
    HAYAKU_CHECK(precision_ > 0,
                 "Account precision must be greater than zero");
  }

  [[nodiscard]] Datetime initDatetime() const noexcept {
    return init_datetime_;
  }

  [[nodiscard]] price_t initialCash() const noexcept { return initial_cash_; }

  [[nodiscard]] const TradeCostPtr& costPolicy() const noexcept {
    return cost_policy_;
  }

  [[nodiscard]] const string& name() const noexcept { return name_; }

  [[nodiscard]] int precision() const noexcept { return precision_; }

  [[nodiscard]] bool supportBorrowCash() const noexcept {
    return support_borrow_cash_;
  }

  [[nodiscard]] bool supportBorrowStock() const noexcept {
    return support_borrow_stock_;
  }

  [[nodiscard]] AccountId accountId() const noexcept { return account_id_; }

  [[nodiscard]] const std::vector<OrderBrokerPtr>& brokers() const noexcept {
    return brokers_;
  }

 private:
  Datetime init_datetime_;
  price_t initial_cash_;
  TradeCostPtr cost_policy_;
  string name_;
  int precision_;
  bool support_borrow_cash_;
  bool support_borrow_stock_;
  AccountId account_id_;
  std::vector<OrderBrokerPtr> brokers_;
};

}  // namespace hayaku
