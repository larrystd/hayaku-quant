#pragma once

/*
 * TwoLineEnviroment.h
 *
 *  Created on: 2016-5-17
 *      Author: Administrator
 */


#include "operators/Indicator.h"
#include "EnvironmentBase.h"

namespace hayaku {

class TwoLineEnvironment : public EnvironmentBase {
public:
    TwoLineEnvironment();
    TwoLineEnvironment(const Indicator& fast, const Indicator& slow);
    virtual ~TwoLineEnvironment();

    virtual void _checkParam(const string& name) const override;
    virtual void _calculate() override;
    virtual EnvironmentPtr _clone() override;

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
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase);
        ar& BOOST_SERIALIZATION_NVP(m_fast);
        ar& BOOST_SERIALIZATION_NVP(m_slow);
    }
#endif
};

} /* namespace hayaku */
