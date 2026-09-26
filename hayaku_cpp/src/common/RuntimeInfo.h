#pragma once

/*
 * runtimeinfo.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-10-1
 *      Author: fasiondog
 */


#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "Kernel32.lib")
#endif

namespace hayaku {

bool supportChineseSimple();

inline bool supportChineseSimple() {
#ifdef _WIN32
    return 0X0804 == GetUserDefaultUILanguage() ? true : false;
#endif
    return true;
}

} /* namespace hayaku */
