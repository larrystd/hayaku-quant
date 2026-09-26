/*
 * test_AMA.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include <data/DataRuntime.h>
#include <operators/BooleanOperators.h>
#include <operators/SeriesOperators.h>

#include <fstream>

#include "doctest/doctest.h"

using namespace hayaku;

/**
 * @defgroup test_indicator_AMA test_indicator_NDAY
 * @ingroup test_hayaku_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_NDAY_dyn") {
  Stock stock = getDataRuntime().getStock("sh000001");
  KData kdata = stock.getKData(KQuery(-30));
  // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
  Indicator c = CLOSE(kdata);
  Indicator o = OPEN(kdata);
  Indicator expect = NDAY(c, o, 10);
  Indicator result = NDAY(c, o, CVAL(c, 10));
  // CHECK_EQ(expect.discard(), result.discard());
  CHECK_EQ(expect.size(), result.size());
  for (size_t i = 0; i < result.discard(); i++) {
    CHECK_UNARY(std::isnan(result[i]));
  }
  for (size_t i = expect.discard(); i < expect.size(); i++) {
    CHECK_EQ(expect[i], doctest::Approx(result[i]));
  }

  result = NDAY(c, o, IndParam(CVAL(c, 10)));
  // CHECK_EQ(expect.discard(), result.discard());
  CHECK_EQ(expect.size(), result.size());
  for (size_t i = 0; i < result.discard(); i++) {
    CHECK_UNARY(std::isnan(result[i]));
  }
  for (size_t i = expect.discard(); i < expect.size(); i++) {
    CHECK_EQ(expect[i], doctest::Approx(result[i]));
  }

  expect = NDAY(c, o, 0);
  result = NDAY(c, o, CVAL(c, 0));
  CHECK_EQ(expect.discard(), result.discard());
  CHECK_EQ(expect.size(), result.size());
  for (size_t i = 0; i < result.discard(); i++) {
    CHECK_UNARY(!std::isnan(result[i]));
  }
  for (size_t i = expect.discard(); i < expect.size(); i++) {
    CHECK_EQ(expect[i], doctest::Approx(result[i]));
  }
}

/** @} */
