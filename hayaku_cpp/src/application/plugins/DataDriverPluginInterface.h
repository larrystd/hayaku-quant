#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-01
 *      Author: fasiondog
 */

#include "application/plugins/PluginBase.h"
#include "data/storage/BaseInfoDriver.h"
#include "data/storage/BlockInfoDriver.h"
#include "data/storage/KDataDriver.h"
#include "operators/FactorStore.h"

namespace hayaku {

class DataDriverPluginInterface : public PluginBase, public FactorStore {
 public:
  static constexpr uint32_t PLUGIN_INTERFACE_VERSION = 1;
  DataDriverPluginInterface() = default;
  virtual ~DataDriverPluginInterface() = default;

  virtual KDataDriverPtr getKDataDriver() = 0;
  virtual BlockInfoDriverPtr getBlockInfoDriver() = 0;
  virtual BaseInfoDriverPtr getBaseInfoDriver() = 0;
};

}  // namespace hayaku
