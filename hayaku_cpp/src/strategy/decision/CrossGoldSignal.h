#pragma once

/*
 * CrossGoldSignal.h
 *
 *  Created on: 2017-6-13
 *      Author: fasiondog
 */


#include "operators/Indicator.h"
#include "SignalBase.h"

namespace hayaku {

class CrossGoldSignal : public SignalBase {
public:
    CrossGoldSignal();
    CrossGoldSignal(const Indicator& fast, const Indicator& slow);
    virtual ~CrossGoldSignal();

    virtual SignalPtr _clone() override;
    virtual void _calculate(const KData& kdata) override;

private:
    Indicator m_fast;
    Indicator m_slow;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);
        ar& BOOST_SERIALIZATION_NVP(m_fast);
        ar& BOOST_SERIALIZATION_NVP(m_slow);
    }
#endif
};

} /* namespace hayaku */
