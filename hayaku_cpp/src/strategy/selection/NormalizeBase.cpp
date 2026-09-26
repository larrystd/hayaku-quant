/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#include "NormalizeBase.h"

namespace hayaku {

HAYAKU_API std::ostream& operator<<(std::ostream& out, const NormalizeBase& norm) {
    out << "Normalize{" << "\n  name: " << norm.name() << "\n  params: " << norm.getParameter();
    out << "\n}";
    return out;
}

HAYAKU_API std::ostream& operator<<(std::ostream& out, const NormalizePtr& norm) {
    if (norm) {
        out << *norm;
    } else {
        out << "Normalize(NULL)";
    }
    return out;
}

void NormalizeBase::paramChanged() {}

void NormalizeBase::baseCheckParam(const string& name) const {}

NormalizePtr NormalizeBase::clone() {
    NormalizePtr p;
    p = _clone();
    HAYAKU_ERROR_IF(!p, "Failed clone! {}", m_name);

    p->m_name = m_name;
    p->m_params = m_params;
    p->m_is_python_object = m_is_python_object;
    return p;
}

}  // namespace hayaku
