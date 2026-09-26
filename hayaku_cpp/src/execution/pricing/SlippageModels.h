#pragma once

/*
 * build_in.h
 *
 *  Created on: 2013-5-5
 *      Author: fasiondog
 */


#include <random>




/*
 * SL_FixedPercent.h
 *
 *  Created on: 2016-5-7
 *      Author: Administrator
 */


#include "SlippageBase.h"

namespace hayaku {

/**
 * Fixed percentage slippage algorithm
 * @details Actual buy price = planned buy price * (1 + p), actual sell price = planned sell price *
 *          (1 - p)
 * @param p the fixed offset percentage
 * @return SPPtr
 */
SlippagePtr HAYAKU_API SP_FixedPercent(double p = 0.001);

} /* namespace hayaku */


/*
 * SL_FixedValue.h
 *
 *  Created on: 2016-5-7
 *      Author: Administrator
 */



namespace hayaku {

/**
 * Fixed price slippage algorithm
 * @details Actual buy price = planned buy price + offset price, actual sell price = planned sell
 *          price - offset price
 * @param value
 * @return
 */
SlippagePtr HAYAKU_API SP_FixedValue(double value = 0.01);

} /* namespace hayaku */


/*
 * SP_Normal.h
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */



namespace hayaku {

/**
 * Normal distribution random price slippage algorithm: the buy and sell operations are a random
 * price offset based on the normal distribution
 * @param mean mean of the normal distribution
 * @param stddev standard deviation of the normal distribution
 * @return the slippage object pointer
 */
SlippagePtr HAYAKU_API SP_Normal(double mean = 0.0, double stddev = 0.05);

} /* namespace hayaku */

/*
 * SP_LogNormal.h
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */



namespace hayaku {

/**
 * Log-normal distribution random price slippage algorithm: the buy and sell operations are a random
 * price offset based on the log-normal distribution
 * @param mean the mean parameter of the log-normal distribution
 * @param stddev the standard deviation parameter of the log-normal distribution
 * @return the slippage object pointer
 */
SlippagePtr HAYAKU_API SP_LogNormal(double mean = 0.0, double stddev = 0.05);

} /* namespace hayaku */

/*
 * SP_TruncNormal.h
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */



namespace hayaku {

/**
 * Truncated normal distribution random price slippage algorithm: the buy and sell operations are a
 * random price offset based on the truncated normal distribution
 * @param mean mean of the normal distribution
 * @param stddev standard deviation of the normal distribution
 * @param min_value the truncation minimum
 * @param max_value the truncation maximum
 * @return the slippage object pointer
 */
SlippagePtr HAYAKU_API SP_TruncNormal(double mean = 0.0, double stddev = 0.05, double min_value = -0.1,
                                   double max_value = 0.1);

} /* namespace hayaku */

/*
 * SL_FixedValue.h
 *
 *  Created on: 2016-5-7
 *      Author: Administrator
 */



namespace hayaku {

/**
 * Uniform distribution random price slippage algorithm: the buy and sell operations are a random
 * offset of the price with a uniform distribution within the range [min_value, max_value]
 * @param min_value the lower limit of the offset price
 * @param max_value the upper limit of the offset price
 * @return
 */
SlippagePtr HAYAKU_API SP_Uniform(double min_value = -0.05, double max_value = 0.05);

} /* namespace hayaku */

/*
 * FixedPercentSlippage.h
 *
 *  Created on: 2016-5-7
 *      Author: Administrator
 */



namespace hayaku {

class FixedPercentSlippage : public SlippageBase {
    SLIPPAGE_IMP(FixedPercentSlippage)
    SLIPPAGE_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedPercentSlippage();
    virtual ~FixedPercentSlippage();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */


/*
 * FixedValueSlippage.h
 *
 *  Created on: 2016-5-7
 *      Author: Administrator
 */



namespace hayaku {

class FixedValueSlippage : public SlippageBase {
    SLIPPAGE_IMP(FixedValueSlippage)
    SLIPPAGE_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedValueSlippage();
    virtual ~FixedValueSlippage();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */


/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */



namespace hayaku {

class NormalSlippage : public SlippageBase {
    SLIPPAGE_IMP(NormalSlippage)
    SLIPPAGE_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    NormalSlippage();
    virtual ~NormalSlippage();
    virtual void _checkParam(const string& name) const override;

private:
    static std::random_device ms_rd;
    static std::mt19937 ms_gen;
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */



namespace hayaku {

class LogNormalSlippage : public SlippageBase {
    SLIPPAGE_IMP(LogNormalSlippage)
    SLIPPAGE_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    LogNormalSlippage();
    virtual ~LogNormalSlippage();
    virtual void _checkParam(const string& name) const override;

private:
    static std::random_device ms_rd;
    static std::mt19937 ms_gen;
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */



namespace hayaku {

class TruncNormalSlippage : public SlippageBase {
    SLIPPAGE_IMP(TruncNormalSlippage)
    SLIPPAGE_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TruncNormalSlippage();
    virtual ~TruncNormalSlippage();
    virtual void _checkParam(const string& name) const override;

private:
    static std::random_device ms_rd;
    static std::mt19937 ms_gen;
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */



namespace hayaku {

class UniformSlippage : public SlippageBase {
    SLIPPAGE_IMP(UniformSlippage)
    SLIPPAGE_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    UniformSlippage();
    virtual ~UniformSlippage();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */
