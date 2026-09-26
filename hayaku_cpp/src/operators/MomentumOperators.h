#pragma once

#include "Indicator.h"

namespace hayaku {

/**
 * ADX average directional index
 * @details
 * <pre>
 * ADX (Average Directional Index) is a trend strength indicator, it does not
 * distinguish the rising or falling direction, it only judges whether there is
 * a trend.
 *
 * The original formula of Wilder is used, with the period N = 14:
 *
 * 1. TR true range (the maximum of the three choices):
 *    TR = max(H-L, |H-C_prev|, |L-C_prev|)
 *
 * 2. DM directional movement:
 *    +DM = H - H_prev (if H > H_prev and the upward move > the downward move),
 * otherwise 0 -DM = L_prev - L (if L < L_prev and the downward move > the
 * upward move), otherwise 0
 *
 * 3. Wilder smoothing (a simple average of N periods at the beginning,
 * recursive afterwards): ATR_t = ATR_{t-1} × (N-1)/N + TR_t/N S+DM_t =
 * S+DM_{t-1} × (N-1)/N + +DM_t/N S-DM_t = S-DM_{t-1} × (N-1)/N + -DM_t/N
 *
 * 4. ±DI directional indicator (percentage 0~100):
 *    +DI = (S+DM / ATR) × 100
 *    -DI = (S-DM / ATR) × 100
 *
 * 5. DX directional indicator:
 *    DX = |+DI - (-DI)| / (+DI + -DI) × 100
 *
 * 6. ADX average directional index (DX is smoothed by Wilder once more):
 *    ADX_t = ADX_{t-1} × (N-1)/N + DX_t/N
 *
 * Result set:
 * - 0: ADX itself (trend strength, value range 0~100)
 * - 1: +DI (upward directional line, bull power)
 * - 2: -DI (downward directional line, bear power)
 *
 * Judgment criteria:
 * - ADX >= 25: a clear one-sided trend exists (either rising or falling)
 * - ADX < 25: no trend, range-bound oscillation
 * - The larger the ADX value, the stronger the trend
 * </pre>
 * @param n calculation period, 14 by default
 * @ingroup Indicator
 */
Indicator HAYAKU_API ADX(int n = 14);

/**
 * ADX average directional index
 * @param kdata the source data to be calculated
 * @param n calculation period, 14 by default
 * @ingroup Indicator
 */
Indicator HAYAKU_API ADX(const KData& kdata, int n = 14);

}  // namespace hayaku

#include <operators/Indicator.h>

namespace hayaku {

/**
 * @brief Average directional index (ADX2) - using the EMA smoothing way
 *
 * ADX2 is a trend strength indicator, it does not distinguish the rising or
 * falling direction, it only judges whether there is a trend. Unlike ADX, it
 * uses EMA (exponential moving average) instead of the Wilder smoothing.
 *
 * Result set:
 * - result(0): ADX itself (trend strength, value range 0~100)
 * - result(1): +DI (upward directional line, bull power)
 * - result(2): -DI (downward directional line, bear power)
 *
 * Judgment criteria:
 * - ADX >= 25: a clear one-sided trend exists (either rising or falling)
 * - ADX < 25: no trend, range-bound oscillation
 * - The larger the ADX value, the stronger the trend
 *
 * @param kdata the source data to be calculated
 * @param n calculation period, 14 by default, it must be an integer greater
 * than 1
 * @return the Indicator with three result sets
 */
Indicator HAYAKU_API ADX2(const KData& kdata, int n = 14);

/**
 * @brief Average directional index (ADX2) - using the EMA smoothing way
 *
 * Create an ADX2 indicator calculator, the context needs to be set through
 * setContext
 *
 * @param n calculation period, 14 by default, it must be an integer greater
 * than 1
 * @return the ADX2 indicator calculator
 */
Indicator HAYAKU_API ADX2(int n = 14);

}  // namespace hayaku

/*
 * ATR.h
 *
 *  Created on: 2016-5-4
 *      Author: Administrator
 */

namespace hayaku {

/**
 * Average True Range (ATR), the simple average of TR
 * @param n the period window for calculating the average, it must be an integer
 * greater than 1
 * @ingroup Indicator
 */
Indicator HAYAKU_API ATR(int n = 14);

/**
 * Average True Range (ATR)
 * @param kdata the source data to be calculated
 * @param n the period window for calculating the average, it must be an integer
 * greater than 1
 * @ingroup Indicator
 */
Indicator HAYAKU_API ATR(const KData& kdata, int n = 14);

}  // namespace hayaku

/*
 * DIFF.h
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Difference indicator, i.e. data[i] - data[i-n]
 * @param n difference period, 1 by default
 * @ingroup Indicator
 */
Indicator HAYAKU_API DIFF(int n = 1);

/**
 * Difference indicator, i.e. data[i] - data[i-n]
 * @param data the data to be calculated
 * @param n difference period, 1 by default
 * @ingroup Indicator
 */
Indicator HAYAKU_API DIFF(const Indicator& data, int n = 1);

}  // namespace hayaku

/*
 * MACD.h
 *
 *  Created on: 2013-4-11
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * MACD moving average convergence / divergence
 * @param n1 short-term EMA time window, 12 by default
 * @param n2 long-term EMA time window, 26 by default
 * @param n3 EMA smoothing time window of (short-term EMA - long-term EMA), 9 by
 * default
 * @return
 * <pre>
 * MACD BAR: MACD histogram, i.e. MACD fast line - MACD slow line
 * DIFF: fast line, i.e. (short-term EMA - long-term EMA)
 * DEA: slow line, i.e. the n3-period EMA smoothing of the fast line
 * </pre>
 * @ingroup Indicator
 */
