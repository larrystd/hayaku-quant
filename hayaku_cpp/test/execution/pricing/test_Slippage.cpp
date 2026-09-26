/*
 * test_Slippage.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <data/DataRuntime.h>
#include <execution/pricing/SlippageBase.h>

#include "doctest/doctest.h"

using namespace hayaku;

class SlippageTest : public SlippageBase {
 public:
  SlippageTest() : SlippageBase("SlippageTest"), x_(0) {}
  virtual ~SlippageTest() {}

  virtual price_t getRealBuyPrice(const Datetime &datetime, price_t price) {
    return x_ < 10 ? 0.0 : 1.0;
  }

  virtual price_t getRealSellPrice(const Datetime &datetime, price_t price) {
    return x_ < 10 ? 0.0 : 1.0;
  }

  virtual void _reset() { x_ = 0; }

  virtual SlippagePtr _clone() {
    SlippageTest *p = new SlippageTest;
    p->x_ = x_;
    return SlippagePtr(p);
  }

  virtual void _calculate() {}

  int getX() const { return x_; }
  void setX(int x) { x_ = x; }

 private:
  int x_;
};

/**
 * @defgroup test_Slippage test_Slippage
 * @ingroup test_hayaku_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_Slippage") {
  /** @arg The basic operation */
  SlippagePtr p(new SlippageTest);
  CHECK_EQ(p->name(), "SlippageTest");
  CHECK_EQ(p->getRealBuyPrice(Datetime(200101010000), 1.0), 0.0);
  CHECK_EQ(p->getRealSellPrice(Datetime(200101010000), 1.0), 0.0);
  SlippageTest *p_src = (SlippageTest *)p.get();
  CHECK_EQ(p_src->getX(), 0);

  p_src->setX(10);
  CHECK_EQ(p->getRealBuyPrice(Datetime(200101010000), 1.0), 1.0);
  CHECK_EQ(p->getRealSellPrice(Datetime(200101010000), 1.0), 1.0);
  CHECK_EQ(p_src->getX(), 10);
  p->reset();
  CHECK_EQ(p_src->getX(), 0);

  /** @arg Test the clone operation */
  p_src->setX(10);
  SlippagePtr p_clone = p->clone();
  CHECK_EQ(p_clone->name(), "SlippageTest");
  p_src = (SlippageTest *)p_clone.get();
  CHECK_EQ(p_src->getX(), 10);
  CHECK_NE(p, p_clone);
}

/** @} */
