/*
 * test_Stoploss.cpp
 *
 *  Created on: 2013-3-13
 *      Author: fasiondog
 */

#include <data/DataRuntime.h>
#include <strategy/risk/StoplossBase.h>

#include "doctest/doctest.h"

using namespace hayaku;

class StoplossTest : public StoplossBase {
 public:
  StoplossTest() : StoplossBase("StoplossTest"), x_(0) {}
  virtual ~StoplossTest() {}

  virtual price_t getPrice(const Datetime &datetime, price_t price) {
    return x_ < 10 ? 0.0 : 1.0;
  }

  virtual void _reset() { x_ = 0; }

  virtual StoplossPtr _clone() {
    StoplossTest *p = new StoplossTest;
    p->x_ = x_;
    return StoplossPtr(p);
  }

  virtual void _calculate() {}

  int getX() const { return x_; }
  void setX(int x) { x_ = x; }

 private:
  int x_;
};

/**
 * @defgroup test_Stoploss test_Stoploss
 * @ingroup test_hayaku_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_Stoploss") {
  /** @arg The basic operation */
  StoplossPtr p(new StoplossTest);
  CHECK_EQ(p->name(), "StoplossTest");
  CHECK_EQ(p->getPrice(Datetime(200101010000), 1.0), 0.0);
  StoplossTest *p_src = (StoplossTest *)p.get();
  CHECK_EQ(p_src->getX(), 0);

  p_src->setX(10);
  CHECK_EQ(p->getPrice(Datetime(200101010000), 1.0), 1.0);
  CHECK_EQ(p_src->getX(), 10);
  p->reset();
  CHECK_EQ(p_src->getX(), 0);

  /** @arg Test the clone operation */
  p_src->setX(10);
  StoplossPtr p_clone = p->clone();
  CHECK_EQ(p_clone->name(), "StoplossTest");
  p_src = (StoplossTest *)p_clone.get();
  CHECK_EQ(p_src->getX(), 10);
  CHECK_NE(p, p_clone);
}

/** @} */
