#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-13
 *      Author: fasiondog
 */

#include "ScoresFilterBase.h"

namespace hayaku {

class HAYAKU_API MinAmountPercentSCFilter : public ScoresFilterBase {
  SCORESFILTER_IMP(MinAmountPercentSCFilter)
  SCORESFILTER_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  MinAmountPercentSCFilter();
  virtual ~MinAmountPercentSCFilter() override = default;

  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku
