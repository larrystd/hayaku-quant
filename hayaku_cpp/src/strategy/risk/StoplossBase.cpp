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

StoplossBase::StoplossBase() : name_("StoplossBase") {}

StoplossBase::StoplossBase(const string& name) : name_(name) {}

StoplossBase::~StoplossBase() {}

void StoplossBase::baseCheckParam(const string& name) const {}
void StoplossBase::paramChanged() {}

void StoplossBase::reset() {
  kdata_ = Null<KData>();
  account_.reset();
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

  p->is_python_object_ = is_python_object_;
  p->name_ = name_;
  p->params_ = params_;
  // The account is injected by StrategyRuntime for each run.
  p->kdata_ = kdata_;
  return p;
}

void StoplossBase::setTO(const KData& kdata) {
  HAYAKU_IF_RETURN(kdata_ == kdata, void());
  kdata_ = kdata;
  if (!kdata.empty()) {
    _calculate();
  }
}

} /* namespace hayaku */
