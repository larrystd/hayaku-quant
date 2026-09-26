#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-13
 *      Author: fasiondog
 */

#include "ScoresFilterBase.h"

namespace hayaku {

class HAYAKU_API PriceSCFilter : public ScoresFilterBase {
  SCORESFILTER_IMP(PriceSCFilter)
  SCORESFILTER_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  PriceSCFilter();
  virtual ~PriceSCFilter() override = default;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku
