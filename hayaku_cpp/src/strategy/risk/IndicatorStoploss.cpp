/*
 * IndicatorStoploss.cpp
 *
 *  Created on: 2013-4-21
 *      Author: fasiondog
 */

#include "IndicatorStoploss.h"

#include "operators/SeriesOperators.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IndicatorStoploss)
#endif

namespace hayaku {

IndicatorStoploss::IndicatorStoploss() : StoplossBase("ST_Indicator") {}

IndicatorStoploss::IndicatorStoploss(const Indicator& op)
    : StoplossBase("ST_Indicator"), ind_(op) {}

IndicatorStoploss::~IndicatorStoploss() {}

price_t IndicatorStoploss::getPrice(const Datetime& datetime, price_t price) {
  return result_.count(datetime) ? result_[datetime] : 0.0;
}

void IndicatorStoploss::_reset() { result_.clear(); }

StoplossPtr IndicatorStoploss::_clone() {
  auto p = make_shared<IndicatorStoploss>();
  p->ind_ = ind_;
  p->result_ = result_;
  return p;
}

void IndicatorStoploss::_calculate() {
  Indicator ind = ind_(kdata_);
  size_t total = ind.size();
  auto const* ind_data = ind.data();
  auto const* ks = kdata_.data();
  for (size_t i = ind.discard(); i < total; ++i) {
    result_[ks[i].datetime] = ind_data[i];
  }
}

StoplossPtr HAYAKU_API ST_Indicator(const Indicator& ind) {
  return make_shared<IndicatorStoploss>(ind);
}

} /* namespace hayaku */
