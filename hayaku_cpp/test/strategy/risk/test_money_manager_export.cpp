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
#include <strategy/risk/MoneyManagers.h>

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <fstream>

using namespace hayaku;

/**
 * @defgroup test_money_manager_serialization test_money_manager_serialization
 * @ingroup test_hayaku_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MM_FixedCount_export") {
  DataRuntime& sm = getDataRuntime();
  string filename(sm.tmpdir());
  filename += "/Fixed_MM.xml";

  MoneyManagerPtr mm1 = MM_FixedCount(100);
  {
    std::ofstream ofs(filename);
    boost::archive::xml_oarchive oa(ofs);
    oa << BOOST_SERIALIZATION_NVP(mm1);
  }

  MoneyManagerPtr mm2;
  {
    std::ifstream ifs(filename);
    boost::archive::xml_iarchive ia(ifs);
    ia >> BOOST_SERIALIZATION_NVP(mm2);
  }

  CHECK_EQ(mm1->name(), mm2->name());
}

/** @} */

#endif /* HAYAKU_SUPPORT_SERIALIZATION */
