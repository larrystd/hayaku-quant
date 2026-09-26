/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-02-03
 *      Author: fasiondog
 */

#include "BoolEnvironment.h"

#include "data/DataRuntime.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::BoolEnvironment)
#endif

namespace hayaku {

BoolEnvironment::BoolEnvironment() : EnvironmentBase("EV_Bool") {
  setParam<string>("market", "SH");
}

BoolEnvironment::BoolEnvironment(const Indicator& ind)
    : EnvironmentBase("EV_Bool"), ind_(ind) {
  setParam<string>("market", "SH");
}

BoolEnvironment::~BoolEnvironment() {}

void BoolEnvironment::_checkParam(const string& name) const {
  if ("market" == name) {
    string market = getParam<string>(name);
    auto market_info = getDataRuntime().getMarketInfo(market);
    HAYAKU_CHECK(market_info != Null<MarketInfo>(), "Invalid market: {}",
                 market);
  }
}

EnvironmentPtr BoolEnvironment::_clone() {
  return make_shared<BoolEnvironment>(ind_.clone());
}

void BoolEnvironment::_calculate() {
  string market = getParam<string>("market");
  const auto& sm = getDataRuntime();
  MarketInfo market_info = sm.getMarketInfo(market);
  HAYAKU_ERROR_IF_RETURN(market_info == Null<MarketInfo>(), void(),
                         "Can't find maket({}) info!", market);

  Stock stock = sm.getStock(market + market_info.code());
  KData kdata = stock.getKData(query_);

  auto ds = kdata.getDatetimeList();
  ind_.setContext(kdata);
  auto const* ind_data = ind_.data();
  for (size_t i = ind_.discard(), len = ind_.size(); i < len; i++) {
    if (!std::isnan(ind_data[i]) && ind_data[i] > 0.) {
      _addValid(ds[i]);
    }
  }
}

EVPtr EV_Bool(const Indicator& ind, const string& market) {
  EVPtr p = make_shared<BoolEnvironment>(ind);
  p->setParam<string>("market", market);
  return p;
}

}  // namespace hayaku
