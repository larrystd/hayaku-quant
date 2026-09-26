/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "StrategyExecutionPort.h"

namespace hayaku::internal {

StrategyExecutionPort::StrategyExecutionPort(ExecutionAccountPortPtr account)
    : account_(std::move(account)) {
  HAYAKU_CHECK(account_,
               "StrategyExecutionPort requires an execution account");
}

ExecutionReport StrategyExecutionPort::submit(const OrderRequest& request) {
  return account_->submit(request);
}

Datetime StrategyExecutionPort::initDatetime() const {
  return account_->initDatetime();
}

Datetime StrategyExecutionPort::lastDatetime() const {
  return account_->lastDatetime();
}

PositionRecord StrategyExecutionPort::position(const Datetime& datetime,
                                               const Stock& stock) const {
  return account_->getPosition(datetime, stock);
}

FundsRecord StrategyExecutionPort::funds(const Datetime& datetime,
                                         const KQuery::KType& ktype) const {
  return account_->getFunds(datetime, ktype);
}

}  // namespace hayaku::internal
