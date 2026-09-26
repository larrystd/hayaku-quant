#pragma once

/*
 * TimeLineRecord_serialization.h
 *
 *  Created on: 2019-2-8
 *      Author: fasiondog
 */


#include "common/Config.h"
#include "data/TimeLineRecord.h"

#if HAYAKU_SUPPORT_SERIALIZATION

namespace boost {
namespace serialization {
template <class Archive>
void save(Archive& ar, const hayaku::TimeLineRecord& record, unsigned int version) {
    hayaku::uint64_t datetime = record.datetime.number();
    ar& BOOST_SERIALIZATION_NVP(datetime);
    ar& make_nvp("price", record.price);
    ar& make_nvp("vol", record.vol);
}

template <class Archive>
void load(Archive& ar, hayaku::TimeLineRecord& record, unsigned int version) {
    hayaku::uint64_t datetime;
    ar& BOOST_SERIALIZATION_NVP(datetime);
    record.datetime = hayaku::Datetime(datetime);
    ar& make_nvp("price", record.price);
    ar& make_nvp("vol", record.vol);
}
}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hayaku::TimeLineRecord)

#endif /* HAYAKU_SUPPORT_SERIALIZATION */
