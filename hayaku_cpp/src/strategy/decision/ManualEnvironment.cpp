/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#include "ManualEnvironment.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ManualEnvironment)
#endif

namespace hayaku {

ManualEnvironment::ManualEnvironment() : EnvironmentBase("EV_Manual") {}

void ManualEnvironment::_calculate() {}

EVPtr EV_Manual() { return make_shared<ManualEnvironment>(); }

}  // namespace hayaku
