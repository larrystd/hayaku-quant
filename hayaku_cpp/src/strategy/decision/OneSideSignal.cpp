/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-13
 *      Author: fasiondog
 */

#include "OneSideSignal.h"

#include "operators/SeriesOperators.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OneSideSignal)
#endif

namespace hayaku {

OneSideSignal::OneSideSignal() : SignalBase("SG_OneSide") {
  setParam<bool>("alternate", false);
  setParam<bool>("is_buy",
                 true);  // A buy signal, otherwise a sell signal is added
}

OneSideSignal::OneSideSignal(const Indicator& ind, bool is_buy)
    : SignalBase("SG_OneSide"), m_ind(ind.clone()) {
  setParam<bool>("alternate", false);
  setParam<bool>("is_buy", is_buy);
}

OneSideSignal::~OneSideSignal() {}

void OneSideSignal::_checkParam(const string& name) const {
  if (name == "alternate") {
    HAYAKU_CHECK(!getParam<bool>(name), "alternate only be false!");
  }
}

SignalPtr OneSideSignal::_clone() {
  auto p = make_shared<OneSideSignal>();
  p->m_ind = m_ind.clone();
  return p;
}

void OneSideSignal::_calculate(const KData& kdata) {
  Indicator ind = m_ind(kdata);
  HAYAKU_IF_RETURN(ind.empty() || ind.size() != kdata.size(), void());

  bool is_buy = getParam<bool>("is_buy");
  const auto* src = ind.data();
  auto const* ks = kdata.data();
  size_t discard = ind.discard();
  size_t total = ind.size();

  for (size_t i = discard; i < total; ++i) {
    if (src[i] > 0.0) {
      if (is_buy) {
        _addBuySignal(ks[i].datetime);
      } else {
        _addSellSignal(ks[i].datetime);
      }
    }
  }
}

SignalPtr HAYAKU_API SG_OneSide(const Indicator& ind, bool is_buy) {
  return make_shared<OneSideSignal>(ind, is_buy);
}

} /* namespace hayaku */
