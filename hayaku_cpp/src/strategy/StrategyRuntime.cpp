/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "StrategyRuntime.h"

#include "ComponentContext.h"

namespace hayaku::internal {

namespace {
TradeRecord submitStrategyOrder(StrategyExecutionPort& execution,
                                const OrderRequest& request) {
  return execution.submit(request).trade();
}
}  // namespace

StrategyRuntime::StrategyRuntime(const StrategyDefinition& definition,
                                 ExecutionAccountPortPtr account)
    : account_(std::move(account)),
      execution_(account_),
      mm_(definition.moneyManager()),
      ev_(definition.environment()),
      cn_(definition.condition()),
      sg_(definition.signal()),
      st_(definition.stoploss()),
      tp_(definition.takeProfit()),
      pg_(definition.profitGoal()),
      sp_(definition.slippage()),
      name_(definition.name()) {
  HAYAKU_CHECK(account_, "StrategyRuntime requires an execution account");
  initParameters(definition.parameters());
}

void StrategyRuntime::initParameters(const Parameter& overrides) {
  parameters_.set<bool>("trace", false);
  parameters_.set<int>("max_delay_count", 3);
  parameters_.set<bool>("buy_delay", true);
  parameters_.set<bool>("sell_delay", true);
  parameters_.set<bool>("delay_use_current_price", true);
  parameters_.set<bool>("tp_monotonic", true);
  parameters_.set<int>("tp_delay_n", 1);
  parameters_.set<bool>("ignore_sell_sg", false);
  parameters_.set<bool>("can_trade_when_high_eq_low", false);
  parameters_.set<bool>("ev_open_position", false);
  parameters_.set<bool>("cn_open_position", false);
  parameters_.set<bool>("support_borrow_cash", false);
  parameters_.set<bool>("support_borrow_stock", false);
  parameters_.set<bool>("shared_account", false);
  parameters_.set<bool>("shared_ev", true);
  parameters_.set<bool>("shared_cn", false);
  parameters_.set<bool>("shared_sg", false);
  parameters_.set<bool>("shared_mm", false);
  parameters_.set<bool>("shared_st", false);
  parameters_.set<bool>("shared_tp", false);
  parameters_.set<bool>("shared_pg", false);
  parameters_.set<bool>("shared_sp", false);
  for (const auto& item : overrides) {
    parameters_.set<boost::any>(item.first, item.second);
  }
}

void StrategyRuntime::resetState(bool all) {
  if (all || !getParam<bool>("shared_account")) {
    account_->reset();
  }
  if (ev_ && (all || !getParam<bool>("shared_ev"))) {
    ev_->reset();
  }
  if (cn_ && (all || !getParam<bool>("shared_cn"))) {
    cn_->reset();
  }
  if (mm_ && (all || !getParam<bool>("shared_mm"))) {
    mm_->reset();
  }
  if (sg_ && (all || !getParam<bool>("shared_sg"))) {
    sg_->reset();
  }
  if (st_ && (all || !getParam<bool>("shared_st"))) {
    st_->reset();
  }
  if (tp_ && (all || !getParam<bool>("shared_tp"))) {
    tp_->reset();
  }
  if (pg_ && (all || !getParam<bool>("shared_pg"))) {
    pg_->reset();
  }
  if (sp_ && (all || !getParam<bool>("shared_sp"))) {
    sp_->reset();
  }

  if (all) {
    stock_ = Null<Stock>();
    kdata_ = Null<KData>();
    raw_k_data_ = Null<KData>();
  }
  calculated_ = false;
  pre_environment_valid_ = !ev_;
  pre_condition_valid_ = !cn_;
  buy_days_ = 0;
  sell_short_days_ = 0;
  trades_.clear();
  last_take_profit_ = 0.0;
  last_short_take_profit_ = 0.0;
  pending_orders_.clear();
}

void StrategyRuntime::prepare() { ComponentContext(*this).prepare(); }

void StrategyRuntime::bind(const KData& kdata) {
  ComponentContext(*this).bind(kdata);
}

void StrategyRuntime::run(const BacktestRequest& request) {
  if (request.resetAll()) {
    resetState(true);
  } else if (request.reset()) {
    resetState(false);
  }

  HAYAKU_IF_RETURN(stopRequested(), void());
  HAYAKU_DEBUG_IF_RETURN(calculated_ && kdata_ == request.kdata(), void(),
                         "Not need calculate.");
  prepare();
  bind(request.kdata());

  const bool trace = getParam<bool>("trace");
  const size_t total = kdata_.size();
  const auto* records = kdata_.data();
  const auto* rawRecords = raw_k_data_.data();
  HAYAKU_ASSERT(kdata_.size() == raw_k_data_.size());

  Datetime initDatetime = execution_.initDatetime();
  Datetime lastDatetime = execution_.lastDatetime();
  if (KQuery::getKTypeInSeconds(kdata_.getQuery().kType()) >= 86400) {
    initDatetime = initDatetime.startOfDay();
    lastDatetime = lastDatetime.startOfDay();
  }

  for (size_t i = 0; i < total; ++i) {
    if (stopRequested()) {
      calculated_ = false;
      break;
    }
    if (records[i].datetime < initDatetime ||
        records[i].datetime < lastDatetime) {
      continue;
    }
    const auto trade = runMomentNative(records[i], rawRecords[i]);
    if (trace) {
      HAYAKU_INFO_IF(!trade.isNull(), "{}", trade);
      const PositionRecord position =
          execution_.position(records[i].datetime, stock_);
      const FundsRecord funds =
          execution_.funds(records[i].datetime, kdata_.getQuery().kType());
      if (position.number > 0.0) {
        HAYAKU_INFO(
            "total: {:.2f}, cash: {:.2f}, position: {:.2f}, close: {:.2f}",
            funds.total_assets(), funds.cash, position.number,
            rawRecords[i].closePrice);
      }
    }
  }
  if (!stopRequested()) {
    calculated_ = true;
  }
}

void StrategyRuntime::run(const KQuery& query, bool resetState, bool resetAll) {
  HAYAKU_CHECK(!stock_.isNull(), "Strategy stock is null");
  run(BacktestRequest(stock_.getKData(query), resetState, resetAll));
}

void StrategyRuntime::run(const Stock& stock, const KQuery& query,
                          bool resetState, bool resetAll) {
  HAYAKU_CHECK(!stock.isNull(), "Strategy stock is null");
  run(BacktestRequest(stock.getKData(query), resetState, resetAll));
}

TradeRecord StrategyRuntime::runMoment(const Datetime& datetime) {
  const size_t pos = kdata_.getPos(datetime);
  HAYAKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());
  return runMomentNative(kdata_.getKRecord(pos), raw_k_data_.getKRecord(pos));
}

