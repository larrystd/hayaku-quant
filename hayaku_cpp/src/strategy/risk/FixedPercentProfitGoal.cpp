/*
 * FixedPercentProfitGoal.cpp
 *
 *  Created on: 2016-5-6
 *      Author: Administrator
 */

#include "FixedPercentProfitGoal.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::FixedPercentProfitGoal)
#endif

namespace hayaku {

FixedPercentProfitGoal::FixedPercentProfitGoal()
    : ProfitGoalBase("PG_FixedPercent") {
  setParam<double>("p", 0.2);
}

/**
 * @brief Destructor, it releases the resources of the FixedPercentProfitGoal
 * object
 *
 * This destructor is the default destructor implementation of the
 * FixedPercentProfitGoal class, responsible for cleaning up the resources
 * occupied by the object. Because this class has no dynamically allocated
 * resources, the default empty implementation is used.
 */
FixedPercentProfitGoal::~FixedPercentProfitGoal() {}

void FixedPercentProfitGoal::_checkParam(const string& name) const {
  if ("p" == name) {
    double p = getParam<double>(name);
    HAYAKU_ASSERT(p > 0.0);
  }
}

price_t FixedPercentProfitGoal::getGoal(const Datetime& datetime,
                                        price_t price) {
  Stock stock = getTO().getStock();
  PositionRecord position = getAccount()->getPosition(datetime, stock);
  return position.number != 0 ? (position.buyMoney / position.number) *
                                    (1 + getParam<double>("p"))
                              : price * (1 + getParam<double>("p"));
}

ProfitGoalPtr PG_FixedPercent(double p) {
  ProfitGoalPtr ptr = make_shared<FixedPercentProfitGoal>();
  ptr->setParam<double>("p", p);
  return ptr;
}

} /* namespace hayaku */
