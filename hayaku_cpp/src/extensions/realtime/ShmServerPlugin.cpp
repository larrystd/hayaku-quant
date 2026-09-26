/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-06
 *      Author: fasiondog
 */

#include "ShmServerPlugin.h"

#include <atomic>

#include "application/DataRuntimeAssembly.h"
#include "application/PluginRuntime.h"
#include "application/plugins/PluginIds.h"
#include "application/plugins/ShmServerPluginInterface.h"
#include "data/DataRuntime.h"

namespace hayaku {

namespace {
// The facade holds the service plugin pointer of "this process has started":
// stop / isRunning only query this pointer and never go through the on-demand
// loading of getPlugin. Reason: stopShmServer() is called on final Session
// close; going through getPlugin would repeatedly try dlopen on an ordinary
// standalone process without an installed / started service and pollute the
// close path.
std::atomic<ShmServerPluginInterface*> g_shm_server_plugin{nullptr};
}  // namespace

bool startShmServer(const std::string& datadir, bool publish_shm,
                    bool recv_spot) noexcept {
  // Mark the app-side server role so future data reloads cannot connect this
  // process back to its own service.
  setShmServerRole(true);
  if (auto* runtime = getDataRuntimeIfExists()) {
    runtime->setBaseInfoCacheEvictionEnabled(true);
  }

  // Idempotent: a success is returned directly when the service has been
  // started in this process
  HAYAKU_INFO_IF_RETURN(
      g_shm_server_plugin.load(std::memory_order_acquire) != nullptr, true,
      "hayaku shm server is already running.");

  ShmServerPluginInterface* plugin =
      getPlugin<ShmServerPluginInterface>(HAYAKU_PLUGIN_SHM_SERVER);
  if (!plugin) {
    setShmServerRole(false);
    if (auto* runtime = getDataRuntimeIfExists()) {
      runtime->setBaseInfoCacheEvictionEnabled(false);
    }
    HAYAKU_ERROR(
        "Can't find {} plugin! It is an optional realtime plugin; check that "
        "it is "
        "installed and properly licensed.",
        HAYAKU_PLUGIN_SHM_SERVER);
    return false;
  }

  // The plugin start() is responsible internally for: the authorization check,
  // the self-connection guard (rejected when isIpcClientMode is true),
  // registering the ShmMirrorSink and the LoadEvent callbacks, startSpotAgent
  // (recv_spot) and the first publish
  if (!plugin->start(datadir, publish_shm, recv_spot)) {
    setShmServerRole(false);
    if (auto* runtime = getDataRuntimeIfExists()) {
      runtime->setBaseInfoCacheEvictionEnabled(false);
    }
    HAYAKU_ERROR("Failed to start hayaku shm server plugin!");
    return false;
  }

  g_shm_server_plugin.store(plugin, std::memory_order_release);
  return true;
}

void stopShmServer() noexcept {
  // Take and clear the pointer: the exchange guarantees the idempotence of stop
  // and isRunning immediately reports not running afterwards
  ShmServerPluginInterface* plugin =
      g_shm_server_plugin.exchange(nullptr, std::memory_order_acq_rel);
  if (!plugin) {
    setShmServerRole(false);
    if (auto* runtime = getDataRuntimeIfExists()) {
      runtime->setBaseInfoCacheEvictionEnabled(false);
    }
    return;
  }
  // The plugin stop() does internally, in order: stopping the market data,
  // unregistering the ShmMirrorSink, unregistering the LoadEvent callbacks,
  // ShmServer::stop(true) and destroying the publisher
  plugin->stop();
  setShmServerRole(false);
  if (auto* runtime = getDataRuntimeIfExists()) {
    runtime->setBaseInfoCacheEvictionEnabled(false);
  }
}

bool isShmServerRunning() noexcept {
  ShmServerPluginInterface* plugin =
      g_shm_server_plugin.load(std::memory_order_acquire);
  return plugin ? plugin->running() : false;
}

}  // namespace hayaku
