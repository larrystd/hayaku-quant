#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <utility>

#include "AccountId.h"
#include "FundsRecord.h"
#include "PositionRecord.h"

namespace hayaku {

/** Value-owned, read-only view of account state. */
class HAYAKU_API AccountView {
 public:
  AccountView(AccountId id, Datetime initDatetime, Datetime lastDatetime,
              FundsRecord funds, PositionRecordList positions,
              PositionRecordList shortPositions)
      : id_(id),
        init_datetime_(initDatetime),
        last_datetime_(lastDatetime),
        funds_(std::move(funds)),
        positions_(std::move(positions)),
        short_positions_(std::move(shortPositions)) {}

  [[nodiscard]] AccountId id() const noexcept { return id_; }

  [[nodiscard]] Datetime initDatetime() const noexcept {
    return init_datetime_;
  }

  [[nodiscard]] Datetime lastDatetime() const noexcept {
    return last_datetime_;
  }

  [[nodiscard]] const FundsRecord& funds() const noexcept { return funds_; }

  [[nodiscard]] const PositionRecordList& positions() const noexcept {
    return positions_;
  }

  [[nodiscard]] const PositionRecordList& shortPositions() const noexcept {
    return short_positions_;
  }

 private:
  AccountId id_;
  Datetime init_datetime_;
  Datetime last_datetime_;
  FundsRecord funds_;
  PositionRecordList positions_;
  PositionRecordList short_positions_;
};

}  // namespace hayaku
