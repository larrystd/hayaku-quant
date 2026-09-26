/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-02
 *      Author: fasiondog
 */

/*************************************************************
 *
 * This example uses hayaku in C++
 * 1. Initialize hayaku
 * 2. Print the K-line data
 * For more usage see python: they are basically the same, only the naming style
 *differs
 *
 *************************************************************/

#include <common/Os.h>
#include <extensions/realtime/GlobalSpotAgent.h>
#include <hayaku.h>

#include <chrono>
#include <thread>

#if defined(_WIN32)
#include <Windows.h>
#endif

using namespace hayaku;

int main(int argc, char* argv[]) {
#if defined(_WIN32)
  // Set the console output code page to UTF8 on Windows
  auto old_cp = GetConsoleOutputCP();
  SetConsoleOutputCP(CP_UTF8);
#endif

  // Modify the config file location yourself
  auto session =
      HayakuSession::open(fmt::format("{}/.hayaku/hayaku.ini", getUserDir()));
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
