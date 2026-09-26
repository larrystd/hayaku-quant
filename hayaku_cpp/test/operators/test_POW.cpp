/*
 * test_POW.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

#include <data/DataRuntime.h>
#include <operators/ScalarMathOperators.h>
#include <operators/SeriesOperators.h>

#include <fstream>

#include "doctest/doctest.h"

using namespace hayaku;

/**
 * @defgroup test_indicator_POW test_indicator_POW
 * @ingroup test_hayaku_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_POW") {
  Indicator result;

  PriceList a;
  for (int i = 0; i < 10; ++i) {
    a.push_back(i);
  }

  Indicator data = PRICELIST(a);

  result = POW(data, 2);
  CHECK_EQ(result.name(), "POW");
  CHECK_EQ(result.discard(), 0);
  for (int i = 0; i < 10; ++i) {
    CHECK_EQ(result[i], std::pow(data[i], 2));
  }

  result = POW(-11, 3);
  CHECK_EQ(result.name(), "POW");
  CHECK_EQ(result.size(), 1);
  CHECK_EQ(result.discard(), 0);
  CHECK_EQ(result[0], std::pow(-11, 3));
}

/** @par Test points */
TEST_CASE("test_POW_dyn") {
  Stock stock = getDataRuntime().getStock("sh000001");
  KData kdata = stock.getKData(KQuery(-30));
  // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
  Indicator c = CLOSE(kdata);
  Indicator expect = POW(c, 10);
  Indicator result = POW(c, CVAL(c, 10));
  CHECK_EQ(expect.size(), result.size());
  CHECK_EQ(expect.discard(), result.discard());
  for (size_t i = 0; i < result.discard(); i++) {
    CHECK_UNARY(std::isnan(result[i]));
  }
  for (size_t i = expect.discard(); i < expect.size(); i++) {
    CHECK_EQ(expect[i], doctest::Approx(result[i]));
  }

  result = POW(c, IndParam(CVAL(c, 10)));
  CHECK_EQ(expect.size(), result.size());
  CHECK_EQ(expect.discard(), result.discard());
  for (size_t i = 0; i < result.discard(); i++) {
    CHECK_UNARY(std::isnan(result[i]));
  }
  for (size_t i = expect.discard(); i < expect.size(); i++) {
    CHECK_EQ(expect[i], doctest::Approx(result[i]));
  }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HAYAKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_POW_export") {
  DataRuntime& sm = getDataRuntime();
  string filename(sm.tmpdir());
  filename += "/POW.xml";

  Stock stock = sm.getStock("sh000001");
  KData kdata = stock.getKData(KQuery(-20));
  Indicator x1 = POW(CLOSE(kdata), 2);
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

  CHECK_EQ(x1.name(), x2.name());
  CHECK_EQ(x1.size(), x2.size());
  CHECK_EQ(x1.discard(), x2.discard());
  CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
  for (size_t i = 0; i < x1.size(); ++i) {
    CHECK_EQ(x1[i], doctest::Approx(x2[i]));
  }
}
#endif /* #if HAYAKU_SUPPORT_SERIALIZATION */

/** @} */
