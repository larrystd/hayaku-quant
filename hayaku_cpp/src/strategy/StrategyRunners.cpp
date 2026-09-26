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

RunSystemInStrategy::RunSystemInStrategy(
    const internal::StrategyRuntimePtr& strategy, const OrderBrokerPtr& broker,
    const KQuery& query, const TradeCostPtr& costfunc)
    : strategy_(strategy), broker_(broker) {
  HAYAKU_ASSERT(strategy && broker);

  if (query.queryType() == KQuery::INDEX) {
    query_ = KQueryByIndex(query.start(), Null<int64_t>(), query.kType(),
                            query.recoverType());
  } else if (query.queryType() == KQuery::DATE) {
    query_ = KQueryByDate(query.startDatetime(), Null<Datetime>(),
                           query.kType(), query.recoverType());
  } else {
    HAYAKU_THROW("Invalid query: {}", query);
  }

  auto account = internal::makeExecutionAccount(
      AccountConfig(Datetime::now(), 0.0, costfunc, strategy->name(), 2, false,
                    false, {}, {broker}));
  broker_port_ =
      std::dynamic_pointer_cast<internal::ExecutionBrokerPort>(account);
  auto portfolio_account =
      std::dynamic_pointer_cast<internal::PortfolioAccountPort>(account);
  HAYAKU_CHECK(broker_port_ && portfolio_account,
               "Execution account does not support live strategy capabilities");
  strategy_->setAccount(std::move(portfolio_account));
  strategy_->setSP(SlippagePtr());
  strategy_->prepare();
}

void RunSystemInStrategy::run(const Stock& stock) {
  if (strategy_->getParam<bool>("buy_delay") && buy_request_.valid) {
    KData k = stock.getKData(KQueryByIndex(-1, Null<int64_t>(), query_.kType(),
                                           query_.recoverType()));
    const auto& stock = strategy_->getStock();
    broker_->buy(buy_request_.datetime, stock.market(), stock.code(), 10.0,
                  buy_request_.number, buy_request_.stoploss, buy_request_.goal,
                  buy_request_.origin, buy_request_.remark);
  }

  if (strategy_->getParam<bool>("sell_delay") && sell_request_.valid) {
    KData k = stock.getKData(KQueryByIndex(-1, Null<int64_t>(), query_.kType(),
                                           query_.recoverType()));
    const auto& stock = strategy_->getStock();
    broker_->sell(sell_request_.datetime, stock.market(), stock.code(), 10.0,
                   sell_request_.number, sell_request_.stoploss,
                   sell_request_.goal, sell_request_.origin,
                   sell_request_.remark);
  }

  broker_port_->fetchAssetInfoFromBroker(broker_);
  strategy_->run(BacktestRequest(stock.getKData(query_)));
  const auto& pending = strategy_->pendingOrders();

  if (strategy_->getParam<bool>("buy_delay")) {
    buy_request_ = pending.buy();
  }

  if (strategy_->getParam<bool>("sell_delay")) {
    sell_request_ = pending.sell();
  }
}

void RunSystemInStrategy::runMomentOnOpen(const Stock& stock) {
  auto k = stock.getKData(query_);
  strategy_->bind(k);
  broker_port_->fetchAssetInfoFromBroker(broker_);
  static_cast<void>(strategy_->runMomentOnOpen(k.back().datetime));
}

void RunSystemInStrategy::runMomentOnClose(const Stock& stock) {
  auto k = stock.getKData(query_);
  strategy_->bind(k);
  broker_port_->fetchAssetInfoFromBroker(broker_);
  static_cast<void>(strategy_->runMomentOnClose(k.back().datetime));
}

