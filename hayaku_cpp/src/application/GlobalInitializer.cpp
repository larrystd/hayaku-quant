/*
 * Copyright (c) 2026 hikyuu.org
 */

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GlobalInitializer.h"

#include <fmt/format.h>
#include <nng/nng.h>

#include <mutex>
#include <thread>

#include "application/SystemInfo.h"
#include "common/Log.h"
#include "common/Os.h"
#include "common/concurrency/ParallelAlgorithms.h"
#include "config.h"
#include "data/storage/DataDriverFactory.h"
#include "operators/IndicatorImp.h"

#if HAYAKU_ENABLE_HDF5_KDATA
#include <H5public.h>
#endif

#if HAYAKU_ENABLE_TA_LIB
#include <ta-lib/ta_libc.h>
#endif

namespace hayaku {

namespace {

struct ProcessRuntimeState {
  std::mutex mutex;
  size_t users{0};
};

ProcessRuntimeState& processState() {
  static auto* state = new ProcessRuntimeState;
  return *state;
}

void initializeProcessRuntime() {
  bool taInitialized = false;
  try {
    IndicatorImp::initEngine();
#if HAYAKU_USE_LOW_PRECISION
    fmt::print("Initialize hayaku_{}_low_precision ...\n",
               getVersionWithBuild());
#else
    fmt::print("Initialize hayaku_{} ...\n", getVersionWithBuild());
#endif

    if (createDir(fmt::format("{}/.hayaku", getUserDir()))) {
      initLogger(false, fmt::format("{}/.hayaku/hayaku.log", getUserDir()));
    } else {
      initLogger();
    }

    // This initializes in-memory state only. Network feedback is never started
    // implicitly.
    sysinfo_init();

#if HAYAKU_ENABLE_TA_LIB
    TA_Initialize();
    taInitialized = true;
#endif

    size_t cpuNum = std::thread::hardware_concurrency();
    if (cpuNum <= 10) {
      cpuNum *= 2;
    } else if (cpuNum <= 64) {
      cpuNum = cpuNum * 3 / 2;
    } else {
      cpuNum = cpuNum * 5 / 4;
    }
    init_global_task_group(cpuNum);
    DataDriverFactory::init();
  } catch (...) {
    DataDriverFactory::release();
    release_global_task_group();
#if HAYAKU_ENABLE_TA_LIB
    if (taInitialized) {
      TA_Shutdown();
    }
#endif
    IndicatorImp::releaseEngine();
    spdlog::drop_all();
    throw;
  }
}

void shutdownProcessRuntime() noexcept {
  try {
    DataDriverFactory::release();
    IndicatorImp::releaseEngine();

#if HAYAKU_ENABLE_TA_LIB
    TA_Shutdown();
#endif

    release_global_task_group();

#if !HAYAKU_OS_OSX
    nng_fini();
#endif

#if HAYAKU_ENABLE_HDF5_KDATA
    H5close();
#endif

    spdlog::drop_all();
  } catch (...) {
    // A noexcept shutdown must not terminate the host process.
  }
}

}  // namespace

void acquireProcessRuntime() {
  auto& state = processState();
  std::lock_guard<std::mutex> lock(state.mutex);
  if (state.users == 0) {
    initializeProcessRuntime();
  }
  ++state.users;
}

void releaseProcessRuntime() noexcept {
  auto& state = processState();
  std::lock_guard<std::mutex> lock(state.mutex);
  if (state.users == 0) {
    return;
  }
  --state.users;
  if (state.users == 0) {
    shutdownProcessRuntime();
  }
}

bool processRuntimeActive() noexcept {
  auto& state = processState();
  std::lock_guard<std::mutex> lock(state.mutex);
  return state.users != 0;
}

}  // namespace hayaku
