/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-16
 *     Author: fasiondog
 */

#include "Strategy.h"

#include <csignal>
#include <unordered_set>

#include "application/HayakuSession.h"
#include "application/IniParser.h"
#include "application/SessionOptions.h"
#include "application/SystemInfo.h"
#include "common/Os.h"
#include "data/DataRuntime.h"
#include "execution/AccountConfig.h"
#include "execution/ExecutionAccountFactory.h"
#include "execution/ExecutionBrokerPort.h"
#include "execution/PortfolioAccountPort.h"
#include "extensions/realtime/RealtimePort.h"
#include "extensions/realtime/Scheduler.h"
#include "hayaku.h"
#include "strategy/BacktestRequest.h"
#include "strategy/StrategyRuntime.h"

namespace hayaku {

namespace {

internal::ExecutionAccountPortPtr makeBrokerAccount(
    const OrderBrokerPtr& broker, const TradeCostPtr& costfunc,
    const string& name, const std::vector<OrderBrokerPtr>& other_brokers) {
  std::vector<OrderBrokerPtr> brokers;
  brokers.reserve(other_brokers.size() + 1);
  brokers.emplace_back(broker);
  for (const auto& item : other_brokers) {
    if (item) {
      brokers.emplace_back(item);
    }
  }

  auto account = internal::makeExecutionAccount(
      AccountConfig(Datetime::now(), 0.0, costfunc, name, 2, false, false, {},
                    std::move(brokers)));
  auto broker_port =
      std::dynamic_pointer_cast<internal::ExecutionBrokerPort>(account);
  HAYAKU_CHECK(
      broker_port,
      "Execution account does not support live broker synchronization");
  broker_port->fetchAssetInfoFromBroker(broker);
  return account;
}

}  // namespace

std::atomic_bool Strategy::ms_keep_running = true;
std::atomic<bool> Strategy::ms_sig_registered = false;

void Strategy::sig_handler(int sig) {
  if (sig == SIGINT || sig == SIGTERM) {
    try {
      ms_keep_running = false;
      auto* scheduler = getScheduler();
      scheduler->stop();
    } catch (...) {
      // Ignore the exception
    }
    std::exit(EXIT_SUCCESS);
  }
}

void Strategy::register_signal() {
  // Make sure it is registered only once
  bool expected = false;
  if (ms_sig_registered.compare_exchange_strong(expected, true)) {
    if (std::signal(SIGINT, sig_handler) == SIG_ERR) {
      ms_sig_registered.store(false);
    }
  }
}

Strategy::Strategy() : Strategy("Strategy", "") {}

Strategy::Strategy(const string& name, const string& config_file)
    : m_name(name), m_config_file(config_file) {
  _initParam();
  if (m_config_file.empty()) {
    string home = getUserDir();
    HAYAKU_ERROR_IF(home == "", "Failed get user home path!");
#if HAYAKU_OS_WINOWS
    m_config_file = format("{}\\{}", home, ".hayaku\\hayaku.ini");
#else
    m_config_file = format("{}/{}", home, ".hayaku/hayaku.ini");
#endif
  }
}

Strategy::Strategy(const vector<string>& codeList,
                   const vector<KQuery::KType>& ktypeList,
                   const unordered_map<string, int64_t>& preloadNum,
                   const string& name, const string& config_file)
    : Strategy(name, config_file) {
  _initParam();
  m_context.setStockCodeList(codeList);
  m_context.setKTypeList(ktypeList);
  m_context.setPreloadNum(preloadNum);
}

Strategy::Strategy(const StrategyContext& context, const string& name,
                   const string& config_file)
    : Strategy(name, config_file) {
  _initParam();
  m_context = context;
}

Strategy::~Strategy() {
  // ms_keep_running is used for the global ctrl-c termination; it must not be
  // released on the release, otherwise a newly created strategy object would
  // run ms_keep_running = false;
  event([]() {});
}

void Strategy::_initParam() {
  setParam<int>("spot_worker_num", 1);
  setParam<string>("quotation_server", string());
}

void Strategy::baseCheckParam(const string& name) const {
  if (name == "spot_worker_num") {
    HAYAKU_ASSERT(getParam<int>(name) > 0);
  }
}

void Strategy::paramChanged() {}

bool Strategy::running() const { return ms_keep_running; }

void Strategy::_init() {
  auto& sm = getDataRuntime();

  // Initialize sm when it has not been initialized yet
  if (sm.thread_id() == std::thread::id()) {
    // Register the ctrl-c termination signal
    if (!runningInPython()) {
      // std::signal(SIGINT, sig_handler);
      register_signal();
    }

    CLS_INFO("{} is running! You can press Ctrl-C to terminte ...", m_name);

    // Initialization
    m_session = std::make_unique<HayakuSession>(HayakuSession::open(
        SessionOptions::fromIni(m_config_file, false, m_context)));

  } else {
    m_context = sm.getStrategyContext();
  }

  if (!runningInPython()) {
    register_signal();
  }

  CLS_CHECK(!m_context.getStockCodeList().empty(),
            "The context does not contain any stocks!");

  // Stop the market data receiving agent first, so that the handlers can be
  // added later
  stopRealtimeForStrategy();
}

void Strategy::start(bool autoRecieveSpot) {
  HAYAKU_WARN_IF_RETURN(pythonInInteractive(), void(),
                        "Can not start strategy in python interactive mode!");
  HAYAKU_WARN_IF(!m_on_recieved_spot && !m_on_change &&
                     m_run_daily_at_list.empty() &&
                     m_run_daily_at_funcs.empty(),
                 "No any process function is set!");

  _init();

  _runDailyAt();

  if (autoRecieveSpot) {
    RealtimeSpotProcess process = [this](const SpotRecord& spot) {
      _receivedSpot(spot);
    };
    RealtimePostProcess postProcess = [this](Datetime revTime) {
      if (m_on_recieved_spot) {
        event([this, revTime]() { m_on_recieved_spot(this, revTime); });
      }
    };
    startRealtimeForStrategy(process, postProcess,
                             getParam<int>("spot_worker_num"),
                             getParam<string>("quotation_server"));
  }

  _runDaily();

  CLS_INFO("{} start even loop ...", name());
  _startEventLoop();
}

void Strategy::onChange(
    const std::function<void(Strategy*, const Stock&, const SpotRecord& spot)>&
        changeFunc) {
  HAYAKU_CHECK(changeFunc, "Invalid changeFunc!");
  m_on_change = std::move(changeFunc);
}

void Strategy::onReceivedSpot(
    const std::function<void(Strategy*, const Datetime&)>& recievedFucn) {
  HAYAKU_CHECK(recievedFucn, "Invalid recievedFucn!");
  m_on_recieved_spot = std::move(recievedFucn);
}

void Strategy::_receivedSpot(const SpotRecord& spot) {
  Stock stk = getStock(format("{}{}", spot.market, spot.code));
  if (!stk.isNull()) {
    if (m_on_change) {
      event([this, stk, spot]() { m_on_change(this, stk, spot); });
    }
  }
}

void Strategy::runDaily(const std::function<void(Strategy*)>& func,
                        const TimeDelta& delta, const std::string& market,
                        bool ignoreMarket) {
  HAYAKU_CHECK(func, "Invalid func!");
  HAYAKU_CHECK(!market.empty(), "The market can not be empty!");
  HAYAKU_WARN_IF(delta > Hours(1), "The delta may be large! {}", delta);

  RunDailyAt run_at;
  run_at.delta = delta;
  run_at.market = market;
  run_at.ignoreMarket = ignoreMarket;

  if (ignoreMarket) {
    run_at.func = [this, f = std::move(func)]() {
      event([this, f]() { f(this); });
    };

  } else {
    run_at.func = [this, market = run_at.market, f = std::move(func)]() {
      const auto& sm = getDataRuntime();
      auto today = Datetime::today();
      int day = today.dayOfWeek();
      if (day == 0 || day == 6 || sm.isHoliday(today)) {
        return;
      }

      auto market_info = sm.getMarketInfo(market);
      Datetime open1 = today + market_info.openTime1();
      Datetime close1 = today + market_info.closeTime1();
      Datetime open2 = today + market_info.openTime2();
      Datetime close2 = today + market_info.closeTime2();
      Datetime now = Datetime::now();
      if ((now >= open1 && now <= close1) || (now >= open2 && now <= close2)) {
        event([this, f]() { f(this); });
      }
    };
  }

  m_run_daily_at_list.push_front(run_at);
}

void Strategy::_runDaily() {
  HAYAKU_IF_RETURN(m_run_daily_at_list.empty(), void());

  auto* scheduler = getScheduler();

  for (auto& run_at : m_run_daily_at_list) {
    if (run_at.ignoreMarket) {
      scheduler->addDurationFunc(std::numeric_limits<int>::max(), run_at.delta,
                                 run_at.func);

    } else {
      try {
        const auto& sm = getDataRuntime();
        auto market_info = sm.getMarketInfo(run_at.market);
        HAYAKU_ERROR_IF_RETURN(
            market_info == Null<MarketInfo>(), void(),
            "market {} not found! The run daily func is discard!",
            run_at.market);

        auto today = Datetime::today();
        auto now = Datetime::now();
        TimeDelta now_time = now - today;
        if (now_time >= market_info.closeTime2()) {
          scheduler->addFuncAtTime(
              today.nextDay() + market_info.openTime1(), [&run_at]() {
                run_at.func();
                auto* sched = getScheduler();
                sched->addDurationFunc(std::numeric_limits<int>::max(),
                                       run_at.delta, run_at.func);
              });

        } else if (now_time >= market_info.openTime2()) {
          int64_t ticks = now_time.ticks() - market_info.openTime2().ticks();
          int64_t delta_ticks = run_at.delta.ticks();
          if (ticks % delta_ticks == 0) {
            scheduler->addDurationFunc(std::numeric_limits<int>::max(),
                                       run_at.delta, run_at.func);
          } else {
            auto delay = TimeDelta::fromTicks(
                (ticks / delta_ticks + 1) * delta_ticks - ticks);
            scheduler->addFuncAtTime(now + delay, [&run_at]() {
              run_at.func();
              auto* sched = getScheduler();
              sched->addDurationFunc(std::numeric_limits<int>::max(),
                                     run_at.delta, run_at.func);
            });
          }

        } else if (now_time >= market_info.closeTime1()) {
          scheduler->addFuncAtTime(
              today + market_info.openTime2(), [&run_at]() {
                run_at.func();
                auto* sched = getScheduler();
                sched->addDurationFunc(std::numeric_limits<int>::max(),
                                       run_at.delta, run_at.func);
              });

        } else if (now_time < market_info.closeTime1() &&
                   now_time >= market_info.openTime1()) {
          int64_t ticks = now_time.ticks() - market_info.openTime1().ticks();
          int64_t delta_ticks = run_at.delta.ticks();
          if (ticks % delta_ticks == 0) {
            scheduler->addDurationFunc(std::numeric_limits<int>::max(),
                                       run_at.delta, run_at.func);
          } else {
            auto delay = TimeDelta::fromTicks(
                (ticks / delta_ticks + 1) * delta_ticks - ticks);
            scheduler->addFuncAtTime(now + delay, [&run_at]() {
              run_at.func();
              auto* sched = getScheduler();
              sched->addDurationFunc(std::numeric_limits<int>::max(),
                                     run_at.delta, run_at.func);
            });
          }

        } else if (now_time < market_info.openTime1()) {
          scheduler->addFuncAtTime(
              today + market_info.openTime1(), [&run_at]() {
                run_at.func();
                auto* sched = getScheduler();
                sched->addDurationFunc(std::numeric_limits<int>::max(),
                                       run_at.delta, run_at.func);
              });

        } else {
          CLS_ERROR("Unknown process! now_time: {}", now_time);
        }
      } catch (const std::exception& e) {
        CLS_THROW("{}", e.what());
      }
    }
  }
}

void Strategy::runDailyAt(const std::function<void(Strategy*)>& func,
                          const TimeDelta& delta, bool ignoreHoliday) {
  HAYAKU_CHECK(func, "Invalid func!");
  HAYAKU_CHECK(delta < Days(1), "TimeDelta must < Days(1)!");
  HAYAKU_CHECK(m_run_daily_at_funcs.find(delta) == m_run_daily_at_funcs.end(),
               "A task already exists at this point in time!");

  std::function<void()> new_func;
  if (ignoreHoliday) {
    new_func = [this, f = std::move(func)]() {
      const auto& sm = getDataRuntime();
      auto today = Datetime::today();
      int day = today.dayOfWeek();
      if (day != 0 && day != 6 && !sm.isHoliday(today)) {
        event([this, f]() { f(this); });
      }
    };

  } else {
    new_func = [this, f = std::move(func)]() {
      event([this, f]() { f(this); });
    };
  }

  m_run_daily_at_funcs[delta] = new_func;
}

void Strategy::_runDailyAt() {
  auto* scheduler = getScheduler();
  for (const auto& [time, func] : m_run_daily_at_funcs) {
    scheduler->addFuncAtTimeEveryDay(time, func);
  }
  m_run_daily_at_funcs.clear();
}

/*
 * Process the event queue in the main thread, avoiding the python GIL
 */
void Strategy::_startEventLoop() {
  while (ms_keep_running) {
    event_type task;
    m_event_queue.wait_and_pop(task);
    if (task.isNullTask()) {
      ms_keep_running = false;
    } else {
      try {
        task();
      } catch (const std::exception& e) {
        CLS_ERROR("Failed run task! {}", e.what());
      } catch (...) {
        CLS_ERROR("Failed run task! Unknow error!");
      }
    }
  }
}

price_t Strategy::getCurrentPrice(const Stock& stk,
                                  const KQuery::KType& ktype) const {
  KData k = getLastKData(stk, 1, ktype);
  HAYAKU_IF_RETURN(k.empty(), Null<price_t>());
  const auto& kr = k.front();
  return kr.datetime.startOfDay() != today() ? Null<price_t>() : kr.closePrice;
}

KData Strategy::getKData(const Stock& stk, const Datetime& start_date,
                         const Datetime& end_date, const KQuery::KType& ktype,
                         KQuery::RecoverType recover_type) const {
  Datetime new_end_date = end_date;
  if (end_date.isNull() || end_date > now()) {
    new_end_date = nextDatetime();
  }
  return stk.getKData(
      KQueryByDate(start_date, new_end_date, ktype, recover_type));
}

price_t Strategy::getPriceByTime(const Stock& stk, const TimeDelta& time,
                                 const KQuery::KType& ktype) const {
  Datetime start = today() + time;
  Datetime end = start + time;
  if ((now() - today()) != TimeDelta()) {
    // For a non-daily level such as the minute line, the price after the
    // current time is clamped to the current time
    if (end > now()) {
      end = now();
    }
  }
  end = end + Seconds(KQuery::getKTypeInSeconds(ktype));
  KData k = stk.getKData(KQueryByDate(start, end, ktype));
  return k.empty() ? Null<price_t>() : k.back().closePrice;
}

KData Strategy::getLastKData(const Stock& stk, size_t lastnum,
                             const KQuery::KType& ktype,
                             KQuery::RecoverType recover_type) const {
  KData ret;
  KQuery query = KQueryByDate(Datetime::min(), nextDatetime(), ktype);
  size_t out_start = 0, out_end = 0;
  HAYAKU_IF_RETURN(!stk.getIndexRange(query, out_start, out_end), ret);

  int64_t startidx = 0, endidx = 0;
  endidx = out_end;
  int64_t num = static_cast<int64_t>(lastnum);
  startidx = (endidx > num) ? endidx - num : out_start;

  query = KQueryByIndex(startidx, endidx, ktype, recover_type);
  ret = stk.getKData(query);
  return ret;
}

TradeRecord Strategy::order(const Stock& stk, double num,
                            const string& remark) {
  TradeRecord ret;
  HAYAKU_WARN_IF_RETURN(num == 0.0, ret, "{} {} order num is zero!",
                        stk.market_code(), stk.name());

  double min_trade_num = stk.minTradeNumber();
  double max_trade_num = stk.maxTradeNumber();
  if (num > 0.0) {
    // HAYAKU_WARN_IF_RETURN(num < min_trade_num, ret,
    //                    "Ignore! {} {} order num({}) is less than min trade
    //                    number({})!", stk.market_code(), stk.name(), num,
    //                    min_trade_num);
    HAYAKU_IF_RETURN(num < min_trade_num, ret);
    double buy_num = int64_t(num / min_trade_num) * min_trade_num;
    if (buy_num > max_trade_num) {
      buy_num = max_trade_num;
    }
    ret = buy(stk, 0.0, num, 0.0, 0.0, OrderOrigin::SIGNAL, remark);

  } else {
    if (num == -MAX_DOUBLE) {
      ret = sell(stk, 0.0, MAX_DOUBLE, 0.0, 0.0, OrderOrigin::SIGNAL, remark);
      return ret;
    }
    double sell_num = int64_t(std::abs(num) / min_trade_num) * min_trade_num;
    if (sell_num > max_trade_num && sell_num != MAX_DOUBLE) {
      sell_num = max_trade_num;
    } else if ((sell_num + num) < min_trade_num) {
      sell_num = MAX_DOUBLE;  // Indicate selling all the remaining
    }
    ret = sell(stk, 0.0, sell_num, 0.0, 0.0, OrderOrigin::SIGNAL, remark);
  }

  return ret;
}

TradeRecord Strategy::orderValue(const Stock& stk, price_t value,
                                 const string& remark) {
  TradeRecord ret;
  HAYAKU_WARN_IF_RETURN(value == 0.0, ret, "{} {} order value is zero!",
                        stk.market_code(), stk.name());

  auto k = getLastKData(stk, 1, KQuery::DAY,
                        KQuery::NO_RECOVER);  // The current daily-line price
  HAYAKU_IF_RETURN(k.empty() || k[0].datetime.startOfDay() != today(), ret);

  price_t price = k[0].closePrice;
  if (value > 0.0) {
    double n = value / price;
    HAYAKU_CHECK(m_account, "Strategy execution account is not configured");
    CostRecord cost = m_account->getBuyCost(now(), stk, price, n);
    price_t need_cash = n * price + cost.total;
    price_t current_cash = m_account->currentCash();
    double min_trade = stk.minTradeNumber();
    while (n > min_trade && need_cash > current_cash) {
      n = n - min_trade;
      cost = m_account->getBuyCost(now(), stk, price, n);
      need_cash = n * price + cost.total;
    }
    if (need_cash > current_cash) {
      n = 0.0;
    }
    if (n == 0.0) {
      HAYAKU_WARN("{} {} can buy number is zero!", stk.market_code(),
                  stk.name());
    } else {
      ret = order(stk, n, remark);
    }
  } else {
    ret = order(stk, value / price, remark);
  }
  return ret;
}

TradeRecord Strategy::buy(const Stock& stk, price_t price, double num,
                          double stoploss, double goal_price,
                          OrderOrigin origin, const string& remark) {
  HAYAKU_ASSERT(m_account);
  return m_account
      ->submit(OrderRequest(OrderSide::BUY, Datetime::now(), stk, price, num,
                            stoploss, goal_price, price, origin, remark))
      .trade();
}

TradeRecord Strategy::sell(const Stock& stk, price_t price, double num,
                           price_t stoploss, price_t goal_price,
                           OrderOrigin origin, const string& remark) {
  HAYAKU_ASSERT(m_account);
  return m_account
      ->submit(OrderRequest(OrderSide::SELL, Datetime::now(), stk, price, num,
                            stoploss, goal_price, price, origin, remark))
      .trade();
}

void HAYAKU_API
runInStrategy(const internal::StrategyRuntimePtr& strategy, const Stock& stk,
              const KQuery& query, const OrderBrokerPtr& broker,
              const TradeCostPtr& costfunc,
              const std::vector<OrderBrokerPtr>& other_brokers) {
  HAYAKU_ASSERT(strategy && broker);
  HAYAKU_ASSERT(!stk.isNull());
  HAYAKU_ASSERT(query != Null<KQuery>());
  HAYAKU_CHECK(!strategy->getParam<bool>("buy_delay") &&
                   !strategy->getParam<bool>("sell_delay"),
               "Thie method only support buy|sell on close!");

  auto account =
      makeBrokerAccount(broker, costfunc, strategy->name(), other_brokers);
  auto portfolio_account =
      std::dynamic_pointer_cast<internal::PortfolioAccountPort>(account);
  HAYAKU_CHECK(
      portfolio_account,
      "Execution account does not support strategy capital management");
  strategy->setAccount(std::move(portfolio_account));
  strategy->setSP(SlippagePtr());
  strategy->run(BacktestRequest(stk.getKData(query)));
}

void HAYAKU_API
runInStrategy(const PFPtr& pf, const KQuery& query,
              const OrderBrokerPtr& broker, const TradeCostPtr& costfunc,
              const std::vector<OrderBrokerPtr>& other_brokers) {
  HAYAKU_ASSERT(pf && broker);
  HAYAKU_ASSERT(query != Null<KQuery>());

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
  }

  auto account = makeBrokerAccount(broker, costfunc, pf->name(), other_brokers);
  auto portfolio_account =
      std::dynamic_pointer_cast<internal::PortfolioAccountPort>(account);
  HAYAKU_CHECK(
      portfolio_account,
      "Execution account does not support portfolio capital management");
  pf->setAccount(std::move(portfolio_account));
  pf->run(query, true);
}

}  // namespace hayaku
