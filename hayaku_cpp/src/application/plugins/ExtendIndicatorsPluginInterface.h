#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-19
 *      Author: fasiondog
 */


#include "operators/Indicator.h"
#include "application/plugins/PluginBase.h"

namespace hayaku {

class ExtendIndicatorsPluginInterface : public PluginBase {
public:
    static constexpr uint32_t PLUGIN_INTERFACE_VERSION = 1;
    ExtendIndicatorsPluginInterface() = default;
    virtual ~ExtendIndicatorsPluginInterface() = default;

    virtual Indicator getIndicator(const std::string& name, const Parameter& params) const = 0;

    virtual Indicator getIndicator(const std::string& name, const Indicator& ref_ind,
                                   const Parameter& params) const = 0;

    using agg_func_t =
      std::function<Indicator::value_t(const DatetimeList& src_ds, const Indicator::value_t* src,
                                       size_t group_start, size_t group_last)>;
    virtual Indicator getAggFuncIndicator(const Indicator& ref_ind, agg_func_t agg_func,
                                          const Parameter& params) const = 0;

    using group_func_t =
      std::function<void(Indicator::value_t* dst, const DatetimeList& src_ds,
                         const Indicator::value_t* src, size_t group_start, size_t group_last)>;
    virtual Indicator getGroupFuncIndicator(const Indicator& ref_ind, group_func_t group_func,
                                            const Parameter& params) const = 0;
};

}  // namespace hayaku
