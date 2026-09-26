#pragma once

/*
 * StockWeight.h
 *
 *  Created on: 2013-5-1
 *      Author: fasiondog
 */


#include "common/Config.h"
#include "data/StockWeight.h"

#if HAYAKU_SUPPORT_SERIALIZATION

namespace boost {
namespace serialization {
template <class Archive>
void save(Archive& ar, const hayaku::StockWeight& record, unsigned int version) {
    hayaku::uint64_t datetime = record.datetime().number();
    hayaku::price_t countAsGift = record.countAsGift();
    hayaku::price_t countForSell = record.countForSell();
    hayaku::price_t priceForSell = record.priceForSell();
    hayaku::price_t bonus = record.bonus();
    hayaku::price_t increasement = record.increasement();
    hayaku::price_t totalCount = record.totalCount();
    hayaku::price_t freeCount = record.freeCount();
    hayaku::price_t suogu = record.suogu();
    ar& BOOST_SERIALIZATION_NVP(datetime);
    ar& BOOST_SERIALIZATION_NVP(countAsGift);
    ar& BOOST_SERIALIZATION_NVP(countForSell);
    ar& BOOST_SERIALIZATION_NVP(priceForSell);
    ar& BOOST_SERIALIZATION_NVP(bonus);
    ar& BOOST_SERIALIZATION_NVP(increasement);
    ar& BOOST_SERIALIZATION_NVP(totalCount);
    ar& BOOST_SERIALIZATION_NVP(freeCount);
    ar& BOOST_SERIALIZATION_NVP(suogu);
}

template <class Archive>
void load(Archive& ar, hayaku::StockWeight& record, unsigned int version) {
    hayaku::uint64_t datetime;
    hayaku::price_t countAsGift, countForSell, priceForSell, bonus;
    hayaku::price_t increasement, totalCount, freeCount, suogu;
    ar& BOOST_SERIALIZATION_NVP(datetime);
    ar& BOOST_SERIALIZATION_NVP(countAsGift);
    ar& BOOST_SERIALIZATION_NVP(countForSell);
    ar& BOOST_SERIALIZATION_NVP(priceForSell);
    ar& BOOST_SERIALIZATION_NVP(bonus);
    ar& BOOST_SERIALIZATION_NVP(increasement);
    ar& BOOST_SERIALIZATION_NVP(totalCount);
    ar& BOOST_SERIALIZATION_NVP(freeCount);
    ar& BOOST_SERIALIZATION_NVP(suogu);
    record = hayaku::StockWeight(hayaku::Datetime(datetime), countAsGift, countForSell, priceForSell,
                              bonus, increasement, totalCount, freeCount, suogu);
}
}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hayaku::StockWeight)

#endif /* HAYAKU_SUPPORT_SERIALIZATION */
