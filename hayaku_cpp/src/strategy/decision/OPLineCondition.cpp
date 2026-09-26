/*
 * OPCondition.cpp
 *
 *  Created on: 2016-5-9
 *      Author: Administrator
 */

#include "OPLineCondition.h"
#include "operators/SeriesOperators.h"
#include "execution/AccountConfig.h"
#include "execution/pricing/TradeCosts.h"
#include "execution/ExecutionAccountFactory.h"
#include "strategy/StrategyDefinition.h"
#include "strategy/risk/MoneyManagers.h"
#include "strategy/StrategyRuntime.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OPLineCondition)
#endif

namespace hayaku {

OPLineCondition::OPLineCondition() : ConditionBase("CN_OPLine") {}

OPLineCondition::OPLineCondition(const Indicator& op) : ConditionBase("CN_OPLine"), m_op(op) {}

OPLineCondition::~OPLineCondition() {}

ConditionPtr OPLineCondition::_clone() {
    return make_shared<OPLineCondition>(m_op.clone());
}

void OPLineCondition::_calculate() {
    Stock stock = m_kdata.getStock();
    KQuery query = m_kdata.getQuery();
    MMPtr mm = MM_FixedCount(stock.minTradeNumber());
    mm->setParam<bool>("auto-checkin", true);
    Parameter parameters;
    parameters.set<bool>("buy_delay", true);
    parameters.set<bool>("sell_delay", true);
    auto account = internal::makeExecutionAccount(
      AccountConfig(m_kdata[0].datetime, 0.0, TC_Zero(), "CN_OPLine"));
    internal::StrategyRuntime runtime(
      StrategyDefinition(mm, m_sg, "CN_OPLine", {}, {}, {}, {}, {}, {}, parameters), account);
    runtime.run(m_kdata.getStock(), m_kdata.getQuery());
    KQuery::KType ktype = query.kType();
    DatetimeList dates = m_kdata.getDatetimeList();
    Indicator profit = PRICELIST(account->getProfitCurve(dates, ktype));
    Indicator op = m_op(profit);

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
