#include "WindowOperators.h"

/*
 * ICount.h
 *
 *  Created on: 2019-3-25
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ICOUNT_H_
#define INDICATOR_IMP_ICOUNT_H_

#include "Indicator.h"

namespace hayaku {

/*
 * Count the total number; count the number of the periods satisfying the condition.
 * COUNT(X,N) counts the number of the periods satisfying the X condition within N periods; if N=0
 * it starts from the first valid value.
 * COUNT(CLOSE>OPEN,20) counts the number of the periods closing up within 20 periods
 */
class ICount : public IndicatorImp {
    INDICATOR_IMP(ICount)
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ICount();
    virtual ~ICount() override;
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_ICOUNT_H_ */

/*
 * ISum.h
 *
 *  Created on: 2019-4-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_HAYAKU_ISUM_H_
#define INDICATOR_IMP_HAYAKU_ISUM_H_


namespace hayaku {

class ISum : public IndicatorImp {
    INDICATOR_IMP(ISum)
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISum();
    virtual ~ISum() override;
    virtual void _checkParam(const string& name) const override;

    virtual bool supportIncrementCalculate() const override;
    virtual size_t min_increment_start() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_HAYAKU_ISUM_H_ */

/*
 * IDevsq.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-16
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IDEVSQ_H_
#define INDICATOR_IMP_IDEVSQ_H_


namespace hayaku {

class IDevsq : public hayaku::IndicatorImp {
    INDICATOR_IMP(IDevsq)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IDevsq();
    virtual ~IDevsq() override;
    virtual void _checkParam(const string& name) const override;
    virtual size_t min_increment_start() const override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_IDEVSQ_H_ */

/*
 * Icount.cpp
 *
 *  Created on: 2019-3-25
 *      Author: fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ICount)
#endif

namespace hayaku {

ICount::ICount() : IndicatorImp("COUNT", 1) {
    setParam<int>("n", 20);
}

ICount::~ICount() {}

void ICount::_checkParam(const string& name) const {
    if ("n" == name) {
        HAYAKU_ASSERT(getParam<int>("n") >= 0);
    }
}

void ICount::_calculate(const Indicator& data) {
    size_t total = data.size();
    if (0 == total) {
        m_discard = 0;
        return;
    }

    if (data.discard() >= total) {
        m_discard = total;
        return;
    }

    int n = getParam<int>("n");

    auto const* src = data.data();
    auto* dst = this->data();

    if (0 == n) {
        m_discard = data.discard();
        int count = 0;
        for (size_t i = m_discard; i < total; ++i) {
            if (src[i] != 0) {
                count++;
            }
            dst[i] = count;
        }
        return;
    }

    m_discard = data.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    size_t startPos = data.discard();
    int sum = 0;
    size_t first_end = startPos + n >= total ? total : startPos + n;
    for (size_t i = startPos; i < first_end; ++i) {
        if (src[i] != 0) {
            sum++;
        }
    }

    if (first_end >= 1) {
        dst[first_end - 1] = sum;
    }

    for (size_t i = first_end; i < total; ++i) {
        if (src[i] != 0) {
            sum++;
        }
        if (src[i - n] != 0) {
            sum--;
        }
        dst[i] = sum;
    }
}

void ICount::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    size_t start = 0;
    if (0 == step) {
        start = ind.discard();
    } else if (curPos < ind.discard() + step - 1) {
        return;
    } else {
        start = curPos + 1 - step;
    }
    price_t count = 0;
    for (size_t i = start; i <= curPos; i++) {
        if (ind[i] != 0.0) {
            count++;
        }
    }
    _set(count, curPos);
}

Indicator HAYAKU_API COUNT(int n) {
    IndicatorImpPtr p = make_shared<ICount>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HAYAKU_API COUNT(const IndParam& n) {
    IndicatorImpPtr p = make_shared<ICount>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hayaku */

/*
 * ISum.cpp
 *
 *  Created on: 2019-4-1
 *      Author: fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ISum)
#endif

namespace hayaku {

ISum::ISum() : IndicatorImp("SUM", 1) {
    setParam<int>("n", 20);
}

ISum::~ISum() {}

void ISum::_checkParam(const string& name) const {
    if ("n" == name) {
        HAYAKU_ASSERT(getParam<int>("n") >= 0);
    }
}

void ISum::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    if (0 == total || ind.discard() >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst = this->data();

    int n = getParam<int>("n");
    if (n <= 0) {
        m_discard = ind.discard();
        price_t sum = 0;
        for (size_t i = m_discard; i < total; i++) {
            sum += src[i];
            dst[i] = sum;
        }
        return;
    }

    m_discard = ind.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    if (n == 1) {
        memcpy(dst + m_discard, src + m_discard, (total - m_discard) * sizeof(value_t));
        return;
    }

    _increment_calculate(ind, m_discard);
    return;
}

bool ISum::supportIncrementCalculate() const {
    return getParam<int>("n") > 1;
}

size_t ISum::min_increment_start() const {
    return getParam<int>("n") - 1;
}

void ISum::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    auto const* src = ind.data();
    auto* dst = this->data();

    int n = getParam<int>("n");
    price_t sum = 0.0;
    for (size_t i = start_pos + 1 - n; i <= start_pos; i++) {
        sum += src[i];
    }
    dst[start_pos] = sum;

    for (size_t i = start_pos + 1; i < total; i++) {
        sum = sum - src[i - n] + src[i];
        dst[i] = sum;
    }
}

void ISum::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    size_t start = _get_step_start(curPos, step, ind.discard());
    if (curPos + 1 < ind.discard() + step) {
        return;
    }
    price_t sum = 0.0;
    for (size_t i = start; i <= curPos; i++) {
        sum += ind[i];
    }
    _set(sum, curPos);
}

Indicator HAYAKU_API SUM(int n) {
    IndicatorImpPtr p = make_shared<ISum>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HAYAKU_API SUM(const IndParam& n) {
    IndicatorImpPtr p = make_shared<ISum>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hayaku */

/*
 * IDevsq.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-4-18
 *      Author: fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IDevsq)
#endif

namespace hayaku {

IDevsq::IDevsq() : IndicatorImp("DEVSQ", 1) {
    setParam<int>("n", 10);
}

IDevsq::~IDevsq() {}

void IDevsq::_checkParam(const string& name) const {
    if ("n" == name) {
        HAYAKU_ASSERT(getParam<int>("n") >= 2);
    }
}

void IDevsq::_calculate(const Indicator& data) {
    size_t total = data.size();
    int n = getParam<int>("n");

    m_discard = data.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

size_t IDevsq::min_increment_start() const {
    return getParam<int>("n");
}

void IDevsq::_increment_calculate(const Indicator& data, size_t start_pos) {
    size_t total = data.size();
    int n = getParam<int>("n");

    auto const* src = data.data();

    std::vector<price_t> ma(total);
    size_t start = start_pos + 1 - n;
    price_t sum = 0.0;
    for (size_t i = start; i <= start_pos; ++i) {
        if (!std::isnan(src[i])) {
            sum += src[i];
        }
    }

    ma[start_pos] = sum / n;
    for (size_t i = start_pos + 1; i < total; ++i) {
        if (!std::isnan(src[i]) && !std::isnan(src[i - n])) {
            sum = src[i] + sum - src[i - n];
            ma[i] = sum / n;
        }
    }

    auto const* mean = ma.data();
    auto* dst = this->data();
    for (size_t i = start_pos; i < total; ++i) {
        sum = 0.0;
        start = i + 1 - n;
        for (size_t j = start; j <= i; ++j) {
            sum += std::pow(src[j] - mean[i], 2);
        }
        dst[i] = sum;
    }
}

void IDevsq::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    size_t start = _get_step_start(curPos, step, ind.discard());
    if (curPos + 1 < ind.discard() + step) {
        return;
    }
    price_t sum = 0.0;
    for (size_t i = start; i <= curPos; i++) {
        sum += ind[i];
    }
    price_t mean = sum / (curPos - start + 1);
    sum = 0.0;
    for (size_t i = start; i <= curPos; i++) {
        sum += std::pow(ind[i] - mean, 2);
    }
    _set(sum, curPos);
}

Indicator HAYAKU_API DEVSQ(int n) {
    IndicatorImpPtr p = make_shared<IDevsq>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HAYAKU_API DEVSQ(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IDevsq>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hayaku */
