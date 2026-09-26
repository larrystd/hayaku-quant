#pragma once

/*
 * Internal process-level data runtime.
 */

#include <atomic>
#include <mutex>
#include <thread>

#include "common/Parameter.h"
#include "common/concurrency/thread.h"
#include "data/Block.h"
#include "data/MarketInfo.h"
#include "data/RealtimeDataSource.h"
#include "data/StockTypeInfo.h"
#include "data/StrategyContext.h"
#include "data/storage/DataDriverFactory.h"

namespace hayaku {

/**
 * Internal process-level data runtime. It owns drivers, caches, preload tasks
 * and data state.
 */
class DataRuntime {
 public:
  DataRuntime();
  ~DataRuntime();

  DataRuntime(const DataRuntime&) = delete;
  DataRuntime& operator=(const DataRuntime&) = delete;

  /**
   * Initialization function, it must be called at the program entry
   * @param baseInfoParam base info driver parameter
   * @param blockParam sector driver parameter
   * @param kdataParam K-line driver parameter
   * @param preloadParam preload parameter
   * @param hayakuParam other parameters
   * @param context strategy context
   */
  void init(const Parameter& baseInfoParam, const Parameter& blockParam,
            const Parameter& kdataParam, const Parameter& preloadParam,
            const Parameter& hayakuParam,
            const StrategyContext& context = StrategyContext({"all"}));

  /** Reload */
  void reload();

  /**
   * Reload with a strategy context parameter; if the security list in the
   * context is empty, the original context is kept
   * @param context strategy context
   */
  void reloadWith(const StrategyContext& context);

  /**
   * Whether it is in IPC client mode (the data is provided by the server, with
   * no local preload buffer)
   * @note Used by core paths such as Stock::realtimeUpdate to decide whether an
   * update has to be forwarded to the main process
   */
  bool isIpcClientMode() const;

  /// For unit tests only: force the client mode flag, in order to verify the
  /// "forward vs local buffer" gating branch of Stock::realtimeUpdate /
  /// getLastUpdateTime. Production code must not call it. The caller has to
  /// reset it when the test case ends, so that other cases in the same process
  /// are not polluted.
  void _testingSetIpcClientMode(bool mode) { ipc_client_mode_ = mode; }

  /** Get the base info driver parameter */
  const Parameter& getBaseInfoDriverParameter() const;

  /** Get the sector driver parameter */
  const Parameter& getBlockDriverParameter() const;

  /** Get the K-line data driver parameter */
  const Parameter& getKDataDriverParameter() const;

  /** Get the preload parameter */
  const Parameter& getPreloadParameter() const;

  /** Get the other parameters */
  const Parameter& getHayakuParameter() const;

  /** Get the strategy context */
  const StrategyContext& getStrategyContext() const;

  /** Get the base info driver */
  BaseInfoDriverPtr getBaseInfoDriver() const;

  /**
   * Get the temporary directory used for temporary variables and the like; the
   * current directory is used when it is not configured It is specified by
   * "tmpdir" in m_config
   */
  const string& tmpdir() const;

  /** Get the data directory */
  const string& datadir() const;

  /** Get the number of securities */
  size_t size() const noexcept;

  /**
   * Get the security instance matching "market abbreviation + security code"
   * @param querystr in the form of "market abbreviation + security code", e.g.
   * "sh000001"
   * @return the matching security instance; Null<Stock>() if it does not exist,
   * no exception is thrown
   */
  Stock getStock(const string& querystr) const;

  /** Same as getStock @see getStock */
  Stock operator[](const string&) const;

  StockList getStockList(std::function<bool(const Stock&)>&& filter =
                             std::function<bool(const Stock&)>()) const;

  /**
   * Get the market information
   * @param market the given market identifier
   * @return the matching market information; Null<MarketInfo>() if it does not
   * exist
   */
  MarketInfo getMarketInfo(const string& market) const noexcept;

