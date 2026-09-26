/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-12
 *      Author: fasiondog
 */

#pragma once

#include <data/KRecord.h>

#include "common/PybindSupport.h"

namespace hayaku {

KRecordList df_to_krecords(const py::object& df, const StringList& cols);

}