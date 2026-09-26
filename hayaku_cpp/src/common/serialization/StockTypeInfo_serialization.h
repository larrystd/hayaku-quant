#pragma once

/*
 * StockTypeInfo.h
 *
 *  Created on: 2013-5-1
 *      Author: fasiondog
 */


#include "common/Config.h"
#include "data/StockTypeInfo.h"

#if HAYAKU_SUPPORT_SERIALIZATION

namespace boost {
namespace serialization {

template <class Archive>
void save(Archive& ar, const hayaku::StockTypeInfo& record, unsigned int version) {
    hayaku::uint32_t type = record.type();
    hayaku::string description = record.description();
    hayaku::price_t tick = record.tick();
    hayaku::price_t tickValue = record.tickValue();
    int precision = record.precision();
    double minTradeNumber = record.minTradeNumber();
    double maxTradeNumber = record.maxTradeNumber();
    ar& BOOST_SERIALIZATION_NVP(type);
    ar& BOOST_SERIALIZATION_NVP(description);
    ar& BOOST_SERIALIZATION_NVP(tick);
    ar& BOOST_SERIALIZATION_NVP(tickValue);
    ar& BOOST_SERIALIZATION_NVP(precision);
    ar& BOOST_SERIALIZATION_NVP(minTradeNumber);
    ar& BOOST_SERIALIZATION_NVP(maxTradeNumber);
}

template <class Archive>
void load(Archive& ar, hayaku::StockTypeInfo& record, unsigned int version) {
    hayaku::uint32_t type;
    hayaku::string description;
    hayaku::price_t tick, tickValue;
    int precision;
    double minTradeNumber, maxTradeNumber;
    ar& BOOST_SERIALIZATION_NVP(type);
    ar& BOOST_SERIALIZATION_NVP(description);
    ar& BOOST_SERIALIZATION_NVP(tick);
    ar& BOOST_SERIALIZATION_NVP(tickValue);
    ar& BOOST_SERIALIZATION_NVP(precision);
    ar& BOOST_SERIALIZATION_NVP(minTradeNumber);
    ar& BOOST_SERIALIZATION_NVP(maxTradeNumber);
    record = hayaku::StockTypeInfo(type, description, tick, tickValue, precision, minTradeNumber,
                                maxTradeNumber);
}
}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hayaku::StockTypeInfo)

#endif /* HAYAKU_SUPPORT_SERIALIZATION */
