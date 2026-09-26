#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Explicit process-runtime lifecycle. The historical file name is kept
 * temporarily to avoid a path-only refactor; there is deliberately no
 * header-level static initializer.
 */

namespace hayaku {

/** Acquire the core process runtime. The first acquisition performs
 * initialization. */
void acquireProcessRuntime();

/** Release one acquisition. The final release shuts down the core process
 * runtime. */
void releaseProcessRuntime() noexcept;

[[nodiscard]] bool processRuntimeActive() noexcept;

}  // namespace hayaku
