/*
 * NotMoneyManager.cpp
 *
 *  Created on: 2017-5-22
 *      Author: Administrator
 */

#include "NotMoneyManager.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::NotMoneyManager)
#endif

namespace hayaku {

NotMoneyManager::NotMoneyManager() : MoneyManagerBase("MM_Nothing") {
  // Do not buy any more when there is already a position
  setParam<bool>("if_have_a_position_will_not_buy", false);
}

NotMoneyManager::~NotMoneyManager() {}

double NotMoneyManager ::_getBuyNumber(const Datetime& datetime,
                                       const Stock& stock, price_t price,
                                       price_t risk, OrderOrigin origin) {
  if (getParam<bool>("if_have_a_position_will_not_buy") &&
      account_->getHoldNumber(datetime, stock) > 0.) {
    return 0.0;
  }
  return account_->currentCash() / price;
}

MoneyManagerPtr MM_Nothing() { return make_shared<NotMoneyManager>(); }

} /* namespace hayaku */