TradeRecord StrategyRuntime::runMomentOnOpen(const Datetime& datetime) {
  const size_t pos = kdata_.getPos(datetime);
  HAYAKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());
  return runMomentOnOpenNative(kdata_.getKRecord(pos),
                               raw_k_data_.getKRecord(pos));
}

TradeRecord StrategyRuntime::runMomentOnClose(const Datetime& datetime) {
  const size_t pos = kdata_.getPos(datetime);
  HAYAKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());
  return runMomentOnCloseNative(kdata_.getKRecord(pos),
                                raw_k_data_.getKRecord(pos));
}

TradeRecord StrategyRuntime::sellForceOnOpen(const Datetime& datetime,
                                             double number,
                                             OrderOrigin origin) {
  return sellForce(datetime, number, origin, true);
}

TradeRecord StrategyRuntime::sellForceOnClose(const Datetime& datetime,
                                              double number,
                                              OrderOrigin origin) {
  return sellForce(datetime, number, origin, false);
}

void StrategyRuntime::clearPendingBuy() { pending_orders_.buy().clear(); }

TradeRecord StrategyRuntime::processPendingBuy(const Datetime& datetime) {
  const size_t pos = kdata_.getPos(datetime);
  HAYAKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());
  return buyDelay(kdata_.getKRecord(pos), raw_k_data_.getKRecord(pos));
}

TradeRecord StrategyRuntime::processPendingSell(const Datetime& datetime) {
  const size_t pos = kdata_.getPos(datetime);
  HAYAKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());
  return sellDelay(kdata_.getKRecord(pos), raw_k_data_.getKRecord(pos));
}

const PendingOrderState& StrategyRuntime::pendingOrders() const noexcept {
  return pending_orders_;
}

AccountId StrategyRuntime::accountId() const noexcept {
  return account_->accountId();
}

const string& StrategyRuntime::name() const noexcept { return name_; }

void StrategyRuntime::name(string value) { name_ = std::move(value); }

Stock StrategyRuntime::getStock() const { return stock_; }

void StrategyRuntime::setStock(const Stock& stock) {
  if (stock_ != stock) {
    stock_ = stock;
    calculated_ = false;
  }
}

KData StrategyRuntime::getTO() const { return kdata_; }

MoneyManagerPtr StrategyRuntime::getMM() const { return mm_; }

SignalPtr StrategyRuntime::getSG() const { return sg_; }

SlippagePtr StrategyRuntime::getSP() const { return sp_; }

void StrategyRuntime::setSP(SlippagePtr slippage) {
  sp_ = std::move(slippage);
  calculated_ = false;
}

PortfolioAccountPortPtr StrategyRuntime::getAccount() const {
  return std::dynamic_pointer_cast<PortfolioAccountPort>(account_);
}

void StrategyRuntime::setAccount(PortfolioAccountPortPtr account) {
  HAYAKU_CHECK(account, "StrategyRuntime requires an execution account");
  account_ = std::move(account);
  execution_ = StrategyExecutionPort(account_);
  calculated_ = false;
}

StrategyRuntimePtr StrategyRuntime::clone() const {
  StrategyDefinition definition(mm_ ? mm_->clone() : MoneyManagerPtr(),
                                sg_ ? sg_->clone() : SignalPtr(), name_,
                                ev_ ? ev_->clone() : EnvironmentPtr(),
                                cn_ ? cn_->clone() : ConditionPtr(),
                                st_ ? st_->clone() : StoplossPtr(),
                                tp_ ? tp_->clone() : StoplossPtr(),
                                pg_ ? pg_->clone() : ProfitGoalPtr(),
                                sp_ ? sp_->clone() : SlippagePtr(),
                                parameters_);
  auto portfolioAccount = getAccount();
  ExecutionAccountPortPtr clonedAccount =
      getParam<bool>("shared_account") || !portfolioAccount
          ? account_
          : portfolioAccount->cloneAccount();
  auto result =
      std::make_shared<StrategyRuntime>(definition, std::move(clonedAccount));
  result->stock_ = stock_;
  result->kdata_ = kdata_;
  result->raw_k_data_ = raw_k_data_;
  return result;
}

void StrategyRuntime::reset() { resetState(false); }

void StrategyRuntime::forceResetAll() { resetState(true); }

nlohmann::json StrategyRuntime::lastSuggestion() const {
  nlohmann::json result;
  result["name"] = name_;
  result["stock"] = stock_.isNull() ? nlohmann::json(nullptr)
                                     : nlohmann::json(stock_.market_code());
  nlohmann::json pending = nlohmann::json::array();
  const auto append = [&](const PendingOrder& order) {
    if (order.valid) {
      pending.push_back({{"business", getBusinessName(order.business)},
                         {"datetime", order.datetime.str()},
                         {"stoploss", order.stoploss},
                         {"goal", order.goal},
                         {"number", order.number},
                         {"origin", getOrderOriginName(order.origin)},
                         {"remark", order.remark},
                         {"count", order.count}});
    }
  };
  append(pending_orders_.buy());
  append(pending_orders_.sell());
  append(pending_orders_.sellShort());
  append(pending_orders_.buyShort());
  result["pending_orders"] = pending;
  return result;
}

const TradeRecordList& StrategyRuntime::trades() const noexcept {
  return trades_;
}

void StrategyRuntime::setStopToken(const std::atomic_bool* stopToken) noexcept {
  stop_token_ = stopToken;
}

bool StrategyRuntime::stopRequested() const noexcept {
  return stop_token_ && stop_token_->load(std::memory_order_acquire);
}

bool StrategyRuntime::environmentIsValid(const Datetime& datetime) {
  return ev_ ? ev_->isValid(datetime) : true;
}

bool StrategyRuntime::conditionIsValid(const Datetime& datetime) {
  return cn_ ? cn_->isValid(datetime) : true;
}

void StrategyRuntime::buyNotifyAll(const TradeRecord& record) {
  if (mm_) {
    mm_->buyNotify(record);
  }
  if (pg_) {
    pg_->buyNotify(record);
  }
}

void StrategyRuntime::sellNotifyAll(const TradeRecord& record) {
  if (mm_) {
    mm_->sellNotify(record);
  }
  if (pg_) {
    pg_->sellNotify(record);
  }
}

double StrategyRuntime::getBuyNumber(const Datetime& datetime, price_t price,
                                     price_t risk, OrderOrigin origin) {
  return mm_ ? mm_->getBuyNumber(datetime, stock_, price, risk, origin)
              : 0.0;
}

double StrategyRuntime::getSellNumber(const Datetime& datetime, price_t price,
                                      price_t risk, OrderOrigin origin) {
  return mm_ ? mm_->getSellNumber(datetime, stock_, price, risk, origin)
              : 0.0;
}

