#pragma once

/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-15
 *      Author: fasiondog
 */

#include "Indicator.h"
#include "operators/Factor.h"

namespace hayaku {

/**
 * Factor to indicator conversion
 * @details Convert a Factor object into an Indicator, so that it can be used in
 * the indicator system. This indicator needs a K-line context to be calculated.
 * @note Two Factors are considered the same by their names only
 * @param factor the factor object
 * @return the Indicator object
 * @ingroup Factor
 */
Indicator FACTOR(const Factor& factor);

/**
 * Factor to indicator conversion (convenience version)
 * @details Create an Indicator by the factor name, so that it can be used in
 * the indicator system. This indicator needs a K-line context to be calculated.
 * @note Two Factors are considered the same by their names only
 * @param name factor name
 * @return the Indicator object
 * @ingroup Factor
 */
inline Indicator FACTOR(const string& name) { return FACTOR(Factor(name)); }

}  // namespace hayaku

/*
 * ADVANCE.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-6-1
 *      Author: fasiondog
 */

#include "Indicator.h"
#include "data/StockTypeInfo.h"

namespace hayaku {

/**
 * Number of the rising stocks. When there is a given context and ignore_context
 * is false, the query, market and stk_type parameters are ignored.
 * @param query query condition
 * @param market the market it belongs to; when it is "" all the markets are got
 * @param stk_type security type; when it is greater than constant.STOCKTYPE_TMP
 * all the security types are got
 * @param ignore_context whether to ignore the context. When it is ignored, the
 * query, market and stk_type parameters are used forcibly.
 * @param fill_null fill the missing date data with nan when the data is
 * aligned.
 * @ingroup Indicator
 */
Indicator ADVANCE(const KQuery& query = KQueryByIndex(-100),
                  const string& market = "SH", int stk_type = STOCKTYPE_A,
                  bool ignore_context = false, bool fill_null = true);

}  // namespace hayaku

/*
 * DECLINE.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-6-3
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Number of the falling stocks. When there is a given context and
 * ignore_context is false, the query, market and stk_type parameters are
 * ignored.
 * @param query query condition
 * @param market the market it belongs to; when it is "" all the markets are got
 * @param stk_type security type; when it is greater than constant.STOCKTYPE_TMP
 * all the security types are got
 * @param ignore_context whether to ignore the context. When it is ignored, the
 * query, market and stk_type parameters are used forcibly.
 * @param fill_null fill the missing data with nan when the dates are aligned
 * @ingroup Indicator
 */
Indicator DECLINE(const KQuery& query = KQueryByIndex(-100),
                  const string& market = "SH", int stk_type = STOCKTYPE_A,
                  bool ignore_context = false, bool fill_null = true);

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-21
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Cross-sectional statistics (it returns the number of the stocks in the block)
 * @param blk the block to be counted
 * @param query the statistics range
 * @return Indicator
 */
Indicator BLOCKSETNUM(const Block& blk, const KQuery& query);
Indicator BLOCKSETNUM(const Block& blk);

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-26
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * @brief Return whether the security belongs to a certain block

 * @param category the given block category

 * @param name block name

 * @return Indicator
 */
Indicator INBLOCK(const string& category, const string& name);

/**
 * @brief Return whether the security belongs to a certain block

 * @param kdata K-line data

 * @param category the given block category

 * @param name block name

 * @return Indicator
 */
Indicator INBLOCK(const KData& kdata, const string& category,
                  const string& name);

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-21
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Return the calculated value of the corresponding output of this indicator for
 * every member of the block according to the calculation type.
 * @note Note: when INSUM uses mode 4/5 it is equivalent to the RANK function,
 * but it is not suitable for use in MF; when it is used in MF the calculation
 * amount is of the N x N level, and the calculation is slow. If it is expected
 * to be used in MF, it is recommended to use the RANK [donation user] indicator
 * directly.
 * @param block the given block
 * @param query the given range
 * @param ind the given indicator
 * @param mode calculation type: 0-accumulation, 1-average, 2-maximum,
 * 3-minimum, 4-descending rank (the highest indicator value has the rank 1),
 *             5-ascending rank (the lowest indicator value has the rank 1),
 * @param fill_null fill the missing data with nan when the dates are aligned.
 * @return Indicator
 */
