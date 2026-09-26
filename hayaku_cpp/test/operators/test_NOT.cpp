/*
 * test_ABS.cpp
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */
#include <data/DataRuntime.h>
#include <operators/BooleanOperators.h>
#include <operators/SeriesOperators.h>

#include <fstream>

#include "doctest/doctest.h"

using namespace hayaku;

/**
 * @defgroup test_indicator_NOT test_indicator_NOT
 * @ingroup test_hayaku_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_NOT") {
  Indicator result;

  PriceList a;
  for (int i = 0; i < 10; ++i) {
    a.push_back(5 - i);
  }

  vector<price_t> expect = {0., 0., 0., 0., 0., 1., 1., 1., 1., 1.};

  Indicator data = PRICELIST(a);

  result = NOT(data);
  CHECK_EQ(result.name(), "NOT");
  CHECK_EQ(result.discard(), 0);
  CHECK_EQ(result[0], 0.0);
  for (int i = 1; i < 10; ++i) {
    CHECK_EQ(result[i], expect[i]);
  }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HAYAKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_NOT_export") {
  DataRuntime& sm = getDataRuntime();
  string filename(sm.tmpdir());
  filename += "/NOT.xml";

  Stock stock = sm.getStock("sh000001");
  KData kdata = stock.getKData(KQuery(-20));
  Indicator x1 = NOT(CLOSE(kdata));
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
