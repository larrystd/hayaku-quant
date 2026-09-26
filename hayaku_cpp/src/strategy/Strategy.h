#pragma once

/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-16
 *     Author: fasiondog
 */

#include <forward_list>
#include <future>
#include <memory>

#include "common/concurrency/FuncWrapper.h"
#include "common/concurrency/ThreadSafeQueue.h"
#include "data/MarketTypes.h"
#include "data/StrategyContext.h"
#include "execution/ExecutionAccountPort.h"
#include "execution/OrderRequest.h"
#include "execution/broker/OrderBrokerBase.h"
#include "execution/pricing/TradeCostBase.h"
#include "extensions/realtime/SpotRecord.h"
#include "strategy/portfolio/Portfolio.h"

namespace hayaku {

class HayakuSession;

/**
 * @ingroup Strategy
 * @{
 */

/**
 * @brief Strategy runtime
 */
class HAYAKU_API Strategy {
  CLASS_LOGGER_IMP(Strategy)
  PARAMETER_SUPPORT_WITH_CHECK

 public:
  Strategy();
  explicit Strategy(const string& name, const string& config_file = "");
  Strategy(const vector<string>& codeList,
           const vector<KQuery::KType>& ktypeList,
           const unordered_map<string, int64_t>& preloadNum = {},
           const string& name = "Strategy", const string& config_file = "");
  explicit Strategy(const StrategyContext& context,
                    const string& name = "Strategy",
                    const string& config_file = "");

  Strategy(const Strategy&) = delete;
  Strategy& operator=(const Strategy&) = delete;

  virtual ~Strategy();

  const string& name() const { return name_; }

  void name(const string& name) { name_ = name; }

  const StrategyContext& context() const { return context_; }

  bool running() const;

  /**
   * Within the daily trading hours, execute the given task periodically with
   * delta as the cycle
   * @param func the task to be executed
   * @param delta the interval time
   * @param market the given market, used to get the opening / closing time
   * @param ignoreMarket whether to ignore the market time limit; if it is true,
   * the periodic execution is not limited by the opening and closing time
   */
  void runDaily(const std::function<void(Strategy*)>& func,
                const TimeDelta& delta, const std::string& market = "SH",
                bool ignoreMarket = false);

  /**
   * Execute the task at the given time every day
   * @param func the task to be executed
   * @param delta the given time
   * @param ignoreHoliday ignore the holidays, i.e. it is not executed on
   * holidays
   */
  void runDailyAt(const std::function<void(Strategy*)>& func,
                  const TimeDelta& delta, bool ignoreHoliday = true);

  /**
   * It is called when the correct data changes, i.e. the change of the
   * corresponding market data is received
   * @note It is usually used for debugging. It is triggered as long as a market
   * data collecting message is received, without being limited by the opening
   * and closing time
   * @param changeFunc the callback function
   */
  void onChange(const std::function<void(Strategy*, const Stock&,
                                         const SpotRecord& spot)>& changeFunc);

  /**
   * Notify after a batch of market data has been received
   * @note It is usually used for the debugging printing only; the batch of
   * market data does not necessarily contain the stock contained in the
   * context, and it is triggered as long as a market data collecting message is
   * received, without being limited by the opening and closing time.
   * @param recievedFucn the callback function
   */
  void onReceivedSpot(
      const std::function<void(Strategy*, const Datetime&)>& recievedFucn);

  /**
   * Start the strategy execution; it must be executed after the related
   * handlers have been registered
   */
  void start(bool autoRecieveSpot = true);

  //==========================================================================
  // The following is the external interface of the strategy runtime; it is
  // recommended to use these interfaces instead of the other functions with the
  // same names, so that the backtest and the live trading stay consistent
  //==========================================================================

  internal::ExecutionAccountPortPtr getAccount() const noexcept {
    return account_;
  }

  void setAccount(internal::ExecutionAccountPortPtr account) noexcept {
    account_ = std::move(account);
  }

  /** Used in the backtest state only */
  SlippagePtr getSP() const noexcept { return sp_; }

  void setSP(const SlippagePtr& slippage) noexcept { sp_ = slippage; }

  // Get the current price; Null<price_t>() is returned when it is invalid
  price_t getCurrentPrice(const Stock& stk, const KQuery::KType& ktype) const;

  /**
   * @brief Get the price at the given time point of the day
   * @param stk the stock object
   * @param time the given time
   * @param ktype K-line type, the minute line by default
   * @return the price; Null<price_t>() is returned when it is invalid
   */
  price_t getPriceByTime(const Stock& stk, const TimeDelta& time,
                         const KQuery::KType& ktype = KQuery::MIN) const;

  KData getLastKData(
      const Stock& stk, const Datetime& start_date, const KQuery::KType& ktype,
      KQuery::RecoverType recover_type = KQuery::NO_RECOVER) const {
    return getKData(stk, start_date, Null<Datetime>(), ktype, recover_type);
  }

  KData getLastKData(
      const Stock& stk, size_t lastnum, const KQuery::KType& ktype,
      KQuery::RecoverType recover_type = KQuery::NO_RECOVER) const;

  /**
   * @brief Place an order by the share number, a positive number buys and a
   * negative number sells
   * @param stk the trading target
   * @param num trade quantity
   * @param remark trade remark
   * @return TradeRecord
   */
  virtual TradeRecord order(const Stock& stk, double num,
                            const string& remark = "");

  /**
   * @brief Place an order by the value, i.e. buy the stocks of the given amount
   * of funds
   * @param stk the trading target
   * @param value the value
   * @param remark trade remark
   * @return TradeRecord
   */
  virtual TradeRecord orderValue(const Stock& stk, price_t value,
                                 const string& remark = "");

  virtual KData getKData(
      const Stock& stk, const Datetime& start_date, const Datetime& end_date,
      const KQuery::KType& ktype,
      KQuery::RecoverType recover_type = KQuery::NO_RECOVER) const;
  virtual Datetime today() const { return Datetime::today(); }

  virtual Datetime now() const { return Datetime::now(); }

  virtual Datetime nextDatetime() const { return Null<Datetime>(); }

  virtual TradeRecord buy(const Stock& stk, price_t price, double num,
                          double stoploss = 0.0, double goal_price = 0.0,
                          OrderOrigin origin = OrderOrigin::SIGNAL,
                          const string& remark = "");

  virtual TradeRecord sell(const Stock& stk, price_t price, double num,
                           price_t stoploss = 0.0, price_t goal_price = 0.0,
                           OrderOrigin origin = OrderOrigin::SIGNAL,
                           const string& remark = "");

  virtual bool isBacktesting() const { return false; }

 protected:
  string name_;
  string config_file_;
  StrategyContext context_;
  std::unique_ptr<HayakuSession> session_;
  internal::ExecutionAccountPortPtr account_;
  SlippagePtr sp_;

  std::function<void(Strategy*, const Datetime&)> on_recieved_spot_;
  std::function<void(Strategy*, const Stock&, const SpotRecord& spot)>
      on_change_;

  struct RunDailyAt {
    std::function<void()> func;
    TimeDelta delta;
    string market;
    bool ignoreMarket{false};
  };
  std::forward_list<RunDailyAt> run_daily_at_list_;

  std::unordered_map<TimeDelta, std::function<void()>> run_daily_at_funcs_;

 protected:
  static std::atomic_bool ms_keep_running;

 protected:
  void _init();

 private:
  void _initParam();
  void _receivedSpot(const SpotRecord& spot);
  void _runDaily();
  void _runDailyAt();

 private:
  static void sig_handler(int sig);
  static void register_signal();
  static std::atomic<bool> ms_sig_registered;

  typedef FuncWrapper event_type;
  ThreadSafeQueue<event_type> event_queue_;  // Message queue

  /** The type of the corresponding future returned after submitting a task to
   * the message queue
   */
  template <typename ResultType>
  using event_handle = std::future<ResultType>;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

  /** Submit a task to the thread pool */
  template <typename FunctionType>
  auto event(FunctionType f) {
    typedef typename std::invoke_result<FunctionType>::type result_type;
    std::packaged_task<result_type()> task(f);
    event_handle<result_type> res(task.get_future());
    event_queue_.push(std::move(task));
    return res;
  }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

  void _startEventLoop();
};

typedef shared_ptr<Strategy> StrategyPtr;

/**
 * @brief Execute the system trading SYS in the strategy runtime
 * @note Currently only the system with both buy_delay and sell_delay equal to
 * false is supported, i.e. the trade is executed at the close
 * @param sys the trading system
 * @param stk the trading object
 * @param query query condition
 * @param broker the order broker (the order broker dedicated to synchronizing
 * with the account assets)
 * @param costfunc the cost function
 * @param other_brokers the other order brokers
 */
void HAYAKU_API
runInStrategy(const internal::StrategyRuntimePtr& strategy, const Stock& stk,
              const KQuery& query, const OrderBrokerPtr& broker,
              const TradeCostPtr& costfunc,
              const std::vector<OrderBrokerPtr>& other_brokers = {});

/**
 * @brief Execute the portfolio strategy PF in the strategy runtime
 * @note Currently only the system with both buy_delay and sell_delay equal to
 * false is supported, i.e. the trade is executed at the close
 * @param pf the portfolio
 * @param query query condition
 * @param broker the order broker (the order broker dedicated to synchronizing
 * with the account assets)
 * @param costfunc the cost function
 * @param other_brokers the other order brokers
 */
void HAYAKU_API
runInStrategy(const PFPtr& pf, const KQuery& query,
              const OrderBrokerPtr& broker, const TradeCostPtr& costfunc,
              const std::vector<OrderBrokerPtr>& other_brokers = {});

/** @} */
}  // namespace hayaku
