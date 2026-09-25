/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Internal account capabilities used by portfolio capital allocation.
 */

#pragma once
#ifndef HIKYUU_TRADE_INTERNAL_PORTFOLIOACCOUNTPORT_H
#define HIKYUU_TRADE_INTERNAL_PORTFOLIOACCOUNTPORT_H

#include "ExecutionAccountPort.h"

namespace hku::internal {

class PortfolioAccountPort;
using PortfolioAccountPortPtr = std::shared_ptr<PortfolioAccountPort>;

/** Portfolio-only account operations, kept out of the strategy hot-path contract. */
class PortfolioAccountPort : public ExecutionAccountPort {
public:
    ~PortfolioAccountPort() override = default;

    [[nodiscard]] virtual PortfolioAccountPortPtr cloneAccount() const = 0;
    [[nodiscard]] virtual PortfolioAccountPortPtr createChildAccount(
      string name, price_t initialCash = 0.0) const = 0;
    virtual bool checkout(const Datetime& datetime, price_t cash) = 0;
    virtual bool addTradeRecord(const TradeRecord& record) = 0;
};

}  // namespace hku::internal

#endif /* HIKYUU_TRADE_INTERNAL_PORTFOLIOACCOUNTPORT_H */