  /**
   * Get the representative index security of the given market
   * @param market the given market identifier
   */
  Stock getMarketStock(const string& market) const;

  /**
   * Get the detailed information of the given security type
   * @param type security type
   * @return the matching security type information; Null<StockTypeInf>() if it
   * does not exist
   */
  StockTypeInfo getStockTypeInfo(uint32_t type) const;

  /** Get the information of all security types */
  vector<StockTypeInfo> getStockTypeInfoList() const;

  /** Get the list of market abbreviations */
  StringList getAllMarket() const;

  /** Get all sector categories */
  StringList getAllCategory();

  /**
   * Get a predefined sector
   * @param category sector category
   * @param name sector name
   * @return the sector; empty if it cannot be found
   */
  Block getBlock(const string& category, const string& name);

  void addBlock(const Block& blk) { saveBlock(blk); }

  void saveBlock(const Block& blk);
  void removeBlock(const string& category, const string& name);
  void removeBlock(const Block& blk) {
    removeBlock(blk.category(), blk.name());
  }

  /**
   * Get the sector list of the given category
   * @param category sector category; if it is an empty string, the list of all
   * sectors
   * @return sector list
   */
  BlockList getBlockList(const string& category = "");

  /**
   * Get the sector list of the given index; an empty list is returned if it
   * does not exist
   * @param index_stk index
   * @return sector list
   */
  BlockList getBlockListByIndexStock(const Stock& stk);

  /**
   * Get the sector list that the given security belongs to
   * @param stk the given security
   * @param category sector category; if it is an empty string, the sectors of
   * all categories are returned
   * @return BlockList
   */
  BlockList getStockBelongs(const Stock& stk, const string& category);

  /**
   * Get the trading calendar; currently only "SH" is supported
   * @param query
   * @param market
   * @return DatetimeList
   */
  DatetimeList getTradingCalendar(const KQuery& query,
                                  const string& market = "SH");

  /**
   * Get the merged trading calendar of the given security list (mainly used
   * when securities of different markets are included)
   * @param stk_list
   * @param query
   * @return DatetimeList
   */
  DatetimeList getTradingCalendar(const StockList& stk_list,
                                  const KQuery& query);

  /**
   * Get the 10-year Chinese government bond yield
   */
  const ZhBond10List& getZhBond10() const;

  /**
   * Whether the date of the given time is a holiday (including Saturday and
   * Sunday)
   * @note Only the mainland China market is supported; Null<Datetime>() throws
   * an exception
   * @param d the given time
   */
  bool isHoliday(const Datetime& d) const;

  /**
   * Roughly judge whether the given time is inside the trading hours
   * @param d the given time; Null<Datetime>() throws an exception
   * @param market the given market
   */
  bool isTradingHours(const Datetime& d, const string& market = "SH") const;

  const string& getHistoryFinanceFieldName(size_t ix) const;
  size_t getHistoryFinanceFieldIndex(const string& name) const;
  vector<std::pair<size_t, string>> getHistoryFinanceAllFields() const;

  vector<HistoryFinanceInfo> getHistoryFinance(const Stock& stk, Datetime start,
                                               Datetime end);

  /**
   * Get the equity/dividend adjustment (weight) list of the given security
   * (through the base info driver; in client mode it reads shm first and falls
   * back to IPC/local when not covered)
   * @note Used by DataRuntime::loadAllStockWeights to materialize the full
   * weight set at startup (the isAll branch calls getAllStockWeightList); in
   * client mode it is also used by Stock::getWeight as the lazy-loading
   * fallback for securities that were not materialized (config disabled, added
   * by addStock, newly constructed, ...)
   */
  StockWeightList getStockWeightList(const Stock& stk, Datetime start,
                                     Datetime end);

  /**
   * Add a Stock; only for special Stocks that are added temporarily
   * @param stock
   * @return true on success | false on failure
   */
  bool addStock(const Stock& stock);

