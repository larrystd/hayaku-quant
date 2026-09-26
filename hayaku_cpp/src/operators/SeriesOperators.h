#pragma once

// ---- Merged from CVAL.h ----
/*
 * CVAL.h
 *
 *  Created on: 2017-6-25
 *      Author: fasiondog
 */
#include "Indicator.h"

namespace hayaku {

// Kept visible because RECOVER validates KDATA by its concrete implementation type.
class IKData : public IndicatorImp {
    INDICATOR_IMP(IKData)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IKData();
    virtual ~IKData() override;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

namespace hayaku {

/**
 * Create a constant indicator with the given length

 * @param value the constant

 * @param discard the number to discard, 0 by default

 * @ingroup Indicator
 */
Indicator HAYAKU_API CVAL(double value, size_t discard=0);

/**
 * Create a constant indicator whose length is the same as the input ind and whose value is fixed to
 * the given value

 * @param ind the data to be calculated

 * @param value the constant

 * @param discard the number to discard, 0 by default

 * @ingroup Indicator
 */
Indicator HAYAKU_API CVAL(const Indicator& ind, double value = 0.0, int discard = 0);

} /* namespace */


// ---- Merged from KDATA.h ----
/*
 * IKDATA.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Wrap KData into an Indicator, used for the calculation of the other indicators
 * @ingroup Indicator
 */
Indicator HAYAKU_API KDATA();
Indicator HAYAKU_API KDATA(const KData&);

/**
 * Wrap the open price of KData into an Indicator, used for the calculation of the other indicators
 * @ingroup Indicator
 */
Indicator HAYAKU_API OPEN();
Indicator HAYAKU_API OPEN(const KData&);

/**
 * Wrap the high price of KData into an Indicator, used for the calculation of the other indicators
 * @ingroup Indicator
 */
Indicator HAYAKU_API HIGH();
Indicator HAYAKU_API HIGH(const KData&);

/**
 * Wrap the low price of KData into an Indicator, used for the calculation of the other indicators
 * @ingroup Indicator
 */
Indicator HAYAKU_API LOW();
Indicator HAYAKU_API LOW(const KData&);

/**
 * Wrap the close price of KData into an Indicator, used for the calculation of the other indicators
 * @ingroup Indicator
 */
Indicator HAYAKU_API CLOSE();
Indicator HAYAKU_API CLOSE(const KData&);

/**
 * Wrap the turnover amount of KData into an Indicator, used for the calculation of the other
 * indicators
 * @ingroup Indicator
 */
Indicator HAYAKU_API AMO();
Indicator HAYAKU_API AMO(const KData&);

/**
 * Wrap the volume of KData into an Indicator, used for the calculation of the other indicators
 * @ingroup Indicator
 */
Indicator HAYAKU_API VOL();
Indicator HAYAKU_API VOL(const KData&);

/**
 * Return KDATA/OPEN/HIGH/LOW/CLOSE/AMO/VOL according to the string
 * @param kdata K-line data
 * @param kpart KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
 * @see KDATA, OPEN, HIGH, LOW, CLOSE, AMO, VOL
 * @ingroup Indicator
 */
Indicator HAYAKU_API KDATA_PART(const KData& kdata, const string& kpart);
Indicator HAYAKU_API KDATA_PART(const string& kpart);

}  // namespace hayaku

// ---- Merged from PRICELIST.h ----
/*
 * PRICELIST.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#define VALUE PRICELIST

namespace hayaku {

/**
 * Wrap a PriceList into an Indicator
 * @param data source data
 * @param discard the number of the data points discarded at the front; the discarded values are
 *                 filled with Null<price_t>()
 * @ingroup Indicator
 */
Indicator HAYAKU_API PRICELIST(const PriceList& data, int discard = 0);
Indicator HAYAKU_API PRICELIST(PriceList&& data, int discard = 0);
Indicator HAYAKU_API PRICELIST(const PriceList& data, const DatetimeList& ds, int discard = 0);
Indicator HAYAKU_API PRICELIST(PriceList&& data, const DatetimeList&& ds, int discard = 0);
Indicator HAYAKU_API PRICELIST(size_t size, double value, int discard = 0);
Indicator HAYAKU_API PRICELIST(const DatetimeList& dates, double value, int discard = 0);
Indicator HAYAKU_API PRICELIST(DatetimeList&& dates, double value, int discard = 0);
Indicator HAYAKU_API PRICELIST();

/**
 * Wrap an array into an Indicator, used to calculate the other indicators
 * @param data price_t[]
 * @param total array size
 * @ingroup Indicator
 */
// Indicator HAYAKU_API PRICELIST(double* data, size_t total);
template <typename ValueT>
Indicator PRICELIST(ValueT* data, size_t total) {
    HAYAKU_IF_RETURN(!data || total == 0, PRICELIST(PriceList()));
    PriceList tmp(total);
    std::copy(data, data + total, tmp.begin());
    return PRICELIST(tmp);
}

}  // namespace hayaku


// ---- Merged from CONTEXT.h ----
/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-28
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Independent context indicator
 * @param ind the indicator to be wrapped
 * @param fill_null whether to fill the null values, false by default
 * @param use_self_ktype use the K-line type of its own independent context, otherwise the K-line
 *                        type of the calculation context is used
 * @param use_self_recover_type use the adjustment type of its own independent context, otherwise
 *                              the adjustment type of the calculation context is used
 * @ingroup Indicator
 */
Indicator HAYAKU_API CONTEXT(const Indicator& ind, bool fill_null = false, bool use_self_ktype = false,
                          bool use_self_recover_type = false);
Indicator HAYAKU_API CONTEXT(bool fill_null = false, bool use_self_ktype = false,
                          bool use_self_recover_type = false);

/**
 * @brief Set an independent context for the indicator by the given stock
 * @param ind the input indicator formula; it is ignored if it carries a context itself
 * @param stk the given stock
 * @param fill_null whether to fill the null values, false by default
 * @return Indicator
 */
Indicator HAYAKU_API CONTEXT(const Indicator& ind, const Stock& stk, bool fill_null = false);

/**
 * Get the indicator context
 * @note The Indicator::getContext() method gets the current context, but for the CONTEXT
 * independent context indicator its given independent context cannot be got by that method, it
 * must be got with this method.
 * Once this indicator participates in the calculation as a formula, its context may change, but
 * its stock remains unchanged, only the query range changes
 * @param ind
 * @return KData
 */
KData HAYAKU_API CONTEXT_K(const Indicator& ind);

/**
 * @brief Judge whether the indicator is an independent context indicator
 * @param ind
 * @return true
 * @return false
 */
bool HAYAKU_API is_standalone_context(const Indicator& ind);

}  // namespace hayaku

// ---- Merged from RESULT.h ----
/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-12
 *      Author: fasiondog
 */

namespace hayaku {

Indicator HAYAKU_API RESULT(int result_ix);

inline Indicator RESULT(const Indicator& ind, int result_ix) {
    return RESULT(result_ix)(ind);
}

}  // namespace hayaku

// ---- Merged from RECOVER.h ----
/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240317 added by fasiondog
 */

namespace hayaku {

/**
 * Adjustment indicators; they accept the CLOSE/OPEN/HIGH/LOW indicators or a KData (close is used
 * by default) as the input parameter
 * @return Indicator
 * @ingroup Indicator
 */
Indicator HAYAKU_API RECOVER_FORWARD();
Indicator HAYAKU_API RECOVER_BACKWARD();
Indicator HAYAKU_API RECOVER_EQUAL_FORWARD();
Indicator HAYAKU_API RECOVER_EQUAL_BACKWARD();

Indicator HAYAKU_API RECOVER_FORWARD(const Indicator&);
Indicator HAYAKU_API RECOVER_BACKWARD(const Indicator&);
Indicator HAYAKU_API RECOVER_EQUAL_FORWARD(const Indicator&);
Indicator HAYAKU_API RECOVER_EQUAL_BACKWARD(const Indicator&);

inline Indicator RECOVER_FORWARD(const KData& kdata) {
    return RECOVER_FORWARD(kdata.close());
}

inline Indicator RECOVER_BACKWARD(const KData& kdata) {
    return RECOVER_BACKWARD(kdata.close());
}

inline Indicator RECOVER_EQUAL_FORWARD(const KData& kdata) {
    return RECOVER_EQUAL_FORWARD(kdata.close());
}

inline Indicator RECOVER_EQUAL_BACKWARD(const KData& kdata) {
    return RECOVER_EQUAL_BACKWARD(kdata.close());
}

}  // namespace hayaku

// ---- Merged from REF.h ----
/*
 * REF.h
 *
 *  Created on: 2015-3-21
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * REF forward reference (i.e. shift right)
 * Reference the data of several periods before.
 * Usage: REF(X,A) references the X value A periods before.
 * @param n references the value n periods before, i.e. shifting right by n
 * @ingroup Indicator
 */
Indicator HAYAKU_API REF(int n);
Indicator HAYAKU_API REF(const IndParam& n);

/**
 * REF forward reference (i.e. shift right)
 * Reference the data of several periods before.
 * Usage: REF(X,A) references the X value A periods before.
 * @param ind the data to be calculated
 * @param n references the value n periods before, i.e. shifting right by n
 * @ingroup Indicator
 */
inline Indicator REF(const Indicator& ind, int n) {
    return REF(n)(ind);
}

inline Indicator REF(const Indicator& ind, const IndParam& n) {
    return REF(n)(ind);
}

inline Indicator REF(const Indicator& ind, const Indicator& n) {
    return REF(IndParam(n))(ind);
}

} /* namespace hayaku */


// ---- Merged from REFX.h ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-22
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Enhanced REF (unsafe reference), used to shift the data left or right; when the period is an
 * integer it works the same as REF
 * @note It should not be used for a backtest, it is usually used in scenarios such as the AI model
 *       training
 * @param n reference period
 * @ingroup Indicator
 */
Indicator HAYAKU_API REFX(int n);
inline Indicator REFX(const Indicator& ind, int n) {
    return REFX(n)(ind);
}

}  // namespace hayaku

// ---- Merged from LASTVALUE.h ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-04
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Take the last value of the input indicator as a constant, i.e. all the values in the result are
 * the last value of the input indicator; use it with caution
 * @note It is equivalent to the TDX CONST indicator; it is named LASTVALUE because of the name
 *       conflict of CONST under Windows
 * @ingroup Indicator
 */
Indicator HAYAKU_API LASTVALUE(bool ignore_discard = false);

inline Indicator LASTVALUE(const Indicator& ind, bool ignore_discard = false) {
    return LASTVALUE(ignore_discard)(ind);
}

}  // namespace hayaku


// ---- Merged from ALIGN.h ----
/*
 * ALIGN.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-21
 *      Author: fasiondog
 */

namespace hayaku {

#if defined(ALIGN)
#undef ALIGN
#endif

/**
 * Align by the given dates
 * @ingroup Indicator
 */
Indicator HAYAKU_API ALIGN(bool fill_null = true);
Indicator HAYAKU_API ALIGN(const DatetimeList&, bool fill_null = true);
Indicator HAYAKU_API ALIGN(DatetimeList&&, bool fill_null = true);

inline Indicator ALIGN(const Indicator& ind, const DatetimeList& ref, bool fill_null = true) {
    return ALIGN(ref, fill_null)(ind);
}

inline Indicator ALIGN(const Indicator& ind, DatetimeList&& ref, bool fill_null = true) {
    return ALIGN(std::move(ref), fill_null)(ind);
}

inline Indicator ALIGN(const Indicator& ind, const Indicator& ref, bool fill_null = true) {
    return ALIGN(ref.getDatetimeList(), fill_null)(ind);
}

inline Indicator ALIGN(const Indicator& ind, const KData& ref, bool fill_null = true) {
    return ALIGN(ref.getDatetimeList(), fill_null)(ind);
}

}  // namespace hayaku


