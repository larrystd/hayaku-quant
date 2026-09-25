/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Session initialization options.
 */

#pragma once
#ifndef HIKYUU_APPLICATION_SESSIONOPTIONS_H
#define HIKYUU_APPLICATION_SESSIONOPTIONS_H

#include "data/StrategyContext.h"
#include "common/Parameter.h"

namespace hku {

class HKU_API SessionOptions {
public:
    SessionOptions() = default;

    SessionOptions(Parameter baseInfoParam, Parameter blockParam, Parameter kdataParam,
                   Parameter preloadParam, Parameter hikyuuParam,
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

    [[nodiscard]] const Parameter& hikyuuParam() const noexcept {
        return m_hikyuuParam;
    }

    [[nodiscard]] const StrategyContext& context() const noexcept {
        return m_context;
    }

private:
    Parameter m_baseInfoParam;
    Parameter m_blockParam;
    Parameter m_kdataParam;
    Parameter m_preloadParam;
    Parameter m_hikyuuParam;
    StrategyContext m_context{{"all"}};
};

}  // namespace hku

#endif /* HIKYUU_APPLICATION_SESSIONOPTIONS_H */
