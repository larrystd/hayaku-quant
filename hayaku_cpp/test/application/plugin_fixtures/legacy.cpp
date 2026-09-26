/* Copyright (c) 2026 hikyuu.org */

#include "application/plugins/PluginBase.h"

namespace {
int g_createCount = 0;
int g_destroyCount = 0;

class LegacyPlugin final : public hayaku::PluginBase {
 public:
  LegacyPlugin() { ++g_createCount; }
  ~LegacyPlugin() override { ++g_destroyCount; }

  std::string info() const noexcept override { return "plugin-legacy"; }
};
}  // namespace

HAYAKU_PLUGIN_DEFINE(LegacyPlugin)

HAYAKU_PLUGIN_EXPORT int hayakuTestCreateCount() noexcept {
  return g_createCount;
}
HAYAKU_PLUGIN_EXPORT int hayakuTestDestroyCount() noexcept {
  return g_destroyCount;
}
