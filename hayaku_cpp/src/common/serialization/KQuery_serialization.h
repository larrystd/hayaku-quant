#pragma once

/*
 * KQuery.h
 *
 *  Created on: 2013-5-1
 *      Author: fasiondog
 */


#include "data/KQuery.h"
#include "common/Config.h"

#if HAYAKU_SUPPORT_SERIALIZATION

namespace boost {
namespace serialization {

template <class Archive>
void save(Archive& ar, const hayaku::KQuery& query, unsigned int version) {
    hayaku::string queryType, kType, recoverType;
    queryType = hayaku::KQuery::getQueryTypeName(query.queryType());
    kType = hayaku::KQuery::getKTypeName(query.kType());
    recoverType = hayaku::KQuery::getRecoverTypeName(query.recoverType());
    ar& BOOST_SERIALIZATION_NVP(queryType);
    ar& BOOST_SERIALIZATION_NVP(kType);
    ar& BOOST_SERIALIZATION_NVP(recoverType);

    if (query.queryType() == hayaku::KQuery::INDEX) {
        hayaku::int64_t start = query.start();
        hayaku::int64_t end = query.end();
        ar& BOOST_SERIALIZATION_NVP(start);
        ar& BOOST_SERIALIZATION_NVP(end);
    } else if (query.queryType() == hayaku::KQuery::DATE) {
        hayaku::uint64_t start = query.startDatetime().number();
        hayaku::uint64_t end = query.endDatetime().number();
        ar& BOOST_SERIALIZATION_NVP(start);
        ar& BOOST_SERIALIZATION_NVP(end);
    } else {
        // Illegal, ignored
    }
}

template <class Archive>
void load(Archive& ar, hayaku::KQuery& query, unsigned int version) {
    hayaku::string queryType, kType, recoverType;
    ar& BOOST_SERIALIZATION_NVP(queryType);
    ar& BOOST_SERIALIZATION_NVP(kType);
    ar& BOOST_SERIALIZATION_NVP(recoverType);

    hayaku::KQuery::QueryType enum_query = hayaku::KQuery::getQueryTypeEnum(queryType);
    hayaku::KQuery::KType enmu_ktype = hayaku::KQuery::getKTypeEnum(kType);
    hayaku::KQuery::RecoverType enum_recover = hayaku::KQuery::getRecoverTypeEnum(recoverType);

    if (enum_query == hayaku::KQuery::INDEX) {
        hayaku::int64_t start, end;
        ar& BOOST_SERIALIZATION_NVP(start);
        ar& BOOST_SERIALIZATION_NVP(end);
        query = hayaku::KQuery(start, end, enmu_ktype, enum_recover);
    } else if (enum_query == hayaku::KQuery::DATE) {
        hayaku::uint64_t start, end;
        ar& BOOST_SERIALIZATION_NVP(start);
        ar& BOOST_SERIALIZATION_NVP(end);
        query =
          hayaku::KQueryByDate(hayaku::Datetime(start), hayaku::Datetime(end), enmu_ktype, enum_recover);
    } else {
        // Illegal, ignored
    }
}

}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hayaku::KQuery)

#endif /* HAYAKU_SUPPORT_SERIALIZATION */
