/*
 * EqualWeightAllocateFunds.cpp
 *
 *  Created on: 2018-2-8
 *      Author: fasiondog
 */

#include "EqualWeightAllocateFunds.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::EqualWeightAllocateFunds)
#endif

namespace hayaku {

EqualWeightAllocateFunds::EqualWeightAllocateFunds()
    : AllocateFundsBase("AF_EqualWeight") {}

EqualWeightAllocateFunds::~EqualWeightAllocateFunds() {}

StrategyWeightList EqualWeightAllocateFunds ::_allocateWeight(
    const Datetime& date, const StrategyWeightList& se_list) {
  StrategyWeightList result;
  for (auto iter = se_list.begin(); iter != se_list.end(); ++iter) {
    result.emplace_back(iter->strategy, 1.0);
  }

  return result;
}

AFPtr HAYAKU_API AF_EqualWeight() {
  return make_shared<EqualWeightAllocateFunds>();
}

} /* namespace hayaku */
