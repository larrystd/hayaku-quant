#pragma once

/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */


#include "Indicator.h"

namespace hayaku {

/**
 * Maximum drawdown percentage (there is no time window limit when n=0); it is a positive value as
 * per the industry convention

 * @note the values less than or equal to 0 and the nan values are not handled

 * @param n time window size

 * @ingroup Indicator
 */
Indicator HAYAKU_API MDD(int n = 0);

/**
 * Maximum drawdown percentage (there is no time window limit when n=0); it is a positive value as
 * per the industry convention

 * @param ind the data to be calculated

 * @param n time window size

 * @ingroup Indicator
 */
inline Indicator MDD(const Indicator& ind, int n = 0) {
    return MDD(n)(ind);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-07-02
 *      Author: fasiondog
 */



namespace hayaku {

/**
 * The drawdown percentage from the current point to the historical highest point; it is a positive
 * value as per the industry convention
 * @note the values less than or equal to 0 and the nan values are not handled, nan is returned at
 *       those positions
 * @ingroup Indicator
 */
Indicator HAYAKU_API MDD_CURRENT();

/**
 * The drawdown percentage from the current point to the historical highest point; it is a positive
 * value as per the industry convention
 * @param ind the data to be calculated
 * @ingroup Indicator
 */
inline Indicator MDD_CURRENT(const Indicator& ind) {
    return MDD_CURRENT()(ind);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-12
 *  Author: fasiondog
 */



namespace hayaku {

/**
 * Original RSRS (underlying β) indicator, based on the rolling N-day OLS regression

 * Formula: High = α + β · Low

 *
 * Every K-line contributes a coordinate point (Low[i], High[i]), and the N points within the rolling
 * window are used for the OLS regression.

 * β is the most original RSRS slope, representing the strength of the support and resistance.

 * Defect: the β center fluctuates greatly in different market ranges, so it cannot be compared
 * directly across the periods.

 *
 * @param n the rolling window, 20 by default

 * @param kdata K-line data

 * @ingroup Indicator
 */
Indicator HAYAKU_API RSRS_BETA(int n = 20);
Indicator HAYAKU_API RSRS_BETA(const KData& kdata, int n = 20);

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-12
 *  Author: fasiondog
 */



namespace hayaku {

Indicator HAYAKU_API RSRS_BULL(int n = 20, int m = 60);
Indicator HAYAKU_API RSRS_BULL(const KData& kdata, int n = 20, int m = 60);

}  // namespace hayaku

/*
 * SAFTYLOSS.h
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */



namespace hayaku {

/**
 * Alexander Elder's safe zone stop-loss
 * @details
 * <pre>
 * See "Come Into My Trading Room" (2007, Earthquake Press) by Alexander Elder, P202
 * Calculation description: within the lookback period (generally 10 to 20 days), add up the lengths
 *         of all the downward crossings and divide by the number of the downward crossings to get
 *         the average noise (i.e. within the lookback period, the length by which every low price
 * is lower than the previous day's low price divided by the number of times), and subtract (the
 * previous day's average noise multiplied by a multiple) from today's low price to get the
 * stop-loss line. To offset the fluctuation and guarantee that the stop-loss line moves upward, the
 * highest value within N days (generally 3 days) is taken based on the above result
 * </pre>
 * @note: the first (lookback period width + the width for taking the highest value) points in the
 *        returned result are invalid
 * @param n1 the lookback time window for calculating the average noise, 10 days by default
 * @param n2 take the highest value within n2 days for the preliminary stop-loss line, 3 by default
 * @param p the noise coefficient, 2 by default
 * @ingroup Indicator
 */
Indicator HAYAKU_API SAFTYLOSS(int n1 = 10, int n2 = 3, double p = 2.0);
Indicator HAYAKU_API SAFTYLOSS(const IndParam& n1, const IndParam& n2, double p = 2.0);
Indicator HAYAKU_API SAFTYLOSS(const IndParam& n1, const IndParam& n2, const IndParam& p);

/**
 * Alexander Elder's safe zone stop-loss
 * @details
 * <pre>
 * See "Come Into My Trading Room" (2007, Earthquake Press) by Alexander Elder, P202
 * Calculation description: within the lookback period (generally 10 to 20 days), add up the lengths
 *         of all the downward crossings and divide by the number of the downward crossings to get
 *         the average noise (i.e. within the lookback period, the length by which every low price
 * is lower than the previous day's low price divided by the number of times), and subtract (the
 * previous day's average noise multiplied by a multiple) from today's low price to get the
 * stop-loss line. To offset the fluctuation and guarantee that the stop-loss line moves upward, the
 * highest value within N days (generally 3 days) is taken based on the above result
 * </pre>
 * @note: the first (lookback period width + the width for taking the highest value) points in the
 *        returned result are invalid
 * @param data the input data, a single input
 * @param n1 the lookback time window for calculating the average noise, 10 days by default
 * @param n2 take the highest value within n2 days for the preliminary stop-loss line, 3 by default
 * @param p the noise coefficient, 2 by default
 * @ingroup Indicator
 */
inline Indicator SAFTYLOSS(const Indicator& data, int n1 = 10, int n2 = 3, double p = 2.0) {
    return SAFTYLOSS(n1, n2, p)(data);
}

inline Indicator SAFTYLOSS(const Indicator& data, const IndParam& n1, const IndParam& n2,
                           double p = 2.0) {
    return SAFTYLOSS(n1, n2, p)(data);
}

inline Indicator SAFTYLOSS(const Indicator& data, const IndParam& n1, const IndParam& n2,
                           const IndParam& p) {
    return SAFTYLOSS(n1, n2, p)(data);
}

inline Indicator SAFTYLOSS(const Indicator& data, const Indicator& n1, const Indicator& n2,
                           double p = 2.0) {
    return SAFTYLOSS(IndParam(n1), IndParam(n2), p)(data);
}

inline Indicator SAFTYLOSS(const Indicator& data, const Indicator& n1, const Indicator& n2,
                           const Indicator& p) {
    return SAFTYLOSS(IndParam(n1), IndParam(n2), IndParam(p))(data);
}

}  // namespace hayaku
