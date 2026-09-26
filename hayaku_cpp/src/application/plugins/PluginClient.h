#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-06
 *      Author: fasiondog
 */

#include "PluginLoader.h"

namespace hayaku {

template <typename InterfaceT>
class PluginClient : public InterfaceT {
 public:
  PluginClient() = delete;
  PluginClient(const std::string &path, const std::string &filename) {
    loader_ = std::make_unique<PluginLoader>(path);
    HAYAKU_CHECK(
        loader_->load(filename, true, pluginInterfaceVersion<InterfaceT>()),
        "load plugin failed! {}/{}", path, filename);
    impl_ = loader_->instance<InterfaceT>();
    HAYAKU_CHECK(impl_, "plugin interface mismatch! {}/{}", path, filename);
  }
  virtual ~PluginClient() = default;

  PluginClient(const PluginClient &) = delete;
  PluginClient &operator=(const PluginClient &) = delete;

  PluginClient(PluginClient &&rhs)
      : impl_(rhs.impl_), loader_(std::move(rhs.loader_)) {
    rhs.impl_ = nullptr;
  }

  PluginClient &operator=(PluginClient &&rhs) {
    if (this != &rhs) {
      loader_ = std::move(rhs.loader_);
      impl_ = rhs.impl_;
      rhs.impl_ = nullptr;
    }
    return *this;
  }

  std::string info() const noexcept override { return impl_->info(); }

  InterfaceT *getPlugin() const { return impl_; }

 protected:
  InterfaceT *impl_{nullptr};

 protected:
  std::unique_ptr<PluginLoader> loader_;
};

}  // namespace hayaku
