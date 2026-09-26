#pragma once

/*
 * EqualWeightAllocateFunds.h
 *
 *  Created on: 2018-2-8
 *      Author: fasiondog
 */

#include "AllocateFundsBase.h"

namespace hayaku {

class EqualWeightAllocateFunds : public AllocateFundsBase {
  ALLOCATEFUNDS_IMP(EqualWeightAllocateFunds)
  ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  EqualWeightAllocateFunds();
  virtual ~EqualWeightAllocateFunds();
};

} /* namespace hayaku */
