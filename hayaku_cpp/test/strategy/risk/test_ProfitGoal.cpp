/*
 * test_ProfitGoal.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <data/DataRuntime.h>
#include <strategy/risk/ProfitGoalBase.h>

#include "doctest/doctest.h"

using namespace hayaku;

class ProfitGoalTest : public ProfitGoalBase {
 public:
  ProfitGoalTest() : ProfitGoalBase("ProfitGoalTest"), x_(0) {}
  virtual ~ProfitGoalTest() {}

  virtual price_t getGoal(const Datetime &datetime, price_t price) {
    return x_ < 10 ? 0.0 : 1.0;
  }

  virtual void _reset() { x_ = 0; }

  virtual ProfitGoalPtr _clone() {
    ProfitGoalTest *p = new ProfitGoalTest;
    p->x_ = x_;
    return ProfitGoalPtr(p);
  }

  virtual void _calculate() {}

  int getX() const { return x_; }
  void setX(int x) { x_ = x; }

 private:
  int x_;
};

/**
 * @defgroup test_ProfitGoal test_ProfitGoal
 * @ingroup test_hayaku_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_ProfitGoal") {
  /** @arg The basic operation */
  ProfitGoalPtr p(new ProfitGoalTest);
  CHECK_EQ(p->name(), "ProfitGoalTest");
  CHECK_EQ(p->getGoal(Datetime(200101010000), 1.0), 0.0);
  ProfitGoalTest *p_src = (ProfitGoalTest *)p.get();
  CHECK_EQ(p_src->getX(), 0);

  p_src->setX(10);
  CHECK_EQ(p->getGoal(Datetime(200101010000), 1.0), 1.0);
  CHECK_EQ(p_src->getX(), 10);
  p->reset();
  CHECK_EQ(p_src->getX(), 0);

  /** @arg Test the clone operation */
  p_src->setX(10);
  ProfitGoalPtr p_clone = p->clone();
  CHECK_EQ(p_clone->name(), "ProfitGoalTest");
  p_src = (ProfitGoalTest *)p_clone.get();
  CHECK_EQ(p_src->getX(), 10);
  CHECK_NE(p, p_clone);
}

/** @} */
