#pragma once

/*
 * Datetime_Serialization.h
 *
 *  Created on: 2013-4-29
 *      Author: fasiondog
 */

#include "common/Config.h"
#include "common/time/Datetime.h"

#if HAYAKU_SUPPORT_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>

namespace boost {
namespace serialization {
template <class Archive>
void save(Archive& ar, const hayaku::Datetime& date, unsigned int version) {
  std::string datetime = date.str();
  ar& BOOST_SERIALIZATION_NVP(datetime);
}

template <class Archive>
void load(Archive& ar, hayaku::Datetime& date, unsigned int version) {
  std::string datetime;
  ar& BOOST_SERIALIZATION_NVP(datetime);
  date = hayaku::Datetime(datetime);
}
}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hayaku::Datetime)

#endif /* HAYAKU_SUPPORT_SERIALIZATION */
