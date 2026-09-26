/*
 * StrategyWeight.cpp
 *
 *  Created on: 2018-1-29
 *      Author: fasiondog
 */

#include "StrategyWeight.h"

namespace hayaku {

std::ostream& operator<<(std::ostream& os, const StrategyWeight& item) {
  os << std::fixed;
  (void)os.precision(4);

  string name("NULL");
  string stk_name("(Stock(NULL))");
  if (item.strategy) {
    name = item.strategy->name();

    Stock stk = item.strategy->getStock();
    if (!stk.isNull()) {
      stk_name = "(Stock(" + stk.market_code() + "))";
    }
  }

  os << "StrategyWeight(strategy: " << name << stk_name
     << ", weight: " << item.weight << ")";

  os.unsetf(std::ostream::floatfield);
  (void)os.precision();
  return os;
}

StrategyWeight& StrategyWeight::operator=(StrategyWeight&& other) {
  if (this != &other) {
    strategy = std::move(other.strategy);
    weight = other.weight;
  }
  return *this;
}

} /* namespace hayaku */
