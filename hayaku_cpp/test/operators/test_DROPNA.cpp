/*
 * test_DROPNA.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-28
 *      Author: fasiondog
 */

#include <data/DataRuntime.h>
#include <operators/SeriesOperators.h>

#include <fstream>

#include "doctest/doctest.h"

using namespace hayaku;

/**
 * @defgroup test_indicator_DROPNA test_indicator_DROPNA
 * @ingroup test_hayaku_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_DROPNA") {
  Indicator result;

  PriceList a;
  for (int i = 0; i < 10; ++i) {
    a.push_back(i);
  }

  Indicator data = PRICELIST(a);

  /** @arg There is no nan value */
  result = DROPNA(data);
  CHECK_EQ(result.name(), "DROPNA");
  CHECK_EQ(result.discard(), 0);
  for (int i = 0; i < 10; ++i) {
    CHECK_EQ(result[i], data[i]);
  }

  /** @arg All the values are nan */
  a.clear();
  for (int i = 0; i < 10; i++) {
    a.push_back(Null<price_t>());
  }

  data = VALUE(a);
  result = DROPNA(data);
  CHECK_EQ(result.size(), 0);
  CHECK_EQ(result.discard(), 0);

  /** @arg There is a nan value in the middle */
  a.push_back(Null<price_t>());
  a.push_back(12);
  a.push_back(Null<price_t>());
  a.push_back(15);
  a.push_back(Null<price_t>());
  data = VALUE(a);
  result = DROPNA(data);
  CHECK_EQ(result.size(), 2);
  CHECK_EQ(result.discard(), 0);
  CHECK_EQ(result[0], 12);
  CHECK_EQ(result[1], 15);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HAYAKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_DROPNA_export") {
  DataRuntime& sm = getDataRuntime();
  string filename(sm.tmpdir());
  filename += "/DROPNA.xml";

  Stock stock = sm.getStock("sh000001");
  KData kdata = stock.getKData(KQuery(-20));
  Indicator x1 = DROPNA(CLOSE(kdata));
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

  CHECK_EQ(x2.name(), "DROPNA");
  CHECK_EQ(x1.size(), x2.size());
  CHECK_EQ(x1.discard(), x2.discard());
  CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
  for (size_t i = 0; i < x1.size(); ++i) {
    CHECK_EQ(x1[i], doctest::Approx(x2[i]));
  }
}
#endif /* #if HAYAKU_SUPPORT_SERIALIZATION */

/** @} */
