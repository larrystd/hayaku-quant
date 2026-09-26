#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Session initialization options.
 */

#include "common/Parameter.h"
#include "data/StrategyContext.h"

namespace hayaku {

class HAYAKU_API SessionOptions {
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
    return m_baseInfoParam;
  }

  [[nodiscard]] const Parameter& blockParam() const noexcept {
    return m_blockParam;
  }

  [[nodiscard]] const Parameter& kdataParam() const noexcept {
    return m_kdataParam;
  }

  [[nodiscard]] const Parameter& preloadParam() const noexcept {
    return m_preloadParam;
  }

  [[nodiscard]] const Parameter& hayakuParam() const noexcept {
    return m_hayakuParam;
  }

  [[nodiscard]] const StrategyContext& context() const noexcept {
    return m_context;
  }

 private:
  Parameter m_baseInfoParam;
  Parameter m_blockParam;
  Parameter m_kdataParam;
  Parameter m_preloadParam;
  Parameter m_hayakuParam;
  StrategyContext m_context{{"all"}};
};

}  // namespace hayaku
