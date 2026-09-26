/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "ExecutionEngine.h"

#include "ExecutionRuntime.h"

namespace hayaku {

ExecutionEngine::ExecutionEngine(const AccountConfig& config)
    : runtime_(std::make_shared<ExecutionRuntime>(config)) {}

ExecutionEngine::ExecutionEngine(std::shared_ptr<ExecutionRuntime> runtime)
    : runtime_(std::move(runtime)) {
  HAYAKU_CHECK(runtime_,
               "ExecutionEngine requires a non-null ExecutionRuntime");
}

ExecutionEngine::~ExecutionEngine() = default;

ExecutionEngine::ExecutionEngine(ExecutionEngine&&) noexcept = default;

ExecutionEngine& ExecutionEngine::operator=(ExecutionEngine&&) noexcept =
    default;

void ExecutionEngine::_attachSession(
    const std::shared_ptr<std::atomic_bool>& active) noexcept {
  session_active_ = active;
  session_bound_ = true;
}

void ExecutionEngine::_ensureActive() const {
  if (session_bound_) {
    const auto active = session_active_.lock();
    HAYAKU_CHECK(active && active->load(std::memory_order_acquire),
                 "ExecutionEngine belongs to a closed HayakuSession");
  }
}

bool ExecutionEngine::_isBoundTo(
    const std::shared_ptr<ExecutionRuntime>& runtime) const noexcept {
  return runtime_ && runtime_ == runtime;
}

std::shared_ptr<internal::ExecutionAccountPort> ExecutionEngine::_accountPort()
    const noexcept {
  return runtime_;
}

ExecutionReport ExecutionEngine::submit(const OrderRequest& request) {
  _ensureActive();
  HAYAKU_CHECK(runtime_, "ExecutionEngine was moved from");
  return runtime_->submit(request);
}

AccountSnapshot ExecutionEngine::snapshot() const {
  _ensureActive();
  HAYAKU_CHECK(runtime_, "ExecutionEngine was moved from");
  return AccountSnapshot(runtime_->getFunds(), runtime_->getPositionList(),
                         runtime_->getShortPositionList());
}

AccountView ExecutionEngine::view() const {
  _ensureActive();
  HAYAKU_CHECK(runtime_, "ExecutionEngine was moved from");
  return runtime_->view();
}

TradeRecordList ExecutionEngine::history() const {
  _ensureActive();
  HAYAKU_CHECK(runtime_, "ExecutionEngine was moved from");
  return runtime_->getTradeList();
}

AccountId ExecutionEngine::accountId() const noexcept {
  return runtime_ ? runtime_->accountId() : AccountId{};
}

}  // namespace hayaku
