/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <utility>

#include "data/KQuery.h"
#include "common/config/ConfigLoader.h"
#include "SessionOptions.h"

namespace hku {

SessionOptions::SessionOptions(Parameter baseInfoParam, Parameter blockParam, Parameter kdataParam,
                               Parameter preloadParam, Parameter hikyuuParam,
                               StrategyContext context)
: m_baseInfoParam(std::move(baseInfoParam)),
  m_blockParam(std::move(blockParam)),
  m_kdataParam(std::move(kdataParam)),
  m_preloadParam(std::move(preloadParam)),
  m_hikyuuParam(std::move(hikyuuParam)),
  m_context(std::move(context)) {}

SessionOptions SessionOptions::fromIni(const string& filename, bool ignorePreload,
                                       const StrategyContext& context) {
    Parameter baseInfoParam;
    Parameter blockParam;
    Parameter kdataParam;
    Parameter preloadParam;
    Parameter hikyuuParam;
    getConfigFromIni(filename, baseInfoParam, blockParam, kdataParam, preloadParam, hikyuuParam);

    if (ignorePreload) {
        for (const auto& ktype : KQuery::getBaseKTypeList()) {
            string lowKtype = ktype;
            to_lower(lowKtype);
            preloadParam.set<bool>(lowKtype, false);
        }
    }

    return SessionOptions(std::move(baseInfoParam), std::move(blockParam), std::move(kdataParam),
                          std::move(preloadParam), std::move(hikyuuParam), context);
}

}  // namespace hku