  /**
   * Remove the matching Stock from the data runtime; generally used to remove a
   * temporarily added Stock from sm
   * @param market_code
   */
  void removeStock(const string& market_code);

  /**
   * Add a temporary Stock from a CSV file (K-line data); it can be used for a
   * temporary test when the K-line data is only available in CSV format
   * @details The market of the added temporary Stock is "TMP"
   * @param code a self-assigned security code, which must not duplicate an
   * existing Stock, otherwise Null<Stock> is returned
   * @param day_filename daily CSV file name
   * @param min_filename minute CSV file name
   * @param tick minimum tick size, 0.01 by default
   * @param tickValue value of the minimum tick, 0.01 by default
   * @param precision price precision, 2 by default
   * @param minTradeNumber minimum quantity per trade, 1 by default
   * @param maxTradeNumber maximum quantity per trade, 1000000 by default
   * @return
   */
  Stock addTempCsvStock(const string& code, const string& day_filename,
                        const string& min_filename, price_t tick = 0.01,
                        price_t tickValue = 0.01, int precision = 2,
                        size_t minTradeNumber = 1,
                        size_t maxTradeNumber = 1000000);

  /**
   * Remove an added temporary Stock
   * @param code
   */
  void removeTempCsvStock(const string& code);

  /** Whether all the data is ready */
  bool dataReady() const;

  /** Simply block until all the data is ready */
  void waitDataReady() const;

  /** Whether it is being initialized */
  bool initializing() const;

  /**
   * Get the id of the current executing thread; mainly used to tell whether a
   * Strategy runs as a separate process or as a thread
   */
  std::thread::id thread_id() const noexcept { return thread_id_; }

  /** Only used when the program exits!!! */
  ThreadPool* getLoadTaskGroup() { return load_tg_.get(); }

  /** Cancel the loading, used when exiting */
  void cancelLoad() { cancel_load_ = true; }

  bool hasCancelLoad() const { return cancel_load_; }

  /*
   * Wait for the background preload thread to exit (idempotent: it returns
   * immediately when the thread was never started or has already finished). It
   * is only called on the program exit path, after cancelLoad() and before
   * stopping m_load_tg, so that concurrent access (TOCTOU/UAF) to m_load_tg
   * from the preload thread and the exit sequence is eliminated. That thread
   * only loads data and dispatches load events, it performs no nng operation at
   * all, and it checks m_cancel_load all the way, so it exits quickly after
   * being cancelled; therefore this join will not become a new blocking point
   * during the static destruction phase on Windows.
   */
  void joinPreloadThread();

  /**
   * Release the locally cached weight and historical finance data of each
   * security under the shm server role, reclaiming the memory duplicated with
   * the shared-memory snapshot
   * @details Called by the shmserver plugin after the "base info snapshot
   * containing historical finance data" is published successfully (the
   * HISTORY_FINANCE_LOADED branch of _onLoadEvent and the fallback publish at
   * start()). After that, clients read through shared memory, so the server
   * does not need to keep two copies. It really runs only when this process
   * acts as the shm server and is not in client mode, otherwise it is a no-op:
   *          - After the call, m_weight_ready / m_history_finance_ready of each
   * security are set to false, the cache containers are cleared and the memory
   * is returned; later Stock::getWeight / Stock::getHistoryFinance (IPC
   * fallback reply, in-process API access) lazily reload them through the base
   * info driver on demand, so the result stays correct at the cost of the first
   * query of the accessed security;
   *          - On the next data reload, loadAllStockWeights / the historical
   * finance preload materialize them again first, so the next snapshot rebuild
   * is not affected;
   *          - Never call it after publishing only the weight data
   * (include_finance=false, i.e. the first publish after BASE_DATA_READY),
   * otherwise the following publish that includes finance data would read an
   * empty weight table.
   */
  void releaseShmServerBaseInfoCache();

