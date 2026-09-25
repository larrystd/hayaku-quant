/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-24
 *      Author: fasiondog
 */

#include "RunSystemInStrategy.h"

#include "execution/AccountConfig.h"
#include "execution/internal/ExecutionAccountFactory.h"
#include "execution/internal/PortfolioAccountPort.h"
#include "strategy/engine/BacktestRequest.h"
#include "strategy/engine/internal/StrategyRuntime.h"

namespace hku {

RunSystemInStrategy::RunSystemInStrategy(const internal::StrategyRuntimePtr& strategy,
                                         const OrderBrokerPtr& broker, const KQuery& query,
                                         const TradeCostPtr& costfunc)
: m_strategy(strategy), m_broker(broker) {
    HKU_ASSERT(strategy && broker);

    if (query.queryType() == KQuery::INDEX) {
        m_query = KQueryByIndex(query.start(), Null<int64_t>(), query.kType(), query.recoverType());
    } else if (query.queryType() == KQuery::DATE) {
        m_query =
          KQueryByDate(query.startDatetime(), Null<Datetime>(), query.kType(), query.recoverType());
    } else {
        HKU_THROW("Invalid query: {}", query);
    }

    auto account = internal::makeExecutionAccount(
      AccountConfig(Datetime::now(), 0.0, costfunc, strategy->name(), 2, false, false, {},
                    {broker}));
    m_broker_port = std::dynamic_pointer_cast<internal::ExecutionBrokerPort>(account);
    auto portfolio_account = std::dynamic_pointer_cast<internal::PortfolioAccountPort>(account);
    HKU_CHECK(m_broker_port && portfolio_account,
              "Execution account does not support live strategy capabilities");
    m_strategy->setAccount(std::move(portfolio_account));
    m_strategy->setSP(SlippagePtr());
    m_strategy->prepare();
}

void RunSystemInStrategy::run(const Stock& stock) {
    if (m_strategy->getParam<bool>("buy_delay") && m_buyRequest.valid) {
        KData k = stock.getKData(
          KQueryByIndex(-1, Null<int64_t>(), m_query.kType(), m_query.recoverType()));
        const auto& stock = m_strategy->getStock();
        m_broker->buy(m_buyRequest.datetime, stock.market(), stock.code(), 10.0,
                      m_buyRequest.number, m_buyRequest.stoploss, m_buyRequest.goal,
                      m_buyRequest.origin, m_buyRequest.remark);
    }

    if (m_strategy->getParam<bool>("sell_delay") && m_sellRequest.valid) {
        KData k = stock.getKData(
          KQueryByIndex(-1, Null<int64_t>(), m_query.kType(), m_query.recoverType()));
        const auto& stock = m_strategy->getStock();
        m_broker->sell(m_sellRequest.datetime, stock.market(), stock.code(), 10.0,
                       m_sellRequest.number, m_sellRequest.stoploss, m_sellRequest.goal,
                       m_sellRequest.origin, m_sellRequest.remark);
    }

    m_broker_port->fetchAssetInfoFromBroker(m_broker);
    m_strategy->run(BacktestRequest(stock.getKData(m_query)));
    const auto& pending = m_strategy->pendingOrders();

    if (m_strategy->getParam<bool>("buy_delay")) {
        m_buyRequest = pending.buy();
    }

    if (m_strategy->getParam<bool>("sell_delay")) {
        m_sellRequest = pending.sell();
    }
}

void RunSystemInStrategy::runMomentOnOpen(const Stock& stock) {
    auto k = stock.getKData(m_query);
    m_strategy->bind(k);
    m_broker_port->fetchAssetInfoFromBroker(m_broker);
    static_cast<void>(m_strategy->runMomentOnOpen(k.back().datetime));
}

void RunSystemInStrategy::runMomentOnClose(const Stock& stock) {
    auto k = stock.getKData(m_query);
    m_strategy->bind(k);
    m_broker_port->fetchAssetInfoFromBroker(m_broker);
    static_cast<void>(m_strategy->runMomentOnClose(k.back().datetime));
}

StrategyPtr HKU_API crtSysStrategy(const internal::StrategyRuntimePtr& strategy,
                                   const string& stk_market_code,
                                   const KQuery& query, const OrderBrokerPtr& broker,
                                   const TradeCostPtr& costfunc, const string& name,
                                   const std::vector<OrderBrokerPtr>& other_brokers,
    const string& config_file) {
    std::shared_ptr<RunSystemInStrategy> runner =
      std::make_shared<RunSystemInStrategy>(strategy, broker, query, costfunc);

    auto broker_port = std::dynamic_pointer_cast<internal::ExecutionBrokerPort>(
      strategy->getAccount());
    HKU_CHECK(broker_port, "Execution account does not support live broker registration");
    for (const auto& brk : other_brokers) {
        if (brk) {
            broker_port->regBroker(brk);
        }
    }

    std::function<void(Strategy*)> func = [=](Strategy*) {
        runner->run(getStock(stk_market_code));
    };

    KQuery::KType ktype = query.kType();
    StrategyPtr stg = std::make_shared<Strategy>(
      vector<string>{stk_market_code, "SH000001"}, vector<KQuery::KType>{ktype},
      unordered_map<string, int64_t>{}, name, config_file);

    int64_t m = KQuery::getKTypeInSeconds(ktype);
    if (m < KQuery::getKTypeInSeconds(KQuery::DAY)) {
        stg->runDaily(std::move(func), Seconds(m), "SH");
    } else {
        stg->runDailyAt(std::move(func), TimeDelta(0, 14, 50));
    }
    return stg;
}

}  // namespace hku
