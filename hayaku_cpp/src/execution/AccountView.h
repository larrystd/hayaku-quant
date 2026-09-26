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
      : m_id(id),
        m_initDatetime(initDatetime),
        m_lastDatetime(lastDatetime),
        m_funds(std::move(funds)),
        m_positions(std::move(positions)),
        m_shortPositions(std::move(shortPositions)) {}

  [[nodiscard]] AccountId id() const noexcept { return m_id; }

  [[nodiscard]] Datetime initDatetime() const noexcept {
    return m_initDatetime;
  }

  [[nodiscard]] Datetime lastDatetime() const noexcept {
    return m_lastDatetime;
  }

  [[nodiscard]] const FundsRecord& funds() const noexcept { return m_funds; }

  [[nodiscard]] const PositionRecordList& positions() const noexcept {
    return m_positions;
  }

  [[nodiscard]] const PositionRecordList& shortPositions() const noexcept {
    return m_shortPositions;
  }

 private:
  AccountId m_id;
  Datetime m_initDatetime;
  Datetime m_lastDatetime;
  FundsRecord m_funds;
  PositionRecordList m_positions;
  PositionRecordList m_shortPositions;
};

}  // namespace hayaku
