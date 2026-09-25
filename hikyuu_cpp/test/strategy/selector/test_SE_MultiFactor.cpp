/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-06-09
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <data/internal/DataRuntime.h>
#include <strategy/selector/crt/SE_MultiFactor.h>
#include <strategy/selector/imp/MultiFactorSelector.h>
#include <strategy/signal/crt/SG_Cycle.h>
#include <strategy/moneymanager/crt/MM_Nothing.h>
#include "create_test_strategy.h"
#include <data/indicator/crt/KDATA.h>
#include <data/indicator/crt/MA.h>
#include <data/indicator/crt/AMA.h>
#include <data/indicator/crt/EMA.h>

using namespace hku;

/**
 * @defgroup test_SE_MultiFactor test_SE_MultiFactor
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_SE_MultiFactor") {
    DataRuntime& sm = getDataRuntime();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQueryByDate(Datetime(20110712) - Days(30), Datetime(20111206));
    IndicatorList src_inds{MA(CLOSE())};

    auto sys = create_test_strategy(MM_Nothing(), SG_Cycle());
    sys->setParam<bool>("buy_delay", false);

    /** @arg Test trying to change a parameter into an illegal value */
    auto ret = SE_MultiFactor(src_inds);
    CHECK_THROWS(ret->setParam<int>("ic_n", 0));
    CHECK_THROWS(ret->setParam<int>("ic_rolling_n", 0));
    CHECK_THROWS(ret->setParam<string>("mode", "MF"));

    /** @arg src_inds is empty and the others are the default parameters */
    CHECK_THROWS(SE_MultiFactor(IndicatorList{}));

    /** @arg The default parameters */
    ret = SE_MultiFactor(src_inds, 10, 5, 120, ref_stk, "MF_ICIRWeight");
    ret->addStockList(stks, sys);
    auto proto_list = ret->getProtoSystemList();
    ret->calculate(proto_list, query);
    auto sw_list = ret->getSelected(Datetime(20110712));
    CHECK_EQ(sw_list.size(), 3);

    ret = SE_MultiFactor(src_inds, 10, 5, 120, ref_stk, "MF_EqualWeight");
    ret->addStockList(stks, sys);
    proto_list = ret->getProtoSystemList();
    ret->calculate(proto_list, query);
    sw_list = ret->getSelected(Datetime(20110712));
    CHECK_EQ(sw_list.size(), 3);

    ret = SE_MultiFactor(src_inds, 10, 5, 120, ref_stk, "MF_ICWeight");
    ret->addStockList(stks, sys);
    proto_list = ret->getProtoSystemList();
    ret->calculate(proto_list, query);
    sw_list = ret->getSelected(Datetime(20110712));
    CHECK_EQ(sw_list.size(), 3);

    /** @arg topn = 2 */
    ret = SE_MultiFactor(src_inds, 2, 5, 120, ref_stk);
    ret->addStockList(stks, sys);
    proto_list = ret->getProtoSystemList();
    ret->calculate(proto_list, query);
    sw_list = ret->getSelected(Datetime(20110712));
    CHECK_EQ(sw_list.size(), 2);

    // for (const auto& sw : sw_list) {
    //     HKU_INFO("{} {}", sw.strategy->name(), sw.weight);
    // }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_SE_MultiFactor_export") {
    DataRuntime& sm = getDataRuntime();
    string filename(fmt::format("{}/SE_MultiFactor.xml", sm.tmpdir()));

    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQueryByDate(Datetime(20110712) - Days(30), Datetime(20111206));
    IndicatorList src_inds{MA(CLOSE())};

    auto sys = create_test_strategy(MM_Nothing(), SG_Cycle());
    sys->setParam<bool>("buy_delay", false);

    auto x1 = SE_MultiFactor(src_inds, 10, 5, 120, ref_stk);
    x1->addStockList(stks, sys);

    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(x1);
    }

    SelectorPtr x2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(x2);
    }

    CHECK_UNARY(x2->getProtoSystemList().empty());
    x2->addStockList(stks, sys);
    auto proto_list = x2->getProtoSystemList();
    x2->calculate(proto_list, query);
    auto sw_list = x2->getSelected(Datetime(20110712));
    CHECK_EQ(sw_list.size(), 3);
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
