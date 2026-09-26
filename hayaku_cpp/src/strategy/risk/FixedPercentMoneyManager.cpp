/*
 * PercentRiskMoneyManager.cpp
 *
 *  Created on: 2015-4-4
 *      Author: fasiondog
 */

#include "FixedPercentMoneyManager.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::FixedPercentMoneyManager)
#endif

namespace hayaku {

FixedPercentMoneyManager::FixedPercentMoneyManager()
    : MoneyManagerBase("MM_FixedPercent") {
  setParam<double>("p", 0.02);  // Percentage of the total assets
}

FixedPercentMoneyManager::~FixedPercentMoneyManager() {}

void FixedPercentMoneyManager::_checkParam(const string& name) const {
  if ("p" == name) {
    double p = getParam<double>("p");
    HAYAKU_ASSERT(p > 0 && p <= 1.0);
  }
}

double FixedPercentMoneyManager ::_getBuyNumber(const Datetime& datetime,
                                                const Stock& stock,
                                                price_t price, price_t risk,
                                                OrderOrigin origin) {
  double p = getParam<double>("p");
  return account_->cash(datetime, query_.kType()) * p / risk;
}

MoneyManagerPtr HAYAKU_API MM_FixedPercent(double p) {
  MoneyManagerPtr ptr = make_shared<FixedPercentMoneyManager>();
  ptr->setParam<double>("p", p);
  return ptr;
}

} /* namespace hayaku */
