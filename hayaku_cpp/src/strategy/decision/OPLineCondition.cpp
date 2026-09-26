/*
 * OPCondition.cpp
 *
 *  Created on: 2016-5-9
 *      Author: Administrator
 */

#include "OPLineCondition.h"

#include "execution/AccountConfig.h"
#include "execution/ExecutionAccountFactory.h"
#include "execution/pricing/TradeCosts.h"
#include "operators/SeriesOperators.h"
#include "strategy/StrategyDefinition.h"
#include "strategy/StrategyRuntime.h"
#include "strategy/risk/MoneyManagers.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OPLineCondition)
#endif

namespace hayaku {

OPLineCondition::OPLineCondition() : ConditionBase("CN_OPLine") {}

OPLineCondition::OPLineCondition(const Indicator& op)
    : ConditionBase("CN_OPLine"), op_(op) {}

OPLineCondition::~OPLineCondition() {}

ConditionPtr OPLineCondition::_clone() {
  return make_shared<OPLineCondition>(op_.clone());
}

void OPLineCondition::_calculate() {
  Stock stock = kdata_.getStock();
  KQuery query = kdata_.getQuery();
  MMPtr mm = MM_FixedCount(stock.minTradeNumber());
  mm->setParam<bool>("auto-checkin", true);
  Parameter parameters;
  parameters.set<bool>("buy_delay", true);
  parameters.set<bool>("sell_delay", true);
  auto account = internal::makeExecutionAccount(
      AccountConfig(kdata_[0].datetime, 0.0, TC_Zero(), "CN_OPLine"));
  internal::StrategyRuntime runtime(
      StrategyDefinition(mm, sg_, "CN_OPLine", {}, {}, {}, {}, {}, {},
                         parameters),
      account);
  runtime.run(kdata_.getStock(), kdata_.getQuery());
  KQuery::KType ktype = query.kType();
  DatetimeList dates = kdata_.getDatetimeList();
  Indicator profit = PRICELIST(account->getProfitCurve(dates, ktype));
  Indicator op = op_(profit);

  Indicator x = profit - op;
  auto const* xdata = x.data();
  for (size_t i = 0; i < x.size(); i++) {
    if (xdata[i] > 0) {
      _addValid(dates[i]);
    }
  }
}

CNPtr HAYAKU_API CN_OPLine(const Indicator& op) {
  return make_shared<OPLineCondition>(op);
}

} /* namespace hayaku */
