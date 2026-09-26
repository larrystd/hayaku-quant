#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Explicit process-level Hayaku runtime session.
 */


#include <atomic>
#include <memory>

#include "data/DataEngine.h"
#include "execution/ExecutionEngine.h"
#include "strategy/StrategyEngine.h"
#include "SessionOptions.h"

namespace hayaku {

class HAYAKU_API HayakuSession {
public:
    HayakuSession(const HayakuSession&) = delete;
    HayakuSession& operator=(const HayakuSession&) = delete;
    HayakuSession(HayakuSession&&) noexcept = default;
    HayakuSession& operator=(HayakuSession&&) noexcept = delete;
    ~HayakuSession();

    [[nodiscard]] static HayakuSession open(const SessionOptions& options);
    /** Open a session with its native execution account already installed. */
    [[nodiscard]] static HayakuSession open(const SessionOptions& options,
                                            const AccountConfig& accountConfig);
    [[nodiscard]] static HayakuSession open(
      const string& filename, bool ignorePreload = false,
      const StrategyContext& context = StrategyContext({"all"}));

    /**
     * Close this handle and release the data runtime after the final session closes.
     * This synchronous operation cannot run inside a SpotAgent callback: stop the agent
     * there, then close the session from its owning thread after the callback returns.
     */
    void close() noexcept;
    [[nodiscard]] bool isOpen() const noexcept;
    [[nodiscard]] bool ready() const;
    void waitReady() const;

    [[nodiscard]] DataEngine& data();
    [[nodiscard]] const DataEngine& data() const;

    /** Bind a strategy definition once. Its account must match the execution account. */
    [[nodiscard]] StrategyEngine& bindStrategy(const StrategyDefinition& definition);

    [[nodiscard]] bool hasExecution() const noexcept;
    [[nodiscard]] bool hasStrategy() const noexcept;
    [[nodiscard]] ExecutionEngine& execution();
    [[nodiscard]] const ExecutionEngine& execution() const;
    [[nodiscard]] StrategyEngine& strategy();
    [[nodiscard]] const StrategyEngine& strategy() const;

private:
    HayakuSession();
    void _installExecution(const AccountConfig& accountConfig);

private:
    std::shared_ptr<std::atomic_bool> m_active;
    DataEngine m_data;
    std::unique_ptr<ExecutionEngine> m_execution;
    std::unique_ptr<StrategyEngine> m_strategy;
};

}  // namespace hayaku
