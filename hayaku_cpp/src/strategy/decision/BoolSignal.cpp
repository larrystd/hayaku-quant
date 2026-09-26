/*
 * BoolSignal.cpp
 *
 *  Created on: 2017-7-2
 *      Author: fasiondog
 */

#include "BoolSignal.h"

#include "operators/SeriesOperators.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::BoolSignal)
#endif

namespace hayaku {

BoolSignal::BoolSignal() : SignalBase("SG_Bool") {}

BoolSignal::BoolSignal(const Indicator& buy, const Indicator& sell)
    : SignalBase("SG_Bool"), bool_buy_(buy), bool_sell_(sell) {}

BoolSignal::~BoolSignal() {}

SignalPtr BoolSignal::_clone() {
  auto p = make_shared<BoolSignal>();
  p->bool_buy_ = bool_buy_.clone();
  p->bool_sell_ = bool_sell_.clone();
  return p;
}

void BoolSignal::_calculate(const KData& kdata) {
  Indicator buy = ALIGN(bool_buy_(kdata), kdata);
  Indicator sell = ALIGN(bool_sell_(kdata), kdata);
  HAYAKU_ERROR_IF_RETURN(buy.size() != sell.size(), void(),
                         "buy.size() != sell.size()");

  size_t discard =
      buy.discard() > sell.discard() ? buy.discard() : sell.discard();
  size_t total = buy.size();
  auto const* buydata = buy.data();
  auto const* selldata = sell.data();
  auto const* ks = kdata.data();
  for (size_t i = discard; i < total; ++i) {
    if (buydata[i] > 0.0) _addBuySignal(ks[i].datetime);
    if (selldata[i] > 0.0) _addSellSignal(ks[i].datetime);
  }
}

SignalPtr SG_Bool(const Indicator& buy, const Indicator& sell, bool alternate) {
  auto p = make_shared<BoolSignal>(buy, sell);
  p->setParam<bool>("alternate", alternate);
  return p;
}

} /* namespace hayaku */
