/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-09
 *      Author: fasiondog
 */

#include "common/Arithmetic.h"
#include "common/Log.h"
#include "common/MoFileReader.h"
#include "common/Os.h"
#include "common/OsDef.h"
#if HAYAKU_OS_WINDOWS
#include <windows.h>
#elif HAYAKU_OS_OSX
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "Lang.h"

namespace hayaku {

void loadLocalLanguage(const std::string &path) {
  std::string lang = getSystemLanguage();
  if (lang == "zh_cn") {
    auto &reader = moFileLib::moFileReaderSingleton::GetInstance();
    reader.ReadFile(fmt::format("{}/zh_CN/hayaku.mo", path).c_str());
    reader.ReadFile(fmt::format("{}/zh_CN/hayaku_plugin.mo", path).c_str());
  }
}

std::string HAYAKU_API lang_htr(const char *id) {
  return moFileLib::moFileReaderSingleton::GetInstance().Lookup(id);
}

std::string HAYAKU_API lang_hctr(const char *ctx, const char *id) {
  return moFileLib::moFileReaderSingleton::GetInstance().LookupWithContext(ctx,
                                                                           id);
}

}  // namespace hayaku