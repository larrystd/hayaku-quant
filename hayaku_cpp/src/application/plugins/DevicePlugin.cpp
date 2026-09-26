/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */

#include "application/PluginRuntime.h"
#include "PluginIds.h"
#include "DevicePluginInterface.h"
#include "DevicePlugin.h"

namespace hayaku {

void HAYAKU_API bindEmail(const std::string& email, const std::string& active_code) {
    auto* plugin = getPlugin<DevicePluginInterface>(HAYAKU_PLUGIN_DEVICE);
    HAYAKU_ERROR_IF_RETURN(!plugin, void(), htr("Can't find {} plugin!", HAYAKU_PLUGIN_DEVICE));
    plugin->bind(email, active_code);
}

void HAYAKU_API activeDevice(const std::string& active_code, bool replace) {
    auto* plugin = getPlugin<DevicePluginInterface>(HAYAKU_PLUGIN_DEVICE);
    HAYAKU_ERROR_IF_RETURN(!plugin, void(), htr("Can't find {} plugin!", HAYAKU_PLUGIN_DEVICE));
    plugin->activate(active_code, replace);
}

std::string HAYAKU_API viewLicense() {
    auto* plugin = getPlugin<DevicePluginInterface>(HAYAKU_PLUGIN_DEVICE);
    if (!plugin) {
        return fmt::format("Can't find {} plugin!", HAYAKU_PLUGIN_DEVICE);
    }
    return plugin->viewLicense();
}

void HAYAKU_API removeLicense() {
    auto* plugin = getPlugin<DevicePluginInterface>(HAYAKU_PLUGIN_DEVICE);
    HAYAKU_ERROR_IF_RETURN(!plugin, void(), htr("Can't find {} plugin!", HAYAKU_PLUGIN_DEVICE));
    plugin->removeLicense();
}

std::string HAYAKU_API fetchTrialLicense(const std::string& email) {
    auto* plugin = getPlugin<DevicePluginInterface>(HAYAKU_PLUGIN_DEVICE);
    if (!plugin) {
        return fmt::format("Can't find {} plugin!", HAYAKU_PLUGIN_DEVICE);
    }
    return plugin->fetchTrialLicense(email);
}

bool HAYAKU_API isValidLicense() {
    auto* plugin = getPlugin<DevicePluginInterface>(HAYAKU_PLUGIN_DEVICE, false);
    if (!plugin) {
        return false;
    }
    return plugin->isValidLicsense();
}

Datetime HAYAKU_API getExpireDate() {
    auto* plugin = getPlugin<DevicePluginInterface>(HAYAKU_PLUGIN_DEVICE, false);
    HAYAKU_IF_RETURN(!plugin, Datetime::min());
    return plugin->getExpireDate();
}

}  // namespace hayaku
