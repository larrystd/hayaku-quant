#include "ScalarMathOperators.h"

// ---- Merged implementation type from ICeil.h ----
/*
 * ICeil.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-15
 *      Author: fasiondog
 */
#include "Indicator.h"

namespace hayaku {

/**
 * Round up (round in the direction of increasing value) to an integer
 */
class ICeil : public IndicatorImp {
    INDICATOR_IMP(ICeil)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ICeil();
    virtual ~ICeil() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IFloor.h ----
/*
 * IFloor.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-15
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Round down (round in the direction of decreasing value) to an integer
 */
class IFloor : public IndicatorImp {
    INDICATOR_IMP(IFloor)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IFloor();
    virtual ~IFloor() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IIntpart.h ----
/*
 * IIntpart.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-18
 *      Author: fasiondog
 */

namespace hayaku {

/*
 * Round to an integer (rounded toward a smaller absolute value, i.e. the integer part of the data)
 */
class IIntpart : public IndicatorImp {
    INDICATOR_IMP(IIntpart)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IIntpart();
    virtual ~IIntpart() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IRound.h ----
/*
 * IRound.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Rounding
 */
class IRound : public IndicatorImp {
    INDICATOR_IMP(IRound)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IRound();
    virtual ~IRound() override;
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IRoundDown.h ----
/*
 * IRoundDown.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Truncate downward, e.g. 10.1 is truncated to 10
 */
class IRoundDown : public IndicatorImp {
    INDICATOR_IMP(IRoundDown)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IRoundDown();
    virtual ~IRoundDown() override;
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IRoundUp.h ----
/*
 * IRoundUp.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Truncate upward, e.g. 10.1 is truncated to 11
 */
class IRoundUp : public IndicatorImp {
    INDICATOR_IMP(IRoundUp)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IRoundUp();
    virtual ~IRoundUp() override;
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */

// ---- Merged implementation from ICeil.cpp ----
/*
 * ICeil.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-04-14
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ICeil)
#endif

namespace hayaku {

ICeil::ICeil() : IndicatorImp("CEILING", 1) {}

ICeil::~ICeil() {}

void ICeil::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void ICeil::_increment_calculate(const Indicator &data, size_t start_pos) {
    auto const *src = data.data();
    auto *dst = this->data();
    for (size_t i = start_pos, end = data.size(); i < end; ++i) {
        dst[i] = std::ceil(src[i]);
    }
}

Indicator HAYAKU_API CEILING() {
    return Indicator(make_shared<ICeil>());
}

} /* namespace hayaku */

// ---- Merged implementation from IFloor.cpp ----
/*
 * IFloor.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-04-14
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IFloor)
#endif

namespace hayaku {

IFloor::IFloor() : IndicatorImp("FLOOR", 1) {}

IFloor::~IFloor() {}

void IFloor::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void IFloor::_increment_calculate(const Indicator& data, size_t start_pos) {
    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = start_pos, end = data.size(); i < end; ++i) {
        dst[i] = std::floor(src[i]);
    }
}

Indicator HAYAKU_API FLOOR() {
    return Indicator(make_shared<IFloor>());
}

} /* namespace hayaku */

// ---- Merged implementation from IIntpart.cpp ----
/*
 * IIntpart.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-04-18
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IIntpart)
#endif

namespace hayaku {

IIntpart::IIntpart() : IndicatorImp("INTPART", 1) {}

IIntpart::~IIntpart() {}

void IIntpart::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void IIntpart::_increment_calculate(const Indicator& data, size_t start_pos) {
    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = start_pos, end = data.size(); i < end; ++i) {
        dst[i] = int(src[i]);
    }
}

Indicator HAYAKU_API INTPART() {
    return Indicator(make_shared<IIntpart>());
}

} /* namespace hayaku */

// ---- Merged implementation from IRound.cpp ----
/*
 * IRound.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-04-14
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IRound)
#endif

namespace hayaku {

IRound::IRound() : IndicatorImp("ROUND", 1) {
    setParam<int>("ndigits", 2);
}

IRound::~IRound() {}

void IRound::_checkParam(const string& name) const {
    if ("ndigits" == name) {
        HAYAKU_ASSERT(getParam<int>("ndigits") >= 0);
    }
}

void IRound::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void IRound::_increment_calculate(const Indicator& data, size_t start_pos) {
    int n = getParam<int>("ndigits");
    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = start_pos, total = data.size(); i < total; ++i) {
        dst[i] = roundEx(src[i], n);
    }
}

Indicator HAYAKU_API ROUND(int ndigits) {
    IndicatorImpPtr p = make_shared<IRound>();
    p->setParam<int>("ndigits", ndigits);
    return Indicator(p);
}

} /* namespace hayaku */

// ---- Merged implementation from IRoundDown.cpp ----
/*
 * IRoundDown.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-04-14
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IRoundDown)
#endif

namespace hayaku {

IRoundDown::IRoundDown() : IndicatorImp("ROUNDDOWN", 1) {
    setParam<int>("ndigits", 2);
}

IRoundDown::~IRoundDown() {}

void IRoundDown::_checkParam(const string& name) const {
    if ("ndigits" == name) {
        HAYAKU_ASSERT(getParam<int>("ndigits") >= 0);
    }
}

void IRoundDown::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void IRoundDown::_increment_calculate(const Indicator& data, size_t start_pos) {
    int n = getParam<int>("ndigits");
    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = start_pos, total = data.size(); i < total; ++i) {
        dst[i] = roundDown(src[i], n);
    }
}

Indicator HAYAKU_API ROUNDDOWN(int ndigits) {
    IndicatorImpPtr p = make_shared<IRoundDown>();
    p->setParam<int>("ndigits", ndigits);
    return Indicator(p);
}

} /* namespace hayaku */

// ---- Merged implementation from IRoundUp.cpp ----
/*
 * IRoundUp.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-04-14
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IRoundUp)
#endif

namespace hayaku {

IRoundUp::IRoundUp() : IndicatorImp("ROUNDUP", 1) {
    setParam<int>("ndigits", 2);
}

IRoundUp::~IRoundUp() {}

void IRoundUp::_checkParam(const string& name) const {
    if ("ndigits" == name) {
        HAYAKU_ASSERT(getParam<int>("ndigits") >= 0);
    }
}

void IRoundUp::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void IRoundUp::_increment_calculate(const Indicator& data, size_t start_pos) {
    int n = getParam<int>("ndigits");
    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = start_pos, total = data.size(); i < total; ++i) {
        dst[i] = roundUp(src[i], n);
    }
}

Indicator HAYAKU_API ROUNDUP(int ndigits) {
    IndicatorImpPtr p = make_shared<IRoundUp>();
    p->setParam<int>("ndigits", ndigits);
    return Indicator(p);
}

} /* namespace hayaku */
