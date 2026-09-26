/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-21
 *      Author: fasiondog
 */

#include "BoolCondition.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::BoolCondition)
#endif

namespace hayaku {

BoolCondition::BoolCondition() : ConditionBase("CN_Bool") {}

BoolCondition::BoolCondition(const Indicator& ind)
    : ConditionBase("CN_Bool"), ind_(ind) {}

BoolCondition::~BoolCondition() {}

ConditionPtr BoolCondition::_clone() {
  return make_shared<BoolCondition>(ind_.clone());
}

void BoolCondition::_calculate() {
  auto ds = kdata_.getDatetimeList();
  ind_.setContext(kdata_);
  auto const* ind_data = ind_.data();
  for (size_t i = ind_.discard(), len = ind_.size(); i < len; i++) {
    if (!std::isnan(ind_data[i]) && ind_data[i] > 0.) {
      _addValid(ds[i]);
    }
  }
}

CNPtr HAYAKU_API CN_Bool(const Indicator& ind) {
  return make_shared<BoolCondition>(ind);
}

}  // namespace hayaku
