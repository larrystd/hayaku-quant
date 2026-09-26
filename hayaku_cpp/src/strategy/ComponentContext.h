#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Internal read-only view and compatibility binder for strategy components.
 */


#include "data/KData.h"

namespace hayaku {

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
}  // namespace hayaku
