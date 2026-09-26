#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Immutable inputs for one strategy backtest run.
 */

#include "data/KData.h"

namespace hayaku {

class HAYAKU_API BacktestRequest {
 public:
  explicit BacktestRequest(const KData& kdata, bool reset = true,
                           bool resetAll = false);

  [[nodiscard]] const KData& kdata() const noexcept;
  [[nodiscard]] bool reset() const noexcept;
  [[nodiscard]] bool resetAll() const noexcept;

 private:
  KData m_kdata;
  bool m_reset;
  bool m_resetAll;
};

inline BacktestRequest::BacktestRequest(const KData& kdata, bool reset,
                                        bool resetAll)
    : m_kdata(kdata), m_reset(reset), m_resetAll(resetAll) {}

inline const KData& BacktestRequest::kdata() const noexcept { return m_kdata; }

inline bool BacktestRequest::reset() const noexcept { return m_reset; }

inline bool BacktestRequest::resetAll() const noexcept { return m_resetAll; }

}  // namespace hayaku
