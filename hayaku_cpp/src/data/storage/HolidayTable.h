#pragma once

/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-06
 *     Author: fasiondog
 */

#include <limits>

#include "common/database/SQLStatementBase.h"
#include "common/database/TableMacro.h"

namespace hayaku {

class HolidayTable {
  TABLE_BIND1(HolidayTable, holiday, date)

 public:
  Datetime datetime() const {
    HAYAKU_CHECK(date <= 99999999, "Invalid holiday date: {}!", date);
    return Datetime(date * 10000LL);
  }

 private:
  uint64_t date{Datetime().number()};
};

}  // namespace hayaku
