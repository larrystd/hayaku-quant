#pragma once

/*
 * BoolSignal.h
 *
 *  Created on: 2017-7-2
 *      Author: fasiondog
 */

#include "SignalBase.h"
#include "operators/Indicator.h"

namespace hayaku {

class BoolSignal : public SignalBase {
 public:
  BoolSignal();
  BoolSignal(const Indicator& buy, const Indicator& sell);
  virtual ~BoolSignal();

  virtual SignalPtr _clone() override;
  virtual void _calculate(const KData& kdata) override;

 private:
  Indicator m_bool_buy;
  Indicator m_bool_sell;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);
    ar& BOOST_SERIALIZATION_NVP(m_bool_buy);
    ar& BOOST_SERIALIZATION_NVP(m_bool_sell);
  }
#endif
};

} /* namespace hayaku */
