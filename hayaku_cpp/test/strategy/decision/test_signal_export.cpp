/*
 * test_export.cpp
 *
 *  Created on: 2013-4-30
 *      Author: fasiondog
 */
#include <config.h>

#include "doctest/doctest.h"

#if HAYAKU_SUPPORT_SERIALIZATION

#include <data/DataRuntime.h>
#include <operators/SeriesOperators.h>
#include <operators/WindowOperators.h>
#include <strategy/decision/Signals.h>

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <fstream>

using namespace hayaku;

/**
 * @defgroup test_signal_serialization test_signal_serialization
 * @ingroup test_hayaku_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_SG_AMA_export") {
  DataRuntime& sm = getDataRuntime();
  string filename(sm.tmpdir());
  filename += "/SG_AMA.xml";

  SignalPtr sg1 = SG_Single(AMA(CLOSE()));
  auto k = getKData("sz000001", KQueryByIndex(-100));
  sg1->setTO(k);
  {
    std::ofstream ofs(filename);
    boost::archive::xml_oarchive oa(ofs);
    oa << BOOST_SERIALIZATION_NVP(sg1);
  }

  SignalPtr sg2;
  {
    std::ifstream ifs(filename);
    boost::archive::xml_iarchive ia(ifs);
    ia >> BOOST_SERIALIZATION_NVP(sg2);
  }

  CHECK_EQ(sg1->name(), sg2->name());
  CHECK_UNARY(sg2->getTO().empty());
}

/** @} */

#endif /* HAYAKU_SUPPORT_SERIALIZATION */
