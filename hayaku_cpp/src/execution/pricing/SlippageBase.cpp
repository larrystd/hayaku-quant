/*
 * SlippageBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "SlippageBase.h"

namespace hayaku {

HAYAKU_API std::ostream& operator<<(std::ostream& os, const SlippageBase& sp) {
  os << "Slippage(" << sp.name() << ", " << sp.getParameter() << ")";
  return os;
}

HAYAKU_API std::ostream& operator<<(std::ostream& os, const SlippagePtr& sp) {
  if (sp) {
    os << *sp;
  } else {
    os << "Slippage(NULL)";
  }
  return os;
}

SlippageBase::SlippageBase() : name_("SlippageBase") {}

SlippageBase::SlippageBase(const string& name) : name_(name) {}

void SlippageBase::baseCheckParam(const string& name) const {}
void SlippageBase::paramChanged() {}

void SlippageBase::reset() {
  kdata_ = Null<KData>();
  _reset();
}

SlippagePtr SlippageBase::clone() {
  SlippagePtr p;
  try {
    p = _clone();
  } catch (...) {
    HAYAKU_ERROR("Subclass _clone failed!");
    p = SlippagePtr();
  }

  if (!p || p.get() == this) {
    HAYAKU_ERROR("Failed clone! Will use self-ptr!");
    return shared_from_this();
  }

  p->params_ = params_;
  p->name_ = name_;
  p->is_python_object_ = is_python_object_;
  p->kdata_ = kdata_;
  return p;
}

void SlippageBase::setTO(const KData& kdata) {
  HAYAKU_IF_RETURN(kdata_ == kdata, void());
  kdata_ = kdata;
  if (!kdata.empty()) {
    _calculate();
  }
}

} /* namespace hayaku */
