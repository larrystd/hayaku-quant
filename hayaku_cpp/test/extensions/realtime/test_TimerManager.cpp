/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-09
 *     Author: fasiondog
 */

#include <extensions/realtime/TimerManager.h>

#include "common/Log.h"
#include "doctest/doctest.h"

using namespace hayaku;

/**
 * @defgroup test_hayaku_TimerManager test_hayaku_TimerManager
 * @ingroup test_hayaku_utilities
 * @{
 */

static void hello_test() { HAYAKU_TRACE("hello"); }

TEST_CASE("test_TimerManager") {
  TimerManager tm;
  tm.start();

  CHECK_THROWS_AS(tm.addDurationFunc(1, TimeDelta(), hello_test),
                  hayaku::exception);
  CHECK_THROWS_AS(tm.addDurationFunc(0, TimeDelta(1), hello_test),
                  hayaku::exception);
  CHECK_THROWS_AS(tm.addDurationFunc(-1, TimeDelta(1), hello_test),
                  hayaku::exception);

  CHECK_THROWS_AS(tm.addDelayFunc(TimeDelta(0), hello_test), hayaku::exception);
  CHECK_THROWS_AS(tm.addDelayFunc(TimeDelta(-1), hello_test),
                  hayaku::exception);

  CHECK_THROWS_AS(tm.addFuncAtTime(Datetime::min(), hello_test),
                  hayaku::exception);

  CHECK_THROWS_AS(tm.addFuncAtTimeEveryDay(TimeDelta(-1), hello_test),
                  hayaku::exception);
  CHECK_THROWS_AS(tm.addFuncAtTimeEveryDay(TimeDelta(1), hello_test),
                  hayaku::exception);

  CHECK_THROWS_AS(tm.addFuncAtTimeEveryDay(Datetime::min(), Datetime::max(),
                                           TimeDelta(-1), hello_test),
                  hayaku::exception);
  CHECK_THROWS_AS(tm.addFuncAtTimeEveryDay(Datetime::min(), Datetime::max(),
                                           TimeDelta(1), hello_test),
                  hayaku::exception);
  CHECK_THROWS_AS(tm.addFuncAtTimeEveryDay(Datetime::max(), Datetime::min(),
                                           TimeDelta(0, 1), hello_test),
                  hayaku::exception);
  CHECK_THROWS_AS(tm.addFuncAtTimeEveryDay(Datetime::min(), Datetime(),
                                           TimeDelta(0, 1), hello_test),
                  hayaku::exception);
  CHECK_THROWS_AS(tm.addFuncAtTimeEveryDay(Datetime(), Datetime::max(),
                                           TimeDelta(0, 1), hello_test),
                  hayaku::exception);

#if 0
    Datetime now = Datetime::now();
    TimeDelta time = now - now.startOfDay();
    tm.addDelayFunc(Microseconds(30), []() { HAYAKU_INFO("1 test delay *************************"); });
    tm.addDelayFunc(Seconds(2), []() { HAYAKU_INFO("2 test delay *************************"); });
    tm.addDurationFunc(3, Milliseconds(500),
                       []() { HAYAKU_INFO("3 test delay *************************"); });
    tm.addFuncAtTime(now + Seconds(1),
                     []() { HAYAKU_INFO("addFuncAtTime test delay *************************"); });
    tm.addFuncAtTimeEveryDay(Datetime::today(), Datetime::max(), time + Seconds(30), []() {
        HAYAKU_INFO("addFuncAtTimeEveryDay test delay 1 *************************");
    });
    tm.addFuncAtTimeEveryDay(time + Seconds(45), []() {
        HAYAKU_INFO("addFuncAtTimeEveryDay test delay 2 *************************");
    });
    std::this_thread::sleep_for(std::chrono::seconds(60));
#endif
}

/** @} */
