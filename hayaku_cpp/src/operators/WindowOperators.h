#pragma once

/*
 * BARSCOUNT.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-12
 *      Author: fasiondog
 */

#include "Indicator.h"

namespace hayaku {

/**
 * Number of the valid value periods; gives the total number of the periods.
 * @details
 * <pre>
 * Usage: BARSCOUNT(X) gives the number of days from the first valid data until
 * now. For example: for the daily line data BARSCOUNT(CLOSE) gets the total
 * number of the trading days since the listing, and for the 1-minute line it
 * gets the number of the trading minutes of the day
 * </pre>
 * @ingroup Indicator
 */
Indicator BARSCOUNT();

inline Indicator BARSCOUNT(const Indicator& ind) { return BARSCOUNT()(ind); }

}  // namespace hayaku

/*
 * BARSLAST.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#include "SeriesOperators.h"

namespace hayaku {

/**
 * Position where the condition held last time; the number of periods from the
 * last time the condition held to the current one.
 * @details
 * <pre>
 * Usage: BARSLAST(X): the number of days from the last time X was not 0 until
 * now. For example: BARSLAST(CLOSE/REF(CLOSE,1)>=1.1) gives the number of
 * periods from the last limit-up until now.
 * </pre>
 * @ingroup Indicator
 */
Indicator BARSLAST();

inline Indicator BARSLAST(const Indicator& ind) { return BARSLAST()(ind); }

inline Indicator BARSLAST(Indicator::value_t val) {
  return BARSLAST(CVAL(val));
}

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-06-01
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Count the number of the consecutive periods satisfying the condition
 * @details
 * <pre>
 * Usage: BARSLASTCOUNT(X), where X is a condition expression.
 * For example: BARSLASTCOUNT(CLOSE>OPEN) counts the number of the consecutive
 * periods closing up
 * </pre>
 * @ingroup Indicator
 */
Indicator BARSLASTCOUNT();

inline Indicator BARSLASTCOUNT(const Indicator& ind) {
  return BARSLASTCOUNT()(ind);
}

}  // namespace hayaku

/*
 * BARSLASTS.h
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-17
 *      Author: hayaku
 */

namespace hayaku {

/**
 * The number of periods from the position where the condition holds for the
 * N-th time to the current one.
 * @details
 * <pre>
 * Usage: BARSLASTS(X, N): the number of days from the N-th time X is not 0
 * until now. For example: BARSLASTS(CLOSE/REF(CLOSE,1)>=1.1, 2) gives the
 * number of periods from the second limit-up until now. Note: when N=1,
 * BARSLASTS(X, 1) is equivalent to BARSLAST(X).
 * </pre>
 * @param n the N-th time the condition holds, n is a positive integer
 * @ingroup Indicator
 */
Indicator BARSLASTS(int n);
Indicator BARSLASTS(const IndParam& n);

inline Indicator BARSLASTS(const Indicator& ind, int n) {
  return BARSLASTS(n)(ind);
}

inline Indicator BARSLASTS(const Indicator& ind, const IndParam& n) {
  return BARSLASTS(n)(ind);
}

inline Indicator BARSLASTS(const Indicator& ind, const Indicator& n) {
  return BARSLASTS(IndParam(n))(ind);
}

inline Indicator BARSLASTS(Indicator::value_t val, int n) {
  return BARSLASTS(CVAL(val), n);
}

inline Indicator BARSLASTS(Indicator::value_t val, const IndParam& n) {
  return BARSLASTS(CVAL(val), n);
}

inline Indicator BARSLASTS(Indicator::value_t val, const Indicator& n) {
  return BARSLASTS(CVAL(val), IndParam(n));
}

}  // namespace hayaku

/*
 * BARSSINCE.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * The number of periods from the position where the condition first holds to
 the current one.

 * @details
 * <pre>
 * Usage: BARSSINCE(X): the number of days from the first time X is not 0 until
 now.

 * For example: BARSSINCE(HIGH>10) gives the number of periods from the time the
 stock price exceeds
 * 10 yuan until now

 * </pre>
 * @ingroup Indicator
 */
Indicator BARSSINCE();

inline Indicator BARSSINCE(const Indicator& ind) { return BARSSINCE()(ind); }

