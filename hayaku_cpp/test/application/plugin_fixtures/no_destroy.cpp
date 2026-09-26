/* Copyright (c) 2026 hikyuu.org */

#include "application/plugins/PluginBase.h"

namespace {
int g_createCount = 0;

class NoDestroyPlugin final : public hayaku::PluginBase {
 public:
  NoDestroyPlugin() { ++g_createCount; }
  std::string info() const noexcept override { return "no-destroy"; }
};
}  // namespace

HAYAKU_PLUGIN_DEFINE(NoDestroyPlugin)
HAYAKU_PLUGIN_EXPORT uint32_t hayakuPluginAbiVersion() noexcept {
  return hayaku::HAYAKU_PLUGIN_ABI_VERSION;
}
HAYAKU_PLUGIN_EXPORT uint32_t hayakuPluginInterfaceVersion() noexcept {
  return 1;
}
HAYAKU_PLUGIN_EXPORT const char* hayakuPluginId() noexcept {
  return "hayaku_abi_no_destroy";
}
HAYAKU_PLUGIN_EXPORT int hayakuTestCreateCount() noexcept {
  return g_createCount;
}
