#pragma once

/*
 * FixedPercentStoploss.h
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#include "StoplossBase.h"

namespace hayaku {

/**
 * Fixed percentage stop-loss strategy, i.e. the stop-loss is triggered when the
 * price is lower than the buy price by a certain percentage
 */
class FixedPercentStoploss : public StoplossBase {
  STOPLOSS_IMP(FixedPercentStoploss, "FixedPercentSL")
  STOPLOSS_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  FixedPercentStoploss();
  virtual ~FixedPercentStoploss();
  virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */
