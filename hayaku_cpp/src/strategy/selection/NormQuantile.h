#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#include "NormalizeBase.h"

namespace hayaku {

/*
 * Quantile uniform distribution standardization
 */
class NormQuantile : public NormalizeBase {
  NORMALIZE_IMP(NormQuantile)
  NORMALIZE_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  NormQuantile();
  NormQuantile(double quantile_min, double quantile_max);
  virtual ~NormQuantile() override;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku
