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

  AccountId account_id_;
  Datetime init_datetime_;
  price_t init_cash_{0.0};
  Datetime last_update_datetime_;

  price_t cash_{0.0};
  price_t checkin_cash_{0.0};
  price_t checkout_cash_{0.0};
  price_t checkin_stock_{0.0};
  price_t checkout_stock_{0.0};
  price_t borrow_cash_{0.0};

  std::list<LoanRecord> loan_list_;
  BorrowStockMap borrow_stock_;
  TradeRecordList trade_list_;
  PositionMap position_;
  PositionRecordList position_history_;
  PositionMap short_position_;
  PositionRecordList short_position_history_;
  std::list<string> actions_;
};

}  // namespace hayaku
