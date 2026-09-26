#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Explicit synchronous execution result.
 */

#include <cstdint>
#include <utility>

#include "TradeRecord.h"

namespace hayaku {

enum class ExecutionStatus : std::uint8_t {
  FILLED,
  REJECTED,
};

class ExecutionReport {
 public:
  explicit ExecutionReport(TradeRecord record)
      : status_(record.isNull() ? ExecutionStatus::REJECTED
                                : ExecutionStatus::FILLED),
        record_(std::move(record)) {}

  [[nodiscard]] ExecutionStatus status() const noexcept { return status_; }

  [[nodiscard]] bool filled() const noexcept {
    return status_ == ExecutionStatus::FILLED;
  }

  [[nodiscard]] bool rejected() const noexcept {
    return status_ == ExecutionStatus::REJECTED;
  }

  [[nodiscard]] const TradeRecord& trade() const noexcept { return record_; }

 private:
  ExecutionStatus status_;
  TradeRecord record_;
};

}  // namespace hayaku
