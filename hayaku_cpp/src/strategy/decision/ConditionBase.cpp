/*
 * ConditionBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "ConditionBase.h"

#include "operators/SeriesOperators.h"

namespace hayaku {

HAYAKU_API std::ostream& operator<<(std::ostream& os, const ConditionBase& cn) {
  os << "Condition(" << cn.name() << ", " << cn.getParameter() << ")";
  return os;
}

HAYAKU_API std::ostream& operator<<(std::ostream& os, const ConditionPtr& cn) {
  if (cn) {
    os << *cn;
  } else {
    os << "Condition(NULL)";
  }
  return os;
}

ConditionBase::ConditionBase() : name_("ConditionBase") {}

ConditionBase::ConditionBase(const string& name) : name_(name) {}

ConditionBase::~ConditionBase() {}

void ConditionBase::baseCheckParam(const string& name) const {}
void ConditionBase::paramChanged() {}

void ConditionBase::reset() {
  kdata_ = Null<KData>();
  account_.reset();
  sg_.reset();
  date_index_.clear();
  values_.clear();
  _reset();
}

ConditionPtr ConditionBase::clone() {
  ConditionPtr p;
  try {
    p = _clone();
  } catch (...) {
    HAYAKU_ERROR("Subclass _clone failed!");
    p = ConditionPtr();
  }

  if (!p || p.get() == this) {
    HAYAKU_ERROR("Failed clone! Will use self-ptr!");
    return shared_from_this();
  }

  p->params_ = params_;
  p->name_ = name_;
  p->is_python_object_ = is_python_object_;
  p->kdata_ = kdata_;
  p->date_index_ = date_index_;
  p->values_ = values_;

  // tm and sg are set by the system at runtime, they are not cloned
  // The account is injected by StrategyRuntime for each run.
  // p->m_sg = m_sg->clone();
  return p;
}

void ConditionBase::setTO(const KData& kdata) {
  HAYAKU_IF_RETURN(kdata == kdata_, void());
  kdata_ = kdata;
  if (!kdata.empty()) {
    date_index_.clear();
    size_t total = kdata.size();
    values_.resize(total);
    auto const* ks = kdata_.data();
    for (size_t i = 0; i < total; i++) {
      values_[i] = 0.0;
      date_index_[ks[i].datetime] = i;
    }
    _calculate();
  }
}

void ConditionBase::_addValid(const Datetime& datetime, price_t value) {
  auto iter = date_index_.find(datetime);
  HAYAKU_IF_RETURN(iter == date_index_.end(), void());
  values_[iter->second] += value;
}

bool ConditionBase::isValid(const Datetime& datetime) {
  auto iter = date_index_.find(datetime);
  HAYAKU_IF_RETURN(iter == date_index_.end(), false);
  return values_[iter->second] > 0.;
}

DatetimeList ConditionBase::getDatetimeList() const {
  DatetimeList result;
  for (const auto& d : date_index_) {
    if (values_[d.second] > 0.0) {
      result.emplace_back(d.first);
    }
  }
  return result;
}

Indicator ConditionBase::getValues() const {
  DatetimeList dates;
  PriceList values;
  for (const auto& d : date_index_) {
    dates.push_back(d.first);
  }

  values.reserve(dates.size());
  for (const auto& d : dates) {
    values.push_back(values_[date_index_.at(d)]);
  }

  return PRICELIST(values, dates);
}

} /* namespace hayaku */
