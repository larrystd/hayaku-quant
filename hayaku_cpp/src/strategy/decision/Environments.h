#pragma once

/*
 * build_in.h
 *
 *  Created on: 2013-5-5
 *      Author: fasiondog
 */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-02-03
 *      Author: fasiondog
 */

#include "operators/Indicator.h"
#include "strategy/decision/EnvironmentBase.h"

namespace hayaku {

/**
 * Market environment of the boolean signal
 * @param ind the boolean type indicator; a value > 0 at the corresponding
 * position means the market is valid, otherwise it is invalid
 * @param market the given market, used to get the corresponding trading
 * calendar
 * @return
 */
EVPtr HAYAKU_API EV_Bool(const Indicator& ind, const string& market = "SH");

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * The AND of two market environments, equivalent to the intersection of the two
 * @param ev1 market environment 1
 * @param ev2 market environment 2
 * @return the AndCondition instance pointer
 */
HAYAKU_API EnvironmentPtr operator&(const EnvironmentPtr& ev1,
                                    const EnvironmentPtr& ev2);

/**
 * The OR of two market environments, equivalent to the union of the two
 * @param ev1 market environment 1
 * @param ev2 market environment 2
 * @return the OrCondition instance pointer
 */
HAYAKU_API EnvironmentPtr operator|(const EnvironmentPtr& ev1,
                                    const EnvironmentPtr& ev2);

HAYAKU_API EnvironmentPtr operator+(const EnvironmentPtr& ev1,
                                    const EnvironmentPtr& ev2);
HAYAKU_API EnvironmentPtr operator-(const EnvironmentPtr& ev1,
                                    const EnvironmentPtr& ev2);
HAYAKU_API EnvironmentPtr operator*(const EnvironmentPtr& ev1,
                                    const EnvironmentPtr& ev2);
HAYAKU_API EnvironmentPtr operator/(const EnvironmentPtr& ev1,
                                    const EnvironmentPtr& ev2);

}  // namespace hayaku

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#include "EnvironmentBase.h"

namespace hayaku {

/**
 * An EV whose system environment validity can only be added manually, used for
 * the testing or other purposes
 * @return EVPtr
 */
EVPtr HAYAKU_API EV_Manual();

}  // namespace hayaku

/*
 * EV_TwoLine.h
 *
 *  Created on: 2016-5-17
 *      Author: Administrator
 */

namespace hayaku {

/**
 * Fast and slow line strategy: the market is valid when the fast line of the
 * market index is greater than the slow line, otherwise it is invalid.
 * @param fast fast line indicator
 * @param slow slow line indicator
 * @param market market name, "SH" by default
 * @return
 */
EVPtr HAYAKU_API EV_TwoLine(const Indicator& fast, const Indicator& slow,
                            const string& market = "SH");

} /* namespace hayaku */
