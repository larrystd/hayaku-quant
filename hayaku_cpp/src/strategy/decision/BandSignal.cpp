/*
 * BandSignal.cpp
 *
 *   Created on: 2023-09-23
 *       Author: yangrq1018
 */
#include "BandSignal.h"

#include "operators/SeriesOperators.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::BandSignal)
#endif

namespace hayaku {

BandSignal::BandSignal() : SignalBase("SG_Band") {}

BandSignal::BandSignal(const Indicator& ind, price_t lower, price_t upper)
    : SignalBase("SG_Band"), ind_(ind.clone()), lower_(lower), upper_(upper) {
  HAYAKU_CHECK(lower < upper,
               "BandSignal: lower track is greater than upper track");
}

BandSignal::~BandSignal() {}

SignalPtr BandSignal::_clone() {
  auto p = make_shared<BandSignal>();
  p->upper_ = upper_;
  p->lower_ = lower_;
  p->ind_ = ind_.clone();
  return p;
}

void BandSignal::_calculate(const KData& kdata) {
  Indicator ind = ind_(kdata);
  size_t discard = ind.discard();
  size_t total = ind.size();

  auto const* inddata = ind.data();
  auto const* ks = kdata.data();
  for (size_t i = discard; i < total; ++i) {
    if (inddata[i] > upper_) {
      _addBuySignal(ks[i].datetime);
    } else if (inddata[i] < lower_) {
      _addSellSignal(ks[i].datetime);
    }
  }
}

SignalPtr SG_Band(const Indicator& sig, price_t lower, price_t upper) {
  return make_shared<BandSignal>(sig, lower, upper);
}

}  // namespace hayaku
