#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-19
 *      Author: fasiondog
 */

#include "application/plugins/PluginBase.h"
#include "data/KDataExtension.h"

namespace hayaku {

class HayakuExtraPluginInterface : public PluginBase, public KDataExtension {
 public:
  static constexpr uint32_t PLUGIN_INTERFACE_VERSION = 1;
  HayakuExtraPluginInterface() = default;
  virtual ~HayakuExtraPluginInterface() = default;
};

}  // namespace hayaku
