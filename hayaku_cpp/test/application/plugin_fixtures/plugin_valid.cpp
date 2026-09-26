/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-16
 *      Author: fasiondog
 */

#include "application/PluginRuntime.h"
#include "application/plugins/PluginIds.h"
#include "application/plugins/ExtendIndicatorsPluginInterface.h"
#include <application/plugins/ExtendIndicatorsPlugin.h>
#include <application/plugins/DevicePlugin.h>
#include "application/plugin_fixtures/plugin_valid.h"

using namespace hayaku;

bool pluginValid() {
    auto* plugin = getPlugin<ExtendIndicatorsPluginInterface>(HAYAKU_PLUGIN_EXTEND_INDICATOR);
    return plugin && isValidLicense();
}
