#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-18
 *      Author: fasiondog
 */


#include <cstdint>
#include <exception>
#include <string>
#include "common/Config.h"
#include "common/OsDef.h"
#include "common/Log.h"

namespace hayaku {

// The C entry points below are optional for binaries built before ABI v1. Do not add virtual
// methods to PluginBase: doing so would change the vtable of existing out-of-tree plugins.
inline constexpr uint32_t HAYAKU_PLUGIN_ABI_VERSION = 1;
inline constexpr uint32_t HAYAKU_PLUGIN_DEFAULT_INTERFACE_VERSION = 1;

template <typename T>
constexpr uint32_t pluginInterfaceVersion() noexcept {
    if constexpr (requires { T::PLUGIN_INTERFACE_VERSION; }) {
        return T::PLUGIN_INTERFACE_VERSION;
    } else {
        return HAYAKU_PLUGIN_DEFAULT_INTERFACE_VERSION;
    }
}

class PluginBase {
public:
    PluginBase() = default;
    virtual ~PluginBase() = default;

    /**
     * @brief Return the plugin information
     * @details
     * The plugin information is in the json format, containing the name (string), version (int),
     * description (string) and author (string) fields
     * e.g.: {"name":"unknown","version": 1.0,"description":"","author":"unknown"}
     * @return std::string
     */
    virtual std::string info() const noexcept = 0;
};

}  // namespace hayaku

#if HAYAKU_OS_WINDOWS
#define HAYAKU_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
#define HAYAKU_PLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
#endif

#define HAYAKU_PLUGIN_DEFINE(plugin)                                  \
    HAYAKU_PLUGIN_EXPORT hayaku::PluginBase* createPlugin() {             \
        try {                                                       \
            return new plugin();                                    \
        } catch (const std::exception& e) {                         \
            HAYAKU_ERROR("{}", e.what());                              \
            return nullptr;                                         \
        } catch (...) {                                             \
            return nullptr;                                         \
        }                                                           \
    }

// Opt-in ABI v1. The host checks the version and stable ID before calling createPlugin, then
// calls destroyPlugin before unloading the library. Keep HAYAKU_PLUGIN_DEFINE legacy-compatible
// because external plugins may provide their own metadata/destroy symbols alongside that macro.
#define HAYAKU_PLUGIN_DEFINE_V1(plugin, pluginId, interfaceVersion)    \
    HAYAKU_PLUGIN_DEFINE(plugin)                                       \
    HAYAKU_PLUGIN_EXPORT uint32_t hayakuPluginAbiVersion() noexcept {    \
        return hayaku::HAYAKU_PLUGIN_ABI_VERSION;                         \
    }                                                               \
    HAYAKU_PLUGIN_EXPORT uint32_t hayakuPluginInterfaceVersion() noexcept { \
        return interfaceVersion;                                    \
    }                                                               \
    HAYAKU_PLUGIN_EXPORT const char* hayakuPluginId() noexcept {         \
        return pluginId;                                            \
    }                                                               \
    HAYAKU_PLUGIN_EXPORT void destroyPlugin(hayaku::PluginBase* instance) noexcept { \
        delete instance;                                            \
    }
