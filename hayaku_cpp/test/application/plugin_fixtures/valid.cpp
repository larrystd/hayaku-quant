/* Copyright (c) 2026 hikyuu.org */

#include "VersionTwoPluginInterface.h"

namespace {
int g_createCount = 0;
int g_destroyCount = 0;

class ValidPlugin final : public hayaku::VersionTwoPluginInterface {
public:
    ValidPlugin() { ++g_createCount; }
    ~ValidPlugin() override { ++g_destroyCount; }

    std::string info() const noexcept override { return "plugin-abi-v1"; }
};
}  // namespace

HAYAKU_PLUGIN_DEFINE_V1(ValidPlugin, "hayaku_abi_valid", 1)

HAYAKU_PLUGIN_EXPORT int hayakuTestCreateCount() noexcept { return g_createCount; }
HAYAKU_PLUGIN_EXPORT int hayakuTestDestroyCount() noexcept { return g_destroyCount; }
