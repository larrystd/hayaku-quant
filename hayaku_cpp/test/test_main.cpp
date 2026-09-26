/*
 * Copyright(C) hikyuu.org
 *
 *  Created on: 2010-5-26
 *      Author: fasiondog
 */

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"

#include <config.h>

#if defined(_WIN32)
#include <Windows.h>
#endif

#include <hayaku.h>
#include <application/PluginRuntime.h>
#include <data/DataRuntime.h>
#include <common/Os.h>
#include <common/Debug.h>
using namespace hayaku;

namespace {
std::unique_ptr<HayakuSession> g_testSession;
}

#ifdef HAYAKU_USE_REAL_DATA_TEST
void init_hayaku_test() {
    set_log_level(LOG_LEVEL::LOG_TRACE);
    std::string config_file(fmt::format("{}/.hayaku/hayaku.ini", getUserDir()));
    fmt::print("configure file: {}\n", config_file);
    g_testSession = std::make_unique<HayakuSession>(HayakuSession::open(config_file));
    auto& data_runtime = getDataRuntime();
    createDir(data_runtime.tmpdir());
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
#else
void init_hayaku_test() {
    set_log_level(LOG_LEVEL::LOG_TRACE);

    auto current = fmt::format("{}/test_data", getCurrentDir());
    fmt::print("current path: {}\n", current);

#if HAYAKU_OS_WINDOWS
    std::string config_file(fmt::format("{}\\hayaku_win.ini", current));
#else
    std::string config_file(fmt::format("{}/hayaku_linux.ini", current));
#endif

    auto plugin_path = fmt::format("{}/plugin", getCurrentDir());
    setPluginPath(plugin_path);

#if !HAYAKU_ENABLE_HDF5_KDATA
    fmt::print("HDF5 is disabled; running the backend-independent small-test subset.\n");
    return;
#endif

    fmt::print("configure file: {}\n", config_file);
    g_testSession = std::make_unique<HayakuSession>(HayakuSession::open(config_file));
    fmt::print("current plugin path: {}\n", getPluginPath());

    std::string tmp_dir(fmt::format("{}/tmp", current));
    createDir(tmp_dir);
}
#endif

int main(int argc, char** argv) {
#if defined(_WIN32)
    // Set the console output code page to UTF8 on Windows
    auto old_cp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif

    doctest::Context context;

    // !!! THIS IS JUST AN EXAMPLE SHOWING HOW DEFAULTS/OVERRIDES ARE SET !!!

    // defaults
    // context.addFilter("test-case-exclude", "*math*");  // exclude test cases with "math" in their
    // name
    context.setOption("abort-after", 5);    // stop test execution after 5 failed assertions
    context.setOption("order-by", "name");  // sort the test cases by their name

    context.applyCommandLine(argc, argv);

    // overrides
    context.setOption("no-breaks", true);  // don't break in the debugger when assertions fail

    init_hayaku_test();
    HAYAKU_INFO("total memory: {}", getMemoryMaxSize());
    HAYAKU_INFO("idle memory: {}", getMemoryIdleSize());

    int res = 0;
    {
        SPEND_TIME_MSG(total_test_run, "Total test time");
        res = context.run();  // run
        std::cout << std::endl;
    }

    if (context.shouldExit())  // important - query flags (and --exit) rely on the user doing this
        return res;            // propagate the result of the tests

    int client_stuff_return_code = 0;
    // your program - if the testing framework is integrated in your production code

#if defined(_WIN32)
    SetConsoleOutputCP(old_cp);
#endif

    return res + client_stuff_return_code;  // the result from doctest is propagated here as well
}
