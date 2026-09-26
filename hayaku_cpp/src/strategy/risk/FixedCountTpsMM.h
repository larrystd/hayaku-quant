#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-20
 *      Author: fasiondog
 */

#include "MoneyManagerBase.h"

namespace hayaku {

class FixedCountTpsMM : public MoneyManagerBase {
 public:
  FixedCountTpsMM();
  FixedCountTpsMM(const vector<double>& buy_counts,
                  const vector<double>& sell_counts);
  virtual ~FixedCountTpsMM();

  virtual MoneyManagerPtr _clone() override;
  virtual double _getBuyNumber(const Datetime& datetime, const Stock& stock,
                               price_t price, price_t risk,
                               OrderOrigin origin) override;
  virtual double _getSellNumber(const Datetime& datetime, const Stock& stock,
                                price_t price, price_t risk,
                                OrderOrigin origin) override;

 private:
  vector<double> buy_counts_;
  vector<double> sell_counts_;

#if HAYAKU_SUPPORT_SERIALIZATION
 private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(MoneyManagerBase);
  }
#endif
};

} /* namespace hayaku */
