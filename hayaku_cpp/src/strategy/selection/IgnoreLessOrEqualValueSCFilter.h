#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-14
 *      Author: fasiondog
 */

#include "ScoresFilterBase.h"

namespace hayaku {

class HAYAKU_API IgnoreLessOrEqualValueSCFilter : public ScoresFilterBase {
  SCORESFILTER_IMP(IgnoreLessOrEqualValueSCFilter)
  SCORESFILTER_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IgnoreLessOrEqualValueSCFilter();
  virtual ~IgnoreLessOrEqualValueSCFilter() override = default;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku
