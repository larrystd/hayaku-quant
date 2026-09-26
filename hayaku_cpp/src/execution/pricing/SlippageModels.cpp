#include "SlippageModels.h"

/*
 * FixedPercentSlippage.cpp
 *
 *  Created on: 2016-5-7
 *      Author: Administrator
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::FixedPercentSlippage)
#endif

namespace hayaku {

FixedPercentSlippage::FixedPercentSlippage() : SlippageBase("SP_FixedPercent") {
    setParam<double>("p", 0.001);
}

FixedPercentSlippage::~FixedPercentSlippage() {}

void FixedPercentSlippage::_checkParam(const string& name) const {
    if ("p" == name) {
        double p = getParam<double>(name);
        HAYAKU_ASSERT(p >= 0.0 && p < 1.0);
    }
}

price_t FixedPercentSlippage ::getRealBuyPrice(const Datetime& datetime, price_t price) {
    return price * (1 + getParam<double>("p"));
}

price_t FixedPercentSlippage ::getRealSellPrice(const Datetime& datetime, price_t price) {
    return price * (1 - getParam<double>("p"));
}

void FixedPercentSlippage::_calculate() {}

SlippagePtr HAYAKU_API SP_FixedPercent(double p) {
    SlippagePtr ptr = make_shared<FixedPercentSlippage>();
    ptr->setParam("p", p);
    return ptr;
}

} /* namespace hayaku */

/*
 * FixedValueSlippage.cpp
 *
 *  Created on: 2016-5-7
 *      Author: Administrator
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::FixedValueSlippage)
#endif

namespace hayaku {

FixedValueSlippage::FixedValueSlippage() : SlippageBase("SP_FixedValue") {
    setParam<double>("value", 0.01);
}

FixedValueSlippage::~FixedValueSlippage() {}

void FixedValueSlippage::_checkParam(const string& name) const {
    if ("value" == name) {
        HAYAKU_ASSERT(getParam<double>(name) >= 0.0);
    }
}

price_t FixedValueSlippage ::getRealBuyPrice(const Datetime& datetime, price_t price) {
    return price + getParam<double>("value");
}

price_t FixedValueSlippage ::getRealSellPrice(const Datetime& datetime, price_t price) {
    return price - getParam<double>("value");
}

void FixedValueSlippage::_calculate() {}

SlippagePtr HAYAKU_API SP_FixedValue(double value) {
    SlippagePtr ptr = make_shared<FixedValueSlippage>();
    ptr->setParam("value", value);
    return ptr;
}

} /* namespace hayaku */

/*
 * NormalSlippage.cpp
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */

#include <random>

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::NormalSlippage)
#endif

namespace hayaku {

std::random_device NormalSlippage::ms_rd;
std::mt19937 NormalSlippage::ms_gen(ms_rd());

NormalSlippage::NormalSlippage() : SlippageBase("SP_Normal") {
    setParam<double>("mean", 0.0);
    setParam<double>("stddev", 0.05);
}

NormalSlippage::~NormalSlippage() {}

void NormalSlippage::_checkParam(const string& name) const {
    if ("stddev" == name) {
        HAYAKU_ASSERT(getParam<double>("stddev") >= 0.0);
    }
}

price_t NormalSlippage::getRealBuyPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");

    std::normal_distribution<double> dis(mean, stddev);

    double value = dis(ms_gen);
    return price + std::abs(value);
}

price_t NormalSlippage::getRealSellPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");

    std::normal_distribution<double> dis(mean, stddev);

    double value = dis(ms_gen);
    return price - std::abs(value);
}

void NormalSlippage::_calculate() {}

SlippagePtr HAYAKU_API SP_Normal(double mean, double stddev) {
    SlippagePtr ptr = make_shared<NormalSlippage>();
    ptr->setParam("mean", mean);
    ptr->setParam("stddev", stddev);
    return ptr;
}

} /* namespace hayaku */

/*
 * LogNormalSlippage.cpp
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::LogNormalSlippage)
#endif

namespace hayaku {

std::random_device LogNormalSlippage::ms_rd;
std::mt19937 LogNormalSlippage::ms_gen(ms_rd());

LogNormalSlippage::LogNormalSlippage() : SlippageBase("SP_LogNormal") {
    setParam<double>("mean", 0.0);
    setParam<double>("stddev", 0.05);
}

LogNormalSlippage::~LogNormalSlippage() {}

void LogNormalSlippage::_checkParam(const string& name) const {
    if ("mean" == name) {
        HAYAKU_ASSERT(!std::isnan(getParam<double>("mean")));
    } else if ("stddev" == name) {
        HAYAKU_ASSERT(getParam<double>("stddev") >= 0.0);
    }
}

price_t LogNormalSlippage::getRealBuyPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");

    std::lognormal_distribution<double> dis(mean, stddev);

    double value = dis(ms_gen);
    // To distribute the slippage values around the mean, exp(mean+stddev^2/2) is subtracted to
    // center them
    double centered_value = value - std::exp(mean + stddev * stddev / 2.0);
    // On a buy the price always goes higher (the unfavorable direction)
    return price + std::abs(centered_value);
}

price_t LogNormalSlippage::getRealSellPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");

    std::lognormal_distribution<double> dis(mean, stddev);

    double value = dis(ms_gen);
    // To distribute the slippage values around the mean, exp(mean+stddev^2/2) is subtracted to
    // center them
    double centered_value = value - std::exp(mean + stddev * stddev / 2.0);
    // On a sell the price always goes lower (the unfavorable direction)
    return price - std::abs(centered_value);
}

void LogNormalSlippage::_calculate() {}

SlippagePtr HAYAKU_API SP_LogNormal(double mean, double stddev) {
    SlippagePtr ptr = make_shared<LogNormalSlippage>();
    ptr->setParam("mean", mean);
    ptr->setParam("stddev", stddev);
    return ptr;
}

} /* namespace hayaku */

/*
 * TruncNormalSlippage.cpp
 *
 *  Created on: 2025-10-25
 *      Author: fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::TruncNormalSlippage)
#endif

namespace hayaku {

std::random_device TruncNormalSlippage::ms_rd;
std::mt19937 TruncNormalSlippage::ms_gen(ms_rd());

TruncNormalSlippage::TruncNormalSlippage() : SlippageBase("SP_TruncNormal") {
    setParam<double>("mean", 0.0);
    setParam<double>("stddev", 0.05);
    setParam<double>("min_value", -0.1);
    setParam<double>("max_value", 0.1);
}

TruncNormalSlippage::~TruncNormalSlippage() {}

void TruncNormalSlippage::_checkParam(const string& name) const {
    if ("mean" == name) {
        HAYAKU_ASSERT(!std::isnan(getParam<double>("mean")));
    } else if ("stddev" == name) {
        HAYAKU_ASSERT(getParam<double>("stddev") >= 0.0);
    } else if ("min_value" == name) {
        HAYAKU_ASSERT(!std::isnan(getParam<double>("min_value")));
    } else if ("max_value" == name) {
        HAYAKU_ASSERT(!std::isnan(getParam<double>("max_value")));
    }

    if (haveParam("min_value") && haveParam("max_value")) {
        double min_v = getParam<double>("min_value");
        double max_v = getParam<double>("max_value");
        HAYAKU_ASSERT(min_v <= max_v);
    }
}

price_t TruncNormalSlippage::getRealBuyPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");
    double min_v = getParam<double>("min_value");
    double max_v = getParam<double>("max_value");

    std::normal_distribution<double> dis(mean, stddev);

    double value;
    // Generate a value within the truncated range
    do {
        value = dis(ms_gen);
    } while (value < min_v || value > max_v);

    // On a buy the price always goes higher (the unfavorable direction)
    return price + std::abs(value);
}

price_t TruncNormalSlippage::getRealSellPrice(const Datetime& datetime, price_t price) {
    double mean = getParam<double>("mean");
    double stddev = getParam<double>("stddev");
    double min_v = getParam<double>("min_value");
    double max_v = getParam<double>("max_value");

    std::normal_distribution<double> dis(mean, stddev);

    double value;
    // Generate a value within the truncated range
    do {
        value = dis(ms_gen);
    } while (value < min_v || value > max_v);

    // On a sell the price always goes lower (the unfavorable direction)
    return price - std::abs(value);
}

void TruncNormalSlippage::_calculate() {}

SlippagePtr HAYAKU_API SP_TruncNormal(double mean, double stddev, double min_value, double max_value) {
    SlippagePtr ptr = make_shared<TruncNormalSlippage>();
    ptr->setParam("mean", mean);
    ptr->setParam("stddev", stddev);
    ptr->setParam("min_value", min_value);
    ptr->setParam("max_value", max_value);
    return ptr;
}

} /* namespace hayaku */

/*
 * UniformSlippage.cpp
 *
 *  Created on: 2016-5-7
 *      Author: Administrator
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::UniformSlippage)
#endif

namespace hayaku {

UniformSlippage::UniformSlippage() : SlippageBase("SP_Uniform") {
    setParam<double>("min_value", -0.05);
    setParam<double>("max_value", 0.05);
}

UniformSlippage::~UniformSlippage() {}

void UniformSlippage::_checkParam(const string& name) const {
    if ("min_value" == name) {
        HAYAKU_ASSERT(!std::isnan(getParam<double>("min_value")));
    } else if ("max_value" == name) {
        HAYAKU_ASSERT(!std::isnan(getParam<double>("max_value")));
    }

    if (haveParam("min_value") && haveParam("max_value")) {
        double min_v = getParam<double>("min_value");
        double max_v = getParam<double>("max_value");
        HAYAKU_ASSERT(min_v <= max_v);
    }
}

price_t UniformSlippage ::getRealBuyPrice(const Datetime& datetime, price_t price) {
    double min_v = getParam<double>("min_value");
    double max_v = getParam<double>("max_value");
    double value = min_v + (rand() / (RAND_MAX + 1.0)) * (max_v - min_v);
    return price + std::abs(value);
}

price_t UniformSlippage ::getRealSellPrice(const Datetime& datetime, price_t price) {
    double min_v = getParam<double>("min_value");
    double max_v = getParam<double>("max_value");
    double value = min_v + (rand() / (RAND_MAX + 1.0)) * (max_v - min_v);
    return price - std::abs(value);
}

void UniformSlippage::_calculate() {}

SlippagePtr HAYAKU_API SP_Uniform(double min_value, double max_value) {
    SlippagePtr ptr = make_shared<UniformSlippage>();
    ptr->setParam("min_value", min_value);
    ptr->setParam("max_value", max_value);
    return ptr;
}

} /* namespace hayaku */
