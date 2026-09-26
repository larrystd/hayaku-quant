#pragma once

// ---- Merged from ACOS.h ----
/*
 * ACOS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */
#include "SeriesOperators.h"

namespace hayaku {

/**
 * Cosine value
 * @ingroup Indicator
 */
Indicator HAYAKU_API ACOS();

inline Indicator ACOS(const Indicator& ind) {
    return ACOS()(ind);
}

inline Indicator ACOS(Indicator::value_t val) {
    return ACOS(CVAL(val));
}

}  // namespace hayaku


// ---- Merged from ASIN.h ----
/*
 * ASIN.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Arcsine value
 * @ingroup Indicator
 */
Indicator HAYAKU_API ASIN();

inline Indicator ASIN(const Indicator& ind) {
    return ASIN()(ind);
}

inline Indicator ASIN(Indicator::value_t val) {
    return ASIN(CVAL(val));
}

}  // namespace hayaku


// ---- Merged from ATAN.h ----
/*
 * ATAN.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Arctangent value
 * @ingroup Indicator
 */
Indicator HAYAKU_API ATAN();

inline Indicator ATAN(const Indicator& ind) {
    return ATAN()(ind);
}

inline Indicator ATAN(Indicator::value_t val) {
    return ATAN(CVAL(val));
}

}  // namespace hayaku


// ---- Merged from COS.h ----
/*
 * COS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Cosine value
 * @ingroup Indicator
 */
Indicator HAYAKU_API COS();

inline Indicator COS(const Indicator& ind) {
    return COS()(ind);
}

inline Indicator COS(Indicator::value_t val) {
    return COS(CVAL(val));
}

}  // namespace hayaku


// ---- Merged from SIN.h ----
/*
 * SIN.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Sine value
 * @ingroup Indicator
 */
Indicator HAYAKU_API SIN();

inline Indicator SIN(const Indicator& ind) {
    return SIN()(ind);
}

inline Indicator SIN(Indicator::value_t val) {
    return SIN(CVAL(val));
}

}  // namespace hayaku


// ---- Merged from TAN.h ----
/*
 * TAN.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Tangent value
 * @ingroup Indicator
 */
Indicator HAYAKU_API TAN();

inline Indicator TAN(const Indicator& ind) {
    return TAN()(ind);
}

inline Indicator TAN(Indicator::value_t val) {
    return TAN(CVAL(val));
}

}  // namespace hayaku
