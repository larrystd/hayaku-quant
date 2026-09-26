#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * INI configuration loading shared by the application boundary.
 */

#include "common/Parameter.h"

namespace hayaku {

void getConfigFromIni(const string& configFileName, Parameter& baseParam,
                      Parameter& blockParam, Parameter& kdataParam,
                      Parameter& preloadParam, Parameter& hayakuParam);

}  // namespace hayaku
