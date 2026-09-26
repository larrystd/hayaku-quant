#pragma once

/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-19
 *      Author: fasiondog
 */

#include "SelectorBase.h"

namespace hayaku {

class SignalSelector : public SelectorBase {
  SELECTOR_IMP(SignalSelector)
  SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  SignalSelector();
  virtual ~SignalSelector();

  virtual void _reset() override { sys_dict_.clear(); }

 private:
  unordered_map<Datetime, StrategyWeightList> sys_dict_;
};

}  // namespace hayaku
