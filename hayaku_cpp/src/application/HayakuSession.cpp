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
    : m_active(std::make_shared<std::atomic_bool>(false)), m_data(m_active) {}

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
    session.m_data._attach(runtime);
    ++g_openSessionCount;
    session.m_active->store(true, std::memory_order_release);
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
  if (!m_active || !m_active->load(std::memory_order_acquire)) {
    return;
  }
  if (detail::spotAgentCallbackActive()) {
    HAYAKU_ERROR(
        "HayakuSession::close must be called outside a SpotAgent callback");
    return;
  }
  if (!m_active->exchange(false, std::memory_order_acq_rel)) {
    return;
  }

  if (m_strategy) {
    m_strategy->_stopAndWait();
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
  return m_active && m_active->load(std::memory_order_acquire);
}

bool HayakuSession::ready() const { return data().ready(); }

void HayakuSession::waitReady() const { data().waitReady(); }

DataEngine& HayakuSession::data() {
  HAYAKU_CHECK(isOpen(), "HayakuSession is closed");
  return m_data;
}

const DataEngine& HayakuSession::data() const {
  HAYAKU_CHECK(isOpen(), "HayakuSession is closed");
  return m_data;
}

void HayakuSession::_installExecution(const AccountConfig& accountConfig) {
  HAYAKU_CHECK(isOpen(), "HayakuSession is closed");
  HAYAKU_CHECK(!m_execution, "HayakuSession already has an ExecutionEngine");
  auto engine = std::make_unique<ExecutionEngine>(accountConfig);
  engine->_attachSession(m_active);
  m_execution = std::move(engine);
}

StrategyEngine& HayakuSession::bindStrategy(
    const StrategyDefinition& definition) {
  HAYAKU_CHECK(isOpen(), "HayakuSession is closed");
  HAYAKU_CHECK(
      m_execution,
      "HayakuSession requires an ExecutionEngine before binding a strategy");
  if (m_strategy) {
    HAYAKU_CHECK(m_strategy->_matches(definition),
                 "HayakuSession already has a different StrategyDefinition");
    return *m_strategy;
  }

  auto strategy = std::make_unique<StrategyEngine>(definition, *m_execution);
  strategy->_attachSession(m_active);
  m_strategy = std::move(strategy);
  return *m_strategy;
}

bool HayakuSession::hasExecution() const noexcept {
  return isOpen() && static_cast<bool>(m_execution);
}

bool HayakuSession::hasStrategy() const noexcept {
  return isOpen() && static_cast<bool>(m_strategy);
}

ExecutionEngine& HayakuSession::execution() {
  HAYAKU_CHECK(isOpen(), "HayakuSession is closed");
  HAYAKU_CHECK(m_execution, "HayakuSession has no ExecutionEngine");
  return *m_execution;
}

const ExecutionEngine& HayakuSession::execution() const {
  HAYAKU_CHECK(isOpen(), "HayakuSession is closed");
  HAYAKU_CHECK(m_execution, "HayakuSession has no ExecutionEngine");
  return *m_execution;
}

StrategyEngine& HayakuSession::strategy() {
  HAYAKU_CHECK(isOpen(), "HayakuSession is closed");
  HAYAKU_CHECK(m_strategy, "HayakuSession has no StrategyEngine");
  return *m_strategy;
}

const StrategyEngine& HayakuSession::strategy() const {
  HAYAKU_CHECK(isOpen(), "HayakuSession is closed");
  HAYAKU_CHECK(m_strategy, "HayakuSession has no StrategyEngine");
  return *m_strategy;
}

}  // namespace hayaku
