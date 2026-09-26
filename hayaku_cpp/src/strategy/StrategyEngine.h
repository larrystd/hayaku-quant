#pragma once

/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-25
 *      Author: hayaku
 */

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

#include "BacktestRequest.h"
#include "BacktestResult.h"
#include "StrategyDefinition.h"

namespace hayaku {

class HayakuSession;
class ExecutionEngine;
namespace internal {
class StrategyRuntime;
}

/**
 * Narrow strategy orchestration facade.
 *
 * Construction accepts an explicit component definition and a narrow execution
 * boundary. The facade does not expose the mutable strategy runtime or account
 * implementation.
 */
class HAYAKU_API StrategyEngine {
 public:
  StrategyEngine(StrategyDefinition definition, ExecutionEngine& execution);
  ~StrategyEngine();

  StrategyEngine(const StrategyEngine&) = delete;
  StrategyEngine& operator=(const StrategyEngine&) = delete;
  StrategyEngine(StrategyEngine&&) = delete;
  StrategyEngine& operator=(StrategyEngine&&) = delete;

  /** Run the bound definition synchronously and return a stable result
   * snapshot. */
  [[nodiscard]] BacktestResult run(const BacktestRequest& request);

  /** Request cooperative cancellation of the current run. */
  void stop() noexcept;

  /** Return true while run is active. */
  [[nodiscard]] bool running() const noexcept;

 private:
  friend class HayakuSession;

  void _attachSession(const std::shared_ptr<std::atomic_bool>& active) noexcept;
  void _ensureActive() const;
  void _stopAndWait() noexcept;
  [[nodiscard]] bool _matches(const StrategyDefinition& definition) const;

 private:
  StrategyDefinition m_definition;
  internal::ExecutionAccountPortPtr m_account;
  std::unique_ptr<internal::StrategyRuntime> m_runtime;
  mutable std::mutex m_stateMutex;
  std::condition_variable m_stateChanged;
  std::atomic_bool m_running{false};
  std::atomic_bool m_stopRequested{false};
  std::weak_ptr<std::atomic_bool> m_sessionActive;
  bool m_sessionBound{false};
};

}  // namespace hayaku
