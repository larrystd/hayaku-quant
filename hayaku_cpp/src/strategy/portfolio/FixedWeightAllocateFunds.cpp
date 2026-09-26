/*
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2018-2-8
 *      Author: fasiondog
 */

#include "FixedWeightAllocateFunds.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::FixedWeightAllocateFunds)
#endif

namespace hayaku {

FixedWeightAllocateFunds::FixedWeightAllocateFunds()
    : AllocateFundsBase("AF_FixedWeight") {
  setParam<double>("weight", 0.1);

  // The common parameter must be set to false, the automatic weight adjustment
  // is forbidden
  setParam<bool>("auto_adjust_weight", false);
}

FixedWeightAllocateFunds::~FixedWeightAllocateFunds() {}

void FixedWeightAllocateFunds::_checkParam(const string& name) const {
  if ("weight" == name) {
    double weight = getParam<double>("weight");
    HAYAKU_ASSERT(weight > 0.0 && weight <= 1.);
  } else if ("auto_adjust_weight" == name) {
    bool auto_adjust_weight = getParam<bool>("auto_adjust_weight");
    HAYAKU_CHECK(!auto_adjust_weight,
                 R"(param "auto_adjust_weight" must be false!)");
  }
}

StrategyWeightList FixedWeightAllocateFunds ::_allocateWeight(
    const Datetime& date, const StrategyWeightList& se_list) {
  StrategyWeightList result;
  price_t weight = getParam<double>("weight");
  for (auto iter = se_list.begin(); iter != se_list.end(); ++iter) {
    result.emplace_back(iter->strategy, weight);
  }

  return result;
}

AFPtr HAYAKU_API AF_FixedWeight(double weight) {
  auto p = make_shared<FixedWeightAllocateFunds>();
  p->setParam<double>("weight", weight);
  return p;
}

} /* namespace hayaku */
