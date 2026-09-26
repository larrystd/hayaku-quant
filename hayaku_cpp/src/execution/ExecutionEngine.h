#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Stable facade for order execution and account inspection.
 */


#include <atomic>
#include <memory>

#include "AccountConfig.h"
#include "AccountSnapshot.h"
#include "AccountView.h"
#include "ExecutionReport.h"
#include "OrderRequest.h"

namespace hayaku {

class ExecutionRuntime;
class HayakuSession;
class StrategyEngine;
namespace internal {
class ExecutionAccountPort;
}

/**
 * Synchronous execution facade owning an explicitly configured runtime.
 *
 * The engine deliberately has no default constructor: account creation and ownership policy belong
 * to the application/session boundary.
 */
class HAYAKU_API ExecutionEngine {
public:
    explicit ExecutionEngine(const AccountConfig& config);
    ~ExecutionEngine();

    ExecutionEngine(const ExecutionEngine&) = delete;
    ExecutionEngine& operator=(const ExecutionEngine&) = delete;
    ExecutionEngine(ExecutionEngine&&) noexcept;
    ExecutionEngine& operator=(ExecutionEngine&&) noexcept;

    [[nodiscard]] ExecutionReport submit(const OrderRequest& request);
    [[nodiscard]] AccountSnapshot snapshot() const;
    [[nodiscard]] AccountView view() const;
    [[nodiscard]] TradeRecordList history() const;
    [[nodiscard]] AccountId accountId() const noexcept;

private:
    friend class HayakuSession;
    friend class StrategyEngine;

    explicit ExecutionEngine(std::shared_ptr<ExecutionRuntime> runtime);

    void _attachSession(const std::shared_ptr<std::atomic_bool>& active) noexcept;
    void _ensureActive() const;
    [[nodiscard]] bool _isBoundTo(const std::shared_ptr<ExecutionRuntime>& runtime) const noexcept;
    [[nodiscard]] std::shared_ptr<internal::ExecutionAccountPort> _accountPort() const noexcept;

private:
    std::shared_ptr<ExecutionRuntime> m_runtime;
    std::weak_ptr<std::atomic_bool> m_sessionActive;
    bool m_sessionBound{false};
};

}  // namespace hayaku
