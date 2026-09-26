#pragma once

/*
 * build_in.h
 *
 *  Created on: 2013-5-5
 *      Author: fasiondog
 */




/*
 * PG_FixedPercent.h
 *
 *  Created on: 2016-5-6
 *      Author: Administrator
 */


#include "ProfitGoalBase.h"

namespace hayaku {

/**
 * Profit goal strategy of a fixed holding days
 * @param days: the allowed holding days (counted in trading days), 5 days by default
 * @return PGPtr
 * @ingroup ProfitGoal
 */
ProfitGoalPtr HAYAKU_API PG_FixedHoldDays(int days = 5);

} /* namespace hayaku */


/*
 * PG_FixedPercent.h
 *
 *  Created on: 2016-5-6
 *      Author: Administrator
 */



namespace hayaku {

/**
 * Fixed percentage profit goal, target price = buy price * (1 + p)
 * @param p percentage
 * @return PGPtr
 */
ProfitGoalPtr HAYAKU_API PG_FixedPercent(double p = 0.2);

} /* namespace hayaku */


/*
 * PG_NoGoal.h
 *
 *  Created on: 2016-5-6
 *      Author: Administrator
 */



namespace hayaku {

/**
 * No profit goal strategy, it is usually used for the testing or the comparison
 * @return PGPtr
 */
ProfitGoalPtr HAYAKU_API PG_NoGoal();

} /* namespace hayaku */