// ---- Merged from BACKSET.h ----
/*
 * BACKSET.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-13
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Forward assignment, it sets the data from the current position to several periods before to 1
 * @details
 * <pre>
 * Usage: BACKSET(X,N): if X is not 0, the values from the current position to N periods before are
 * set to 1.
 * For example: BACKSET(CLOSE>OPEN,2) sets the values of the current period and the previous period
 * to 1 if the candle closes up, otherwise 0
 * </pre>
 * @ingroup Indicator
 */
Indicator HAYAKU_API BACKSET(int n = 2);
Indicator HAYAKU_API BACKSET(const IndParam& n);

inline Indicator BACKSET(const Indicator& ind, int n = 2) {
    return BACKSET(n)(ind);
}

inline Indicator BACKSET(const Indicator& ind, const IndParam& n) {
    return BACKSET(n)(ind);
}

inline Indicator BACKSET(const Indicator& ind, const Indicator& n) {
    return BACKSET(IndParam(n))(ind);
}

}  // namespace hayaku


// ---- Merged from DISCARD.h ----
/*
 * COS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

namespace hayaku {

/** Set the discard value in the way of an indicator formula */
Indicator HAYAKU_API DISCARD(int discard);

inline Indicator DISCARD(const Indicator& ind, int discard) {
    return DISCARD(discard)(ind);
}

}  // namespace hayaku


// ---- Merged from DROPNA.h ----
/*
 * DROPNA.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-28
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Remove the nan values
 * @ingroup Indicator
 */
Indicator HAYAKU_API DROPNA();

inline Indicator DROPNA(const Indicator& ind) {
    return DROPNA()(ind);
}

}  // namespace hayaku


// ---- Merged from REPLACE.h ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-12
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Replace the given value, it is usually used to replace the Nan values
 * @param old_val the value to be replaced
 * @param new_val the replaced value
 * @param ignore_discard ignore discard; if the nan values are replaced, the new discard is set to 0
 * @ingroup Indicator
 */
Indicator HAYAKU_API REPLACE(double old_val = Null<double>(), double new_val = 0.0,
                          bool ignore_discard = false);

inline Indicator REPLACE(const Indicator& ind, double old_val = Null<double>(),
                         double new_val = 0.0, bool ignore_discard = false) {
    return REPLACE(old_val, new_val, ignore_discard)(ind);
}

}  // namespace hayaku

// ---- Merged from REVERSE.h ----
/*
 * REVERSE.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Calculate the opposite number, REVERSE(X) returns -X
 * @ingroup Indicator
 */
Indicator HAYAKU_API REVERSE();

inline Indicator REVERSE(const Indicator& ind) {
    return REVERSE()(ind);
}

inline Indicator REVERSE(Indicator::value_t val) {
    return REVERSE(CVAL(val));
}

}  // namespace hayaku


// ---- Merged from SLICE.h ----
/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-27
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Get the data of the given range [start, end) in a PriceList
 * @param data source data
 * @param start the start range, it can be negative
 * @param end the end range (excluded), it can be negative
 * @ingroup Indicator
 */
Indicator HAYAKU_API SLICE(const PriceList& data, int64_t start, int64_t end);

/**
 * Get the data of the given range in an indicator
 * @param start the start range, it can be negative
 * @param end the end range (excluded), it can be negative
 * @param result_index the given result set in the source data, less than 0 means all
 * @ingroup Indicator
 */
Indicator HAYAKU_API SLICE(int64_t start, int64_t end, int result_index = -1);

/**
 * Get the data of the given range in an indicator
 * @param ind source data
 * @param start the start range, it can be negative
 * @param end the end range (excluded), it can be negative
 * @param result_index the given result set in the source data, less than 0 means all
 * @ingroup Indicator
 */
inline Indicator SLICE(const Indicator& ind, int64_t start, int64_t end, int result_index = -1) {
    return SLICE(start, end, result_index)(ind);
}

}  // namespace hayaku
