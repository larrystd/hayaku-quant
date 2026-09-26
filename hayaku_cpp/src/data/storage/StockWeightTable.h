#pragma once

/*
 * StockWeightTable.h
 *
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-8-11
 *      Author: fasiondog
 */

#include "common/database/TableMacro.h"

namespace hayaku {

class StockWeightTable {
  TABLE_BIND10(StockWeightTable, stkweight, stockid, date, countAsGift,
               countForSell, priceForSell, bonus, countOfIncreasement,
               totalCount, freeCount, suogu)

 public:
  uint64_t stockid{0};
  uint64_t date{0};
  double countAsGift{0.};
  double countForSell{0.};
  double priceForSell{0.};
  double bonus{0.};
  double countOfIncreasement{0.};
  double totalCount{0.};
  double freeCount{0.};
  double suogu{0.};
};

struct StockWeightTableView {
  TABLE_BIND10(StockWeightTableView, stkweight, market_code, date, countAsGift,
               countForSell, priceForSell, bonus, countOfIncreasement,
               totalCount, freeCount, suogu)
  string market_code;
  uint64_t date{0};
  double countAsGift{0.};
  double countForSell{0.};
  double priceForSell{0.};
  double bonus{0.};
  double countOfIncreasement{0.};
  double totalCount{0.};
  double freeCount{0.};
  double suogu{0.0};
};

}  // namespace hayaku
