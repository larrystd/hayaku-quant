/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <mutex>

#include "FactorStore.h"

namespace hayaku {

namespace {

struct FactorStoreState {
    std::mutex mutex;
    FactorStoreResolver resolver;
};

FactorStoreState& factorStoreState() {
    static auto* state = new FactorStoreState;
    return *state;
}

}  // namespace

void setFactorStoreResolver(FactorStoreResolver resolver) {
    auto& state = factorStoreState();
    std::lock_guard<std::mutex> lock(state.mutex);
    state.resolver = std::move(resolver);
}

FactorStore* getFactorStore() noexcept {
    FactorStoreResolver resolver;
    {
        auto& state = factorStoreState();
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

bool hasFactorStore() noexcept {
    return getFactorStore() != nullptr;
}

bool hasFactor(const string& name, const KQuery::KType& ktype) {
    auto* store = getFactorStore();
    return store ? store->hasFactor(name, ktype) : false;
}

Factor getFactor(const string& name, const KQuery::KType& ktype) {
    auto* store = getFactorStore();
    return store ? store->getFactor(name, ktype) : Factor();
}

void saveFactor(const Factor& factor, bool updateBefore) {
    auto* store = getFactorStore();
    HAYAKU_ERROR_IF_RETURN(!store, void(), "No factor store is configured");
    store->saveFactor(factor, updateBefore);
}

void saveSpecialFactorValues(const Factor& factor, const Stock& stock,
                             const DatetimeList& dates, const PriceList& values, bool replace) {
    auto* store = getFactorStore();
    HAYAKU_ERROR_IF_RETURN(!store, void(), "No factor store is configured");
    store->saveSpecialFactorValues(factor, stock, dates, values, replace);
}

void removeFactor(const string& name, const KQuery::KType& ktype) {
    auto* store = getFactorStore();
    HAYAKU_ERROR_IF_RETURN(!store, void(), "No factor store is configured");
    store->removeFactor(name, ktype);
}

FactorList getAllFactors() {
    auto* store = getFactorStore();
    return store ? store->getAllFactors() : FactorList();
}

FactorSetList getAllFactorSets() {
    auto* store = getFactorStore();
    return store ? store->getAllFactorSets() : FactorSetList();
}

void updateAllFactorsValues(const KQuery::KType& ktype) {
    auto* store = getFactorStore();
    HAYAKU_ERROR_IF_RETURN(!store, void(), "No factor store is configured");
    store->updateAllFactorsValues(ktype);
}

void saveFactorSet(const FactorSet& set) {
    auto* store = getFactorStore();
    HAYAKU_ERROR_IF_RETURN(!store, void(), "No factor store is configured");
    store->saveFactorSet(set);
}

void removeFactorSet(const string& name, const KQuery::KType& ktype) {
    auto* store = getFactorStore();
    HAYAKU_ERROR_IF_RETURN(!store, void(), "No factor store is configured");
    store->removeFactorSet(name, ktype);
}

FactorSet getFactorSet(const string& name, const KQuery::KType& ktype) {
    auto* store = getFactorStore();
    return store ? store->getFactorSet(name, ktype) : FactorSet();
}

bool isValidFactorName(const string& name) {
    auto* store = getFactorStore();
    return store ? store->isValidFactorName(name) : true;
}

IndicatorList getValues(const Factor& factor, const StockList& stocks, const KQuery& query,
                        bool align, bool fillNull, bool toValue,
                        const DatetimeList& alignDates) {
    auto* store = getFactorStore();
    return store ? store->getValues(factor, stocks, query, align, fillNull, toValue, alignDates)
                 : IndicatorList();
}

vector<IndicatorList> getValues(const FactorSet& factorSet, const StockList& stocks,
                                const KQuery& query, bool align, bool fillNull, bool toValue,
                                const DatetimeList& alignDates) {
    auto* store = getFactorStore();
    return store ? store->getValues(factorSet, stocks, query, align, fillNull, toValue, alignDates)
                 : vector<IndicatorList>();
}

}  // namespace hayaku
