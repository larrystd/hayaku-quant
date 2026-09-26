#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Process-level owner for optional plugin libraries.
 */

#include <mutex>
#include <string>

#include "application/plugins/PluginManager.h"

#ifndef HAYAKU_API
#define HAYAKU_API
#endif

namespace hayaku {

/**
 * Optional plugins belong to the application assembly layer, not to
 * DataRuntime.
 *
 * The runtime intentionally lives for the process lifetime. Plugin facades
 * borrow raw interface pointers and may outlive an HayakuSession, so
 * HayakuSession::close() never unloads plugin libraries. Runtime
 * dlclose/hot-unload is not supported: services and plugin-created objects must
 * be stopped and destroyed before a library could be unloaded. The
 * process-level manager is not cleared.
 */
class PluginRuntime final {
 public:
  PluginRuntime(const PluginRuntime&) = delete;
  PluginRuntime& operator=(const PluginRuntime&) = delete;

  void setPluginPath(const std::string& path) noexcept;
  [[nodiscard]] std::string pluginPath() const;
  void configurePluginPath(const std::string& sessionPath);

  template <typename PluginInterfaceT>
  PluginInterfaceT* get(const std::string& pluginName,
                        bool print = true) noexcept {
    return m_manager.getPlugin<PluginInterfaceT>(pluginName, print);
  }

 private:
  friend PluginRuntime& getPluginRuntime();
  PluginRuntime() : m_manager("./plugin") {}

 private:
  mutable std::mutex m_mutex;
  PluginManager m_manager;
  bool m_userConfiguredPath{false};
};

HAYAKU_API PluginRuntime& getPluginRuntime();
HAYAKU_API void setPluginPath(const std::string& path) noexcept;
HAYAKU_API std::string getPluginPath();

template <typename PluginInterfaceT>
PluginInterfaceT* getPlugin(const std::string& pluginName,
                            bool print = true) noexcept {
  return getPluginRuntime().get<PluginInterfaceT>(pluginName, print);
}

}  // namespace hayaku
