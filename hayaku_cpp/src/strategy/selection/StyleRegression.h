#pragma once

/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-07-27
 *      Author: woleigegg
 *
 *  Style factor neutralization regression helpers.
 */

#include "data/MarketTypes.h"

namespace hayaku {

PriceList calculate_style_residuals(const PriceList& y,
                                    const vector<PriceList>& x);

}  // namespace hayaku