inline Indicator BARSSINCE(Indicator::value_t val) {
  return BARSSINCE(CVAL(val));
}

/**
 * The position where the condition first holds within N periods

 * @details The number of periods from the first time the condition holds within
 N periods until now

 * <pre>
 * Usage: BARSSINCEN(X,N): the number of periods from the first time X is not 0
 within N periods
 * until now, N is a constant BARSSINCEN(X,N):

 * For example: BARSSINCEN(HIGH>10,10) gives the number of periods from the time
 the stock price
 * exceeds 10 yuan within 10 periods until now

 * </pre>
 * @ingroup Indicator
 */
Indicator BARSSINCEN(int n);
inline Indicator BARSSINCEN(const Indicator& ind, int n) {
  return BARSSINCEN(n)(ind);
}

}  // namespace hayaku

/*
 * LAST.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-28
 *      Author: fasiondog
 */

#include "BooleanOperators.h"

namespace hayaku {

#if defined(MAX)
#undef MAX
#endif
#if defined(MIN)
#undef MIN
#endif

inline Indicator MAX(const Indicator& ind1, const Indicator& ind2);
inline Indicator MIN(const Indicator& ind1, const Indicator& ind2);
inline Indicator MA(const Indicator& ind, int n);
inline Indicator MA(const Indicator& ind, const IndParam& n);
inline Indicator MA(const Indicator& ind, const Indicator& n);

/**
 * Existence in an interval
 * @details
 * <pre>
 * Usage: LAST (X,M,N) means the condition X exists from M periods ago to N
 * periods ago For example: LAST(CLOSE>OPEN,10,5) means the candles are bullish
 * all the way from 10 days ago to 5 days ago. If A is 0 it means starting from
 * the first day, if B is 0 it means ending at the last day.
 * </pre>
 * @ingroup Indicator
 */
inline Indicator LAST(int m = 10, int n = 5) {
  int max = std::max(m, n);
  int min = std::min(m, n);
  Indicator result = REF(EVERY(max - min + 1), min);
  result.name("LAST");
  return result;
}

inline Indicator LAST(const IndParam& m, int n = 5) {
  Indicator ind_m = m.get();
  Indicator ind_n = CVAL(ind_m, n);
  Indicator max = MAX(ind_m, ind_n);
  Indicator min = MIN(ind_m, ind_n);
  Indicator result = REF(EVERY(max - min + 1), min);
  result.name("LAST");
  return result;
}

inline Indicator LAST(int m, const IndParam& n) {
  Indicator ind_n = n.get();
  Indicator ind_m = CVAL(ind_n, m);
  Indicator max = MAX(ind_m, ind_n);
  Indicator min = MIN(ind_m, ind_n);
  Indicator result = REF(EVERY(max - min + 1), min);
  result.name("LAST");
  return result;
}

inline Indicator LAST(const IndParam& m, const IndParam& n) {
  Indicator ind_m = m.get();
  Indicator ind_n = n.get();
  Indicator max = MAX(ind_m, ind_n);
  Indicator min = MIN(ind_m, ind_n);
  Indicator result = REF(EVERY(max - min + 1), min);
  result.name("LAST");
  return result;
}

inline Indicator LAST(const Indicator& ind, int m = 10, int n = 5) {
  return LAST(m, n)(ind);
}

inline Indicator LAST(const Indicator& ind, const IndParam& m, int n = 5) {
  return LAST(m, n)(ind);
}

inline Indicator LAST(const Indicator& ind, int m, const IndParam& n) {
  return LAST(m, n)(ind);
}

inline Indicator LAST(const Indicator& ind, const IndParam& m,
                      const IndParam& n) {
  return LAST(m, n)(ind);
}

inline Indicator LAST(const Indicator& ind, const Indicator& m, int n = 5) {
  return LAST(IndParam(m), n)(ind);
}

inline Indicator LAST(const Indicator& ind, int m, const Indicator& n) {
  return LAST(m, IndParam(n))(ind);
}

inline Indicator LAST(const Indicator& ind, const Indicator& m,
                      const Indicator& n) {
  return LAST(IndParam(m), IndParam(n))(ind);
}

}  // namespace hayaku

/*
 * SUMBARS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Accumulate to the given number of periods; the number of periods from the
 * accumulation forward to the given value until now
 * @details
 * <pre>
 * Usage: SUMBARS(X,A): accumulate X forward until it is greater than or equal
 * to A, and return the number of periods of this interval For example:
 * SUMBARS(VOL,CAPITAL) gives the number of periods from the full turnover until
 * now
 * </pre>
 * @note discard semantics (there is a difference between the scalar parameter
 * and the sequence parameter):
 *  - Scalar parameter `SUMBARS(ind, double)`: if a position is still < a after
 * accumulating to the leftmost end of the sequence, the whole segment is marked
 * as discard (a static global optimization, because when the scalar a is
 * unreachable monotonically the earlier positions are even more unreachable).
 *  - Sequence parameter `SUMBARS(ind, IndParam)`: NaN is written to every
 * unreachable position, but discard is not advanced (the dynamic a sequence is
 * not monotonic; a[i] being unreachable does not mean a[i+1] is unreachable,
 * and advancing would wipe out the later calculable positions). That is, the
 * dynamic path does not guarantee "all the values after discard are valid". The
 *  downstream should handle it with `std::isnan`,
 *  and should not assume "everything after discard is valid".
 * @ingroup Indicator
 */
Indicator SUMBARS(double a);
Indicator SUMBARS(const IndParam& a);

inline Indicator SUMBARS(const Indicator& ind, double a) {
  return SUMBARS(a)(ind);
}

inline Indicator SUMBARS(const Indicator& ind, const IndParam& a) {
  return SUMBARS(a)(ind);
}

inline Indicator SUMBARS(const Indicator& ind, const Indicator& a) {
  return SUMBARS(IndParam(a))(ind);
}

}  // namespace hayaku

/*
 * HHV.h
 *
 *  Created on: 2016-4-2
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * The highest price within N days; when N=0 it starts from the first valid
 * value
 * @param n N-day time window
 * @ingroup Indicator
 */
Indicator HHV(int n = 20);
Indicator HHV(const IndParam& n);

/**
 * The highest price within N days; when N=0 it starts from the first valid
 * value
 * @param ind the data to be calculated
 * @param n N-day time window
 * @ingroup Indicator
 */
inline Indicator HHV(const Indicator& ind, int n = 20) { return HHV(n)(ind); }

inline Indicator HHV(const Indicator& ind, const IndParam& n) {
  return HHV(n)(ind);
}

inline Indicator HHV(const Indicator& ind, const Indicator& n) {
  return HHV(IndParam(n))(ind);
}

}  // namespace hayaku

/*
 * HHVBARS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Position of the previous high; the number of periods from the previous high
 * to the current one.
 * @details
 * <pre>
 * Usage: HHVBARS(X,N): the number of periods from the highest value of X within
 * N periods until now; N=0 means counting starts from the first valid value For
 * example: HHVBARS(HIGH,0) gives the number of periods from the historical new
 * high until now
 * </pre>
 * @ingroup Indicator
 */
Indicator HHVBARS(int n = 20);
Indicator HHVBARS(const IndParam& n);

inline Indicator HHVBARS(const Indicator& ind, int n = 20) {
  return HHVBARS(n)(ind);
}

inline Indicator HHVBARS(const Indicator& ind, const IndParam& n) {
  return HHVBARS(n)(ind);
}

inline Indicator HHVBARS(const Indicator& ind, const Indicator& n) {
  return HHVBARS(IndParam(n))(ind);
}

}  // namespace hayaku

/*
 * LLV.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2016-4-2
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * The lowest price within N days
 * @param n N-day time window; when N=0 it starts from the first valid value.
 * @ingroup Indicator
 */
Indicator LLV(int n = 20);
Indicator LLV(const IndParam& n);

/**
 * The lowest price within N days; when N=0 it starts from the first valid
 * value.
 * @param ind the data to be calculated
 * @param n N-day time window
 * @ingroup Indicator
 */
inline Indicator LLV(const Indicator& ind, int n = 20) { return LLV(n)(ind); }

inline Indicator LLV(const Indicator& ind, const IndParam& n) {
  return LLV(n)(ind);
}

