/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-20
 *      Author: fasiondog
 */

#include "FixedCountTpsMM.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::FixedCountTpsMM)
#endif

namespace hayaku {

FixedCountTpsMM::FixedCountTpsMM() : MoneyManagerBase("MM_FixedCountTpsMM") {}

FixedCountTpsMM::FixedCountTpsMM(const vector<double>& buy_counts,
                                 const vector<double>& sell_counts)
    : MoneyManagerBase("MM_FixedCountTpsMM"),
      buy_counts_(buy_counts),
      sell_counts_(sell_counts) {
  double total_buy_count = 0.0;
  for (size_t i = 0, total = buy_counts.size(); i < total; i++) {
    HAYAKU_CHECK(buy_counts[i] >= 0.0, "buy_counts[{}] must >= 0.0!", i);
    total_buy_count += buy_counts[i];
  }

  double total_sell_count = 0.0;
  for (size_t i = 0, total = sell_counts.size(); i < total; i++) {
    HAYAKU_CHECK(sell_counts[i] >= 0.0, "sell_counts[{}] must >= 0.0!", i);
    total_sell_count += sell_counts[i];
  }

  HAYAKU_WARN_IF(
      total_buy_count != total_sell_count,
      "The total number of buy ({}) and the total number ({}) of sell are not "
      "consistent, which may lead to an imbalance.",
      total_buy_count, total_sell_count);
}

FixedCountTpsMM::~FixedCountTpsMM() {}

MoneyManagerPtr FixedCountTpsMM::_clone() {
  auto p = make_shared<FixedCountTpsMM>();
  p->buy_counts_ = buy_counts_;
  p->sell_counts_ = sell_counts_;
  return p;
}

double FixedCountTpsMM::_getBuyNumber(const Datetime& datetime,
                                      const Stock& stock, price_t price,
                                      price_t risk, OrderOrigin origin) {
  size_t current_buy_count = currentBuyCount(stock);
  return (current_buy_count < buy_counts_.size())
             ? buy_counts_[current_buy_count]
             : 0.0;
}

double FixedCountTpsMM::_getSellNumber(const Datetime& datetime,
                                       const Stock& stock, price_t price,
                                       price_t risk, OrderOrigin origin) {
  size_t current_sell_count = currentSellCount(stock);
  return (current_sell_count < sell_counts_.size())
             ? sell_counts_[current_sell_count]
             : 0.0;
}

MoneyManagerPtr HAYAKU_API MM_FixedCountTps(const vector<double>& buy_counts,
                                            const vector<double>& sell_counts) {
  return make_shared<FixedCountTpsMM>(buy_counts, sell_counts);
}

} /* namespace hayaku */
