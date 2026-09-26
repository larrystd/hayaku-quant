#pragma once

/*
 * SingleSignal.h
 *
 *  Created on: 2015-2-22
 *      Author: fasiondog
 */

#include "SignalBase.h"
#include "operators/Indicator.h"

namespace hayaku {

class SingleSignal : public SignalBase {
 public:
  SingleSignal();
  explicit SingleSignal(const Indicator& ind);
  virtual ~SingleSignal();

  virtual void _checkParam(const string& name) const override;
  virtual SignalPtr _clone() override;
  virtual void _calculate(const KData& kdata) override;

 private:
  Indicator m_ind;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);
    ar& BOOST_SERIALIZATION_NVP(m_ind);
  }
#endif
};

} /* namespace hayaku */
