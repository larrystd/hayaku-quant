/*
 * ProfitGoal.cpp
 *
 *  Created on: 2013-3-7
 *      Author: fasiondog
 */

#include "ProfitGoalBase.h"

namespace hayaku {

HAYAKU_API std::ostream& operator<<(std::ostream& os,
                                    const ProfitGoalBase& pg) {
  os << "ProfitGoal(" << pg.name() << ", " << pg.getParameter() << ")";
  return os;
}

HAYAKU_API std::ostream& operator<<(std::ostream& os, const ProfitGoalPtr& pg) {
  if (pg) {
    os << *pg;
  } else {
    os << "ProfitGoal(NULL)";
  }
  return os;
}

ProfitGoalBase::ProfitGoalBase() : name_("ProfitGoalBase") {}

ProfitGoalBase::ProfitGoalBase(const string& name) : name_(name) {}

ProfitGoalBase::~ProfitGoalBase() {}

void ProfitGoalBase::baseCheckParam(const string& name) const {}
void ProfitGoalBase::paramChanged() {}

void ProfitGoalBase::reset() {
  kdata_ = Null<KData>();
  account_.reset();
  _reset();
}

ProfitGoalPtr ProfitGoalBase::clone() {
  ProfitGoalPtr p;
  try {
    p = _clone();
  } catch (...) {
    HAYAKU_ERROR("Subclass _clone failed!");
    p = ProfitGoalPtr();
  }

  if (!p || p.get() == this) {
    HAYAKU_ERROR("Failed clone! Will use self-ptr!");
    return shared_from_this();
  }

  p->params_ = params_;
  p->name_ = name_;
  p->is_python_object_ = is_python_object_;
  p->account_ = account_;
  p->kdata_ = kdata_;
  return p;
}

void ProfitGoalBase::setTO(const KData& kdata) {
  HAYAKU_IF_RETURN(kdata_ == kdata, void());
  kdata_ = kdata;
  if (!kdata.empty()) {
    _calculate();
  }
}

} /* namespace hayaku */
