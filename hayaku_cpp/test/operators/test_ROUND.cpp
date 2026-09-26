/*
 * test_ROUND.cpp
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <data/DataRuntime.h>
#include <operators/ScalarMathOperators.h>
#include <operators/SeriesOperators.h>

using namespace hayaku;

/**
 * @defgroup test_indicator_ROUND test_indicator_ROUND
 * @ingroup test_hayaku_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_ROUND") {
    Indicator result;

    PriceList a;
    a.push_back(1.323);
    a.push_back(0.300);
    a.push_back(0.305);
    a.push_back(2.675);

    Indicator data = PRICELIST(a);

    result = ROUND(data, 2);
    CHECK_EQ(result.name(), "ROUND");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 4);
    CHECK_EQ(result[0], doctest::Approx(1.32));
    CHECK_EQ(result[1], doctest::Approx(0.30));
    CHECK_EQ(result[2], doctest::Approx(0.31));
    CHECK_EQ(result[3], doctest::Approx(2.68));

    result = ROUND(-11.15, 1);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], doctest::Approx(-11.2));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HAYAKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_ROUND_export") {
    DataRuntime& sm = getDataRuntime();
    string filename(sm.tmpdir());
    filename += "/ROUND.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = ROUND(CLOSE(kdata));
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

    CHECK_EQ(x2.name(), "ROUND");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HAYAKU_SUPPORT_SERIALIZATION */

/** @} */
