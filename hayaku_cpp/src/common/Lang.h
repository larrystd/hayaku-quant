#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-09
 *      Author: fasiondog
 */

#include <fmt/format.h>

#include <string>
#include <utility>

#include "common/CppDef.h"

#ifndef HAYAKU_API
#define HAYAKU_API
#endif

namespace hayaku {

// Note: use it inside hayaku related projects only, to avoid polluting other
// projects

void loadLocalLanguage(const std::string &path = "i8n");

std::string HAYAKU_API lang_htr(const char *id);

// Get the translation by context
std::string HAYAKU_API lang_hctr(const char *ctx, const char *id);

template <typename... Args>
std::string htr(const char *key, Args &&...args) {
  std::string fmt_str = lang_htr(key);
  return fmt::vformat(fmt_str,
                      fmt::make_format_args(std::forward<Args>(args)...));
}

template <typename... Args>
std::string chtr(const char *ctx, const char *key, Args &&...args) {
  std::string fmt_str = lang_hctr(ctx, key);
  return fmt::vformat(fmt_str,
                      fmt::make_format_args(std::forward<Args>(args)...));
}

}  // namespace hayaku
