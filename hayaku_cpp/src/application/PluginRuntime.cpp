/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "PluginRuntime.h"

namespace hayaku {

PluginRuntime& getPluginRuntime() {
    // Do not participate in static destruction ordering. Facades and Python objects can retain
    // plugin interface pointers until interpreter/process shutdown.
    static auto* runtime = new PluginRuntime;
    return *runtime;
}

void PluginRuntime::setPluginPath(const std::string& path) noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_manager.pluginPath(path);
    m_userConfiguredPath = true;
}

std::string PluginRuntime::pluginPath() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_manager.pluginPath();
}

void PluginRuntime::configurePluginPath(const std::string& sessionPath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_userConfiguredPath) {
        return;
    }

    const std::string requestedPath = sessionPath.empty() ? "./plugin" : sessionPath;
    m_manager.pluginPath(requestedPath);
    HAYAKU_CHECK(m_manager.pluginPath() == requestedPath,
              "Plugin path cannot change after a plugin has been loaded (current: {}, requested: {})",
              m_manager.pluginPath(), requestedPath);
}

void setPluginPath(const std::string& path) noexcept {
    getPluginRuntime().setPluginPath(path);
}

std::string getPluginPath() {
    return getPluginRuntime().pluginPath();
}

}  // namespace hayaku
