#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Application-side assembly of optional data capabilities.
 */


#include "SessionOptions.h"

namespace hayaku {

/** Prepare optional adapters before DataRuntime::init. Called for the first active Session only. */
void prepareDataRuntimeAssembly(const SessionOptions& options);

/** Stop bridges that can call back into a released DataRuntime. */
void releaseDataRuntimeAssembly() noexcept;

/** Mark whether this process is being prepared as the realtime service host. */
void setShmServerRole(bool role) noexcept;
[[nodiscard]] bool isShmServerRole() noexcept;

}  // namespace hayaku
