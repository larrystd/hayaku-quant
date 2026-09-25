/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */

#include "data/internal/DataRuntime.h"
#include "interface/plugins.h"
#include "device.h"

namespace hku {

void HKU_API bindEmail(const std::string& email, const std::string& active_code) {
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<DevicePluginInterface>(HKU_PLUGIN_DEVICE);
    HKU_ERROR_IF_RETURN(!plugin, void(), htr("Can't find {} plugin!", HKU_PLUGIN_DEVICE));
    plugin->bind(email, active_code);
}

void HKU_API activeDevice(const std::string& active_code, bool replace) {
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<DevicePluginInterface>(HKU_PLUGIN_DEVICE);
    HKU_ERROR_IF_RETURN(!plugin, void(), htr("Can't find {} plugin!", HKU_PLUGIN_DEVICE));
    plugin->activate(active_code, replace);
}

std::string HKU_API viewLicense() {
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<DevicePluginInterface>(HKU_PLUGIN_DEVICE);
    if (!plugin) {
        return fmt::format("Can't find {} plugin!", HKU_PLUGIN_DEVICE);
    }
    return plugin->viewLicense();
}

void HKU_API removeLicense() {
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<DevicePluginInterface>(HKU_PLUGIN_DEVICE);
    HKU_ERROR_IF_RETURN(!plugin, void(), htr("Can't find {} plugin!", HKU_PLUGIN_DEVICE));
    plugin->removeLicense();
}

std::string HKU_API fetchTrialLicense(const std::string& email) {
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<DevicePluginInterface>(HKU_PLUGIN_DEVICE);
    if (!plugin) {
        return fmt::format("Can't find {} plugin!", HKU_PLUGIN_DEVICE);
    }
    return plugin->fetchTrialLicense(email);
}

bool HKU_API isValidLicense() {
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<DevicePluginInterface>(HKU_PLUGIN_DEVICE, false);
    if (!plugin) {
        return false;
    }
    return plugin->isValidLicsense();
}

Datetime HKU_API getExpireDate() {
    auto& runtime = getDataRuntime();
    auto* plugin = runtime.getPlugin<DevicePluginInterface>(HKU_PLUGIN_DEVICE, false);
    HKU_IF_RETURN(!plugin, Datetime::min());
    return plugin->getExpireDate();
}

}  // namespace hku