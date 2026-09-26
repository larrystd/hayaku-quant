/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-02
 *      Author: fasiondog
 */

#include "IndParam.h"

#include "Indicator.h"

namespace hayaku {

std::ostream& operator<<(std::ostream& os, const IndParam& ind) {
  os << "IndParam: \n" << ind.ind_->formula();
  return os;
}

IndParam::IndParam() {}

IndParam::IndParam(const IndicatorImpPtr& ind) : ind_(ind) {};

IndParam::IndParam(const Indicator& ind) : ind_(ind.getImp()) {};

Indicator IndParam::get() const { return Indicator(ind_); }

}  // namespace hayaku
