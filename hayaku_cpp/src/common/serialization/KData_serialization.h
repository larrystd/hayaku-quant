#pragma once

/*
 * KData.h
 *
 *  Created on: 2013-5-1
 *      Author: fasiondog
 */

#include "common/Config.h"
#include "data/KData.h"

#if HAYAKU_SUPPORT_SERIALIZATION
#include "KQuery_serialization.h"
#include "Stock_serialization.h"

namespace boost {
namespace serialization {
template <class Archive>
void save(Archive& ar, const hayaku::KData& kdata, unsigned int version) {
  hayaku::Stock stock = kdata.getStock();
  hayaku::KQuery query = kdata.getQuery();
  ar& BOOST_SERIALIZATION_NVP(stock);
  ar& BOOST_SERIALIZATION_NVP(query);
}

template <class Archive>
void load(Archive& ar, hayaku::KData& kdata, unsigned int version) {
  hayaku::Stock stock;
  hayaku::KQuery query;
  ar& BOOST_SERIALIZATION_NVP(stock);
  ar& BOOST_SERIALIZATION_NVP(query);
  kdata = stock.isNull() ? hayaku::KData() : hayaku::KData(stock, query);
}

}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hayaku::KData)

#endif /* HAYAKU_SUPPORT_SERIALIZATION */
