/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "ExecutionAccountFactory.h"

#include "ExecutionRuntime.h"

namespace hayaku::internal {

ExecutionAccountPortPtr makeExecutionAccount(const AccountConfig& config) {
  return std::make_shared<ExecutionRuntime>(config);
}

}  // namespace hayaku::internal
