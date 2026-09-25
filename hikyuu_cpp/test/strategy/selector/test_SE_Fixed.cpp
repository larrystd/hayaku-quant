/*
 * test_Signal.cpp
 *
 *  Created on: 2013-3-11
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <data/internal/DataRuntime.h>
#include "create_test_strategy.h"
#include <strategy/selector/crt/SE_Fixed.h>
#include <strategy/signal/crt/SG_Cross.h>
#include <strategy/moneymanager/crt/MM_FixedCount.h>
#include <data/indicator/crt/KDATA.h>
#include <data/indicator/crt/MA.h>

using namespace hku;

/**
 * @defgroup test_SE_Fixed test_SE_Fixed
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_SE_Fixed") {
    DataRuntime& sm = getDataRuntime();

    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);
    auto sys = create_test_strategy(mm, sg);
    SEPtr se = SE_Fixed();

    /** @arg Try to add a stock that does not exist */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg Try to add an empty system strategy prototype */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], internal::StrategyRuntimePtr()), std::exception);

    // Currently a PF must specify the actually executed subsystem, so the code below cannot run
    // /** @arg getSelectedSystemList */
    se->addStock(sm["sh600000"], sys);
    se->addStock(sm["sz000001"], sys);
    se->addStock(sm["sz000002"], sys);

    auto proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);

    se->calculate(proto_sys_list, KQuery(-20));
    auto result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].strategy->getStock());
    CHECK_EQ(sm["sz000001"], result[1].strategy->getStock());
    CHECK_EQ(sm["sz000002"], result[2].strategy->getStock());

    /** @arg reset */
    se->reset();
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 0);

    /** @arg The clone operation */
    proto_sys_list = se->getProtoSystemList();
    se->calculate(proto_sys_list, KQuery(-20));
    SEPtr se2;
    se2 = se->clone();
    CHECK_NE(se2.get(), se.get());
    result = se2->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].strategy->getStock());
    CHECK_EQ(sm["sz000001"], result[1].strategy->getStock());
    CHECK_EQ(sm["sz000002"], result[2].strategy->getStock());
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_SE_Fixed_export") {
    DataRuntime& sm = getDataRuntime();
    string filename(sm.tmpdir());
    filename += "/SE_FIXED.xml";

    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);
    auto sys = create_test_strategy(mm, sg, "fixed-selector-export");
    StockList stkList;
    stkList.push_back(sm["sh600000"]);
    stkList.push_back(sm["sz000001"]);

    SEPtr se1 = SE_Fixed(stkList, sys);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(se1);
    }

    SEPtr se2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(se2);
    }

    CHECK_EQ(se1->name(), se2->name());
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
