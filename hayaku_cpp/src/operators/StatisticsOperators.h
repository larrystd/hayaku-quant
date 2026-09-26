#pragma once

/*
 * KURT.h
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "Indicator.h"

namespace hayaku {

/**
 * Calculate the excess kurtosis within N periods (unadjusted population
 * kurtosis - 3)
 * @param n N-day time window (greater than or equal to 4 or equal to 0); when
 * it is 0 the actual length of the input ind is used
 * @ingroup Indicator
 */
Indicator HAYAKU_API KURT(int n = 10);
Indicator HAYAKU_API KURT(const IndParam& n);

inline Indicator KURT(const Indicator& data, int n = 10) {
  return KURT(n)(data);
}

inline Indicator KURT(const Indicator& data, const IndParam& n) {
  return KURT(n)(data);
}

inline Indicator KURT(const Indicator& data, const Indicator& n) {
  return KURT(IndParam(n))(data);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Maximum profit percentage (calculated in the opposite direction corresponding
 * to MDD)
 * @ingroup Indicator
 */
Indicator HAYAKU_API MRR(int n = 0);

/**
 * Maximum profit percentage
 * @param ind the data to be calculated
 * @ingroup Indicator
 */
inline Indicator MRR(const Indicator& ind, int n = 0) { return MRR(n)(ind); }

}  // namespace hayaku

/*
 * SKEW.h
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Calculate the unadjusted population skewness within N periods
 * @param n N-day time window (greater than or equal to 3 or equal to 0); when
 * it is 0 the actual length of the input ind is used
 * @ingroup Indicator
 */
Indicator HAYAKU_API SKEW(int n = 10);
Indicator HAYAKU_API SKEW(const IndParam& n);

inline Indicator SKEW(const Indicator& data, int n = 10) {
  return SKEW(n)(data);
}

inline Indicator SKEW(const Indicator& data, const IndParam& n) {
  return SKEW(n)(data);
}

inline Indicator SKEW(const Indicator& data, const Indicator& n) {
  return SKEW(IndParam(n))(data);
}

}  // namespace hayaku

/*
 * STD.h
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Calculate the sample standard deviation within N periods
 * @param n N-day time window
 * @ingroup Indicator
 */
Indicator HAYAKU_API STDEV(int n = 10);
Indicator HAYAKU_API STDEV(const IndParam& n);

/**
 * Calculate the sample standard deviation within N periods
 * @param data the input data, a single input
 * @param n N-day time window
 * @ingroup Indicator
 */
inline Indicator STDEV(const Indicator& data, int n = 10) {
  return STDEV(n)(data);
}

inline Indicator STDEV(const Indicator& data, const IndParam& n) {
  return STDEV(n)(data);
}

inline Indicator STDEV(const Indicator& data, const Indicator& n) {
  return STDEV(IndParam(n))(data);
}

}  // namespace hayaku

/*
 * STD.h
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Calculate the population standard deviation within N periods
 * @param n N-day time window
 * @ingroup Indicator
 */
Indicator HAYAKU_API STDP(int n = 10);
Indicator HAYAKU_API STDP(const IndParam& n);

inline Indicator STDP(const Indicator& data, int n = 10) {
  return STDP(n)(data);
}

inline Indicator STDP(const Indicator& data, const IndParam& n) {
  return STDP(n)(data);
}

inline Indicator STDP(const Indicator& data, const Indicator& n) {
  return STDP(IndParam(n))(data);
}

}  // namespace hayaku

/*
 * VAR.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Calculate the estimated sample variance within N periods
 * @param n N-day time window
 * @ingroup Indicator
 */
Indicator HAYAKU_API VAR(int n = 10);
Indicator HAYAKU_API VAR(const IndParam& n);

inline Indicator VAR(const Indicator& data, int n = 10) { return VAR(n)(data); }

inline Indicator VAR(const Indicator& data, const IndParam& n) {
  return VAR(n)(data);
}

inline Indicator VAR(const Indicator& data, const Indicator& n) {
  return VAR(IndParam(n))(data);
}

}  // namespace hayaku

/*
 * VARP.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Calculate the population sample variance within N periods
 * @param n N-day time window
 * @ingroup Indicator
 */
Indicator HAYAKU_API VARP(int n = 10);
Indicator HAYAKU_API VARP(const IndParam& n);

inline Indicator VARP(const Indicator& data, int n = 10) {
  return VARP(n)(data);
}

