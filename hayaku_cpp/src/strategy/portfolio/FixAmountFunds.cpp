

#include "FixAmountFunds.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::FixedAmountFunds)
#endif

namespace hayaku {

FixedAmountFunds::FixedAmountFunds() : AllocateFundsBase("AF_FixedAmount") {
  setParam<double>("amount", 20000);

  // The common parameter must be set to false, the automatic weight adjustment
  // is forbidden
  setParam<bool>("auto_adjust_weight", false);
}

FixedAmountFunds::~FixedAmountFunds() {}

void FixedAmountFunds::_checkParam(const string& name) const {
  if ("amount" == name) {
    double amount = getParam<double>("amount");
    HAYAKU_ASSERT(amount > 500.0);
  } else if ("auto_adjust_weight" == name) {
    bool auto_adjust_weight = getParam<bool>("auto_adjust_weight");
    HAYAKU_CHECK(!auto_adjust_weight,
                 R"(param "auto_adjust_weight" must be false!)");
  }
}

StrategyWeightList FixedAmountFunds ::_allocateWeight(
    const Datetime& date, const StrategyWeightList& se_list) {
  const auto& q = getQuery();
  FundsRecord funds = getAccount()->getFunds(
      date, q.kType());  // The total assets come from the total account
  price_t total_funds = funds.cash + funds.market_value + funds.borrow_asset -
                        funds.short_market_value;

  price_t t_cash = funds.cash;
  StrategyWeightList result;
  price_t amount = getParam<double>("amount");
  for (auto iter = se_list.begin(); iter != se_list.end(); ++iter) {
    if (t_cash <= 0.6 * amount) {
      break;
    }

    double w = amount / total_funds;
    result.emplace_back(iter->strategy, w);
    t_cash -= amount;
  }

  return result;
}

AFPtr AF_FixedAmount(double amount) {
  auto p = make_shared<FixedAmountFunds>();
  p->setParam<double>("amount", amount);
  return p;
}

} /* namespace hayaku */
