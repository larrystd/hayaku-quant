/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-06
 *      Author: fasiondog
 */

#include "data/KDataPrivatedBufferImp.h"
#include "data/internal/DataRuntime.h"
#include "interface/plugins.h"
#include "hkuextra.h"

namespace hku {

void HKU_API registerExtraKType(const string& ktype, const string& basetype, int32_t minutes,
                                std::function<Datetime(const Datetime&)> getPhaseEnd) {
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    HKU_ERROR_IF_RETURN(!plugin, void(), htr("Can't find {} plugin!", HKU_PLUGIN_HKU_EXTRA));
    plugin->registerKTypeExtra(ktype, basetype, minutes, getPhaseEnd);
}

void HKU_API registerExtraKType(const string& ktype, const string& basetype, int32_t nbars) {
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    HKU_ERROR_IF_RETURN(!plugin, void(), htr("Can't find {} plugin!", HKU_PLUGIN_HKU_EXTRA));
    plugin->registerKTypeExtra(ktype, basetype, nbars, std::function<Datetime(const Datetime&)>());
}

void HKU_API releaseExtraKType() {
    // Python invokes this during interpreter shutdown. Do not recreate an already released data
    // runtime merely to discover that no extra-K plugin needs cleanup.
    auto* runtime = getDataRuntimeIfExists();
    HKU_IF_RETURN(!runtime || runtime->thread_id() == std::thread::id(), void());
    auto* plugin = runtime->getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA, false);
    HKU_IF_RETURN(!plugin, void());
    plugin->releaseKExtra();
}

void HKU_API enableKDataCache(bool enable) {
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    HKU_IF_RETURN(!plugin, void());
    plugin->enableKDataCache(enable);
}

bool isExtraKType(const string& ktype) {
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    HKU_IF_RETURN(!plugin, false);
    return plugin->isExtraKType(ktype);
}

int32_t getKTypeExtraMinutes(const string& ktype) {
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    HKU_ERROR_IF_RETURN(!plugin, 0, htr("Can't find {} plugin!", HKU_PLUGIN_HKU_EXTRA));
    return plugin->getKTypeExtraMinutes(ktype);
}

std::vector<string> getExtraKTypeList() {
    std::vector<string> ret;
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    HKU_IF_RETURN(!plugin, ret);
    ret = plugin->getExtraKTypeList();
    return ret;
}

KRecordList getExtraKRecordList(const Stock& stk, const KQuery& query) {
    KRecordList ret;
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    HKU_IF_RETURN(!plugin, ret);
    ret = plugin->getExtraKRecordList(stk, query);
    ret.shrink_to_fit();
    return ret;
}

size_t getStockExtraCount(const Stock& stk, const string& ktype) {
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    HKU_IF_RETURN(!plugin, 0);
    return plugin->getStockExtraCount(stk, ktype);
}

KDataImpPtr getKDataImp(const Stock& stk, const KQuery& query) {
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    HKU_IF_RETURN(!plugin, make_shared<KDataPrivatedBufferImp>(stk, query));
    return plugin->getKDataImp(stk, query);
}

bool getStockExtraIndexRange(const Stock& stk, const KQuery& query, size_t& out_start,
                             size_t& out_end) {
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    HKU_IF_RETURN(!plugin, false);
    return plugin->getStockExtraIndexRange(stk, query, out_start, out_end);
}

bool canLazyLoad(const KQuery::KType& ktype) {
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    HKU_IF_RETURN(!plugin, false);
    return plugin->canLazyLoad(ktype);
}

}  // namespace hku
