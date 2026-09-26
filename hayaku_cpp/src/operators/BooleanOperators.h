#pragma once

// ---- Merged from BETWEEN.h ----
/*
 * BETWEEN.h
 *
 *  Created on: 2019-4-8
 *      Author: fasiondog
 */
#include "SeriesOperators.h"

namespace hayaku {

/**
 * Between (between two numbers)
 * @details
 * <pre>
 * Usage: BETWEEN(A,B,C) returns 1 when A is between B and C, otherwise it
 * returns 0 For example: BETWEEN(CLOSE,MA(CLOSE,10),MA(CLOSE,5)) means the
 * close price is between the 5-day moving average and the 10-day moving average
 * @ingroup Indicator
 * </pre>
 */
Indicator BETWEEN(const Indicator&, const Indicator&, const Indicator&);
Indicator BETWEEN(const Indicator&, const Indicator&, Indicator::value_t);
Indicator BETWEEN(const Indicator&, Indicator::value_t, const Indicator&);
Indicator BETWEEN(const Indicator&, Indicator::value_t, Indicator::value_t);
Indicator BETWEEN(Indicator::value_t, const Indicator&, const Indicator&);
Indicator BETWEEN(Indicator::value_t, const Indicator&, Indicator::value_t);
Indicator BETWEEN(Indicator::value_t, Indicator::value_t, const Indicator&);
Indicator BETWEEN(Indicator::value_t, Indicator::value_t, Indicator::value_t);

inline Indicator BETWEEN(const Indicator& a, const Indicator& b,
                         const Indicator& c) {
  Indicator result =
      IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
  result.name("BETWEEN");
  return result;
}

inline Indicator BETWEEN(const Indicator& a, const Indicator& b,
                         Indicator::value_t c) {
  Indicator result =
      IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
  result.name("BETWEEN");
  return result;
}

inline Indicator BETWEEN(const Indicator& a, Indicator::value_t b,
                         const Indicator& c) {
  Indicator result =
      IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
  result.name("BETWEEN");
  return result;
}

inline Indicator BETWEEN(const Indicator& a, Indicator::value_t b,
                         Indicator::value_t c) {
  Indicator result =
      IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
  result.name("BETWEEN");
  return result;
}

inline Indicator BETWEEN(Indicator::value_t a, const Indicator& b,
                         const Indicator& c) {
  Indicator result =
      IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
  result.name("BETWEEN");
  return result;
}

inline Indicator BETWEEN(Indicator::value_t a, const Indicator& b,
                         Indicator::value_t c) {
  Indicator result =
      IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
  result.name("BETWEEN");
  return result;
}

inline Indicator BETWEEN(Indicator::value_t a, Indicator::value_t b,
                         const Indicator& c) {
  Indicator result =
      IF(((b > c) & (a < b) & (a > c)) | ((b < c) & (a > b) & (a < c)), 1, 0);
  result.name("BETWEEN");
  return result;
}

inline Indicator BETWEEN(Indicator::value_t a, Indicator::value_t b,
                         Indicator::value_t c) {
  Indicator result = CVAL(
      (((b > c) && (a < b) && (a > c)) || ((b < c) && (a > b) && (a < c))) ? 1
                                                                           : 0);
  result.name("BETWEEN");
  return result;
}

}  // namespace hayaku

// ---- Merged from EVERY.h ----
/*
 * EVERY.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-28
 *      Author: fasiondog
 */
#include "Indicator.h"

namespace hayaku {

/**
 * Always existing
 * @details
 * <pre>
 * Usage: EVERY (X,N) means the condition X always exists within N periods
 * For example: EVERY(CLOSE>OPEN,10) means the candles are bullish all the way
 * within the previous 10 days
 * </pre>
 * @ingroup Indicator
 */
Indicator HAYAKU_API EVERY(int n = 20);
Indicator HAYAKU_API EVERY(const IndParam& n);

inline Indicator EVERY(const Indicator& ind, int n = 20) {
  return EVERY(n)(ind);
}

inline Indicator EVERY(const Indicator& ind, const IndParam& n) {
  return EVERY(n)(ind);
}

inline Indicator EVERY(const Indicator& ind, const Indicator& n) {
  return EVERY(IndParam(n))(ind);
}

}  // namespace hayaku

// ---- Merged from EXIST.h ----
/*
 * EXIST.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-19
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Existence
 * @details
 * <pre>
 * Usage: EXIST(X,N) means the condition X exists within N periods
 * For example: EXIST(C>O,10) means there are bullish candles within the
 * previous 10 days
 * </pre>
 * @ingroup Indicator
 */
