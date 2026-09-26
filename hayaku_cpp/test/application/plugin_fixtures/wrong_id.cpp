/* Copyright (c) 2026 hikyuu.org */

#include "application/plugins/PluginBase.h"

namespace {
int g_createCount = 0;

class WrongIdPlugin final : public hayaku::PluginBase {
public:
    WrongIdPlugin() { ++g_createCount; }
    std::string info() const noexcept override { return "wrong-id"; }
};
}  // namespace

HAYAKU_PLUGIN_DEFINE_V1(WrongIdPlugin, "different-plugin-id", 1)

HAYAKU_PLUGIN_EXPORT int hayakuTestCreateCount() noexcept { return g_createCount; }
