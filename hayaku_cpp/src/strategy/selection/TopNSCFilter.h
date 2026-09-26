#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-14
 *      Author: fasiondog
 */

#include "ScoresFilterBase.h"

namespace hayaku {

class TopNSCFilter : public ScoresFilterBase {
  SCORESFILTER_IMP(TopNSCFilter)
  SCORESFILTER_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  TopNSCFilter();
  virtual ~TopNSCFilter() override = default;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku
