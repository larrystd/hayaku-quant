/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "FactorPlugin.h"

#include <atomic>

#include "DataDriverPluginInterface.h"
#include "PluginIds.h"
#include "application/PluginRuntime.h"

namespace hayaku {

namespace {
std::atomic_bool g_factorStoreEnabled{false};
}

void installFactorStorePluginBridge(bool enabled) {
  g_factorStoreEnabled.store(enabled, std::memory_order_release);
  setFactorStoreResolver([]() -> FactorStore* {
    if (!g_factorStoreEnabled.load(std::memory_order_acquire)) {
      return nullptr;
    }
    return getPlugin<DataDriverPluginInterface>(HAYAKU_PLUGIN_CLICKHOUSE_DRIVER,
                                                false);
  });
}

void uninstallFactorStorePluginBridge() noexcept {
  g_factorStoreEnabled.store(false, std::memory_order_release);
  setFactorStoreResolver({});
}

}  // namespace hayaku
