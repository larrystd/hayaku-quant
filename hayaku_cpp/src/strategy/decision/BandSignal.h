#pragma once

/*
 * BandSignal.h
 *
 *   Created on: 2023-09-23
 *       Author: yangrq1018
 */

#include "SignalBase.h"
#include "operators/Indicator.h"

namespace hayaku {

class BandSignal : public SignalBase {
 public:
  BandSignal();
  BandSignal(const Indicator& sig, price_t lower, price_t upper);
  virtual ~BandSignal();

  virtual SignalPtr _clone() override;
  virtual void _calculate(const KData& kdata) override;

 private:
  Indicator m_ind;
  price_t m_lower = 0.0, m_upper = 0.0;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);
    ar& BOOST_SERIALIZATION_NVP(m_ind);
    ar& BOOST_SERIALIZATION_NVP(m_lower);
    ar& BOOST_SERIALIZATION_NVP(m_upper);
  }
#endif
};
}  // namespace hayaku
