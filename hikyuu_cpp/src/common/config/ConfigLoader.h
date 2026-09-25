/*
 * Copyright (c) 2026 hikyuu.org
 *
 * INI configuration loading shared by the application boundary.
 */

#pragma once
#ifndef HIKYUU_COMMON_CONFIG_CONFIGLOADER_H
#define HIKYUU_COMMON_CONFIG_CONFIGLOADER_H

#include "common/Parameter.h"

namespace hku {

void HKU_API getConfigFromIni(const string& configFileName, Parameter& baseParam,
                              Parameter& blockParam, Parameter& kdataParam,
                              Parameter& preloadParam, Parameter& hkuParam);

}  // namespace hku

#endif /* HIKYUU_COMMON_CONFIG_CONFIGLOADER_H */
