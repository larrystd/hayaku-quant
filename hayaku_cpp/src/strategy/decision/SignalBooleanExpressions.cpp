#include "SignalExpressions.h"

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::AndSignal)
#endif

namespace hayaku {

void AndSignal::_calculate(const KData& kdata) {
  HAYAKU_IF_RETURN(!sg1_ || !sg2_, void());

  auto const* ks = kdata.data();
  size_t total = kdata.size();

  sub_sg_calculate(sg1_, kdata);
  sub_sg_calculate(sg2_, kdata);
  for (size_t i = 0; i < total; ++i) {
    double buy_value =
        sg1_->getBuyValue(ks[i].datetime) * sg2_->getBuyValue(ks[i].datetime);
    double sell_value = 0.0 - sg1_->getSellValue(ks[i].datetime) *
                                  sg2_->getSellValue(ks[i].datetime);
    auto value = buy_value + sell_value;
    if (value > 0.0) {
      _addBuySignal(ks[i].datetime);
    } else if (value < 0.0) {
      _addSellSignal(ks[i].datetime);
    }
  }
}

HAYAKU_API SignalPtr operator&(const SignalPtr& sg1, const SignalPtr& sg2) {
  return make_shared<AndSignal>(sg1, sg2);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OrSignal)
#endif

namespace hayaku {

void OrSignal::_calculate(const KData& kdata) {
  HAYAKU_IF_RETURN(!sg1_ && !sg2_, void());

  auto const* ks = kdata.data();
  size_t total = kdata.size();

  if (sg1_ && !sg2_) {
    sub_sg_calculate(sg1_, kdata);
    for (size_t i = 0; i < total; ++i) {
      auto value = sg1_->getValue(ks[i].datetime);
      if (value > 0.0) {
        _addBuySignal(ks[i].datetime);
      } else if (value < 0.0) {
        _addSellSignal(ks[i].datetime);
      }
    }
    return;
  }

  if (!sg1_ && sg2_) {
    sub_sg_calculate(sg2_, kdata);
    for (size_t i = 0; i < total; i++) {
      auto value = sg2_->getValue(ks[i].datetime);
      if (value > 0.0) {
        _addBuySignal(ks[i].datetime);
      } else if (value < 0.0) {
        _addSellSignal(ks[i].datetime);
      }
    }
    return;
  }

  sub_sg_calculate(sg1_, kdata);
  sub_sg_calculate(sg2_, kdata);
  for (size_t i = 0; i < total; ++i) {
    double value =
        sg1_->getValue(ks[i].datetime) + sg2_->getValue(ks[i].datetime);
    if (value > 0.0) {
      _addBuySignal(ks[i].datetime);
    } else if (value < 0.0) {
      _addSellSignal(ks[i].datetime);
    }
  }
}

HAYAKU_API SignalPtr operator|(const SignalPtr& sg1, const SignalPtr& sg2) {
  return make_shared<OrSignal>(sg1, sg2);
}

} /* namespace hayaku */
