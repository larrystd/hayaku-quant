#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-27
 *      Author: fasiondog
 */


#include <vector>
#include "data/KQuery.h"
#include "metrics/Performance.h"
#include "metrics/ReportExtension.h"
#include "execution/ExecutionAccountPort.h"
#include "execution/PositionExtInfo.h"
#include "application/plugins/PluginBase.h"

namespace hayaku {

class TMReportPluginInterface : public PluginBase, public ReportExtension {
public:
    static constexpr uint32_t PLUGIN_INTERFACE_VERSION = 1;
    TMReportPluginInterface() = default;
    virtual ~TMReportPluginInterface() = default;

    // Get the maximum drawdown percentage of the account up to the given moment (inclusive), it is
    // calculated from the close price only
    virtual price_t getMaxPullBack(const internal::ExecutionAccountPortPtr& tm, const Datetime& date,
                                   const KQuery::KType& ktype) = 0;

    /**
     * @brief Get the extended detail of the historical positions of the account
     * @param tm the given account
     * @param ktype K-line type
     * @param trade_mode trade mode, it affects some statistics items: 0-trade at the close, 1-trade
     *                   at the next open
     * @return std::vector<PositionExtInfo>
     */
    virtual std::vector<PositionExtInfo> getHistoryPositionExtInfoList(const internal::ExecutionAccountPortPtr& tm,
                                                                       const KQuery::KType& ktype,
                                                                       int trade_mode) = 0;

    /**
     * @brief Get the detail of the positions after the last trade moment of the account
     * @param tm the given account
     * @param current_time the current moment (it should be greater than or equal to the last trade
     *                     moment)
     * @param ktype K-line type
     * @param trade_mode trade mode, it affects some statistics items: 0-trade at the close, 1-trade
     *                   at the next open
     * @return std::vector<PositionExtInfo>
     */
    virtual std::vector<PositionExtInfo> getPositionExtInfoList(const internal::ExecutionAccountPortPtr& tm,
                                                                const Datetime& current_time,
                                                                const KQuery::KType& ktype,
                                                                int trade_mode) = 0;

    /**
     * @brief Get the position detail of the given security of the account at the given moment
     * @param tm account
     * @param stock the held stock
     * @param current_time the current moment (it should be greater than or equal to the last trade
     *                     moment)
     * @param ktype K-line type
     * @param trade_mode trade mode, it affects some statistics items: 0-trade at the close, 1-trade
     *                   at the next open
     * @return PositionExtInfo
     */
    virtual PositionExtInfo getPositionExtInfo(const internal::ExecutionAccountPortPtr& tm, const Stock& stock,
                                               const Datetime& current_time,
                                               const KQuery::KType& ktype, int trade_mode) = 0;

    /**
     * Count the system performance up to a certain moment; datetime must be greater than or equal
     * to lastDatetime so that it can be used to calculate the current market value
     * @param tm the given trade management instance
     * @param datetime the statistics end moment
     */
    virtual Performance getExtPerformance(const internal::ExecutionAccountPortPtr& tm, const Datetime& datetime,
                                          const KQuery::KType& ktype) override = 0;

    /**
     * @brief Get the monthly return percentages before the given end time
     * @param tm
     * @param datetime
     * @return std::vector<std::pair<Datetime, double>>
     */
    virtual std::vector<std::pair<Datetime, double>> getProfitPercentMonthly(
      const internal::ExecutionAccountPortPtr& tm, const Datetime& datetime) = 0;

    /**
     * @brief Get the yearly return percentages before the given end time
     * @param tm
     * @param datetime
     * @return std::vector<std::pair<Datetime, double>>
     */
    virtual std::vector<std::pair<Datetime, double>> getProfitPercentYearly(
      const internal::ExecutionAccountPortPtr& tm, const Datetime& datetime) = 0;
};

}  // namespace hayaku
