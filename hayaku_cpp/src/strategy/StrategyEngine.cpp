/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-25
 *      Author: hayaku
 */

#include "StrategyEngine.h"

#include <utility>

#include "execution/ExecutionEngine.h"
#include "StrategyRuntime.h"

namespace hayaku {

StrategyEngine::StrategyEngine(StrategyDefinition definition, ExecutionEngine& execution)
: m_definition(std::move(definition)),
  m_account(execution._accountPort()),
  m_runtime(std::make_unique<internal::StrategyRuntime>(m_definition, m_account)) {}

StrategyEngine::~StrategyEngine() {
    _stopAndWait();
}

void StrategyEngine::_attachSession(const std::shared_ptr<std::atomic_bool>& active) noexcept {
    m_sessionActive = active;
    m_sessionBound = true;
}

void StrategyEngine::_ensureActive() const {
    if (m_sessionBound) {
        const auto active = m_sessionActive.lock();
        HAYAKU_CHECK(active && active->load(std::memory_order_acquire),
                  "StrategyEngine belongs to a closed HayakuSession");
    }
}

bool StrategyEngine::_matches(const StrategyDefinition& definition) const {
    return m_definition == definition;
}

BacktestResult StrategyEngine::run(const BacktestRequest& request) {
    _ensureActive();
    {
        std::lock_guard lock(m_stateMutex);
        _ensureActive();
        HAYAKU_CHECK(!m_running.load(std::memory_order_relaxed), "StrategyEngine is already running");
        m_stopRequested.store(false, std::memory_order_relaxed);
        m_running.store(true, std::memory_order_release);
    }

    m_runtime->setStopToken(&m_stopRequested);
    try {
        m_runtime->run(request);
        BacktestResult result(request.kdata().getStock(), request.kdata().getQuery(),
                              m_account->getTradeList());
        m_runtime->setStopToken(nullptr);
        {
            std::lock_guard lock(m_stateMutex);
            m_running.store(false, std::memory_order_release);
        }
        m_stateChanged.notify_all();
        return result;

    } catch (...) {
        m_runtime->setStopToken(nullptr);
        {
            std::lock_guard lock(m_stateMutex);
            m_running.store(false, std::memory_order_release);
        }
        m_stateChanged.notify_all();
        throw;
    }
}

void StrategyEngine::stop() noexcept {
    std::lock_guard lock(m_stateMutex);
    if (m_running.load(std::memory_order_relaxed)) {
        m_stopRequested.store(true, std::memory_order_release);
    }
}

bool StrategyEngine::running() const noexcept {
    return m_running.load(std::memory_order_acquire);
}

void StrategyEngine::_stopAndWait() noexcept {
    std::unique_lock lock(m_stateMutex);
    if (m_running.load(std::memory_order_relaxed)) {
        m_stopRequested.store(true, std::memory_order_release);
        m_stateChanged.wait(lock, [this] { return !m_running.load(std::memory_order_acquire); });
    }
}

}  // namespace hayaku
