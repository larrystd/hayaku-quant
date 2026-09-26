/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-03
 *      Author: fasiondog
 */

#include "Indicator2InImp.h"

#include "operators/SeriesOperators.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::Indicator2InImp)
#endif

namespace hayaku {

Indicator2InImp::Indicator2InImp() : IndicatorImp("Indicator2InImp") {
  need_self_alike_compare_ = true;
  setParam<bool>("fill_null", true);
}

Indicator2InImp::Indicator2InImp(const string& name, size_t result_num)
    : IndicatorImp(name, result_num) {
  need_self_alike_compare_ = true;
  setParam<bool>("fill_null", true);
}

Indicator2InImp::Indicator2InImp(const string& name, const Indicator& ref_ind,
                                 bool fill_null, size_t result_num)
    : IndicatorImp(name, result_num), ref_ind_(ref_ind) {
  need_self_alike_compare_ = true;
  setParam<bool>("fill_null", fill_null);
}

Indicator2InImp::~Indicator2InImp() {}

IndicatorImpPtr Indicator2InImp::_clone() {
  auto p = make_shared<Indicator2InImp>();
  p->ref_ind_ = ref_ind_.clone();
  return p;
}

bool Indicator2InImp::selfAlike(const IndicatorImp& other) const noexcept {
  const auto* other_ctx = dynamic_cast<const Indicator2InImp*>(&other);
  HAYAKU_IF_RETURN(other_ctx == nullptr, false);
  return ref_ind_.getImp()->alike(*(other_ctx->ref_ind_.getImp()));
}

void Indicator2InImp::getSelfInnerNodesWithInputConext(
    vector<IndicatorImpPtr>& nodes) const {
  vector<IndicatorImpPtr> self_nodes;
  ref_ind_.getImp()->getAllSubNodes(self_nodes);
  nodes.emplace_back(ref_ind_.getImp());
  for (auto& node : self_nodes) {
    nodes.emplace_back(node);
  }
}

Indicator Indicator2InImp::prepare(const Indicator& ind) {
  bool is_value = false;
  const auto& k = getContext();
  if (k != Null<KData>()) {
    ref_ind_.setContext(k);
  } else {
    const auto& ind_k = ind.getContext();
    const auto& ref_k = ref_ind_.getContext();
    if (ref_k == Null<KData>() || ind_k.getStock() == ref_k.getStock()) {
      ref_ind_.setContext(ind_k);
    } else {
      is_value = true;
    }
  }

  Indicator ref = ref_ind_;
  auto dates = ref.getDatetimeList();
  if (is_value || dates.empty()) {
    // If it is not a time series, take ind as the reference and align at the
    // right end, filling with nan when it is shorter and truncating the left
    // end when it is longer
    if (ref.size() > ind.size()) {
      ref = SLICE(ref, ref.size() - ind.size(), ref.size());
    } else if (ref.size() < ind.size()) {
      ref = CVAL(ind, 0.) + ref;
      if (ref.size() < ind.size()) {
        ref = ref(ind.getContext());
      }
    }
  } else if (k != ind.getContext()) {
    // If it is a time series and the contexts of the two are different, align
    // them by date
    ref = ALIGN(ref_ind_, ind, getParam<bool>("fill_null"));
  }

  return ref;
}

}  // namespace hayaku
