#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Narrow execution seam used by StrategyRuntime.
 */


#include "execution/ExecutionAccountPort.h"

namespace hayaku::internal {

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

}  // namespace hayaku::internal
