/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <utility>

#include "data/KQuery.h"
#include "application/ConfigLoader.h"
#include "SessionOptions.h"

namespace hayaku {

SessionOptions::SessionOptions(Parameter baseInfoParam, Parameter blockParam, Parameter kdataParam,
                               Parameter preloadParam, Parameter hayakuParam,
                               StrategyContext context)
: m_baseInfoParam(std::move(baseInfoParam)),
  m_blockParam(std::move(blockParam)),
  m_kdataParam(std::move(kdataParam)),
  m_preloadParam(std::move(preloadParam)),
  m_hayakuParam(std::move(hayakuParam)),
  m_context(std::move(context)) {}

SessionOptions SessionOptions::fromIni(const string& filename, bool ignorePreload,
                                       const StrategyContext& context) {
    Parameter baseInfoParam;
    Parameter blockParam;
    Parameter kdataParam;
    Parameter preloadParam;
    Parameter hayakuParam;
    getConfigFromIni(filename, baseInfoParam, blockParam, kdataParam, preloadParam, hayakuParam);

    if (ignorePreload) {
        for (const auto& ktype : KQuery::getBaseKTypeList()) {
            string lowKtype = ktype;
            to_lower(lowKtype);
            preloadParam.set<bool>(lowKtype, false);
        }
    }

    return SessionOptions(std::move(baseInfoParam), std::move(blockParam), std::move(kdataParam),
                          std::move(preloadParam), std::move(hayakuParam), context);
}

}  // namespace hayaku
