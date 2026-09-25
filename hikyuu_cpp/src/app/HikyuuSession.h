/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Explicit process-level Hikyuu runtime session.
 */

#pragma once
#ifndef HIKYUU_APPLICATION_HIKYUUSESSION_H
#define HIKYUU_APPLICATION_HIKYUUSESSION_H

#include <atomic>
#include <memory>

#include "data/DataEngine.h"
#include "execution/ExecutionEngine.h"
#include "strategy/engine/StrategyEngine.h"
#include "SessionOptions.h"

namespace hku {

class HKU_API HikyuuSession {
public:
    HikyuuSession(const HikyuuSession&) = delete;
    HikyuuSession& operator=(const HikyuuSession&) = delete;
    HikyuuSession(HikyuuSession&&) noexcept = default;
    HikyuuSession& operator=(HikyuuSession&&) noexcept = delete;
    ~HikyuuSession();

    [[nodiscard]] static HikyuuSession open(const SessionOptions& options);
    /** Open a session with its native execution account already installed. */
    [[nodiscard]] static HikyuuSession open(const SessionOptions& options,
                                            const AccountConfig& accountConfig);
    [[nodiscard]] static HikyuuSession open(
      const string& filename, bool ignorePreload = false,
      const StrategyContext& context = StrategyContext({"all"}));

    /** Close this handle and release the data runtime after the final session closes. */
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
    HikyuuSession();
    void _installExecution(const AccountConfig& accountConfig);

private:
    std::shared_ptr<std::atomic_bool> m_active;
    DataEngine m_data;
    std::unique_ptr<ExecutionEngine> m_execution;
    std::unique_ptr<StrategyEngine> m_strategy;
};

}  // namespace hku

#endif /* HIKYUU_APPLICATION_HIKYUUSESSION_H */
