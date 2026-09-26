#pragma once

/*
 * NoGoalProfitGoal.h
 *
 *  Created on: 2016-5-6
 *      Author: Administrator
 */


#include "ProfitGoalBase.h"

namespace hayaku {

class NoGoalProfitGoal : public ProfitGoalBase {
    PROFITGOAL_IMP(NoGoalProfitGoal)
    PROFIT_GOAL_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    NoGoalProfitGoal();
    virtual ~NoGoalProfitGoal();
};

} /* namespace hayaku */
