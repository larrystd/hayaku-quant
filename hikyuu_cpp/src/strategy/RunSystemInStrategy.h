/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-24
 *      Author: fasiondog
 */

#pragma once
#include "execution/internal/ExecutionBrokerPort.h"
#include "strategy/engine/internal/StrategyRuntime.h"
#include "Strategy.h"

namespace hku {

class HKU_API RunSystemInStrategy {
public:
    RunSystemInStrategy() = default;
    RunSystemInStrategy(const internal::StrategyRuntimePtr& strategy,
                        const OrderBrokerPtr& broker, const KQuery& query,
                        const TradeCostPtr& costfunc);
    virtual ~RunSystemInStrategy() = default;

    void run(const Stock& stock);

    void runMomentOnOpen(const Stock& stock);
    void runMomentOnClose(const Stock& stock);

private:
    internal::StrategyRuntimePtr m_strategy;
    internal::ExecutionBrokerPortPtr m_broker_port;
    OrderBrokerPtr m_broker;
    KQuery m_query;

    internal::PendingOrder m_buyRequest;
    internal::PendingOrder m_sellRequest;
};

StrategyPtr HKU_API crtSysStrategy(const internal::StrategyRuntimePtr& strategy,
                                   const string& stk_market_code,
                                   const KQuery& query, const OrderBrokerPtr& broker,
                                   const TradeCostPtr& costfunc, const string& name = "SYSStrategy",
                                   const std::vector<OrderBrokerPtr>& other_brokers = {},
                                   const string& config_file = "");

}  // namespace hku
