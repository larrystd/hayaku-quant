/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-25
 *      Author: hayaku
 */

#include "BacktestResult.h"

#include <utility>

namespace hayaku {

BacktestResult::BacktestResult(Stock stock, KQuery query,
                               TradeRecordList trades)
    : stock_(std::move(stock)),
      query_(std::move(query)),
      trades_(std::move(trades)) {}

}  // namespace hayaku
