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
      : m_initDatetime(initDatetime),
        m_initialCash(initialCash),
        m_costPolicy(std::move(costPolicy)),
        m_name(std::move(name)),
        m_precision(precision),
        m_supportBorrowCash(supportBorrowCash),
        m_supportBorrowStock(supportBorrowStock),
        m_accountId(accountId),
        m_brokers(std::move(brokers)) {
    HAYAKU_CHECK(m_precision > 0,
                 "Account precision must be greater than zero");
  }

  [[nodiscard]] Datetime initDatetime() const noexcept {
    return m_initDatetime;
  }

  [[nodiscard]] price_t initialCash() const noexcept { return m_initialCash; }

  [[nodiscard]] const TradeCostPtr& costPolicy() const noexcept {
    return m_costPolicy;
  }

  [[nodiscard]] const string& name() const noexcept { return m_name; }

  [[nodiscard]] int precision() const noexcept { return m_precision; }

  [[nodiscard]] bool supportBorrowCash() const noexcept {
    return m_supportBorrowCash;
  }

  [[nodiscard]] bool supportBorrowStock() const noexcept {
    return m_supportBorrowStock;
  }

  [[nodiscard]] AccountId accountId() const noexcept { return m_accountId; }

  [[nodiscard]] const std::vector<OrderBrokerPtr>& brokers() const noexcept {
    return m_brokers;
  }

 private:
  Datetime m_initDatetime;
  price_t m_initialCash;
  TradeCostPtr m_costPolicy;
  string m_name;
  int m_precision;
  bool m_supportBorrowCash;
  bool m_supportBorrowStock;
  AccountId m_accountId;
  std::vector<OrderBrokerPtr> m_brokers;
};

}  // namespace hayaku