Indicator HAYAKU_API EXIST(int n = 20);
Indicator HAYAKU_API EXIST(const IndParam& n);

inline Indicator EXIST(const Indicator& ind, int n = 20) {
  return EXIST(n)(ind);
}

inline Indicator EXIST(const Indicator& ind, const IndParam& n) {
  return EXIST(n)(ind);
}

inline Indicator EXIST(const Indicator& ind, const Indicator& n) {
  return EXIST(IndParam(n))(ind);
}

}  // namespace hayaku

// ---- Merged from FILTER.h ----
/*
 * FILTER.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Signal filtering, it filters the signals that appear consecutively.
 * @details
 * <pre>
 * Usage: FILTER(X,N): after X satisfies the condition, the data within the
 * following N periods is deleted and set to 0 For example: FILTER(CLOSE>OPEN,5)
 * finds the bullish candles, and the bullish candles appearing again within 5
 * days are not recorded.
 * </pre>
 * @ingroup Indicator
 */
Indicator HAYAKU_API FILTER(int n = 5);
Indicator HAYAKU_API FILTER(const IndParam& n);

inline Indicator FILTER(const Indicator& ind, int n = 5) {
  return FILTER(n)(ind);
}

inline Indicator FILTER(const Indicator& ind, const IndParam& n) {
  return FILTER(n)(ind);
}

inline Indicator FILTER(const Indicator& ind, const Indicator& n) {
  return FILTER(IndParam(n))(ind);
}

}  // namespace hayaku

// ---- Merged from NOT.h ----
/*
 * NOT.h
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Logical NOT. NOT(X) returns the negation of X, i.e. it returns 1 when X<=0,
 * otherwise 0.
 * @ingroup Indicator
 */
Indicator HAYAKU_API NOT();

/**
 * Logical NOT. NOT(X) returns the negation of X, i.e. it returns 1 when X=0,
 * otherwise 0.
 * @param ind the data to be calculated
 * @ingroup Indicator
 */
inline Indicator NOT(const Indicator& ind) { return NOT()(ind); }

}  // namespace hayaku

// ---- Merged from ISINF.h ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Whether it is positive infinity (use ISINFA for negative infinity)
 * @ingroup Indicator
 */
Indicator HAYAKU_API ISINF();

inline Indicator ISINF(const Indicator& ind) { return ISINF()(ind); }

}  // namespace hayaku

// ---- Merged from ISINFA.h ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Whether it is negative infinity (use ISINF for positive infinity)
 * @ingroup Indicator
 */
Indicator HAYAKU_API ISINFA();

inline Indicator ISINFA(const Indicator& ind) { return ISINFA()(ind); }

}  // namespace hayaku

// ---- Merged from ISLASTBAR.h ----
/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-16
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Judge whether the current data is the last one; it returns 1 if it is the
 * last one, otherwise 0.
 * @ingroup Indicator
 */
Indicator HAYAKU_API ISLASTBAR();
Indicator HAYAKU_API ISLASTBAR(const KData& kdata);

inline Indicator ISLASTBAR(const Indicator& ind) { return ISLASTBAR()(ind); }

}  // namespace hayaku

// ---- Merged from ISLIMITDOWN.h ----
/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-26
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * @brief Indicator for judging whether a stock is limit down
 * @ingroup Indicator
 * @details Judge whether the close price of the day reaches the limit-down
 * price according to the different stock types:
 * - Ordinary A-share stock: the limit-down range is 10%
 * - Beijing Stock Exchange stock: the limit-down range is 30%
 * - ChiNext / STAR Market stock: the limit-down range is 20%
 * - ST stock: the limit-down range is 5%, but it is not handled yet because the
 * historical date information of the ST identifier is missing
 *
 * Limit-down judgment logic: close price of the day <= close price of the
 * previous day × (1 - limit-down range)
 *
 * Notes:
 * - The first K-line data is marked as discard because the previous day data is
 * missing
 * - An unsupported stock type returns the non-limit-down state by default
 * - The calculation result is a boolean value: 1 means limit down, 0 means not
 * limit down
 *
 * <pre>
 * Example:
 * @code
 * // Create the limit-down judgment indicator
 * Indicator limit_down = ISLIMITDOWN();
 *
 * // Judge the given K-line data
 * KData kdata = sm["sh000001"].getKData(KQuery(-100));
 * Indicator result = ISLIMITDOWN(kdata);
 *
 * // Get the judgment result
 * for (size_t i = 0; i < result.size(); ++i) {
 *     if (result[i] == 1.0) {
 *         cout << "day " << i << " is limit down" << endl;
 *     }
 * }
 * @endcode
 * </pre>
 *
 * @return Indicator the limit-down judgment indicator instance
 */