double StrategyRuntime::getSellShortNumber(const Datetime& datetime,
                                           price_t price, price_t risk,
                                           OrderOrigin origin) {
  return mm_ ? mm_->getSellShortNumber(datetime, stock_, price, risk, origin)
              : 0.0;
}

double StrategyRuntime::getBuyShortNumber(const Datetime& datetime,
                                          price_t price, price_t risk,
                                          OrderOrigin origin) {
  return mm_ ? mm_->getBuyShortNumber(datetime, stock_, price, risk, origin)
              : 0.0;
}

price_t StrategyRuntime::getTakeProfitPrice(const Datetime& datetime,
                                            price_t currentPrice) {
  return tp_ ? tp_->getPrice(datetime, currentPrice) : 0.0;
}

price_t StrategyRuntime::getGoalPrice(const Datetime& datetime, price_t price) {
  return pg_ ? pg_->getGoal(datetime, price) : Null<price_t>();
}

price_t StrategyRuntime::getShortGoalPrice(const Datetime& datetime,
                                           price_t price) {
  return pg_ ? pg_->getShortGoal(datetime, price) : 0.0;
}

price_t StrategyRuntime::getRealBuyPrice(const Datetime& datetime,
                                         price_t planPrice) {
  return sp_ ? sp_->getRealBuyPrice(datetime, planPrice) : planPrice;
}

price_t StrategyRuntime::getRealSellPrice(const Datetime& datetime,
                                          price_t planPrice) {
  return sp_ ? sp_->getRealSellPrice(datetime, planPrice) : planPrice;
}

TradeRecord StrategyRuntime::runMomentNative(const KRecord& today,
                                             const KRecord& rawToday) {
  const TradeRecord openTrade = runMomentOnOpenNative(today, rawToday);
  const TradeRecord closeTrade = runMomentOnCloseNative(today, rawToday);
  return closeTrade.isNull() ? openTrade : closeTrade;
}

TradeRecord StrategyRuntime::runMomentOnOpenNative(const KRecord& today,
                                                   const KRecord& rawToday) {
  const bool trace = getParam<bool>("trace");
  if (trace) {
    HAYAKU_INFO("{} ------------------------------------------------------",
                today.datetime);
    HAYAKU_INFO(htr("[{}] cal today {}", name_, today));
    HAYAKU_INFO_IF(kdata_.getQuery().recoverType() != KQuery::NO_RECOVER,
                   htr("[{}] raw today {}", name_, rawToday));
  }
  ++buy_days_;
  ++sell_short_days_;
  HAYAKU_DEBUG_IF_RETURN(
      (today.closePrice > today.highPrice ||
       today.closePrice < today.lowPrice || today.lowPrice > today.highPrice),
      TradeRecord(), "[{}] ignore invalid price data at {}", name_,
      today.datetime);
  return processRequest(today, rawToday);
}

TradeRecord StrategyRuntime::runMomentOnCloseNative(const KRecord& today,
                                                    const KRecord& rawToday) {
  const bool trace = getParam<bool>("trace");
  const bool environmentValid = environmentIsValid(today.datetime);
  if (!environmentValid) {
    TradeRecord trade;
    if (account_->have(stock_)) {
      trade = sell(today, rawToday, OrderOrigin::ENVIRONMENT);
    }
    pre_environment_valid_ = false;
    return trade;
  }
  if (!pre_environment_valid_ && getParam<bool>("ev_open_position")) {
    pre_environment_valid_ = true;
    return buy(today, rawToday, OrderOrigin::ENVIRONMENT);
  }
  pre_environment_valid_ = true;

  const bool conditionValid = conditionIsValid(today.datetime);
  if (!conditionValid) {
    TradeRecord trade;
    if (account_->have(stock_)) {
      trade = sell(today, rawToday, OrderOrigin::CONDITION);
    }
    pre_condition_valid_ = false;
    return trade;
  }
  if (!pre_condition_valid_ && getParam<bool>("cn_open_position")) {
    pre_condition_valid_ = true;
    return buy(today, rawToday, OrderOrigin::CONDITION);
  }
  pre_condition_valid_ = true;

  if (sg_->shouldBuy(today.datetime)) {
    return account_->haveShort(stock_)
               ? buyShort(today, rawToday, OrderOrigin::SIGNAL)
               : buy(today, rawToday, OrderOrigin::SIGNAL);
  }
  if (sg_->shouldSell(today.datetime)) {
    return account_->have(stock_)
               ? sell(today, rawToday, OrderOrigin::SIGNAL)
               : sellShort(today, rawToday, OrderOrigin::SIGNAL);
  }

  const price_t currentPrice = today.closePrice;
  const price_t rawCurrentPrice = rawToday.closePrice;
  const PositionRecord position =
      account_->getPosition(today.datetime, stock_);
  HAYAKU_INFO_IF(trace,
                 htr("[{}] current position: {}", name_, position.number));
  if (position.number == 0.0) {
    return {};
  }
  if (rawCurrentPrice <= position.stoploss) {
    return sell(today, rawToday, OrderOrigin::STOP_LOSS);
  }
  if (rawCurrentPrice >= getGoalPrice(today.datetime, rawCurrentPrice)) {
    return sell(today, rawToday, OrderOrigin::PROFIT_GOAL);
  }
  price_t takeProfit = getTakeProfitPrice(today.datetime, currentPrice);
  if (takeProfit == 0.0) {
    return {};
  }
  if (takeProfit < last_take_profit_) {
    takeProfit = last_take_profit_;
  } else {
    last_take_profit_ = takeProfit;
  }
  const size_t pos = kdata_.getPos(today.datetime);
  const size_t positionPos = kdata_.getPos(position.takeDatetime);
  const price_t profit =
      position.number * rawToday.closePrice - position.totalCost;
  if (pos - positionPos >= getParam<int>("tp_delay_n") &&
      currentPrice <= takeProfit &&
      profit > (position.buyMoney - position.sellMoney)) {
    return sell(today, rawToday, OrderOrigin::TAKE_PROFIT);
  }
  return {};
}

