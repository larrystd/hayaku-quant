#pragma once

/*
 * SingleSignal2.h
 *
 *  Created on: 2016-4-16
 *      Author: fasiondog
 */


#include "operators/Indicator.h"
#include "SignalBase.h"

namespace hayaku {

class SingleSignal2 : public SignalBase {
public:
    SingleSignal2();
    explicit SingleSignal2(const Indicator&);
    virtual ~SingleSignal2();

    virtual void _checkParam(const string& name) const override;
    virtual SignalPtr _clone() override;
    virtual void _calculate(const KData& kdata) override;

private:
    Indicator m_ind;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);
        ar& BOOST_SERIALIZATION_NVP(m_ind);
    }
#endif
};

} /* namespace hayaku */
