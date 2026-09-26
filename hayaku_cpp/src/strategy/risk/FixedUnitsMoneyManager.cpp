/*
 * FixedUnitsMoneyManager.cpp
 *
 *  Created on: 2016-5-3
 *      Author: Administrator
 */

#include "FixedUnitsMoneyManager.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::FixedUnitsMoneyManager)
#endif

namespace hayaku {

FixedUnitsMoneyManager::FixedUnitsMoneyManager()
    : MoneyManagerBase("MM_FixedUnits") {
  setParam<int>("n", 33);
}

FixedUnitsMoneyManager::~FixedUnitsMoneyManager() {}

void FixedUnitsMoneyManager::_checkParam(const string& name) const {
  if ("n" == name) {
    int n = getParam<int>("n");
    HAYAKU_ASSERT(n > 0);
  }
}

double FixedUnitsMoneyManager ::_getBuyNumber(const Datetime& datetime,
                                              const Stock& stock, price_t price,
                                              price_t risk,
                                              OrderOrigin origin) {
  int n = getParam<int>("n");
  m_account->updateWithWeight(datetime);
  price_t fixed_risk = (m_account->currentCash() > m_account->initCash())
                           ? m_account->currentCash() / n
                           : m_account->initCash() / n;

  return fixed_risk / risk;
}

MoneyManagerPtr HAYAKU_API MM_FixedUnits(int n) {
  MoneyManagerPtr p = make_shared<FixedUnitsMoneyManager>();
  p->setParam<int>("n", n);
  return p;
}

} /* namespace hayaku */
