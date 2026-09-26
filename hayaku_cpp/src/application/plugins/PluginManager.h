#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-10
 *      Author: fasiondog
 */

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "application/plugins/PluginLoader.h"
#include "common/Log.h"

namespace hayaku {

class PluginManager final {
 public:
  PluginManager() : plugin_path_(".") {};
  explicit PluginManager(const std::string& plugin_path)
      : plugin_path_(plugin_path) {}

  ~PluginManager() = default;
  PluginManager(const PluginManager&) = delete;
  PluginManager(PluginManager&&) = delete;
  PluginManager& operator=(const PluginManager&) = delete;
  PluginManager& operator=(PluginManager&&) = delete;

  const std::string& pluginPath() const noexcept { return plugin_path_; }

  void pluginPath(const std::string& plugin_path) noexcept {
    HAYAKU_TRACE_IF_RETURN(
        !plugins_.empty(), void(),
        "Existing loaded plugins, Ignore set plugin path: {}, ", plugin_path);
    plugin_path_ = plugin_path;
  }

  // Invalidates every raw interface pointer returned by getPlugin(). Callers
  // must first stop all plugin services and release every interface pointer and
  // plugin-created object; the lock below protects the map only, not a caller
  // using a pointer after getPlugin() returns.
  void clear() noexcept {
    std::unique_lock<std::shared_mutex> write_lock(mutex_);
    plugins_.clear();
  }

  // The returned pointer is borrowed. It is valid only while this manager, the
  // corresponding plugin library and its loaded instance remain alive and
  // clear() is not called.
  template <typename PluginInterfaceT>
  PluginInterfaceT* getPlugin(const std::string& pluginname,
                              bool print = true) noexcept {
    PluginInterfaceT* ret{nullptr};
    try {
      {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        auto it = plugins_.find(pluginname);
        if (it != plugins_.end()) {
          if (!it->second->supportsInterfaceVersion(
                  pluginInterfaceVersion<PluginInterfaceT>())) {
            HAYAKU_WARN_IF(
                print,
                "Plugin {} does not match the requested interface version",
                pluginname);
            return nullptr;
          }
          ret = it->second->instance<PluginInterfaceT>();
          return ret;
        }
      }

      std::unique_ptr<PluginLoader> loader =
          std::make_unique<PluginLoader>(plugin_path_);
      if (!loader->load(pluginname, print,
                        pluginInterfaceVersion<PluginInterfaceT>())) {
        HAYAKU_DEBUG("Load plugin {} failed: {}", pluginname,
                     loader->getFileName(pluginname));
        return ret;
      }
      ret = loader->instance<PluginInterfaceT>();
      if (!ret) {
        HAYAKU_WARN_IF(print,
                       "Plugin {} does not implement the requested interface",
                       pluginname);
        return nullptr;
      }

      {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        auto it = plugins_.find(pluginname);
        if (it != plugins_.end()) {
          // Reuse the plugin instance already inserted
          ret = it->second->supportsInterfaceVersion(
                    pluginInterfaceVersion<PluginInterfaceT>())
                    ? it->second->instance<PluginInterfaceT>()
                    : nullptr;
        } else {
          // Insert the newly loaded plugin
          auto [it, success] =
              plugins_.insert(std::make_pair(pluginname, std::move(loader)));
          if (success) {
            ret = it->second->instance<PluginInterfaceT>();
          }
        }
        return ret;
      }
    } catch (const std::exception& e) {
      HAYAKU_ERROR_IF(print, "Load plugin {} failed: {}", pluginname, e.what());
      ret = nullptr;
    } catch (...) {
      HAYAKU_ERROR_IF(print, "Load plugin {} failed: unknown exception",
                      pluginname);
      ret = nullptr;
    }
    return ret;
  }

 private:
  std::string plugin_path_;
  std::unordered_map<std::string, std::unique_ptr<PluginLoader>> plugins_;
  std::shared_mutex mutex_;
};

}  // namespace hayaku
