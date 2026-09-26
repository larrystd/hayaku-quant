/*
 * test_ACOS.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#include "test_config.h"
#include <fstream>
#include <data/DataRuntime.h>
#include <operators/MomentumOperators.h>
#include <operators/WindowOperators.h>
#include <operators/SeriesOperators.h>

using namespace hayaku;

/**
 * @defgroup test_indicator_ATR test_indicator_ATR
 * @ingroup test_hayaku_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_ATR") {
    auto k = getKData("sh000001", KQuery(-30));

    auto atr = ATR(k, 10);
    CHECK_EQ(atr.name(), "ATR");
    CHECK_EQ(atr.size(), k.size());
    CHECK_EQ(atr.discard(), 11);

    auto expect = DISCARD(MA(TR(), 10), 11)(k);
    for (size_t i = atr.discard(); i < atr.size(); ++i) {
        CHECK_EQ(atr[i], doctest::Approx(expect[i]));
    }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_ATR_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    int cycle = 1000;  // Test loop count

    {
        BENCHMARK_TIME_MSG(test_ATR_benchmark, cycle, fmt::format("data len: {}", kdata.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = ATR();
            Indicator result = ind(kdata);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HAYAKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_ATR_export") {
    DataRuntime& sm = getDataRuntime();
    string filename(sm.tmpdir());
    filename += "/ATR.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = ATR(kdata);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(x1);
    }

    Indicator x2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(x2);
    }

    CHECK_EQ(x2.name(), "ATR");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HAYAKU_SUPPORT_SERIALIZATION */

/** @} */
