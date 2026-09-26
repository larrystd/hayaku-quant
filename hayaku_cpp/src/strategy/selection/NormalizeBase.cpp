/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#include "NormalizeBase.h"

namespace hayaku {

std::ostream& operator<<(std::ostream& out, const NormalizeBase& norm) {
  out << "Normalize{" << "\n  name: " << norm.name()
      << "\n  params: " << norm.getParameter();
  out << "\n}";
  return out;
}

std::ostream& operator<<(std::ostream& out, const NormalizePtr& norm) {
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
  HAYAKU_ERROR_IF(!p, "Failed clone! {}", name_);

  p->name_ = name_;
  p->params_ = params_;
  p->is_python_object_ = is_python_object_;
  return p;
}

}  // namespace hayaku
