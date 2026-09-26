#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-19
 *      Author: fasiondog
 */


#include "data/Stock.h"
#include "application/plugins/PluginBase.h"

namespace hayaku {

class CheckDataPluginInterface : public PluginBase {
public:
    static constexpr uint32_t PLUGIN_INTERFACE_VERSION = 1;
    CheckDataPluginInterface() = default;
    virtual ~CheckDataPluginInterface() = default;

    virtual std::pair<std::string, vector<std::string>> checkData(
      const StockList& stock_list, const Datetime& start_date, const Datetime& end_date,
      const KQuery::KType& check_ktype) = 0;
};

}  // namespace hayaku