inline Indicator LLV(const Indicator& ind, const Indicator& n) {
  return LLV(IndParam(n))(ind);
}

}  // namespace hayaku

/*
 * LLVBARS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Position of the previous low; the number of periods from the previous low to
 * the current one.
 * @details
 * <pre>
 * Usage: LLVBARS(X,N): the number of periods from the lowest value of X within
 * N periods until now; N=0 means counting starts from the first valid value For
 * example: LLVBARS(HIGH,20) gives the number of periods from the 20-day lowest
 * point until now
 * </pre>
 * @ingroup Indicator
 */
Indicator LLVBARS(int n = 20);
Indicator LLVBARS(const IndParam& n);

inline Indicator LLVBARS(const Indicator& ind, int n = 20) {
  return LLVBARS(n)(ind);
}

inline Indicator LLVBARS(const Indicator& ind, const IndParam& n) {
  return LLVBARS(n)(ind);
}

inline Indicator LLVBARS(const Indicator& ind, const Indicator& n) {
  return LLVBARS(IndParam(n))(ind);
}

}  // namespace hayaku

/*
 * MAX.h
 *
 *  Created on: 2019-4-8
 *      Author: fasiondog
 */

namespace hayaku {

#if defined(MAX)
#undef MAX
#endif

/**
 * Calculate the maximum
 * @details
 * <pre>
 * Usage: MAX(A,B) returns the larger one of A and B
 * For example: MAX(CLOSE-OPEN,0) returns their difference if the close price is
 * greater than the open price, otherwise it returns 0
 * </pre>
 * @ingroup Indicator
 */
inline Indicator MAX(const Indicator& ind1, const Indicator& ind2) {
  Indicator result = IF(ind1 > ind2, ind1, ind2);
  result.name("MAX");
  return result;
}

inline Indicator MAX(const Indicator& ind, Indicator::value_t val) {
  Indicator result = IF(ind > val, ind, val);
  result.name("MAX");
  return result;
}

inline Indicator MAX(Indicator::value_t val, const Indicator& ind) {
  Indicator result = IF(val > ind, val, ind);
  result.name("MAX");
  return result;
}

}  // namespace hayaku

/*
 * MIN.h
 *
 *  Created on: 2019-4-8
 *      Author: fasiondog
 */

namespace hayaku {

#if defined(MIN)
#undef MIN
#endif

/**
 * Calculate the minimum
 * @details
 * <pre>
 * Usage: MIN(A,B) returns the smaller one of A and B
 * For example: MIN(CLOSE,OPEN) returns the smaller one of the open price and
 * the close price
 * </pre>
 * @ingroup Indicator
 */
inline Indicator MIN(const Indicator& ind1, const Indicator& ind2) {
  Indicator result = IF(ind1 < ind2, ind1, ind2);
  result.name("MIN");
  return result;
}

inline Indicator MIN(const Indicator& ind, Indicator::value_t val) {
  Indicator result = IF(ind < val, ind, val);
  result.name("MIN");
  return result;
}

inline Indicator MIN(Indicator::value_t val, const Indicator& ind) {
  Indicator result = IF(val < ind, val, ind);
  result.name("MIN");
  return result;
}

}  // namespace hayaku

/*
 * COUNT.h
 *
 *  Created on: 2019-3-25
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Count the total number; count the number of the periods satisfying the
 * condition.
 * @details
 * <pre>
 * Usage: COUNT(X,N) counts the number of the periods satisfying the X condition
 * within N periods; if N=0 it starts from the first valid value. For example:
 * COUNT(CLOSE>OPEN,20) counts the number of the periods closing up within 20
 * periods
 * </pre>
 * @param n number of the periods
 * @ingroup Indicator
 */
Indicator COUNT(int n = 20);
Indicator COUNT(const IndParam& n);

/**
 * Count the total number; count the number of the periods satisfying the
 * condition.
 * @details
 * <pre>
 * Usage: COUNT(X,N) counts the number of the periods satisfying the X condition
 * within N periods; if N=0 it starts from the first valid value. For example:
 * COUNT(CLOSE>OPEN,20) counts the number of the periods closing up within 20
 * periods
 * </pre>
 * @param ind the indicator to be counted
 * @param n number of the periods
 * @ingroup Indicator
 */
inline Indicator COUNT(const Indicator& ind, int n = 20) {
  return COUNT(n)(ind);
}

inline Indicator COUNT(const Indicator& ind, const IndParam& n) {
  return COUNT(n)(ind);
}

inline Indicator COUNT(const Indicator& ind, const Indicator& n) {
  return COUNT(IndParam(n))(ind);
}

}  // namespace hayaku

/*
 * SUM.h
 *
 *  Created on: 2019-4-1

 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Calculate the sum. SUM(X,N) sums up X within N periods; when N=0 it starts
 from the first valid
 * value.

 * @param n N-day time window

 * @ingroup Indicator
 */
Indicator SUM(int n = 20);
Indicator SUM(const IndParam& n);

/**
 * Calculate the sum. SUM(X,N) sums up X within N periods; when N=0 it starts
 from the first valid
 * value.

 * @param ind the data to be calculated

 * @param n N-day time window

 * @ingroup Indicator
 */
inline Indicator SUM(const Indicator& ind, int n = 20) { return SUM(n)(ind); }

inline Indicator SUM(const Indicator& ind, const IndParam& n) {
  return SUM(n)(ind);
}

inline Indicator SUM(const Indicator& ind, const Indicator& n) {
  return SUM(IndParam(n))(ind);
}

}  // namespace hayaku

/*
 * AVEDEV.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2015-5-16
 *      Author: fasiondog
 */

#include "ScalarMathOperators.h"

namespace hayaku {

/**
 * Mean absolute deviation, it gives the N-day mean absolute deviation of X
 * @param ind the data to be calculated
 * @param n time window
 * @ingroup Indicator
 */
inline Indicator AVEDEV(const Indicator& ind, int n = 22) {
  Indicator result = ABS(ind - MA(ind, n)) / n;
  result.name("AVEDEV");
  return result;
}

inline Indicator AVEDEV(const Indicator& ind, const Indicator& n) {
  Indicator result = ABS(ind - MA(ind, n)) / n;
  result.name("AVEDEV");
  return result;
}

inline Indicator AVEDEV(const Indicator& ind, const IndParam& n) {
  Indicator result = ABS(ind - MA(ind, n)) / n.get();
  result.name("AVEDEV");
  return result;
}

}  // namespace hayaku

/*
 * DEVSQ.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Sum of squares of the data deviations, it gives the N-day sum of squares of
 * the data deviations of X
 * @param n N-day time window
 * @ingroup Indicator
 */
Indicator DEVSQ(int n = 10);
Indicator DEVSQ(const IndParam& n);

inline Indicator DEVSQ(const Indicator& data, int n = 10) {
  return DEVSQ(n)(data);
}

inline Indicator DEVSQ(const Indicator& data, const IndParam& n) {
  return DEVSQ(n)(data);
}

inline Indicator DEVSQ(const Indicator& data, const Indicator& n) {
  return DEVSQ(IndParam(n))(data);
}

}  // namespace hayaku

/*
 * EMA.h
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Exponential Moving Average (EMA)
 * @param n the period window for calculating the average, it must be an integer
 * greater than 0
 * @ingroup Indicator
 */
Indicator EMA(int n = 22);
Indicator EMA(const IndParam& n);

/**
 * Exponential Moving Average (EMA)
 * @param data the source data to be calculated
 * @param n the period window for calculating the average, it must be an integer
 * greater than 0
 * @ingroup Indicator
 */
inline Indicator EMA(const Indicator& data, int n = 22) { return EMA(n)(data); }

inline Indicator EMA(const Indicator& data, const IndParam& n) {
  return EMA(n)(data);
}

inline Indicator EMA(const Indicator& data, const Indicator& n) {
  return EMA(IndParam(n))(data);
}

}  // namespace hayaku

/*
 * MA.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Simple moving average
 * @param n the period window for calculating the average; when n is 0 the
 * calculation starts from the first valid data
 * @ingroup Indicator
 */
Indicator MA(int n = 22);
Indicator MA(const IndParam& n);

/**
 * Simple moving average
 * @param ind the data to be calculated
 * @param n the period window for calculating the average; when n is 0 the
 * calculation starts from the first valid data
 * @ingroup Indicator
 */
