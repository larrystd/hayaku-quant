#pragma once

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-02-03
 *      Author: fasiondog
 */

#include "operators/Indicator.h"
#include "strategy/decision/EnvironmentBase.h"

namespace hayaku {

class BoolEnvironment : public EnvironmentBase {
 public:
  BoolEnvironment();
  explicit BoolEnvironment(const Indicator& ind);
  virtual ~BoolEnvironment();

  virtual void _checkParam(const string& name) const override;
  virtual void _calculate() override;
  virtual EnvironmentPtr _clone() override;

 private:
  Indicator m_ind;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase);
    ar& BOOST_SERIALIZATION_NVP(m_ind);
  }
#endif
};

}  // namespace hayaku
