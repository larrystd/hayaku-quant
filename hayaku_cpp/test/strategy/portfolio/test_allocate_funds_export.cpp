/*
 * test_export.cpp
 *
 *  Created on: 2018-2-10
 *      Author: fasiondog
 */

#include <config.h>

#include "doctest/doctest.h"

#if HAYAKU_SUPPORT_SERIALIZATION

#include <data/DataRuntime.h>
#include <strategy/portfolio/AllocationPolicies.h>

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <fstream>

using namespace hayaku;

/**
 * @defgroup test_allocatefunds_serialization test_allocatefunds_serialization
 * @ingroup test_hayaku_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_AF_EqualWeight_export") {
  DataRuntime& sm = getDataRuntime();
  string filename(sm.tmpdir());
  filename += "/AF_EQUALWEIGHT.xml";

  AFPtr af1 = AF_EqualWeight();
  {
    std::ofstream ofs(filename);
    boost::archive::xml_oarchive oa(ofs);
    oa << BOOST_SERIALIZATION_NVP(af1);
  }

  AFPtr af2;
  {
    std::ifstream ifs(filename);
    boost::archive::xml_iarchive ia(ifs);
    ia >> BOOST_SERIALIZATION_NVP(af2);
  }

  CHECK_EQ(af1->name(), af2->name());
}

/** @} */

#endif /* HAYAKU_SUPPORT_SERIALIZATION */
