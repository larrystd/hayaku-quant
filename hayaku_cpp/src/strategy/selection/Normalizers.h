#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#include "NormalizeBase.h"

namespace hayaku {

/**
 * @ingroup MultiFactor
 * @{
 */

inline NormPtr NORM_NOTHING() { return NormPtr(); }

/** Min-max standardization */
NormPtr HAYAKU_API NORM_MinMax();

/** Normal standardization */
NormPtr HAYAKU_API NORM_Zscore(bool outExtreme = false, double nsigma = 3.0,
                               bool recursive = false);

/** Quantile distribution standardization */
NormPtr HAYAKU_API NORM_Quantile(double quantile_min = 0.01,
                                 double quantile_max = 0.99);

/** Quantile uniform distribution standardization */
NormPtr HAYAKU_API NORM_Quantile_Uniform(double quantile_min = 0.01,
                                         double quantile_max = 0.99);

/* @} */
}  // namespace hayaku
