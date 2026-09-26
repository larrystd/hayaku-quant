/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "PluginRuntime.h"

namespace hayaku {

PluginRuntime& getPluginRuntime() {
  // Do not participate in static destruction ordering. Facades and Python
  // objects can retain plugin interface pointers until interpreter/process
  // shutdown.
  static auto* runtime = new PluginRuntime;
  return *runtime;
}

void PluginRuntime::setPluginPath(const std::string& path) noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  manager_.pluginPath(path);
  user_configured_path_ = true;
}

std::string PluginRuntime::pluginPath() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return manager_.pluginPath();
}

void PluginRuntime::configurePluginPath(const std::string& sessionPath) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (user_configured_path_) {
    return;
  }

  const std::string requestedPath =
      sessionPath.empty() ? "./plugin" : sessionPath;
  manager_.pluginPath(requestedPath);
  HAYAKU_CHECK(manager_.pluginPath() == requestedPath,
               "Plugin path cannot change after a plugin has been loaded "
               "(current: {}, requested: {})",
               manager_.pluginPath(), requestedPath);
}

void setPluginPath(const std::string& path) noexcept {
  getPluginRuntime().setPluginPath(path);
}

std::string getPluginPath() { return getPluginRuntime().pluginPath(); }

}  // namespace hayaku
