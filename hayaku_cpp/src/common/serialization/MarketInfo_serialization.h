#pragma once

/*
 * MarketInfo.h
 *
 *  Created on: 2013-5-1
 *      Author: fasiondog
 */

#include "common/Config.h"
#include "data/MarketInfo.h"

#if HAYAKU_SUPPORT_SERIALIZATION
#include "TimeDelta_serialization.h"

namespace boost {
namespace serialization {

template <class Archive>
void save(Archive& ar, const hayaku::MarketInfo& record, unsigned int version) {
  hayaku::string market = record.market();
  hayaku::string name = record.name();
  hayaku::string description = record.description();
  hayaku::string code = record.code();
  hayaku::uint64_t lastDate = record.lastDate().number();
  hayaku::TimeDelta openTime1 = record.openTime1();
  hayaku::TimeDelta openTime2 = record.openTime2();
  hayaku::TimeDelta closeTime1 = record.closeTime1();
  hayaku::TimeDelta closeTime2 = record.closeTime2();
  ar& BOOST_SERIALIZATION_NVP(market);
  ar& BOOST_SERIALIZATION_NVP(name);
  ar& BOOST_SERIALIZATION_NVP(description);
  ar& BOOST_SERIALIZATION_NVP(code);
  ar& BOOST_SERIALIZATION_NVP(lastDate);
  ar& BOOST_SERIALIZATION_NVP(openTime1);
  ar& BOOST_SERIALIZATION_NVP(closeTime1);
  ar& BOOST_SERIALIZATION_NVP(openTime2);
  ar& BOOST_SERIALIZATION_NVP(closeTime2);
}

template <class Archive>
void load(Archive& ar, hayaku::MarketInfo& record, unsigned int version) {
  hayaku::string market, name, description, code;
  hayaku::uint64_t lastDate;
  hayaku::TimeDelta openTime1, openTime2, closeTime1, closeTime2;
  ar& BOOST_SERIALIZATION_NVP(market);
  ar& BOOST_SERIALIZATION_NVP(name);
  ar& BOOST_SERIALIZATION_NVP(description);
  ar& BOOST_SERIALIZATION_NVP(code);
  ar& BOOST_SERIALIZATION_NVP(lastDate);
  ar& BOOST_SERIALIZATION_NVP(openTime1);
  ar& BOOST_SERIALIZATION_NVP(closeTime1);
  ar& BOOST_SERIALIZATION_NVP(openTime2);
  ar& BOOST_SERIALIZATION_NVP(closeTime2);
  record = hayaku::MarketInfo(market, name, description, code,
                              hayaku::Datetime(lastDate), openTime1, closeTime1,
                              openTime2, closeTime2);
}

}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hayaku::MarketInfo)

#endif /* HAYAKU_SUPPORT_SERIALIZATION */
