#pragma once

/*
 * TimeLineRecord_serialization.h
 *
 *  Created on: 2019-2-11
 *      Author: fasiondog
 */

#include "common/Config.h"
#include "data/TransRecord.h"

#if HAYAKU_SUPPORT_SERIALIZATION

namespace boost {
namespace serialization {
template <class Archive>
void save(Archive& ar, const hayaku::TransRecord& record,
          unsigned int version) {
  hayaku::uint64_t datetime = record.datetime.number();
  ar& BOOST_SERIALIZATION_NVP(datetime);
  ar& make_nvp("price", record.price);
  ar& make_nvp("vol", record.vol);
  ar& make_nvp("direct", record.direct);
}

template <class Archive>
void load(Archive& ar, hayaku::TransRecord& record, unsigned int version) {
  hayaku::uint64_t datetime;
  ar& BOOST_SERIALIZATION_NVP(datetime);
  record.datetime = hayaku::Datetime(datetime);
  ar& make_nvp("price", record.price);
  ar& make_nvp("vol", record.vol);
  ar& make_nvp("direct", record.direct);
}
}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hayaku::TransRecord)

#endif /* HAYAKU_SUPPORT_SERIALIZATION */
