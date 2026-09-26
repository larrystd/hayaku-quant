/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */

#include "DevicePlugin.h"

#include "DevicePluginInterface.h"
#include "PluginIds.h"
#include "application/PluginRuntime.h"

namespace hayaku {

void bindEmail(const std::string& email, const std::string& active_code) {
  auto* plugin = getPlugin<DevicePluginInterface>(HAYAKU_PLUGIN_DEVICE);
  HAYAKU_ERROR_IF_RETURN(
      !plugin, void(),
      fmt::format("Can't find {} plugin!", HAYAKU_PLUGIN_DEVICE));
  plugin->bind(email, active_code);
}

void activeDevice(const std::string& active_code, bool replace) {
  auto* plugin = getPlugin<DevicePluginInterface>(HAYAKU_PLUGIN_DEVICE);
  HAYAKU_ERROR_IF_RETURN(
      !plugin, void(),
      fmt::format("Can't find {} plugin!", HAYAKU_PLUGIN_DEVICE));
  plugin->activate(active_code, replace);
}

std::string viewLicense() {
  auto* plugin = getPlugin<DevicePluginInterface>(HAYAKU_PLUGIN_DEVICE);
  if (!plugin) {
    return fmt::format("Can't find {} plugin!", HAYAKU_PLUGIN_DEVICE);
  }
  return plugin->viewLicense();
}

void removeLicense() {
  auto* plugin = getPlugin<DevicePluginInterface>(HAYAKU_PLUGIN_DEVICE);
  HAYAKU_ERROR_IF_RETURN(
      !plugin, void(),
      fmt::format("Can't find {} plugin!", HAYAKU_PLUGIN_DEVICE));
  plugin->removeLicense();
}

std::string fetchTrialLicense(const std::string& email) {
  auto* plugin = getPlugin<DevicePluginInterface>(HAYAKU_PLUGIN_DEVICE);
  if (!plugin) {
    return fmt::format("Can't find {} plugin!", HAYAKU_PLUGIN_DEVICE);
  }
  return plugin->fetchTrialLicense(email);
}

bool isValidLicense() {
  auto* plugin = getPlugin<DevicePluginInterface>(HAYAKU_PLUGIN_DEVICE, false);
  if (!plugin) {
    return false;
  }
  return plugin->isValidLicsense();
}

Datetime getExpireDate() {
  auto* plugin = getPlugin<DevicePluginInterface>(HAYAKU_PLUGIN_DEVICE, false);
  HAYAKU_IF_RETURN(!plugin, Datetime::min());
  return plugin->getExpireDate();
}

}  // namespace hayaku
