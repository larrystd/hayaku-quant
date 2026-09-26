#pragma once

/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-03-15
 *      Author: fasiondog
 */

#include "common/Exception.h"

namespace hayaku {

struct HttpTimeoutException : hayaku::exception {
  HttpTimeoutException() : hayaku::exception("Http timeout!") {}
  explicit HttpTimeoutException(const char* msg) : hayaku::exception(msg) {}
  explicit HttpTimeoutException(const std::string& msg)
      : hayaku::exception(msg) {}
  virtual ~HttpTimeoutException() noexcept override = default;
};

}  // namespace hayaku
