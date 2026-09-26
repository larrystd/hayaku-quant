/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#include "CycleSignal.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::CycleSignal)
#endif

namespace hayaku {

CycleSignal::CycleSignal() : SignalBase("SG_Cycle") {
    setParam<bool>("cycle", true);
    setParam<bool>("alternate", false);
}

void CycleSignal::_checkParam(const string& name) const {
    if ("cycle" == name) {
        bool cycle = getParam<bool>(name);
        HAYAKU_CHECK(cycle, "param cycle must be true!");
    } else if ("alternate" == name) {
        bool alternate = getParam<bool>("alternate");
        HAYAKU_CHECK(!alternate, "param alternate must be false!");
    }
}

void CycleSignal::_calculate(const KData& kdata) {
    _addBuySignal(getCycleStart());
}

SignalPtr HAYAKU_API SG_Cycle() {
    return make_shared<CycleSignal>();
}

}  // namespace hayaku