inline Indicator VARP(const Indicator& data, const IndParam& n) {
  return VARP(n)(data);
}

inline Indicator VARP(const Indicator& data, const Indicator& n) {
  return VARP(IndParam(n))(data);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Perform a ZScore standardization (3sigma) on the data for the extreme value
 * handling
 * @note It is not a window rolling one; for a window rolling standardization,
 * just use (x - MA(x, n)) / STDEV(x, n)
 * @param outExtreme indicates the removal of the extreme values (i.e.
 * truncating the extreme values, the ones exceeding the limit are replaced with
 * the limit value)
 * @param nsigma the nsigma multiple of sigma used when removing the extreme
 * values
 * @param recursive whether to remove the extreme values recursively
 * @return Indicator
 * @ingroup Indicator
 */
Indicator HAYAKU_API ZSCORE(bool outExtreme = false, double nsigma = 3.0,
                            bool recursive = false);

inline Indicator ZSCORE(const Indicator& data, bool outExtreme = false,
                        double nsigma = 3.0, bool recursive = false) {
  return ZSCORE(outExtreme, nsigma, recursive)(data);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-XX-XX
 *  Author: fasiondog
 */

namespace hayaku {

/**
 * Calculate the Beta coefficient, such as measuring the sensitivity between the
 asset return and
 * the market return

 * Beta = Cov(stock_return, market_return) / Var(market_return)
 *
 * @note BETA itself does not perform the return conversion (pct_change) on the
 input data,

 *       the input indicators should be the already calculated return data.

 * @param ind1 the input indicator, such as the stock return indicator

 * @param ind2 the reference indicator, such as the market return indicator

 * @param n the rolling window (greater than 2 or equal to 0); when it is 0 the
 actual length of the
 *          input ind is used.

 * @param fill_null fill the missing dates with nan when the dates are aligned

 * @ingroup Indicator
 */
Indicator HAYAKU_API BETA(const Indicator& ind1, const Indicator& ind2,
                          int n = 10, bool fill_null = true);
Indicator HAYAKU_API BETA(const Indicator& ref_ind, int n = 10,
                          bool fill_null = true);

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Calculate the sample correlation coefficient and covariance. In the returned
 * result sets, the first is the correlation coefficient and the second is the
 * covariance
 * @param ind1 indicator 1
 * @param ind2 indicator 2
 * @param n the rolling window (greater than 2 or equal to 0); when it is 0 the
 * actual length of the input ind is used.
 * @param fill_null fill the missing dates with nan when the dates are aligned
 * @ingroup Indicator
 */
Indicator HAYAKU_API CORR(const Indicator& ind1, const Indicator& ind2,
                          int n = 10, bool fill_null = true);
Indicator HAYAKU_API CORR(const Indicator& ref_ind, int n = 10,
                          bool fill_null = true);

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Calculate the sample covariance
 * @param ind1 indicator 1
 * @param ind2 indicator 2
 * @param n the rolling window (greater than 2 or equal to 0); when it is 0 the
 * actual length of the input ind is used.
 * @param fill_null fill the missing dates with nan when the dates are aligned
 * @ingroup Indicator
 */
Indicator HAYAKU_API COV(const Indicator& ind1, const Indicator& ind2,
                         int n = 10, bool fill_null = true);
Indicator HAYAKU_API COV(const Indicator& ref_ind, int n = 10,
                         bool fill_null = true);

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Spearman correlation coefficient
 * @param ind indicator 1
 * @param ref_ind indicator 2
 * @param n the rolling window (greater than 2 or equal to 0); when it is 0, n
 * actually uses the length of ind
 * @param fill_null fill the missing values
 * @ingroup Indicator
 */
Indicator HAYAKU_API SPEARMAN(const Indicator& ind, const Indicator& ref_ind,
                              int n = 0, bool fill_null = true);
Indicator HAYAKU_API SPEARMAN(const Indicator& ref_ind, int n = 0,
                              bool fill_null = true);

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Perform the data truncation with the quantile rule
 * @param int n window size
 * @param quantile_min minimum quantile
 * @param quantile_max maximum quantile
 * @return Indicator
 * @ingroup Indicator
 */
Indicator HAYAKU_API QUANTILE_TRUNC(int n = 60, double quantile_min = 0.01,
                                    double quantile_max = 0.99);

inline Indicator QUANTILE_TRUNC(const Indicator& data, int n = 60,
                                double quantile_min = 0.01,
                                double quantile_max = 0.99) {
  return QUANTILE_TRUNC(n, quantile_min, quantile_max)(data);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-09
 *      Author: fasiondog
 */

#include "data/Block.h"

namespace hayaku {

/**
 * @brief Calculate the IC of the given factor relative to the reference
 * security (actually RankIC)
 * @note The IC originally needs "the factor value at t -> the return at t+1";
 * here it is changed to calculating "the factor value at t -> the return of the
 * N days before t" (such as the return of the past 5 days), which is called the
 * "current IC". (Otherwise the current values would all be missing NA) If a
 * strict "the factor value at t -> the return at t+1" calculation is needed,
 * please set strict=True (note that in this mode the last n values are NA)
 * @param stks the security portfolio
 * @param n time window (corresponding to the n-day return)
 * @param spearman use the spearman correlation coefficient, otherwise pearson
 * @param strict strict mode, it follows the IC definition "the factor value at
 * t -> the return at t+1"
 * @return Indicator
 * @ingroup Indicator
 */
Indicator HAYAKU_API IC(const StockList& stks, int n = 1, bool spearman = true,
                        bool strict = false);

Indicator HAYAKU_API IC(const Block& blk, int n = 1, bool spearman = true,
                        bool strict = false);

inline Indicator IC(const Indicator& ind, const StockList& stks, int n = 1,
                    bool spearman = true, bool strict = false) {
  return IC(stks, n, spearman, strict)(ind);
}

inline Indicator IC(const Indicator& ind, const Block& blk, int n = 1,
                    bool spearman = true, bool strict = false) {
  return IC(blk, n, spearman, strict)(ind);
}

/**
 * @brief Calculate the IC of the given factor list relative to the given return
 * list, where inds and returns are both already calculated and aligned by date.
 * inds does not need to be shifted right by n.
 * @note It is a numeric calculation only, the returned result has no aligned
 * dates
 * @param inds factor list, inds does not need to be shifted right by n.
 * @param returns return list
 * @param n time window (corresponding to the n-day return)
 * @param use_spearman use the spearman correlation coefficient, otherwise
 * pearson
 * @param strict strict mode, it follows the IC definition "the factor value at
 * t -> the return at t+1"
 */
Indicator HAYAKU_API IC(IndicatorList inds, IndicatorList returns, int n = 1,
                        bool use_spearman = true, bool strict = false);

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "WindowOperators.h"

namespace hayaku {

/**
 * @brief Calculate the ICIR of the given factor relative to the reference
 * security (actually RankIC)
 * @details IR: Information Ratio (IR for short) =
 * the multi-period mean of the IC / the standard deviation of the IC, it
 * represents the ability of the factor to obtain a stable Alpha.
 * @note The IC originally needs "the factor value at t -> the return at t+1";
 * here it is changed to calculating "the factor value at t -> the return of the
 * N days before t" (such as the return of the past 5 days), which is called the
 * "current IC". (Otherwise the current values would all be missing NA) If a
 * strict "the factor value at t -> the return at t+1" calculation is needed,
 * please set strict=True (note that in this mode the last n values are NA)
 * @param ind factor formula
 * @param stks the security portfolio
 * @param query query condition
 * @param n the N-day return corresponding to the IC
 * @param rolling_n the rolling time window
 * @param spearman use the spearman correlation coefficient, otherwise pearson
 * @param strict whether it is the strict mode
 * @return Indicator
 * @ingroup Indicator
 */
inline Indicator ICIR(const Indicator& ind, const StockList& stks, int n = 1,
                      int rolling_n = 120, bool spearman = true,
                      bool strict = false) {
  Indicator ic = IC(ind, stks, n, spearman, strict);
  Indicator x = MA(ic, rolling_n) / STDEV(ic, rolling_n);
  x.name("ICIR");
  x.setParam<int>("n", n);
  x.setParam<int>("rolling_n", rolling_n);
  return x;
}

inline Indicator ICIR(const Indicator& ind, const Block& blk, int n = 1,
                      int rolling_n = 120, bool spearman = true,
                      bool strict = false) {
  Indicator ic = IC(ind, blk, n, spearman, strict);
  Indicator x = MA(ic, rolling_n) / STDEV(ic, rolling_n);
  x.name("ICIR");
  x.setParam<int>("n", n);
  x.setParam<int>("rolling_n", rolling_n);
  return x;
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "MomentumOperators.h"

namespace hayaku {

/**
 * Information Ratio (IR)

 * @details
 * <pre>
 * Formula: (P-B) / TE

 * P: portfolio return

 * B: benchmark return

 * TE: the standard deviation between the daily p and b in the investment period

 * In practice, P is generally the asset curve of TM and B is the close price of
 the CSI 300, e.g.:

 * ref_k = sm["sh000300"].get_kdata(query)
 * funds = my_tm.get_funds_curve(ref_k.get_datetime.list())
 * ir = IR(PRICELIST(funds), ref_k.close, 0)
 * </pre>
 * @note If the IR is expected to be calculated from the IC values, please use
 the ICIR indicator

 * @param p
 * @param b
 * @param n
 * @ingroup Indicator
 */
inline Indicator IR(const Indicator& p, const Indicator& b, int n = 100) {
  Indicator p_return = ROCP(p, n);
  Indicator b_return = ROCP(b, n);
  Indicator x = (p_return - b_return);
  Indicator ret = x / STDEV(x, n);
  ret.name("IR");
  ret.setParam<int>("n", n);
  return ret;
}

}  // namespace hayaku

/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Calculate the linear regression slope, the goodness of fit R² and the
 * relative maximum residual; N supports a variable
 * @param n time window
 * @return Indicator, it contains three result sets:
 *         - result(0): slope
 *         - result(1): goodness of fit R²
 *         - result(2): relative maximum residual RelMaxRes = max|yi - ŷi| / ȳ
 */
Indicator HAYAKU_API SLOPE(int n = 22);
Indicator HAYAKU_API SLOPE(const IndParam& n);

/**
 * Calculate the linear regression slope, the goodness of fit R² and the
 * relative maximum residual; N supports a variable
 * @param ind the indicator to be calculated
 * @param n time window
 * @return Indicator, it contains three result sets:
 *         - result(0): slope
 *         - result(1): goodness of fit R²
 *         - result(2): relative maximum residual RelMaxRes = max|yi - ŷi| / ȳ
 */
inline Indicator SLOPE(const Indicator& ind, int n = 22) {
  return SLOPE(n)(ind);
}

inline Indicator HAYAKU_API SLOPE(const Indicator& ind, const IndParam& n) {
  return SLOPE(n)(ind);
}

inline Indicator SLOPE(const Indicator& ind, const Indicator& n) {
  return SLOPE(IndParam(n))(ind);
}

}  // namespace hayaku

/*
 * TS_RANK.h
 *
 *  Created on: 2026-6-9
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Time series rank, it calculates the rank ratio of the current value within
 * the past N periods
 * @details
 * <pre>
 * Usage: TS_RANK(X,N) means the rank of X within the past N periods (from 1 to
 * N) divided by N For example: TS_RANK(CLOSE,20) means the rank ratio of the
 * close price within the past 20 periods
 *
 * Definition in Alpha101:
 * TS_RANK(x, n) = (rank of x in the last n observations) / n
 * where rank is the ascending rank, i.e. the smaller values are ranked first
 *
 * Implementation description:
 * For every period i, count the number of the elements in the window [i-n+1, i]
 * that are less than or equal to x[i], then TS_RANK = count / n
 * </pre>
 * @param n number of the periods
 * @ingroup Indicator
 */
Indicator HAYAKU_API TS_RANK(int n = 20);
Indicator HAYAKU_API TS_RANK(const IndParam& n);

/**
 * Time series rank, it calculates the rank ratio of the current value within
 * the past N periods
 * @details
 * <pre>
 * Usage: TS_RANK(X,N) means the rank of X within the past N periods (from 1 to
 * N) divided by N For example: TS_RANK(CLOSE,20) means the rank ratio of the
 * close price within the past 20 periods
 * </pre>
 * @param ind the data to be calculated
 * @param n number of the periods
 * @ingroup Indicator
 */
inline Indicator TS_RANK(const Indicator& ind, int n = 20) {
  return TS_RANK(n)(ind);
}

inline Indicator TS_RANK(const Indicator& ind, const IndParam& n) {
  return TS_RANK(n)(ind);
}

inline Indicator TS_RANK(const Indicator& ind, const Indicator& n) {
  return TS_RANK(IndParam(n))(ind);
}

}  // namespace hayaku
