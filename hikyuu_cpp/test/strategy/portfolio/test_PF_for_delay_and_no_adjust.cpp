/*
 * test_Portfolio.cpp
 *
 *  Created on: 2013-4-20
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <data/internal/DataRuntime.h>
#include <strategy/portfolio/crt/PF_Simple.h>
#include <strategy/selector/crt/SE_Fixed.h>
#include <strategy/allocatefunds/crt/AF_EqualWeight.h>

#include <strategy/signal/crt/SG_CrossGold.h>
#include <strategy/moneymanager/crt/MM_FixedCount.h>
#include <data/indicator/crt/KDATA.h>
#include <data/indicator/crt/EMA.h>
#include "../selector/create_test_strategy.h"

using namespace hku;

/**
 * @defgroup test_Portfolio test_Portfolio
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test point: all the instances are in the delay mode and the positions are not adjusted */
TEST_CASE("test_PF_for_delay_and_no_adjust") {
    DataRuntime& sm = getDataRuntime();

    auto sys = create_test_strategy(MM_FixedCount(100),
                                    SG_CrossGold(EMA(CLOSE(), 12), EMA(CLOSE(), 26)));

    auto account = create_test_account(AccountConfig(Datetime(199001010000L), 500000));
    SEPtr se = SE_Fixed();
    se->addStockList({sm["sz000001"], sm["sz000063"], sm["sz000651"]}, sys);
    AFPtr af = AF_EqualWeight();
    PFPtr pf = PF_Simple(account, se, af);

    KQuery query = KQueryByDate(Datetime(201101010000L), Null<Datetime>(), KQuery::DAY);
    pf->run(query);

    /** @arg */
}

/** @} */