inline Indicator MA(const Indicator& ind, int n = 22) { return MA(n)(ind); }

inline Indicator MA(const Indicator& ind, const IndParam& n) {
  return MA(n)(ind);
}

inline Indicator MA(const Indicator& ind, const Indicator& n) {
  return MA(IndParam(n))(ind);
}

}  // namespace hayaku

/*
 * SMA.h
 *
 *  Created on: 2015-2-16
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Calculate the moving average
 * @details
 * <pre>
 * Usage: if Y=SMA(X,N,M) then Y=[M*X+(N-M)*Y')/N, where Y' is the Y value of
 * the previous period
 * </pre>
 * @param n the period window for calculating the average, it must be an integer
 * greater than 0
 * @param m coefficient
 * @ingroup Indicator
 */
Indicator SMA(int n = 22, double m = 2.0);
Indicator SMA(int, const IndParam& m);
Indicator SMA(const IndParam& n, double m = 2.0);
Indicator SMA(const IndParam& n, const IndParam& m);

/**
 * Calculate the moving average
 * @details
 * <pre>
 * Usage: if Y=SMA(X,N,M) then Y=[M*X+(N-M)*Y')/N, where Y' is the Y value of
 * the previous period
 * </pre>
 * @param ind the data to be calculated
 * @param n the period window for calculating the average, it must be an integer
 * greater than 0
 * @param m coefficient
 * @ingroup Indicator
 */
inline Indicator SMA(const Indicator& ind, int n = 22, double m = 2.0) {
  return SMA(n, m)(ind);
}

inline Indicator SMA(const Indicator& ind, int n, const IndParam& m) {
  return SMA(n, m)(ind);
}

inline Indicator SMA(const Indicator& ind, const IndParam& n, double m = 2.0) {
  return SMA(n, m)(ind);
}

inline Indicator SMA(const Indicator& ind, const IndParam& n,
                     const IndParam& m) {
  return SMA(n, m)(ind);
}

inline Indicator SMA(const Indicator& ind, int n, const Indicator& m) {
  return SMA(n, IndParam(m))(ind);
}

inline Indicator SMA(const Indicator& ind, const Indicator& n, double m = 2.0) {
  return SMA(IndParam(n), m)(ind);
}

inline Indicator SMA(const Indicator& ind, const Indicator& n,
                     const Indicator& m) {
  return SMA(IndParam(n), IndParam(m))(ind);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-15
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Weighted moving average

 * Algorithm: Yn=(1*X1+2*X2+...+n*Xn)/(1+2+...+n)

 * @param n the period window for calculating the average, n >= 1

 * @ingroup Indicator
 */
Indicator WMA(int n = 22);
Indicator WMA(const IndParam& n);

/**
 * Weighted moving average

 * Algorithm: Yn=(1*X1+2*X2+...+n*Xn)/(1+2+...+n)

 * @param ind the data to be calculated

 * @param n the period window for calculating the average

 * @ingroup Indicator
 */
inline Indicator WMA(const Indicator& ind, int n = 22) { return WMA(n)(ind); }

inline Indicator WMA(const Indicator& ind, const IndParam& n) {
  return WMA(n)(ind);
}

inline Indicator WMA(const Indicator& ind, const Indicator& n) {
  return WMA(IndParam(n))(ind);
}

}  // namespace hayaku

/*
 * AMA.h
 *
 *  Created on: 2013-4-8
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Perry J. Kaufman adaptive moving average, see "Smarter Trading" (2006,
 * Guangdong Economy Publishing House)
 * @param n the period window for calculating the average, it must be an integer
 * greater than 2, 10 days by default
 * @param fast_n the corresponding fast period N, 2 by default
 * @param slow_n the N value of the corresponding slow EMA line, 30 by default;
 * the indicator converges when it exceeds about 60 and there is not much
 * influence
 * @return it has 2 result sets, result(0) is AMA and result(1) is ER
 * @ingroup Indicator
 */
Indicator AMA(int n = 10, int fast_n = 2, int slow_n = 30);
Indicator AMA(int n, int fast_n, const IndParam& slow_n);
Indicator AMA(int n, const IndParam& fast_n, int slow_n = 30);
Indicator AMA(int n, const IndParam& fast_n, const IndParam& slow_n);

