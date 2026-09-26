#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */


#include "strategy/decision/EnvironmentBase.h"

namespace hayaku {

class HAYAKU_API AddEnvironment : public EnvironmentBase {
public:
    AddEnvironment();
    AddEnvironment(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
    virtual ~AddEnvironment();

    virtual void _calculate() override;
    virtual void _reset() override;
    virtual EnvironmentPtr _clone() override;

private:
    EnvironmentPtr m_ev1;
    EnvironmentPtr m_ev2;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase);
        ar& BOOST_SERIALIZATION_NVP(m_ev1);
        ar& BOOST_SERIALIZATION_NVP(m_ev2);
    }
#endif
};

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */



namespace hayaku {

class HAYAKU_API AndEnvironment : public EnvironmentBase {
public:
    AndEnvironment();
    AndEnvironment(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
    virtual ~AndEnvironment();

    virtual void _calculate() override;
    virtual void _reset() override;
    virtual EnvironmentPtr _clone() override;

private:
    EnvironmentPtr m_ev1;
    EnvironmentPtr m_ev2;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase);
        ar& BOOST_SERIALIZATION_NVP(m_ev1);
        ar& BOOST_SERIALIZATION_NVP(m_ev2);
    }
#endif
};

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */



namespace hayaku {

class HAYAKU_API DivEnvironment : public EnvironmentBase {
public:
    DivEnvironment();
    DivEnvironment(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
    virtual ~DivEnvironment();

    virtual void _calculate() override;
    virtual void _reset() override;
    virtual EnvironmentPtr _clone() override;

private:
    EnvironmentPtr m_ev1;
    EnvironmentPtr m_ev2;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase);
        ar& BOOST_SERIALIZATION_NVP(m_ev1);
        ar& BOOST_SERIALIZATION_NVP(m_ev2);
    }
#endif
};

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */



namespace hayaku {

class HAYAKU_API MultiEnvironment : public EnvironmentBase {
public:
    MultiEnvironment();
    MultiEnvironment(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
    virtual ~MultiEnvironment();

    virtual void _calculate() override;
    virtual void _reset() override;
    virtual EnvironmentPtr _clone() override;

private:
    EnvironmentPtr m_ev1;
    EnvironmentPtr m_ev2;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase);
        ar& BOOST_SERIALIZATION_NVP(m_ev1);
        ar& BOOST_SERIALIZATION_NVP(m_ev2);
    }
#endif
};

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */



namespace hayaku {

class HAYAKU_API OrEnvironment : public EnvironmentBase {
public:
    OrEnvironment();
    OrEnvironment(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
    virtual ~OrEnvironment();

    virtual void _calculate() override;
    virtual void _reset() override;
    virtual EnvironmentPtr _clone() override;

private:
    EnvironmentPtr m_ev1;
    EnvironmentPtr m_ev2;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase);
        ar& BOOST_SERIALIZATION_NVP(m_ev1);
        ar& BOOST_SERIALIZATION_NVP(m_ev2);
    }
#endif
};

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */



namespace hayaku {

class HAYAKU_API SubEnvironment : public EnvironmentBase {
public:
    SubEnvironment();
    SubEnvironment(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
    virtual ~SubEnvironment();

    virtual void _calculate() override;
    virtual void _reset() override;
    virtual EnvironmentPtr _clone() override;

private:
    EnvironmentPtr m_ev1;
    EnvironmentPtr m_ev2;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase);
        ar& BOOST_SERIALIZATION_NVP(m_ev1);
        ar& BOOST_SERIALIZATION_NVP(m_ev2);
    }
#endif
};

}  // namespace hayaku
