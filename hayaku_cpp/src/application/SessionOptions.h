#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Session initialization options.
 */

#include "common/Parameter.h"
#include "data/StrategyContext.h"

namespace hayaku {

class SessionOptions {
 public:
  SessionOptions() = default;

  SessionOptions(Parameter baseInfoParam, Parameter blockParam,
                 Parameter kdataParam, Parameter preloadParam,
                 Parameter hayakuParam,
                 StrategyContext context = StrategyContext({"all"}));

  [[nodiscard]] static SessionOptions fromIni(
      const string& filename, bool ignorePreload = false,
      const StrategyContext& context = StrategyContext({"all"}));

  [[nodiscard]] const Parameter& baseInfoParam() const noexcept {
    return base_info_param_;
  }

  [[nodiscard]] const Parameter& blockParam() const noexcept {
    return block_param_;
  }

  [[nodiscard]] const Parameter& kdataParam() const noexcept {
    return kdata_param_;
  }

  [[nodiscard]] const Parameter& preloadParam() const noexcept {
    return preload_param_;
  }

  [[nodiscard]] const Parameter& hayakuParam() const noexcept {
    return hayaku_param_;
  }

  [[nodiscard]] const StrategyContext& context() const noexcept {
    return context_;
  }

 private:
  Parameter base_info_param_;
  Parameter block_param_;
  Parameter kdata_param_;
  Parameter preload_param_;
  Parameter hayaku_param_;
  StrategyContext context_{{"all"}};
};

}  // namespace hayaku
