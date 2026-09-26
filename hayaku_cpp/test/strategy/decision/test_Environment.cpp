/*
 * test_Environment.cpp
 *
 *  Created on: 2013-3-10
 *      Author: fasiondog
 */

#include <data/DataRuntime.h>
#include <strategy/decision/EnvironmentBase.h>

#include "doctest/doctest.h"

using namespace hayaku;

class EnvironmentTest : public EnvironmentBase {
 public:
  EnvironmentTest() : EnvironmentBase("TEST") { setParam<int>("n", 10); }

  virtual ~EnvironmentTest() {}

  virtual void _calculate() { _addValid(Datetime(200001010000LL)); }

  virtual EnvironmentPtr _clone() { return make_shared<EnvironmentTest>(); }
};

/**
 * @defgroup test_Environment test_Environment
 * @ingroup test_hayaku_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_Environment") {
  /** @arg The basic operation */
  EnvironmentPtr p = make_shared<EnvironmentTest>();
  CHECK_EQ(p->name(), "TEST");
  CHECK_EQ(p->isValid(Datetime(200001010000)), false);
  CHECK_EQ(p->isValid(Datetime(200001020000)), false);

  p->setQuery(KQuery(-100));
  CHECK_EQ(p->isValid(Datetime(200001010000)), true);
  CHECK_EQ(p->isValid(Datetime(200001020000)), false);
  CHECK_EQ(p->getParam<int>("n"), 10);

  /** @arg The clone operation */
  p->setParam<int>("n", 20);
  EnvironmentPtr p_clone = p->clone();
  CHECK_NE(p, p_clone);
  CHECK_EQ(p_clone->name(), "TEST");
  CHECK_EQ(p_clone->isValid(Datetime(200001010000)), true);
  CHECK_EQ(p_clone->isValid(Datetime(200001020000)), false);
  CHECK_EQ(p_clone->getParam<int>("n"), 20);
}

/** @} */
