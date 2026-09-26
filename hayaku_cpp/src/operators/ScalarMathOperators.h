#pragma once

// ---- Merged from ABS.h ----
/*
 * ABS.h
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */
#include "SeriesOperators.h"

namespace hayaku {

/**
 * Calculate the absolute value
 * @ingroup Indicator
 */
Indicator ABS();

inline Indicator ABS(const Indicator& ind) { return ABS()(ind); }

inline Indicator ABS(Indicator::value_t val) { return ABS(CVAL(val)); }

}  // namespace hayaku

// ---- Merged from CEILING.h ----
/*
 * CEILING.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-15
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Round up (round in the direction of increasing value)
 * Usage: CEILING(A) returns the nearest integer in the direction of increasing
 * value of A For example: CEILING(12.3) gives 13; CEILING(-3.5) gives -3
 * @ingroup Indicator
 */
Indicator CEILING();

inline Indicator CEILING(const Indicator& ind) { return CEILING()(ind); }

inline Indicator CEILING(Indicator::value_t val) { return CEILING(CVAL(val)); }

}  // namespace hayaku

// ---- Merged from FLOOR.h ----
/*
 * FLOOR.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-15
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Round down (round in the direction of decreasing value) to an integer
 * Usage: FLOOR(A) returns the nearest integer in the direction of decreasing
 * value of A For example: FLOOR(12.3) gives 12
 * @ingroup Indicator
 */
Indicator FLOOR();

inline Indicator FLOOR(const Indicator& ind) { return FLOOR()(ind); }

inline Indicator FLOOR(Indicator::value_t val) { return FLOOR(CVAL(val)); }

}  // namespace hayaku

// ---- Merged from INTPART.h ----
/*
 * INTPART.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-18
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Round up (round in the direction of increasing value)
 * Usage: CEILING(A) returns the nearest integer in the direction of increasing
 * value of A For example: CEILING(12.3) gives 13; CEILING(-3.5) gives -3
 * @ingroup Indicator
 */
Indicator INTPART();

inline Indicator INTPART(const Indicator& ind) { return INTPART()(ind); }

inline Indicator INTPART(Indicator::value_t val) { return INTPART(CVAL(val)); }

}  // namespace hayaku

// ---- Merged from MOD.h ----
/*
 * MOD.h
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Modulo after rounding
 * @details
 * <pre>
 * This function exists only for the TDX compatibility. In fact, the modulo of
 * the indicators can be done directly with the % operator
 * </pre>
 * @param ind1 indicator 1, it is rounded
 * @param ind2 indicator 2, it is rounded
 * @ingroup Indicator
 */

inline Indicator MOD(const Indicator& ind1, const Indicator& ind2) {
  return (ind1 % ind2);
}

inline Indicator MOD(const Indicator& ind1, Indicator::value_t ind2) {
  return ind1 % CVAL(ind1, ind2);
}

inline Indicator MOD(Indicator::value_t ind1, const Indicator& ind2) {
  return CVAL(ind2, ind1) % ind2;
}

inline Indicator MOD(Indicator::value_t ind1, Indicator::value_t ind2) {
  return CVAL(ind1) % CVAL(ind2);
}

}  // namespace hayaku

// ---- Merged from EXP.h ----
/*
 * EXP.h
 *
 *  Created on: 2019-4-3
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Exponent, EXP(X) is e to the power of X
 * @ingroup Indicator
 */
Indicator EXP();

inline Indicator EXP(const Indicator& ind) { return EXP()(ind); }

inline Indicator EXP(Indicator::value_t val) { return EXP(CVAL(val)); }

}  // namespace hayaku

// ---- Merged from LN.h ----
/*
 * LN.h
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Calculate the natural logarithm
 * Usage: LN(X) is the logarithm with the base e
 * For example: LN(CLOSE) gives the logarithm of the close price
 * @ingroup Indicator
 */
Indicator LN();

inline Indicator LN(const Indicator& ind) { return LN()(ind); }

inline Indicator LN(Indicator::value_t val) { return LN(CVAL(val)); }

}  // namespace hayaku

// ---- Merged from LOG.h ----
/*
 * LOG.h
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Logarithm with the base 10
 * Usage: LOG(X) gets the logarithm of X
 * @ingroup Indicator
 */
Indicator LOG();

inline Indicator LOG(const Indicator& ind) { return LOG()(ind); }

inline Indicator LOG(Indicator::value_t val) { return LOG(CVAL(val)); }

}  // namespace hayaku

