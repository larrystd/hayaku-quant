/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-18
 *      Author: fasiondog
 */

#include <data/DataRuntime.h>
#include <extensions/talib/TalibOperators.h>
#include <operators/SeriesOperators.h>

#include <fstream>

#include "test_config.h"

using namespace hayaku;

/**
 * @defgroup test_indicator_TA_AD test_indicator_TA_AD
 * @ingroup test_hayaku_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_TA_AD") {
  KData kdata = getKData("sz000001", KQuery(-30));
  Indicator result = TA_AD(kdata);
  CHECK_EQ(result.name(), "TA_AD");
  CHECK_EQ(result.discard(), 0);
  CHECK_EQ(result.size(), kdata.size());
  CHECK_EQ(result[0], doctest::Approx(59332.81081).epsilon(0.00001));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HAYAKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_TA_AD_export") {
  DataRuntime& sm = getDataRuntime();
  string filename(sm.tmpdir());
  filename += "/TA_AD.xml";

  Stock stock = sm.getStock("sh000001");
  KData kdata = stock.getKData(KQuery(-20));
  Indicator x1 = TA_AD(kdata);
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
  CHECK_UNARY(x1.size() == x2.size());
  CHECK_UNARY(x1.discard() == x2.discard());
  CHECK_UNARY(x1.getResultNumber() == x2.getResultNumber());
  for (size_t i = x1.discard(); i < x1.size(); ++i) {
    CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
  }
}
#endif /* #if HAYAKU_SUPPORT_SERIALIZATION */

/** @} */