  /**
   * Enable lazy reload after the realtime host releases its local base-info
   * cache following a successful snapshot publish. The application assembly
   * owns this policy; data stores only the resulting cache behavior, not a
   * service-role flag.
   */
  void setBaseInfoCacheEvictionEnabled(bool enabled) noexcept {
    base_info_cache_eviction_enabled_.store(enabled, std::memory_order_release);
  }
  [[nodiscard]] bool isBaseInfoCacheEvictionEnabled() const noexcept {
    return base_info_cache_eviction_enabled_.load(std::memory_order_acquire);
  }

 public:
  typedef StockMapIterator const_iterator;
  const_iterator begin() const { return stock_dict_.begin(); }
  const_iterator end() const { return stock_dict_.end(); }

 private:
  /* Load all the data */
  void loadData();

  /* Get the K-line driver connection pool; the IPC proxy driver pool is
   * returned in client mode
   */
  KDataDriverConnectPoolPtr _getKDataDriverPool();

  /* Pure client negotiation of the shm data service: it only probes and
   * connects to an existing service, degrades to standalone mode on failure,
   * and never starts a service by itself */
  void _negotiateShmServer();

  /* Load the K-line data into the cache */
  void loadAllKData();
  std::unordered_set<string> tryLoadAllKDataFromColumnFirst(
      const vector<KQuery::KType>& ktypes);

  /* Load all the K-line data and historical finance data serially (when the
   * driver does not support parallel loading), executed in a separate thread */
  void _loadAllKDataSerial(vector<KQuery::KType> ktypes,
                           vector<string> low_ktypes);

  /* Load all the K-line data and historical finance data in parallel, executed
   * in a separate thread
   */
  void _loadAllKDataParallel(vector<KQuery::KType> ktypes,
                             vector<string> low_ktypes);

  /*
   * Dispatch the data loading event to the registered plugin callbacks
   * (declared unconditionally; called by loadData and the two loading
   * functions). The core library no longer perceives the existence of the
   * server, it only notifies in order; there is no overhead when no callback is
   * registered.
   * @note Calling register/unregisterLoadEventCallback inside a callback is
   * forbidden (it deadlocks)
   */
  void _fireLoadEvent(LoadEvent event);

  /* Load the holiday information */
  void loadAllHolidays();

  /* Add the market information during initialization */
  void loadAllMarketInfos();

  /* Add the security type information during initialization */
  void loadAllStockTypeInfo();

  /* Load all the securities */
  void loadAllStocks();

  /* Load the internally generated sectors */
  void loadInnerBlocks();

  /* Load all the weight data */
  void loadAllStockWeights();

  /** Load the 10-year Chinese government bond yield data */
  void loadAllZhBond10();

  /** Load the historical financial field index */
  void loadHistoryFinanceField();

 private:
  std::mutex init_mutex_;
  bool initializing_{false};
  std::atomic_bool cancel_load_{
      false};  // Cancel the loading, used as the exit indicator
  std::atomic_bool data_ready_{
      true};  // Indicates whether all the data is ready; true when it
              // has not been initialized
  std::thread::id
      thread_id_;  // Records the thread id, used to tell whether a Strategy
                   // runs as a separate process or as a thread
  string tmpdir_;
  string datadir_;
  BaseInfoDriverPtr base_info_driver_;
  BlockInfoDriverPtr block_driver_;

  // Internally generated sectors, created during initialization and not read
  // from the database
  std::unordered_map<string, Block> inner_blocks_;

  StockMapIterator::stock_map_t stock_dict_;  // SH000001 -> stock
  std::shared_mutex* stock_dict_mutex_;

  typedef unordered_map<string, MarketInfo> MarketInfoMap;
  mutable MarketInfoMap market_info_dict_;

  typedef unordered_map<uint32_t, StockTypeInfo> StockTypeInfoMap;
  mutable StockTypeInfoMap stock_type_info_;

  std::unordered_set<Datetime> holidays_;  // Holidays

  ZhBond10List zh_bond10_;  // 10-year Chinese government bond yield data

