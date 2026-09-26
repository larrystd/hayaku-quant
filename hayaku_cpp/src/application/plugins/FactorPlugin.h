#pragma once

/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-21
 *      Author: fasiondog
 */

#include "operators/FactorStore.h"

namespace hayaku {

/** Enable the ClickHouse-backed factor store bridge for the active Session
 * configuration. */
void installFactorStorePluginBridge(bool enabled);
void uninstallFactorStorePluginBridge() noexcept;

}  // namespace hayaku
