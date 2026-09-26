/*
 * test_MoneyManager.cpp
 *
 *  Created on: 2013-3-11
 *      Author: fasiondog
 */

#include <data/DataRuntime.h>
#include <strategy/risk/MoneyManagerBase.h>

#include "../selection/create_test_strategy.h"
#include "doctest/doctest.h"

using namespace hayaku;

class MoneyManagerTest : public MoneyManagerBase {
 public:
  MoneyManagerTest() : MoneyManagerBase("MoneyManagerTest") { x_ = 0; }
  virtual ~MoneyManagerTest() {}

  int getX() const { return x_; }
  void setX(int x) { x_ = x; }

  virtual double _getBuyNumber(const Datetime &datetime, const Stock &stock,
                               price_t price, price_t risk, OrderOrigin from) {
    return 0;
  }

  virtual void _reset() { x_ = 0; }

  virtual MoneyManagerPtr _clone() {
    MoneyManagerTest *p = new MoneyManagerTest;
    p->x_ = x_;
    return MoneyManagerPtr(p);
  }

 private:
  int x_;
};

/**
 * @defgroup test_MoneyManager test_MoneyManager
 * @ingroup test_hayaku_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MoneyManager") {
  DataRuntime &sm = getDataRuntime();
  Stock stock = sm["sh000001"];
  internal::PortfolioAccountPortPtr tm = create_test_account(AccountConfig());

  /** @arg The basic operation */
  MoneyManagerPtr p(new MoneyManagerTest);
  MoneyManagerTest *p_src = (MoneyManagerTest *)p.get();
  CHECK_EQ(p->name(), "MoneyManagerTest");
  CHECK_EQ(p_src->getAccount(), internal::PortfolioAccountPortPtr());
  p->setAccount(tm);
  CHECK_EQ(p_src->getAccount(), tm);
  CHECK_EQ(p->getBuyNumber(Datetime(200001010000), stock, 10.0, 10.0,
                           OrderOrigin::SIGNAL),
           0);
  CHECK_UNARY((p->getSellNumber(Datetime(200001010000), stock, 10.0, 10.0,
                                OrderOrigin::SIGNAL) == MAX_DOUBLE));
  CHECK_EQ(p_src->getX(), 0);
  p_src->setX(10);
  CHECK_EQ(p_src->getX(), 10);
  p->reset();
  CHECK_EQ(p_src->getX(), 0);

  /** @arg The clone operation */
  p_src->setX(10);
  MoneyManagerPtr p_clone = p->clone();
  CHECK_NE(p, p_clone);
  p_src = (MoneyManagerTest *)p_clone.get();
  CHECK_EQ(p->name(), "MoneyManagerTest");
  // CHECK_EQ(p_src->getAccount() == tm);
  CHECK_EQ(p_src->getX(), 10);
}

/** @} */