// ---- Merged from POW.h ----
/*
 * POW.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Power
 * @details
 * <pre>
 * Usage: POW(A,B) returns A to the power of B
 * For example: POW(CLOSE,3) gives the cube of the close price
 * </pre>
 * @ingroup Indicator
 */
Indicator POW(int n);
Indicator POW(const IndParam& n);

inline Indicator POW(const Indicator& ind, int n) { return POW(n)(ind); }

inline Indicator POW(const Indicator& ind, const IndParam& n) {
  return POW(n)(ind);
}

inline Indicator POW(const Indicator& ind, const Indicator& n) {
  return POW(IndParam(n))(ind);
}

inline Indicator POW(Indicator::value_t val, int n) {
  return POW(CVAL(val), n);
}

}  // namespace hayaku

// ---- Merged from SQRT.h ----
/*
 * SQRT.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Square root
 * @details
 * <pre>
 * Usage: SQRT(X) is the square root of X
 * For example: SQRT(CLOSE) is the square root of the close price
 * </pre>
 * @ingroup Indicator
 */
Indicator SQRT();

inline Indicator SQRT(const Indicator& ind) { return SQRT()(ind); }

inline Indicator SQRT(Indicator::value_t val) { return SQRT(CVAL(val)); }

}  // namespace hayaku

// ---- Merged from SGN.h ----
/*
 * SGN.h
 *
 *  Created on: 2019-4-1
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Calculate the sign value, SGN(X) returns 1, 0 and -1 respectively when X>0,
 * X=0 and X<0.
 * @ingroup Indicator
 */
Indicator SGN();

inline Indicator SGN(const Indicator& ind) { return SGN()(ind); }

inline Indicator SGN(Indicator::value_t val) { return SGN(CVAL(val)); }

}  // namespace hayaku

// ---- Merged from SIGNED_POWER.h ----
/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-06-09
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Power
 * @details
 * <pre>
 * SIGNED_POWER(A,B) returns A to the power of B, but keeps the original sign
 * For example: SIGNED_POWER(CLOSE,3) gives the cube of the close price and
 * keeps the original sign
 * </pre>
 * @ingroup Indicator
 */
Indicator SIGNED_POWER(int n);
Indicator SIGNED_POWER(const IndParam& n);

inline Indicator SIGNED_POWER(const Indicator& ind, int n) {
  return SIGNED_POWER(n)(ind);
}

inline Indicator SIGNED_POWER(const Indicator& ind, const IndParam& n) {
  return SIGNED_POWER(n)(ind);
}

inline Indicator SIGNED_POWER(const Indicator& ind, const Indicator& n) {
  return SIGNED_POWER(IndParam(n))(ind);
}

inline Indicator SIGNED_POWER(Indicator::value_t val, int n) {
  return SIGNED_POWER(CVAL(val), n);
}

}  // namespace hayaku

// ---- Merged from ROUND.h ----
/*
 * ROUND.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Rounding
 * @ingroup Indicator
 */
Indicator ROUND(int ndigits = 2);

inline Indicator ROUND(const Indicator& ind, int n = 2) {
  return ROUND(n)(ind);
}

inline Indicator ROUND(Indicator::value_t val, int n = 2) {
  return ROUND(CVAL(val), n);
}

}  // namespace hayaku

// ---- Merged from ROUNDDOWN.h ----
/*
 * ROUNDDOWN.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Truncate upward, e.g. 10.1 is truncated to 11
 * @ingroup Indicator
 */
Indicator ROUNDDOWN(int ndigits = 2);

inline Indicator ROUNDDOWN(const Indicator& ind, int n = 2) {
  return ROUNDDOWN(n)(ind);
}

inline Indicator ROUNDDOWN(Indicator::value_t val, int n = 2) {
  return ROUNDDOWN(CVAL(val), n);
}

}  // namespace hayaku

// ---- Merged from ROUNDUP.h ----
/*
 * ROUNDUP.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Truncate upward, e.g. 10.1 is truncated to 11
 * @ingroup Indicator
 */
Indicator ROUNDUP(int ndigits = 2);
;

inline Indicator ROUNDUP(const Indicator& ind, int n = 2) {
  return ROUNDUP(n)(ind);
}

inline Indicator ROUNDUP(Indicator::value_t val, int n = 2) {
  return ROUNDUP(CVAL(val), n);
}

}  // namespace hayaku
