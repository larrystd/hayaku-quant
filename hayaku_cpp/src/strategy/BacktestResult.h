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
class HAYAKU_API BacktestResult {
 public:
  BacktestResult(Stock stock, KQuery query, TradeRecordList trades);

  [[nodiscard]] const Stock& stock() const noexcept;
  [[nodiscard]] const KQuery& query() const noexcept;
  [[nodiscard]] const TradeRecordList& trades() const noexcept;
  [[nodiscard]] size_t tradeCount() const noexcept;
  [[nodiscard]] bool empty() const noexcept;

 private:
  Stock m_stock;
  KQuery m_query;
  TradeRecordList m_trades;
};

inline const Stock& BacktestResult::stock() const noexcept { return m_stock; }

inline const KQuery& BacktestResult::query() const noexcept { return m_query; }

inline const TradeRecordList& BacktestResult::trades() const noexcept {
  return m_trades;
}

inline size_t BacktestResult::tradeCount() const noexcept {
  return m_trades.size();
}

inline bool BacktestResult::empty() const noexcept { return m_trades.empty(); }

}  // namespace hayaku
