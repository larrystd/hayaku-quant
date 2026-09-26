#include "EnvironmentExpressions.h"

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::AddEnvironment)
#endif

namespace hayaku {

AddEnvironment::AddEnvironment() : EnvironmentBase("EV_Add") {}

AddEnvironment::AddEnvironment(const EnvironmentPtr& ev1,
                               const EnvironmentPtr& ev2)
    : EnvironmentBase("EV_Add") {
  if (ev1) {
    ev1_ = ev1->clone();
  }
  if (ev2) {
    ev2_ = ev2->clone();
  }
}

AddEnvironment::~AddEnvironment() {}

void AddEnvironment::_calculate() {
  HAYAKU_IF_RETURN(!ev1_ && !ev2_, void());

  if (ev1_) {
    ev1_->setQuery(query_);
  }

  if (ev2_) {
    ev2_->setQuery(query_);
  }

  if (ev1_ && !ev2_) {
    auto values = ev1_->getValues();
    auto dates = values.getDatetimeList();
    size_t total = dates.size();
    values_.resize(total);
    for (size_t i = 0; i < total; i++) {
      date_index_[dates[i]] = i;
      values_[i] = values[i];
    }
    return;
  }

  if (!ev1_ && ev2_) {
    auto values = ev2_->getValues();
    auto dates = values.getDatetimeList();
    size_t total = dates.size();
    values_.resize(total);
    for (size_t i = 0; i < total; i++) {
      date_index_[dates[i]] = i;
      values_[i] = values[i];
    }
    return;
  }

  auto values = ev1_->getValues();
  auto dates = values.getDatetimeList();
  size_t total = dates.size();
  values_.resize(total);
  for (size_t i = 0; i < total; i++) {
    date_index_[dates[i]] = i;
    values_[i] = values[i];
  }

  values = ev2_->getValues();
  dates = values.getDatetimeList();
  total = dates.size();
  const auto* src = values.data();
  for (size_t i = 0; i < total; i++) {
    _addValid(dates[i], src[i]);
  }
}

void AddEnvironment::_reset() {
  if (ev1_) {
    ev1_->reset();
  }
  if (ev2_) {
    ev2_->reset();
  }
}

EnvironmentPtr AddEnvironment::_clone() {
  auto p = make_shared<AddEnvironment>();
  if (ev1_) {
    p->ev1_ = ev1_->clone();
  }
  if (ev2_) {
    p->ev2_ = ev2_->clone();
  }
  return p;
}

HAYAKU_API EnvironmentPtr operator+(const EnvironmentPtr& ev1,
                                    const EnvironmentPtr& ev2) {
  return make_shared<AddEnvironment>(ev1, ev2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::AndEnvironment)
#endif

namespace hayaku {

AndEnvironment::AndEnvironment() : EnvironmentBase("EV_And") {}

AndEnvironment::AndEnvironment(const EnvironmentPtr& ev1,
                               const EnvironmentPtr& ev2)
    : EnvironmentBase("EV_And") {
  if (ev1) {
    ev1_ = ev1->clone();
  }
  if (ev2) {
    ev2_ = ev2->clone();
  }
}

AndEnvironment::~AndEnvironment() {}

void AndEnvironment::_calculate() {
  HAYAKU_IF_RETURN(!ev1_ || !ev2_, void());

  ev1_->setQuery(query_);
  ev2_->setQuery(query_);

  auto values1 = ev1_->getValues();
  auto dates1 = values1.getDatetimeList();
  unordered_map<Datetime, price_t> value1_map;
  const auto* src1 = values1.data();
  for (size_t i = 0, total = dates1.size(); i < total; i++) {
    if (src1[i] > 0.0) {
      value1_map[dates1[i]] = 1.0;
    }
  }

  auto value2 = ev2_->getValues();
  auto dates2 = value2.getDatetimeList();
  const auto* src2 = value2.data();
  for (size_t i = 0, total = dates2.size(); i < total; i++) {
    if (src2[i] > 0.0) {
      auto iter = value1_map.find(dates2[i]);
      if (iter != value1_map.end()) {
        _addValid(iter->first, 1.0);
      }
    }
  }
}

void AndEnvironment::_reset() {
  if (ev1_) {
    ev1_->reset();
  }
  if (ev2_) {
    ev2_->reset();
  }
}

EnvironmentPtr AndEnvironment::_clone() {
  auto p = make_shared<AndEnvironment>();
  if (ev1_) {
    p->ev1_ = ev1_->clone();
  }
  if (ev2_) {
    p->ev2_ = ev2_->clone();
  }
  return p;
}

HAYAKU_API EnvironmentPtr operator&(const EnvironmentPtr& ev1,
                                    const EnvironmentPtr& ev2) {
  return make_shared<AndEnvironment>(ev1, ev2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::DivEnvironment)
#endif

namespace hayaku {

DivEnvironment::DivEnvironment() : EnvironmentBase("EV_Div") {}

DivEnvironment::DivEnvironment(const EnvironmentPtr& ev1,
                               const EnvironmentPtr& ev2)
    : EnvironmentBase("EV_Div") {
  if (ev1) {
    ev1_ = ev1->clone();
  }
  if (ev2) {
    ev2_ = ev2->clone();
  }
}

DivEnvironment::~DivEnvironment() {}

void DivEnvironment::_calculate() {
  HAYAKU_IF_RETURN(!ev1_ || !ev2_, void());

  ev1_->setQuery(query_);
  ev2_->setQuery(query_);

  auto values1 = ev1_->getValues();
  auto dates1 = values1.getDatetimeList();
  unordered_map<Datetime, price_t> value1_map;
  const auto* src1 = values1.data();
  for (size_t i = 0, total = dates1.size(); i < total; i++) {
    if (src1[i] != 0.0) {
      value1_map[dates1[i]] = src1[i];
    }
  }

  auto value2 = ev2_->getValues();
  auto dates2 = value2.getDatetimeList();
  const auto* src2 = value2.data();
  for (size_t i = 0, total = dates2.size(); i < total; i++) {
    if (src2[i] != 0.0) {
      auto iter = value1_map.find(dates2[i]);
      if (iter != value1_map.end()) {
        _addValid(iter->first, iter->second / src2[i]);
      }
    }
  }
}

void DivEnvironment::_reset() {
  if (ev1_) {
    ev1_->reset();
  }
  if (ev2_) {
    ev2_->reset();
  }
}

EnvironmentPtr DivEnvironment::_clone() {
  auto p = make_shared<DivEnvironment>();
  if (ev1_) {
    p->ev1_ = ev1_->clone();
  }
  if (ev2_) {
    p->ev2_ = ev2_->clone();
  }
  return p;
}

HAYAKU_API EnvironmentPtr operator/(const EnvironmentPtr& ev1,
                                    const EnvironmentPtr& ev2) {
  return make_shared<DivEnvironment>(ev1, ev2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::MultiEnvironment)
#endif

namespace hayaku {

MultiEnvironment::MultiEnvironment() : EnvironmentBase("EV_Multi") {}

MultiEnvironment::MultiEnvironment(const EnvironmentPtr& ev1,
                                   const EnvironmentPtr& ev2)
    : EnvironmentBase("EV_Multi") {
  if (ev1) {
    ev1_ = ev1->clone();
  }
  if (ev2) {
    ev2_ = ev2->clone();
  }
}

MultiEnvironment::~MultiEnvironment() {}

void MultiEnvironment::_calculate() {
  HAYAKU_IF_RETURN(!ev1_ || !ev2_, void());

  ev1_->setQuery(query_);
  ev2_->setQuery(query_);

  auto values1 = ev1_->getValues();
  auto dates1 = values1.getDatetimeList();
  unordered_map<Datetime, price_t> value1_map;
  const auto* src1 = values1.data();
  for (size_t i = 0, total = dates1.size(); i < total; i++) {
    if (src1[i] != 0.0) {
      value1_map[dates1[i]] = src1[i];
    }
  }

  auto value2 = ev2_->getValues();
  auto dates2 = value2.getDatetimeList();
  const auto* src2 = value2.data();
  for (size_t i = 0, total = dates2.size(); i < total; i++) {
    if (src2[i] != 0.0) {
      auto iter = value1_map.find(dates2[i]);
      if (iter != value1_map.end()) {
        _addValid(iter->first, iter->second * src2[i]);
      }
    }
  }
}

void MultiEnvironment::_reset() {
  if (ev1_) {
    ev1_->reset();
  }
  if (ev2_) {
    ev2_->reset();
  }
}

EnvironmentPtr MultiEnvironment::_clone() {
  auto p = make_shared<MultiEnvironment>();
  if (ev1_) {
    p->ev1_ = ev1_->clone();
  }
  if (ev2_) {
    p->ev2_ = ev2_->clone();
  }
  return p;
}

HAYAKU_API EnvironmentPtr operator*(const EnvironmentPtr& ev1,
                                    const EnvironmentPtr& ev2) {
  return make_shared<MultiEnvironment>(ev1, ev2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OrEnvironment)
#endif

namespace hayaku {

OrEnvironment::OrEnvironment() : EnvironmentBase("EV_Or") {}

OrEnvironment::OrEnvironment(const EnvironmentPtr& ev1,
                             const EnvironmentPtr& ev2)
    : EnvironmentBase("EV_Or") {
  if (ev1) {
    ev1_ = ev1->clone();
  }
  if (ev2) {
    ev2_ = ev2->clone();
  }
}

OrEnvironment::~OrEnvironment() {}

void OrEnvironment::_calculate() {
  HAYAKU_IF_RETURN(!ev1_ && !ev2_, void());

  if (ev1_) {
    ev1_->setQuery(query_);
  }

  if (ev2_) {
    ev2_->setQuery(query_);
  }

  if (ev1_ && !ev2_) {
    auto values = ev1_->getValues();
    auto dates = values.getDatetimeList();
    size_t total = dates.size();
    values_.reserve(total);
    for (size_t i = 0; i < total; i++) {
      if (values[i] > 0.0) {
        date_index_[dates[i]] = values_.size();
        values_.push_back(1.0);
      }
    }
    return;
  }

  if (!ev1_ && ev2_) {
    auto values = ev2_->getValues();
    auto dates = values.getDatetimeList();
    size_t total = dates.size();
    values_.reserve(total);
    for (size_t i = 0; i < total; i++) {
      if (values[i] > 0.0) {
        date_index_[dates[i]] = values_.size();
        values_.push_back(1.0);
      }
    }
    return;
  }

  auto values = ev1_->getValues();
  auto dates = values.getDatetimeList();
  size_t total = dates.size();
  for (size_t i = 0; i < total; i++) {
    if (values[i] > 0.0) {
      date_index_[dates[i]] = values_.size();
      values_.push_back(1.0);
    }
  }

  values = ev2_->getValues();
  dates = values.getDatetimeList();
  total = dates.size();
  for (size_t i = 0; i < total; i++) {
    if (values[i] > 0.0) {
      auto iter = date_index_.find(dates[i]);
      if (iter == date_index_.end()) {
        date_index_[dates[i]] = values_.size();
        values_.push_back(1.0);
      }
    }
  }
}

void OrEnvironment::_reset() {
  if (ev1_) {
    ev1_->reset();
  }
  if (ev2_) {
    ev2_->reset();
  }
}

EnvironmentPtr OrEnvironment::_clone() {
  auto p = make_shared<OrEnvironment>();
  if (ev1_) {
    p->ev1_ = ev1_->clone();
  }
  if (ev2_) {
    p->ev2_ = ev2_->clone();
  }
  return p;
}

HAYAKU_API EnvironmentPtr operator|(const EnvironmentPtr& ev1,
                                    const EnvironmentPtr& ev2) {
  return make_shared<OrEnvironment>(ev1, ev2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::SubEnvironment)
#endif

namespace hayaku {

SubEnvironment::SubEnvironment() : EnvironmentBase("EV_Sub") {}

SubEnvironment::SubEnvironment(const EnvironmentPtr& ev1,
                               const EnvironmentPtr& ev2)
    : EnvironmentBase("EV_Sub") {
  if (ev1) {
    ev1_ = ev1->clone();
  }
  if (ev2) {
    ev2_ = ev2->clone();
  }
}

SubEnvironment::~SubEnvironment() {}

void SubEnvironment::_calculate() {
  HAYAKU_IF_RETURN(!ev1_ && !ev2_, void());

  if (ev1_) {
    ev1_->setQuery(query_);
  }

  if (ev2_) {
    ev2_->setQuery(query_);
  }

  if (ev1_ && !ev2_) {
    auto values = ev1_->getValues();
    auto dates = values.getDatetimeList();
    size_t total = dates.size();
    values_.resize(total);
    for (size_t i = 0; i < total; i++) {
      date_index_[dates[i]] = i;
      values_[i] = values[i];
    }
    return;
  }

  if (!ev1_ && ev2_) {
    auto values = ev2_->getValues();
    auto dates = values.getDatetimeList();
    size_t total = dates.size();
    values_.resize(total);
    for (size_t i = 0; i < total; i++) {
      date_index_[dates[i]] = i;
      values_[i] = -values[i];
    }
    return;
  }

  auto values = ev1_->getValues();
  auto dates = values.getDatetimeList();
  size_t total = dates.size();
  values_.resize(total);
  for (size_t i = 0; i < total; i++) {
    date_index_[dates[i]] = i;
    values_[i] = values[i];
  }

  values = ev2_->getValues();
  dates = values.getDatetimeList();
  total = dates.size();
  const auto* src = values.data();
  for (size_t i = 0; i < total; i++) {
    _addValid(dates[i], -src[i]);
  }
}

void SubEnvironment::_reset() {
  if (ev1_) {
    ev1_->reset();
  }
  if (ev2_) {
    ev2_->reset();
  }
}

EnvironmentPtr SubEnvironment::_clone() {
  auto p = make_shared<SubEnvironment>();
  if (ev1_) {
    p->ev1_ = ev1_->clone();
  }
  if (ev2_) {
    p->ev2_ = ev2_->clone();
  }
  return p;
}

HAYAKU_API EnvironmentPtr operator-(const EnvironmentPtr& ev1,
                                    const EnvironmentPtr& ev2) {
  return make_shared<SubEnvironment>(ev1, ev2);
}

}  // namespace hayaku
