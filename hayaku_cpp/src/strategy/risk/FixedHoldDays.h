#pragma once

/*
 * FixedHoldDays.h
 *
 *  Created on: 2018-1-20
 *      Author: fasiondog
 */

#include "ProfitGoalBase.h"

namespace hayaku {

class FixedHoldDays : public ProfitGoalBase {
  PROFITGOAL_IMP(FixedHoldDays)
  PROFIT_GOAL_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  FixedHoldDays();
  virtual ~FixedHoldDays();

  virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */
