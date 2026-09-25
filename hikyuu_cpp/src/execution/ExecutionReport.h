/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Explicit synchronous execution result.
 */

#pragma once
#ifndef HIKYUU_TRADE_EXECUTIONREPORT_H
#define HIKYUU_TRADE_EXECUTIONREPORT_H

#include <cstdint>
#include <utility>

#include "TradeRecord.h"

namespace hku {

enum class ExecutionStatus : std::uint8_t {
    FILLED,
    REJECTED,
};

class HKU_API ExecutionReport {
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

}  // namespace hku

#endif /* HIKYUU_TRADE_EXECUTIONREPORT_H */
