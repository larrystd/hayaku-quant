#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-13
 *      Author: fasiondog
 */

#include "ScoresFilterBase.h"

namespace hayaku {

class GroupSCFilter : public ScoresFilterBase {
  SCORESFILTER_IMP(GroupSCFilter)
  SCORESFILTER_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  GroupSCFilter();
  virtual ~GroupSCFilter() override = default;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku
