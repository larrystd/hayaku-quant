#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-13
 *      Author: fasiondog
 */


#include "operators/Indicator.h"
#include "SignalBase.h"

namespace hayaku {

// Build a one-sided signal (containing the buy or the sell signals only) from the input indicator;
// a signal is added if the indicator value is greater than 0
class OneSideSignal : public SignalBase {
public:
    OneSideSignal();
    OneSideSignal(const Indicator& ind, bool is_buy);
    virtual ~OneSideSignal();

    virtual SignalPtr _clone() override;
    virtual void _calculate(const KData& kdata) override;
    virtual void _checkParam(const string& name) const override;

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
