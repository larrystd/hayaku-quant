#pragma once

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-22
 *      Author: fasiondog
 */

#include "OptimalSelectorBase.h"

namespace hayaku {

class MaxFundsOptimalSelector : public OptimalSelectorBase {
  OPTIMAL_SELECTOR_IMP(MaxFundsOptimalSelector)
  OPTIMAL_SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  MaxFundsOptimalSelector();
  virtual ~MaxFundsOptimalSelector();
};

}  // namespace hayaku