TradeRecord StrategyRuntime::buy(const KRecord& today, const KRecord& src_today,
                                 OrderOrigin from) {
  TradeRecord result;

  bool trace = getParam<bool>("trace");

  // A delayed buy
  if (getParam<bool>("buy_delay")) {
    submitBuyRequest(today, src_today, from);
    HAYAKU_INFO_IF(trace, htr("[{}] will be delay to buy", name_));
    return result;
  }

  // Check whether it is a one-line limit up board
  if (today.highPrice == today.lowPrice) {
    if (getParam<bool>("can_trade_when_high_eq_low")) {
      HAYAKU_WARN_IF(trace, htr("[{}] buy one-price board", name_));
      return buyNow(today, src_today, from);
    }

    // Get yesterday's close price and check whether it is a one-line limit up
    size_t pos = kdata_.getPos(today.datetime);
    if (pos == 0 || pos == Null<size_t>()) {
      HAYAKU_INFO_IF(trace, htr("[{}] delay to buy, one-price board", name_));
      submitBuyRequest(today, src_today, from);
      return result;
    }

    const auto& pre_day = kdata_.getKRecord(pos - 1);
    if (today.closePrice > pre_day.closePrice) {
      HAYAKU_INFO_IF(
          trace, htr("[{}] delay to buy, one-price up-limit board", name_));
      submitBuyRequest(today, src_today, from);
      return result;
    }
  }

  // Delay the trade when the volume and the turnover amount are 0
  if (iszero(today.transAmount) || iszero(today.transCount)) {
    HAYAKU_INFO_IF(
        trace,
        htr("[{}] delay to buy, current amount == 0 or count == 0", name_));
    submitBuyRequest(today, src_today, from);
    return result;
  }

  return buyNow(today, src_today, from);
}

TradeRecord StrategyRuntime::buyNow(const KRecord& today,
                                    const KRecord& src_today,
                                    OrderOrigin from) {
  TradeRecord result;

  // Take the current close price as the planned price
  price_t planPrice = src_today.closePrice;

  // Calculate the stop-loss price
  price_t stoploss = getStoplossPrice(today, src_today, today.closePrice);

  // Give up the trade when the planned price is not higher than the stop-loss
  // price
  bool trace = getParam<bool>("trace");
  if (planPrice <= stoploss) {
    HAYAKU_INFO_IF(trace, htr("[{}] buy failed, planPrice: {} <= stoploss: {}",
                              name_, planPrice, stoploss));
    return result;
  }

  // Get the buyable quantity
  double number =
      getBuyNumber(today.datetime, planPrice, planPrice - stoploss, from);
  double min_num = stock_.minTradeNumber();
  HAYAKU_ASSERT(min_num != 0.0);
  number = int64_t(number / min_num) * min_num;
  if (iszero(number) || number > stock_.maxTradeNumber()) {
    HAYAKU_INFO_IF(
        trace, "[{}] {}, number: {} == 0 or > maxTradeNumber: {}, {}", name_,
        htr("buy failed"), number, stock_.maxTradeNumber(), mm_);
    return result;
  }

  price_t realPrice = getRealBuyPrice(today.datetime, planPrice);
  price_t goalPrice = getGoalPrice(today.datetime, planPrice);
  TradeRecord record = submitStrategyOrder(
      execution_,
      OrderRequest(OrderSide::BUY, today.datetime, stock_, realPrice, number,
                   stoploss, goalPrice, planPrice, from));
  if (BUSINESS_BUY != record.business) {
    HAYAKU_INFO_IF(trace, htr("[{}] buy failed, {}", name_, record));
    return result;
  }

  last_take_profit_ = record.realPrice;
  trades_.push_back(record);
  buyNotifyAll(record);
  return record;
}

TradeRecord StrategyRuntime::buyDelay(const KRecord& today,
                                      const KRecord& src_today) {
  TradeRecord result;
  bool trace = getParam<bool>("trace");

  // Delay the trade when the volume and the turnover amount are 0
  if (iszero(today.transAmount) || iszero(today.transCount)) {
    HAYAKU_INFO_IF(
        trace,
        htr("[{}] delay to buy, current amount == 0 or count == 0", name_));
    submitBuyRequest(today, src_today, pending_orders_.buy().origin);
    return result;
  }

  if (today.highPrice == today.lowPrice &&
      !getParam<bool>("can_trade_when_high_eq_low")) {
    // Get yesterday's close price and check whether it is a one-line limit up
    size_t pos = kdata_.getPos(today.datetime);
    if (pos == 0 || pos == Null<size_t>()) {
      HAYAKU_INFO_IF(trace, htr("[{}] delay to buy, one-price board", name_));
      submitBuyRequest(today, src_today, pending_orders_.buy().origin);
      return result;
    }

    const auto& pre_day = kdata_.getKRecord(pos - 1);
    if (today.closePrice > pre_day.closePrice) {
      HAYAKU_INFO_IF(
          trace, htr("[{}] delay to buy, one-price up-limit board", name_));
      submitBuyRequest(today, src_today, pending_orders_.buy().origin);
      return result;
    }
  }

  // A delayed operation, take the open price of the current moment
  price_t planPrice =
      src_today.openPrice;  // Take the open price of the current moment

  // Calculate the stop-loss price and the buyable quantity
  price_t stoploss = 0.0;
  double number = 0.0;
  price_t goalPrice = 0.0;
  if (getParam<bool>("delay_use_current_price")) {
    // Calculate the stop-loss price and the buyable quantity with the current
    // planned price
    stoploss = getStoplossPrice(today, src_today, today.openPrice);
    number = planPrice <= stoploss
                 ? 0.0
                 : getBuyNumber(today.datetime, planPrice, planPrice - stoploss,
                                pending_orders_.buy().origin);
    goalPrice = getGoalPrice(today.datetime, planPrice);

  } else {
    stoploss = pending_orders_.buy().stoploss;
    number = pending_orders_.buy().number;
    goalPrice = pending_orders_.buy().goal;
  }

  // If the planned buy price is not higher than the stop-loss price or the buy
  // quantity is 0
  if (planPrice <= stoploss || number <= 0) {
    pending_orders_.buy().clear();
    return result;
  }

  double min_num = stock_.minTradeNumber();
  number = int64_t(number / min_num) * min_num;

  price_t realPrice = getRealBuyPrice(today.datetime, planPrice);
  TradeRecord record = submitStrategyOrder(
      execution_, OrderRequest(OrderSide::BUY, today.datetime, stock_,
                                realPrice, number, stoploss, goalPrice,
                                planPrice, pending_orders_.buy().origin));
  if (BUSINESS_BUY != record.business) {
    pending_orders_.buy().clear();
    return result;
  }

  buy_days_ = 0;
  last_take_profit_ = record.realPrice;
  trades_.push_back(record);
  buyNotifyAll(record);
  pending_orders_.buy().clear();
  return record;
}

void StrategyRuntime::submitBuyRequest(const KRecord& today,
                                       const KRecord& src_today,
                                       OrderOrigin from) {
  if (pending_orders_.buy().valid) {
    if (pending_orders_.buy().count > getParam<int>("max_delay_count")) {
      // The maximum number of the delays has been exceeded, clear the buy
      // request
      pending_orders_.buy().clear();
      return;
    }
    pending_orders_.buy().count++;

  } else {
    pending_orders_.buy().valid = true;
    pending_orders_.buy().business = BUSINESS_BUY;
    pending_orders_.buy().origin = from;
    pending_orders_.buy().count = 1;
  }

  pending_orders_.buy().datetime = today.datetime;
  pending_orders_.buy().stoploss =
      getStoplossPrice(today, src_today, today.closePrice);
  pending_orders_.buy().goal =
      getGoalPrice(today.datetime, src_today.closePrice);
  pending_orders_.buy().number =
      getBuyNumber(today.datetime, src_today.closePrice,
                   src_today.closePrice - pending_orders_.buy().stoploss,
                   pending_orders_.buy().origin);
}

TradeRecord StrategyRuntime::sellForce(const Datetime& date, double num,
                                       OrderOrigin from, bool on_open) {
  bool trace = getParam<bool>("trace");
  HAYAKU_INFO_IF(trace, "[{}] {} {} by {}", name_, htr("force sell"), num,
                 getOrderOriginName(from));

  TradeRecord record;
  size_t pos = kdata_.getPos(date);
  HAYAKU_TRACE_IF_RETURN(pos == Null<size_t>(), record,
                         "Failed to sellForce {}, the day {} could'nt sell!",
                         stock_.market_code(), date);

  PositionRecord position = account_->getPosition(date, stock_);
  HAYAKU_IF_RETURN(position.number <= 0.0, record);

  const auto& krecord = kdata_.getKRecord(pos);
  const auto& src_krecord =
      stock_.getKRecord(kdata_.startPos() + pos, kdata_.getQuery().kType());

  price_t realPrice =
      getRealSellPrice(krecord.datetime, on_open ? src_krecord.openPrice
                                                 : src_krecord.closePrice);

  double min_num = stock_.minTradeNumber();
  // Round the quantity to be sold to an integer multiple of the minimum trade
  // unit; when the remainder is less than the minimum trade unit, sell
  // everything at once
  double realsell_num = static_cast<int64_t>(num / min_num) * min_num;
  if (position.number - realsell_num < min_num) {
    realsell_num = position.number;
  }

  record = submitStrategyOrder(
      execution_,
      OrderRequest(OrderSide::SELL, date, stock_, realPrice, realsell_num,
                   position.stoploss, position.goalPrice,
                   on_open ? src_krecord.openPrice : src_krecord.closePrice,
                   from));
  HAYAKU_WARN_IF_RETURN(record == Null<TradeRecord>(), record,
                        "[{}] {}: {} by {}", name_, htr("Failed force sell"),
                        num, getOrderOriginName(from));

  // The last take-profit price is initialized to 0 when there is no position
  if (!account_->have(stock_)) {
    last_take_profit_ = 0.0;
  }

  trades_.push_back(record);
  sellNotifyAll(record);
  return record;
}

TradeRecord StrategyRuntime::sell(const KRecord& today,
                                  const KRecord& src_today, OrderOrigin from) {
  bool trace = getParam<bool>("trace");
  TradeRecord result;
  if (getParam<bool>("sell_delay")) {
    submitSellRequest(today, src_today, from);
    HAYAKU_INFO_IF(trace, htr("[{}] will be delay to sell", name_));
    return result;
  }

  // Check whether it may be a one-line limit down
  if (today.highPrice == today.lowPrice) {
    if (getParam<bool>("can_trade_when_high_eq_low")) {
      HAYAKU_WARN_IF(trace, htr("[{}] sell one-price board", name_));
      return sellNow(today, src_today, from);
    }

    // Get yesterday's data and check whether it is a one-line limit down; on a
    // one-line limit down the sell is delayed
    size_t pos = kdata_.getPos(today.datetime);
    if (pos == 0 || pos == Null<size_t>()) {
      HAYAKU_INFO_IF(trace, htr("[{}] delay to sell, one-price board", name_));
      submitSellRequest(today, src_today, from);
      return result;
    }

    const auto& preday = kdata_.getKRecord(pos - 1);
    if (today.closePrice < preday.closePrice) {
      HAYAKU_INFO_IF(trace, htr("[{}] sell delayed: limit-down lock", name_));
      submitSellRequest(today, src_today, from);
      return result;
    }
  }

  if (iszero(today.transAmount) || iszero(today.transCount)) {
    HAYAKU_INFO_IF(
        trace,
        htr("[{}] delay to sell, current amount == 0 or count == 0", name_));
    submitSellRequest(today, src_today, from);
    return result;
  }

  result = sellNow(today, src_today, from);
  HAYAKU_INFO_IF(trace, htr("[{}] sell now: {}", name_, result));
  return result;
}

TradeRecord StrategyRuntime::sellNow(const KRecord& today,
                                     const KRecord& src_today,
                                     OrderOrigin from) {
  TradeRecord result;
  price_t planPrice = src_today.closePrice;
  double number = 0;

  // Calculate the new stop-loss price
  price_t stoploss = getStoplossPrice(today, src_today, today.closePrice);

  // When the new planned price is not higher than the new stop-loss price, the
  // whole position is to be sold
  number = getSellNumber(today.datetime, planPrice, planPrice - stoploss, from);
  if (number <= 0) {
    return result;
  }

  price_t goalPrice = getGoalPrice(today.datetime, planPrice);
  price_t realPrice = getRealSellPrice(today.datetime, planPrice);
  TradeRecord record = submitStrategyOrder(
      execution_,
      OrderRequest(OrderSide::SELL, today.datetime, stock_, realPrice, number,
                   stoploss, goalPrice, planPrice, from));
  if (BUSINESS_SELL != record.business) {
    return result;  // The sell operation failed
  }

  // The last take-profit price is initialized to 0 when there is no position
  if (!account_->have(stock_)) {
    last_take_profit_ = 0.0;
  } else {
    last_take_profit_ = src_today.closePrice;
  }

  trades_.push_back(record);
  sellNotifyAll(record);
  return record;
}

TradeRecord StrategyRuntime::sellDelay(const KRecord& today,
                                       const KRecord& src_today) {
  bool trace = getParam<bool>("trace");
  TradeRecord result;
  if (iszero(today.transAmount) || iszero(today.transCount)) {
    HAYAKU_INFO_IF(
        trace,
        htr("[{}] delay to sell, current amount == 0 or count == 0", name_));
    submitSellRequest(today, src_today, pending_orders_.sell().origin);
    return result;
  }

  if (today.highPrice == today.lowPrice &&
      !getParam<bool>("can_trade_when_high_eq_low")) {
    // Get yesterday's data and check whether it is a one-line limit down; on a
    // one-line limit down the sell is delayed
    size_t pos = kdata_.getPos(today.datetime);
    if (pos == 0 || pos == Null<size_t>()) {
      HAYAKU_INFO_IF(trace, htr("[{}] delay to sell, one-price board", name_));
      submitSellRequest(today, src_today, pending_orders_.sell().origin);
      return result;
    }

    const auto& preday = kdata_.getKRecord(pos - 1);
    if (today.closePrice < preday.closePrice) {
      HAYAKU_INFO_IF(trace, htr("[{}] sell delayed: limit-down lock", name_));
      submitSellRequest(today, src_today, pending_orders_.sell().origin);
      return result;
    }
  }

  price_t planPrice =
      src_today.openPrice;  // Take the open price of the current moment

  // The stop-loss price at the moment the sell request is issued
  price_t stoploss = 0.0;
  double number = 0.0;
  price_t goalPrice = 0.0;

  OrderOrigin from = pending_orders_.sell().origin;

  if (getParam<bool>("delay_use_current_price")) {
    stoploss = getStoplossPrice(today, src_today, today.openPrice);
    number =
        getSellNumber(today.datetime, planPrice, planPrice - stoploss, from);
    goalPrice = getGoalPrice(today.datetime, planPrice);
  } else {
    stoploss = pending_orders_.sell().stoploss;
    number = pending_orders_.sell().number;
    goalPrice = pending_orders_.sell().goal;
  }

  if (number <= 0) {
    pending_orders_.sell().clear();
    return result;
  }

  price_t realPrice = getRealSellPrice(today.datetime, planPrice);
  TradeRecord record = submitStrategyOrder(
      execution_, OrderRequest(OrderSide::SELL, today.datetime, stock_,
                                realPrice, number, stoploss, goalPrice,
                                planPrice, pending_orders_.sell().origin));
  if (BUSINESS_SELL != record.business) {
    pending_orders_.sell().clear();
    return result;  // The sell operation failed
  }

  // The last take-profit price is initialized to 0 when there is no position
  if (!account_->have(stock_)) {
    last_take_profit_ = 0.0;
  } else {
    last_take_profit_ = src_today.openPrice;
  }

  trades_.push_back(record);
  sellNotifyAll(record);
  pending_orders_.sell().clear();
  return record;
}

void StrategyRuntime::submitSellRequest(const KRecord& today,
                                        const KRecord& src_today,
                                        OrderOrigin from) {
  if (pending_orders_.sell().valid) {
    if (pending_orders_.sell().count > getParam<int>("max_delay_count")) {
      // The maximum number of the delays has been exceeded, clear the buy
      // request
      pending_orders_.sell().clear();
      return;
    }
    pending_orders_.sell().count++;

  } else {
    pending_orders_.sell().valid = true;
    pending_orders_.sell().business = BUSINESS_SELL;
    pending_orders_.sell().count = 1;
  }

  pending_orders_.sell().origin = from;
  pending_orders_.sell().datetime = today.datetime;
  pending_orders_.sell().stoploss =
      getStoplossPrice(today, src_today, today.closePrice);
  if (src_today.closePrice <= pending_orders_.sell().stoploss) {
    pending_orders_.sell().number =
        account_->getHoldNumber(today.datetime, stock_);
  } else {
    pending_orders_.sell().number = getSellNumber(
        today.datetime, src_today.closePrice,
        src_today.closePrice - pending_orders_.sell().stoploss, from);
  }

  pending_orders_.sell().goal =
      getGoalPrice(today.datetime, src_today.closePrice);
}

TradeRecord StrategyRuntime::buyShort(const KRecord& today,
                                      const KRecord& src_today,
                                      OrderOrigin from) {
  TradeRecord result;
  if (getParam<bool>("support_borrow_stock") == false) return result;

  bool trace = getParam<bool>("trace");
  if (getParam<bool>("buy_delay")) {
    submitBuyShortRequest(today, src_today, from);
    HAYAKU_INFO_IF(trace, htr("[{}] will buy short next bar open", name_));
    return result;
  }

  // The one-line board case
  if (today.highPrice == today.lowPrice) {
    if (getParam<bool>("can_trade_when_high_eq_low")) {
      HAYAKU_WARN_IF(trace, htr("[{}] buy short one-price board", name_));
      return buyShortNow(today, src_today, from);
    }

    // Get yesterday's data and check whether it is a one-line limit up,
    size_t pos = kdata_.getPos(today.datetime);
    if (pos == 0 || pos == Null<size_t>()) {
      HAYAKU_INFO_IF(trace,
                     htr("[{}] delay to buy short, one-price board", name_));
      submitBuyShortRequest(today, src_today, from);
      return result;
    }

    const auto& preday = kdata_.getKRecord(pos - 1);
    if (today.closePrice > preday.closePrice) {
      HAYAKU_INFO_IF(trace,
                     htr("[{}] short covering delayed: limit-up lock", name_));
      submitBuyShortRequest(today, src_today, from);
      return result;
    }
  }

  if (iszero(today.transAmount) || iszero(today.transCount)) {
    HAYAKU_INFO_IF(
        trace, htr("[{}] delay to buy short, current amount == 0 or count == 0",
                   name_));
    submitBuyShortRequest(today, src_today, from);
    return result;
  }

  return buyShortNow(today, src_today, from);
}

TradeRecord StrategyRuntime::buyShortNow(const KRecord& today,
                                         const KRecord& src_today,
                                         OrderOrigin from) {
  TradeRecord result;
  price_t planPrice =
      src_today.closePrice;  // Take the close price of the current moment

  // Take the stop-loss price corresponding to the close price of the current
  // moment
  price_t stoploss = getShortStoplossPrice(today, src_today, today.closePrice);

  // Determine the quantity
  double number =
      getBuyShortNumber(today.datetime, planPrice, stoploss - planPrice, from);
  if (number <= 0) {
    pending_orders_.buyShort().clear();
    return result;
  }

  // Get the holding status of the current short position
  PositionRecord pos = account_->getShortPosition(stock_);
  if (pos.number <= 0) {
    pending_orders_.buyShort().clear();
    return result;
  }

  if (number > pos.number) {
    number = pos.number;
  }

  price_t goalPrice = getShortGoalPrice(today.datetime, planPrice);
  price_t realPrice = getRealBuyPrice(today.datetime, planPrice);

  TradeRecord record = submitStrategyOrder(
      execution_, OrderRequest(OrderSide::BUY_SHORT, today.datetime, stock_,
                                realPrice, number, stoploss, goalPrice,
                                planPrice, OrderOrigin::SIGNAL));
  if (BUSINESS_BUY_SHORT != record.business) {
    pending_orders_.buyShort().clear();
    return result;
  }

  sell_short_days_ = 0;
  last_take_profit_ = realPrice;  // The take-profit is assigned the buy price
  trades_.push_back(record);
  buyNotifyAll(record);
  pending_orders_.buyShort().clear();
  return record;
}

