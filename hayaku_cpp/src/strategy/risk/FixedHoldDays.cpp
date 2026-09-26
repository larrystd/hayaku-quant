/*
 * FixedHoldDays.cpp
 *
 *  Created on: 2018-1-20
 *      Author: fasiondog
 */

#include "FixedHoldDays.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::FixedHoldDays)
#endif

namespace hayaku {

FixedHoldDays::FixedHoldDays() : ProfitGoalBase("PG_FixedHoldDays") {
  setParam<int>("days", 5);
}

FixedHoldDays::~FixedHoldDays() {}

void FixedHoldDays::_checkParam(const string& name) const {
  if ("days" == name) {
    int days = getParam<int>(name);
    HAYAKU_ASSERT(days > 0);
  }
}

price_t FixedHoldDays::getGoal(const Datetime& datetime, price_t price) {
  Stock stk = kdata_.getStock();
  PositionRecord position = account_->getPosition(datetime, stk);
  Datetime take_date = position.takeDatetime;

  KQuery query = KQueryByDate(Datetime(take_date.date()),
                              Datetime(datetime.date()), KQuery::DAY);

  size_t start_out, end_out;
  if (stk.getIndexRange(query, start_out, end_out)) {
    size_t d = end_out - start_out;
    if (d >= getParam<int>("days")) {
      return 0.0;
    }
  }

  return Null<price_t>();
}

ProfitGoalPtr PG_FixedHoldDays(int days) {
  ProfitGoalPtr ptr = make_shared<FixedHoldDays>();
  ptr->setParam<int>("days", days);
  return ptr;
}

} /* namespace hayaku */
