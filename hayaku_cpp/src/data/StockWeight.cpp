/*
 * StockWeight.cpp
 *
 *  Created on: 2011-12-9
 *      Author: fasiondog
 */

#include "StockWeight.h"

namespace hayaku {

HAYAKU_API std::ostream& operator<<(std::ostream& os,
                                    const StockWeight& record) {
  if (Null<StockWeight>() == record) {
    os << "Weight(Null)";
    return os;
  }

  os << "Weight(" << record.datetime() << ", " << record.countAsGift() << ", "
     << record.countForSell() << ", " << record.priceForSell() << ", "
     << record.bonus() << ", " << record.increasement() << ", "
     << record.totalCount() << ", " << record.freeCount() << ", "
     << record.suogu() << ")";
  (void)os.precision(6);
  return os;
}

StockWeight::StockWeight(const Datetime& datetime)
    : datetime_(datetime),
      count_as_gift_(0.0),
      count_for_sell_(0.0),
      price_for_sell_(0.0),
      bonus_(0.0),
      increasement_(0.0),
      total_count_(0.0),
      free_count_(0.0),
      suogu_(0.0) {}

StockWeight::StockWeight(const Datetime& datetime, price_t countAsGift,
                         price_t countForSell, price_t priceForSell,
                         price_t bonus, price_t increasement,
                         price_t totalCount, price_t freeCount, price_t suogu)
    : datetime_(datetime),
      count_as_gift_(countAsGift),
      count_for_sell_(countForSell),
      price_for_sell_(priceForSell),
      bonus_(bonus),
      increasement_(increasement),
      total_count_(totalCount),
      free_count_(freeCount),
      suogu_(suogu) {}

}  // namespace hayaku