Indicator HAYAKU_API ISLIMITDOWN();

/**
 * @brief Judge whether the stock in the given K-line data is limit down
 * @ingroup Indicator
 * @param k K-line data
 * @return Indicator the limit-down judgment indicator instance
 * @see ISLIMITDOWN()
 */
Indicator HAYAKU_API ISLIMITDOWN(const KData& k);

}  // namespace hayaku

// ---- Merged from ISLIMITUP.h ----
/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-26
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * @brief Indicator for judging whether a stock is limit up
 * @ingroup Indicator
 * @details Judge whether the close price of the day reaches the limit-up price
 * according to the different stock types:
 * - Ordinary A-share stock: the limit-up range is 10%
 * - Beijing Stock Exchange stock: the limit-up range is 30%
 * - ChiNext / STAR Market stock: the limit-up range is 20%
 * - ST stock: the limit-up range is 5%, but it is not handled yet because the
 * historical date information of the ST identifier is missing
 *
 * Limit-up judgment logic: close price of the day >= close price of the
 * previous day × (1 + limit-up range)
 *
 * Notes:
 * - The first K-line data is marked as discard because the previous day data is
 * missing
 * - An unsupported stock type returns the non-limit-up state by default
 * - The calculation result is a boolean value: 1 means limit up, 0 means not
 * limit up
 *
 * <pre>
 * Example:
 * @code
 * // Create the limit-up judgment indicator
 * Indicator limit_up = ISLIMITUP();
 *
 * // Judge the given K-line data
 * KData kdata = sm["sh000001"].getKData(KQuery(-100));
 * Indicator result = ISLIMITUP(kdata);
 *
 * @endcode
 * </pre>
 *
 * @return Indicator the limit-up judgment indicator instance
 */
Indicator HAYAKU_API ISLIMITUP();

/**
 * @brief Judge whether the stock in the given K-line data is limit up
 * @ingroup Indicator
 * @param k K-line data
 * @return Indicator the limit-up judgment indicator instance
 * @see ISLIMITUP()
 */
Indicator HAYAKU_API ISLIMITUP(const KData& k);

}  // namespace hayaku

// ---- Merged from ISNA.h ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Whether it is a NaN value
 * @param ignore_discard whether to ignore the discard values, false by default
 * @ingroup Indicator
 */
Indicator HAYAKU_API ISNA(bool ignore_discard = false);

inline Indicator ISNA(const Indicator& ind, bool ignore_discard = false) {
  return ISNA(ignore_discard)(ind);
}

}  // namespace hayaku

// ---- Merged from CROSS.h ----
/*
 * CROSS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-2
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Two lines crossing; CROSS(A,B) returns 1 when A crosses B upward from below,
 * otherwise 0.
 * @ingroup Indicator
 */
inline Indicator CROSS(const Indicator& x, const Indicator& y) {
  Indicator result = (REF(x, 1) < REF(y, 1)) & (x > y);
  result.name("CROSS");
  return result;
}

inline Indicator CROSS(const Indicator& x, Indicator::value_t y) {
  return CROSS(x, CVAL(x, y));
}

inline Indicator CROSS(Indicator::value_t x, const Indicator& y) {
  return CROSS(CVAL(y, x), y);
}

inline Indicator CROSS(Indicator::value_t x, Indicator::value_t y) {
  return CROSS(CVAL(x), CVAL(y));
}

}  // namespace hayaku

// ---- Merged from NDAY.h ----
/*
 * NDAY.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-2
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Consecutive greater; NDAY(X,Y,N) means the condition X>Y exists persistently
 * for N periods
 * @ingroup Indicator
 */
inline Indicator NDAY(const Indicator& x, const Indicator& y, int n = 3) {
  Indicator result = EVERY(x > y, n);
  result.name("NDAY");
  return result;
}

inline Indicator NDAY(const Indicator& x, const Indicator& y,
                      const Indicator& n) {
  Indicator result = EVERY(x > y, n);
  result.name("NDAY");
  return result;
}

inline Indicator NDAY(const Indicator& x, const Indicator& y,
                      const IndParam& n) {
  Indicator result = EVERY(x > y, n);
  result.name("NDAY");
  return result;
}

}  // namespace hayaku

