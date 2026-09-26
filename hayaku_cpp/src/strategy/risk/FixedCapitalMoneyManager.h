#pragma once

/*
 * FixedCapitalMoneyManager.h
 *
 *  Created on: 2016-5-3
 *      Author: Administrator
 */

#include "MoneyManagerBase.h"

namespace hayaku {

class FixedCapitalMoneyManager : public MoneyManagerBase {
  MONEY_MANAGER_IMP(FixedCapitalMoneyManager)
  MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  FixedCapitalMoneyManager();
  virtual ~FixedCapitalMoneyManager();
  virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */
