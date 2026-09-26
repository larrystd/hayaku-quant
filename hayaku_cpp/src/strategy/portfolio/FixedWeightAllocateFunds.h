#pragma once

/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2018-2-8
 *      Author: fasiondog
 */

#include "AllocateFundsBase.h"

namespace hayaku {

class FixedWeightAllocateFunds : public AllocateFundsBase {
  ALLOCATEFUNDS_IMP(FixedWeightAllocateFunds)
  ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  FixedWeightAllocateFunds();
  virtual ~FixedWeightAllocateFunds();
  virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */
