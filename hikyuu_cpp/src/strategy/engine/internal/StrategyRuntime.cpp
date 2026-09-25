/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "StrategyRuntime.h"

#include "ComponentContext.h"

namespace hku::internal {

namespace {
TradeRecord submitStrategyOrder(StrategyExecutionPort& execution, const OrderRequest& request) {
    return execution.submit(request).trade();
}
}  // namespace

StrategyRuntime::StrategyRuntime(const StrategyDefinition& definition,
                                 ExecutionAccountPortPtr account)
: m_account(std::move(account)),
  m_execution(m_account),
  m_mm(definition.moneyManager()),
  m_ev(definition.environment()),
  m_cn(definition.condition()),
  m_sg(definition.signal()),
  m_st(definition.stoploss()),
  m_tp(definition.takeProfit()),
  m_pg(definition.profitGoal()),
  m_sp(definition.slippage()),
  m_name(definition.name()) {
    HKU_CHECK(m_account, "StrategyRuntime requires an execution account");
    initParameters(definition.parameters());
}

void StrategyRuntime::initParameters(const Parameter& overrides) {
    m_parameters.set<bool>("trace", false);
    m_parameters.set<int>("max_delay_count", 3);
    m_parameters.set<bool>("buy_delay", true);
    m_parameters.set<bool>("sell_delay", true);
    m_parameters.set<bool>("delay_use_current_price", true);
    m_parameters.set<bool>("tp_monotonic", true);
    m_parameters.set<int>("tp_delay_n", 1);
    m_parameters.set<bool>("ignore_sell_sg", false);
    m_parameters.set<bool>("can_trade_when_high_eq_low", false);
    m_parameters.set<bool>("ev_open_position", false);
    m_parameters.set<bool>("cn_open_position", false);
    m_parameters.set<bool>("support_borrow_cash", false);
    m_parameters.set<bool>("support_borrow_stock", false);
    m_parameters.set<bool>("shared_account", false);
    m_parameters.set<bool>("shared_ev", true);
    m_parameters.set<bool>("shared_cn", false);
    m_parameters.set<bool>("shared_sg", false);
    m_parameters.set<bool>("shared_mm", false);
    m_parameters.set<bool>("shared_st", false);
    m_parameters.set<bool>("shared_tp", false);
    m_parameters.set<bool>("shared_pg", false);
    m_parameters.set<bool>("shared_sp", false);
    for (const auto& item : overrides) {
        m_parameters.set<boost::any>(item.first, item.second);
    }
}

void StrategyRuntime::resetState(bool all) {
    if (all || !getParam<bool>("shared_account")) {
        m_account->reset();
    }
    if (m_ev && (all || !getParam<bool>("shared_ev"))) {
        m_ev->reset();
    }
    if (m_cn && (all || !getParam<bool>("shared_cn"))) {
        m_cn->reset();
    }
    if (m_mm && (all || !getParam<bool>("shared_mm"))) {
        m_mm->reset();
    }
    if (m_sg && (all || !getParam<bool>("shared_sg"))) {
        m_sg->reset();
    }
    if (m_st && (all || !getParam<bool>("shared_st"))) {
        m_st->reset();
    }
    if (m_tp && (all || !getParam<bool>("shared_tp"))) {
        m_tp->reset();
    }
    if (m_pg && (all || !getParam<bool>("shared_pg"))) {
        m_pg->reset();
    }
    if (m_sp && (all || !getParam<bool>("shared_sp"))) {
        m_sp->reset();
    }

    if (all) {
        m_stock = Null<Stock>();
        m_kdata = Null<KData>();
        m_rawKData = Null<KData>();
    }
    m_calculated = false;
    m_preEnvironmentValid = !m_ev;
    m_preConditionValid = !m_cn;
    m_buyDays = 0;
    m_sellShortDays = 0;
    m_trades.clear();
    m_lastTakeProfit = 0.0;
    m_lastShortTakeProfit = 0.0;
    m_pendingOrders.clear();
}

void StrategyRuntime::prepare() {
    ComponentContext(*this).prepare();
}

void StrategyRuntime::bind(const KData& kdata) {
    ComponentContext(*this).bind(kdata);
}

void StrategyRuntime::run(const BacktestRequest& request) {
    if (request.resetAll()) {
        resetState(true);
    } else if (request.reset()) {
        resetState(false);
    }

    HKU_IF_RETURN(stopRequested(), void());
    HKU_DEBUG_IF_RETURN(m_calculated && m_kdata == request.kdata(), void(), "Not need calculate.");
    prepare();
    bind(request.kdata());

    const bool trace = getParam<bool>("trace");
    const size_t total = m_kdata.size();
    const auto* records = m_kdata.data();
    const auto* rawRecords = m_rawKData.data();
    HKU_ASSERT(m_kdata.size() == m_rawKData.size());

    Datetime initDatetime = m_execution.initDatetime();
    Datetime lastDatetime = m_execution.lastDatetime();
    if (KQuery::getKTypeInSeconds(m_kdata.getQuery().kType()) >= 86400) {
        initDatetime = initDatetime.startOfDay();
        lastDatetime = lastDatetime.startOfDay();
    }

    for (size_t i = 0; i < total; ++i) {
        if (stopRequested()) {
            m_calculated = false;
            break;
        }
        if (records[i].datetime < initDatetime || records[i].datetime < lastDatetime) {
            continue;
        }
        const auto trade = runMomentNative(records[i], rawRecords[i]);
        if (trace) {
            HKU_INFO_IF(!trade.isNull(), "{}", trade);
            const PositionRecord position = m_execution.position(records[i].datetime, m_stock);
            const FundsRecord funds = m_execution.funds(records[i].datetime,
                                                        m_kdata.getQuery().kType());
            if (position.number > 0.0) {
                HKU_INFO("total: {:.2f}, cash: {:.2f}, position: {:.2f}, close: {:.2f}",
                         funds.total_assets(), funds.cash, position.number,
                         rawRecords[i].closePrice);
            }
        }
    }
    if (!stopRequested()) {
        m_calculated = true;
    }
}

void StrategyRuntime::run(const KQuery& query, bool resetState, bool resetAll) {
    HKU_CHECK(!m_stock.isNull(), "Strategy stock is null");
    run(BacktestRequest(m_stock.getKData(query), resetState, resetAll));
}

void StrategyRuntime::run(const Stock& stock, const KQuery& query, bool resetState, bool resetAll) {
    HKU_CHECK(!stock.isNull(), "Strategy stock is null");
    run(BacktestRequest(stock.getKData(query), resetState, resetAll));
}

TradeRecord StrategyRuntime::runMoment(const Datetime& datetime) {
    const size_t pos = m_kdata.getPos(datetime);
    HKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());
    return runMomentNative(m_kdata.getKRecord(pos), m_rawKData.getKRecord(pos));
}

TradeRecord StrategyRuntime::runMomentOnOpen(const Datetime& datetime) {
    const size_t pos = m_kdata.getPos(datetime);
    HKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());
    return runMomentOnOpenNative(m_kdata.getKRecord(pos), m_rawKData.getKRecord(pos));
}

TradeRecord StrategyRuntime::runMomentOnClose(const Datetime& datetime) {
    const size_t pos = m_kdata.getPos(datetime);
    HKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());
    return runMomentOnCloseNative(m_kdata.getKRecord(pos), m_rawKData.getKRecord(pos));
}

TradeRecord StrategyRuntime::sellForceOnOpen(const Datetime& datetime, double number,
                                             OrderOrigin origin) {
    return sellForce(datetime, number, origin, true);
}

TradeRecord StrategyRuntime::sellForceOnClose(const Datetime& datetime, double number,
                                              OrderOrigin origin) {
    return sellForce(datetime, number, origin, false);
}

void StrategyRuntime::clearPendingBuy() {
    m_pendingOrders.buy().clear();
}

TradeRecord StrategyRuntime::processPendingBuy(const Datetime& datetime) {
    const size_t pos = m_kdata.getPos(datetime);
    HKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());
    return buyDelay(m_kdata.getKRecord(pos), m_rawKData.getKRecord(pos));
}

TradeRecord StrategyRuntime::processPendingSell(const Datetime& datetime) {
    const size_t pos = m_kdata.getPos(datetime);
    HKU_IF_RETURN(pos == Null<size_t>(), TradeRecord());
    return sellDelay(m_kdata.getKRecord(pos), m_rawKData.getKRecord(pos));
}

const PendingOrderState& StrategyRuntime::pendingOrders() const noexcept {
    return m_pendingOrders;
}

AccountId StrategyRuntime::accountId() const noexcept {
    return m_account->accountId();
}

const string& StrategyRuntime::name() const noexcept {
    return m_name;
}

void StrategyRuntime::name(string value) {
    m_name = std::move(value);
}

Stock StrategyRuntime::getStock() const {
    return m_stock;
}

void StrategyRuntime::setStock(const Stock& stock) {
    if (m_stock != stock) {
        m_stock = stock;
        m_calculated = false;
    }
}

KData StrategyRuntime::getTO() const {
    return m_kdata;
}

MoneyManagerPtr StrategyRuntime::getMM() const {
    return m_mm;
}

SignalPtr StrategyRuntime::getSG() const {
    return m_sg;
}

SlippagePtr StrategyRuntime::getSP() const {
    return m_sp;
}

void StrategyRuntime::setSP(SlippagePtr slippage) {
    m_sp = std::move(slippage);
    m_calculated = false;
}

PortfolioAccountPortPtr StrategyRuntime::getAccount() const {
    return std::dynamic_pointer_cast<PortfolioAccountPort>(m_account);
}

void StrategyRuntime::setAccount(PortfolioAccountPortPtr account) {
    HKU_CHECK(account, "StrategyRuntime requires an execution account");
    m_account = std::move(account);
    m_execution = StrategyExecutionPort(m_account);
    m_calculated = false;
}

StrategyRuntimePtr StrategyRuntime::clone() const {
    StrategyDefinition definition(m_mm ? m_mm->clone() : MoneyManagerPtr(),
                                  m_sg ? m_sg->clone() : SignalPtr(), m_name,
                                  m_ev ? m_ev->clone() : EnvironmentPtr(),
                                  m_cn ? m_cn->clone() : ConditionPtr(),
                                  m_st ? m_st->clone() : StoplossPtr(),
                                  m_tp ? m_tp->clone() : StoplossPtr(),
                                  m_pg ? m_pg->clone() : ProfitGoalPtr(),
                                  m_sp ? m_sp->clone() : SlippagePtr(), m_parameters);
    auto portfolioAccount = getAccount();
    ExecutionAccountPortPtr clonedAccount =
      getParam<bool>("shared_account") || !portfolioAccount ? m_account
                                                             : portfolioAccount->cloneAccount();
    auto result = std::make_shared<StrategyRuntime>(definition, std::move(clonedAccount));
    result->m_stock = m_stock;
    result->m_kdata = m_kdata;
    result->m_rawKData = m_rawKData;
    return result;
}

void StrategyRuntime::reset() {
    resetState(false);
}

void StrategyRuntime::forceResetAll() {
    resetState(true);
}

nlohmann::json StrategyRuntime::lastSuggestion() const {
    nlohmann::json result;
    result["name"] = m_name;
    result["stock"] = m_stock.isNull() ? nlohmann::json(nullptr)
                                        : nlohmann::json(m_stock.market_code());
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
    append(m_pendingOrders.buy());
    append(m_pendingOrders.sell());
    append(m_pendingOrders.sellShort());
    append(m_pendingOrders.buyShort());
    result["pending_orders"] = pending;
    return result;
}

const TradeRecordList& StrategyRuntime::trades() const noexcept {
    return m_trades;
}

void StrategyRuntime::setStopToken(const std::atomic_bool* stopToken) noexcept {
    m_stopToken = stopToken;
}

bool StrategyRuntime::stopRequested() const noexcept {
    return m_stopToken && m_stopToken->load(std::memory_order_acquire);
}

bool StrategyRuntime::environmentIsValid(const Datetime& datetime) {
    return m_ev ? m_ev->isValid(datetime) : true;
}

bool StrategyRuntime::conditionIsValid(const Datetime& datetime) {
    return m_cn ? m_cn->isValid(datetime) : true;
}

void StrategyRuntime::buyNotifyAll(const TradeRecord& record) {
    if (m_mm) {
        m_mm->buyNotify(record);
    }
    if (m_pg) {
        m_pg->buyNotify(record);
    }
}

void StrategyRuntime::sellNotifyAll(const TradeRecord& record) {
    if (m_mm) {
        m_mm->sellNotify(record);
    }
    if (m_pg) {
        m_pg->sellNotify(record);
    }
}

double StrategyRuntime::getBuyNumber(const Datetime& datetime, price_t price, price_t risk,
                                     OrderOrigin origin) {
    return m_mm ? m_mm->getBuyNumber(datetime, m_stock, price, risk, origin) : 0.0;
}

double StrategyRuntime::getSellNumber(const Datetime& datetime, price_t price, price_t risk,
                                      OrderOrigin origin) {
    return m_mm ? m_mm->getSellNumber(datetime, m_stock, price, risk, origin) : 0.0;
}

double StrategyRuntime::getSellShortNumber(const Datetime& datetime, price_t price, price_t risk,
                                           OrderOrigin origin) {
    return m_mm ? m_mm->getSellShortNumber(datetime, m_stock, price, risk, origin) : 0.0;
}

double StrategyRuntime::getBuyShortNumber(const Datetime& datetime, price_t price, price_t risk,
                                          OrderOrigin origin) {
    return m_mm ? m_mm->getBuyShortNumber(datetime, m_stock, price, risk, origin) : 0.0;
}

price_t StrategyRuntime::getTakeProfitPrice(const Datetime& datetime, price_t currentPrice) {
    return m_tp ? m_tp->getPrice(datetime, currentPrice) : 0.0;
}

price_t StrategyRuntime::getGoalPrice(const Datetime& datetime, price_t price) {
    return m_pg ? m_pg->getGoal(datetime, price) : Null<price_t>();
}

price_t StrategyRuntime::getShortGoalPrice(const Datetime& datetime, price_t price) {
    return m_pg ? m_pg->getShortGoal(datetime, price) : 0.0;
}

price_t StrategyRuntime::getRealBuyPrice(const Datetime& datetime, price_t planPrice) {
    return m_sp ? m_sp->getRealBuyPrice(datetime, planPrice) : planPrice;
}

price_t StrategyRuntime::getRealSellPrice(const Datetime& datetime, price_t planPrice) {
    return m_sp ? m_sp->getRealSellPrice(datetime, planPrice) : planPrice;
}

TradeRecord StrategyRuntime::runMomentNative(const KRecord& today, const KRecord& rawToday) {
    const TradeRecord openTrade = runMomentOnOpenNative(today, rawToday);
    const TradeRecord closeTrade = runMomentOnCloseNative(today, rawToday);
    return closeTrade.isNull() ? openTrade : closeTrade;
}

TradeRecord StrategyRuntime::runMomentOnOpenNative(const KRecord& today,
                                                   const KRecord& rawToday) {
    const bool trace = getParam<bool>("trace");
    if (trace) {
        HKU_INFO("{} ------------------------------------------------------", today.datetime);
        HKU_INFO(htr("[{}] cal today {}", m_name, today));
        HKU_INFO_IF(m_kdata.getQuery().recoverType() != KQuery::NO_RECOVER,
                    htr("[{}] raw today {}", m_name, rawToday));
    }
    ++m_buyDays;
    ++m_sellShortDays;
    HKU_DEBUG_IF_RETURN((today.closePrice > today.highPrice || today.closePrice < today.lowPrice ||
                         today.lowPrice > today.highPrice),
                        TradeRecord(), "[{}] ignore invalid price data at {}", m_name,
                        today.datetime);
    return processRequest(today, rawToday);
}

TradeRecord StrategyRuntime::runMomentOnCloseNative(const KRecord& today,
                                                    const KRecord& rawToday) {
    const bool trace = getParam<bool>("trace");
    const bool environmentValid = environmentIsValid(today.datetime);
    if (!environmentValid) {
        TradeRecord trade;
        if (m_account->have(m_stock)) {
            trade = sell(today, rawToday, OrderOrigin::ENVIRONMENT);
        }
        m_preEnvironmentValid = false;
        return trade;
    }
    if (!m_preEnvironmentValid && getParam<bool>("ev_open_position")) {
        m_preEnvironmentValid = true;
        return buy(today, rawToday, OrderOrigin::ENVIRONMENT);
    }
    m_preEnvironmentValid = true;

    const bool conditionValid = conditionIsValid(today.datetime);
    if (!conditionValid) {
        TradeRecord trade;
        if (m_account->have(m_stock)) {
            trade = sell(today, rawToday, OrderOrigin::CONDITION);
        }
        m_preConditionValid = false;
        return trade;
    }
    if (!m_preConditionValid && getParam<bool>("cn_open_position")) {
        m_preConditionValid = true;
        return buy(today, rawToday, OrderOrigin::CONDITION);
    }
    m_preConditionValid = true;

    if (m_sg->shouldBuy(today.datetime)) {
        return m_account->haveShort(m_stock) ? buyShort(today, rawToday, OrderOrigin::SIGNAL)
                                             : buy(today, rawToday, OrderOrigin::SIGNAL);
    }
    if (m_sg->shouldSell(today.datetime)) {
        return m_account->have(m_stock) ? sell(today, rawToday, OrderOrigin::SIGNAL)
                                        : sellShort(today, rawToday, OrderOrigin::SIGNAL);
    }

    const price_t currentPrice = today.closePrice;
    const price_t rawCurrentPrice = rawToday.closePrice;
    const PositionRecord position = m_account->getPosition(today.datetime, m_stock);
    HKU_INFO_IF(trace, htr("[{}] current position: {}", m_name, position.number));
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
    if (takeProfit < m_lastTakeProfit) {
        takeProfit = m_lastTakeProfit;
    } else {
        m_lastTakeProfit = takeProfit;
    }
    const size_t pos = m_kdata.getPos(today.datetime);
    const size_t positionPos = m_kdata.getPos(position.takeDatetime);
    const price_t profit = position.number * rawToday.closePrice - position.totalCost;
    if (pos - positionPos >= getParam<int>("tp_delay_n") && currentPrice <= takeProfit &&
        profit > (position.buyMoney - position.sellMoney)) {
        return sell(today, rawToday, OrderOrigin::TAKE_PROFIT);
    }
    return {};
}

TradeRecord StrategyRuntime::buy(const KRecord& today, const KRecord& src_today, OrderOrigin from) {
    TradeRecord result;

    bool trace = getParam<bool>("trace");

    // A delayed buy
    if (getParam<bool>("buy_delay")) {
        submitBuyRequest(today, src_today, from);
        HKU_INFO_IF(trace, htr("[{}] will be delay to buy", m_name));
        return result;
    }

    // Check whether it is a one-line limit up board
    if (today.highPrice == today.lowPrice) {
        if (getParam<bool>("can_trade_when_high_eq_low")) {
            HKU_WARN_IF(trace, htr("[{}] buy one-price board", m_name));
            return buyNow(today, src_today, from);
        }

        // Get yesterday's close price and check whether it is a one-line limit up
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to buy, one-price board", m_name));
            submitBuyRequest(today, src_today, from);
            return result;
        }

        const auto& pre_day = m_kdata.getKRecord(pos - 1);
        if (today.closePrice > pre_day.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] delay to buy, one-price up-limit board", m_name));
            submitBuyRequest(today, src_today, from);
            return result;
        }
    }

    // Delay the trade when the volume and the turnover amount are 0
    if (iszero(today.transAmount) || iszero(today.transCount)) {
        HKU_INFO_IF(trace, htr("[{}] delay to buy, current amount == 0 or count == 0", m_name));
        submitBuyRequest(today, src_today, from);
        return result;
    }

    return buyNow(today, src_today, from);
}

TradeRecord StrategyRuntime::buyNow(const KRecord& today, const KRecord& src_today, OrderOrigin from) {
    TradeRecord result;

    // Take the current close price as the planned price
    price_t planPrice = src_today.closePrice;

    // Calculate the stop-loss price
    price_t stoploss = getStoplossPrice(today, src_today, today.closePrice);

    // Give up the trade when the planned price is not higher than the stop-loss price
    bool trace = getParam<bool>("trace");
    if (planPrice <= stoploss) {
        HKU_INFO_IF(trace, htr("[{}] buy failed, planPrice: {} <= stoploss: {}", m_name, planPrice,
                               stoploss));
        return result;
    }

    // Get the buyable quantity
    double number = getBuyNumber(today.datetime, planPrice, planPrice - stoploss, from);
    double min_num = m_stock.minTradeNumber();
    HKU_ASSERT(min_num != 0.0);
    number = int64_t(number / min_num) * min_num;
    if (iszero(number) || number > m_stock.maxTradeNumber()) {
        HKU_INFO_IF(trace, "[{}] {}, number: {} == 0 or > maxTradeNumber: {}, {}", m_name,
                    htr("buy failed"), number, m_stock.maxTradeNumber(), m_mm);
        return result;
    }

    price_t realPrice = getRealBuyPrice(today.datetime, planPrice);
    price_t goalPrice = getGoalPrice(today.datetime, planPrice);
    TradeRecord record =
      submitStrategyOrder(m_execution, OrderRequest(OrderSide::BUY, today.datetime, m_stock, realPrice,
                                             number, stoploss, goalPrice, planPrice, from));
    if (BUSINESS_BUY != record.business) {
        HKU_INFO_IF(trace, htr("[{}] buy failed, {}", m_name, record));
        return result;
    }

    m_lastTakeProfit = record.realPrice;
    m_trades.push_back(record);
    buyNotifyAll(record);
    return record;
}

TradeRecord StrategyRuntime::buyDelay(const KRecord& today, const KRecord& src_today) {
    TradeRecord result;
    bool trace = getParam<bool>("trace");

    // Delay the trade when the volume and the turnover amount are 0
    if (iszero(today.transAmount) || iszero(today.transCount)) {
        HKU_INFO_IF(trace, htr("[{}] delay to buy, current amount == 0 or count == 0", m_name));
        submitBuyRequest(today, src_today, m_pendingOrders.buy().origin);
        return result;
    }

    if (today.highPrice == today.lowPrice && !getParam<bool>("can_trade_when_high_eq_low")) {
        // Get yesterday's close price and check whether it is a one-line limit up
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to buy, one-price board", m_name));
            submitBuyRequest(today, src_today, m_pendingOrders.buy().origin);
            return result;
        }

        const auto& pre_day = m_kdata.getKRecord(pos - 1);
        if (today.closePrice > pre_day.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] delay to buy, one-price up-limit board", m_name));
            submitBuyRequest(today, src_today, m_pendingOrders.buy().origin);
            return result;
        }
    }

    // A delayed operation, take the open price of the current moment
    price_t planPrice = src_today.openPrice;  // Take the open price of the current moment

    // Calculate the stop-loss price and the buyable quantity
    price_t stoploss = 0.0;
    double number = 0.0;
    price_t goalPrice = 0.0;
    if (getParam<bool>("delay_use_current_price")) {
        // Calculate the stop-loss price and the buyable quantity with the current planned price
        stoploss = getStoplossPrice(today, src_today, today.openPrice);
        number = planPrice <= stoploss
                   ? 0.0
                   : getBuyNumber(today.datetime, planPrice, planPrice - stoploss,
                                   m_pendingOrders.buy().origin);
        goalPrice = getGoalPrice(today.datetime, planPrice);

    } else {
        stoploss = m_pendingOrders.buy().stoploss;
        number = m_pendingOrders.buy().number;
        goalPrice = m_pendingOrders.buy().goal;
    }

    // If the planned buy price is not higher than the stop-loss price or the buy quantity is 0
    if (planPrice <= stoploss || number <= 0) {
        m_pendingOrders.buy().clear();
        return result;
    }

    double min_num = m_stock.minTradeNumber();
    number = int64_t(number / min_num) * min_num;

    price_t realPrice = getRealBuyPrice(today.datetime, planPrice);
    TradeRecord record = submitStrategyOrder(
      m_execution, OrderRequest(OrderSide::BUY, today.datetime, m_stock, realPrice, number, stoploss,
                         goalPrice, planPrice, m_pendingOrders.buy().origin));
    if (BUSINESS_BUY != record.business) {
        m_pendingOrders.buy().clear();
        return result;
    }

    m_buyDays = 0;
    m_lastTakeProfit = record.realPrice;
    m_trades.push_back(record);
    buyNotifyAll(record);
    m_pendingOrders.buy().clear();
    return record;
}

void StrategyRuntime::submitBuyRequest(const KRecord& today, const KRecord& src_today, OrderOrigin from) {
    if (m_pendingOrders.buy().valid) {
        if (m_pendingOrders.buy().count > getParam<int>("max_delay_count")) {
            // The maximum number of the delays has been exceeded, clear the buy request
            m_pendingOrders.buy().clear();
            return;
        }
        m_pendingOrders.buy().count++;

    } else {
        m_pendingOrders.buy().valid = true;
        m_pendingOrders.buy().business = BUSINESS_BUY;
        m_pendingOrders.buy().origin = from;
        m_pendingOrders.buy().count = 1;
    }

    m_pendingOrders.buy().datetime = today.datetime;
    m_pendingOrders.buy().stoploss = getStoplossPrice(today, src_today, today.closePrice);
    m_pendingOrders.buy().goal = getGoalPrice(today.datetime, src_today.closePrice);
    m_pendingOrders.buy().number = getBuyNumber(
      today.datetime, src_today.closePrice, src_today.closePrice - m_pendingOrders.buy().stoploss,
      m_pendingOrders.buy().origin);
}

TradeRecord StrategyRuntime::sellForce(const Datetime& date, double num, OrderOrigin from, bool on_open) {
    bool trace = getParam<bool>("trace");
    HKU_INFO_IF(trace, "[{}] {} {} by {}", m_name, htr("force sell"), num, getOrderOriginName(from));

    TradeRecord record;
    size_t pos = m_kdata.getPos(date);
    HKU_TRACE_IF_RETURN(pos == Null<size_t>(), record,
                        "Failed to sellForce {}, the day {} could'nt sell!", m_stock.market_code(),
                        date);

    PositionRecord position = m_account->getPosition(date, m_stock);
    HKU_IF_RETURN(position.number <= 0.0, record);

    const auto& krecord = m_kdata.getKRecord(pos);
    const auto& src_krecord =
      m_stock.getKRecord(m_kdata.startPos() + pos, m_kdata.getQuery().kType());

    price_t realPrice =
      getRealSellPrice(krecord.datetime, on_open ? src_krecord.openPrice : src_krecord.closePrice);

    double min_num = m_stock.minTradeNumber();
    // Round the quantity to be sold to an integer multiple of the minimum trade unit; when the
    // remainder is less than the minimum trade unit, sell everything at once
    double realsell_num = static_cast<int64_t>(num / min_num) * min_num;
    if (position.number - realsell_num < min_num) {
        realsell_num = position.number;
    }

    record = submitStrategyOrder(
      m_execution, OrderRequest(OrderSide::SELL, date, m_stock, realPrice, realsell_num,
                         position.stoploss, position.goalPrice,
                         on_open ? src_krecord.openPrice : src_krecord.closePrice, from));
    HKU_WARN_IF_RETURN(record == Null<TradeRecord>(), record, "[{}] {}: {} by {}", m_name,
                       htr("Failed force sell"), num, getOrderOriginName(from));

    // The last take-profit price is initialized to 0 when there is no position
    if (!m_account->have(m_stock)) {
        m_lastTakeProfit = 0.0;
    }

    m_trades.push_back(record);
    sellNotifyAll(record);
    return record;
}

TradeRecord StrategyRuntime::sell(const KRecord& today, const KRecord& src_today, OrderOrigin from) {
    bool trace = getParam<bool>("trace");
    TradeRecord result;
    if (getParam<bool>("sell_delay")) {
        submitSellRequest(today, src_today, from);
        HKU_INFO_IF(trace, htr("[{}] will be delay to sell", m_name));
        return result;
    }

    // Check whether it may be a one-line limit down
    if (today.highPrice == today.lowPrice) {
        if (getParam<bool>("can_trade_when_high_eq_low")) {
            HKU_WARN_IF(trace, htr("[{}] sell one-price board", m_name));
            return sellNow(today, src_today, from);
        }

        // Get yesterday's data and check whether it is a one-line limit down; on a one-line limit
        // down the sell is delayed
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to sell, one-price board", m_name));
            submitSellRequest(today, src_today, from);
            return result;
        }

        const auto& preday = m_kdata.getKRecord(pos - 1);
        if (today.closePrice < preday.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] sell delayed: limit-down lock", m_name));
            submitSellRequest(today, src_today, from);
            return result;
        }
    }

    if (iszero(today.transAmount) || iszero(today.transCount)) {
        HKU_INFO_IF(trace, htr("[{}] delay to sell, current amount == 0 or count == 0", m_name));
        submitSellRequest(today, src_today, from);
        return result;
    }

    result = sellNow(today, src_today, from);
    HKU_INFO_IF(trace, htr("[{}] sell now: {}", m_name, result));
    return result;
}

TradeRecord StrategyRuntime::sellNow(const KRecord& today, const KRecord& src_today, OrderOrigin from) {
    TradeRecord result;
    price_t planPrice = src_today.closePrice;
    double number = 0;

    // Calculate the new stop-loss price
    price_t stoploss = getStoplossPrice(today, src_today, today.closePrice);

    // When the new planned price is not higher than the new stop-loss price, the whole position is
    // to be sold
    number = getSellNumber(today.datetime, planPrice, planPrice - stoploss, from);
    if (number <= 0) {
        return result;
    }

    price_t goalPrice = getGoalPrice(today.datetime, planPrice);
    price_t realPrice = getRealSellPrice(today.datetime, planPrice);
    TradeRecord record =
      submitStrategyOrder(m_execution, OrderRequest(OrderSide::SELL, today.datetime, m_stock, realPrice,
                                             number, stoploss, goalPrice, planPrice, from));
    if (BUSINESS_SELL != record.business) {
        return result;  // The sell operation failed
    }

    // The last take-profit price is initialized to 0 when there is no position
    if (!m_account->have(m_stock)) {
        m_lastTakeProfit = 0.0;
    } else {
        m_lastTakeProfit = src_today.closePrice;
    }

    m_trades.push_back(record);
    sellNotifyAll(record);
    return record;
}

TradeRecord StrategyRuntime::sellDelay(const KRecord& today, const KRecord& src_today) {
    bool trace = getParam<bool>("trace");
    TradeRecord result;
    if (iszero(today.transAmount) || iszero(today.transCount)) {
        HKU_INFO_IF(trace, htr("[{}] delay to sell, current amount == 0 or count == 0", m_name));
        submitSellRequest(today, src_today, m_pendingOrders.sell().origin);
        return result;
    }

    if (today.highPrice == today.lowPrice && !getParam<bool>("can_trade_when_high_eq_low")) {
        // Get yesterday's data and check whether it is a one-line limit down; on a one-line limit
        // down the sell is delayed
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to sell, one-price board", m_name));
            submitSellRequest(today, src_today, m_pendingOrders.sell().origin);
            return result;
        }

        const auto& preday = m_kdata.getKRecord(pos - 1);
        if (today.closePrice < preday.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] sell delayed: limit-down lock", m_name));
            submitSellRequest(today, src_today, m_pendingOrders.sell().origin);
            return result;
        }
    }

    price_t planPrice = src_today.openPrice;  // Take the open price of the current moment

    // The stop-loss price at the moment the sell request is issued
    price_t stoploss = 0.0;
    double number = 0.0;
    price_t goalPrice = 0.0;

    OrderOrigin from = m_pendingOrders.sell().origin;

    if (getParam<bool>("delay_use_current_price")) {
        stoploss = getStoplossPrice(today, src_today, today.openPrice);
        number = getSellNumber(today.datetime, planPrice, planPrice - stoploss, from);
        goalPrice = getGoalPrice(today.datetime, planPrice);
    } else {
        stoploss = m_pendingOrders.sell().stoploss;
        number = m_pendingOrders.sell().number;
        goalPrice = m_pendingOrders.sell().goal;
    }

    if (number <= 0) {
        m_pendingOrders.sell().clear();
        return result;
    }

    price_t realPrice = getRealSellPrice(today.datetime, planPrice);
    TradeRecord record = submitStrategyOrder(
      m_execution, OrderRequest(OrderSide::SELL, today.datetime, m_stock, realPrice, number, stoploss,
                         goalPrice, planPrice, m_pendingOrders.sell().origin));
    if (BUSINESS_SELL != record.business) {
        m_pendingOrders.sell().clear();
        return result;  // The sell operation failed
    }

    // The last take-profit price is initialized to 0 when there is no position
    if (!m_account->have(m_stock)) {
        m_lastTakeProfit = 0.0;
    } else {
        m_lastTakeProfit = src_today.openPrice;
    }

    m_trades.push_back(record);
    sellNotifyAll(record);
    m_pendingOrders.sell().clear();
    return record;
}

void StrategyRuntime::submitSellRequest(const KRecord& today, const KRecord& src_today, OrderOrigin from) {
    if (m_pendingOrders.sell().valid) {
        if (m_pendingOrders.sell().count > getParam<int>("max_delay_count")) {
            // The maximum number of the delays has been exceeded, clear the buy request
            m_pendingOrders.sell().clear();
            return;
        }
        m_pendingOrders.sell().count++;

    } else {
        m_pendingOrders.sell().valid = true;
        m_pendingOrders.sell().business = BUSINESS_SELL;
        m_pendingOrders.sell().count = 1;
    }

    m_pendingOrders.sell().origin = from;
    m_pendingOrders.sell().datetime = today.datetime;
    m_pendingOrders.sell().stoploss = getStoplossPrice(today, src_today, today.closePrice);
    if (src_today.closePrice <= m_pendingOrders.sell().stoploss) {
        m_pendingOrders.sell().number = m_account->getHoldNumber(today.datetime, m_stock);
    } else {
        m_pendingOrders.sell().number =
          getSellNumber(today.datetime, src_today.closePrice,
                         src_today.closePrice - m_pendingOrders.sell().stoploss, from);
    }

    m_pendingOrders.sell().goal = getGoalPrice(today.datetime, src_today.closePrice);
}

TradeRecord StrategyRuntime::buyShort(const KRecord& today, const KRecord& src_today, OrderOrigin from) {
    TradeRecord result;
    if (getParam<bool>("support_borrow_stock") == false)
        return result;

    bool trace = getParam<bool>("trace");
    if (getParam<bool>("buy_delay")) {
        submitBuyShortRequest(today, src_today, from);
        HKU_INFO_IF(trace, htr("[{}] will buy short next bar open", m_name));
        return result;
    }

    // The one-line board case
    if (today.highPrice == today.lowPrice) {
        if (getParam<bool>("can_trade_when_high_eq_low")) {
            HKU_WARN_IF(trace, htr("[{}] buy short one-price board", m_name));
            return buyShortNow(today, src_today, from);
        }

        // Get yesterday's data and check whether it is a one-line limit up,
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to buy short, one-price board", m_name));
            submitBuyShortRequest(today, src_today, from);
            return result;
        }

        const auto& preday = m_kdata.getKRecord(pos - 1);
        if (today.closePrice > preday.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] short covering delayed: limit-up lock", m_name));
            submitBuyShortRequest(today, src_today, from);
            return result;
        }
    }

    if (iszero(today.transAmount) || iszero(today.transCount)) {
        HKU_INFO_IF(trace,
                    htr("[{}] delay to buy short, current amount == 0 or count == 0", m_name));
        submitBuyShortRequest(today, src_today, from);
        return result;
    }

    return buyShortNow(today, src_today, from);
}

TradeRecord StrategyRuntime::buyShortNow(const KRecord& today, const KRecord& src_today, OrderOrigin from) {
    TradeRecord result;
    price_t planPrice = src_today.closePrice;  // Take the close price of the current moment

    // Take the stop-loss price corresponding to the close price of the current moment
    price_t stoploss = getShortStoplossPrice(today, src_today, today.closePrice);

    // Determine the quantity
    double number = getBuyShortNumber(today.datetime, planPrice, stoploss - planPrice, from);
    if (number <= 0) {
        m_pendingOrders.buyShort().clear();
        return result;
    }

    // Get the holding status of the current short position
    PositionRecord pos = m_account->getShortPosition(m_stock);
    if (pos.number <= 0) {
        m_pendingOrders.buyShort().clear();
        return result;
    }

    if (number > pos.number) {
        number = pos.number;
    }

    price_t goalPrice = getShortGoalPrice(today.datetime, planPrice);
    price_t realPrice = getRealBuyPrice(today.datetime, planPrice);

    TradeRecord record = submitStrategyOrder(
      m_execution,
      OrderRequest(OrderSide::BUY_SHORT, today.datetime, m_stock, realPrice, number, stoploss,
                         goalPrice, planPrice, OrderOrigin::SIGNAL));
    if (BUSINESS_BUY_SHORT != record.business) {
        m_pendingOrders.buyShort().clear();
        return result;
    }

    m_sellShortDays = 0;
    m_lastTakeProfit = realPrice;  // The take-profit is assigned the buy price
    m_trades.push_back(record);
    buyNotifyAll(record);
    m_pendingOrders.buyShort().clear();
    return record;
}

TradeRecord StrategyRuntime::buyShortDelay(const KRecord& today, const KRecord& src_today) {
    TradeRecord result;
    bool trace = getParam<bool>("trace");
    if (iszero(today.transAmount) || iszero(today.transCount)) {
        HKU_INFO_IF(trace,
                    htr("[{}] delay to buy short, current amount == 0 or count == 0", m_name));
        submitBuyShortRequest(today, src_today, m_pendingOrders.buyShort().origin);
        return result;
    }

    if (today.highPrice == today.lowPrice && !getParam<bool>("can_trade_when_high_eq_low")) {
        // Get yesterday's data and check whether it is a one-line limit up,
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to buy short, one-price board", m_name));
            submitBuyShortRequest(today, src_today, m_pendingOrders.buyShort().origin);
            return result;
        }

        const auto& preday = m_kdata.getKRecord(pos - 1);
        if (today.closePrice > preday.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] short covering delayed: limit-up lock", m_name));
            submitBuyShortRequest(today, src_today, m_pendingOrders.buyShort().origin);
            return result;
        }
    }

    price_t planPrice = src_today.openPrice;  // Take the close price of the current moment

    price_t stoploss = 0.0;
    double number = 0.0;
    price_t goalPrice = 0.0;
    if (getParam<bool>("delay_use_current_price")) {
        // Take the stop-loss price corresponding to the close price of the current moment
        stoploss = getShortStoplossPrice(today, src_today, today.openPrice);
        number = getBuyShortNumber(today.datetime, planPrice, stoploss - planPrice,
                                    m_pendingOrders.buy().origin);
        goalPrice = getShortGoalPrice(today.datetime, planPrice);

    } else {
        stoploss = m_pendingOrders.buyShort().stoploss;
        number = m_pendingOrders.buyShort().number;
        goalPrice = m_pendingOrders.buyShort().goal;
    }

    if (number <= 0) {
        m_pendingOrders.buyShort().clear();
        return result;
    }

    // Get the holding status of the current short position
    PositionRecord pos = m_account->getShortPosition(m_stock);
    if (pos.number <= 0) {
        m_pendingOrders.buyShort().clear();
        return result;
    }

    if (number > pos.number) {
        number = pos.number;
    }

    price_t realPrice = getRealBuyPrice(today.datetime, planPrice);
    TradeRecord record = submitStrategyOrder(
      m_execution,
      OrderRequest(OrderSide::BUY_SHORT, today.datetime, m_stock, realPrice, number, stoploss,
                         goalPrice, planPrice, OrderOrigin::SIGNAL));
    if (BUSINESS_BUY_SHORT != record.business) {
        m_pendingOrders.buyShort().clear();
        return result;
    }

    m_sellShortDays = 0;
    m_lastTakeProfit = realPrice;  // The take-profit is assigned the buy price
    m_trades.push_back(record);
    buyNotifyAll(record);
    m_pendingOrders.buyShort().clear();
    return result;
}

void StrategyRuntime::submitBuyShortRequest(const KRecord& today, const KRecord& src_today, OrderOrigin from) {
    if (m_pendingOrders.buyShort().valid) {
        if (m_pendingOrders.buyShort().count > getParam<int>("max_delay_count")) {
            // The maximum number of the delays has been exceeded, clear the buy request
            m_pendingOrders.buy().clear();
            return;
        }
        m_pendingOrders.buyShort().count++;

    } else {
        m_pendingOrders.buyShort().valid = true;
        m_pendingOrders.buyShort().business = BUSINESS_BUY;
        m_pendingOrders.buyShort().origin = from;
        m_pendingOrders.buyShort().count = 1;
    }

    m_pendingOrders.buyShort().datetime = today.datetime;
    m_pendingOrders.buyShort().stoploss =
      getShortStoplossPrice(today, src_today, today.closePrice);
    m_pendingOrders.buyShort().goal = getShortGoalPrice(today.datetime, src_today.closePrice);
    m_pendingOrders.buyShort().number = getBuyShortNumber(
      today.datetime, src_today.closePrice,
      m_pendingOrders.buyShort().stoploss - src_today.closePrice, m_pendingOrders.buyShort().origin);
}

TradeRecord StrategyRuntime::sellShort(const KRecord& today, const KRecord& src_today, OrderOrigin from) {
    TradeRecord result;
    bool trace = getParam<bool>("trace");
    if (getParam<bool>("support_borrow_stock") == false) {
        // HKU_WARN("set system param support_borrow_stock to true to short sell");
        return result;
    }

    if (getParam<bool>("sell_delay")) {
        submitSellShortRequest(today, src_today, from);
        return result;
    }

    if (today.highPrice == today.lowPrice) {
        if (getParam<bool>("can_trade_when_high_eq_low")) {
            HKU_WARN_IF(trace, htr("[{}] buy short one-price board", m_name));
            return sellShortNow(today, src_today, from);
        }

        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to sell short, one-price board", m_name));
            submitSellShortRequest(today, src_today, from);
            return result;
        }

        const auto& preday = m_kdata.getKRecord(pos - 1);
        if (today.closePrice < preday.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] short selling delayed: limit-down lock", m_name));
            submitSellShortRequest(today, src_today, from);
            return result;
        }
    }

    if (iszero(today.transAmount) || iszero(today.transCount)) {
        HKU_INFO_IF(trace,
                    htr("[{}] delay to sell short, current amount == 0 or count == 0", m_name));
        submitSellShortRequest(today, src_today, from);
        return result;
    }

    return sellShortNow(today, src_today, from);
}

TradeRecord StrategyRuntime::sellShortNow(const KRecord& today, const KRecord& src_today, OrderOrigin from) {
    TradeRecord result;
    if (today.highPrice == today.lowPrice && !getParam<bool>("can_trade_when_high_eq_low")) {
        // It cannot be sold at the moment, delay the sell to the next moment
        submitSellShortRequest(today, src_today, from);
        return result;
    }

    price_t planPrice = src_today.closePrice;

    // Calculate the stop-loss price
    price_t stoploss = getShortStoplossPrice(today, src_today, today.closePrice);

    double number = getSellShortNumber(today.datetime, planPrice, stoploss - planPrice, from);
    if (number <= 0) {
        m_pendingOrders.sellShort().clear();
        return result;
    }

    price_t goalPrice = getShortGoalPrice(today.datetime, planPrice);
    price_t realPrice = getRealSellPrice(today.datetime, planPrice);
    TradeRecord record = submitStrategyOrder(
      m_execution, OrderRequest(OrderSide::SELL_SHORT, today.datetime, m_stock, realPrice, number,
                         stoploss, goalPrice, planPrice, OrderOrigin::SIGNAL));
    if (BUSINESS_SELL_SHORT != record.business) {
        m_pendingOrders.sellShort().clear();
        return result;  // The sell operation failed
    }

    m_sellShortDays = 0;
    m_lastShortTakeProfit = realPrice;
    m_trades.push_back(record);
    sellNotifyAll(record);
    m_pendingOrders.sellShort().clear();
    return record;
}

TradeRecord StrategyRuntime::sellShortDelay(const KRecord& today, const KRecord& src_today) {
    TradeRecord result;
    bool trace = getParam<bool>("trace");
    if (iszero(today.transAmount) || iszero(today.transCount)) {
        HKU_INFO_IF(trace,
                    htr("[{}] delay to sell short, current amount == 0 or count == 0", m_name));
        submitSellShortRequest(today, src_today, m_pendingOrders.sellShort().origin);
        return result;
    }

    if (today.highPrice == today.lowPrice && !getParam<bool>("can_trade_when_high_eq_low")) {
        size_t pos = m_kdata.getPos(today.datetime);
        if (pos == 0 || pos == Null<size_t>()) {
            HKU_INFO_IF(trace, htr("[{}] delay to sell short, one-price board", m_name));
            submitSellShortRequest(today, src_today, m_pendingOrders.sellShort().origin);
            return result;
        }

        const auto& preday = m_kdata.getKRecord(pos - 1);
        if (today.closePrice < preday.closePrice) {
            HKU_INFO_IF(trace, htr("[{}] short selling delayed: limit-down lock", m_name));
            submitSellShortRequest(today, src_today, m_pendingOrders.sellShort().origin);
            return result;
        }
    }

    price_t planPrice = src_today.openPrice;  // Take the open price of the current moment

    // The stop-loss price at the moment the sell request is issued
    price_t stoploss = 0.0;
    double number = 0;
    price_t goalPrice = 0.0;
    if (getParam<bool>("delay_use_current_price")) {
        stoploss = getShortStoplossPrice(today, src_today, today.openPrice);
        number = getSellShortNumber(today.datetime, planPrice, stoploss - planPrice,
                                     m_pendingOrders.sellShort().origin);
        goalPrice = getShortGoalPrice(today.datetime, planPrice);
    } else {
        stoploss = m_pendingOrders.sellShort().stoploss;
        number = m_pendingOrders.sellShort().number;
        goalPrice = m_pendingOrders.sellShort().goal;
    }

    if (number <= 0) {
        m_pendingOrders.sellShort().clear();
        return result;
    }

    price_t realPrice = getRealSellPrice(today.datetime, planPrice);

    TradeRecord record = submitStrategyOrder(
      m_execution, OrderRequest(OrderSide::SELL_SHORT, today.datetime, m_stock, realPrice, number,
                         stoploss, goalPrice, planPrice, m_pendingOrders.sellShort().origin));
    if (BUSINESS_SELL_SHORT != record.business) {
        m_pendingOrders.sellShort().clear();
        return result;  // The sell operation failed
    }

    m_sellShortDays = 0;
    m_lastShortTakeProfit = realPrice;
    m_trades.push_back(record);
    sellNotifyAll(record);
    m_pendingOrders.sellShort().clear();
    return record;
}

void StrategyRuntime::submitSellShortRequest(const KRecord& today, const KRecord& src_today, OrderOrigin from) {
    if (m_pendingOrders.sellShort().valid) {
        if (m_pendingOrders.sellShort().count > getParam<int>("max_delay_count")) {
            // The maximum number of the delays has been exceeded, clear the buy request
            m_pendingOrders.sellShort().clear();
            return;
        }
        m_pendingOrders.sellShort().count++;

    } else {
        m_pendingOrders.sellShort().valid = true;
        m_pendingOrders.sellShort().business = BUSINESS_SELL_SHORT;
        m_pendingOrders.sellShort().origin = from;
        m_pendingOrders.sellShort().count = 1;
    }

    m_pendingOrders.sellShort().datetime = today.datetime;
    m_pendingOrders.sellShort().stoploss = getStoplossPrice(today, src_today, today.closePrice);
    m_pendingOrders.sellShort().goal = getGoalPrice(today.datetime, src_today.closePrice);
    m_pendingOrders.sellShort().number =
      getSellNumber(today.datetime, src_today.closePrice,
                     src_today.closePrice - m_pendingOrders.sellShort().stoploss,
                     m_pendingOrders.sellShort().origin);
}

TradeRecord StrategyRuntime::processRequest(const KRecord& today, const KRecord& src_today) {
    HKU_IF_RETURN(m_pendingOrders.buy().valid, buyDelay(today, src_today));
    HKU_IF_RETURN(m_pendingOrders.sell().valid, sellDelay(today, src_today));
    HKU_IF_RETURN(m_pendingOrders.sellShort().valid, sellShortDelay(today, src_today));
    HKU_IF_RETURN(m_pendingOrders.buyShort().valid, buyShortDelay(today, src_today));
    return TradeRecord();
}

price_t StrategyRuntime::getStoplossPrice(const KRecord& today, const KRecord& src_today, price_t price) {
    HKU_IF_RETURN(!m_st, 0.0);
    HKU_IF_RETURN(today.highPrice == today.lowPrice, src_today.lowPrice);
    price_t stoploss = m_st->getPrice(today.datetime, price);
    price_t adjust = (stoploss - today.lowPrice) / (today.highPrice - today.lowPrice) *
                       (src_today.highPrice - src_today.lowPrice) +
                     src_today.lowPrice;
    return adjust >= 0.0 ? adjust : 0.0;
}

price_t StrategyRuntime::getShortStoplossPrice(const KRecord& today, const KRecord& src_today,
                                        price_t price) {
    HKU_IF_RETURN(!m_st, 0.0);
    HKU_IF_RETURN(today.highPrice == today.lowPrice, src_today.lowPrice);
    price_t stoploss = m_st->getShortPrice(today.datetime, price);
    price_t adjust = (stoploss - today.lowPrice) / (today.highPrice - today.lowPrice) *
                       (src_today.highPrice - src_today.lowPrice) +
                     src_today.lowPrice;
    return adjust >= 0.0 ? adjust : 0.0;
}

}  // namespace hku::internal
