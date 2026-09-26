/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "StrategyExecutionPort.h"

namespace hayaku::internal {

StrategyExecutionPort::StrategyExecutionPort(ExecutionAccountPortPtr account)
: m_account(std::move(account)) {
    HAYAKU_CHECK(m_account, "StrategyExecutionPort requires an execution account");
}

ExecutionReport StrategyExecutionPort::submit(const OrderRequest& request) {
    return m_account->submit(request);
}

Datetime StrategyExecutionPort::initDatetime() const {
    return m_account->initDatetime();
}

Datetime StrategyExecutionPort::lastDatetime() const {
    return m_account->lastDatetime();
}

PositionRecord StrategyExecutionPort::position(const Datetime& datetime, const Stock& stock) const {
    return m_account->getPosition(datetime, stock);
}

FundsRecord StrategyExecutionPort::funds(const Datetime& datetime,
                                         const KQuery::KType& ktype) const {
    return m_account->getFunds(datetime, ktype);
}

}  // namespace hayaku::internal
