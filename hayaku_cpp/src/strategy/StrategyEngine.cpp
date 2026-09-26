/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-25
 *      Author: hayaku
 */

#include "StrategyEngine.h"

#include <utility>

#include "StrategyRuntime.h"
#include "execution/ExecutionEngine.h"

namespace hayaku {

StrategyEngine::StrategyEngine(StrategyDefinition definition,
                               ExecutionEngine& execution)
    : definition_(std::move(definition)),
      account_(execution._accountPort()),
      runtime_(std::make_unique<internal::StrategyRuntime>(definition_,
                                                            account_)) {}

StrategyEngine::~StrategyEngine() { _stopAndWait(); }

void StrategyEngine::_attachSession(
    const std::shared_ptr<std::atomic_bool>& active) noexcept {
  session_active_ = active;
  session_bound_ = true;
}

void StrategyEngine::_ensureActive() const {
  if (session_bound_) {
    const auto active = session_active_.lock();
    HAYAKU_CHECK(active && active->load(std::memory_order_acquire),
                 "StrategyEngine belongs to a closed HayakuSession");
  }
}

bool StrategyEngine::_matches(const StrategyDefinition& definition) const {
  return definition_ == definition;
}

BacktestResult StrategyEngine::run(const BacktestRequest& request) {
  _ensureActive();
  {
    std::lock_guard lock(state_mutex_);
    _ensureActive();
    HAYAKU_CHECK(!running_.load(std::memory_order_relaxed),
                 "StrategyEngine is already running");
    stop_requested_.store(false, std::memory_order_relaxed);
    running_.store(true, std::memory_order_release);
  }

  runtime_->setStopToken(&stop_requested_);
  try {
    runtime_->run(request);
    BacktestResult result(request.kdata().getStock(),
                          request.kdata().getQuery(),
                          account_->getTradeList());
    runtime_->setStopToken(nullptr);
    {
      std::lock_guard lock(state_mutex_);
      running_.store(false, std::memory_order_release);
    }
    state_changed_.notify_all();
    return result;

  } catch (...) {
    runtime_->setStopToken(nullptr);
    {
      std::lock_guard lock(state_mutex_);
      running_.store(false, std::memory_order_release);
    }
    state_changed_.notify_all();
    throw;
  }
}

void StrategyEngine::stop() noexcept {
  std::lock_guard lock(state_mutex_);
  if (running_.load(std::memory_order_relaxed)) {
    stop_requested_.store(true, std::memory_order_release);
  }
}

bool StrategyEngine::running() const noexcept {
  return running_.load(std::memory_order_acquire);
}

void StrategyEngine::_stopAndWait() noexcept {
  std::unique_lock lock(state_mutex_);
  if (running_.load(std::memory_order_relaxed)) {
    stop_requested_.store(true, std::memory_order_release);
    state_changed_.wait(
        lock, [this] { return !running_.load(std::memory_order_acquire); });
  }
}

}  // namespace hayaku
