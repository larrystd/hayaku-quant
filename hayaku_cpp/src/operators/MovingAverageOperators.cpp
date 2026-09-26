#include "WindowOperators.h"

/*
 * IEma.h
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#pragma once

#include "Indicator.h"

namespace hayaku {

/*
 * Exponential Moving Average (EMA)
 * Parameters: n: the period window for calculating the average, it must be an integer greater than
 * 0 Discard number = 0
 */
class IEma : public IndicatorImp {
    INDICATOR_IMP(IEma)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IEma();
    virtual ~IEma() override;
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */

/*
 * IMa.h
 *
 *  Created on: 2013-2-10
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR__IMP_IMA_H_
#define INDICATOR__IMP_IMA_H_


namespace hayaku {

class IMa : public IndicatorImp {
    INDICATOR_IMP(IMa)
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IMa();
    virtual ~IMa() override;
    virtual void _checkParam(const string& name) const override;

    virtual bool supportIncrementCalculate() const override;
    virtual size_t min_increment_start() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

} /* namespace hayaku */
#endif /* INDICATOR__IMP_IMA_H_ */

/*
 * ISma.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-15
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ISMA_H_
#define INDICATOR_IMP_ISMA_H_


namespace hayaku {

class ISma : public IndicatorImp {
    INDICATOR_IMP(ISma)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISma();
    virtual ~ISma() override;

    virtual void _checkParam(const string& name) const override;
    virtual void _dyn_calculate(const Indicator&) override;
    virtual size_t min_increment_start() const override;

private:
    void _dyn_one_circle(const Indicator& ind, size_t curPos, int n, double m);
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_ISMA_H_ */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-15
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IWMA_H_
#define INDICATOR_IMP_IWMA_H_


namespace hayaku {

/*
 * WMA(X,N): the N-day weighted moving average of X. Algorithm:
 * Yn=(1*X1+2*X2+...+n*Xn)/(1+2+...+n).
 */
class IWma : public IndicatorImp {
    INDICATOR_IMP(IWma)
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IWma();
    virtual ~IWma() override;
    virtual void _checkParam(const string& name) const override;
    virtual bool supportIncrementCalculate() const override;
    virtual size_t min_increment_start() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_IWMA_H_ */

/*
 * IEma.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include "SeriesOperators.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IEma)
#endif

namespace hayaku {

IEma::IEma() : IndicatorImp("EMA", 1) {
    setParam<int>("n", 22);
}

IEma::~IEma() {}

void IEma::_checkParam(const string& name) const {
    if ("n" == name) {
        HAYAKU_ASSERT(getParam<int>("n") > 0);
    }
}

void IEma::_calculate(const Indicator& indicator) {
    size_t total = indicator.size();
    m_discard = indicator.discard();
    if (total <= m_discard) {
        m_discard = total;
        return;
    }

    auto const* src = indicator.data();
    auto* dst = this->data();

    int n = getParam<int>("n");
    size_t startPos = discard();
    dst[startPos] = src[startPos];

    value_t multiplier = 2.0 / (n + 1);
    for (size_t i = startPos + 1; i < total; ++i) {
        dst[i] = (src[i] - dst[i - 1]) * multiplier + dst[i - 1];
    }
}

void IEma::_increment_calculate(const Indicator& data, size_t start_pos) {
    int n = getParam<int>("n");
    value_t multiplier = 2.0 / (n + 1);

    auto const* src = data.data();
    auto* dst = this->data();

    size_t total = data.size();
    for (size_t i = start_pos + 1; i < total; ++i) {
        dst[i] = (src[i] - dst[i - 1]) * multiplier + dst[i - 1];
    }
}

void IEma::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    Indicator slice = SLICE(ind, 0, curPos + 1);
    Indicator ema = EMA(slice, step);
    if (ema.size() > 0) {
        _set(ema[ema.size() - 1], curPos);
    }
}

Indicator HAYAKU_API EMA(int n) {
    IndicatorImpPtr p = make_shared<IEma>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HAYAKU_API EMA(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IEma>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hayaku */

/*
 * IMa.cpp
 *
 *  Created on: 2013-2-10
 *      Author: fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IMa)
#endif

namespace hayaku {

IMa::IMa() : IndicatorImp("MA", 1) {
    setParam<int>("n", 22);
}

IMa::~IMa() {}

void IMa::_checkParam(const string& name) const {
    if ("n" == name) {
        HAYAKU_ASSERT(getParam<int>("n") >= 0);
    }
}

void IMa::_calculate(const Indicator& indicator) {
    size_t total = indicator.size();
    auto const* src = indicator.data();
    auto* dst = this->data();

    int n = getParam<int>("n");
    if (n <= 0) {
        m_discard = indicator.discard();
        if (m_discard >= total) {
            m_discard = total;
            return;
        }

        price_t sum = 0.0;
        size_t valid_count = 0;
        for (size_t i = m_discard; i < total; i++) {
            if (!std::isnan(src[i])) {
                sum += src[i];
                valid_count++;
                dst[i] = sum / valid_count;
            }
        }
        return;
    }

    m_discard = indicator.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    if (n == 1) {
        memcpy(dst + m_discard, src + m_discard, (total - m_discard) * sizeof(value_t));
        return;
    }

    // Welford rolling mean: the state is (valid_count, mean) and the enqueue / dequeue is O(1).
    // It shares the same valid_count increment / decrement logic with IStdev, guaranteeing that
    // MA/STDEV are based on exactly the same sample set.
    size_t startPos = indicator.discard();
    size_t valid_count = 0;
    price_t mean = 0.0;
    for (size_t i = startPos; i < total; ++i) {
        // Remove the leaving value (when the window is full)
        if (i >= static_cast<size_t>(startPos) + static_cast<size_t>(n)) {
            price_t leaving = src[i - n];
            if (!std::isnan(leaving)) {
                if (valid_count > 1) {
                    price_t delta = leaving - mean;
                    mean -= delta / (valid_count - 1);
                } else {
                    // valid_count == 1, the window becomes empty after the removal
                    mean = 0.0;
                }
                valid_count--;
            }
        }
        // Add the entering value
        price_t entering = src[i];
        if (!std::isnan(entering)) {
            valid_count++;
            if (valid_count == 1) {
                mean = entering;
            } else {
                price_t delta = entering - mean;
                mean += delta / valid_count;
            }
        }
        // Write no output when the window is not full or there is no valid value (the buffer is
        // already NaN)
        if (i >= m_discard && valid_count > 0) {
            dst[i] = mean;
        }
    }
}

bool IMa::supportIncrementCalculate() const {
    int n = getParam<int>("n");
    return n > 1;
}

size_t IMa::min_increment_start() const {
    int n = getParam<int>("n");
    return n;
}

void IMa::_increment_calculate(const Indicator& indicator, size_t startPos) {
    size_t total = indicator.size();
    auto const* src = indicator.data();
    auto* dst = this->data();

    int n = getParam<int>("n");
    HAYAKU_ASSERT(startPos + 1 >= (size_t)n);
    // Rebuild the Welford mean state with a single pass from the window start startPos+1-n, then
    // roll to total
    size_t start = startPos + 1 - n;
    size_t valid_count = 0;
    price_t mean = 0.0;
    for (size_t i = start; i < total; ++i) {
        if (i > startPos) {
            price_t leaving = src[i - n];
            if (!std::isnan(leaving)) {
                if (valid_count > 1) {
                    price_t delta = leaving - mean;
                    mean -= delta / (valid_count - 1);
                } else {
                    mean = 0.0;
                }
                valid_count--;
            }
        }
        price_t entering = src[i];
        if (!std::isnan(entering)) {
            valid_count++;
            if (valid_count == 1) {
                mean = entering;
            } else {
                price_t delta = entering - mean;
                mean += delta / valid_count;
            }
        }
        if (i >= startPos && valid_count > 0) {
            dst[i] = mean;
        }
    }
}

void IMa::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    if (curPos + 1 < ind.discard() + step) {
        return;
    }
    size_t start = _get_step_start(curPos, step, ind.discard());
    // A single pass Welford mean (the left boundary of the dynamic window jumps, so remove is not
    // used)
    size_t valid_count = 0;
    price_t mean = 0.0;
    for (size_t i = start; i <= curPos; i++) {
        if (!std::isnan(ind[i])) {
            valid_count++;
            if (valid_count == 1) {
                mean = ind[i];
            } else {
                price_t delta = ind[i] - mean;
                mean += delta / valid_count;
            }
        }
    }
    if (valid_count > 0) {
        _set(mean, curPos);
    }
}

Indicator HAYAKU_API MA(int n) {
    IndicatorImpPtr p = make_shared<IMa>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HAYAKU_API MA(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IMa>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hayaku */

/*
 * ISma.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-15
 *      Author: fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ISma)
#endif

namespace hayaku {

ISma::ISma() : IndicatorImp("SMA", 1) {
    setParam<int>("n", 22);
    setParam<double>("m", 2.0);
}

ISma::~ISma() {}

void ISma::_checkParam(const string& name) const {
    if ("n" == name) {
        HAYAKU_ASSERT(getParam<int>("n") >= 1);
    }
}

void ISma::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    m_discard = ind.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _set(ind[m_discard], m_discard);
    _increment_calculate(ind, m_discard);
}

size_t ISma::min_increment_start() const {
    return 1;
}

void ISma::_increment_calculate(const Indicator& data, size_t start_pos) {
    size_t total = data.size();
    double n = getParam<int>("n");
    double m = getParam<double>("m");

    auto const* src = data.data();
    auto* dst = this->data();

    double p = n - m;
    for (size_t i = m_discard + 1; i < total; i++) {
        dst[i] = (m * src[i] + p * dst[i - 1]) / n;
    }
}

void ISma::_dyn_one_circle(const Indicator& ind, size_t curPos, int n, double m) {
    HAYAKU_IF_RETURN(n < 1, void());
    Indicator slice = SLICE(ind, 0, curPos + 1);
    Indicator sma = SMA(slice, n, m);
    if (sma.size() > 0) {
        _set(sma[sma.size() - 1], curPos);
    }
}

void ISma::_dyn_calculate(const Indicator& ind) {
    auto iter = m_ind_params.find("n");
    Indicator n =
      iter != m_ind_params.end() ? Indicator(iter->second) : CVAL(ind, getParam<int>("n"));
    iter = m_ind_params.find("m");
    Indicator m =
      iter != m_ind_params.end() ? Indicator(iter->second) : CVAL(ind, getParam<int>("m"));

    HAYAKU_CHECK(n.size() == ind.size(), "ind_param(n).size()={}, ind.size()={}!", n.size(),
              ind.size());
    HAYAKU_CHECK(m.size() == ind.size(), "ind_param(m).size()={}, ind.size()={}!", m.size(),
              ind.size());

    m_discard = std::max(ind.discard(), n.discard());
    m_discard = std::max(m_discard, m.discard());
    size_t total = ind.size();
    HAYAKU_IF_RETURN(0 == total || m_discard >= total, void());

    global_parallel_for_index_void(
      ind.discard(), total, [&](size_t i) { _dyn_one_circle(ind, i, n[i], m[i]); }, 400);

    updateDiscard();
}

Indicator HAYAKU_API SMA(int n, double m) {
    IndicatorImpPtr p = make_shared<ISma>();
    p->setParam<int>("n", n);
    p->setParam<double>("m", m);
    return Indicator(p);
}

Indicator HAYAKU_API SMA(int n, const IndParam& m) {
    IndicatorImpPtr p = make_shared<ISma>();
    p->setParam<int>("n", n);
    p->setIndParam("m", m);
    return Indicator(p);
}

Indicator HAYAKU_API SMA(const IndParam& n, const IndParam& m) {
    IndicatorImpPtr p = make_shared<ISma>();
    p->setIndParam("n", n);
    p->setIndParam("m", m);
    return Indicator(p);
}

Indicator HAYAKU_API SMA(const IndParam& n, double m) {
    IndicatorImpPtr p = make_shared<ISma>();
    p->setIndParam("n", n);
    p->setParam<double>("m", m);
    return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-15
 *      Author: fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IWma)
#endif

namespace hayaku {

IWma::IWma() : IndicatorImp("WMA", 1) {
    setParam<int>("n", 22);
}

IWma::~IWma() {}

void IWma::_checkParam(const string& name) const {
    if ("n" == name) {
        HAYAKU_CHECK(getParam<int>("n") >= 1, "WMA n must >= 1!");
    }
}

void IWma::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    HAYAKU_IF_RETURN(total == 0, void());

    int n = getParam<int>("n");
    m_discard = ind.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst = this->data();

    if (n == 1) {
        memcpy(dst, src, total * sizeof(value_t));
        return;
    }

    value_t subsum = 0.0, sum = 0.0;
    for (size_t i = ind.discard(), end = m_discard + 1, count = 1; i < end; i++, count++) {
        subsum += src[i];
        sum += src[i] * count;
    }

    value_t divider = n * (n + 1) / 2.0;
    dst[m_discard] = sum / divider;

    size_t trailingIdx = ind.discard();
    for (size_t i = m_discard + 1; i < total; i++) {
        value_t tmp = src[i];
        sum -= subsum;
        subsum += tmp;
        subsum -= src[trailingIdx++];
        sum += tmp * n;
        dst[i] = sum / divider;
    }
}

bool IWma::supportIncrementCalculate() const {
    return getParam<int>("n") > 1;
}

size_t IWma::min_increment_start() const {
    return getParam<int>("n") - 1;
}

void IWma::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    int n = getParam<int>("n");
    auto const* src = ind.data();
    auto* dst = this->data();

    value_t subsum = 0.0, sum = 0.0;
    for (size_t i = start_pos - n, end = start_pos, count = 1; i < end; i++, count++) {
        subsum += src[i];
        sum += src[i] * count;
    }

    value_t divider = n * (n + 1) / 2.0;
    // dst[m_discard] = sum / divider;

    size_t trailingIdx = start_pos - n;
    for (size_t i = start_pos; i < total; i++) {
        value_t tmp = src[i];
        sum -= subsum;
        subsum += tmp;
        subsum -= src[trailingIdx++];
        sum += tmp * n;
        dst[i] = sum / divider;
    }
}

void IWma::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    if (step < 1) {
        _set(Null<value_t>(), curPos);
        return;
    }

    if (step == 1) {
        _set(ind[curPos], curPos);
        return;
    }

    size_t start = _get_step_start(curPos, step, ind.discard());
    if (curPos + 1 < step + start) {
        _set(Null<value_t>(), curPos);
        return;
    }

    value_t sum = 0.0;
    size_t n = 1;
    for (size_t i = start; i <= curPos; i++, n++) {
        sum += (ind[i] * n);
    }
    _set(sum / (step * (step + 1) / 2.), curPos);
}

Indicator HAYAKU_API WMA(int n) {
    IndicatorImpPtr p = make_shared<IWma>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HAYAKU_API WMA(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IWma>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hayaku */
