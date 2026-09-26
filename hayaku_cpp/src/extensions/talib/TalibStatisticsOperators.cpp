#include "TalibOperators.h"
#include "TalibSupport.h"

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */


#include "operators/Indicator.h"

namespace hayaku {

class TaBbands : public IndicatorImp {
    INDICATOR_IMP(TaBbands)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaBbands();
    virtual ~TaBbands() = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::TaBbands)
#endif

namespace hayaku {

TaBbands::TaBbands() : IndicatorImp("TA_BBANDS", 3) {
    setParam<int>("n", 5);
    setParam<double>("nbdevup", 2.0);
    setParam<double>("nbdevdn", 2.0);
    setParam<int>("matype", 0);
}

void TaBbands::_checkParam(const string& name) const {
    if (name == "n") {
        int n = getParam<int>("n");
        HAYAKU_ASSERT(n >= 2 && n <= 100000);
    } else if (name == "matype") {
        int matype = getParam<int>("matype");
        HAYAKU_ASSERT(matype >= 0 && matype <= 8);
    } else if (name == "nbdevup") {
        HAYAKU_ASSERT(!std::isnan(getParam<double>("nbdevup")));
    } else if (name == "nbdevdn") {
        HAYAKU_ASSERT(!std::isnan(getParam<double>("nbdevdn")));
    }
}

void TaBbands::_calculate(const Indicator& data) {
    int n = getParam<int>("n");
    double nbdevup = getParam<double>("nbdevup");
    double nbdevdn = getParam<double>("nbdevdn");
    TA_MAType matype = (TA_MAType)getParam<int>("matype");
    size_t total = data.size();
    int lookback = TA_BBANDS_Lookback(n, nbdevup, nbdevdn, matype);
    if (lookback < 0) {
        m_discard = total;
        return;
    }

    m_discard = data.discard() + lookback;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto* dst0 = this->data(0);
    auto* dst1 = this->data(1);
    auto* dst2 = this->data(2);

    const double* src = data.data();

    int outBegIdx;
    int outNbElement;
    ::TA_BBANDS(m_discard, total - 1, src, n, nbdevup, nbdevdn, matype, &outBegIdx, &outNbElement,
              dst0 + m_discard, dst1 + m_discard, dst2 + m_discard);
    HAYAKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);
}

Indicator HAYAKU_API TA_BBANDS(int n, double nbdevup, double nbdevdn, int matype) {
    auto p = make_shared<TaBbands>();
    p->setParam<int>("n", n);
    p->setParam<double>("nbdevup", nbdevup);
    p->setParam<double>("nbdevdn", nbdevdn);
    p->setParam<int>("matype", matype);
    return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-22
 *      Author: fasiondog
 */


#include "operators/Indicator.h"

namespace hayaku {

class TaStddev : public IndicatorImp {
    INDICATOR_IMP(TaStddev)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaStddev();
    virtual ~TaStddev() = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-22
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::TaStddev)
#endif

namespace hayaku {

TaStddev::TaStddev() : IndicatorImp("TA_STDDEV", 1) {
    setParam<int>("n", 5);
    setParam<double>("nbdev", 1.0);
}

void TaStddev::_checkParam(const string& name) const {
    if (name == "n") {
        int n = getParam<int>(name);
        HAYAKU_CHECK(n >= 2 && n <= 100000, "n must in [2, 100000]!]");
    } else if (name == "nbdev") {
        double nbdev = getParam<double>("nbdev");
        HAYAKU_ASSERT(!std::isnan(nbdev));
    }
}

void TaStddev::_calculate(const Indicator& data) {
    int n = getParam<int>("n");
    double nbdev = getParam<double>("nbdev");
    size_t total = data.size();
    int lookback = TA_STDDEV_Lookback(n, nbdev);
    if (lookback < 0) {
        m_discard = total;
        return;
    }

    m_discard = data.discard() + lookback;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    const double* src = data.data();
    auto* dst = this->data();

    int outBegIdx;
    int outNbElement;
    ::TA_STDDEV(m_discard, total - 1, src, n, nbdev, &outBegIdx, &outNbElement,
                dst + m_discard);
    HAYAKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);
}

Indicator HAYAKU_API TA_STDDEV(int n, double nbdev) {
    auto p = make_shared<TaStddev>();
    p->setParam<int>("n", n);
    p->setParam<double>("nbdev", nbdev);
    return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-22
 *      Author: fasiondog
 */


#include "operators/Indicator.h"

namespace hayaku {

class TaVar : public IndicatorImp {
    INDICATOR_IMP(TaVar)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaVar();
    virtual ~TaVar() = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-22
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::TaVar)
#endif

namespace hayaku {

TaVar::TaVar() : IndicatorImp("TA_VAR", 1) {
    setParam<int>("n", 5);
    setParam<double>("nbdev", 1.0);
}

void TaVar::_checkParam(const string& name) const {
    if (name == "n") {
        int n = getParam<int>(name);
        HAYAKU_CHECK(n >= 1 && n <= 100000, "n must >= 1 and <= 100000 ");
    } else if (name == "nbdev") {
        double nbdev = getParam<double>("nbdev");
        HAYAKU_ASSERT(!std::isnan(nbdev));
    }
}

void TaVar::_calculate(const Indicator& data) {
    int n = getParam<int>("n");
    double nbdev = getParam<double>("nbdev");
    size_t total = data.size();
    int lookback = TA_VAR_Lookback(n, nbdev);
    if (lookback < 0) {
        m_discard = total;
        return;
    }

    m_discard = data.discard() + lookback;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    const double* src = data.data();
    auto* dst = this->data();

    int outBegIdx;
    int outNbElement;
    ::TA_VAR(m_discard, total - 1, src, n, nbdev, &outBegIdx, &outNbElement, dst + m_discard);
    HAYAKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);
}

Indicator HAYAKU_API TA_VAR(int n, double nbdev) {
    auto p = make_shared<TaVar>();
    p->setParam<int>("n", n);
    p->setParam<double>("nbdev", nbdev);
    return Indicator(p);
}

} /* namespace hayaku */
