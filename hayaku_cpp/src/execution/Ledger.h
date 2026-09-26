#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * The sole storage owner for mutable execution account state.
 */

#include <list>
#include <map>

#include "AccountId.h"
#include "BorrowRecord.h"
#include "LoanRecord.h"
#include "PositionRecord.h"
#include "TradeRecord.h"

namespace hayaku {

class ExecutionRuntime;

class Ledger {
 public:
  Ledger() = default;
  Ledger(const Ledger&) = default;
  Ledger& operator=(const Ledger&) = default;

 private:
  friend class ExecutionRuntime;

  using BorrowStockMap = std::map<uint64_t, BorrowRecord>;
  using PositionMap = std::map<uint64_t, PositionRecord>;

  AccountId m_accountId;
  Datetime m_initDatetime;
  price_t m_initCash{0.0};
  Datetime m_lastUpdateDatetime;

  price_t m_cash{0.0};
  price_t m_checkinCash{0.0};
  price_t m_checkoutCash{0.0};
  price_t m_checkinStock{0.0};
  price_t m_checkoutStock{0.0};
  price_t m_borrowCash{0.0};

  std::list<LoanRecord> m_loanList;
  BorrowStockMap m_borrowStock;
  TradeRecordList m_tradeList;
  PositionMap m_position;
  PositionRecordList m_positionHistory;
  PositionMap m_shortPosition;
  PositionRecordList m_shortPositionHistory;
  std::list<string> m_actions;
};

}  // namespace hayaku
