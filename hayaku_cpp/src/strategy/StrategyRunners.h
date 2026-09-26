#pragma once

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-24
 *      Author: fasiondog
 */

#include "Strategy.h"
#include "execution/ExecutionBrokerPort.h"
#include "strategy/StrategyRuntime.h"

namespace hayaku {

class HAYAKU_API RunSystemInStrategy {
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

StrategyPtr HAYAKU_API crtSysStrategy(
    const internal::StrategyRuntimePtr& strategy, const string& stk_market_code,
    const KQuery& query, const OrderBrokerPtr& broker,
    const TradeCostPtr& costfunc, const string& name = "SYSStrategy",
    const std::vector<OrderBrokerPtr>& other_brokers = {},
    const string& config_file = "");

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-25
 *      Author: fasiondog
 */

#include "strategy/portfolio/Portfolio.h"

namespace hayaku {

class HAYAKU_API RunPortfolioInStrategy {
 public:
  RunPortfolioInStrategy() = default;
  RunPortfolioInStrategy(const PFPtr& pf, const KQuery& query,
                         const OrderBrokerPtr& broker,
                         const TradeCostPtr& costfunc);
  virtual ~RunPortfolioInStrategy() = default;

  void run();

 private:
  PFPtr m_pf;
  internal::ExecutionBrokerPortPtr m_broker_port;
  OrderBrokerPtr m_broker;
  KQuery m_query;
  std::unordered_set<Stock> m_stocks;
};

StrategyPtr HAYAKU_API crtPFStrategy(
    const PFPtr& pf, const KQuery& query, const OrderBrokerPtr& broker,
    const TradeCostPtr& costfunc, const string& name = "PFStrategy",
    const std::vector<OrderBrokerPtr>& other_brokers = {},
    const string& config_file = "");

}  // namespace hayaku
