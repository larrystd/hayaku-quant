/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Internal read-only view and compatibility binder for strategy components.
 */

#pragma once
#ifndef HIKYUU_TRADE_SYS_ENGINE_INTERNAL_COMPONENTCONTEXT_H
#define HIKYUU_TRADE_SYS_ENGINE_INTERNAL_COMPONENTCONTEXT_H

#include "data/KData.h"

namespace hku {

namespace internal {

class StrategyRuntime;

class ComponentContext {
public:
    explicit ComponentContext(StrategyRuntime& runtime) noexcept;

    [[nodiscard]] const KData& kdata() const noexcept;
    [[nodiscard]] const KData& rawKData() const noexcept;
    [[nodiscard]] const Stock& stock() const noexcept;

    void prepare();
    void bind(const KData& kdata);

private:
    StrategyRuntime& m_runtime;
};

}  // namespace internal
}  // namespace hku

#endif /* HIKYUU_TRADE_SYS_ENGINE_INTERNAL_COMPONENTCONTEXT_H */
