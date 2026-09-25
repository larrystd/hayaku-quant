/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <mutex>

#include "data/internal/DataRuntime.h"
#include "HikyuuSession.h"

namespace hku {

namespace {

std::mutex g_sessionMutex;
size_t g_openSessionCount = 0;

}  // namespace

HikyuuSession::HikyuuSession()
: m_active(std::make_shared<std::atomic_bool>(false)), m_data(m_active) {}

HikyuuSession::~HikyuuSession() {
    close();
}

HikyuuSession HikyuuSession::open(const SessionOptions& options) {
    HikyuuSession session;
    std::lock_guard<std::mutex> lock(g_sessionMutex);

    auto& runtime = getDataRuntime();
    runtime.init(options.baseInfoParam(), options.blockParam(), options.kdataParam(),
                 options.preloadParam(), options.hikyuuParam(), options.context());
    session.m_data._attach(runtime);
    ++g_openSessionCount;
    session.m_active->store(true, std::memory_order_release);
    return session;
}

HikyuuSession HikyuuSession::open(const SessionOptions& options,
                                  const AccountConfig& accountConfig) {
    HikyuuSession session = open(options);
    session._installExecution(accountConfig);
    return session;
}

HikyuuSession HikyuuSession::open(const string& filename, bool ignorePreload,
                                  const StrategyContext& context) {
    return open(SessionOptions::fromIni(filename, ignorePreload, context));
}

void HikyuuSession::close() noexcept {
    if (!m_active || !m_active->exchange(false, std::memory_order_acq_rel)) {
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
        if (auto* runtime = getDataRuntimeIfExists()) {
            runtime->cancelLoad();
            runtime->joinPreloadThread();
        }
        releaseDataRuntime();
    }
}

bool HikyuuSession::isOpen() const noexcept {
    return m_active && m_active->load(std::memory_order_acquire);
}

bool HikyuuSession::ready() const {
    return data().ready();
}

void HikyuuSession::waitReady() const {
    data().waitReady();
}

DataEngine& HikyuuSession::data() {
    HKU_CHECK(isOpen(), "HikyuuSession is closed");
    return m_data;
}

const DataEngine& HikyuuSession::data() const {
    HKU_CHECK(isOpen(), "HikyuuSession is closed");
    return m_data;
}

void HikyuuSession::_installExecution(const AccountConfig& accountConfig) {
    HKU_CHECK(isOpen(), "HikyuuSession is closed");
    HKU_CHECK(!m_execution, "HikyuuSession already has an ExecutionEngine");
    auto engine = std::make_unique<ExecutionEngine>(accountConfig);
    engine->_attachSession(m_active);
    m_execution = std::move(engine);
}

StrategyEngine& HikyuuSession::bindStrategy(const StrategyDefinition& definition) {
    HKU_CHECK(isOpen(), "HikyuuSession is closed");
    HKU_CHECK(m_execution,
              "HikyuuSession requires an ExecutionEngine before binding a strategy");
    if (m_strategy) {
        HKU_CHECK(m_strategy->_matches(definition),
                  "HikyuuSession already has a different StrategyDefinition");
        return *m_strategy;
    }

    auto strategy = std::make_unique<StrategyEngine>(definition, *m_execution);
    strategy->_attachSession(m_active);
    m_strategy = std::move(strategy);
    return *m_strategy;
}

bool HikyuuSession::hasExecution() const noexcept {
    return isOpen() && static_cast<bool>(m_execution);
}

bool HikyuuSession::hasStrategy() const noexcept {
    return isOpen() && static_cast<bool>(m_strategy);
}

ExecutionEngine& HikyuuSession::execution() {
    HKU_CHECK(isOpen(), "HikyuuSession is closed");
    HKU_CHECK(m_execution, "HikyuuSession has no ExecutionEngine");
    return *m_execution;
}

const ExecutionEngine& HikyuuSession::execution() const {
    HKU_CHECK(isOpen(), "HikyuuSession is closed");
    HKU_CHECK(m_execution, "HikyuuSession has no ExecutionEngine");
    return *m_execution;
}

StrategyEngine& HikyuuSession::strategy() {
    HKU_CHECK(isOpen(), "HikyuuSession is closed");
    HKU_CHECK(m_strategy, "HikyuuSession has no StrategyEngine");
    return *m_strategy;
}

const StrategyEngine& HikyuuSession::strategy() const {
    HKU_CHECK(isOpen(), "HikyuuSession is closed");
    HKU_CHECK(m_strategy, "HikyuuSession has no StrategyEngine");
    return *m_strategy;
}

}  // namespace hku
