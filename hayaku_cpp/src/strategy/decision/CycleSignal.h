#pragma once

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-01
 *      Author: fasiondog
 */

#include "SignalBase.h"

namespace hayaku {

class CycleSignal : public SignalBase {
  SIGNAL_IMP(CycleSignal)
  SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  CycleSignal();
  virtual ~CycleSignal() = default;

  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku
