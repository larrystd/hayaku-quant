#pragma once

/*
 * FixedUnitsMoneyManager.h
 *
 *  Created on: 2016-5-3
 *      Author: Administrator
 */

#include "MoneyManagerBase.h"

namespace hayaku {

class FixedUnitsMoneyManager : public MoneyManagerBase {
  MONEY_MANAGER_IMP(FixedUnitsMoneyManager)
  MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  FixedUnitsMoneyManager();
  virtual ~FixedUnitsMoneyManager();
  virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */
