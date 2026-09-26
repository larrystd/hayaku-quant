/*
 * NoGoalProfitGoal.cpp
 *
 *  Created on: 2016-5-6
 *      Author: Administrator
 */

#include "NoGoalProfitGoal.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::NoGoalProfitGoal)
#endif

namespace hayaku {

NoGoalProfitGoal::NoGoalProfitGoal() : ProfitGoalBase("PG_NoGoal") {}

NoGoalProfitGoal::~NoGoalProfitGoal() {}

price_t NoGoalProfitGoal::getGoal(const Datetime& datetime, price_t price) {
  return Null<price_t>();
}

ProfitGoalPtr HAYAKU_API PG_NoGoal() { return make_shared<NoGoalProfitGoal>(); }

} /* namespace hayaku */
