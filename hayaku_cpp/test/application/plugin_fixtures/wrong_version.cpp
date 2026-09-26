/* Copyright (c) 2026 hikyuu.org */

#include "application/plugins/PluginBase.h"

namespace {
int g_createCount = 0;

class WrongVersionPlugin final : public hayaku::PluginBase {
 public:
  WrongVersionPlugin() { ++g_createCount; }
  std::string info() const noexcept override { return "wrong-version"; }
};
}  // namespace

HAYAKU_PLUGIN_DEFINE(WrongVersionPlugin)
HAYAKU_PLUGIN_EXPORT uint32_t hayakuPluginAbiVersion() noexcept {
  return hayaku::HAYAKU_PLUGIN_ABI_VERSION + 1;
}
HAYAKU_PLUGIN_EXPORT uint32_t hayakuPluginInterfaceVersion() noexcept {
  return 1;
}
HAYAKU_PLUGIN_EXPORT const char* hayakuPluginId() noexcept {
  return "hayaku_abi_wrong_version";
}
HAYAKU_PLUGIN_EXPORT void destroyPlugin(hayaku::PluginBase* instance) noexcept {
  delete instance;
}
HAYAKU_PLUGIN_EXPORT int hayakuTestCreateCount() noexcept {
  return g_createCount;
}
