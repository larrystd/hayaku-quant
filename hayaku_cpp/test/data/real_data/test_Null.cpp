/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-27
 *      Author: fasiondog
 */

#include <doctest/doctest.h>
#include <cstdint>
#include <cmath>
#include <data/MarketTypes.h>
#include <common/Null.h>

using namespace hayaku;

TEST_CASE("test_Null_size_t") {
    size_t null_size = Null<size_t>();
    CHECK_EQ(std::numeric_limits<std::size_t>::max(), null_size);

    CHECK_UNARY(std::isnan(Null<double>()));
    CHECK_UNARY(std::isnan(Null<price_t>()));
}
