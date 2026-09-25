/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "ExecutionEngine.h"
#include "internal/ExecutionRuntime.h"

namespace hku {

ExecutionEngine::ExecutionEngine(const AccountConfig& config)
: m_runtime(std::make_shared<ExecutionRuntime>(config)) {}

ExecutionEngine::ExecutionEngine(std::shared_ptr<ExecutionRuntime> runtime)
: m_runtime(std::move(runtime)) {
    HKU_CHECK(m_runtime, "ExecutionEngine requires a non-null ExecutionRuntime");
}

ExecutionEngine::~ExecutionEngine() = default;

ExecutionEngine::ExecutionEngine(ExecutionEngine&&) noexcept = default;

ExecutionEngine& ExecutionEngine::operator=(ExecutionEngine&&) noexcept = default;

void ExecutionEngine::_attachSession(const std::shared_ptr<std::atomic_bool>& active) noexcept {
    m_sessionActive = active;
    m_sessionBound = true;
}

void ExecutionEngine::_ensureActive() const {
    if (m_sessionBound) {
        const auto active = m_sessionActive.lock();
        HKU_CHECK(active && active->load(std::memory_order_acquire),
                  "ExecutionEngine belongs to a closed HikyuuSession");
    }
}

bool ExecutionEngine::_isBoundTo(const std::shared_ptr<ExecutionRuntime>& runtime) const noexcept {
    return m_runtime && m_runtime == runtime;
}

std::shared_ptr<internal::ExecutionAccountPort> ExecutionEngine::_accountPort() const noexcept {
    return m_runtime;
}

ExecutionReport ExecutionEngine::submit(const OrderRequest& request) {
    _ensureActive();
    HKU_CHECK(m_runtime, "ExecutionEngine was moved from");
    return m_runtime->submit(request);
}

AccountSnapshot ExecutionEngine::snapshot() const {
    _ensureActive();
    HKU_CHECK(m_runtime, "ExecutionEngine was moved from");
    return AccountSnapshot(m_runtime->getFunds(), m_runtime->getPositionList(),
                           m_runtime->getShortPositionList());
}

AccountView ExecutionEngine::view() const {
    _ensureActive();
    HKU_CHECK(m_runtime, "ExecutionEngine was moved from");
    return m_runtime->view();
}

TradeRecordList ExecutionEngine::history() const {
    _ensureActive();
    HKU_CHECK(m_runtime, "ExecutionEngine was moved from");
    return m_runtime->getTradeList();
}

AccountId ExecutionEngine::accountId() const noexcept {
    return m_runtime ? m_runtime->accountId() : AccountId{};
}

}  // namespace hku
