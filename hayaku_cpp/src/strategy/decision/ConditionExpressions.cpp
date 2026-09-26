#include "ConditionExpressions.h"

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::AddCondition)
#endif

namespace hayaku {

AddCondition::AddCondition() : ConditionBase("CN_Add") {}

AddCondition::AddCondition(const ConditionPtr& cond1, const ConditionPtr& cond2)
    : ConditionBase("CN_Add") {
  if (cond1) {
    cond1_ = cond1->clone();
  }
  if (cond2) {
    cond2_ = cond2->clone();
  }
}

AddCondition::~AddCondition() {}

void AddCondition::_calculate() {
  HAYAKU_IF_RETURN(!cond1_ && !cond2_, void());

  if (cond1_) {
    cond1_->setAccount(account_);
    cond1_->setSG(sg_);
    cond1_->setTO(kdata_);
  }

  if (cond2_) {
    cond2_->setAccount(account_);
    cond2_->setSG(sg_);
    cond2_->setTO(kdata_);
  }

  if (cond1_ && !cond2_) {
    price_t const* data = cond1_->data();
    for (size_t i = 0, total = cond1_->size(); i < total; i++) {
      values_[i] = data[i];
    }
    return;
  }

  if (!cond1_ && cond2_) {
    auto const* data = cond2_->data();
    for (size_t i = 0, total = cond2_->size(); i < total; i++) {
      values_[i] = data[i];
    }
    return;
  }

  size_t total = kdata_.size();
  HAYAKU_ASSERT(cond1_->size() == total && cond2_->size() == total);

  auto const* data1 = cond1_->data();
  auto const* data2 = cond2_->data();
  for (size_t i = 0; i < total; i++) {
    values_[i] = data1[i] + data2[i];
  }
}

void AddCondition::_reset() {
  if (cond1_) {
    cond1_->reset();
  }
  if (cond2_) {
    cond2_->reset();
  }
}

ConditionPtr AddCondition::_clone() {
  auto p = make_shared<AddCondition>();
  if (cond1_) {
    p->cond1_ = cond1_->clone();
  }
  if (cond2_) {
    p->cond2_ = cond2_->clone();
  }
  return p;
}

ConditionPtr operator+(const ConditionPtr& cond1, const ConditionPtr& cond2) {
  return make_shared<AddCondition>(cond1, cond2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-02-16
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::AndCondition)
#endif

namespace hayaku {

AndCondition::AndCondition() : ConditionBase("CN_And") {}

AndCondition::AndCondition(const ConditionPtr& cond1, const ConditionPtr& cond2)
    : ConditionBase("CN_And") {
  if (cond1) {
    cond1_ = cond1->clone();
  }
  if (cond2) {
    cond2_ = cond2->clone();
  }
}

AndCondition::~AndCondition() {}

void AndCondition::_calculate() {
  HAYAKU_IF_RETURN(!cond1_ || !cond2_, void());
  cond1_->setAccount(account_);
  cond2_->setAccount(account_);
  cond1_->setSG(sg_);
  cond2_->setSG(sg_);
  cond1_->setTO(kdata_);
  cond2_->setTO(kdata_);

  size_t total = kdata_.size();
  HAYAKU_ASSERT(cond1_->size() == total && cond2_->size() == total);

  auto const* data1 = cond1_->data();
  auto const* data2 = cond2_->data();
  for (size_t i = 0; i < total; i++) {
    values_[i] = (data1[i] > 0.0 && data2[i] > 0.0) ? 1.0 : 0.0;
  }
}

void AndCondition::_reset() {
  if (cond1_) {
    cond1_->reset();
  }
  if (cond2_) {
    cond2_->reset();
  }
}

ConditionPtr AndCondition::_clone() {
  auto p = make_shared<AndCondition>();
  if (cond1_) {
    p->cond1_ = cond1_->clone();
  }
  if (cond2_) {
    p->cond2_ = cond2_->clone();
  }
  return p;
}

ConditionPtr operator&(const ConditionPtr& cond1, const ConditionPtr& cond2) {
  return make_shared<AndCondition>(cond1, cond2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::DivCondition)
#endif

namespace hayaku {

DivCondition::DivCondition() : ConditionBase("CN_Div") {}

DivCondition::DivCondition(const ConditionPtr& cond1, const ConditionPtr& cond2)
    : ConditionBase("CN_Div") {
  if (cond1) {
    cond1_ = cond1->clone();
  }
  if (cond2) {
    cond2_ = cond2->clone();
  }
}

DivCondition::~DivCondition() {}

void DivCondition::_calculate() {
  HAYAKU_IF_RETURN(!cond1_, void());

  cond1_->setAccount(account_);
  cond1_->setSG(sg_);
  cond1_->setTO(kdata_);

  price_t null_price = Null<price_t>();
  if (!cond2_) {
    for (size_t i = 0, total = cond1_->size(); i < total; i++) {
      values_[i] = null_price;
    }
    return;
  }

  cond2_->setAccount(account_);
  cond2_->setSG(sg_);
  cond2_->setTO(kdata_);

  size_t total = kdata_.size();
  HAYAKU_ASSERT(cond1_->size() == total && cond2_->size() == total);

  auto const* data1 = cond1_->data();
  auto const* data2 = cond2_->data();
  for (size_t i = 0; i < total; i++) {
    values_[i] = data2[i] == 0.0 || std::isnan(data2[i]) ? null_price
                                                         : data1[i] / data2[i];
  }
}

void DivCondition::_reset() {
  if (cond1_) {
    cond1_->reset();
  }
  if (cond2_) {
    cond2_->reset();
  }
}

ConditionPtr DivCondition::_clone() {
  auto p = make_shared<DivCondition>();
  if (cond1_) {
    p->cond1_ = cond1_->clone();
  }
  if (cond2_) {
    p->cond2_ = cond2_->clone();
  }
  return p;
}

ConditionPtr operator/(const ConditionPtr& cond1, const ConditionPtr& cond2) {
  return make_shared<DivCondition>(cond1, cond2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::MultiCondition)
#endif

namespace hayaku {

MultiCondition::MultiCondition() : ConditionBase("CN_Multi") {}

MultiCondition::MultiCondition(const ConditionPtr& cond1,
                               const ConditionPtr& cond2)
    : ConditionBase("CN_Multi") {
  if (cond1) {
    cond1_ = cond1->clone();
  }
  if (cond2) {
    cond2_ = cond2->clone();
  }
}

MultiCondition::~MultiCondition() {}

void MultiCondition::_calculate() {
  HAYAKU_IF_RETURN(!cond1_ || !cond2_, void());

  cond1_->setAccount(account_);
  cond2_->setAccount(account_);
  cond1_->setSG(sg_);
  cond2_->setSG(sg_);
  cond1_->setTO(kdata_);
  cond2_->setTO(kdata_);

  size_t total = kdata_.size();
  HAYAKU_ASSERT(cond1_->size() == total && cond2_->size() == total);

  auto const* data1 = cond1_->data();
  auto const* data2 = cond2_->data();
  for (size_t i = 0; i < total; i++) {
    values_[i] = data1[i] * data2[i];
  }
}

void MultiCondition::_reset() {
  if (cond1_) {
    cond1_->reset();
  }
  if (cond2_) {
    cond2_->reset();
  }
}

ConditionPtr MultiCondition::_clone() {
  auto p = make_shared<MultiCondition>();
  if (cond1_) {
    p->cond1_ = cond1_->clone();
  }
  if (cond2_) {
    p->cond2_ = cond2_->clone();
  }
  return p;
}

ConditionPtr operator*(const ConditionPtr& cond1, const ConditionPtr& cond2) {
  return make_shared<MultiCondition>(cond1, cond2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OrCondition)
#endif

namespace hayaku {

OrCondition::OrCondition() : ConditionBase("CN_Or") {}

OrCondition::OrCondition(const ConditionPtr& cond1, const ConditionPtr& cond2)
    : ConditionBase("CN_Or") {
  if (cond1) {
    cond1_ = cond1->clone();
  }
  if (cond2) {
    cond2_ = cond2->clone();
  }
}

OrCondition::~OrCondition() {}

void OrCondition::_calculate() {
  HAYAKU_IF_RETURN(!cond1_ && !cond2_, void());

  if (cond1_) {
    cond1_->setAccount(account_);
    cond1_->setSG(sg_);
    cond1_->setTO(kdata_);
  }

  if (cond2_) {
    cond2_->setAccount(account_);
    cond2_->setSG(sg_);
    cond2_->setTO(kdata_);
  }

  if (cond1_ && !cond2_) {
    auto const* data = cond1_->data();
    for (size_t i = 0, total = cond1_->size(); i < total; i++) {
      if (data[i] > 0.0) {
        values_[i] = 1.0;
      }
    }
    return;
  }

  if (!cond1_ && cond2_) {
    auto const* data = cond2_->data();
    for (size_t i = 0, total = cond2_->size(); i < total; i++) {
      if (data[i] > 0.0) {
        values_[i] = 1.0;
      }
    }
    return;
  }

  size_t total = kdata_.size();
  HAYAKU_ASSERT(cond1_->size() == total && cond2_->size() == total);

  auto const* data1 = cond1_->data();
  auto const* data2 = cond2_->data();
  for (size_t i = 0; i < total; i++) {
    if (data1[i] > 0. || data2[i] > 0.) {
      values_[i] = 1.0;
    }
  }
}

void OrCondition::_reset() {
  if (cond1_) {
    cond1_->reset();
  }
  if (cond2_) {
    cond2_->reset();
  }
}

ConditionPtr OrCondition::_clone() {
  auto p = make_shared<OrCondition>();
  if (cond1_) {
    p->cond1_ = cond1_->clone();
  }
  if (cond2_) {
    p->cond2_ = cond2_->clone();
  }
  return p;
}

ConditionPtr operator|(const ConditionPtr& cond1, const ConditionPtr& cond2) {
  return make_shared<OrCondition>(cond1, cond2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::SubCondition)
#endif

namespace hayaku {

SubCondition::SubCondition() : ConditionBase("CN_Sub") {}

SubCondition::SubCondition(const ConditionPtr& cond1, const ConditionPtr& cond2)
    : ConditionBase("CN_Sub") {
  if (cond1) {
    cond1_ = cond1->clone();
  }
  if (cond2) {
    cond2_ = cond2->clone();
  }
}

SubCondition::~SubCondition() {}

void SubCondition::_calculate() {
  HAYAKU_IF_RETURN(!cond1_ && !cond2_, void());

  if (cond1_) {
    cond1_->setAccount(account_);
    cond1_->setSG(sg_);
    cond1_->setTO(kdata_);
  }

  if (cond2_) {
    cond2_->setAccount(account_);
    cond2_->setSG(sg_);
    cond2_->setTO(kdata_);
  }

  if (cond1_ && !cond2_) {
    auto const* data = cond1_->data();
    for (size_t i = 0, total = cond1_->size(); i < total; i++) {
      values_[i] = data[i];
    }
    return;
  }

  if (!cond1_ && cond2_) {
    auto const* data = cond2_->data();
    for (size_t i = 0, total = cond2_->size(); i < total; i++) {
      values_[i] = -data[i];
    }
    return;
  }

  size_t total = kdata_.size();
  HAYAKU_ASSERT(cond1_->size() == total && cond2_->size() == total);

  auto const* data1 = cond1_->data();
  auto const* data2 = cond2_->data();
  for (size_t i = 0; i < total; i++) {
    values_[i] = data1[i] - data2[i];
  }
}

void SubCondition::_reset() {
  if (cond1_) {
    cond1_->reset();
  }
  if (cond2_) {
    cond2_->reset();
  }
}

ConditionPtr SubCondition::_clone() {
  auto p = make_shared<SubCondition>();
  if (cond1_) {
    p->cond1_ = cond1_->clone();
  }
  if (cond2_) {
    p->cond2_ = cond2_->clone();
  }
  return p;
}

ConditionPtr operator-(const ConditionPtr& cond1, const ConditionPtr& cond2) {
  return make_shared<SubCondition>(cond1, cond2);
}

}  // namespace hayaku