Indicator HAYAKU_API MACD(int n1 = 12, int n2 = 26, int n3 = 9);
Indicator HAYAKU_API MACD(const IndParam& n1, const IndParam& n2,
                          const IndParam& n3);

/**
 * MACD moving average convergence / divergence
 * @param data the data to be calculated
 * @param n1 short-term EMA time window, 12 by default
 * @param n2 long-term EMA time window, 26 by default
 * @param n3 EMA smoothing time window of (short-term EMA - long-term EMA), 9 by
 * default
 * @return
 * <pre>
 * MACD BAR: MACD histogram, i.e. MACD fast line - MACD slow line
 * DIFF: fast line, i.e. (short-term EMA - long-term EMA)
 * DEA: slow line, i.e. the n3-period EMA smoothing of the fast line
 * </pre>
 * @ingroup Indicator
 */
inline Indicator MACD(const Indicator& data, int n1 = 12, int n2 = 26,
                      int n3 = 9) {
  return MACD(n1, n2, n3)(data);
}

inline Indicator MACD(const Indicator& data, const IndParam& n1,
                      const IndParam& n2, const IndParam& n3) {
  return MACD(n1, n2, n3)(data);
}

inline Indicator MACD(const Indicator& data, const Indicator& n1,
                      const Indicator& n2, const Indicator& n3) {
  return MACD(IndParam(n1), IndParam(n2), IndParam(n3))(data);
}

}  // namespace hayaku

/*
 * TRG.h
 *
 *  Created on: 2019-3-6

 *      Author: fasiondog
 */

namespace hayaku {

/**
 * @brief True range (TR)

 * @details
 * <pre>
 * The true range (TR) is the maximum of the following three values:

 *  1. the difference between the high price (H) and the low price (L) of the
 current period

 *  2. the absolute value of the difference between the high price of the
 current period and the
 *     close price (PC) of the previous period

 *  3. the absolute value of the difference between the low price of the current
 period and the close
 *     price of the previous period

 * </pre>
 * @ingroup Indicator
 */
Indicator HAYAKU_API TR();
Indicator HAYAKU_API TR(const KData&);

}  // namespace hayaku

/*
 * ROC.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Rate of change indicator ((price / prevPrice)-1)*100
 * @ingroup Indicator
 */
Indicator HAYAKU_API ROC(int n = 10);
Indicator HAYAKU_API ROC(const IndParam& n);

inline Indicator ROC(const Indicator& ind, int n = 10) { return ROC(n)(ind); }

inline Indicator ROC(const Indicator& ind, const IndParam& n) {
  return ROC(n)(ind);
}

inline Indicator ROC(const Indicator& ind, const Indicator& n) {
  return ROC(IndParam(n))(ind);
}

}  // namespace hayaku

/*
 * ROCP.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Rate of change indicator (price - prePrice) / prevPrice, the N-day return
 * (the profit beyond the principal)
 * @ingroup Indicator
 */
Indicator HAYAKU_API ROCP(int n = 10);
Indicator HAYAKU_API ROCP(const IndParam& n);

inline Indicator ROCP(const Indicator& ind, int n = 10) { return ROCP(n)(ind); }

inline Indicator ROCP(const Indicator& ind, const IndParam& n) {
  return ROCP(n)(ind);
}

inline Indicator ROCP(const Indicator& ind, const Indicator& n) {
  return ROCP(IndParam(n))(ind);
}

}  // namespace hayaku

/*
 * ROCR.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Rate of change indicator (price / prevPrice), the N-day cumulative return
 * (including the principal)
 * @ingroup Indicator
 */
Indicator HAYAKU_API ROCR(int n = 10);
Indicator HAYAKU_API ROCR(const IndParam& n);

inline Indicator ROCR(const Indicator& ind, int n = 10) { return ROCR(n)(ind); }

inline Indicator ROCR(const Indicator& ind, const IndParam& n) {
  return ROCR(n)(ind);
}

inline Indicator ROCR(const Indicator& ind, const Indicator& n) {
  return ROCR(IndParam(n))(ind);
}

}  // namespace hayaku

/*
 * ROCR100.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Rate of change indicator (price / prevPrice) * 100
 * @ingroup Indicator
 */
Indicator HAYAKU_API ROCR100(int n = 10);
Indicator HAYAKU_API ROCR100(const IndParam& n);

inline Indicator ROCR100(const Indicator& ind, int n = 10) {
  return ROCR100(n)(ind);
}

inline Indicator ROCR100(const Indicator& ind, const IndParam& n) {
  return ROCR100(n)(ind);
}

inline Indicator ROCR100(const Indicator& ind, const Indicator& n) {
  return ROCR100(IndParam(n))(ind);
}

}  // namespace hayaku

/*
 * RSI.h
 *
 *   Created on: 2023-09-23
 *       Author: yangrq1018
 */

#include "SeriesOperators.h"
#include "WindowOperators.h"

namespace hayaku {

/**
 * Relative strength index
 * @ingroup Indicator
 */
Indicator HAYAKU_API RSI(int n = 14);
Indicator HAYAKU_API RSI(const Indicator& data, int n = 14);

}  // namespace hayaku

/*
 * VIGOR.h
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Alexander Elder's force index

 * @details
 * <pre>
 * See "Come Into My Trading Room" (2007, Earthquake Press) (Alexander Elder)
 P131

 * Calculation formula: (today's close price - yesterday's close price) *
 today's volume

 * EMA or MA can generally be used for the smoothing afterwards

 * </pre>
 * @param kdata the K-line data to be calculated

 * @param n EMA smoothing window, it must be greater than or equal to 1

 * @ingroup Indicator
 */
Indicator HAYAKU_API VIGOR(const KData& kdata, int n = 2);

Indicator HAYAKU_API VIGOR(int n = 2);

}  // namespace hayaku
