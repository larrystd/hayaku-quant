#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-08
 *      Author: fasiondog
 */

#include "application/plugins/PluginBase.h"
#include "common/time/Datetime.h"

namespace hayaku {

class DevicePluginInterface : public PluginBase {
 public:
  static constexpr uint32_t PLUGIN_INTERFACE_VERSION = 1;
  DevicePluginInterface() = default;
  virtual ~DevicePluginInterface() = default;

  virtual void bind(const std::string& email,
                    const std::string& active_code) noexcept = 0;
  virtual void activate(const std::string& active_code,
                        bool replace) noexcept = 0;
  virtual std::string viewLicense() noexcept = 0;
  virtual void removeLicense() noexcept = 0;
  virtual std::string fetchTrialLicense(const std::string& email) noexcept = 0;
  virtual bool isValidLicsense() noexcept = 0;
  virtual Datetime getExpireDate() const noexcept = 0;
};

}  // namespace hayaku