  unordered_map<string, size_t>
      field_name_to_ix_;  // Financial field name -> field index
  unordered_map<size_t, string>
      field_ix_to_name_;  // Financial field index -> field name

  Parameter base_info_driver_param_;
  Parameter block_driver_param_;
  Parameter kdata_driver_param_;
  Parameter preload_param_;
  Parameter hayaku_param_;
  StrategyContext context_;

  std::unique_ptr<ThreadPool>
      load_tg_;  // Auxiliary thread group for asynchronous data loading
  std::thread preload_thread_;  // Background preload thread (joinable,
                                // reclaimed by joinPreloadThread when exiting)

  // Whether this process acts as a client of the shm data service (set after a
  // successful connection and the assembly of the proxy driver). The forwarding
  // callback is registered by the plugin itself after a successful connect and
  // unregistered on disconnect; the core library does not hold any plugin type
  // pointer
  bool ipc_client_mode_{false};
  std::atomic_bool base_info_cache_eviction_enabled_{false};
  KDataDriverConnectPoolPtr
      ipc_kdata_pool_;  // IPC K-line driver pool in client mode
};

/** Return the active runtime; it never creates or re-creates one implicitly. */
DataRuntime& getDataRuntime();
/** Create the runtime during explicit application/session assembly. */
DataRuntime& createDataRuntime();
DataRuntime* getDataRuntimeIfExists() noexcept;
void releaseDataRuntime() noexcept;

/** Data loading event callback type */
using LoadEventCallback = std::function<void(LoadEvent)>;

/**
 * Register a data loading event callback, returning the callback id (subscribed
 * when a plugin start()s, used to publish the snapshot at the right moment)
 * @note The lock of the callback container is heap allocated and never
 * released, so this function and its inverse are safe to call during the static
 * destruction phase as well
 */
size_t registerLoadEventCallback(LoadEventCallback&& cb);

/** Unregister a data loading event callback (called when a plugin stop()s); a
 * no-op when the id does not exist */
void unregisterLoadEventCallback(size_t id);

inline size_t DataRuntime::size() const noexcept { return stock_dict_.size(); }

inline bool DataRuntime::dataReady() const {
  return data_ready_.load(std::memory_order_acquire);
}

inline bool DataRuntime::initializing() const { return initializing_; }

inline Stock DataRuntime::operator[](const string& query) const {
  return getStock(query);
}

inline const Parameter& DataRuntime::getBaseInfoDriverParameter() const {
  return base_info_driver_param_;
}

inline const Parameter& DataRuntime::getBlockDriverParameter() const {
  return block_driver_param_;
}

inline const Parameter& DataRuntime::getKDataDriverParameter() const {
  return kdata_driver_param_;
}

inline const Parameter& DataRuntime::getPreloadParameter() const {
  return preload_param_;
}

inline const Parameter& DataRuntime::getHayakuParameter() const {
  return hayaku_param_;
}

inline const StrategyContext& DataRuntime::getStrategyContext() const {
  return context_;
}

inline BaseInfoDriverPtr DataRuntime::getBaseInfoDriver() const {
  return base_info_driver_;
}

inline const string& DataRuntime::getHistoryFinanceFieldName(size_t ix) const {
  return field_ix_to_name_.at(ix);
}

inline size_t DataRuntime::getHistoryFinanceFieldIndex(
    const string& name) const {
  return field_name_to_ix_.at(name);
}

inline vector<HistoryFinanceInfo> DataRuntime::getHistoryFinance(
    const Stock& stk, Datetime start, Datetime end) {
  return base_info_driver_->getHistoryFinance(stk.market(), stk.code(), start,
                                              end);
}

inline StockWeightList DataRuntime::getStockWeightList(const Stock& stk,
                                                       Datetime start,
                                                       Datetime end) {
  return base_info_driver_->getStockWeightList(stk.market(), stk.code(), start,
                                               end);
}

}  // namespace hayaku
