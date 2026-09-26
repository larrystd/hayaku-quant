#pragma once

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-13
 *      Author: fasiondog
 */


#include "common/database/TableMacro.h"

namespace hayaku {

struct HistoryFinanceFieldTable {
    TABLE_BIND1(HistoryFinanceFieldTable, HistoryFinanceField, name)
    std::string name;
};

}  // namespace hayaku
