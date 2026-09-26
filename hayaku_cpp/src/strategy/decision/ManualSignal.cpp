/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#include "ManualSignal.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ManualSignal)
#endif

namespace hayaku {

ManualSignal::ManualSignal() : SignalBase("SG_Manual") {}

void ManualSignal::_calculate(const KData&) {}

SignalPtr SG_Manual() { return make_shared<ManualSignal>(); }

}  // namespace hayaku
