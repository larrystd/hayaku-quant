#pragma once

/*
 * TimeDelta_serialization.h
 *
 *  Copyright (C) 2019 hikyuu.org
 *
 *  Created on: 2019-12-14
 *      Author: fasiondog
 */


#include "common/time/TimeDelta.h"
#include "common/Config.h"

#if HAYAKU_SUPPORT_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>

namespace boost {
namespace serialization {

template <class Archive>
void save(Archive& ar, const hayaku::TimeDelta& td, unsigned int version) {
    int64_t ticks = td.ticks();
    ar& BOOST_SERIALIZATION_NVP(ticks);
}

template <class Archive>
void load(Archive& ar, hayaku::TimeDelta& td, unsigned int version) {
    int64_t ticks;
    ar& BOOST_SERIALIZATION_NVP(ticks);
    td = hayaku::TimeDelta(boost::posix_time::time_duration(0, 0, 0, ticks));
}

}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hayaku::TimeDelta)

#endif /* HAYAKU_SUPPORT_SERIALIZATION */
