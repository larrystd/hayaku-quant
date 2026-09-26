#pragma once

/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-21
 *      Author: fasiondog
 */

#include "ConditionBase.h"
#include "operators/Indicator.h"

namespace hayaku {

class BoolCondition : public ConditionBase {
 public:
  BoolCondition();
  explicit BoolCondition(const Indicator&);
  virtual ~BoolCondition();

  virtual void _calculate() override;
  virtual ConditionPtr _clone() override;

 private:
  Indicator m_ind;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
    ar& BOOST_SERIALIZATION_NVP(m_ind);
  }
#endif
};

}  // namespace hayaku
