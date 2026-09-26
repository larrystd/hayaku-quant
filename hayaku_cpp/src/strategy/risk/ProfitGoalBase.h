#pragma once

/*
 * ProfitGoal.h
 *
 *  Created on: 2013-3-7
 *      Author: fasiondog
 */


#include "data/KData.h"
#include "common/Parameter.h"
#include "execution/ExecutionAccountPort.h"

namespace hayaku {

/**
 * Base class of the profit goal strategy
 * @details The profit goal is determined before the trade, it is used by the system to execute a
 * sell when the price reaches the profit goal
 * @ingroup ProfitGoal
 */
class HAYAKU_API ProfitGoalBase : public enable_shared_from_this<ProfitGoalBase> {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    ProfitGoalBase();
    explicit ProfitGoalBase(const string& name);
    ProfitGoalBase(const ProfitGoalBase&) = default;
    virtual ~ProfitGoalBase();

    /** Set the account */
    void setAccount(const internal::ExecutionAccountPortPtr& account);

    /** Get the account */
    internal::ExecutionAccountPortPtr getAccount() const;

    /** Set the trading object */
    void setTO(const KData& kdata);

    /** Get the trading object */
    KData getTO() const;

    /** Get the name */
    const string& name() const;

    /** Set the name */
    void name(const string& name);

    /** Receive the actual trade change situation */
    virtual void buyNotify(const TradeRecord&) {}

    /** Receive the actual trade change situation */
    virtual void sellNotify(const TradeRecord&) {}

    /** Reset operation */
    void reset();

    typedef shared_ptr<ProfitGoalBase> ProfitGoalPtr;
    /** Clone interface */
    ProfitGoalPtr clone();

    /**
     * Calculate the target price when buying
     * @param datetime the current time
     * @param price the current price
     * @return Null<price_t> means no target is set; 0 means a sell is needed
     */
    virtual price_t getGoal(const Datetime& datetime, price_t price) = 0;

    /** 0 is returned, meaning no target is set */
    virtual price_t getShortGoal(const Datetime&, price_t) {
        return 0.0;
    }

    /** Subclass reset interface */
    virtual void _reset() {}

    /** Subclass clone interface */
    virtual ProfitGoalPtr _clone() = 0;

    /** Subclass calculation interface, it is called by setTO */
    virtual void _calculate() {}

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

    bool m_is_python_object{false};

protected:
    string m_name;
    KData m_kdata;
    internal::ExecutionAccountPortPtr m_account;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HAYAKU_SUPPORT_SERIALIZATION */
};

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(ProfitGoalBase)
#endif

#if HAYAKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used directly for the
 * serialization
 * @code
 * class Drived: public ProfitGoalBase {
 *     PROFIT_GOAL_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup ProfitGoal
 */
#define PROFIT_GOAL_NO_PRIVATE_MEMBER_SERIALIZATION              \
private:                                                         \
    friend class boost::serialization::access;                   \
    template <class Archive>                                     \
    void serialize(Archive& ar, const unsigned int version) {    \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(ProfitGoalBase); \
    }
#else
#define PROFIT_GOAL_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define PROFITGOAL_IMP(classname)             \
public:                                       \
    virtual ProfitGoalPtr _clone() override { \
        return std::make_shared<classname>(); \
    }                                         \
    virtual price_t getGoal(const Datetime&, price_t) override;

/**
 * Client programs should all use this pointer type
 * @ingroup ProfitGoal
 */
typedef shared_ptr<ProfitGoalBase> ProfitGoalPtr;
typedef shared_ptr<ProfitGoalBase> PGPtr;

HAYAKU_API std::ostream& operator<<(std::ostream& os, const ProfitGoalBase& pg);
HAYAKU_API std::ostream& operator<<(std::ostream& os, const ProfitGoalPtr& pg);

inline void ProfitGoalBase::setAccount(const internal::ExecutionAccountPortPtr& account) {
    m_account = account;
}

inline internal::ExecutionAccountPortPtr ProfitGoalBase::getAccount() const {
    return m_account;
}

inline KData ProfitGoalBase::getTO() const {
    return m_kdata;
}

inline const string& ProfitGoalBase::name() const {
    return m_name;
}

inline void ProfitGoalBase::name(const string& name) {
    m_name = name;
}

} /* namespace hayaku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hayaku::ProfitGoalBase> : ostream_formatter {};

template <>
struct fmt::formatter<hayaku::ProfitGoalPtr> : ostream_formatter {};
#endif
