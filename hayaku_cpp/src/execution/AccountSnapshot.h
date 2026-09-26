#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Read-only account value object for ExecutionEngine.
 */

#include <utility>

#include "FundsRecord.h"
#include "PositionRecord.h"

namespace hayaku {

class AccountSnapshot {
 public:
  AccountSnapshot(FundsRecord funds, PositionRecordList positions,
                  PositionRecordList shortPositions)
      : funds_(std::move(funds)),
        positions_(std::move(positions)),
        short_positions_(std::move(shortPositions)) {}

  [[nodiscard]] const FundsRecord& funds() const noexcept { return funds_; }

  [[nodiscard]] const PositionRecordList& positions() const noexcept {
    return positions_;
  }

  [[nodiscard]] const PositionRecordList& shortPositions() const noexcept {
    return short_positions_;
  }

 private:
  FundsRecord funds_;
  PositionRecordList positions_;
  PositionRecordList short_positions_;
};

}  // namespace hayaku
