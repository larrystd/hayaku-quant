/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */

#include "data/internal/DataRuntime.h"
#include "interface/plugins.h"
#include "backtest.h"

namespace hku {

void HKU_API backtest(const StrategyContext& context, const std::function<void(Strategy*)>& on_bar,
                      const internal::ExecutionAccountPortPtr& account,
                      const Datetime& start_date,
                      const Datetime& end_date, const KQuery::KType& ktype,
                      const string& ref_market, int mode, bool support_short, SlippagePtr slip) {
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<BackTestPluginInterface>(HKU_PLUGIN_BACKTEST);
    HKU_ERROR_IF_RETURN(!plugin, void(), htr("Can't find {} plugin!", HKU_PLUGIN_BACKTEST));
    plugin->backtest(context, on_bar, account, start_date, end_date, ktype, ref_market, mode,
                     support_short, slip);
}

void HKU_API backtest(const std::function<void(Strategy*)>& on_bar,
                      const internal::ExecutionAccountPortPtr& account,
                      const Datetime& start_date, const Datetime& end_date,
                      const KQuery::KType& ktype, const string& ref_market, int mode,
                      bool support_short, SlippagePtr slip) {
    const StrategyContext& context = getDataRuntime().getStrategyContext();
    HKU_ERROR_IF_RETURN(
      context.empty(), void(),
      htr("Unable to obtain context. hikyuu may not be initialized. Please check!"));
    backtest(context, on_bar, account, start_date, end_date, ktype, ref_market, mode, support_short,
             slip);
}

}  // namespace hku
