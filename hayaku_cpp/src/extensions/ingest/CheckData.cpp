/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-12-20
 *      Author: fasiondog
 */

#include "CheckData.h"

#include "application/PluginRuntime.h"
#include "application/plugins/CheckDataPluginInterface.h"
#include "application/plugins/PluginIds.h"

namespace hayaku {

std::pair<std::string, vector<std::string>> HAYAKU_API
checkData(const StockList& stock_list, const Datetime& start_date,
          const Datetime& end_date, const KQuery::KType& check_ktype) {
  std::pair<std::string, vector<std::string>> ret;
  auto* plugin = getPlugin<CheckDataPluginInterface>(HAYAKU_PLUGIN_CHECK_DATA);
  HAYAKU_ERROR_IF_RETURN(
      !plugin, ret, htr("Can't find {} plugin!", HAYAKU_PLUGIN_CHECK_DATA));
  return plugin->checkData(stock_list, start_date, end_date, check_ktype);
}

}  // namespace hayaku
