#pragma once

/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-02
 *      Author: fasiondog
 */

#include "IndicatorImp.h"

namespace hayaku {

class HAYAKU_API Indicator;
class HAYAKU_API IndicatorImp;

/**
 * @brief Dynamic indicator parameter, used as the parameter of the other
 * indicators
 * @ingroup Indicator
 */
class HAYAKU_API IndParam {
  HAYAKU_API friend std::ostream& operator<<(std::ostream&, const IndParam&);

 public:
  IndParam();
  explicit IndParam(const IndicatorImpPtr& ind);
  explicit IndParam(const Indicator& ind);

  IndicatorImpPtr getImp() const noexcept { return ind_; }

  Indicator get() const;

 private:
  IndicatorImpPtr ind_;
};

}  // namespace hayaku
