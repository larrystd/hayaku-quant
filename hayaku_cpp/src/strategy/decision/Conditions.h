#pragma once

/*
 * build_in.h
 *
 *  Created on: 2013-5-5
 *      Author: fasiondog
 */




/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-21
 *      Author: fasiondog
 */


#include "operators/Indicator.h"
#include "strategy/decision/ConditionBase.h"

namespace hayaku {

/**
 * System valid condition of the boolean signal
 * @param ind the boolean type indicator; a value > 0 at the corresponding position means the system
 *            is valid, otherwise it is invalid
 * @return
 */
CNPtr HAYAKU_API CN_Bool(const Indicator& ind);

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-02-16
 *      Author: fasiondog
 */



namespace hayaku {

/**
 * The AND of two system valid conditions, equivalent to the intersection of the two
 * @param cond1 system valid condition 1
 * @param cond2 system valid condition 2
 * @return the AndCondition instance pointer
 */
HAYAKU_API ConditionPtr operator&(const ConditionPtr& cond1, const ConditionPtr& cond2);

/**
 * The OR of two system valid conditions, equivalent to the union of the two
 * @param cond1 system valid condition 1
 * @param cond2 system valid condition 2
 * @return the OrCondition instance pointer
 */
HAYAKU_API ConditionPtr operator|(const ConditionPtr& cond1, const ConditionPtr& cond2);

HAYAKU_API ConditionPtr operator+(const ConditionPtr& cond1, const ConditionPtr& cond2);
HAYAKU_API ConditionPtr operator-(const ConditionPtr& cond1, const ConditionPtr& cond2);
HAYAKU_API ConditionPtr operator*(const ConditionPtr& cond1, const ConditionPtr& cond2);
HAYAKU_API ConditionPtr operator/(const ConditionPtr& cond1, const ConditionPtr& cond2);

}  // namespace hayaku

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */


#include "ConditionBase.h"

namespace hayaku {

/**
 * A CN whose validity can only be added manually, used for the testing or other special purposes
 * @return CNPtr
 */
CNPtr HAYAKU_API CN_Manual();

}  // namespace hayaku

/*
 * CN_OPLine.h
 *
 *  Created on: 2016-5-10
 *      Author: Administrator
 */



namespace hayaku {

/**
 * It always trades with the minimum trade quantity of the stock and calculates the op value of the
 * equity curve; the system is valid when the equity curve is higher than op, otherwise it is
 * invalid.
 * @param op
 * @return
 */
CNPtr HAYAKU_API CN_OPLine(const Indicator& op);

} /* namespace hayaku */
