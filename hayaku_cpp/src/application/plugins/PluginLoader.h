#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-18
 *      Author: fasiondog
 */


#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include "common/Config.h"
#include "common/OsDef.h"
#include "common/Os.h"
#include "common/Log.h"
#include "PluginBase.h"

#if HAYAKU_OS_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace hayaku {

class PluginLoader final {
public:
    using DestroyFunction = void (*)(PluginBase*);

    PluginLoader() : PluginLoader(".") {}
    explicit PluginLoader(const std::string& path) : m_path(path) {}

    PluginLoader(const PluginLoader&) = delete;
    PluginLoader(PluginLoader&&) = delete;

    ~PluginLoader() {
        unload();
    }

    template <typename T>
    T* instance() const noexcept {
        HAYAKU_IF_RETURN(!m_plugin, nullptr);
        return dynamic_cast<T*>(m_plugin.get());
    }

    bool supportsInterfaceVersion(uint32_t version) const noexcept {
        return m_plugin && m_interfaceVersion == version;
    }

    bool load(const std::string& pluginname, bool print = true,
              uint32_t expectedInterfaceVersion = HAYAKU_PLUGIN_DEFAULT_INTERFACE_VERSION) noexcept {
        unload();
        std::string filename = getFileName(pluginname);
        // HAYAKU_WARN_IF_RETURN(!existFile(filename), false, "file({}) not exist!", filename);
        HAYAKU_DEBUG_IF_RETURN(!existFile(filename), false, "file({}) not exist!", filename);

#if HAYAKU_OS_WINDOWS
        m_handle = LoadLibrary(HAYAKU_PATH(filename).c_str());
        if (!m_handle) {
            HAYAKU_WARN_IF(print, "load plugin({}) failed! errcode: {}", filename, GetLastError());
            return false;
        }

#else
        m_handle = dlopen(filename.c_str(), RTLD_LAZY);
        if (!m_handle) {
            HAYAKU_WARN_IF(print, "load plugin({}) failed! {}", filename, dlerror());
            return false;
        }
#endif

        using AbiVersionFunction = uint32_t (*)();
        auto abiVersionFunction =
          reinterpret_cast<AbiVersionFunction>(getFunction("hayakuPluginAbiVersion"));
        auto interfaceVersionFunction =
          reinterpret_cast<AbiVersionFunction>(getFunction("hayakuPluginInterfaceVersion"));
        auto destroyFunction = reinterpret_cast<DestroyFunction>(getFunction("destroyPlugin"));
        uint32_t interfaceVersion = interfaceVersionFunction ? interfaceVersionFunction() : 0;
        if (abiVersionFunction) {
            uint32_t abiVersion = abiVersionFunction();
            if (abiVersion != HAYAKU_PLUGIN_ABI_VERSION || !destroyFunction ||
                interfaceVersion != expectedInterfaceVersion) {
                HAYAKU_WARN_IF(print,
                            "Unsupported plugin ABI/interface for {}: ABI {}, expected interface {}, "
                            "interface version {}, destroyPlugin {}",
                            filename, abiVersion, expectedInterfaceVersion,
                            interfaceVersion,
                            destroyFunction ? "present" : "missing");
                unload();
                return false;
            }
        } else {
            if (expectedInterfaceVersion != HAYAKU_PLUGIN_DEFAULT_INTERFACE_VERSION) {
                HAYAKU_WARN_IF(print, "Legacy plugin {} cannot verify interface version {}", filename,
                            expectedInterfaceVersion);
                unload();
                return false;
            }
            HAYAKU_WARN_IF(print, "Plugin {} uses the legacy ABI without a version contract", filename);
        }

        using PluginIdFunction = const char* (*)();
        auto pluginIdFunction = reinterpret_cast<PluginIdFunction>(getFunction("hayakuPluginId"));
        if (abiVersionFunction && !pluginIdFunction) {
            HAYAKU_WARN_IF(print, "Plugin {} has no ID contract", filename);
            unload();
            return false;
        }
        if (pluginIdFunction) {
            const char* id = pluginIdFunction();
            if (!id || std::strcmp(id, pluginname.c_str()) != 0) {
                HAYAKU_WARN_IF(print, "Plugin ID mismatch for {}: expected {}, got {}", filename,
                            pluginname, id ? id : "(null)");
                unload();
                return false;
            }
        }

        using CreateFunction = PluginBase* (*)();
        auto createFunction = reinterpret_cast<CreateFunction>(getFunction("createPlugin"));
        if (!createFunction) {
            HAYAKU_WARN_IF(print, "Failed to get plugin({}) handle!", filename);
            unload();
            return false;
        }

        m_plugin.get_deleter() = destroyFunction ? destroyFunction : destroyLegacyPlugin;
        try {
            m_plugin.reset(createFunction());
        } catch (const std::exception& e) {
            HAYAKU_ERROR_IF(print, "Failed to create plugin ({}): {}", filename, e.what());
            unload();
            return false;
        } catch (...) {
            HAYAKU_ERROR_IF(print, "Failed to create plugin ({}): unknown exception", filename);
            unload();
            return false;
        }
        if (!m_plugin) {
            HAYAKU_ERROR_IF(print, "Failed to create plugin ({})!", filename);
            unload();
            return false;
        }

        m_interfaceVersion = expectedInterfaceVersion;

        return true;
    }

    std::string getFileName(const std::string& pluginname) const noexcept {
#if HAYAKU_OS_WINDOWS
        return fmt::format("{}/{}.dll", m_path, pluginname);
#elif HAYAKU_OS_LINUX
        return fmt::format("{}/lib{}.so", m_path, pluginname);
#elif HAYAKU_OS_OSX
        return fmt::format("{}/lib{}.dylib", m_path, pluginname);
#endif
    }

private:
    static void destroyLegacyPlugin(PluginBase* instance) noexcept {
        delete instance;
    }

    void unload() noexcept {
        m_plugin.reset();
        m_interfaceVersion = 0;
        if (m_handle) {
#if HAYAKU_OS_WINDOWS
            FreeLibrary(m_handle);
#else
            dlclose(m_handle);
#endif
            m_handle = nullptr;
        }
    }

    void* getFunction(const char* symbol) noexcept {
#if HAYAKU_OS_WINDOWS
        void* func = (void*)GetProcAddress(m_handle, symbol);
#else
        void* func = dlsym(m_handle, symbol);
#endif
        return func;
    }

private:
#if HAYAKU_OS_WINDOWS
    HMODULE m_handle{nullptr};
#else
    void* m_handle{nullptr};
#endif
    std::string m_path;
    uint32_t m_interfaceVersion{0};
    std::unique_ptr<PluginBase, DestroyFunction> m_plugin{nullptr, destroyLegacyPlugin};
};

}  // namespace hayaku
