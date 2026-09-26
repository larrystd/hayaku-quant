#pragma once

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#include "SignalBase.h"

namespace hayaku {

class ManualSignal : public SignalBase {
  SIGNAL_IMP(ManualSignal)
  SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ManualSignal();
  virtual ~ManualSignal() = default;
};

}  // namespace hayaku
