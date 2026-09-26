/*
 * TwoLineEnviroment.cpp
 *
 *  Created on: 2016-5-17
 *      Author: Administrator
 */

#include "data/DataRuntime.h"
#include "operators/SeriesOperators.h"
#include "TwoLineEnvironment.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::TwoLineEnvironment)
#endif

namespace hayaku {

TwoLineEnvironment::TwoLineEnvironment() : EnvironmentBase("EV_TwoLine") {
    setParam<string>("market", "SH");
}

TwoLineEnvironment::TwoLineEnvironment(const Indicator& fast, const Indicator& slow)
: EnvironmentBase("EV_TwoLine"), m_fast(fast), m_slow(slow) {
    setParam<string>("market", "SH");
}

TwoLineEnvironment::~TwoLineEnvironment() {}

void TwoLineEnvironment::_checkParam(const string& name) const {
    if ("market" == name) {
        string market = getParam<string>(name);
        auto market_info = getDataRuntime().getMarketInfo(market);
        HAYAKU_CHECK(market_info != Null<MarketInfo>(), "Invalid market: {}", market);
    }
}

EnvironmentPtr TwoLineEnvironment::_clone() {
    auto ptr = make_shared<TwoLineEnvironment>();
    ptr->m_fast = m_fast.clone();
    ptr->m_slow = m_slow.clone();
    return ptr;
}

void TwoLineEnvironment::_calculate() {
    string market = getParam<string>("market");
    const auto& sm = getDataRuntime();
    MarketInfo market_info = sm.getMarketInfo(market);
    HAYAKU_IF_RETURN(market_info == Null<MarketInfo>(), void());

    Stock stock = sm.getStock(market + market_info.code());
    KData kdata = stock.getKData(m_query);
    Indicator close = CLOSE(kdata);
    Indicator fast = m_fast(close);
    Indicator slow = m_slow(close);

    size_t total = close.size();
    size_t start = fast.discard() > slow.discard() ? fast.discard() : slow.discard();
    auto const* fast_data = fast.data();
    auto const* slow_data = slow.data();
    auto const* ks = kdata.data();
    for (size_t i = start; i < total; i++) {
        if (fast_data[i] > slow_data[i]) {
            _addValid(ks[i].datetime);
        }
    }
}

EVPtr HAYAKU_API EV_TwoLine(const Indicator& fast, const Indicator& slow, const string& market) {
    EVPtr ptr = make_shared<TwoLineEnvironment>(fast, slow);
    ptr->setParam<string>("market", market);
    return ptr;
}

} /* namespace hayaku */