Indicator AMA(const IndParam& n, int fast_n = 2, int slow_n = 30);
Indicator AMA(const IndParam& n, int fast_n, const IndParam& slow_n);
Indicator AMA(const IndParam& n, const IndParam& fast_n, int slow_n = 30);
Indicator AMA(const IndParam& n, const IndParam& fast_n,
              const IndParam& slow_n);

/**
 * Perry J. Kaufman adaptive moving average, see "Smarter Trading" (2006,
 * Guangdong Economy Publishing House)
 * @param ind the data to be calculated
 * @param n the period window for calculating the average, it must be an integer
 * greater than 2, 10 days by default
 * @param fast_n the corresponding fast period N, 2 by default
 * @param slow_n the N value of the corresponding slow EMA line, 30 by default;
 * the indicator converges when it exceeds about 60 and there is not much
 * influence
 * @ingroup Indicator
 */
inline Indicator AMA(const Indicator& ind, int n = 10, int fast_n = 2,
                     int slow_n = 30) {
  return AMA(n, fast_n, slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, int n, const IndParam& fast_n,
                     int slow_n = 30) {
  return AMA(n, fast_n, slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, int n, int fast_n,
                     const IndParam& slow_n) {
  return AMA(n, fast_n, slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, int n, const IndParam& fast_n,
                     const IndParam& slow_n) {
  return AMA(n, fast_n, slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, const IndParam& n, int fast_n = 2,
                     int slow_n = 30) {
  return AMA(n, fast_n, slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, const IndParam& n,
                     const IndParam& fast_n, int slow_n = 30) {
  return AMA(n, fast_n, slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, const IndParam& n, int fast_n,
                     const IndParam& slow_n) {
  return AMA(n, fast_n, slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, const IndParam& n,
                     const IndParam& fast_n, const IndParam& slow_n) {
  return AMA(n, fast_n, slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, int n, const Indicator& fast_n,
                     int slow_n) {
  return AMA(n, IndParam(fast_n), slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, int n, int fast_n,
                     const Indicator& slow_n) {
  return AMA(n, fast_n, IndParam(slow_n))(ind);
}

inline Indicator AMA(const Indicator& ind, int n, const Indicator& fast_n,
                     const Indicator& slow_n) {
  return AMA(n, IndParam(fast_n), IndParam(slow_n))(ind);
}

inline Indicator AMA(const Indicator& ind, const Indicator& n, int fast_n = 2,
                     int slow_n = 30) {
  return AMA(IndParam(n), fast_n, slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, const Indicator& n,
                     const Indicator& fast_n, int slow_n = 30) {
  return AMA(IndParam(n), IndParam(fast_n), slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, const Indicator& n, int fast_n,
                     const Indicator& slow_n) {
  return AMA(IndParam(n), fast_n, IndParam(slow_n))(ind);
}

inline Indicator AMA(const Indicator& ind, const Indicator& n,
                     const Indicator& fast_n, const Indicator& slow_n) {
  return AMA(IndParam(n), IndParam(fast_n), IndParam(slow_n))(ind);
}

}  // namespace hayaku

/*
 * DMA.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2015-5-16
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Dynamic moving average

 * @details
 * <pre>
 * Usage: DMA(X,A) gives the dynamic moving average of X.

 * Algorithm: if Y=DMA(X,A) then Y=A*X+(1-A)*Y', where Y' is the Y value of the
 previous period.

 * For example: DMA(CLOSE,VOL/CAPITAL) gives the average price with the turnover
 rate as the
 * smoothing factor

 * </pre>
 * @param x the data to be calculated

 * @param a dynamic coefficient

 * @param fill_null fill the missing data with nan when the dates are aligned

 * @ingroup Indicator
 */
Indicator DMA(const Indicator& x, const Indicator& a, bool fill_null = true);

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-02
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Kalman filter
 * @param q noise variance
 * @param r measurement noise variance
 * @ingroup Indicator
 */
Indicator KALMAN(double q = 0.01, double r = 0.1);

inline Indicator KALMAN(const Indicator& ind, double q = 0.01, double r = 0.1) {
  return KALMAN(q, r)(ind);
}

}  // namespace hayaku
