#pragma once

/*
 * IndicatorStoploss.h
 *
 *  Created on: 2013-4-21
 *      Author: fasiondog
 */

#include "StoplossBase.h"
#include "operators/Indicator.h"

namespace hayaku {

class IndicatorStoploss : public StoplossBase {
 public:
  IndicatorStoploss();  // Default constructor used for the serialization only
  explicit IndicatorStoploss(const Indicator& ind);
  virtual ~IndicatorStoploss();

  virtual price_t getPrice(const Datetime& datetime, price_t price) override;
  virtual void _reset() override;
  virtual StoplossPtr _clone() override;
  virtual void _calculate() override;

 private:
  Indicator m_ind;
  map<Datetime, price_t> m_result;

//========================================
// Serialization support
//========================================
#if HAYAKU_SUPPORT_SERIALIZATION
 private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(StoplossBase);
    ar& BOOST_SERIALIZATION_NVP(m_ind);
    // m_result needs to be recalculated every time the system runs, it is not
    // saved ar & BOOST_SERIALIZATION_NVP(m_result);
  }
#endif /* HAYAKU_SUPPORT_SERIALIZATION */
};

} /* namespace hayaku */
