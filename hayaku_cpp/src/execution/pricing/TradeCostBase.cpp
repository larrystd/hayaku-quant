/*
 * TradeCostBase.cpp
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#include <execution/pricing/TradeCostBase.h>

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::TradeCostBase)
#endif

namespace hayaku {

HAYAKU_API std::ostream& operator<<(std::ostream& os, const TradeCostBase& tc) {
  os << "TradeCostFunc(" << tc.name() << ", " << tc.getParameter() << ")";
  return os;
}

HAYAKU_API std::ostream& operator<<(std::ostream& os, const TradeCostPtr& tc) {
  if (tc) {
    os << *tc;
  } else {
    os << "TradeCostFunc(NULL)";
  }
  return os;
}

TradeCostBase::TradeCostBase(const string& name) : name_(name) {}

TradeCostBase::~TradeCostBase() {}

void TradeCostBase::baseCheckParam(const string& name) const {}
void TradeCostBase::paramChanged() {}

TradeCostPtr TradeCostBase::clone() {
  TradeCostPtr result = _clone();
  TradeCostBase* p = result.get();
  p->params_ = params_;
  p->name_ = name_;
  p->is_python_object_ = is_python_object_;
  return result;
}

} /* namespace hayaku */
