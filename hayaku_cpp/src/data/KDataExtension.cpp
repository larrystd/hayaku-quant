/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <mutex>

#include "KDataExtension.h"
#include "KDataPrivatedBufferImp.h"

namespace hayaku {

namespace {

struct ExtensionState {
    std::mutex mutex;
    KDataExtensionResolver resolver;
};

ExtensionState& extensionState() {
    static auto* state = new ExtensionState;
    return *state;
}

}  // namespace

void setKDataExtensionResolver(KDataExtensionResolver resolver) {
    auto& state = extensionState();
    std::lock_guard<std::mutex> lock(state.mutex);
    state.resolver = std::move(resolver);
}

KDataExtension* getKDataExtension() noexcept {
    KDataExtensionResolver resolver;
    {
        auto& state = extensionState();
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

bool isExtraKType(const string& ktype) {
    auto* extension = getKDataExtension();
    return extension ? extension->isExtraKType(ktype) : false;
}

int32_t getKTypeExtraMinutes(const string& ktype) {
    auto* extension = getKDataExtension();
    return extension ? extension->getKTypeExtraMinutes(ktype) : 0;
}

std::vector<string> getExtraKTypeList() {
    auto* extension = getKDataExtension();
    return extension ? extension->getExtraKTypeList() : std::vector<string>();
}

KRecordList getExtraKRecordList(const Stock& stock, const KQuery& query) {
    auto* extension = getKDataExtension();
    if (!extension) {
        return {};
    }
    auto result = extension->getExtraKRecordList(stock, query);
    result.shrink_to_fit();
    return result;
}

size_t getStockExtraCount(const Stock& stock, const string& ktype) {
    auto* extension = getKDataExtension();
    return extension ? extension->getStockExtraCount(stock, ktype) : 0;
}

bool getStockExtraIndexRange(const Stock& stock, const KQuery& query, size_t& outStart,
                             size_t& outEnd) {
    auto* extension = getKDataExtension();
    return extension ? extension->getStockExtraIndexRange(stock, query, outStart, outEnd) : false;
}

KDataImpPtr getKDataImp(const Stock& stock, const KQuery& query) {
    auto* extension = getKDataExtension();
    return extension ? extension->getKDataImp(stock, query)
                     : make_shared<KDataPrivatedBufferImp>(stock, query);
}

bool canLazyLoad(const KQuery::KType& ktype) {
    auto* extension = getKDataExtension();
    return extension ? extension->canLazyLoad(ktype) : false;
}

}  // namespace hayaku
