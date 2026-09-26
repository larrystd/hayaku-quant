#pragma once

/*
 * BandSignal2.h
 *
 *   Created on: 2023-09-23
 *       Author: yangrq1018
 */

#include "SignalBase.h"
#include "operators/Indicator.h"

namespace hayaku {

class BandSignal2 : public SignalBase {
 public:
  BandSignal2();
  BandSignal2(const Indicator& sig, const Indicator& lower,
              const Indicator& upper);
  virtual ~BandSignal2();

  virtual SignalPtr _clone() override;
  virtual void _calculate(const KData& kdata) override;

 private:
  Indicator ind_;
  Indicator lower_;
  Indicator upper_;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);
    ar& boost::serialization::make_nvp("m_ind", ind_);
    ar& boost::serialization::make_nvp("m_lower", lower_);
    ar& boost::serialization::make_nvp("m_upper", upper_);
  }
#endif
};
}  // namespace hayaku
