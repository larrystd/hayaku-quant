/*
 * test_PRICELIST.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include <data/DataRuntime.h>
#include <operators/SeriesOperators.h>

#include <fstream>

#include "doctest/doctest.h"

using namespace hayaku;

/**
 * @defgroup test_indicator_PRICELIST test_indicator_PRICELIST
 * @ingroup test_hayaku_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_PRICELIST") {
  PriceList tmp_list;
  Indicator result;

  /** @arg The PriceList is empty */
  result = PRICELIST(tmp_list);
  CHECK_EQ(result.size(), tmp_list.size());
  CHECK_EQ(result.empty(), true);
  /** @arg The PriceList is not empty */
  for (size_t i = 0; i < 10; ++i) {
    tmp_list.push_back(i);
  }
  result = PRICELIST(tmp_list);
  CHECK_EQ(result.size(), tmp_list.size());
  CHECK_EQ(result.empty(), false);
  for (size_t i = 0; i < 10; ++i) {
    CHECK_EQ(result[i], tmp_list[i]);
  }

  /** @arg The array pointer is null */
  price_t* p_tmp = NULL;
  result = PRICELIST(p_tmp, 10);
  CHECK_EQ(result.size(), 0);
  CHECK_EQ(result.empty(), true);

  /** @arg The array pointer is not null */
  price_t tmp[10];
  for (size_t i = 0; i < 10; ++i) {
    tmp[i] = i;
  }
  result = PRICELIST(tmp, 10);
  for (size_t i = 0; i < 10; ++i) {
    CHECK_EQ(result[i], tmp[i]);
  }

  /** @arg Constructed from a PriceList */
  result = PRICELIST(tmp_list);
  CHECK_EQ(result.size(), 10);
  for (size_t i = 0; i < 10; ++i) {
    CHECK_EQ(result[i], tmp_list[i]);
  }

  /** @arg From a PriceList with discard=1 */
  result = PRICELIST(tmp_list, 1);
  CHECK_EQ(result.size(), 10);
  CHECK_EQ(result.discard(), 1);
  CHECK_UNARY(std::isnan(result[0]));
  for (size_t i = 1; i < 10; ++i) {
    CHECK_EQ(result[i], tmp_list[i]);
  }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HAYAKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_PRICELIST_export") {
  DataRuntime& sm = getDataRuntime();
  string filename(sm.tmpdir());
  filename += "/PRICELIST.xml";

  PriceList d;
  for (size_t i = 0; i < 20; ++i) {
    d.push_back(i);
  }

  Indicator ma1 = PRICELIST(d);
  {
    std::ofstream ofs(filename);
    boost::archive::xml_oarchive oa(ofs);
    oa << BOOST_SERIALIZATION_NVP(ma1);
  }

  Indicator ma2;
  {
    std::ifstream ifs(filename);
    boost::archive::xml_iarchive ia(ifs);
    ia >> BOOST_SERIALIZATION_NVP(ma2);
  }

  CHECK_EQ(ma1.size(), ma2.size());
  CHECK_EQ(ma1.discard(), ma2.discard());
  CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
  for (size_t i = 0; i < ma1.size(); ++i) {
    CHECK_EQ(ma1[i], doctest::Approx(ma2[i]));
  }
}
#endif /* #if HAYAKU_SUPPORT_SERIALIZATION */

/** @} */
