#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Narrow account contract shared by execution and strategy runtimes.
 */

#include "AccountId.h"
#include "ExecutionReport.h"
#include "FundsRecord.h"
#include "OrderRequest.h"
#include "PositionRecord.h"

namespace hayaku::internal {

/**
 * Core strategy-facing account contract.
 *
 * Portfolio capital transfer, child-account creation and broker synchronization
 * intentionally do not belong here. Those orchestration capabilities use a
 * separate portfolio port.
 */
class ExecutionAccountPort {
 public:
  virtual ~ExecutionAccountPort() = default;

  [[nodiscard]] virtual AccountId accountId() const noexcept = 0;
  virtual void reset() = 0;
  [[nodiscard]] virtual Datetime initDatetime() const = 0;
  [[nodiscard]] virtual Datetime firstDatetime() const = 0;
  [[nodiscard]] virtual Datetime lastDatetime() const = 0;
  virtual void updateWithWeight(const Datetime& datetime) = 0;

  [[nodiscard]] virtual ExecutionReport submit(const OrderRequest& request) = 0;
  [[nodiscard]] virtual PositionRecord getPosition(const Datetime& datetime,
                                                   const Stock& stock) = 0;
  [[nodiscard]] virtual PositionRecord getShortPosition(
      const Stock& stock) const = 0;
  [[nodiscard]] virtual bool have(const Stock& stock) const = 0;
  [[nodiscard]] virtual bool haveShort(const Stock& stock) const = 0;
  [[nodiscard]] virtual double getHoldNumber(const Datetime& datetime,
                                             const Stock& stock) = 0;
  [[nodiscard]] virtual double getShortHoldNumber(const Datetime& datetime,
                                                  const Stock& stock) = 0;

  [[nodiscard]] virtual FundsRecord getFunds(
      const Datetime& datetime, KQuery::KType ktype = KQuery::DAY) = 0;
  [[nodiscard]] virtual PriceList getProfitCurve(
      const DatetimeList& dates, KQuery::KType ktype = KQuery::DAY) = 0;
  [[nodiscard]] virtual price_t cash(const Datetime& datetime,
                                     KQuery::KType ktype = KQuery::DAY) = 0;
  [[nodiscard]] virtual price_t currentCash() const = 0;
  [[nodiscard]] virtual price_t initCash() const = 0;
  virtual bool checkin(const Datetime& datetime, price_t cash) = 0;
  [[nodiscard]] virtual size_t getStockNumber() const = 0;
  [[nodiscard]] virtual int precision() const = 0;
  [[nodiscard]] virtual CostRecord getBuyCost(const Datetime& datetime,
                                              const Stock& stock, price_t price,
                                              double number) const = 0;
  [[nodiscard]] virtual bool supportsBorrowCash() const = 0;
  [[nodiscard]] virtual bool supportsBorrowStock() const = 0;

  /** Read-only history required by result analysis; never mutates the ledger.
   */
  [[nodiscard]] virtual TradeRecordList getTradeList() const = 0;
  [[nodiscard]] virtual PositionRecordList getPositionList() const = 0;
  [[nodiscard]] virtual PositionRecordList getHistoryPositionList() const = 0;
};

using ExecutionAccountPortPtr = std::shared_ptr<ExecutionAccountPort>;

}  // namespace hayaku::internal
