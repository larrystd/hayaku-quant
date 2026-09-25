/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-17
 *      Author: fasiondog
 *
 *  Some miscellaneous functions
 */

#pragma once

#include "analysis/Performance.h"

namespace hku {

/**
 * Get the funds list of the account list at once
 * @param accounts account list
 * @param ref_dates
 * @return vector<FundsList>
 * @ingroup ExecutionAccount
 */
vector<FundsList> HKU_API getFundsList(
  const vector<internal::ExecutionAccountPortPtr>& accounts, const DatetimeList& ref_dates);

/**
 * Get the funds list of the account list at once
 * @param accounts account list
 * @param datetime deadline date
 * @param ktype K-line type
 * @param ext whether to get the extended statistics (donation users, otherwise the basic
 *            statistics are still returned)
 * @return vector<FundsList>
 * @ingroup ExecutionAccount
 */
vector<Performance> HKU_API getPerformanceList(
  const vector<internal::ExecutionAccountPortPtr>& accounts,
  const Datetime& datetime = Datetime::now(), const KQuery::KType& ktype = KQuery::DAY,
  bool ext = true);

}  // namespace hku