// ---- Merged from UPNDAY.h ----
/*
 * UPNDAY.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-2
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Consecutive rising periods; UPNDAY(CLOSE,M) means rising for M consecutive
 * periods
 * @ingroup Indicator
 */
inline Indicator UPNDAY(const Indicator& ind, int n = 3) {
  Indicator result = EVERY(ind > REF(ind, 1), n);
  result.name("UPDAY");
  return result;
}

inline Indicator UPNDAY(const Indicator& ind, const IndParam& n) {
  Indicator result = EVERY(ind > REF(ind, 1), n);
  result.name("UPDAY");
  return result;
}

inline Indicator UPNDAY(const Indicator& ind, const Indicator& n) {
  Indicator result = EVERY(ind > REF(ind, 1), n);
  result.name("UPDAY");
  return result;
}

}  // namespace hayaku

// ---- Merged from DOWNNDAY.h ----
/*
 * DOWNNDAY.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-2
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Consecutive falling periods; DOWNNDAY(CLOSE,M) means falling for M
 * consecutive periods
 * @ingroup Indicator
 */
inline Indicator DOWNNDAY(const Indicator& ind, int n = 3) {
  Indicator result = EVERY(REF(ind, 1) > ind, n);
  result.name("DOWNNDAY");
  return result;
}

inline Indicator DOWNNDAY(const Indicator& ind, const Indicator& n) {
  Indicator result = EVERY(REF(ind, 1) > ind, n);
  result.name("DOWNNDAY");
  return result;
}

inline Indicator DOWNNDAY(const Indicator& ind, const IndParam& n) {
  Indicator result = EVERY(REF(ind, 1) > ind, n.get());
  result.name("DOWNNDAY");
  return result;
}

}  // namespace hayaku

// ---- Merged from JUMPDOWN.h ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Edge jump, jumping from greater than 0.0 to <= 0.0
 * @ingroup Indicator
 */
Indicator HAYAKU_API JUMPDOWN();

inline Indicator JUMPDOWN(const Indicator& ind) { return JUMPDOWN()(ind); }

}  // namespace hayaku

// ---- Merged from JUMPUP.h ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Edge jump, jumping from less than or equal to 0.0 to > 0.0
 * @ingroup Indicator
 */
Indicator HAYAKU_API JUMPUP();

inline Indicator JUMPUP(const Indicator& ind) { return JUMPUP()(ind); }

}  // namespace hayaku

// ---- Merged from LONGCROSS.h ----
/*
 * LONGCROSS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-2
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Two lines cross after maintaining for a certain number of periods
 * @details
 * <pre>
 * LONGCROSS(A,B,N) means A is less than B within N periods,
 * and it returns 1 when A crosses B upward from below in the current period,
 * otherwise 0.
 * </pre>
 * @ingroup Indicator
 */

inline Indicator LONGCROSS(const Indicator& x, const Indicator& y, int n = 3) {
  Indicator result = EVERY((REF(x, 1) < REF(y, 1)), n) & (x > y);
  result.name("LONGCROSS");
  return result;
}

inline Indicator LONGCROSS(const Indicator& x, const Indicator& y,
                           const Indicator& n) {
  Indicator result = EVERY((REF(x, 1) < REF(y, 1)), n) & (x > y);
  result.name("LONGCROSS");
  return result;
}

inline Indicator LONGCROSS(const Indicator& x, Indicator::value_t y,
                           int n = 3) {
  return LONGCROSS(x, CVAL(x, y), n);
}

inline Indicator LONGCROSS(const Indicator& x, Indicator::value_t y,
                           const Indicator& n) {
  return LONGCROSS(x, CVAL(x, y), n);
}

inline Indicator LONGCROSS(Indicator::value_t x, const Indicator& y,
                           int n = 3) {
  return LONGCROSS(CVAL(y, x), y, n);
}

inline Indicator LONGCROSS(Indicator::value_t x, const Indicator& y,
                           const Indicator& n) {
  return LONGCROSS(CVAL(y, x), y, n);
}

inline Indicator LONGCROSS(Indicator::value_t x, Indicator::value_t y,
                           int n = 3) {
  return LONGCROSS(CVAL(x), CVAL(y), n);
}

inline Indicator LONGCROSS(Indicator::value_t x, Indicator::value_t y,
                           const Indicator& n) {
  return LONGCROSS(CVAL(x), CVAL(y), n);
}

}  // namespace hayaku
