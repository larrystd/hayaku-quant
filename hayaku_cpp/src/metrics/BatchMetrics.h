#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-17
 *      Author: fasiondog
 *
 *  Some miscellaneous functions
 */

#include "metrics/Performance.h"

namespace hayaku {

/**
 * Get the funds list of the account list at once
 * @param accounts account list
 * @param ref_dates
 * @return vector<FundsList>
 * @ingroup ExecutionAccount
 */
vector<FundsList> getFundsList(
    const vector<internal::ExecutionAccountPortPtr>& accounts,
    const DatetimeList& ref_dates);

/**
 * Get the funds list of the account list at once
 * @param accounts account list
 * @param datetime deadline date
 * @param ktype K-line type
 * @param ext whether to get the extended statistics (donation users, otherwise
 * the basic statistics are still returned)
 * @return vector<FundsList>
 * @ingroup ExecutionAccount
 */
vector<Performance> getPerformanceList(
    const vector<internal::ExecutionAccountPortPtr>& accounts,
    const Datetime& datetime = Datetime::now(),
    const KQuery::KType& ktype = KQuery::DAY, bool ext = true);

}  // namespace hayaku
