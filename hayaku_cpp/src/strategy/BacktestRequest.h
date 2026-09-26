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
  KData kdata_;
  bool reset_;
  bool reset_all_;
};

inline BacktestRequest::BacktestRequest(const KData& kdata, bool reset,
                                        bool resetAll)
    : kdata_(kdata), reset_(reset), reset_all_(resetAll) {}

inline const KData& BacktestRequest::kdata() const noexcept { return kdata_; }

inline bool BacktestRequest::reset() const noexcept { return reset_; }

inline bool BacktestRequest::resetAll() const noexcept { return reset_all_; }

}  // namespace hayaku
