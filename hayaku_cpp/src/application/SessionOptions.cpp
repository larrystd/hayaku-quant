/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "SessionOptions.h"

#include <utility>

#include "application/ConfigLoader.h"
#include "data/KQuery.h"

namespace hayaku {

SessionOptions::SessionOptions(Parameter baseInfoParam, Parameter blockParam,
                               Parameter kdataParam, Parameter preloadParam,
                               Parameter hayakuParam, StrategyContext context)
    : base_info_param_(std::move(baseInfoParam)),
      block_param_(std::move(blockParam)),
      kdata_param_(std::move(kdataParam)),
      preload_param_(std::move(preloadParam)),
      hayaku_param_(std::move(hayakuParam)),
      context_(std::move(context)) {}

SessionOptions SessionOptions::fromIni(const string& filename,
                                       bool ignorePreload,
                                       const StrategyContext& context) {
  Parameter baseInfoParam;
  Parameter blockParam;
  Parameter kdataParam;
  Parameter preloadParam;
  Parameter hayakuParam;
  getConfigFromIni(filename, baseInfoParam, blockParam, kdataParam,
                   preloadParam, hayakuParam);

  if (ignorePreload) {
    for (const auto& ktype : KQuery::getBaseKTypeList()) {
      string lowKtype = ktype;
      to_lower(lowKtype);
      preloadParam.set<bool>(lowKtype, false);
    }
  }

  return SessionOptions(std::move(baseInfoParam), std::move(blockParam),
                        std::move(kdataParam), std::move(preloadParam),
                        std::move(hayakuParam), context);
}

}  // namespace hayaku
