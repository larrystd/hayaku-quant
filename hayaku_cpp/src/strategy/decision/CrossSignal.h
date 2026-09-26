#pragma once

/*
 * CrossSignal.h
 *
 *  Created on: 2015-2-20
 *      Author: fasiondog
 */

#include "SignalBase.h"
#include "operators/Indicator.h"

namespace hayaku {

class CrossSignal : public SignalBase {
 public:
  CrossSignal();
  CrossSignal(const Indicator& fast, const Indicator& slow);
  virtual ~CrossSignal();

  virtual SignalPtr _clone() override;
  virtual void _calculate(const KData& kdata) override;

 private:
  Indicator fast_;
  Indicator slow_;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);
    ar& boost::serialization::make_nvp("m_fast", fast_);
    ar& boost::serialization::make_nvp("m_slow", slow_);
  }
#endif
};

} /* namespace hayaku */
