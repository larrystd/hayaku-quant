#pragma once

/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-25
 *      Author: hayaku
 */

#include "data/KQuery.h"
#include "execution/TradeRecord.h"

namespace hayaku {

/**
 * Value snapshot returned by StrategyEngine.
 *
 * It stays stable if the strategy runtime is reset or run again.
 */
class BacktestResult {
 public:
  BacktestResult(Stock stock, KQuery query, TradeRecordList trades);

  [[nodiscard]] const Stock& stock() const noexcept;
  [[nodiscard]] const KQuery& query() const noexcept;
  [[nodiscard]] const TradeRecordList& trades() const noexcept;
  [[nodiscard]] size_t tradeCount() const noexcept;
  [[nodiscard]] bool empty() const noexcept;

 private:
  Stock stock_;
  KQuery query_;
  TradeRecordList trades_;
};

inline const Stock& BacktestResult::stock() const noexcept { return stock_; }

inline const KQuery& BacktestResult::query() const noexcept { return query_; }

inline const TradeRecordList& BacktestResult::trades() const noexcept {
  return trades_;
}

inline size_t BacktestResult::tradeCount() const noexcept {
  return trades_.size();
}

inline bool BacktestResult::empty() const noexcept { return trades_.empty(); }

}  // namespace hayaku
