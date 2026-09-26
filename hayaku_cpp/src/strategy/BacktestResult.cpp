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
    : m_stock(std::move(stock)),
      m_query(std::move(query)),
      m_trades(std::move(trades)) {}

}  // namespace hayaku
