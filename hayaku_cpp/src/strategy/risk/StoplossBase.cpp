/*
 * StoplossBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "StoplossBase.h"

namespace hayaku {

HAYAKU_API std::ostream& operator<<(std::ostream& os, const StoplossBase& sl) {
    os << "Stoploss(" << sl.name() << ", " << sl.getParameter() << ")";
    return os;
}

HAYAKU_API std::ostream& operator<<(std::ostream& os, const StoplossPtr& sl) {
    if (sl) {
        os << *sl;
    } else {
        os << "Stoploss(NULL)";
    }
    return os;
}

StoplossBase::StoplossBase() : m_name("StoplossBase") {}

StoplossBase::StoplossBase(const string& name) : m_name(name) {}

StoplossBase::~StoplossBase() {}

void StoplossBase::baseCheckParam(const string& name) const {}
void StoplossBase::paramChanged() {}

void StoplossBase::reset() {
    m_kdata = Null<KData>();
    m_account.reset();
    _reset();
}

StoplossPtr StoplossBase::clone() {
    StoplossPtr p;
    try {
        p = _clone();
    } catch (...) {
        HAYAKU_ERROR("Subclass _clone failed!");
        p = StoplossPtr();
    }

    if (!p || p.get() == this) {
        HAYAKU_ERROR("Failed clone! Will use self-ptr!");
        return shared_from_this();
    }

    p->m_is_python_object = m_is_python_object;
    p->m_name = m_name;
    p->m_params = m_params;
    // The account is injected by StrategyRuntime for each run.
    p->m_kdata = m_kdata;
    return p;
}

void StoplossBase::setTO(const KData& kdata) {
    HAYAKU_IF_RETURN(m_kdata == kdata, void());
    m_kdata = kdata;
    if (!kdata.empty()) {
        _calculate();
    }
}

} /* namespace hayaku */
