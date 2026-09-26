/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-06
 *      Author: fasiondog
 */

#include "ExtraPlugins.h"

#include "HayakuExtraPluginInterface.h"
#include "PluginIds.h"
#include "application/PluginRuntime.h"
#include "data/KDataExtension.h"

namespace hayaku {

void installHayakuExtraPluginBridge() {
  setKDataExtensionResolver([]() -> KDataExtension* {
    return getPlugin<HayakuExtraPluginInterface>(HAYAKU_PLUGIN_HAYAKU_EXTRA,
                                                 false);
  });
}

void uninstallHayakuExtraPluginBridge() noexcept {
  setKDataExtensionResolver({});
}

void HAYAKU_API
registerExtraKType(const string& ktype, const string& basetype, int32_t minutes,
                   std::function<Datetime(const Datetime&)> getPhaseEnd) {
  installHayakuExtraPluginBridge();
  auto* plugin =
      getPlugin<HayakuExtraPluginInterface>(HAYAKU_PLUGIN_HAYAKU_EXTRA);
  HAYAKU_ERROR_IF_RETURN(
      !plugin, void(),
      htr("Can't find {} plugin!", HAYAKU_PLUGIN_HAYAKU_EXTRA));
  plugin->registerKTypeExtra(ktype, basetype, minutes, getPhaseEnd);
}

void HAYAKU_API registerExtraKType(const string& ktype, const string& basetype,
                                   int32_t nbars) {
  installHayakuExtraPluginBridge();
  auto* plugin =
      getPlugin<HayakuExtraPluginInterface>(HAYAKU_PLUGIN_HAYAKU_EXTRA);
  HAYAKU_ERROR_IF_RETURN(
      !plugin, void(),
      htr("Can't find {} plugin!", HAYAKU_PLUGIN_HAYAKU_EXTRA));
  plugin->registerKTypeExtra(ktype, basetype, nbars,
                             std::function<Datetime(const Datetime&)>());
}

void HAYAKU_API releaseExtraKType() {
  auto* extension = getKDataExtension();
  HAYAKU_IF_RETURN(!extension, void());
  extension->releaseKExtra();
}

void HAYAKU_API enableKDataCache(bool enable) {
  installHayakuExtraPluginBridge();
  auto* plugin =
      getPlugin<HayakuExtraPluginInterface>(HAYAKU_PLUGIN_HAYAKU_EXTRA);
  HAYAKU_IF_RETURN(!plugin, void());
  plugin->enableKDataCache(enable);
}

}  // namespace hayaku
