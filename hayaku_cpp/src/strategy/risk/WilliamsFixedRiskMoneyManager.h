#pragma once

/*
 * WilliamsFixedRiskMoneyManager.h
 *
 *  Created on: 2016-5-3
 *      Author: Administrator
 */

#include "MoneyManagerBase.h"

namespace hayaku {

class WilliamsFixedRiskMoneyManager : public MoneyManagerBase {
  MONEY_MANAGER_IMP(WilliamsFixedRiskMoneyManager)
  MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  WilliamsFixedRiskMoneyManager();
  virtual ~WilliamsFixedRiskMoneyManager();
  virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */
