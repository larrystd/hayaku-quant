/*
 * test_MIN.cpp
 *
 *  Created on: 2019-4-9
 *      Author: fasiondog
 */

#include <data/DataRuntime.h>
#include <operators/SeriesOperators.h>
#include <operators/WindowOperators.h>

#include <fstream>

#include "doctest/doctest.h"

using namespace hayaku;

/**
 * @defgroup test_indicator_MIN test_indicator_MIN
 * @ingroup test_hayaku_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MIN") {
  /** @arg n > 0 */
  KData k = getStock("sh600004").getKData(KQuery(-8));
  Indicator C = CLOSE(k);
  Indicator O = OPEN(k);
  Indicator x = MIN(C, O);

  CHECK_EQ(x.name(), "MIN");
  CHECK_EQ(x.size(), 8);
  CHECK_EQ(x.discard(), 0);
  size_t total = x.size();
  for (int i = 0; i < total; i++) {
    if (C[i] < O[i]) {
      CHECK_EQ(x[i], C[i]);
    } else {
      CHECK_EQ(x[i], O[i]);
    }
  }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HAYAKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_MIN_export") {
  DataRuntime& sm = getDataRuntime();
  string filename(sm.tmpdir());
  filename += "/MIN.xml";

  Indicator x1 = MIN(CLOSE(), OPEN());
  x1.setContext(getStock("sh600004"), KQuery(-8));

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
