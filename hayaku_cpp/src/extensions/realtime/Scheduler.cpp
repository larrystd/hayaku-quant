/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-14
 *     Author: fasiondog
 */

#include "Scheduler.h"

#include <mutex>

#include "common/Log.h"

namespace hayaku {

namespace {
std::mutex g_scheduler_mutex;
TimerManager* g_scheduler{nullptr};
}  // namespace

TimerManager* getScheduler() {
  std::lock_guard<std::mutex> lock(g_scheduler_mutex);
  if (!g_scheduler) {
    g_scheduler = new TimerManager(1);
  }
  return g_scheduler;
}

void releaseScheduler() {
  std::lock_guard<std::mutex> lock(g_scheduler_mutex);
  HAYAKU_TRACE("releaseScheduler");
  if (g_scheduler) {
    delete g_scheduler;
    g_scheduler = nullptr;
  }
}

}  // namespace hayaku