StrategyPtr HAYAKU_API crtSysStrategy(
    const internal::StrategyRuntimePtr& strategy, const string& stk_market_code,
    const KQuery& query, const OrderBrokerPtr& broker,
    const TradeCostPtr& costfunc, const string& name,
    const std::vector<OrderBrokerPtr>& other_brokers,
    const string& config_file) {
  std::shared_ptr<RunSystemInStrategy> runner =
      std::make_shared<RunSystemInStrategy>(strategy, broker, query, costfunc);

  auto broker_port = std::dynamic_pointer_cast<internal::ExecutionBrokerPort>(
      strategy->getAccount());
  HAYAKU_CHECK(broker_port,
               "Execution account does not support live broker registration");
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

RunPortfolioInStrategy::RunPortfolioInStrategy(const PFPtr& pf,
                                               const KQuery& query,
                                               const OrderBrokerPtr& broker,
                                               const TradeCostPtr& costfunc)
    : pf_(pf), broker_(broker) {
  HAYAKU_ASSERT(pf && broker);

  if (query.queryType() == KQuery::INDEX) {
    query_ = KQueryByIndex(query.start(), Null<int64_t>(), query.kType(),
                            query.recoverType());
  } else if (query.queryType() == KQuery::DATE) {
    query_ = KQueryByDate(query.startDatetime(), Null<Datetime>(),
                           query.kType(), query.recoverType());
  } else {
    HAYAKU_THROW("Invalid query: {}", query);
  }

  auto se = pf->getSE();
  HAYAKU_ASSERT(se);
  const auto& sys_list = se->getProtoSystemList();
  for (const auto& sys : sys_list) {
    HAYAKU_CHECK(!sys->getSP(),
                 "Exist Slippage part in sys, You must clear it! {}",
                 sys->name());
    HAYAKU_CHECK(
        !sys->getParam<bool>("buy_delay") && !sys->getParam<bool>("sell_delay"),
        "Thie method only support buy|sell on close!");
    stocks_.insert(sys->getStock());
  }

  auto account = internal::makeExecutionAccount(
      AccountConfig(Datetime::now(), 0.0, costfunc, pf->name(), 2, false, false,
                    {}, {broker}));
  broker_port_ =
      std::dynamic_pointer_cast<internal::ExecutionBrokerPort>(account);
  auto portfolio_account =
      std::dynamic_pointer_cast<internal::PortfolioAccountPort>(account);
  HAYAKU_CHECK(
      broker_port_ && portfolio_account,
      "Execution account does not support live portfolio capabilities");
  pf_->setAccount(std::move(portfolio_account));
}

void RunPortfolioInStrategy::run() {
  // Synchronizing forward is not allowed when there are already trade records
  if (!pf_->getAccount()->firstDatetime().isNull()) {
    broker_port_->fetchAssetInfoFromBroker(broker_);
    pf_->run(query_, true);
    return;
  }

  auto brk_asset = broker_->getAssetInfo();
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

        if (stocks_.find(stock) != stocks_.end()) {
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
    broker_port_->fetchAssetInfoFromBroker(broker_);
    pf_->run(query_, true);
    return;
  }

  // Get the latest position adjustment day
  auto k = getKData("sh000001", query_);
  auto cycle = CYCLE(k, pf_->getParam<int>("adjust_cycle"),
                     pf_->getParam<string>("adjust_mode"),
                     pf_->getParam<bool>("delay_to_trading_day"));
  HAYAKU_IF_RETURN(cycle.empty(), void());
  size_t n = cycle.size() - 1 - static_cast<size_t>(cycle[cycle.size() - 1]);
  if (n == 0) {
    broker_port_->fetchAssetInfoFromBroker(broker_);
    pf_->run(query_, true);
    return;
  }

  Datetime adjust_date = k[n].datetime;
  broker_port_->fetchAssetInfoFromBroker(broker_, adjust_date);
  pf_->run(query_, true);
}

StrategyPtr HAYAKU_API crtPFStrategy(
    const PFPtr& pf, const KQuery& query, const OrderBrokerPtr& broker,
    const TradeCostPtr& costfunc, const string& name,
    const std::vector<OrderBrokerPtr>& other_brokers,
    const string& config_file) {
  std::shared_ptr<RunPortfolioInStrategy> runner =
      std::make_shared<RunPortfolioInStrategy>(pf, query, broker, costfunc);

  auto broker_port = std::dynamic_pointer_cast<internal::ExecutionBrokerPort>(
      pf->getAccount());
  HAYAKU_CHECK(broker_port,
               "Execution account does not support live broker registration");
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
      code_list, vector<KQuery::KType>{ktype}, unordered_map<string, int64_t>{},
      name, config_file);

  int64_t m = KQuery::getKTypeInSeconds(ktype);
  if (m < KQuery::getKTypeInSeconds(KQuery::DAY)) {
    stg->runDaily(std::move(func), Seconds(m));
  } else {
    stg->runDailyAt(std::move(func), TimeDelta(0, 14, 50));
  }
  return stg;
}

}  // namespace hayaku
