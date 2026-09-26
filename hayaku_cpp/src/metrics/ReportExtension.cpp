/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <mutex>

#include "ReportExtension.h"

namespace hayaku {

namespace {

struct ReportExtensionState {
    std::mutex mutex;
    ReportExtensionResolver resolver;
};

ReportExtensionState& reportExtensionState() {
    static auto* state = new ReportExtensionState;
    return *state;
}

}  // namespace

void setReportExtensionResolver(ReportExtensionResolver resolver) {
    auto& state = reportExtensionState();
    std::lock_guard<std::mutex> lock(state.mutex);
    state.resolver = std::move(resolver);
}

ReportExtension* getReportExtension() noexcept {
    ReportExtensionResolver resolver;
    {
        auto& state = reportExtensionState();
        std::lock_guard<std::mutex> lock(state.mutex);
        resolver = state.resolver;
    }
    if (!resolver) {
        return nullptr;
    }
    try {
        return resolver();
    } catch (...) {
        return nullptr;
    }
}

}  // namespace hayaku
