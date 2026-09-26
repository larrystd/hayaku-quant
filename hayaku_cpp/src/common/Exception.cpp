/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-13
 *      Author: fasiondog
 */

#include "Exception.h"

namespace hayaku {

const char *exception::what() const noexcept { return msg_.c_str(); }

}  // namespace hayaku