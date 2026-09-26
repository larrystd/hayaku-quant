/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "DataRuntimeAssembly.h"

#include <atomic>
#include <boost/algorithm/string.hpp>

#include "PluginRuntime.h"
#include "application/plugins/DataDriverPluginInterface.h"
#include "application/plugins/DevicePlugin.h"
#include "application/plugins/FactorPlugin.h"
#include "application/plugins/PluginIds.h"
#include "application/plugins/ShmServerPluginInterface.h"
#include "application/plugins/TMReportPluginInterface.h"
#include "data/RealtimeDataSource.h"
#include "data/storage/DataDriverFactory.h"
#include "metrics/ReportExtension.h"

namespace hayaku {

namespace {

std::atomic_bool g_shm_server_role{false};

bool isDriverType(const Parameter& parameter, const string& expected) {
  string type = parameter.tryGet<string>("type", "");
  to_lower(type);
  return type == expected;
}

void registerClickHouseDrivers(const SessionOptions& options) {
  const bool baseInfo = isDriverType(options.baseInfoParam(), "clickhouse");
  const bool block = isDriverType(options.blockParam(), "clickhouse");
  const bool kdata = isDriverType(options.kdataParam(), "clickhouse");
  const bool enabled = baseInfo || block || kdata;

  installFactorStorePluginBridge(kdata);
  if (!enabled) {
    return;
  }

  auto* plugin =
      getPlugin<DataDriverPluginInterface>(HAYAKU_PLUGIN_CLICKHOUSE_DRIVER);
  HAYAKU_CHECK(plugin, "{}: {}", htr("Can not find plugin"),
               HAYAKU_PLUGIN_CLICKHOUSE_DRIVER);

  if (baseInfo) {
    auto driver = plugin->getBaseInfoDriver();
    HAYAKU_CHECK(driver, "{}",
                 htr("Can not get clickhouse driver! Check your license!"));
    DataDriverFactory::regBaseInfoDriver(driver);
  }

  if (block) {
    auto driver = plugin->getBlockInfoDriver();
    HAYAKU_CHECK(driver, "{}",
                 htr("Can not get clickhouse driver! Check your license!"));
    DataDriverFactory::regBlockDriver(driver);
  }

  if (kdata) {
    auto driver = plugin->getKDataDriver();
    HAYAKU_CHECK(driver, "{}",
                 htr("Can not get clickhouse driver! Check your license!"));
    DataDriverFactory::regKDataDriver(driver);
  }
}

}  // namespace

void prepareDataRuntimeAssembly(const SessionOptions& options) {
  auto& plugins = getPluginRuntime();
  plugins.configurePluginPath(
      options.hayakuParam().tryGet<string>("plugindir", ""));
  const auto pluginPath = plugins.pluginPath();
  HAYAKU_INFO(htr("Plugin path: {}", pluginPath));

  registerClickHouseDrivers(options);
  setReportExtensionResolver([]() -> ReportExtension* {
    if (!isValidLicense()) {
      return nullptr;
    }
    return getPlugin<TMReportPluginInterface>(HAYAKU_PLUGIN_TMREPORT, false);
  });

  const bool useShmServer =
      options.hayakuParam().tryGet<bool>("use_shm_server", false);
  if (useShmServer) {
    setRealtimeDataSourceResolver([]() -> RealtimeDataSource* {
      if (isShmServerRole()) {
        return nullptr;
      }
      return getPlugin<ShmServerPluginInterface>(HAYAKU_PLUGIN_SHM_SERVER,
                                                 false);
    });
  } else {
    setRealtimeDataSourceResolver({});
  }
}

void releaseDataRuntimeAssembly() noexcept {
  setRealtimeDataSourceResolver({});
  setReportExtensionResolver({});
  uninstallFactorStorePluginBridge();
  setShmServerRole(false);
}

void setShmServerRole(bool role) noexcept {
  g_shm_server_role.store(role, std::memory_order_release);
}

bool isShmServerRole() noexcept {
  return g_shm_server_role.load(std::memory_order_acquire);
}

}  // namespace hayaku
