/*
 * FixedCapitalMoneyManager.cpp
 *
 *  Created on: 2016-5-3
 *      Author: Administrator
 */

#include "FixedCapitalMoneyManager.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::FixedCapitalMoneyManager)
#endif

namespace hayaku {

FixedCapitalMoneyManager::FixedCapitalMoneyManager()
    : MoneyManagerBase("MM_FixedCapital") {
  setParam<double>("capital", 10000.00);
}

FixedCapitalMoneyManager::~FixedCapitalMoneyManager() {}

void FixedCapitalMoneyManager::_checkParam(const string& name) const {
  if ("capital" == name) {
    double capital = getParam<double>("capital");
    HAYAKU_ASSERT(capital > 0.0);
  }
}

double FixedCapitalMoneyManager ::_getBuyNumber(const Datetime& datetime,
                                                const Stock& stock,
                                                price_t price, price_t risk,
                                                OrderOrigin origin) {
  double capital = getParam<double>("capital");
  return account_->cash(datetime, query_.kType()) / capital;
}

MoneyManagerPtr MM_FixedCapital(double capital) {
  MoneyManagerPtr p = make_shared<FixedCapitalMoneyManager>();
  p->setParam<double>("capital", capital);
  return p;
}

} /* namespace hayaku */
