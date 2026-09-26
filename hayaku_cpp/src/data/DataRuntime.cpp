/*
 * Internal process-level data runtime implementation.
 */

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "DataRuntime.h"

#include <fmt/format.h>

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <chrono>
#include <cstdlib>

#include "common/Os.h"
#include "common/concurrency/ParallelAlgorithms.h"
#include "common/concurrency/ThreadPool.h"
#include "data/KDataExtension.h"
#include "extensions/ingest/KDataTempCsvDriver.h"
#include "extensions/realtime/ShmClientHook.h"

namespace hayaku {

namespace {

struct DataRuntimeState {
  std::mutex mutex;
  std::unique_ptr<DataRuntime> runtime;
  std::atomic<DataRuntime*> active{nullptr};
  std::string language_path;
};

DataRuntimeState& dataRuntimeState() {
  // Keep the state container process-lived to avoid static-destruction
  // ordering; Session-owned DataRuntime instances are still explicitly
  // released.
  static auto* state = new DataRuntimeState;
  return *state;
}

}  // namespace

DataRuntime& getDataRuntime() {
  auto& state = dataRuntimeState();
  auto* runtime = state.active.load(std::memory_order_acquire);
  HAYAKU_CHECK(
      runtime,
      "DataRuntime is not active; open a HayakuSession before accessing data");
  return *runtime;
}

DataRuntime& createDataRuntime() {
  auto& state = dataRuntimeState();
  if (auto* runtime = state.active.load(std::memory_order_acquire)) {
    return *runtime;
  }
  std::lock_guard<std::mutex> lock(state.mutex);
  if (!state.runtime) {
    state.runtime = std::make_unique<DataRuntime>();
    if (!state.language_path.empty()) {
      state.runtime->setLanguagePath(state.language_path);
    }
  }
  state.active.store(state.runtime.get(), std::memory_order_release);
  return *state.runtime;
}

DataRuntime* getDataRuntimeIfExists() noexcept {
  return dataRuntimeState().active.load(std::memory_order_acquire);
}

void releaseDataRuntime() noexcept {
  std::unique_ptr<DataRuntime> runtime;
  auto& state = dataRuntimeState();
  {
    std::lock_guard<std::mutex> lock(state.mutex);
    state.active.store(nullptr, std::memory_order_release);
    runtime = std::move(state.runtime);
  }
}

void setDataRuntimeLanguagePath(const std::string& path) noexcept {
  auto& state = dataRuntimeState();
  std::lock_guard<std::mutex> lock(state.mutex);
  state.language_path = path;
  if (state.runtime) {
    state.runtime->setLanguagePath(path);
  }
}

DataRuntime::DataRuntime() { stock_dict_mutex_ = new std::shared_mutex; }

DataRuntime::~DataRuntime() {
  // Wait for the background preload thread to exit first: otherwise it would
  // still access the members (m_load_tg) after their destruction, causing a
  // UAF; at the same time it guarantees that m_preload_thread is not joinable
  // at its destruction (otherwise the std::thread destructor would trigger
  // std::terminate). Idempotent: clean() has usually joined already, so calling
  // it again here is a no-op (it also covers the destruction path without
  // clean()).
  joinPreloadThread();
  // Unregister the shm client forwarding callbacks (breaking the reference to
  // the plugin implementation): after that the forwarding call of
  // Stock::realtimeUpdate returns directly, avoiding blocking on an already
  // invalid connection during the exit. The server shutdown has been moved to
  // the plugin (the stopShmServer facade)
  ipc::registerShmClient(ipc::ShmClientForwarders());
  delete stock_dict_mutex_;
  fmt::print("Quit Hayaku system!\n\n");
}

void DataRuntime::init(const Parameter& baseInfoParam,
                       const Parameter& blockParam, const Parameter& kdataParam,
                       const Parameter& preloadParam,
                       const Parameter& hayakuParam,
                       const StrategyContext& context) {
  std::lock_guard<std::mutex> lock(init_mutex_);
  HAYAKU_WARN_IF_RETURN(
      initializing_, void(),
      "The last initialization has not finished. Please try again later!");

  // Prevent a duplicated init
  if (thread_id_ != std::thread::id()) {
    return;
  }
  initializing_ = true;
  thread_id_ = std::this_thread::get_id();
  HAYAKU_CHECK(!context.empty(),
               "No stock code list is included in the context!");

  if (i18n_path_.empty()) {
    loadLocalLanguage(fmt::format("{}/i18n", getDllSelfDir()));
  } else {
    loadLocalLanguage(i18n_path_);
  }

  base_info_driver_param_ = baseInfoParam;
  block_driver_param_ = blockParam;
  kdata_driver_param_ = kdataParam;
  preload_param_ = preloadParam;
  hayaku_param_ = hayakuParam;
  context_ = context;

  // Get the path information
  tmpdir_ = hayakuParam.tryGet<string>("tmpdir", ".");
  datadir_ = hayakuParam.tryGet<string>("datadir", ".");

  // Load the basic security information
  base_info_driver_ = DataDriverFactory::getBaseInfoDriver(baseInfoParam);
  HAYAKU_CHECK(base_info_driver_, "Failed get base info driver!");

  // Get the block driver
  block_driver_ = DataDriverFactory::getBlockDriver(blockParam);

  auto driver = DataDriverFactory::getKDataDriverPool(kdata_driver_param_);
  HAYAKU_CHECK(driver, "driver is null!");
  if (kdata_driver_param_ != driver->getPrototype()->getParameter()) {
    kdata_driver_param_ = driver->getPrototype()->getParameter();
  }

  // The pure client negotiates the shm data service (on a successful connection
  // it is replaced by the proxy driver and the local preload is turned off; on
  // failure it degrades to the standalone mode)
  _negotiateShmServer();

  // Load the data
  loadData();

  // The basic data and the snapshot publishing are now done by the plugin
  // subscribing to LoadEvent (see design §5.2), the core library no longer
  // notifies the readiness actively.

  initializing_ = false;
}

void DataRuntime::loadData() {
  std::chrono::system_clock::time_point start_time =
      std::chrono::system_clock::now();
  data_ready_.store(false, std::memory_order_release);

  loadAllHolidays();
  loadAllMarketInfos();
  loadAllStockTypeInfo();
  loadAllStocks();
  loadInnerBlocks();
  loadAllStockWeights();
  // The ex-rights/ex-dividend data is ready, dispatch the BASE_DATA_READY
  // event: the plugin publishes a basic information snapshot accordingly (the
  // historical finance is rebuilt after the preload thread finishes). The two
  // publishes happen in the main thread and the preload thread respectively; at
  // this moment the historical finance has not been preloaded yet, so the
  // plugin side must publish with include_finance=false, otherwise the
  // historical finance of every security would be lazily loaded one by one (see
  // Stock::getHistoryFinance).
  _fireLoadEvent(LoadEvent::BASE_DATA_READY);
  loadAllZhBond10();
  loadHistoryFinanceField();

  HAYAKU_INFO(htr("Loading block..."));
  block_driver_->load();
  // The blocks are loaded, dispatch the BLOCKS_LOADED event: the plugin
  // refreshes the block cache of the IPC service accordingly (the former
  // refreshBlocks)
  _fireLoadEvent(LoadEvent::BLOCKS_LOADED);

  // Get the K-line data driver and preload the given data
  HAYAKU_INFO(htr("Loading KData..."));

  // Load the K-lines and the historical financial information
  loadAllKData();

  std::chrono::duration<double> sec =
      std::chrono::system_clock::now() - start_time;
  auto seconds = sec.count();
  HAYAKU_INFO(htr("{:<.2f}s Loaded Data.", seconds));
}

KDataDriverConnectPoolPtr DataRuntime::_getKDataDriverPool() {
  if (ipc_kdata_pool_) {
    return ipc_kdata_pool_;
  }
  return DataDriverFactory::getKDataDriverPool(kdata_driver_param_);
}

void DataRuntime::_negotiateShmServer() {
  // The master gate: when it is off nothing participates at all (no detection,
  // no mapping, no forwarding) and the behavior equals to the feature being
  // disabled. It is off by default (the process runs in the standalone mode by
  // default); connecting to an existing service as a client requires enabling
  // it explicitly in the config
  HAYAKU_IF_RETURN(!hayaku_param_.tryGet<bool>("use_shm_server", false),
                   void());
  // The app-side resolver returns no source when this process is a service
  // host. The data runtime only asks for an explicitly assembled client source;
  // it does not decide the service role or discover a plugin itself. A missing
  // source degrades to the configured local driver.
  auto* source = getRealtimeDataSource();
  HAYAKU_IF_RETURN(!source, void());

  // Connect to the existing service and wait for its data to be ready (the
  // retries, the interruption check and the forwarding registration are done
  // inside the plugin; the client never starts the service itself)
  auto wait_timeout =
      hayaku_param_.tryGet<int64_t>("shm_server_wait_timeout", 600);
  HAYAKU_WARN_IF_RETURN(
      !source->connect(datadir_,
                       wait_timeout < 0 ? 0 : (uint64_t)wait_timeout),
      void(),
      "Failed connect to hayaku shm server, fallback to standalone mode!");

  // Switch to the client mode: install the proxy driver provided by the plugin
  // and turn off the local preload (an in-memory override only, the config file
  // is not modified)
  ipc_client_mode_ = true;
  base_info_driver_ = source->createBaseInfoDriver(base_info_driver_);
  block_driver_ = source->createBlockDriver(block_driver_);
  // The whole local driver connection pool is passed in (instead of its
  // prototype): the types not preloaded by the service process and the
  // time-sharing / tick data are served by the local driver of the client
  // directly, and a connection must be taken from the pool to avoid multiple
  // clones reusing the same connection / file handle concurrently
  auto local_pool = DataDriverFactory::getKDataDriverPool(kdata_driver_param_);
  ipc_kdata_pool_ = std::make_shared<KDataDriverConnectPool>(
      source->createKDataDriver(local_pool));
  for (const auto& ktype : KQuery::getBaseKTypeList()) {
    auto low_ktype = ktype;
    to_lower(low_ktype);
    preload_param_.set<bool>(low_ktype, false);
  }
  // The client has no preload buffer; the update is applied to the buffer by
  // the service process and mirrored to the shared memory, visible to all the
  // clients
  HAYAKU_INFO("Connected to hayaku shm server: {}, running in client mode.",
              source->serverAddr());
}

bool DataRuntime::isIpcClientMode() const { return ipc_client_mode_; }

// ── LoadEvent event bus (the core library only dispatches and the plugin
// subscribes; see design §5.2)
namespace {
// The callback container is held with new entirely and never deleted: if its
// member lock were destroyed during the static destruction, stopShmServer()
// would throw EINVAL while locking when unregistering the callbacks in clean()
// (the static destruction period) and the noexcept destruction chain would call
// std::terminate (the same as constraint 2 of the old §4.7). Therefore the
// container itself is also placed on the heap and never released.
struct LoadEventState {
  std::shared_mutex mutex;
  std::vector<std::pair<size_t, LoadEventCallback>> callbacks;
  size_t next_id{1};
};
LoadEventState* g_load_event = new LoadEventState;
}  // namespace

size_t registerLoadEventCallback(LoadEventCallback&& cb) {
  std::unique_lock<std::shared_mutex> lock(g_load_event->mutex);
  size_t id = g_load_event->next_id++;
  g_load_event->callbacks.emplace_back(id, std::move(cb));
  return id;
}

void unregisterLoadEventCallback(size_t id) {
  std::unique_lock<std::shared_mutex> lock(g_load_event->mutex);
  auto& v = g_load_event->callbacks;
  v.erase(std::remove_if(v.begin(), v.end(),
                         [id](const std::pair<size_t, LoadEventCallback>& p) {
                           return p.first == id;
                         }),
          v.end());
}

void DataRuntime::_fireLoadEvent(LoadEvent event) {
  // There are very few callbacks (the plugin registers them once each at start
  // / stop) and they are triggered synchronously only in the loading sequence;
  // with no registration the traversal is empty and the cost is negligible
  std::shared_lock<std::shared_mutex> lock(g_load_event->mutex);
  for (const auto& [id, cb] : g_load_event->callbacks) {
    cb(event);
  }
}

void DataRuntime::joinPreloadThread() {
  if (preload_thread_.joinable()) {
    preload_thread_.join();
  }
}

void DataRuntime::loadAllKData() {
  // Control the loading order by the K-line type
  vector<KQuery::KType> ktypes;
  vector<string> low_ktypes;

  // If the context gives a ktype list, load in the order of the ktypes given by
  // the context, otherwise load in the default order
  const auto& context_ktypes = context_.getKTypeList();
  if (context_ktypes.empty()) {
    ktypes = KQuery::getBaseKTypeList();

  } else {
    // Override the global preload parameters with the context preload
    // parameters
    ktypes = context_ktypes;
    for (const auto& ktype : ktypes) {
      auto low_ktype = ktype;
      to_lower(low_ktype);
      preload_param_.set<bool>(low_ktype, true);
    }
  }

  const auto& context_preload_num = context_.getPreloadNum();
  low_ktypes.reserve(ktypes.size());
  for (const auto& ktype : ktypes) {
    auto& back = low_ktypes.emplace_back(ktype);
    to_lower(back);

    // Judge whether the context gives the preload numbers; when it does, they
    // override the default values
    string preload_key = fmt::format("{}_max", back);
    auto context_iter = context_preload_num.find(preload_key);
    if (context_iter != context_preload_num.end()) {
      preload_param_.set<int64_t>(preload_key, context_iter->second);
    }

    int64_t preload_max_num = preload_param_.tryGet<int64_t>(preload_key, 0);
    if (preload_max_num <= 0) {
      preload_max_num = std::numeric_limits<int64_t>::max();
      preload_param_.set<int64_t>(preload_key, preload_max_num);
      HAYAKU_INFO_IF(
          preload_param_.tryGet<bool>(back, false),
          htr("Preloading {} kdata to buffer (max: no limit)!", back));
    } else {
      HAYAKU_INFO_IF(preload_param_.tryGet<bool>(back, false),
                     htr("Preloading {} kdata to buffer (max: {})!", back,
                         preload_max_num));
    }
  }

  bool lazy_preload = hayaku_param_.tryGet<bool>("lazy_preload", false);
  HAYAKU_INFO_IF(lazy_preload && canLazyLoad(KQuery::MIN),
                 htr("Use lazy preload!"));

  // Load the K-lines of the same kind first (the preload is only a cache
  // warm-up, it always runs asynchronously in the background and does not block
  // the initialization; the queries during the warm-up are fetched from the
  // driver in real time and their results are not affected; the scenarios
  // needing to wait for the warm-up to finish can call waitDataReady()
  // explicitly)
  auto driver = _getKDataDriverPool();
  if (isIpcClientMode()) {
    // In the client mode the data is provided by the server, there is no local
    // preload task and it is ready directly
    data_ready_.store(true, std::memory_order_release);
    return;
  }

  // The preload thread is now the joinable member m_preload_thread (no longer
  // detached): at the exit joinPreloadThread() waits for its exit before
  // stopping m_load_tg / destroying the IPC service, eradicating the concurrent
  // access race (C3). If the previous preload thread still exists (a duplicated
  // initialization), join it before the new assignment, avoiding terminate on
  // assigning a joinable thread.
  joinPreloadThread();
  if (!driver->getPrototype()->canParallelLoad()) {
    preload_thread_ = std::thread([this, ktypes, low_ktypes]() mutable {
      _loadAllKDataSerial(std::move(ktypes), std::move(low_ktypes));
    });
  } else {
    // Asynchronous parallel loading
    preload_thread_ = std::thread([this, ktypes, low_ktypes]() mutable {
      _loadAllKDataParallel(std::move(ktypes), std::move(low_ktypes));
    });
  }
}

void DataRuntime::_loadAllKDataSerial(vector<KQuery::KType> ktypes,
                                      vector<string> low_ktypes) {
  // The progress reporting has moved to the plugin side (through LoadEvent +
  // its own polling, see design §5.2); loaded/total is no longer counted here

  for (size_t i = 0, len = ktypes.size(); i < len; i++) {
    if (cancel_load_) {
      break;
    }
    if (canLazyLoad(ktypes[i])) {
      continue;
    }
    std::shared_lock<std::shared_mutex> lock(*stock_dict_mutex_);
    for (auto iter = stock_dict_.begin(); iter != stock_dict_.end(); ++iter) {
      if (cancel_load_) {
        break;
      }
      const auto& low_ktype = low_ktypes[i];
      if (preload_param_.tryGet<bool>(low_ktype, false)) {
        iter->second.loadKDataToBuffer(ktypes[i]);
      }
    }
  }

  // Dispatch KDATA_PRELOAD_FINISHED before the historical finance is loaded, so
  // that the clients get the hot K-line data as early as possible; it is not
  // dispatched when the preload is cancelled (the process exit), avoiding a
  // full serialization that is destroyed immediately
  if (!cancel_load_) {
    _fireLoadEvent(LoadEvent::KDATA_PRELOAD_FINISHED);
  }

  if (!cancel_load_ &&
      hayaku_param_.tryGet<bool>("load_history_finance", true)) {
    ThreadPool tg;
    std::shared_lock<std::shared_mutex> lock(*stock_dict_mutex_);
    for (auto iter = stock_dict_.begin(); iter != stock_dict_.end(); ++iter) {
      if (cancel_load_) {
        break;
      }
      tg.submit([stk = iter->second, this]() {
        HAYAKU_IF_RETURN(cancel_load_, void());
        stk.getHistoryFinance();
      });
    }
    lock.unlock();
    tg.join();
  }

  // The historical finance is ready, dispatch HISTORY_FINANCE_LOADED: the
  // plugin rebuilds the basic snapshot as a whole accordingly (the
  // ex-rights/ex-dividend data and the finance are collected together); The
  // shared memory snapshot of an already connected session is fixed after the
  // negotiation during the connection and does not switch generation
  // automatically with a republish at runtime; the new snapshot is visible only
  // to the sessions negotiated afterwards; it is not dispatched when the
  // preload is cancelled (the process exit): this avoids both a useless full
  // publish and a pointless serialization in the exit sequence
  if (!cancel_load_) {
    _fireLoadEvent(LoadEvent::HISTORY_FINANCE_LOADED);
  }

  data_ready_.store(true, std::memory_order_release);
}

void DataRuntime::_loadAllKDataParallel(vector<KQuery::KType> ktypes,
                                        vector<string> low_ktypes) {
  // The progress reporting has moved to the plugin side (see design §5.2);
  // loaded/total is no longer counted here
  auto loaded_codes = tryLoadAllKDataFromColumnFirst(ktypes);

  // Load the K-lines of the other securities (they may use different K-line
  // drivers)
  this->load_tg_ = std::make_unique<ThreadPool>();
  for (size_t i = 0, len = ktypes.size(); i < len; i++) {
    if (cancel_load_) {
      break;
    }
    if (canLazyLoad(ktypes[i])) {
      continue;
    }
    std::shared_lock<std::shared_mutex> lock(*stock_dict_mutex_);
    for (auto iter = stock_dict_.begin(); iter != stock_dict_.end(); ++iter) {
      if (cancel_load_) {
        break;
      }
      if (loaded_codes.find(iter->first) != loaded_codes.end()) {
        continue;
      }
      if (preload_param_.tryGet<bool>(low_ktypes[i], false)) {
        // ktypes[i] is reused by the inner stock loop within the outer ktype
        // loop; a std::move here would make the first stock submit an empty
        // moved-from ktypes[i], and the following stocks would call
        // loadKDataToBuffer("") and all fail (only the first stock fills the
        // preload buffer). Therefore a copy is used; ktype is a short string
        // and the cost is negligible.
        load_tg_->submit(
            [this, stk = iter->second, ktype = ktypes[i]]() mutable {
              HAYAKU_IF_RETURN(cancel_load_, void());
              stk.loadKDataToBuffer(ktype);
            });
      }
    }
  }

  // Wait for all the K-line preload tasks to finish and then dispatch
  // KDATA_PRELOAD_FINISHED for the plugin to publish the shared memory
  // snapshot; note that the dispatch must happen after the join, otherwise the
  // buffer may not be filled yet; it is not dispatched when the preload is
  // cancelled (the process exit), avoiding a full serialization that is
  // destroyed immediately
  load_tg_->join();
  load_tg_.reset();

  if (!cancel_load_) {
    _fireLoadEvent(LoadEvent::KDATA_PRELOAD_FINISHED);
  }

  if (!cancel_load_ &&
      hayaku_param_.tryGet<bool>("load_history_finance", true)) {
    load_tg_ = std::make_unique<ThreadPool>();
    std::shared_lock<std::shared_mutex> lock(*stock_dict_mutex_);
    for (auto iter = stock_dict_.begin(); iter != stock_dict_.end(); ++iter) {
      if (cancel_load_) {
        break;
      }
      if (loaded_codes.find(iter->first) != loaded_codes.end()) {
        continue;
      }
      load_tg_->submit([this, stk = iter->second]() {
        HAYAKU_IF_RETURN(cancel_load_, void());
        stk.getHistoryFinance();
      });
    }
    lock.unlock();
    load_tg_->join();
    load_tg_.reset();
  }

  // The historical finance is ready, dispatch HISTORY_FINANCE_LOADED: the
  // plugin rebuilds the basic snapshot as a whole accordingly (the
  // ex-rights/ex-dividend data and the finance are collected together); The
  // snapshot of an already connected session is fixed for the session lifetime
  // and does not switch generation automatically with a republish at runtime (a
  // new session maps the latest epoch at the negotiation); it is not dispatched
  // when the preload is cancelled (the process exit), for the same reason as
  // the serial branch
  if (!cancel_load_) {
    _fireLoadEvent(LoadEvent::HISTORY_FINANCE_LOADED);
  }

  data_ready_.store(true, std::memory_order_release);
}

std::unordered_set<string> DataRuntime::tryLoadAllKDataFromColumnFirst(
    const vector<KQuery::KType>& ktypes) {
  std::unordered_set<string> loaded_codes;
  HAYAKU_IF_RETURN(!context_.isAll(), loaded_codes);
  auto driver = _getKDataDriverPool();
  HAYAKU_IF_RETURN(!driver || !driver->getPrototype()->isColumnFirst(),
                   loaded_codes);

  // Try to load the SH000001 K-lines with priority
  Stock sh000001;
  {
    std::shared_lock<std::shared_mutex> lock(*stock_dict_mutex_);
    auto sh000001_iter = stock_dict_.find("SH000001");
    if (sh000001_iter != stock_dict_.end()) {
      sh000001 = sh000001_iter->second;
    }
  }

  HAYAKU_IF_RETURN(sh000001.isNull(), loaded_codes);

  for (size_t i = 0, len = ktypes.size(); i < len; i++) {
    if (cancel_load_) {
      break;
    }
    auto low_ktype = ktypes[i];
    to_lower(low_ktype);
    if (preload_param_.tryGet<bool>(low_ktype, false)) {
      sh000001.loadKDataToBuffer(ktypes[i]);
    }
  }

  HAYAKU_IF_RETURN(cancel_load_, loaded_codes);

  // It is mainly bandwidth limited, no multi-threading is needed
  for (size_t i = 0, len = ktypes.size(); i < len; i++) {
    if (cancel_load_) {
      break;
    }

    if (canLazyLoad(ktypes[i])) {
      continue;
    }

    auto low_ktype = ktypes[i];
    to_lower(low_ktype);
    if (!preload_param_.tryGet<bool>(low_ktype, false)) {
      continue;
    }

    auto k = sh000001.getKRecord(0, ktypes[i]);
    if (k.isValid()) {
      auto datas = driver->getConnect()->getAllKRecordList(
          ktypes[i], k.datetime, cancel_load_);
      if (!datas.empty() && !cancel_load_) {
        std::shared_lock<std::shared_mutex> lock(*stock_dict_mutex_);
        for (auto iter = stock_dict_.begin(); iter != stock_dict_.end();
             ++iter) {
          if (cancel_load_) {
            break;
          }
          auto date_iter = datas.find(iter->second.market_code());
          if (date_iter != datas.end()) {
            iter->second.loadKDataToBufferFromKRecordList(
                ktypes[i], std::move(date_iter->second));
            loaded_codes.insert(iter->second.market_code());
          }
        }
      }
    }
  }

  if (!cancel_load_ &&
      hayaku_param_.tryGet<bool>("load_history_finance", true)) {
    auto finances = base_info_driver_->getAllHistoryFinance(cancel_load_);
    if (!finances.empty() && !cancel_load_) {
      std::shared_lock<std::shared_mutex> lock(*stock_dict_mutex_);
      for (auto iter = stock_dict_.begin(); iter != stock_dict_.end(); ++iter) {
        if (cancel_load_) {
          break;
        }
        auto finance_iter = finances.find(iter->second.market_code());
        if (finance_iter != finances.end()) {
          iter->second.setHistoryFinance(std::move(finance_iter->second));
        }
      }
    }
  }

  return loaded_codes;
}

void DataRuntime::reload() {
  HAYAKU_IF_RETURN(initializing_, void());
  initializing_ = true;

  HAYAKU_INFO("start reload ...");
  loadData();
  initializing_ = false;
}

void DataRuntime::reloadWith(const StrategyContext& context) {
  HAYAKU_IF_RETURN(initializing_, void());
  initializing_ = true;

  if (!context.empty()) {
    context_ = context;
  } else {
    HAYAKU_INFO(htr("The new context is empty, use the original context"));
  }

  HAYAKU_INFO("start reload ...");
  loadData();
  initializing_ = false;
}

const string& DataRuntime::tmpdir() const { return tmpdir_; }

const string& DataRuntime::datadir() const { return datadir_; }

Stock DataRuntime::getStock(const string& querystr) const {
  Stock result;
  string query_str = querystr;
  to_upper(query_str);
  size_t pos = query_str.find('.');
  if (pos != string::npos) {
    // The suffix notation
    std::string suffix = query_str.substr(pos + 1);
    std::string prefix = query_str.substr(0, pos);
    query_str = suffix + prefix;
  }
  std::shared_lock<std::shared_mutex> lock(*stock_dict_mutex_);
  auto iter = stock_dict_.find(query_str);
  return (iter != stock_dict_.end()) ? iter->second : result;
}

StockList DataRuntime::getStockList(
    std::function<bool(const Stock&)>&& filter) const {
  StockList ret;
  std::shared_lock<std::shared_mutex> lock(*stock_dict_mutex_);
  ret.reserve(stock_dict_.size());
  auto iter = stock_dict_.begin();
  if (filter) {
    for (; iter != stock_dict_.end(); ++iter) {
      if (filter(iter->second)) {
        ret.emplace_back(iter->second);
      }
    }
  } else {
    for (; iter != stock_dict_.end(); ++iter) {
      ret.emplace_back(iter->second);
    }
  }
  return ret;
}

MarketInfo DataRuntime::getMarketInfo(const string& market) const noexcept {
  MarketInfo result;
  string market_tmp = market;
  to_upper(market_tmp);

  auto iter = market_info_dict_.find(market_tmp);
  if (iter != market_info_dict_.end()) {
    result = iter->second;
  } else {
    result = base_info_driver_->getMarketInfo(market_tmp);
    if (result != Null<MarketInfo>()) {
      market_info_dict_[market_tmp] = result;
    }
  }
  return result;
}

Stock DataRuntime::getMarketStock(const string& market) const {
  auto market_info = getMarketInfo(market);
  return getStock(
      fmt::format("{}{}", market_info.market(), market_info.code()));
}

StockTypeInfo DataRuntime::getStockTypeInfo(uint32_t type) const {
  StockTypeInfo result;
  auto iter = stock_type_info_.find(type);
  if (iter != stock_type_info_.end()) {
    result = iter->second;
  } else {
    result = base_info_driver_->getStockTypeInfo(type);
    if (result != Null<StockTypeInfo>()) {
      stock_type_info_[type] = result;
    }
  }
  return result;
}

vector<StockTypeInfo> DataRuntime::getStockTypeInfoList() const {
  vector<StockTypeInfo> result;
  result.reserve(stock_type_info_.size());
  for (const auto& item : stock_type_info_) {
    result.push_back(item.second);
  }
  return result;
}

StringList DataRuntime::getAllMarket() const {
  StringList result;
  auto iter = market_info_dict_.begin();
  for (; iter != market_info_dict_.end(); ++iter) {
    result.push_back(iter->first);
  }
  return result;
}

StringList DataRuntime::getAllCategory() {
  return block_driver_ ? block_driver_->getAllCategory() : StringList();
}

Block DataRuntime::getBlock(const string& category, const string& name) {
  Block result;
  HAYAKU_IF_RETURN(!block_driver_ || category.empty() || name.empty(), result);
  auto iter = inner_blocks_.find(fmt::format("{}_{}", category, name));
  if (iter != inner_blocks_.end()) {
    return iter->second;
  }
  result = block_driver_->getBlock(category, name);
  return result;
}

void DataRuntime::saveBlock(const Block& blk) {
  if (block_driver_) {
    HAYAKU_CHECK(!blk.category().empty(), "block's category can not be empty!");
    HAYAKU_CHECK(!blk.name().empty(), "block's name can not be empty!");
    block_driver_->save(blk);
  }
}
void DataRuntime::removeBlock(const string& category, const string& name) {
  if (block_driver_) {
    block_driver_->remove(category, name);
  }
}

BlockList DataRuntime::getBlockList(const string& category) {
  BlockList result;
  HAYAKU_IF_RETURN(!block_driver_, BlockList());
  result = category.empty() ? block_driver_->getBlockList()
                            : block_driver_->getBlockList(category);
  auto iter = inner_blocks_.begin();
  if (category.empty()) {
    for (; iter != inner_blocks_.end(); ++iter) {
      result.push_back(iter->second);
    }
  } else {
    for (; iter != inner_blocks_.end(); ++iter) {
      if (iter->first == category) {
        result.push_back(iter->second);
      }
    }
  }
  return result;
}

BlockList DataRuntime::getBlockListByIndexStock(const Stock& stk) {
  BlockList all = getBlockList();
  BlockList result;
  for (const auto& blk : all) {
    if (blk.getIndexStock() == stk) {
      result.push_back(blk);
    }
  }
  return result;
}

BlockList DataRuntime::getStockBelongs(const Stock& stk,
                                       const string& category) {
  BlockList result;
  BlockList all = getBlockList(category);
  for (const auto& blk : all) {
    if (blk.have(stk)) {
      result.push_back(blk);
    }
  }
  return result;
}

DatetimeList DataRuntime::getTradingCalendar(const KQuery& query,
                                             const string& market) {
  auto marketinfo = getMarketInfo(market);
  return getStock(fmt::format("{}{}", marketinfo.market(), marketinfo.code()))
      .getDatetimeList(query);
}

DatetimeList DataRuntime::getTradingCalendar(const StockList& stk_list,
                                             const KQuery& query) {
  std::unordered_set<string> markets;
  for (const auto& stk : stk_list) {
    if (!stk.isNull()) {
      markets.insert(stk.market());
    }
  }

  std::set<Datetime> date_set;
  for (const auto& market : markets) {
    DatetimeList temp = getTradingCalendar(query, market);
    if (temp.size() > 0) {
      date_set.insert(temp.begin(), temp.end());
    }
  }

  DatetimeList result;
  result.reserve(date_set.size());
  for (const auto& date : date_set) {
    result.push_back(date);
  }

  return result;
}

const ZhBond10List& DataRuntime::getZhBond10() const { return zh_bond10_; }

bool DataRuntime::isHoliday(const Datetime& d) const {
  HAYAKU_IF_RETURN(d.dayOfWeek() == 0 || d.dayOfWeek() == 6, true);
  return holidays_.count(d.startOfDay());
}

bool DataRuntime::isTradingHours(const Datetime& d,
                                 const string& market) const {
  HAYAKU_IF_RETURN(isHoliday(d), false);
  auto hour = d - d.startOfDay();
  MarketInfo marketinfo = getMarketInfo(market);
  HAYAKU_CHECK(marketinfo != Null<MarketInfo>(), "{}: {}!",
               htr("Not found market info"), market);
  HAYAKU_IF_RETURN(
      (hour >= marketinfo.openTime1() && hour <= marketinfo.closeTime1()) ||
          (hour >= marketinfo.openTime2() && hour <= marketinfo.closeTime2()),
      true);
  return false;
}

Stock DataRuntime::addTempCsvStock(const string& code,
                                   const string& day_filename,
                                   const string& min_filename, price_t tick,
                                   price_t tickValue, int precision,
                                   size_t minTradeNumber,
                                   size_t maxTradeNumber) {
  string new_code(code);
  to_upper(new_code);
  Stock result("TMP", new_code, day_filename, STOCKTYPE_TMP, true,
               Datetime(199901010000), Null<Datetime>(), tick, tickValue,
               precision, minTradeNumber, maxTradeNumber);

  Parameter param;
  param.set<string>("type", "TMPCSV");
  auto driver_pool = DataDriverFactory::getKDataDriverPool(param);
  auto driver = driver_pool->getPrototype();
  KDataTempCsvDriver* p = dynamic_cast<KDataTempCsvDriver*>(driver.get());
  p->setDayFileName(day_filename);
  p->setMinFileName(min_filename);
  result.setKDataDriver(driver_pool);
  result.loadKDataToBuffer(KQuery::DAY);
  result.loadKDataToBuffer(KQuery::MIN);
  return addStock(result) ? result : Null<Stock>();
}

void DataRuntime::removeTempCsvStock(const string& code) {
  removeStock(fmt::format("TMP{}", code));
}

bool DataRuntime::addStock(const Stock& stock) {
  string market_code(stock.market_code());
  to_upper(market_code);
  std::unique_lock<std::shared_mutex> lock(*stock_dict_mutex_);
  HAYAKU_ERROR_IF_RETURN(stock_dict_.find(market_code) != stock_dict_.end(),
                         false, "The stock had exist! {}", market_code);
  stock_dict_[market_code] = stock;
  return true;
}

void DataRuntime::removeStock(const string& market_code) {
  string n_market_code(market_code);
  to_upper(n_market_code);
  std::unique_lock<std::shared_mutex> lock(*stock_dict_mutex_);
  auto iter = stock_dict_.find(n_market_code);
  if (iter != stock_dict_.end()) {
    stock_dict_.erase(iter);
  }
}

void DataRuntime::loadAllStocks() {
  HAYAKU_INFO(htr("Loading stock information..."));
  vector<StockInfo> stockInfos;
  if (context_.isAll()) {
    stockInfos = base_info_driver_->getAllStockInfo();
  } else {
    auto load_stock_code_list = context_.getAllNeedLoadStockCodeList();
    auto all_market = getAllMarket();
    for (auto stkcode : load_stock_code_list) {
      to_upper(stkcode);
      bool find = false;
      for (auto& market : all_market) {
        auto pos = stkcode.find(market);
        if (pos != string::npos && market.size() <= stkcode.size()) {
          string stk_market = stkcode.substr(pos, market.size());
          string stk_code = stkcode.substr(market.size(), stkcode.size());
          stockInfos.push_back(
              base_info_driver_->getStockInfo(stk_market, stk_code));
          find = true;
          break;
        }
      }
      HAYAKU_WARN_IF(!find, "Invalid stock code: {}", stkcode);
    }
  }

  auto base_ktypes = KQuery::getBaseKTypeList();
  vector<KQuery::KType> preload_ktypes;
  for (const auto& ktype : base_ktypes) {
    auto nktype = ktype;
    to_lower(nktype);
    if (preload_param_.tryGet<bool>(nktype, false)) {
      preload_ktypes.push_back(ktype);
    }
  }

  auto kdriver = _getKDataDriverPool();

  std::unique_lock<std::shared_mutex> lock(*stock_dict_mutex_);
  for (auto& info : stockInfos) {
    Datetime startDate, endDate;
    try {
      startDate = Datetime(info.startDate * 10000LL);
    } catch (...) {
      startDate = Null<Datetime>();
    }
    try {
      endDate = Datetime(info.endDate * 10000LL);
    } catch (...) {
      endDate = Null<Datetime>();
    }

    string market_code = fmt::format("{}{}", info.market, info.code);
    to_upper(market_code);

    auto iter = stock_dict_.find(market_code);
    if (iter == stock_dict_.end()) {
      Stock _stock(info.market, info.code, info.name, info.type, info.valid,
                   startDate, endDate, info.tick, info.tickValue,
                   info.precision, info.minTradeNumber, info.maxTradeNumber);
      _stock.setKDataDriver(kdriver);
      _stock.setPreload(preload_ktypes);
      stock_dict_[market_code] = std::move(_stock);
    } else {
      Stock& stock = iter->second;
      if (!stock.data_) {
        stock.data_ = shared_ptr<Stock::Data>(new Stock::Data(
            info.market, info.code, info.name, info.type, info.valid, startDate,
            endDate, info.tick, info.tickValue, info.precision,
            info.minTradeNumber, info.maxTradeNumber));
      } else {
        stock.data_->market_ = info.market;
        stock.data_->code_ = info.code;
        stock.data_->name_ = info.name;
        stock.data_->type_ = info.type;
        stock.data_->valid_ = info.valid;
        stock.data_->start_date_ = startDate;
        stock.data_->last_date_ = endDate;
        stock.data_->tick_ = info.tick;
        stock.data_->tick_value_ = info.tickValue;
        stock.data_->precision_ = info.precision;
        stock.data_->min_trade_number_ = info.minTradeNumber;
        stock.data_->max_trade_number_ = info.maxTradeNumber;
        stock.data_->history_finance_ready_ = false;
        // Force releasing all the cached K-line data
        stock.data_->last_update_.clear();
        for (const auto& ktype : base_ktypes) {
          stock.releaseKDataBuffer(ktype);
          stock.data_->last_update_[ktype] = Datetime::min();
        }
        auto ktype_list = KQuery::getExtraKTypeList();
        for (const auto& ktype : ktype_list) {
          stock.data_->last_update_[ktype] = Datetime::min();
        }
      }
      stock.setPreload(preload_ktypes);
      if (!stock.getKDataDirver()) {
        stock.setKDataDriver(kdriver);
      }
    }
  }
}

void DataRuntime::loadAllMarketInfos() {
  HAYAKU_INFO(htr("Loading market information..."));
  auto marketInfos = base_info_driver_->getAllMarketInfo();
  market_info_dict_.clear();
  market_info_dict_.reserve(marketInfos.size());
  for (auto& marketInfo : marketInfos) {
    string market = marketInfo.market();
    to_upper(market);
    market_info_dict_[market] = marketInfo;
  }

  // add special Market, for temp csv file
  market_info_dict_["TMP"] = MarketInfo(
      "TMP", "Temp Csv file", "temp load from csv file", "000001",
      Null<Datetime>(), TimeDelta(0), TimeDelta(0), TimeDelta(0), TimeDelta(0));
}

void DataRuntime::loadAllStockTypeInfo() {
  HAYAKU_INFO(htr("Loading stock type information..."));
  auto stkTypeInfos = base_info_driver_->getAllStockTypeInfo();
  stock_type_info_.clear();
  stock_type_info_.reserve(stkTypeInfos.size());
  for (auto& stkTypeInfo : stkTypeInfos) {
    stock_type_info_[stkTypeInfo.type()] = stkTypeInfo;
  }
}

void DataRuntime::loadAllHolidays() {
  auto holidays = base_info_driver_->getAllHolidays();
  std::unordered_set<Datetime> tmp_holidays(holidays.begin(), holidays.end());
  holidays_ = std::move(tmp_holidays);
}

void DataRuntime::loadInnerBlocks() {
  Block blocka = Block("A", "ALL");
  Block blocksh = Block("A", "SH");
  Block blocksz = Block("A", "SZ");
  Block blockbj = Block("A", "BJ");
  Block blocka_shsz = Block("A", "沪深");
  Block blockzxb = Block("A", "中小板");
  Block blockg = Block("G", "创业板");
  Block blockstart = Block("START", "科创板");
  Block blocketf = Block("ETF", "ALL");

  std::shared_lock<std::shared_mutex> lock(*stock_dict_mutex_);
  auto iter = stock_dict_.begin();
  for (; iter != stock_dict_.end(); ++iter) {
    const Stock& stock = iter->second;
    if (stock.type() == STOCKTYPE_A) {
      blocka.add(stock);
      blocka_shsz.add(stock);
      if (stock.market() == "SH") {
        blocksh.add(stock);
      } else if (stock.market() == "SZ") {
        blocksz.add(stock);
        if (stock.code().size() >= 3 && stock.code().substr(0, 3) == "002") {
          blockzxb.add(stock);
        }
      }
    } else if (stock.type() == STOCKTYPE_A_BJ) {
      blocka.add(stock);
      blockbj.add(stock);
    } else if (stock.type() == STOCKTYPE_GEM) {
      blockg.add(stock);
    } else if (stock.type() == STOCKTYPE_START) {
      blockstart.add(stock);
    } else if (stock.type() == STOCKTYPE_ETF) {
      blocketf.add(stock);
    }
  }

  iter = stock_dict_.find("SH000001");
  if (iter != stock_dict_.end()) {
    blocka.setIndexStock(iter->second);
    blocka_shsz.setIndexStock(iter->second);
    blocksh.setIndexStock(iter->second);
  }
  iter = stock_dict_.find("SZ399001");
  if (iter != stock_dict_.end()) {
    blocksz.setIndexStock(iter->second);
  }
  iter = stock_dict_.find("BJ899050");
  if (iter != stock_dict_.end()) {
    blockbj.setIndexStock(iter->second);
  }
  iter = stock_dict_.find("SZ399005");
  if (iter != stock_dict_.end()) {
    blockzxb.setIndexStock(iter->second);
  }
  iter = stock_dict_.find("SZ399006");
  if (iter != stock_dict_.end()) {
    blockg.setIndexStock(iter->second);
  }
  iter = stock_dict_.find("SH000688");
  if (iter != stock_dict_.end()) {
    blockstart.setIndexStock(iter->second);
  }

  inner_blocks_.clear();
  inner_blocks_["A_ALL"] = std::move(blocka);
  inner_blocks_["A_沪深"] = std::move(blocka_shsz);
  inner_blocks_["A_SH"] = std::move(blocksh);
  inner_blocks_["A_SZ"] = std::move(blocksz);
  inner_blocks_["A_BJ"] = std::move(blockbj);
  inner_blocks_["A_中小板"] = std::move(blockzxb);
  inner_blocks_["G_创业板"] = std::move(blockg);
  inner_blocks_["START_科创板"] = std::move(blockstart);
  inner_blocks_["ETF_ALL"] = std::move(blocketf);
}

void DataRuntime::loadAllStockWeights() {
  HAYAKU_IF_RETURN(!hayaku_param_.tryGet<bool>("load_stock_weight", true),
                   void());
  // The client mode also materializes all the ex-rights/ex-dividend data at the
  // startup according to the config above: the shared memory snapshot has been
  // mapped at once by the connection after waitReady
  // (IpcConnector::mapSessionShm) and IpcBaseInfoDriver reads all the
  // ex-rights/ex-dividend data from the snapshot (when the snapshot does not
  // cover it, the local driver is read directly, sharing the same data source
  // with the main process); after that Stock::getWeight hits the local cache
  // directly, satisfying the high frequency reading of the
  // ex-rights/ex-dividend data; the securities not materialized (the config
  // off, added by addStock or newly constructed) are still handled by the
  // on-demand lazy loading fallback of Stock::getWeight.
  HAYAKU_INFO(htr("Loading stock weight..."));
  if (context_.isAll()) {
    auto all_stkweight_dict = base_info_driver_->getAllStockWeightList();
    for (auto& item : all_stkweight_dict) {
      item.second.shrink_to_fit();
    }
    std::shared_lock<std::shared_mutex> lock1(*stock_dict_mutex_);
    for (auto iter = stock_dict_.begin(); iter != stock_dict_.end(); ++iter) {
      auto weight_iter = all_stkweight_dict.find(iter->first);
      Stock& stock = iter->second;
      {
        std::unique_lock<std::shared_mutex> lock2(stock.data_->weight_mutex_);
        if (weight_iter != all_stkweight_dict.end()) {
          stock.data_->weight_list_.swap(weight_iter->second);
        }
        // It is marked materialized whether the security has the
        // ex-rights/ex-dividend data or not: not being collected means this
        // security has none (such as most ETFs), avoiding the client mode
        // getWeight repeatedly triggering an empty lazy loading query for the
        // securities without the ex-rights/ex-dividend data
        stock.data_->weight_ready_.store(true, std::memory_order_release);
      }
    }
  } else {
    std::shared_lock<std::shared_mutex> lock1(*stock_dict_mutex_);
    for (auto iter = stock_dict_.begin(); iter != stock_dict_.end(); ++iter) {
      Stock& stock = iter->second;
      auto sw_list = base_info_driver_->getStockWeightList(
          stock.market(), stock.code(), context_.startDatetime(),
          Null<Datetime>());
      sw_list.shrink_to_fit();
      {
        std::unique_lock<std::shared_mutex> lock2(stock.data_->weight_mutex_);
        stock.data_->weight_list_ = std::move(sw_list);
        stock.data_->weight_ready_.store(true, std::memory_order_release);
      }
    }
  }
}

void DataRuntime::releaseShmServerBaseInfoCache() {
  // The app assembly enables this only for the process that publishes a
  // complete base-info snapshot. Ordinary standalone and client runtimes keep
  // their materialized cache.
  HAYAKU_IF_RETURN(!isBaseInfoCacheEvictionEnabled() || isIpcClientMode(),
                   void());
  HAYAKU_DEBUG(
      htr("Release stock weight/finance cache after shm base info published"));
  std::shared_lock<std::shared_mutex> lock1(*stock_dict_mutex_);
  for (auto iter = stock_dict_.begin(); iter != stock_dict_.end(); ++iter) {
    Stock& stock = iter->second;
    {
      std::unique_lock<std::shared_mutex> lock2(stock.data_->weight_mutex_);
      StockWeightList().swap(stock.data_->weight_list_);
      // Set it to false: the next Stock::getWeight re-reads it through the
      // driver lazy loading (the server role has the lazy loading fallback)
      stock.data_->weight_ready_.store(false, std::memory_order_release);
    }
    {
      std::unique_lock<std::shared_mutex> lock2(
          stock.data_->history_finance_mutex_);
      vector<HistoryFinanceInfo>().swap(stock.data_->history_finance_);
      // Set it to false: the next Stock::getHistoryFinance re-reads it through
      // the driver lazy loading (every mode has the fallback)
      stock.data_->history_finance_ready_ = false;
    }
  }
}

void DataRuntime::loadAllZhBond10() {
  zh_bond10_ = base_info_driver_->getAllZhBond10();
  zh_bond10_.shrink_to_fit();
}

void DataRuntime::loadHistoryFinanceField() {
  auto fields = base_info_driver_->getHistoryFinanceField();
  for (const auto& field : fields) {
    field_ix_to_name_[field.first - 1] = field.second;
    field_name_to_ix_[field.second] = field.first - 1;
  }
}

vector<std::pair<size_t, string>> DataRuntime::getHistoryFinanceAllFields()
    const {
  vector<std::pair<size_t, string>> ret;
  for (auto iter = field_ix_to_name_.begin(); iter != field_ix_to_name_.end();
       ++iter) {
    ret.emplace_back(iter->first, iter->second);
  }
  std::sort(
      ret.begin(), ret.end(),
      [](const std::pair<size_t, string>& a,
         const std::pair<size_t, string>& b) { return a.first < b.first; });
  return ret;
}

void DataRuntime::waitDataReady() const {
  HAYAKU_INFO_IF(!dataReady(),
                 htr("Waiting for preload data loading to complete..."));
  while (!dataReady()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

}  // namespace hayaku
