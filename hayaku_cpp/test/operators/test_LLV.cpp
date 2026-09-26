/*
 * test_LLV.cpp
 *
 *  Created on: 2019-4-1
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <data/DataRuntime.h>
#include <operators/WindowOperators.h>
#include <operators/SeriesOperators.h>

using namespace hayaku;

/**
 * @defgroup test_indicator_LLV test_indicator_LLV
 * @ingroup test_hayaku_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_LLV") {
    Indicator result;

    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    Indicator data = PRICELIST(a);

    /** @arg n = 0 */
    result = LLV(data, 0);
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], data[0]);
    }

    /** @arg n = 1 */
    result = LLV(data, 1);
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], data[i]);
    }

    /** @arg n = 9 */
    result = LLV(data, 9);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], data[0]);
    CHECK_EQ(result[7], data[0]);
    CHECK_EQ(result[8], data[0]);
    CHECK_EQ(result[9], data[1]);

    /** @arg n = 10 */
    result = LLV(data, 10);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], data[0]);
    CHECK_EQ(result[1], data[0]);
    CHECK_EQ(result[9], data[0]);
}

/** @par Test points */
TEST_CASE("test_LLV_dyn") {
    Indicator result;

    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    Indicator data = PRICELIST(a);

    result = LLV(data, PRICELIST(PriceList(10, 9)));
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], data[0]);
    CHECK_EQ(result[7], data[0]);
    CHECK_EQ(result[8], data[0]);
    CHECK_EQ(result[9], data[1]);

    Indicator expect = LLV(data, 3);
    result = LLV(data, CVAL(data, 3));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < expect.size(); i++) {
        CHECK_EQ(expect[i], result[i]);
    }

    Stock stock = getDataRuntime().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-100));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    expect = LLV(c, 50);
    result = LLV(c, CVAL(c, 50));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], result[i]);
    }

    expect = LLV(c, 0);
    result = LLV(c, CVAL(c, 0));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], result[i]);
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HAYAKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_LLV_export") {
    DataRuntime& sm = getDataRuntime();
    string filename(sm.tmpdir());
    filename += "/LLV.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = LLV(CLOSE(kdata), 2);
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

    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}

/** @par Test points */
TEST_CASE("test_LLV_dyn_export") {
    DataRuntime& sm = getDataRuntime();
    string filename(sm.tmpdir());
    filename += "/LLV.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    auto c = CLOSE(kdata);
    Indicator x1 = LLV(c, CVAL(c, 2));
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

    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HAYAKU_SUPPORT_SERIALIZATION */

/** @} */