TradeRecord StrategyRuntime::buyShortDelay(const KRecord& today,
                                           const KRecord& src_today) {
  TradeRecord result;
  bool trace = getParam<bool>("trace");
  if (iszero(today.transAmount) || iszero(today.transCount)) {
    HAYAKU_INFO_IF(
        trace, htr("[{}] delay to buy short, current amount == 0 or count == 0",
                   name_));
    submitBuyShortRequest(today, src_today, pending_orders_.buyShort().origin);
    return result;
  }

  if (today.highPrice == today.lowPrice &&
      !getParam<bool>("can_trade_when_high_eq_low")) {
    // Get yesterday's data and check whether it is a one-line limit up,
    size_t pos = kdata_.getPos(today.datetime);
    if (pos == 0 || pos == Null<size_t>()) {
      HAYAKU_INFO_IF(trace,
                     htr("[{}] delay to buy short, one-price board", name_));
      submitBuyShortRequest(today, src_today,
                            pending_orders_.buyShort().origin);
      return result;
    }

    const auto& preday = kdata_.getKRecord(pos - 1);
    if (today.closePrice > preday.closePrice) {
      HAYAKU_INFO_IF(trace,
                     htr("[{}] short covering delayed: limit-up lock", name_));
      submitBuyShortRequest(today, src_today,
                            pending_orders_.buyShort().origin);
      return result;
    }
  }

  price_t planPrice =
      src_today.openPrice;  // Take the close price of the current moment

  price_t stoploss = 0.0;
  double number = 0.0;
  price_t goalPrice = 0.0;
  if (getParam<bool>("delay_use_current_price")) {
    // Take the stop-loss price corresponding to the close price of the current
    // moment
    stoploss = getShortStoplossPrice(today, src_today, today.openPrice);
    number = getBuyShortNumber(today.datetime, planPrice, stoploss - planPrice,
                               pending_orders_.buy().origin);
    goalPrice = getShortGoalPrice(today.datetime, planPrice);

  } else {
    stoploss = pending_orders_.buyShort().stoploss;
    number = pending_orders_.buyShort().number;
    goalPrice = pending_orders_.buyShort().goal;
  }

  if (number <= 0) {
    pending_orders_.buyShort().clear();
    return result;
  }

  // Get the holding status of the current short position
  PositionRecord pos = account_->getShortPosition(stock_);
  if (pos.number <= 0) {
    pending_orders_.buyShort().clear();
    return result;
  }

  if (number > pos.number) {
    number = pos.number;
  }

  price_t realPrice = getRealBuyPrice(today.datetime, planPrice);
  TradeRecord record = submitStrategyOrder(
      execution_, OrderRequest(OrderSide::BUY_SHORT, today.datetime, stock_,
                                realPrice, number, stoploss, goalPrice,
                                planPrice, OrderOrigin::SIGNAL));
  if (BUSINESS_BUY_SHORT != record.business) {
    pending_orders_.buyShort().clear();
    return result;
  }

  sell_short_days_ = 0;
  last_take_profit_ = realPrice;  // The take-profit is assigned the buy price
  trades_.push_back(record);
  buyNotifyAll(record);
  pending_orders_.buyShort().clear();
  return result;
}

void StrategyRuntime::submitBuyShortRequest(const KRecord& today,
                                            const KRecord& src_today,
                                            OrderOrigin from) {
  if (pending_orders_.buyShort().valid) {
    if (pending_orders_.buyShort().count > getParam<int>("max_delay_count")) {
      // The maximum number of the delays has been exceeded, clear the buy
      // request
      pending_orders_.buy().clear();
      return;
    }
    pending_orders_.buyShort().count++;

  } else {
    pending_orders_.buyShort().valid = true;
    pending_orders_.buyShort().business = BUSINESS_BUY;
    pending_orders_.buyShort().origin = from;
    pending_orders_.buyShort().count = 1;
  }

  pending_orders_.buyShort().datetime = today.datetime;
  pending_orders_.buyShort().stoploss =
      getShortStoplossPrice(today, src_today, today.closePrice);
  pending_orders_.buyShort().goal =
      getShortGoalPrice(today.datetime, src_today.closePrice);
  pending_orders_.buyShort().number = getBuyShortNumber(
      today.datetime, src_today.closePrice,
      pending_orders_.buyShort().stoploss - src_today.closePrice,
      pending_orders_.buyShort().origin);
}

TradeRecord StrategyRuntime::sellShort(const KRecord& today,
                                       const KRecord& src_today,
                                       OrderOrigin from) {
  TradeRecord result;
  bool trace = getParam<bool>("trace");
  if (getParam<bool>("support_borrow_stock") == false) {
    // HAYAKU_WARN("set system param support_borrow_stock to true to short
    // sell");
    return result;
  }

  if (getParam<bool>("sell_delay")) {
    submitSellShortRequest(today, src_today, from);
    return result;
  }

  if (today.highPrice == today.lowPrice) {
    if (getParam<bool>("can_trade_when_high_eq_low")) {
      HAYAKU_WARN_IF(trace, htr("[{}] buy short one-price board", name_));
      return sellShortNow(today, src_today, from);
    }

    size_t pos = kdata_.getPos(today.datetime);
    if (pos == 0 || pos == Null<size_t>()) {
      HAYAKU_INFO_IF(trace,
                     htr("[{}] delay to sell short, one-price board", name_));
      submitSellShortRequest(today, src_today, from);
      return result;
    }

    const auto& preday = kdata_.getKRecord(pos - 1);
    if (today.closePrice < preday.closePrice) {
      HAYAKU_INFO_IF(
          trace, htr("[{}] short selling delayed: limit-down lock", name_));
      submitSellShortRequest(today, src_today, from);
      return result;
    }
  }

  if (iszero(today.transAmount) || iszero(today.transCount)) {
    HAYAKU_INFO_IF(
        trace,
        htr("[{}] delay to sell short, current amount == 0 or count == 0",
            name_));
    submitSellShortRequest(today, src_today, from);
    return result;
  }

  return sellShortNow(today, src_today, from);
}

