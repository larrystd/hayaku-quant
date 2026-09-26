/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-19
 *      Author: fasiondog
 */

#include "application/PluginRuntime.h"
#include "operators/SeriesOperators.h"
#include "PluginIds.h"
#include "ExtendIndicatorsPluginInterface.h"
#include "ExtendIndicatorsPlugin.h"

namespace hayaku {

static Indicator getExtIndicator(const string& name, const Parameter& params) {
    Indicator ret;
    auto* plugin = getPlugin<ExtendIndicatorsPluginInterface>(HAYAKU_PLUGIN_EXTEND_INDICATOR);
    HAYAKU_ERROR_IF_RETURN(!plugin, ret, htr("Can't find {} plugin!", HAYAKU_PLUGIN_EXTEND_INDICATOR));
    return plugin->getIndicator(name, params);
}

static Indicator getExtIndicator(const string& name, const Indicator& ref_ind,
                                 const Parameter& params) {
    Indicator ret;
    auto* plugin = getPlugin<ExtendIndicatorsPluginInterface>(HAYAKU_PLUGIN_EXTEND_INDICATOR);
    HAYAKU_ERROR_IF_RETURN(!plugin, ret, htr("Can't find {} plugin!", HAYAKU_PLUGIN_EXTEND_INDICATOR));
    return plugin->getIndicator(name, ref_ind, params);
}

Indicator HAYAKU_API FIXED_START_INDEX(int start_index) {
    Parameter params;
    params.set<int>("index", start_index);
    return getExtIndicator("FIXED_START_INDEX", params);
}

Indicator HAYAKU_API FIXED_START_INDEX(const Indicator& ind, int start_index) {
    return FIXED_START_INDEX(start_index)(ind);
}

Indicator HAYAKU_API FIXED_START_DATE(const Datetime& start_date) {
    Parameter params;
    params.set<Datetime>("start_date", start_date);
    return getExtIndicator("FIXED_START_DATE", params);
}

Indicator HAYAKU_API FIXED_START_DATE(const Indicator& ind, const Datetime& start_date) {
    return FIXED_START_DATE(start_date)(ind);
}

Indicator HAYAKU_API WITHKTYPE(const KQuery::KType& ktype, bool fill_null) {
    Parameter params;
    params.set<string>("ktype", ktype);
    params.set<bool>("fill_null", fill_null);
    return getExtIndicator("WITHKTYPE", params);
}

Indicator HAYAKU_API WITHKTYPE(const Indicator& ind, const KQuery::KType& ktype, bool fill_null) {
    Parameter params;
    params.set<string>("ktype", ktype);
    params.set<bool>("fill_null", fill_null);
    return getExtIndicator("WITHKTYPE", ind, params);
}

Indicator HAYAKU_API RANK(const Block& block, int mode, bool fill_null, const string& market) {
    Parameter params;
    params.set<Block>("block", block);
    params.set<int>("mode", mode);
    params.set<bool>("fill_null", fill_null);
    params.set<string>("market", market);
    return getExtIndicator("RANK", params);
}

Indicator HAYAKU_API RANK(const Block& block, const Indicator& ref_ind, int mode, bool fill_null,
                       const string& market) {
    return RANK(block, mode, fill_null, market)(ref_ind);
}

AGG_FUNC_IMP(AGG_MEAN)
AGG_FUNC_IMP(AGG_COUNT)
AGG_FUNC_IMP(AGG_SUM)
AGG_FUNC_IMP(AGG_MAX)
AGG_FUNC_IMP(AGG_MIN)
AGG_FUNC_IMP(AGG_MAD)
AGG_FUNC_IMP(AGG_MEDIAN)
AGG_FUNC_IMP(AGG_PROD)

Indicator HAYAKU_API AGG_SAMPLE(const Indicator& ind, const string& time, const KQuery::KType& ktype,
                             bool fill_null, int unit) {
    Parameter params;
    params.set<string>("time", time);
    params.set<string>("ktype", ktype);
    params.set<bool>("fill_null", fill_null);
    params.set<int>("unit", unit);
    return getExtIndicator("AGG_SAMPLE", ind, params);
}

Indicator HAYAKU_API AGG_SAMPLE_MAX(const Indicator& ind, const string& start_time,
                                 const string& last_time, const KQuery::KType& ktype,
                                 bool fill_null, int unit) {
    Parameter params;
    params.set<string>("start_time", start_time);
    params.set<string>("last_time", last_time);
    params.set<string>("ktype", ktype);
    params.set<bool>("fill_null", fill_null);
    params.set<int>("unit", unit);
    return getExtIndicator("AGG_SAMPLE_MAX", ind, params);
}

Indicator HAYAKU_API AGG_SAMPLE_MIN(const Indicator& ind, const string& start_time,
                                 const string& last_time, const KQuery::KType& ktype,
                                 bool fill_null, int unit) {
    Parameter params;
    params.set<string>("start_time", start_time);
    params.set<string>("last_time", last_time);
    params.set<string>("ktype", ktype);
    params.set<bool>("fill_null", fill_null);
    params.set<int>("unit", unit);
    return getExtIndicator("AGG_SAMPLE_MIN", ind, params);
}

Indicator HAYAKU_API AGG_SAMPLE_MEAN(const Indicator& ind, const string& start_time,
                                  const string& last_time, const KQuery::KType& ktype,
                                  bool fill_null, int unit) {
    Parameter params;
    params.set<string>("start_time", start_time);
    params.set<string>("last_time", last_time);
    params.set<string>("ktype", ktype);
    params.set<bool>("fill_null", fill_null);
    params.set<int>("unit", unit);
    return getExtIndicator("AGG_SAMPLE_MEAN", ind, params);
}

Indicator HAYAKU_API AGG_STD(const Indicator& ind, const KQuery::KType& ktype, bool fill_null,
                          int unit, int ddof) {
    Parameter params;
    params.set<string>("ktype", ktype);
    params.set<bool>("fill_null", fill_null);
    params.set<int>("unit", unit);
    params.set<int>("ddof", ddof);
    return getExtIndicator("AGG_STD", ind, params);
}

Indicator HAYAKU_API AGG_VAR(const Indicator& ind, const KQuery::KType& ktype, bool fill_null,
                          int unit, int ddof) {
    Parameter params;
    params.set<string>("ktype", ktype);
    params.set<bool>("fill_null", fill_null);
    params.set<int>("unit", unit);
    params.set<int>("ddof", ddof);
    return getExtIndicator("AGG_VAR", ind, params);
}

Indicator HAYAKU_API AGG_QUANTILE(const Indicator& ind, const KQuery::KType& ktype, bool fill_null,
                               int unit, double quantile) {
    Parameter params;
    params.set<string>("ktype", ktype);
    params.set<bool>("fill_null", fill_null);
    params.set<int>("unit", unit);
    params.set<double>("quantile", quantile);
    return getExtIndicator("AGG_QUANTILE", ind, params);
}

Indicator HAYAKU_API AGG_VWAP(const KQuery::KType& ktype, bool fill_null, int unit) {
    Parameter params;
    params.set<string>("ktype", ktype);
    params.set<bool>("fill_null", fill_null);
    params.set<int>("unit", unit);
    return getExtIndicator("AGG_VWAP", CLOSE(), params);
}

Indicator HAYAKU_API AGG_FUNC(const Indicator& ind, agg_func_t agg_func, const KQuery::KType& ktype,
                           bool fill_null, int unit) {
    Indicator ret;
    auto* plugin = getPlugin<ExtendIndicatorsPluginInterface>(HAYAKU_PLUGIN_EXTEND_INDICATOR);
    HAYAKU_ERROR_IF_RETURN(!plugin, ret, "Can't find {} plugin!", HAYAKU_PLUGIN_EXTEND_INDICATOR);

    Parameter params;
    params.set<string>("ktype", ktype);
    params.set<bool>("fill_null", fill_null);
    params.set<int>("unit", unit);
    return plugin->getAggFuncIndicator(ind, agg_func, params);
}

GROUP_FUNC_IMP(GROUP_COUNT)
GROUP_FUNC_IMP(GROUP_SUM)
GROUP_FUNC_IMP(GROUP_MEAN)
GROUP_FUNC_IMP(GROUP_PROD)
GROUP_FUNC_IMP(GROUP_MIN)
GROUP_FUNC_IMP(GROUP_MAX)

Indicator HAYAKU_API GROUP_FUNC(const Indicator& ind, group_func_t group_func,
                             const KQuery::KType& ktype, int unit) {
    Indicator ret;
    auto* plugin = getPlugin<ExtendIndicatorsPluginInterface>(HAYAKU_PLUGIN_EXTEND_INDICATOR);
    HAYAKU_ERROR_IF_RETURN(!plugin, ret, "Can't find {} plugin!", HAYAKU_PLUGIN_EXTEND_INDICATOR);

    Parameter params;
    params.set<string>("ktype", ktype);
    params.set<int>("unit", unit);
    return plugin->getGroupFuncIndicator(ind, group_func, params);
}

}  // namespace hayaku
