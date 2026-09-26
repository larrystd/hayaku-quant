#pragma once

/*
 * Stock_Serialization.h
 *
 *  Created on: 2013-4-29
 *      Author: fasiondog
 */


#include "common/Config.h"
#include "data/Stock.h"

//===========================================================================
// The following is the serialization of Stock; currently only the serialization of the Stock
// managed by the data runtime is implemented
// Note: this piece of code is outside namespace hayaku
// TODO implement the serialization of a Stock that is not managed by the data runtime
//===========================================================================
#if HAYAKU_SUPPORT_SERIALIZATION

namespace boost {
namespace serialization {
template <class Archive>

void save(Archive& ar, const hayaku::Stock& stock, unsigned int version) {
    hayaku::string market_code = stock.market_code();
    ar& BOOST_SERIALIZATION_NVP(market_code);
}

template <class Archive>
void load(Archive& ar, hayaku::Stock& stock, unsigned int version) {
    hayaku::string market_code, name;
    ar& BOOST_SERIALIZATION_NVP(market_code);
    stock = hayaku::getStock(market_code);
}
}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hayaku::Stock)

#endif /* HAYAKU_SUPPORT_SERIALIZATION */
