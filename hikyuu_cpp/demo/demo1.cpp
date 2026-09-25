/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-02
 *      Author: fasiondog
 */

/*************************************************************
 *
 * This example uses hikyuu in C++
 * 1. Initialize hikyuu
 * 2. Print the K-line data
 * For more usage see python: they are basically the same, only the naming style differs
 *
 *************************************************************/

#include <hikyuu.h>
#include <thread>
#include <chrono>
#include <app/runtime/GlobalSpotAgent.h>
#include <common/os.h>

#if defined(_WIN32)
#include <Windows.h>
#endif

using namespace hku;

int main(int argc, char* argv[]) {
#if defined(_WIN32)
    // Set the console output code page to UTF8 on Windows
    auto old_cp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif

    // Modify the config file location yourself
    auto session = HikyuuSession::open(fmt::format("{}/.hikyuu/hikyuu.ini", getUserDir()));
    Stock stk = session.data().getStock("sh000001");
    std::cout << stk << std::endl;

    auto k = stk.getKData(KQuery(-10));
    std::cout << k << std::endl;

    for (size_t i = 0; i < k.size(); i++) {
        std::cout << k[i] << std::endl;
    }

    // Start the market data receiving (not needed for a backtest only)
    // startSpotAgent(true);
    // while (true) {
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    // }

#if defined(_WIN32)
    SetConsoleOutputCP(old_cp);
#endif
    return 0;
}
