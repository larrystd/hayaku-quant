#include "SignalExpressions.h"

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::AndSignal)
#endif

namespace hayaku {

void AndSignal::_calculate(const KData& kdata) {
    HAYAKU_IF_RETURN(!m_sg1 || !m_sg2, void());

    auto const* ks = kdata.data();
    size_t total = kdata.size();

    sub_sg_calculate(m_sg1, kdata);
    sub_sg_calculate(m_sg2, kdata);
    for (size_t i = 0; i < total; ++i) {
        double buy_value = m_sg1->getBuyValue(ks[i].datetime) * m_sg2->getBuyValue(ks[i].datetime);
        double sell_value =
          0.0 - m_sg1->getSellValue(ks[i].datetime) * m_sg2->getSellValue(ks[i].datetime);
        auto value = buy_value + sell_value;
        if (value > 0.0) {
            _addBuySignal(ks[i].datetime);
        } else if (value < 0.0) {
            _addSellSignal(ks[i].datetime);
        }
    }
}

HAYAKU_API SignalPtr operator&(const SignalPtr& sg1, const SignalPtr& sg2) {
    return make_shared<AndSignal>(sg1, sg2);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OrSignal)
#endif

namespace hayaku {

void OrSignal::_calculate(const KData& kdata) {
    HAYAKU_IF_RETURN(!m_sg1 && !m_sg2, void());

    auto const* ks = kdata.data();
    size_t total = kdata.size();

    if (m_sg1 && !m_sg2) {
        sub_sg_calculate(m_sg1, kdata);
        for (size_t i = 0; i < total; ++i) {
            auto value = m_sg1->getValue(ks[i].datetime);
            if (value > 0.0) {
                _addBuySignal(ks[i].datetime);
            } else if (value < 0.0) {
                _addSellSignal(ks[i].datetime);
            }
        }
        return;
    }

    if (!m_sg1 && m_sg2) {
        sub_sg_calculate(m_sg2, kdata);
        for (size_t i = 0; i < total; i++) {
            auto value = m_sg2->getValue(ks[i].datetime);
            if (value > 0.0) {
                _addBuySignal(ks[i].datetime);
            } else if (value < 0.0) {
                _addSellSignal(ks[i].datetime);
            }
        }
        return;
    }

    sub_sg_calculate(m_sg1, kdata);
    sub_sg_calculate(m_sg2, kdata);
    for (size_t i = 0; i < total; ++i) {
        double value = m_sg1->getValue(ks[i].datetime) + m_sg2->getValue(ks[i].datetime);
        if (value > 0.0) {
            _addBuySignal(ks[i].datetime);
        } else if (value < 0.0) {
            _addSellSignal(ks[i].datetime);
        }
    }
}

HAYAKU_API SignalPtr operator|(const SignalPtr& sg1, const SignalPtr& sg2) {
    return make_shared<OrSignal>(sg1, sg2);
}

} /* namespace hayaku */
