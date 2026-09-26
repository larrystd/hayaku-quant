/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-15
 *      Author: fasiondog
 */

#include <application/plugins/ExtendIndicatorsPlugin.h>
#include <data/DataRuntime.h>
#include <operators/SeriesOperators.h>

#include "application/plugin_fixtures/plugin_valid.h"
#include "test_config.h"

using namespace hayaku;

/**
 * @defgroup test_indicator_GROUP_PROD test_indicator_GROUP_PROD
 * @ingroup test_hayaku_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_GROUP_PROD") {
  HAYAKU_IF_RETURN(!pluginValid(), void());

  auto stk = getStock("sh000001");
  auto mink = stk.getKData(
      KQueryByDate(Datetime(20111115), Datetime(20111120), KQuery::MIN));

  /** @arg The minute lines grouped by day */
  auto ind = GROUP_PROD(CLOSE(), KQuery::DAY);
  auto result = ind(mink);
  CHECK_EQ(result.name(), "GROUP_PROD");
  CHECK_EQ(result.size(), mink.size());
  CHECK_EQ(result.discard(), 0);
  CHECK_EQ(result[0], doctest::Approx(mink[0].closePrice));
  CHECK_EQ(result[1], doctest::Approx(mink[0].closePrice * mink[1].closePrice));
  CHECK_EQ(result[720], doctest::Approx(mink[720].closePrice));
  CHECK_EQ(result[721],
           doctest::Approx(mink[720].closePrice * mink[721].closePrice));
}

/** @} */
