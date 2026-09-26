#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Explicit process-runtime lifecycle. The historical file name is kept
 * temporarily to avoid a path-only refactor; there is deliberately no
 * header-level static initializer.
 */

#ifndef HAYAKU_API
#define HAYAKU_API
#endif

namespace hayaku {

/** Acquire the core process runtime. The first acquisition performs
 * initialization. */
HAYAKU_API void acquireProcessRuntime();

/** Release one acquisition. The final release shuts down the core process
 * runtime. */
HAYAKU_API void releaseProcessRuntime() noexcept;

[[nodiscard]] HAYAKU_API bool processRuntimeActive() noexcept;

}  // namespace hayaku
