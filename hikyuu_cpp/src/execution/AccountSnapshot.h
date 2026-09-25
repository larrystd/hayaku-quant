/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Read-only account value object for ExecutionEngine.
 */

#pragma once
#ifndef HIKYUU_TRADE_ACCOUNTSNAPSHOT_H
#define HIKYUU_TRADE_ACCOUNTSNAPSHOT_H

#include <utility>

#include "FundsRecord.h"
#include "PositionRecord.h"

namespace hku {

class HKU_API AccountSnapshot {
public:
    AccountSnapshot(FundsRecord funds, PositionRecordList positions,
                    PositionRecordList shortPositions)
    : m_funds(std::move(funds)),
      m_positions(std::move(positions)),
      m_shortPositions(std::move(shortPositions)) {}

    [[nodiscard]] const FundsRecord& funds() const noexcept {
        return m_funds;
    }

    [[nodiscard]] const PositionRecordList& positions() const noexcept {
        return m_positions;
    }

    [[nodiscard]] const PositionRecordList& shortPositions() const noexcept {
        return m_shortPositions;
    }

private:
    FundsRecord m_funds;
    PositionRecordList m_positions;
    PositionRecordList m_shortPositions;
};

}  // namespace hku

#endif /* HIKYUU_TRADE_ACCOUNTSNAPSHOT_H */
