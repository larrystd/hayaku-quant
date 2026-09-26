#include "ConditionExpressions.h"

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::AddCondition)
#endif

namespace hayaku {

AddCondition::AddCondition() : ConditionBase("CN_Add") {}

AddCondition::AddCondition(const ConditionPtr& cond1, const ConditionPtr& cond2)
: ConditionBase("CN_Add") {
    if (cond1) {
        m_cond1 = cond1->clone();
    }
    if (cond2) {
        m_cond2 = cond2->clone();
    }
}

AddCondition::~AddCondition() {}

void AddCondition::_calculate() {
    HAYAKU_IF_RETURN(!m_cond1 && !m_cond2, void());

    if (m_cond1) {
        m_cond1->setAccount(m_account);
        m_cond1->setSG(m_sg);
        m_cond1->setTO(m_kdata);
    }

    if (m_cond2) {
        m_cond2->setAccount(m_account);
        m_cond2->setSG(m_sg);
        m_cond2->setTO(m_kdata);
    }

    if (m_cond1 && !m_cond2) {
        price_t const* data = m_cond1->data();
        for (size_t i = 0, total = m_cond1->size(); i < total; i++) {
            m_values[i] = data[i];
        }
        return;
    }

    if (!m_cond1 && m_cond2) {
        auto const* data = m_cond2->data();
        for (size_t i = 0, total = m_cond2->size(); i < total; i++) {
            m_values[i] = data[i];
        }
        return;
    }

    size_t total = m_kdata.size();
    HAYAKU_ASSERT(m_cond1->size() == total && m_cond2->size() == total);

    auto const* data1 = m_cond1->data();
    auto const* data2 = m_cond2->data();
    for (size_t i = 0; i < total; i++) {
        m_values[i] = data1[i] + data2[i];
    }
}

void AddCondition::_reset() {
    if (m_cond1) {
        m_cond1->reset();
    }
    if (m_cond2) {
        m_cond2->reset();
    }
}

ConditionPtr AddCondition::_clone() {
    auto p = make_shared<AddCondition>();
    if (m_cond1) {
        p->m_cond1 = m_cond1->clone();
    }
    if (m_cond2) {
        p->m_cond2 = m_cond2->clone();
    }
    return p;
}

HAYAKU_API ConditionPtr operator+(const ConditionPtr& cond1, const ConditionPtr& cond2) {
    return make_shared<AddCondition>(cond1, cond2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-02-16
 *      Author: fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::AndCondition)
#endif

namespace hayaku {

AndCondition::AndCondition() : ConditionBase("CN_And") {}

AndCondition::AndCondition(const ConditionPtr& cond1, const ConditionPtr& cond2)
: ConditionBase("CN_And") {
    if (cond1) {
        m_cond1 = cond1->clone();
    }
    if (cond2) {
        m_cond2 = cond2->clone();
    }
}

AndCondition::~AndCondition() {}

void AndCondition::_calculate() {
    HAYAKU_IF_RETURN(!m_cond1 || !m_cond2, void());
    m_cond1->setAccount(m_account);
    m_cond2->setAccount(m_account);
    m_cond1->setSG(m_sg);
    m_cond2->setSG(m_sg);
    m_cond1->setTO(m_kdata);
    m_cond2->setTO(m_kdata);

    size_t total = m_kdata.size();
    HAYAKU_ASSERT(m_cond1->size() == total && m_cond2->size() == total);

    auto const* data1 = m_cond1->data();
    auto const* data2 = m_cond2->data();
    for (size_t i = 0; i < total; i++) {
        m_values[i] = (data1[i] > 0.0 && data2[i] > 0.0) ? 1.0 : 0.0;
    }
}

void AndCondition::_reset() {
    if (m_cond1) {
        m_cond1->reset();
    }
    if (m_cond2) {
        m_cond2->reset();
    }
}

ConditionPtr AndCondition::_clone() {
    auto p = make_shared<AndCondition>();
    if (m_cond1) {
        p->m_cond1 = m_cond1->clone();
    }
    if (m_cond2) {
        p->m_cond2 = m_cond2->clone();
    }
    return p;
}

HAYAKU_API ConditionPtr operator&(const ConditionPtr& cond1, const ConditionPtr& cond2) {
    return make_shared<AndCondition>(cond1, cond2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::DivCondition)
#endif

namespace hayaku {

DivCondition::DivCondition() : ConditionBase("CN_Div") {}

DivCondition::DivCondition(const ConditionPtr& cond1, const ConditionPtr& cond2)
: ConditionBase("CN_Div") {
    if (cond1) {
        m_cond1 = cond1->clone();
    }
    if (cond2) {
        m_cond2 = cond2->clone();
    }
}

DivCondition::~DivCondition() {}

void DivCondition::_calculate() {
    HAYAKU_IF_RETURN(!m_cond1, void());

    m_cond1->setAccount(m_account);
    m_cond1->setSG(m_sg);
    m_cond1->setTO(m_kdata);

    price_t null_price = Null<price_t>();
    if (!m_cond2) {
        for (size_t i = 0, total = m_cond1->size(); i < total; i++) {
            m_values[i] = null_price;
        }
        return;
    }

    m_cond2->setAccount(m_account);
    m_cond2->setSG(m_sg);
    m_cond2->setTO(m_kdata);

    size_t total = m_kdata.size();
    HAYAKU_ASSERT(m_cond1->size() == total && m_cond2->size() == total);

    auto const* data1 = m_cond1->data();
    auto const* data2 = m_cond2->data();
    for (size_t i = 0; i < total; i++) {
        m_values[i] = data2[i] == 0.0 || std::isnan(data2[i]) ? null_price : data1[i] / data2[i];
    }
}

void DivCondition::_reset() {
    if (m_cond1) {
        m_cond1->reset();
    }
    if (m_cond2) {
        m_cond2->reset();
    }
}

ConditionPtr DivCondition::_clone() {
    auto p = make_shared<DivCondition>();
    if (m_cond1) {
        p->m_cond1 = m_cond1->clone();
    }
    if (m_cond2) {
        p->m_cond2 = m_cond2->clone();
    }
    return p;
}

HAYAKU_API ConditionPtr operator/(const ConditionPtr& cond1, const ConditionPtr& cond2) {
    return make_shared<DivCondition>(cond1, cond2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::MultiCondition)
#endif

namespace hayaku {

MultiCondition::MultiCondition() : ConditionBase("CN_Multi") {}

MultiCondition::MultiCondition(const ConditionPtr& cond1, const ConditionPtr& cond2)
: ConditionBase("CN_Multi") {
    if (cond1) {
        m_cond1 = cond1->clone();
    }
    if (cond2) {
        m_cond2 = cond2->clone();
    }
}

MultiCondition::~MultiCondition() {}

void MultiCondition::_calculate() {
    HAYAKU_IF_RETURN(!m_cond1 || !m_cond2, void());

    m_cond1->setAccount(m_account);
    m_cond2->setAccount(m_account);
    m_cond1->setSG(m_sg);
    m_cond2->setSG(m_sg);
    m_cond1->setTO(m_kdata);
    m_cond2->setTO(m_kdata);

    size_t total = m_kdata.size();
    HAYAKU_ASSERT(m_cond1->size() == total && m_cond2->size() == total);

    auto const* data1 = m_cond1->data();
    auto const* data2 = m_cond2->data();
    for (size_t i = 0; i < total; i++) {
        m_values[i] = data1[i] * data2[i];
    }
}

void MultiCondition::_reset() {
    if (m_cond1) {
        m_cond1->reset();
    }
    if (m_cond2) {
        m_cond2->reset();
    }
}

ConditionPtr MultiCondition::_clone() {
    auto p = make_shared<MultiCondition>();
    if (m_cond1) {
        p->m_cond1 = m_cond1->clone();
    }
    if (m_cond2) {
        p->m_cond2 = m_cond2->clone();
    }
    return p;
}

HAYAKU_API ConditionPtr operator*(const ConditionPtr& cond1, const ConditionPtr& cond2) {
    return make_shared<MultiCondition>(cond1, cond2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OrCondition)
#endif

namespace hayaku {

OrCondition::OrCondition() : ConditionBase("CN_Or") {}

OrCondition::OrCondition(const ConditionPtr& cond1, const ConditionPtr& cond2)
: ConditionBase("CN_Or") {
    if (cond1) {
        m_cond1 = cond1->clone();
    }
    if (cond2) {
        m_cond2 = cond2->clone();
    }
}

OrCondition::~OrCondition() {}

void OrCondition::_calculate() {
    HAYAKU_IF_RETURN(!m_cond1 && !m_cond2, void());

    if (m_cond1) {
        m_cond1->setAccount(m_account);
        m_cond1->setSG(m_sg);
        m_cond1->setTO(m_kdata);
    }

    if (m_cond2) {
        m_cond2->setAccount(m_account);
        m_cond2->setSG(m_sg);
        m_cond2->setTO(m_kdata);
    }

    if (m_cond1 && !m_cond2) {
        auto const* data = m_cond1->data();
        for (size_t i = 0, total = m_cond1->size(); i < total; i++) {
            if (data[i] > 0.0) {
                m_values[i] = 1.0;
            }
        }
        return;
    }

    if (!m_cond1 && m_cond2) {
        auto const* data = m_cond2->data();
        for (size_t i = 0, total = m_cond2->size(); i < total; i++) {
            if (data[i] > 0.0) {
                m_values[i] = 1.0;
            }
        }
        return;
    }

    size_t total = m_kdata.size();
    HAYAKU_ASSERT(m_cond1->size() == total && m_cond2->size() == total);

    auto const* data1 = m_cond1->data();
    auto const* data2 = m_cond2->data();
    for (size_t i = 0; i < total; i++) {
        if (data1[i] > 0. || data2[i] > 0.) {
            m_values[i] = 1.0;
        }
    }
}

void OrCondition::_reset() {
    if (m_cond1) {
        m_cond1->reset();
    }
    if (m_cond2) {
        m_cond2->reset();
    }
}

ConditionPtr OrCondition::_clone() {
    auto p = make_shared<OrCondition>();
    if (m_cond1) {
        p->m_cond1 = m_cond1->clone();
    }
    if (m_cond2) {
        p->m_cond2 = m_cond2->clone();
    }
    return p;
}

HAYAKU_API ConditionPtr operator|(const ConditionPtr& cond1, const ConditionPtr& cond2) {
    return make_shared<OrCondition>(cond1, cond2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::SubCondition)
#endif

namespace hayaku {

SubCondition::SubCondition() : ConditionBase("CN_Sub") {}

SubCondition::SubCondition(const ConditionPtr& cond1, const ConditionPtr& cond2)
: ConditionBase("CN_Sub") {
    if (cond1) {
        m_cond1 = cond1->clone();
    }
    if (cond2) {
        m_cond2 = cond2->clone();
    }
}

SubCondition::~SubCondition() {}

void SubCondition::_calculate() {
    HAYAKU_IF_RETURN(!m_cond1 && !m_cond2, void());

    if (m_cond1) {
        m_cond1->setAccount(m_account);
        m_cond1->setSG(m_sg);
        m_cond1->setTO(m_kdata);
    }

    if (m_cond2) {
        m_cond2->setAccount(m_account);
        m_cond2->setSG(m_sg);
        m_cond2->setTO(m_kdata);
    }

    if (m_cond1 && !m_cond2) {
        auto const* data = m_cond1->data();
        for (size_t i = 0, total = m_cond1->size(); i < total; i++) {
            m_values[i] = data[i];
        }
        return;
    }

    if (!m_cond1 && m_cond2) {
        auto const* data = m_cond2->data();
        for (size_t i = 0, total = m_cond2->size(); i < total; i++) {
            m_values[i] = -data[i];
        }
        return;
    }

    size_t total = m_kdata.size();
    HAYAKU_ASSERT(m_cond1->size() == total && m_cond2->size() == total);

    auto const* data1 = m_cond1->data();
    auto const* data2 = m_cond2->data();
    for (size_t i = 0; i < total; i++) {
        m_values[i] = data1[i] - data2[i];
    }
}

void SubCondition::_reset() {
    if (m_cond1) {
        m_cond1->reset();
    }
    if (m_cond2) {
        m_cond2->reset();
    }
}

ConditionPtr SubCondition::_clone() {
    auto p = make_shared<SubCondition>();
    if (m_cond1) {
        p->m_cond1 = m_cond1->clone();
    }
    if (m_cond2) {
        p->m_cond2 = m_cond2->clone();
    }
    return p;
}

HAYAKU_API ConditionPtr operator-(const ConditionPtr& cond1, const ConditionPtr& cond2) {
    return make_shared<SubCondition>(cond1, cond2);
}

}  // namespace hayaku
