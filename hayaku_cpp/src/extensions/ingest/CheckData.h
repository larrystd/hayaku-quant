#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-12-20
 *      Author: fasiondog
 */


#include "data/MarketTypes.h"
#include "IngestExport.h"
#include "application/plugins/CheckDataPluginInterface.h"

namespace hayaku {

std::pair<std::string, vector<std::string>> HAYAKU_INGEST_API checkData(const StockList& stock_list,
                                                              const Datetime& start_date,
                                                              const Datetime& end_date,
                                                              const KQuery::KType& check_ktype);

}
