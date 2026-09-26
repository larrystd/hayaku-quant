/* Copyright (c) 2026 hikyuu.org */

#pragma once

#include "application/plugins/PluginBase.h"

namespace hayaku {

class VersionTwoPluginInterface : public PluginBase {
 public:
  static constexpr uint32_t PLUGIN_INTERFACE_VERSION = 2;
};

}  // namespace hayaku
