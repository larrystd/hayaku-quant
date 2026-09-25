/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Narrow execution seam used by StrategyRuntime.
 */

#pragma once
#ifndef HIKYUU_TRADE_SYS_ENGINE_INTERNAL_STRATEGYEXECUTIONPORT_H
#define HIKYUU_TRADE_SYS_ENGINE_INTERNAL_STRATEGYEXECUTIONPORT_H

#include "execution/internal/ExecutionAccountPort.h"

namespace hku::internal {

class StrategyExecutionPort {
public:
    explicit StrategyExecutionPort(ExecutionAccountPortPtr account);

    [[nodiscard]] ExecutionReport submit(const OrderRequest& request);
    [[nodiscard]] Datetime initDatetime() const;
    [[nodiscard]] Datetime lastDatetime() const;
    [[nodiscard]] PositionRecord position(const Datetime& datetime, const Stock& stock) const;
    [[nodiscard]] FundsRecord funds(const Datetime& datetime, const KQuery::KType& ktype) const;

private:
    ExecutionAccountPortPtr m_account;
};

}  // namespace hku::internal

#endif /* HIKYUU_TRADE_SYS_ENGINE_INTERNAL_STRATEGYEXECUTIONPORT_H */
