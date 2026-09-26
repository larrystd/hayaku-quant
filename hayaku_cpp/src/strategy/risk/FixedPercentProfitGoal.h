#pragma once

/*
 * FixedPercentProfitGoal.h
 *
 *  Created on: 2016-5-6
 *      Author: Administrator
 */

#include "ProfitGoalBase.h"

namespace hayaku {

class FixedPercentProfitGoal : public ProfitGoalBase {
  PROFITGOAL_IMP(FixedPercentProfitGoal)
  PROFIT_GOAL_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  FixedPercentProfitGoal();
  virtual ~FixedPercentProfitGoal();
  virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */
