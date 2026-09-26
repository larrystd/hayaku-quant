/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-14
 *     Author: fasiondog
 */

#include "ScheduledTasks.h"

#include "Scheduler.h"
#include "data/DataRuntime.h"
#include "extensions/realtime/RealtimePort.h"

namespace hayaku {

void initInnerTask() {
  const auto& param = getDataRuntime().getHayakuParameter();
  if (!param.tryGet<bool>("auto_reload", false)) {
    return;
  }

  int64_t hh = 0, mm = 0;
  bool reload_enable = false;
  try {
    string reload_time = param.tryGet<string>("reload_time", "00:00");
    auto hh_mm = split(reload_time, ":");
    HAYAKU_CHECK(hh_mm.size() == 2, "reload_time format error: {}",
                 reload_time);

    hh = std::stoll(string(hh_mm[0]));
    mm = std::stoll(string(hh_mm[1]));
    HAYAKU_CHECK(hh >= 0 && hh <= 23 && mm >= 0 && mm <= 59,
                 "reload_time format error: {}", reload_time);
    reload_enable = true;
  } catch (const std::exception& e) {
    HAYAKU_ERROR("Can't auto reload!", e.what());
  }

  if (reload_enable) {
    // The shm client restarts 5 minutes later, so that the shm server finishes
    // restarting first
    if (getDataRuntime().isIpcClientMode()) {
      constexpr int64_t shm_client_reload_delay_minutes = 5;
      mm += shm_client_reload_delay_minutes;
      hh += mm / 60;
      mm %= 60;
      hh %= 24;  // Wrap around 24 hours (23:59 delayed becomes 00:04 of the
                 // next day)
      HAYAKU_INFO(
          "Running in shm client mode, daily auto reload delayed {} minutes to "
          "{:02d}:{:02d}",
          shm_client_reload_delay_minutes, hh, mm);
    }

    auto* tm = getScheduler();
    tm->addFuncAtTimeEveryDay(Datetime::min(), Datetime::max(),
                              TimeDelta(0, hh, mm), reloadHayakuTask);
  }
}

void reloadHayakuTask() {
  reloadWithRealtimePaused([] { getDataRuntime().reload(); });
}

}  // namespace hayaku