TradeRecord StrategyRuntime::sellShortNow(const KRecord& today,
                                          const KRecord& src_today,
                                          OrderOrigin from) {
  TradeRecord result;
  if (today.highPrice == today.lowPrice &&
      !getParam<bool>("can_trade_when_high_eq_low")) {
    // It cannot be sold at the moment, delay the sell to the next moment
    submitSellShortRequest(today, src_today, from);
    return result;
  }

  price_t planPrice = src_today.closePrice;

  // Calculate the stop-loss price
  price_t stoploss = getShortStoplossPrice(today, src_today, today.closePrice);

  double number =
      getSellShortNumber(today.datetime, planPrice, stoploss - planPrice, from);
  if (number <= 0) {
    pending_orders_.sellShort().clear();
    return result;
  }

  price_t goalPrice = getShortGoalPrice(today.datetime, planPrice);
  price_t realPrice = getRealSellPrice(today.datetime, planPrice);
  TradeRecord record = submitStrategyOrder(
      execution_, OrderRequest(OrderSide::SELL_SHORT, today.datetime, stock_,
                                realPrice, number, stoploss, goalPrice,
                                planPrice, OrderOrigin::SIGNAL));
  if (BUSINESS_SELL_SHORT != record.business) {
    pending_orders_.sellShort().clear();
    return result;  // The sell operation failed
  }

  sell_short_days_ = 0;
  last_short_take_profit_ = realPrice;
  trades_.push_back(record);
  sellNotifyAll(record);
  pending_orders_.sellShort().clear();
  return record;
}

TradeRecord StrategyRuntime::sellShortDelay(const KRecord& today,
                                            const KRecord& src_today) {
  TradeRecord result;
  bool trace = getParam<bool>("trace");
  if (iszero(today.transAmount) || iszero(today.transCount)) {
    HAYAKU_INFO_IF(
        trace,
        htr("[{}] delay to sell short, current amount == 0 or count == 0",
            name_));
    submitSellShortRequest(today, src_today,
                           pending_orders_.sellShort().origin);
    return result;
  }

  if (today.highPrice == today.lowPrice &&
      !getParam<bool>("can_trade_when_high_eq_low")) {
    size_t pos = kdata_.getPos(today.datetime);
    if (pos == 0 || pos == Null<size_t>()) {
      HAYAKU_INFO_IF(trace,
                     htr("[{}] delay to sell short, one-price board", name_));
      submitSellShortRequest(today, src_today,
                             pending_orders_.sellShort().origin);
      return result;
    }

    const auto& preday = kdata_.getKRecord(pos - 1);
    if (today.closePrice < preday.closePrice) {
      HAYAKU_INFO_IF(
          trace, htr("[{}] short selling delayed: limit-down lock", name_));
      submitSellShortRequest(today, src_today,
                             pending_orders_.sellShort().origin);
      return result;
    }
  }

  price_t planPrice =
      src_today.openPrice;  // Take the open price of the current moment

  // The stop-loss price at the moment the sell request is issued
  price_t stoploss = 0.0;
  double number = 0;
  price_t goalPrice = 0.0;
  if (getParam<bool>("delay_use_current_price")) {
    stoploss = getShortStoplossPrice(today, src_today, today.openPrice);
    number = getSellShortNumber(today.datetime, planPrice, stoploss - planPrice,
                                pending_orders_.sellShort().origin);
    goalPrice = getShortGoalPrice(today.datetime, planPrice);
  } else {
    stoploss = pending_orders_.sellShort().stoploss;
    number = pending_orders_.sellShort().number;
    goalPrice = pending_orders_.sellShort().goal;
  }

  if (number <= 0) {
    pending_orders_.sellShort().clear();
    return result;
  }

  price_t realPrice = getRealSellPrice(today.datetime, planPrice);

  TradeRecord record = submitStrategyOrder(
      execution_, OrderRequest(OrderSide::SELL_SHORT, today.datetime, stock_,
                                realPrice, number, stoploss, goalPrice,
                                planPrice, pending_orders_.sellShort().origin));
  if (BUSINESS_SELL_SHORT != record.business) {
    pending_orders_.sellShort().clear();
    return result;  // The sell operation failed
  }

  sell_short_days_ = 0;
  last_short_take_profit_ = realPrice;
  trades_.push_back(record);
  sellNotifyAll(record);
  pending_orders_.sellShort().clear();
  return record;
}

void StrategyRuntime::submitSellShortRequest(const KRecord& today,
                                             const KRecord& src_today,
                                             OrderOrigin from) {
  if (pending_orders_.sellShort().valid) {
    if (pending_orders_.sellShort().count > getParam<int>("max_delay_count")) {
      // The maximum number of the delays has been exceeded, clear the buy
      // request
      pending_orders_.sellShort().clear();
      return;
    }
    pending_orders_.sellShort().count++;

  } else {
    pending_orders_.sellShort().valid = true;
    pending_orders_.sellShort().business = BUSINESS_SELL_SHORT;
    pending_orders_.sellShort().origin = from;
    pending_orders_.sellShort().count = 1;
  }

  pending_orders_.sellShort().datetime = today.datetime;
  pending_orders_.sellShort().stoploss =
      getStoplossPrice(today, src_today, today.closePrice);
  pending_orders_.sellShort().goal =
      getGoalPrice(today.datetime, src_today.closePrice);
  pending_orders_.sellShort().number =
      getSellNumber(today.datetime, src_today.closePrice,
                    src_today.closePrice - pending_orders_.sellShort().stoploss,
                    pending_orders_.sellShort().origin);
}

TradeRecord StrategyRuntime::processRequest(const KRecord& today,
                                            const KRecord& src_today) {
  HAYAKU_IF_RETURN(pending_orders_.buy().valid, buyDelay(today, src_today));
  HAYAKU_IF_RETURN(pending_orders_.sell().valid, sellDelay(today, src_today));
  HAYAKU_IF_RETURN(pending_orders_.sellShort().valid,
                   sellShortDelay(today, src_today));
  HAYAKU_IF_RETURN(pending_orders_.buyShort().valid,
                   buyShortDelay(today, src_today));
  return TradeRecord();
}

price_t StrategyRuntime::getStoplossPrice(const KRecord& today,
                                          const KRecord& src_today,
                                          price_t price) {
  HAYAKU_IF_RETURN(!st_, 0.0);
  HAYAKU_IF_RETURN(today.highPrice == today.lowPrice, src_today.lowPrice);
  price_t stoploss = st_->getPrice(today.datetime, price);
  price_t adjust = (stoploss - today.lowPrice) /
                       (today.highPrice - today.lowPrice) *
                       (src_today.highPrice - src_today.lowPrice) +
                   src_today.lowPrice;
  return adjust >= 0.0 ? adjust : 0.0;
}

price_t StrategyRuntime::getShortStoplossPrice(const KRecord& today,
                                               const KRecord& src_today,
                                               price_t price) {
  HAYAKU_IF_RETURN(!st_, 0.0);
  HAYAKU_IF_RETURN(today.highPrice == today.lowPrice, src_today.lowPrice);
  price_t stoploss = st_->getShortPrice(today.datetime, price);
  price_t adjust = (stoploss - today.lowPrice) /
                       (today.highPrice - today.lowPrice) *
                       (src_today.highPrice - src_today.lowPrice) +
                   src_today.lowPrice;
  return adjust >= 0.0 ? adjust : 0.0;
}

}  // namespace hayaku::internal
