#include "StrategyRunners.h"

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-24
 *      Author: fasiondog
 */


#include "execution/AccountConfig.h"
#include "execution/ExecutionAccountFactory.h"
#include "execution/PortfolioAccountPort.h"
#include "strategy/BacktestRequest.h"
#include "strategy/StrategyRuntime.h"

namespace hayaku {

RunSystemInStrategy::RunSystemInStrategy(const internal::StrategyRuntimePtr& strategy,
                                         const OrderBrokerPtr& broker, const KQuery& query,
                                         const TradeCostPtr& costfunc)
: m_strategy(strategy), m_broker(broker) {
    HAYAKU_ASSERT(strategy && broker);

    if (query.queryType() == KQuery::INDEX) {
        m_query = KQueryByIndex(query.start(), Null<int64_t>(), query.kType(), query.recoverType());
    } else if (query.queryType() == KQuery::DATE) {
        m_query =
          KQueryByDate(query.startDatetime(), Null<Datetime>(), query.kType(), query.recoverType());
    } else {
        HAYAKU_THROW("Invalid query: {}", query);
    }

    auto account = internal::makeExecutionAccount(
      AccountConfig(Datetime::now(), 0.0, costfunc, strategy->name(), 2, false, false, {},
                    {broker}));
    m_broker_port = std::dynamic_pointer_cast<internal::ExecutionBrokerPort>(account);
    auto portfolio_account = std::dynamic_pointer_cast<internal::PortfolioAccountPort>(account);
    HAYAKU_CHECK(m_broker_port && portfolio_account,
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

StrategyPtr HAYAKU_API crtSysStrategy(const internal::StrategyRuntimePtr& strategy,
                                   const string& stk_market_code,
                                   const KQuery& query, const OrderBrokerPtr& broker,
                                   const TradeCostPtr& costfunc, const string& name,
                                   const std::vector<OrderBrokerPtr>& other_brokers,
    const string& config_file) {
    std::shared_ptr<RunSystemInStrategy> runner =
      std::make_shared<RunSystemInStrategy>(strategy, broker, query, costfunc);

    auto broker_port = std::dynamic_pointer_cast<internal::ExecutionBrokerPort>(
      strategy->getAccount());
    HAYAKU_CHECK(broker_port, "Execution account does not support live broker registration");
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

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-25
 *      Author: fasiondog
 */

#include <nlohmann/json.hpp>
#include "operators/MarketOperators.h"

namespace hayaku {

RunPortfolioInStrategy::RunPortfolioInStrategy(const PFPtr& pf, const KQuery& query,
                                               const OrderBrokerPtr& broker,
                                               const TradeCostPtr& costfunc)
: m_pf(pf), m_broker(broker) {
    HAYAKU_ASSERT(pf && broker);

    if (query.queryType() == KQuery::INDEX) {
        m_query = KQueryByIndex(query.start(), Null<int64_t>(), query.kType(), query.recoverType());
    } else if (query.queryType() == KQuery::DATE) {
        m_query =
          KQueryByDate(query.startDatetime(), Null<Datetime>(), query.kType(), query.recoverType());
    } else {
        HAYAKU_THROW("Invalid query: {}", query);
    }

    auto se = pf->getSE();
    HAYAKU_ASSERT(se);
    const auto& sys_list = se->getProtoSystemList();
    for (const auto& sys : sys_list) {
        HAYAKU_CHECK(!sys->getSP(), "Exist Slippage part in sys, You must clear it! {}", sys->name());
        HAYAKU_CHECK(!sys->getParam<bool>("buy_delay") && !sys->getParam<bool>("sell_delay"),
                  "Thie method only support buy|sell on close!");
        m_stocks.insert(sys->getStock());
    }

    auto account = internal::makeExecutionAccount(
      AccountConfig(Datetime::now(), 0.0, costfunc, pf->name(), 2, false, false, {}, {broker}));
    m_broker_port = std::dynamic_pointer_cast<internal::ExecutionBrokerPort>(account);
    auto portfolio_account = std::dynamic_pointer_cast<internal::PortfolioAccountPort>(account);
    HAYAKU_CHECK(m_broker_port && portfolio_account,
              "Execution account does not support live portfolio capabilities");
    m_pf->setAccount(std::move(portfolio_account));
}

void RunPortfolioInStrategy::run() {
    // Synchronizing forward is not allowed when there are already trade records
    if (!m_pf->getAccount()->firstDatetime().isNull()) {
        m_broker_port->fetchAssetInfoFromBroker(m_broker);
        m_pf->run(m_query, true);
        return;
    }

    auto brk_asset = m_broker->getAssetInfo();
    if (brk_asset.empty()) {
        HAYAKU_WARN("Failed fetch asset info from broker!");
        return;
    }

    bool need_update_to_adjust_date = true;
    try {
        nlohmann::json asset = nlohmann::json::parse(brk_asset);
        auto& positions = asset["positions"];
        for (auto iter = positions.cbegin(); iter != positions.cend(); ++iter) {
            try {
                const auto& jpos = *iter;
                auto market = jpos["market"].get<string>();
                auto code = jpos["code"].get<string>();
                Stock stock = getStock(fmt::format("{}{}", market, code));
                if (stock.isNull()) {
                    HAYAKU_DEBUG("Not found stock: {}{}", market, code);
                    continue;
                }

                if (m_stocks.find(stock) != m_stocks.end()) {
                    need_update_to_adjust_date = false;
                    break;
                }

            } catch (const std::exception& e) {
                HAYAKU_ERROR(e.what());
            }
        }
    } catch (const std::exception& e) {
        HAYAKU_ERROR(e.what());
    }

    if (!need_update_to_adjust_date) {
        m_broker_port->fetchAssetInfoFromBroker(m_broker);
        m_pf->run(m_query, true);
        return;
    }

    // Get the latest position adjustment day
    auto k = getKData("sh000001", m_query);
    auto cycle =
      CYCLE(k, m_pf->getParam<int>("adjust_cycle"), m_pf->getParam<string>("adjust_mode"),
            m_pf->getParam<bool>("delay_to_trading_day"));
    HAYAKU_IF_RETURN(cycle.empty(), void());
    size_t n = cycle.size() - 1 - static_cast<size_t>(cycle[cycle.size() - 1]);
    if (n == 0) {
        m_broker_port->fetchAssetInfoFromBroker(m_broker);
        m_pf->run(m_query, true);
        return;
    }

    Datetime adjust_date = k[n].datetime;
    m_broker_port->fetchAssetInfoFromBroker(m_broker, adjust_date);
    m_pf->run(m_query, true);
}

StrategyPtr HAYAKU_API crtPFStrategy(const PFPtr& pf, const KQuery& query,
                                  const OrderBrokerPtr& broker, const TradeCostPtr& costfunc,
                                  const string& name,
                                  const std::vector<OrderBrokerPtr>& other_brokers,
                                  const string& config_file) {
    std::shared_ptr<RunPortfolioInStrategy> runner =
      std::make_shared<RunPortfolioInStrategy>(pf, query, broker, costfunc);

    auto broker_port = std::dynamic_pointer_cast<internal::ExecutionBrokerPort>(pf->getAccount());
    HAYAKU_CHECK(broker_port, "Execution account does not support live broker registration");
    for (const auto& brk : other_brokers) {
        if (brk) {
            broker_port->regBroker(brk);
        }
    }

    std::function<void(Strategy*)> func = [=](Strategy*) { runner->run(); };

    vector<string> code_list;
    std::set<int64_t> stk_set;
    auto sys_list = pf->getSE()->getProtoSystemList();
    for (const auto& sys : sys_list) {
        auto stock = sys->getStock();
        if (stk_set.count(stock.id()) == 0) {
            stk_set.insert(stock.id());
            code_list.emplace_back(stock.market_code());
        }
    }

    KQuery::KType ktype = query.kType();
    StrategyPtr stg = std::make_shared<Strategy>(
      code_list, vector<KQuery::KType>{ktype}, unordered_map<string, int64_t>{}, name, config_file);

    int64_t m = KQuery::getKTypeInSeconds(ktype);
    if (m < KQuery::getKTypeInSeconds(KQuery::DAY)) {
        stg->runDaily(std::move(func), Seconds(m));
    } else {
        stg->runDailyAt(std::move(func), TimeDelta(0, 14, 50));
    }
    return stg;
}

}  // namespace hayaku
