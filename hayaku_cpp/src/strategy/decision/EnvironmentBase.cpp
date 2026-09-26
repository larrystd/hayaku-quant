/*
 * Environment.cpp
 *
 *  Created on: 2013-2-28
 *      Author: fasiondog
 */

#include "EnvironmentBase.h"

#include "operators/SeriesOperators.h"

namespace hayaku {

std::ostream& operator<<(std::ostream& os, const EnvironmentBase& en) {
  os << "Environment(" << en.name() << " " << en.getParameter() << ")";
  return os;
}

std::ostream& operator<<(std::ostream& os, const EnvironmentPtr& en) {
  if (en) {
    os << *en;
  } else {
    os << "Environment(NULL)";
  }
  return os;
}

EnvironmentBase::EnvironmentBase() : name_("EnvironmentBase") {}

EnvironmentBase::EnvironmentBase(const EnvironmentBase& base)
    : params_(base.params_),
      name_(base.name_),
      query_(base.query_),
      date_index_(base.date_index_),
      values_(base.values_) {}

EnvironmentBase::EnvironmentBase(const string& name) : name_(name) {}

EnvironmentBase::~EnvironmentBase() {}

void EnvironmentBase::baseCheckParam(const string& name) const {}
void EnvironmentBase::paramChanged() {}

void EnvironmentBase::reset() {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  query_ = Null<KQuery>();
  date_index_.clear();
  values_.clear();
  _reset();
}

EnvironmentPtr EnvironmentBase::clone() {
  EnvironmentPtr p;
  try {
    p = _clone();
  } catch (...) {
    HAYAKU_ERROR("Subclass _clone failed!");
    p = EnvironmentPtr();
  }

  if (!p || p.get() == this) {
    HAYAKU_ERROR("Failed clone! Will use self-ptr!");
    return shared_from_this();
  }

  p->params_ = params_;
  p->name_ = name_;
  p->is_python_object_ = is_python_object_;
  p->query_ = query_;
  p->date_index_ = date_index_;
  p->values_ = values_;
  return p;
}

void EnvironmentBase::setQuery(const KQuery& query) {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  if (query_ != query) {
    query_ = Null<KQuery>();
    date_index_.clear();
    values_.clear();
    _reset();
    query_ = query;
    _calculate();
  }
}

void EnvironmentBase::_addValid(const Datetime& datetime, price_t value) {
  auto iter = date_index_.find(datetime);
  if (iter == date_index_.end()) {
    date_index_[datetime] = values_.size();
    values_.push_back(value);
  } else {
    values_[iter->second] += value;
  }
}

bool EnvironmentBase::isValid(const Datetime& datetime) const {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  auto iter = date_index_.find(datetime);
  HAYAKU_IF_RETURN(iter == date_index_.end(), false);
  return values_[iter->second] > 0.;
}

price_t EnvironmentBase::getValue(const Datetime& datetime) const {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  auto iter = date_index_.find(datetime);
  return iter == date_index_.end() ? 0. : values_[iter->second];
}

Indicator EnvironmentBase::getValues() const {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  DatetimeList dates;
  PriceList values;
  for (const auto& d : date_index_) {
    dates.emplace_back(d.first);
  }

  values.reserve(dates.size());
  for (const auto& d : dates) {
    values.emplace_back(values_[date_index_.at(d)]);
  }

  return PRICELIST(values, dates);
}

} /* namespace hayaku */
