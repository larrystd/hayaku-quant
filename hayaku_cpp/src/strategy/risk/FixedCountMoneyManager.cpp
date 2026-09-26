/*
 * FixedCountMoneyManager.cpp
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#include "FixedCountMoneyManager.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::FixedCountMoneyManager)
#endif

namespace hayaku {

FixedCountMoneyManager::FixedCountMoneyManager()
    : MoneyManagerBase("MM_FixedCount") {
  setParam<double>("n", 100);
}

FixedCountMoneyManager::~FixedCountMoneyManager() {}

void FixedCountMoneyManager::_checkParam(const string& name) const {
  if ("n" == name) {
    double n = getParam<double>("n");
    HAYAKU_ASSERT(n > 0.0);
  }
}

double FixedCountMoneyManager::_getBuyNumber(const Datetime& datetime,
                                             const Stock& stock, price_t price,
                                             price_t risk, OrderOrigin origin) {
  return getParam<double>("n");
}

double FixedCountMoneyManager::_getSellShortNumber(const Datetime& datetime,
                                                   const Stock& stock,
                                                   price_t price, price_t risk,
                                                   OrderOrigin origin) {
  return getParam<double>("n");
}

MoneyManagerPtr MM_FixedCount(double n) {
  MoneyManagerPtr p = make_shared<FixedCountMoneyManager>();
  p->setParam<double>("n", n);
  return p;
}

} /* namespace hayaku */
