/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "HayakuSession.h"

#include <mutex>
#include <optional>

#include "DataRuntimeAssembly.h"
#include "GlobalInitializer.h"
#include "data/DataRuntime.h"
#include "extensions/realtime/RealtimePort.h"
#include "extensions/realtime/ScheduledTasks.h"
#include "extensions/realtime/Scheduler.h"

namespace hayaku {

namespace {

std::mutex g_sessionMutex;
size_t g_openSessionCount = 0;
std::optional<SessionOptions> g_activeSessionOptions;

bool sameSessionOptions(const SessionOptions& lhs, const SessionOptions& rhs) {
  return lhs.baseInfoParam() == rhs.baseInfoParam() &&
         lhs.blockParam() == rhs.blockParam() &&
         lhs.kdataParam() == rhs.kdataParam() &&
         lhs.preloadParam() == rhs.preloadParam() &&
         lhs.hayakuParam() == rhs.hayakuParam() &&
         lhs.context().str() == rhs.context().str();
}

}  // namespace

HayakuSession::HayakuSession()
    : active_(std::make_shared<std::atomic_bool>(false)), data_(active_) {}

HayakuSession::~HayakuSession() { close(); }

HayakuSession HayakuSession::open(const SessionOptions& options) {
  HayakuSession session;
  acquireProcessRuntime();

  std::unique_lock<std::mutex> lock(g_sessionMutex);
  const bool firstSession = g_openSessionCount == 0;
  try {
    auto& runtime = createDataRuntime();
    if (firstSession) {
      prepareDataRuntimeAssembly(options);
      runtime.setBaseInfoCacheEvictionEnabled(isShmServerRole());
      runtime.init(options.baseInfoParam(), options.blockParam(),
                   options.kdataParam(), options.preloadParam(),
                   options.hayakuParam(), options.context());
      initInnerTask();
      g_activeSessionOptions = options;
    } else {
      HAYAKU_CHECK(g_activeSessionOptions &&
                       sameSessionOptions(*g_activeSessionOptions, options),
                   "All concurrently open HayakuSession handles must use "
                   "identical options");
    }
    session.data_._attach(runtime);
    ++g_openSessionCount;
    session.active_->store(true, std::memory_order_release);
  } catch (...) {
    if (firstSession) {
      g_activeSessionOptions.reset();
      shutdownRealtimeRuntime();
      releaseScheduler();
      if (auto* runtime = getDataRuntimeIfExists()) {
        runtime->cancelLoad();
        runtime->joinPreloadThread();
      }
      releaseDataRuntime();
      releaseDataRuntimeAssembly();
    }
    lock.unlock();
    releaseProcessRuntime();
    throw;
  }

  return session;
}

HayakuSession HayakuSession::open(const SessionOptions& options,
                                  const AccountConfig& accountConfig) {
  HayakuSession session = open(options);
  session._installExecution(accountConfig);
  return session;
}

HayakuSession HayakuSession::open(const string& filename, bool ignorePreload,
                                  const StrategyContext& context) {
  return open(SessionOptions::fromIni(filename, ignorePreload, context));
}

void HayakuSession::close() noexcept {
  if (!active_ || !active_->load(std::memory_order_acquire)) {
    return;
  }
  if (detail::spotAgentCallbackActive()) {
    HAYAKU_ERROR(
        "HayakuSession::close must be called outside a SpotAgent callback");
    return;
  }
  if (!active_->exchange(false, std::memory_order_acq_rel)) {
    return;
  }

  if (strategy_) {
    strategy_->_stopAndWait();
  }

  std::lock_guard<std::mutex> lock(g_sessionMutex);
  if (g_openSessionCount > 0) {
    --g_openSessionCount;
  }
  if (g_openSessionCount == 0) {
    shutdownRealtimeRuntime();
    releaseScheduler();
    if (auto* runtime = getDataRuntimeIfExists()) {
      runtime->cancelLoad();
      runtime->joinPreloadThread();
    }
    releaseDataRuntime();
    releaseDataRuntimeAssembly();
    g_activeSessionOptions.reset();
  }
  releaseProcessRuntime();
}

bool HayakuSession::isOpen() const noexcept {
  return active_ && active_->load(std::memory_order_acquire);
}

bool HayakuSession::ready() const { return data().ready(); }

void HayakuSession::waitReady() const { data().waitReady(); }

DataEngine& HayakuSession::data() {
  HAYAKU_CHECK(isOpen(), "HayakuSession is closed");
  return data_;
}

const DataEngine& HayakuSession::data() const {
  HAYAKU_CHECK(isOpen(), "HayakuSession is closed");
  return data_;
}

void HayakuSession::_installExecution(const AccountConfig& accountConfig) {
  HAYAKU_CHECK(isOpen(), "HayakuSession is closed");
  HAYAKU_CHECK(!execution_, "HayakuSession already has an ExecutionEngine");
  auto engine = std::make_unique<ExecutionEngine>(accountConfig);
  engine->_attachSession(active_);
  execution_ = std::move(engine);
}

StrategyEngine& HayakuSession::bindStrategy(
    const StrategyDefinition& definition) {
  HAYAKU_CHECK(isOpen(), "HayakuSession is closed");
  HAYAKU_CHECK(
      execution_,
      "HayakuSession requires an ExecutionEngine before binding a strategy");
  if (strategy_) {
    HAYAKU_CHECK(strategy_->_matches(definition),
                 "HayakuSession already has a different StrategyDefinition");
    return *strategy_;
  }

  auto strategy = std::make_unique<StrategyEngine>(definition, *execution_);
  strategy->_attachSession(active_);
  strategy_ = std::move(strategy);
  return *strategy_;
}

bool HayakuSession::hasExecution() const noexcept {
  return isOpen() && static_cast<bool>(execution_);
}

bool HayakuSession::hasStrategy() const noexcept {
  return isOpen() && static_cast<bool>(strategy_);
}

ExecutionEngine& HayakuSession::execution() {
  HAYAKU_CHECK(isOpen(), "HayakuSession is closed");
  HAYAKU_CHECK(execution_, "HayakuSession has no ExecutionEngine");
  return *execution_;
}

const ExecutionEngine& HayakuSession::execution() const {
  HAYAKU_CHECK(isOpen(), "HayakuSession is closed");
  HAYAKU_CHECK(execution_, "HayakuSession has no ExecutionEngine");
  return *execution_;
}

StrategyEngine& HayakuSession::strategy() {
  HAYAKU_CHECK(isOpen(), "HayakuSession is closed");
  HAYAKU_CHECK(strategy_, "HayakuSession has no StrategyEngine");
  return *strategy_;
}

const StrategyEngine& HayakuSession::strategy() const {
  HAYAKU_CHECK(isOpen(), "HayakuSession is closed");
  HAYAKU_CHECK(strategy_, "HayakuSession has no StrategyEngine");
  return *strategy_;
}

}  // namespace hayaku
