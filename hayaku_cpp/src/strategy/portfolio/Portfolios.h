#pragma once

/*
 * build_in.h
 *
 *  Created on: 2016-3-28
 *      Author: fasiondog
 */

/*
 * PF_Simple.h
 *
 *  Created on: 2018-1-13
 *      Author: fasiondog
 */

#include "Portfolio.h"
#include "strategy/portfolio/AllocationPolicies.h"
#include "strategy/selection/Selectors.h"

namespace hayaku {

/**
 * @brief Create a portfolio
 * @details
 * <pre>
 * Description of the rebalancing mode adjust_mode:
 *  - In the "query" mode it follows the ktype in the input parameter query; at
 * this time adjust_cycle determines the cycle interval with the ktype in query;
 *  - In the "day" mode adjust_cycle is the number of the days between the
 * rebalancing;
 *  - In the "week" | "month" | "quarter" | "year" mode, adjust_cycle is the
 * corresponding N-th day of every week, the n-th day of every month, the n-th
 * day of every quarter and the n-th day of every year; when
 * delay_to_trading_day is false, the rebalancing is skipped if that day is not
 * a trading day; when delay_to_trading_day is true, it is postponed to the
 * first trading day within the current cycle if that day is not a trading day;
 * for example, if the rebalancing is specified on the 1st day of every month
 * but the 1st is not a trading day, it is postponed to the first trading day of
 * that month
 * </pre>
 * @param tm trade account
 * @param se system selector
 * @param af fund allocation algorithm
 * @param adjust_cycle the rebalancing cycle (affected by adjust_mode), 1 by
 * default
 * @param adjust_mode the rebalancing mode "query" | "day" | "week" | "month" |
 * "year"
 * @param delay_to_trading_day when it is true, it is postponed to the first
 * trading day within the current cycle if the rebalancing day is not a trading
 * day
 * @return the portfolio instance
 */
PortfolioPtr PF_Simple(const internal::PortfolioAccountPortPtr& tm =
                           internal::PortfolioAccountPortPtr(),
                       const SEPtr& se = SE_Fixed(),
                       const AFPtr& af = AF_EqualWeight(), int adjust_cycle = 1,
                       const string& adjust_mode = "query",
                       bool delay_to_trading_day = true);

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-18
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * @brief Portfolio without a fund allocation algorithm
 * @details
 * <pre>
 * Description of the rebalancing mode adjust_mode:
 *  - In the "query" mode it follows the ktype in the input parameter query; at
 * this time adjust_cycle determines the cycle interval with the ktype in query;
 *  - In the "day" mode adjust_cycle is the number of the days between the
 * rebalancing;
 *  - In the "week" | "month" | "quarter" | "year" mode, adjust_cycle is the
 * corresponding N-th day of every week, the n-th day of every month, the n-th
 * day of every quarter and the n-th day of every year; when
 * delay_to_trading_day is false, the rebalancing is skipped if that day is not
 * a trading day; when delay_to_trading_day is true, it is postponed to the
 * first trading day within the current cycle if that day is not a trading day;
 * for example, if the rebalancing is specified on the 1st day of every month
 * but the 1st is not a trading day, it is postponed to the first trading day of
 * that month
 * </pre>
 * @note In the mode without a fund allocation algorithm, only all buying and
 * selling at the open or all buying and selling at the close is supported!
 * @param tm trade account
 * @param se system selector
 * @param adjust_cycle the rebalancing cycle (affected by adjust_mode), 1 by
 * default
 * @param adjust_mode the rebalancing mode "query" | "day" | "week" | "month" |
 * "year"
 * @param delay_to_trading_day when it is true, it is postponed to the first
 * trading day within the current cycle if that day is not a trading day
 * @param trade_on_close execute the trade at the close
 * @param strategy_use_own_account use the prototype strategy account for the
 * calculation (valid in the mode without a fund allocation only), false by
 * default
 * @param sell_at_not_selected whether to force selling the stocks not selected
 * on the rebalancing day, false by default
 * @return the portfolio instance
 */
PortfolioPtr PF_WithoutAF(const internal::PortfolioAccountPortPtr& tm =
                              internal::PortfolioAccountPortPtr(),
                          const SEPtr& se = SE_Fixed(), int adjust_cycle = 1,
                          const string& adjust_mode = "query",
                          bool delay_to_trading_day = true,
                          bool trade_on_close = true,
                          bool strategy_use_own_account = false,
                          bool sell_at_not_selected = false);

} /* namespace hayaku */
