#pragma once

/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-14
 *     Author: fasiondog
 */


#include "data/MarketTypes.h"
#include "extensions/realtime/TimerManager.h"

namespace hayaku {

/**
 * Get the pointer of the global scheduler instance
 */
HAYAKU_API TimerManager* getScheduler();

/**
 * Used to release the global scheduler instance on program exit, for internal use only
 */
void releaseScheduler();

}  // namespace hayaku
