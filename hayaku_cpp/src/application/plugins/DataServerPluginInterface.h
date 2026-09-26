#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-08
 *      Author: fasiondog
 */


#include "application/plugins/PluginBase.h"

namespace hayaku {

class DataServerPluginInterface : public PluginBase {
public:
    static constexpr uint32_t PLUGIN_INTERFACE_VERSION = 1;
    DataServerPluginInterface() = default;
    virtual ~DataServerPluginInterface() = default;

    virtual void start(const std::string& addr, size_t work_num, bool save_tick, bool buf_tick,
                       const std::string& parquet_path) noexcept = 0;
    virtual void stop() noexcept = 0;
};

}  // namespace hayaku
