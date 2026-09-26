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

class HAYAKU_API ExecutionReport {
public:
    explicit ExecutionReport(TradeRecord record)
    : m_status(record.isNull() ? ExecutionStatus::REJECTED : ExecutionStatus::FILLED),
      m_record(std::move(record)) {}

    [[nodiscard]] ExecutionStatus status() const noexcept {
        return m_status;
    }

    [[nodiscard]] bool filled() const noexcept {
        return m_status == ExecutionStatus::FILLED;
    }

    [[nodiscard]] bool rejected() const noexcept {
        return m_status == ExecutionStatus::REJECTED;
    }

    [[nodiscard]] const TradeRecord& trade() const noexcept {
        return m_record;
    }

private:
    ExecutionStatus m_status;
    TradeRecord m_record;
};

}  // namespace hayaku
