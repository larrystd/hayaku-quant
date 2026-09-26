#pragma once

/*
 * KRecord.h
 *
 *  Created on: 2013-5-1
 *      Author: fasiondog
 */

#include "common/Config.h"
#include "data/KRecord.h"

#if HAYAKU_SUPPORT_SERIALIZATION

namespace boost {
namespace serialization {
template <class Archive>
void save(Archive& ar, const hayaku::KRecord& record, unsigned int version) {
  hayaku::uint64_t datetime = record.datetime.number();
  ar& BOOST_SERIALIZATION_NVP(datetime);
  ar& make_nvp("openPrice", record.openPrice);
  ar& make_nvp("highPrice", record.highPrice);
  ar& make_nvp("lowPrice", record.lowPrice);
  ar& make_nvp("closePrice", record.closePrice);
  ar& make_nvp("transAmount", record.transAmount);
  ar& make_nvp("transCount", record.transCount);
}

template <class Archive>
void load(Archive& ar, hayaku::KRecord& record, unsigned int version) {
  hayaku::uint64_t datetime;
  ar& BOOST_SERIALIZATION_NVP(datetime);
  record.datetime = hayaku::Datetime(datetime);
  ar& make_nvp("openPrice", record.openPrice);
  ar& make_nvp("highPrice", record.highPrice);
  ar& make_nvp("lowPrice", record.lowPrice);
  ar& make_nvp("closePrice", record.closePrice);
  ar& make_nvp("transAmount", record.transAmount);
  ar& make_nvp("transCount", record.transCount);
}
}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hayaku::KRecord)

#endif /* HAYAKU_SUPPORT_SERIALIZATION */
