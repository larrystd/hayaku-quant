/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-17
 *      Author: fasiondog
 */

#pragma once

#include "misc.h"
#include "data/internal/DataRuntime.h"
#include "app/plugin/device.h"
#include "app/plugin/interface/plugins.h"

namespace hku {

vector<FundsList> HKU_API getFundsList(const vector<internal::ExecutionAccountPortPtr>& accounts,
                                       const DatetimeList& ref_dates) {
    return global_parallel_for_index(0, accounts.size(), [&](size_t i) {
        FundsList funds;
        if (accounts[i]) {
            funds.reserve(ref_dates.size());
            for (const auto& datetime : ref_dates) {
                funds.push_back(accounts[i]->getFunds(datetime, KQuery::DAY));
            }
        }
        return funds;
    });
}

vector<Performance> HKU_API getPerformanceList(
  const vector<internal::ExecutionAccountPortPtr>& accounts, const Datetime& datetime,
  const KQuery::KType& ktype, bool ext) {
    return global_parallel_for_index(0, accounts.size(), [&, datetime, ktype, ext](size_t i) {
        Performance perf;
        if (accounts[i]) {
            if (ext && isValidLicense()) {
                auto* plugin = getDataRuntime().getPlugin<TMReportPluginInterface>(
                  HKU_PLUGIN_TMREPORT);
                if (plugin) {
                    perf = plugin->getExtPerformance(accounts[i], datetime, ktype);
                } else {
                    perf.statistics(accounts[i], datetime);
                }
            } else {
                perf.statistics(accounts[i], datetime);
            }
        }
        return perf;
    });
}

}  // namespace hku
