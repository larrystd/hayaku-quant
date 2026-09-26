/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-28
 *      Author: fasiondog
 */

#include "KDataImp.h"

#include <functional>

namespace hayaku {

KDataImp::KDataImp(const Stock& stock, const KQuery& query)
    : query_(query), stock_(stock) {}

KDataImp::~KDataImp() {}

} /* namespace hayaku */
