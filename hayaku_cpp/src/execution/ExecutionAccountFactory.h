#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Internal account construction boundary for strategy runtimes.
 */


#include "AccountConfig.h"
#include "ExecutionAccountPort.h"

namespace hayaku::internal {

/** Create an isolated execution account without exposing ExecutionRuntime to strategy code. */
HAYAKU_API ExecutionAccountPortPtr makeExecutionAccount(const AccountConfig& config);

}  // namespace hayaku::internal
