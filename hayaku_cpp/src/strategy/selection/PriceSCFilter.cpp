/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-13
 *      Author: fasiondog
 */

#include "PriceSCFilter.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::PriceSCFilter)
#endif

namespace hayaku {

PriceSCFilter::PriceSCFilter() : ScoresFilterBase("SCFilter_Price") {
  setParam<double>("min_price", 5.0);       // Minimum price
  setParam<double>("max_price", 100000.0);  // Maximum price
}

void PriceSCFilter::_checkParam(const string& name) const {
  if (name == "min_price" || name == "max_price") {
    HAYAKU_ASSERT(!std::isnan(getParam<double>(name)));
  }
}

ScoreRecordList PriceSCFilter::_filter(const ScoreRecordList& scores,
                                       const Datetime& date,
                                       const KQuery& query) {
  ScoreRecordList ret;
  const auto& ktype = query.kType();

  double min_price = getParam<double>("min_price");
  double max_price = getParam<double>("max_price");

  for (size_t i = 0; i < scores.size(); ++i) {
    if (scores[i].stock.isNull()) {
      continue;
    }

    auto kr = scores[i].stock.getKRecord(date, ktype);
    if (!kr.isValid()) {
      continue;
    }

    if (kr.closePrice >= min_price && kr.closePrice <= max_price) {
      ret.emplace_back(scores[i]);
    }
  }

  return ret;
}

ScoresFilterPtr HAYAKU_API SCFilter_Price(double min_price, double max_price) {
  HAYAKU_CHECK(max_price > min_price, "max_price must > min_price!");
  auto p = std::make_shared<PriceSCFilter>();
  p->setParam<double>("min_price", min_price);
  p->setParam<double>("max_price", max_price);
  return p;
}

}  // namespace hayaku
