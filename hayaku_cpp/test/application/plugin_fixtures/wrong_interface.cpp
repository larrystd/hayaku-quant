/* Copyright (c) 2026 hikyuu.org */

#include "application/plugins/PluginBase.h"

namespace {
int g_createCount = 0;

class WrongInterfacePlugin final : public hayaku::PluginBase {
public:
    WrongInterfacePlugin() { ++g_createCount; }
    std::string info() const noexcept override { return "wrong-interface"; }
};
}  // namespace

HAYAKU_PLUGIN_DEFINE_V1(WrongInterfacePlugin, "hayaku_abi_wrong_interface", 2)

HAYAKU_PLUGIN_EXPORT int hayakuTestCreateCount() noexcept { return g_createCount; }
