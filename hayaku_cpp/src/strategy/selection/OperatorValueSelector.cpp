/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#include "OperatorValueSelector.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OperatorValueSelector)
#endif

namespace hayaku {

OperatorValueSelector::OperatorValueSelector()
    : SelectorBase("SE_OpearatorValue") {}

OperatorValueSelector::OperatorValueSelector(const string& name)
    : SelectorBase(name) {}

OperatorValueSelector::OperatorValueSelector(const string& name,
                                             const SelectorPtr& se,
                                             double value)
    : SelectorBase(name), se_(se), value_(value) {
  if (se_) {
    pro_sys_list_ = se_->getProtoSystemList();
  }
}

OperatorValueSelector::~OperatorValueSelector() {}

void OperatorValueSelector::_reset() {
  if (se_) {
    se_->reset();
  }
}

bool OperatorValueSelector::isMatchAF(const AFPtr& af) { return true; }

SelectorPtr OperatorValueSelector::_clone() {
  auto p = make_shared<OperatorValueSelector>();
  if (se_) {
    p->se_ = se_->clone();
  }
  p->value_ = value_;
  return p;
}

void OperatorValueSelector::_calculate() {
  if (se_) {
    se_->calculate(real_sys_list_, query_);
  }
}

}  // namespace hayaku
