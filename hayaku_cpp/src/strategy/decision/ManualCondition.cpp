/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#include "ManualCondition.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ManualCondition)
#endif

namespace hayaku {

ManualCondition::ManualCondition() : ConditionBase("CN_Manual") {}

void ManualCondition::_calculate() {}

CNPtr CN_Manual() { return make_shared<ManualCondition>(); }

}  // namespace hayaku
