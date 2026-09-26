#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-18
 *      Author: fasiondog
 */

#include <cstdlib>
#include <string>
#include <vector>

#include "common/Arithmetic.h"
#include "common/Config.h"
#include "common/Log.h"
#include "common/Os.h"
#include "common/OsDef.h"

#if HAYAKU_OS_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace hayaku {

class DllLoader {
 public:
  DllLoader() { initDefaultSearchPath(); }

  explicit DllLoader(const std::vector<std::string>& path) {
    if (!path.empty()) {
      search_paths_.resize(path.size());
      std::copy(path.begin(), path.end(), search_paths_.begin());
    } else {
      HAYAKU_WARN("DllLoader: empty search path! Use default search path!");
      initDefaultSearchPath();
    }
  }

  DllLoader(const DllLoader&) = delete;
  DllLoader& operator=(const DllLoader&) = delete;

  DllLoader(DllLoader&& rhs)
      : handle_(rhs.handle_), search_paths_(std::move(rhs.search_paths_)) {
    rhs.handle_ = nullptr;
  }

  DllLoader& operator=(DllLoader&& rhs) {
    if (this == &rhs) {
      handle_ = rhs.handle_;
      search_paths_ = std::move(rhs.search_paths_);
      rhs.handle_ = nullptr;
    }
    return *this;
  }

  virtual ~DllLoader() { unload(); }

  bool load(const std::string& dllname) noexcept {
    std::string filename = search(dllname);
    HAYAKU_WARN_IF_RETURN(filename.empty(), false, "Not found dll: {}!",
                          dllname);

#if HAYAKU_OS_WINDOWS
    handle_ = LoadLibrary(HAYAKU_PATH(filename).c_str());
#else
    handle_ = dlopen(filename.c_str(), RTLD_LAZY);
#endif
    HAYAKU_WARN_IF_RETURN(!handle_, false, "load dll({}) failed!", filename);
    return true;
  }

  void unload() noexcept {
    if (handle_) {
#if HAYAKU_OS_WINDOWS
      FreeLibrary(handle_);
#else
      dlclose(handle_);
#endif
    }
  }

  std::string search(const std::string& dllname) const noexcept {
#if HAYAKU_OS_WINDOWS
    std::string dll = fmt::format("{}.dll", dllname);
#elif HAYAKU_OS_OSX
    std::string dll = fmt::format("lib{}.dylib", dllname);
#else
    std::string dll = fmt::format("lib{}.so", dllname);
#endif
    for (const auto& path : search_paths_) {
      auto filename = fmt::format("{}/{}", path, dll);
      if (existFile(filename)) {
        return filename;
      }
    }

    return std::string();
  }

  void* getSymbol(const char* symbol) noexcept {
#if HAYAKU_OS_WINDOWS
    void* func = GetProcAddress(handle_, symbol);
#else
    void* func = dlsym(handle_, symbol);
#endif
    return func;
  }

 private:
  void initDefaultSearchPath() noexcept {
    search_paths_.emplace_back(".");

    std::string userdir = getUserDir();
    if (!userdir.empty()) {
      search_paths_.emplace_back(userdir + "/lib");
    }

#if HAYAKU_OS_WINDOWS
    search_paths_.emplace_back("C:/Windows/System32");
    search_paths_.emplace_back("C:/Windows/SysWOW64");
    search_paths_.emplace_back("C:/Windows");
    const char* path = getenv("PATH");
    if (path) {
      std::string pathstr(path);
      auto items = split(pathstr, ";");
      for (auto& item : items) {
        std::string nitem(item);
        trim(nitem);
        if (!item.empty()) {
          search_paths_.emplace_back(nitem);
        }
      }
    }

#elif HAYAKU_OS_OSX
    const char* path = getenv("DYLD_IMAGE_SUFFIX");
    if (path) {
      std::string pathstr(path);
      auto items = split(pathstr, ";");
      for (auto& item : items) {
        std::string nitem(item);
        trim(nitem);
        if (!item.empty()) {
          search_paths_.emplace_back(nitem);
        }
      }
    }
    path = getenv("DYLD_FRAMEWORK_PATH");
    if (path) {
      std::string pathstr(path);
      auto items = split(pathstr, ";");
      for (auto& item : items) {
        std::string nitem(item);
        trim(nitem);
        if (!item.empty()) {
          search_paths_.emplace_back(nitem);
        }
      }
    }
    path = getenv("DYLD_LIBRARY_PATH");
    if (path) {
      std::string pathstr(path);
      auto items = split(pathstr, ";");
      for (auto& item : items) {
        std::string nitem(item);
        trim(nitem);
        if (!item.empty()) {
          search_paths_.emplace_back(nitem);
        }
      }
    }
    search_paths_.emplace_back("/Library/Frameworks");
    search_paths_.emplace_back("/Network/Library/Frameworks");
    search_paths_.emplace_back("/System/Library/Frameworks");
    search_paths_.emplace_back("/usr/local/lib");
    search_paths_.emplace_back("/usr/lib");
    search_paths_.emplace_back("/lib");

#else
    const char* path = getenv("LD_LIBRARY_PATH");
    if (path) {
      std::string pathstr(path);
      auto items = split(pathstr, ";");
      for (auto& item : items) {
        std::string nitem(item);
        trim(nitem);
        if (!item.empty()) {
          search_paths_.emplace_back(nitem);
        }
      }
    }
    search_paths_.emplace_back("/usr/local/lib");
    search_paths_.emplace_back("/usr/lib");
    search_paths_.emplace_back("/lib");
    if (HAYAKU_ARCH_X64) {
      search_paths_.emplace_back("/usr/lib/x86_64-linux-gnu");
    } else if (HAYAKU_ARCH_ARM64) {
      search_paths_.emplace_back("/usr/lib/aarch64-linux-gnu");
    } else if (HAYAKU_ARCH_X86) {
      search_paths_.emplace_back("/usr/lib/i386-linux-gnu");
    } else if (HAYAKU_ARCH_ARM) {
      search_paths_.emplace_back("/usr/lib/arm-linux-gnueabihf");
    }
#endif
  }

 private:
#if HAYAKU_OS_WINDOWS
  HMODULE handle_{nullptr};
#else
  void* handle_{nullptr};
#endif

  std::vector<std::string> search_paths_;
};

}  // namespace hayaku