Indicator INSUM(const Block& block, const KQuery& query, const Indicator& ind,
                int mode, bool fill_null = true);

Indicator INSUM(const Block& block, const Indicator& ind, int mode,
                bool fill_null = true);

Indicator INSUM(const Block& block, int mode, bool fill_null);

}  // namespace hayaku

/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-17
 *      Author: fasiondog
 */

#include "SeriesOperators.h"

namespace hayaku {

/**
 * @brief Calculate the adjustment factor indicator
 *
 * Calculate the backward adjustment factor sequence based on the
 * ex-rights/ex-dividend data of the stock (bonus shares, rights shares,
 * capitalized shares, cash dividend, etc.). The adjustment factor means: if 1
 * share was held at the listing, how many shares are held now after all the
 * bonus shares, rights shares and capitalized shares. It is calculated in a
 * cumulative multiplication way to ensure the consistency of the adjustment of
 * the price, the volume and the turnover amount.
 *
 * This indicator needs a KData context to work, it is set through the
 * setContext() method.
 *
 * @return Indicator the adjustment factor indicator object
 *
 * @par Usage example:
 * @code{.cpp}
 * // Get the adjustment factor of a stock
 * Stock stock = sm.getStock("sh000001");
 * KData kdata = stock.getKData(Query(-100));
 * Indicator adj_factor = ADJ_FACTOR();
 * adj_factor.setContext(kdata);
 * @endcode
 *
 * @see ADJ_OPEN adjusted open price
 * @see ADJ_HIGH adjusted high price
 * @see ADJ_LOW adjusted low price
 * @see ADJ_CLOSE adjusted close price
 * @see ADJ_VOL adjusted volume
 */
Indicator ADJ_FACTOR();

Indicator ADJ_FACTOR(const KData& kdata);

/**
 * @brief Calculate the adjusted open price indicator
 *
 * The open price is adjusted backward with the adjustment factor, so that the
 * adjusted open price sequence is obtained. Calculation formula: ADJ_OPEN =
 * ADJ_FACTOR * OPEN
 *
 * @return Indicator the adjusted open price indicator object
 *
 * @details Design purpose:
 * - This series of indicators (ADJ_*) is mainly designed to cooperate with the
 * factor management system to calculate the backward proportional adjustment
 * factor quickly
 * - In the factor management scenario, the adjustment calculation can be done
 * efficiently by updating the factor values incrementally and storing them
 * every day
 *
 * @warning Important limitations:
 * - **Period limitation**: it applies to the daily period only. Non-daily
 * periods such as the weekly and monthly periods have alignment problems and
 * the result may be inaccurate
 * - **Depends on factor management**: it needs to be used together with the
 * factor value storage of the factor management system,
 * update_all_factors_values() should be called every day to update and save the
 * factor values to guarantee the accuracy
 * - **Relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially
 * the same as RECOVER_EQUAL_FORWARD; if it is not the factor management
 * scenario, it is recommended to use RECOVER_EQUAL_FORWARD directly
 * - **Calculation start point**: neither of them starts the calculation from
 * the listing date, but from the start point of the currently queried K-line
 * data
 *
 * @see ADJ_FACTOR adjustment factor
 * @see RECOVER_EQUAL_FORWARD equal backward adjustment
 */
inline Indicator ADJ_OPEN() { return ADJ_FACTOR() * OPEN(); }

inline Indicator ADJ_OPEN(const KData& kdata) { return ADJ_OPEN()(kdata); }

/**
 * @brief Calculate the adjusted high price indicator
 *
 * The high price is adjusted backward with the adjustment factor, so that the
 * adjusted high price sequence is obtained. Calculation formula: ADJ_HIGH =
 * ADJ_FACTOR * HIGH
 *
 * @return Indicator the adjusted high price indicator object
 *
 * @details Design purpose:
 * - This series of indicators (ADJ_*) is mainly designed to cooperate with the
 * factor management system to calculate the backward proportional adjustment
 * factor quickly
 * - In the factor management scenario, the adjustment calculation can be done
 * efficiently by updating the factor values incrementally and storing them
 * every day
 *
 * @warning Important limitations:
 * - **Period limitation**: it applies to the daily period only. Non-daily
 * periods such as the weekly and monthly periods have alignment problems and
 * the result may be inaccurate
 * - **Depends on factor management**: it needs to be used together with the
 * factor value storage of the factor management system,
 * update_all_factors_values() should be called every day to update and save the
 * factor values to guarantee the accuracy
 * - **Relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially
 * the same as RECOVER_EQUAL_FORWARD; if it is not the factor management
 * scenario, it is recommended to use RECOVER_EQUAL_FORWARD directly
 * - **Calculation start point**: neither of them starts the calculation from
 * the listing date, but from the start point of the currently queried K-line
 * data
 *
 * @see ADJ_FACTOR adjustment factor
 * @see RECOVER_EQUAL_FORWARD equal backward adjustment
 */
inline Indicator ADJ_HIGH() { return ADJ_FACTOR() * HIGH(); }

inline Indicator ADJ_HIGH(const KData& kdata) { return ADJ_HIGH()(kdata); }

/**
 * @brief Calculate the adjusted low price indicator
 *
 * The low price is adjusted backward with the adjustment factor, so that the
 * adjusted low price sequence is obtained. Calculation formula: ADJ_LOW =
 * ADJ_FACTOR * LOW
 *
 * @return Indicator the adjusted low price indicator object
 *
 * @details Design purpose:
 * - This series of indicators (ADJ_*) is mainly designed to cooperate with the
 * factor management system to calculate the backward proportional adjustment
 * factor quickly
 * - In the factor management scenario, the adjustment calculation can be done
 * efficiently by updating the factor values incrementally and storing them
 * every day
 *
 * @warning Important limitations:
 * - **Period limitation**: it applies to the daily period only. Non-daily
 * periods such as the weekly and monthly periods have alignment problems and
 * the result may be inaccurate
 * - **Depends on factor management**: it needs to be used together with the
 * factor value storage of the factor management system,
 * update_all_factors_values() should be called every day to update and save the
 * factor values to guarantee the accuracy
 * - **Relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially
 * the same as RECOVER_EQUAL_FORWARD; if it is not the factor management
 * scenario, it is recommended to use RECOVER_EQUAL_FORWARD directly
 * - **Calculation start point**: neither of them starts the calculation from
 * the listing date, but from the start point of the currently queried K-line
 * data
 *
 * @see ADJ_FACTOR adjustment factor
 * @see RECOVER_EQUAL_FORWARD equal backward adjustment
 */
inline Indicator ADJ_LOW() { return ADJ_FACTOR() * LOW(); }

inline Indicator ADJ_LOW(const KData& kdata) { return ADJ_LOW()(kdata); }

/**
 * @brief Calculate the adjusted close price indicator
 *
 * The close price is adjusted backward with the adjustment factor, so that the
 * adjusted close price sequence is obtained. Calculation formula: ADJ_CLOSE =
 * ADJ_FACTOR * CLOSE
 *
 * @return Indicator the adjusted close price indicator object
 *
 * @details Design purpose:
 * - This series of indicators (ADJ_*) is mainly designed to cooperate with the
 * factor management system to calculate the backward proportional adjustment
 * factor quickly
 * - In the factor management scenario, the adjustment calculation can be done
 * efficiently by updating the factor values incrementally and storing them
 * every day
 *
 * @warning Important limitations:
 * - **Period limitation**: it applies to the daily period only. Non-daily
 * periods such as the weekly and monthly periods have alignment problems and
 * the result may be inaccurate
 * - **Depends on factor management**: it needs to be used together with the
 * factor value storage of the factor management system,
 * update_all_factors_values() should be called every day to update and save the
 * factor values to guarantee the accuracy
 * - **Relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially
 * the same as RECOVER_EQUAL_FORWARD; if it is not the factor management
 * scenario, it is recommended to use RECOVER_EQUAL_FORWARD directly
 * - **Calculation start point**: neither of them starts the calculation from
 * the listing date, but from the start point of the currently queried K-line
 * data
 *
 * @see ADJ_FACTOR adjustment factor
 * @see RECOVER_EQUAL_FORWARD equal backward adjustment
 */
inline Indicator ADJ_CLOSE() { return ADJ_FACTOR() * CLOSE(); }

inline Indicator ADJ_CLOSE(const KData& kdata) { return ADJ_CLOSE()(kdata); }

/**
 * @brief Calculate the adjusted volume indicator
 *
 * The volume is adjusted backward with the adjustment factor, so that the
 * adjusted volume sequence is obtained. Calculation formula: ADJ_VOL = VOL /
 * ADJ_FACTOR
 *
 * @return Indicator the adjusted volume indicator object
 *
 * @details Design purpose:
 * - This series of indicators (ADJ_*) is mainly designed to cooperate with the
 * factor management system to calculate the backward proportional adjustment
 * factor quickly
 * - In the factor management scenario, the adjustment calculation can be done
 * efficiently by updating the factor values incrementally and storing them
 * every day
 * -
 * Note: the volume adjustment uses division, which is opposite to the
 * multiplication used by the price adjustment. The reason is that when the
 * share capital increases, the volume corresponding to each share should
 * decrease accordingly
 *
 * @warning Important limitations:
 * - **Period limitation**: it applies to the daily period only. Non-daily
 * periods such as the weekly and monthly periods have alignment problems and
 * the result may be inaccurate
 * - **Depends on factor management**: it needs to be used together with the
 * factor value storage of the factor management system,
 * update_all_factors_values() should be called every day to update and save the
 * factor values to guarantee the accuracy
 * - **Relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially
 * the same as RECOVER_EQUAL_FORWARD; if it is not the factor management
 * scenario, it is recommended to use RECOVER_EQUAL_FORWARD directly
 * - **Calculation start point**: neither of them starts the calculation from
 * the listing date, but from the start point of the currently queried K-line
 * data
 *
 * @see ADJ_FACTOR adjustment factor
 * @see RECOVER_EQUAL_FORWARD equal backward adjustment
 */
inline Indicator ADJ_VOL() { return VOL() / ADJ_FACTOR(); }

inline Indicator ADJ_VOL(const KData& kdata) { return ADJ_VOL()(kdata); }

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-04-10
 *      Author: Jet
 */

namespace hayaku {

/**
 * @brief Return whether the security code matches the given pattern
 * @param pattern the match pattern, it supports the wildcards * and ?
 * @return Indicator
 */
Indicator CODELIKE(const string& pattern);

/**
 * @brief Return whether the security code matches the given pattern
 * @param kdata K-line data
 * @param pattern the match pattern, it supports the wildcards * and ?
 * @return Indicator
 */
Indicator CODELIKE(const KData& kdata, const string& pattern);

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */

#include "operators/Indicator.h"

namespace hayaku {

/**
 * PF rebalancing cycle indicator, mainly used for verifying the PF rebalancing
 * days and as SG
 * @param k the associated K-line data
 * @param adjust_cycle rebalancing cycle, 1 by default
 * @param adjust_mode rebalancing mode, "query" by default:
 * day|week|month|quarter|year|query
 * @param delay_to_trading_day delay to the trading day; when the rebalancing
 * day is a non-trading day, it is automatically delayed to the next trading day
 * as the rebalancing day
 * @ingroup Indicator
 */
Indicator CYCLE(const KData& k, int adjust_cycle = 1,
                const string& adjust_mode = "query",
                bool delay_to_trading_day = true);
Indicator CYCLE(int adjust_cycle = 1, const string& adjust_mode = "query",
                bool delay_to_trading_day = true);

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-13
 *      Author: fasiondog
 */

namespace hayaku {

Indicator FINANCE(int field_ix);
Indicator FINANCE(const KData& k, int field_ix);

Indicator FINANCE(const string& field_name);
Indicator FINANCE(const KData& k, const string& field_name);

}  // namespace hayaku

/*
 * LIUTONGPANG.h
 *
 *  Created on: 2019-3-6
 *      Author: fasiondog
 */

#define CAPITAL LIUTONGPAN

namespace hayaku {

/** Outstanding shares, in units of 10 thousand shares */
Indicator LIUTONGPAN();
Indicator LIUTONGPAN(const KData&);

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-04-10
 *      Author: Jet
 */

namespace hayaku {

/**
 * @brief Return whether the security name matches the given pattern
 * @param pattern the match pattern, it supports the wildcards * and ?
 * @return Indicator
 */
Indicator NAMELIKE(const string& pattern);

/**
 * @brief Return whether the security name matches the given pattern
 * @param kdata K-line data
 * @param pattern the match pattern, it supports the wildcards * and ?
 * @return Indicator
 */
Indicator NAMELIKE(const KData& kdata, const string& pattern);

}  // namespace hayaku

/*
 * STKTYPE.h
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-04-17
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Get the stock type indicator

 *
 * Return the type value of the current stock (the StockType enumeration value)

 *
 * @param k K-line data context (optional)

 * @return the indicator object, the value at every position is the type value
 of the stock

 * @ingroup Indicator
 *
 * @par Example:

 * @code
 * // Use the default context

 * auto stktype = STKTYPE();
 *
 * // The given K-line data

 * Stock stock = sm.getStock("sh000001");
 * KData kdata = stock.getKData(KQuery(0, 100));
 * auto stktype = STKTYPE(kdata);
 * @endcode
 */
Indicator STKTYPE();
Indicator STKTYPE(const KData& k);

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-01
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Yield of the 10-year Chinese treasury bond
 * @note The date in the context is used as the reference date with priority
 * @param default_val if the given date is earlier than the existing earliest
 * treasury bond data, this given default value is used
 * @return Indicator
 */
Indicator ZHBOND10(double default_val = 4.0);
Indicator ZHBOND10(const DatetimeList& dates, double default_val = 4.0);
Indicator ZHBOND10(const KData& k, double default_val = 4.0);
inline Indicator ZHBOND10(const Indicator& ind, double default_val = 4.0) {
  return ZHBOND10(default_val)(ind);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-18
 *      Author: fasiondog
 */

namespace hayaku {

Indicator ZONGGUBEN();
Indicator ZONGGUBEN(const KData&);

}  // namespace hayaku

/*
 * AD.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Accumulation / distribution line
 * @param k the associated KData
 * @ingroup Indicator
 */
Indicator AD(const KData& k);
Indicator AD();

}  // namespace hayaku

/*
 * COST.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-19
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Cost distribution
 * @details
 * <pre>
 * Usage: COST(k, X) means what the price is when X% of the positions are
 * profitable For example: COST(k, 10) means what the price is when 10% of the
 * positions are profitable, i.e. 10% of the positions are below that price, and
 * the remaining 90% are above that price and are trapped. This function is
 * valid for the daily analysis period only
 * </pre>
 * @param k the associated K-line data
 * @param x the X% profitable positions
 * @ingroup Indicator
 */
Indicator COST(const KData& k, double x = 10.0);
Indicator COST(double x = 10.0);

Indicator COST2(const KData& k, double x = 10.0);
Indicator COST2(double x = 10.0);

}  // namespace hayaku

/*
 * HSL.h
 *
 *  Created on: 2019-3-6
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Get the turnover rate; multiply it by 100 to get the percentage, it equals
 * VOL(k) / CAPITAL(k) * 0.01
 * @param k the associated K-line data
 * @ingroup Indicator
 */
Indicator HSL(const KData& k);
Indicator HSL();

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-16
 *      Author: fasiondog
 */

namespace hayaku {

// The broad market here always refers to sh000001

/** The corresponding broad market open price: Shanghai Composite Index,
 * Shenzhen Component Index, STAR 50 and ChiNext Index */
Indicator INDEXO(bool fill_null = true);
inline Indicator INDEXO(const KData& k, bool fill_null = true) {
  return INDEXO(fill_null)(k);
}

/** The corresponding broad market high price: Shanghai Composite Index,
 * Shenzhen Component Index, STAR 50 and ChiNext Index */
Indicator INDEXH(bool fill_null = true);
inline Indicator INDEXH(const KData& k, bool fill_null = true) {
  return INDEXH(fill_null)(k);
}

/** The corresponding broad market low price: Shanghai Composite Index, Shenzhen
 * Component Index, STAR 50 and ChiNext Index */
Indicator INDEXL(bool fill_null = true);
inline Indicator INDEXL(const KData& k, bool fill_null = true) {
  return INDEXL(fill_null)(k);
}

/** The corresponding broad market close price: Shanghai Composite Index,
 * Shenzhen Component Index, STAR 50 and ChiNext Index */
Indicator INDEXC(bool fill_null = true);
inline Indicator INDEXC(const KData& k, bool fill_null = true) {
  return INDEXC(fill_null)(k);
}

/** The corresponding broad market turnover amount: Shanghai Composite Index,
 * Shenzhen Component Index, STAR 50 and ChiNext Index */
Indicator INDEXA(bool fill_null = true);
inline Indicator INDEXA(const KData& k, bool fill_null = true) {
  return INDEXA(fill_null)(k);
}

/** The corresponding broad market volume: Shanghai Composite Index, Shenzhen
 * Component Index, STAR 50 and ChiNext Index */
Indicator INDEXV(bool fill_null = true);
inline Indicator INDEXV(const KData& k, bool fill_null = true) {
  return INDEXV(fill_null)(k);
}

/** Number of the rising stocks in the broad market, using the TDX SH880005, it
 * may not be usable for live trading */
Indicator INDEXADV();
Indicator INDEXADV(const KQuery& query);

/** Number of the falling stocks in the broad market, using the TDX SH880005, it
 * may not be usable for live trading */
Indicator INDEXDEC();
Indicator INDEXDEC(const KQuery& query);

}  // namespace hayaku

/*
 * IKDATA.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Get the year, month and day of this period since 1900. Usage: DATE. For
 example, the function
 * returns 1000101, meaning January 1, 2000.

 * @ingroup Indicator
 */
Indicator DATE();
Indicator DATE(const KData&);

/**
 * Get the hour, minute and second of this period. Usage: TIME. The valid value
 range of the
 * function is (000000-235959).

 * @ingroup Indicator
 */
Indicator TIME();
Indicator TIME(const KData&);

/**
 * Get the year of this period.

 * @ingroup Indicator
 */
Indicator YEAR();
Indicator YEAR(const KData&);

/**
 * Get the month of this period. Usage: MONTH. The valid value range of the
 function is (1-12).

 * @ingroup Indicator
 */
Indicator MONTH();
Indicator MONTH(const KData&);

/**
 * Get the day of the week of this period. Usage: WEEK. The valid value range of
 the function is
 * (0-6), 0 means Sunday.

 * @ingroup Indicator
 */
Indicator WEEK();
Indicator WEEK(const KData&);

/**
 * Get the day of this period. Usage: DAY. The valid value range of the function
 is (1-31).

 * @ingroup Indicator
 */
Indicator DAY();
Indicator DAY(const KData&);

/**
 * Get the hour of this period. Usage: HOUR. The valid value range of the
 function is (0-23); the
 * value is 0 for the daily line and longer analysis periods.

 * @ingroup Indicator
 */
Indicator HOUR();
Indicator HOUR(const KData&);

/**
 * Get the minute of this period. Usage: MINUTE. The valid value range of the
 function is (0-59);
 * the value is 0 for the daily line and longer analysis periods.

 * @ingroup Indicator
 */
Indicator MINUTE();
Indicator MINUTE(const KData&);

}  // namespace hayaku

/*
 * TIMELINE.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-15
 *      Author: fasiondog
 */

namespace hayaku {

Indicator TIMELINE();
Indicator TIMELINE(const KData&);

}  // namespace hayaku

/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-12-08
 *     Author: fasiondog
 */

namespace hayaku {

Indicator TIMELINEVOL();
Indicator TIMELINEVOL(const KData&);

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-06
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * @brief Turnover rate = stock volume / number of the outstanding shares × 100%
 * @param n window period
 * @return Indicator
 */
Indicator TURNOVER(int n = 1);

Indicator TURNOVER(const KData& kdata, int n = 1);

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-25
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Proportion of the profitable positions
 * @details
 * <pre>
 * Usage: WINNER(CLOSE) means the proportion of the profitable positions if sold
 * at the current close price. For example: returning 0.1 means 10% of the
 * positions are profitable; WINNER(10.5) means the proportion of the profitable
 * positions at the price of 10.5 yuan This function is valid for the daily
 * analysis period only.
 * </pre>
 * @ingroup Indicator
 */
Indicator WINNER();

inline Indicator WINNER(const Indicator& ind) { return WINNER()(ind); }

inline Indicator WINNER(Indicator::value_t val) { return WINNER(CVAL(val)); }

}  // namespace hayaku
