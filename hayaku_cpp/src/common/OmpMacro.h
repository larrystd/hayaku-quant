#pragma once

/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-01-16
 *      Author: fasiondog
 */


#include "common/Config.h"

#if defined(_OPENMP)
#include <omp.h>
#define HAYAKU_OMP_PARALLEL_FOR _Pragma("omp parallel for")
#define HAYAKU_OMP_SAFETY_PARALLEL_FOR \
    _Pragma("omp parallel for num_threads(omp_get_max_threads()) if (!omp_in_parallel())")
#define HAYAKU_OMP_CHECK_THRESHOLD(guard, threshold)      \
    if ((guard) > (threshold) && !omp_in_parallel()) { \
        omp_set_num_threads(omp_get_max_threads());    \
    }
#else
#define HAYAKU_OMP_PARALLEL_FOR
#define HAYAKU_OMP_SAFETY_PARALLEL_FOR
#define HAYAKU_OMP_CHECK_THRESHOLD(guard, threshold)
#endif
