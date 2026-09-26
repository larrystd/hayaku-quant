#pragma once

/**
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2021/05/19
 *      Author: fasiondog
 */

#if defined(__clang__) || defined(__GNUC__)
#define CPP_STANDARD __cplusplus

#elif defined(_MSC_VER)
#define CPP_STANDARD _MSVC_LANG
#endif

#define CPP_STANDARD_03 199711L
#define CPP_STANDARD_11 201103L
#define CPP_STANDARD_14 201402L
#define CPP_STANDARD_17 201703L
#define CPP_STANDARD_20 202002L
#define CPP_STANDARD_23 202302L
