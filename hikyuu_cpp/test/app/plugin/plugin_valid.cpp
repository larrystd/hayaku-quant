/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-16
 *      Author: fasiondog
 */

#include <app/plugin/interface/plugins.h>
#include <app/plugin/extind.h>
#include <app/plugin/device.h>
#include "app/plugin/plugin_valid.h"

using namespace hku;

bool pluginValid() {
    auto& sm = getDataRuntime();
    auto* plugin = sm.getPlugin<ExtendIndicatorsPluginInterface>(HKU_PLUGIN_EXTEND_INDICATOR);
    return plugin && isValidLicense();
}
