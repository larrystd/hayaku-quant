#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-06
 *      Author: fasiondog
 */


#include <functional>

#include "data/MarketTypes.h"

namespace hayaku {

/**
 * @brief Register an extended K-line type, synthesized by the time
 * @param ktype extended K-line type
 * @param basetype the corresponding base K-line type
 * @param minutes the number of the minutes contained in every K-line
 * @param getPhaseEnd get the phase end time to which the K-line belongs
 */
void HAYAKU_API registerExtraKType(const string& ktype, const string& basetype, int32_t minutes,
                                std::function<Datetime(const Datetime&)> getPhaseEnd);

/**
 * Register an extended K-line type, synthesized by the number of the bars
 * @param ktype extended K-line type
 * @param basetype the corresponding base K-line type
 * @param nbars the number of the base K-lines contained in every K-line
 */
void HAYAKU_API registerExtraKType(const string& ktype, const string& basetype, int32_t nbars);

/**
 * Release the extended K-line
 * @note In some cases, when the extended K-line is registered, a python-defined phase end date
 *       conversion function is used, which may cause a python
 * GIL error on exit; the extended K-line needs to be released manually in advance
 */
void HAYAKU_API releaseExtraKType();

void HAYAKU_API enableKDataCache(bool enable);

/** Install the application-side resolver for the data-owned extended K-line port. */
void installHayakuExtraPluginBridge();
void uninstallHayakuExtraPluginBridge() noexcept;

}  // namespace hayaku
