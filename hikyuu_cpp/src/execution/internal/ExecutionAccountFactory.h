/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Internal account construction boundary for strategy runtimes.
 */

#pragma once
#ifndef HIKYUU_TRADE_INTERNAL_EXECUTIONACCOUNTFACTORY_H
#define HIKYUU_TRADE_INTERNAL_EXECUTIONACCOUNTFACTORY_H

#include "../AccountConfig.h"
#include "ExecutionAccountPort.h"

namespace hku::internal {

/** Create an isolated execution account without exposing ExecutionRuntime to strategy code. */
HKU_API ExecutionAccountPortPtr makeExecutionAccount(const AccountConfig& config);

}  // namespace hku::internal

#endif /* HIKYUU_TRADE_INTERNAL_EXECUTIONACCOUNTFACTORY_H */
