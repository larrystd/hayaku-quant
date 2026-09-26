/*
 * test_Portfolio.cpp
 *
 *  Created on: 2013-4-20
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <data/DataRuntime.h>
#include <strategy/portfolio/Portfolios.h>

using namespace hayaku;

/**
 * @defgroup test_Portfolio test_Portfolio
 * @ingroup test_hayaku_trade_sys_suite
 * @{
 */

/** @par Test point: the basic operations of Portfolio */
TEST_CASE("test_PF_for_base") {
    PortfolioPtr pf = PF_Simple();
    CHECK_EQ(pf->name(), "PF_Simple");

    /** @arg The clone operation */
    PFPtr pf2 = pf->clone();
    CHECK_NE(pf2.get(), pf.get());
    CHECK_EQ(pf2->name(), pf->name());
}

/** @} */
