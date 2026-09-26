/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-27
 *      Author: fasiondog
 */

#include "strategy/portfolio/AllocationPolicies.h"
#include "test_config.h"

using namespace hayaku;

/**
 * @defgroup test_AF_FixedWeight test_AF_FixedWeight
 * @ingroup test_hayaku_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_AF_FixedWeight") {
  /** @arg Invalid parameters */
  CHECK_THROWS_AS(AF_FixedWeight(0.0), std::exception);
  CHECK_THROWS_AS(AF_FixedWeight(-0.1), std::exception);
  CHECK_THROWS_AS(AF_FixedWeight(1.001), std::exception);

  /** @arg Try to change an illegal common parameter */
  auto af = AF_FixedWeight(0.1);
  CHECK_THROWS_AS(af->setParam<bool>("auto_adjust_weight", true),
                  std::exception);
}

/** @} */
