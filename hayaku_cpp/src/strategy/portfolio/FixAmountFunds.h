#pragma once

#include "AllocateFundsBase.h"

namespace hayaku {

class FixedAmountFunds : public AllocateFundsBase {
  ALLOCATEFUNDS_IMP(FixedAmountFunds)
  ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  FixedAmountFunds();
  virtual ~FixedAmountFunds();
  virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */
