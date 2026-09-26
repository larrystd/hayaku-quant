#pragma once

/*
 * Portfolio.h
 *
 *  Created on: 2016-2-21
 *      Author: fasiondog
 */


#include <nlohmann/json.hpp>

#include "execution/PortfolioAccountPort.h"
#include "strategy/portfolio/AllocateFundsBase.h"
#include "strategy/selection/SelectorBase.h"

namespace hayaku {

using json = nlohmann::json;

/*
 * Portfolio
 * @ingroup Portfolio
 */
class HAYAKU_API Portfolio : public enable_shared_from_this<Portfolio> {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    /** Default constructor */
    Portfolio();

    /**
     * @brief Constructor with the given name
     * @param name name
     */
    explicit Portfolio(const string& name);

    /**
     * @brief Constructor
     * @param name portfolio name
     * @param tm account
     * @param se selector
     * @param af asset allocation algorithm
     */
    Portfolio(const string& name, const internal::PortfolioAccountPortPtr& tm, const SelectorPtr& se,
              const AFPtr& af);

    /** Destructor */
    virtual ~Portfolio();

    /** Portfolio name */
    const string& name() const;

    /** Set the portfolio name */
    void name(const string& name);

    /**
     * @brief Run the portfolio
     * @param query query condition, its KType must be KQuery::DAY
     * @param force whether to force the recalculation
     */
    void run(const KQuery& query, bool force = false);

    /** Modify the query condition */
    void setQuery(const KQuery& query);

    /** Get the query condition */
    const KQuery& getQuery() const;

    /** Get the account */
    internal::PortfolioAccountPortPtr getAccount() const;

    /** Set the account */
    void setAccount(const internal::PortfolioAccountPortPtr& tm);

    /** Get the selector */
    SEPtr getSE() const;

    /** Set the selector */
    void setSE(const SEPtr& se);

    /** Get the asset allocation algorithm */
    AFPtr getAF() const;

    /** Set the asset allocation algorithm */
    void setAF(const AFPtr& af);

    const internal::StrategyRuntimeList& getRealSystemList() const;

    /** Reset operation */
    void reset();

    /** Clone operation */
    typedef shared_ptr<Portfolio> PortfolioPtr;
    PortfolioPtr clone();

    /** Preparation before running */
    void readyForRun();

    void runMoment(const Datetime& date, const Datetime& nextCycle, bool adjust);

    /** Get the running date list */
    const DatetimeList& getRunningDates() const noexcept;

    /** Get the rebalancing date list */
    DatetimeList getAdjustDates() const;

    /** Get the rebalancing cycle end date list */
    DatetimeList getCycleEndDates() const;

    /** Get the rebalancing turnover rate list */
    const std::vector<std::pair<Datetime, double>>& getAdjustTurnover() const noexcept;

    /** Used for the printing output */
    virtual string str() const;

    virtual void _reset() {}
    virtual PortfolioPtr _clone() {
        return std::make_shared<Portfolio>();
    }

    virtual void _readyForRun() {}
    virtual void _runMomentOnOpen(const Datetime& date, const Datetime& nextCycle, bool adjust) {}
    virtual void _runMomentOnClose(const Datetime& date, const Datetime& nextCycle, bool adjust) {}

    /**
     * After the backtest is finished, return the trade record of the last day together with the
     * delayed buy and sell requests
     */
    virtual json lastSuggestion() const;

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

private:
    void initParam();

    // Calculate the rebalancing dates
    void _calculateAdjustDate();
    void _calculateAdjustDateOnMode(int adjust_cycle, const string& mode);
    void _calculateAdjustDateOnModeDelayToTradingDay(int adjust_cycle, const string& mode);

protected:
    // Track and print the current TM positions
    void traceMomentTMAfterRunAtOpen(const Datetime& date);
    void traceMomentTMAfterRunAtClose(const Datetime& date);

protected:
    string m_name;
    internal::PortfolioAccountPortPtr m_account;
    internal::PortfolioAccountPortPtr m_cashAccount;  // It is responsible for the internal fund management only (i.e. it only needs
                      // to checkout to the sub-accounts and check in cash from the accounts)
    SEPtr m_se;
    AFPtr m_af;

    KQuery m_query;         // The associated query condition
    bool m_need_calculate;  // Flag of whether the calculation is needed
    bool m_is_python_object{false};

    internal::StrategyRuntimeList m_real_sys_list;  // List of all the actually running sub-systems

    // Temporary data used for the intermediate calculation
    std::unordered_set<internal::StrategyRuntimePtr> m_running_sys_set;
    DatetimeList m_dates;            // Running date list
    vector<uint8_t> m_adjust_flags;  // Rebalancing day flags
    DatetimeList m_cycle_end_dates;  // Rebalancing cycle end dates

    std::vector<std::pair<Datetime, double>>
      m_adjust_turnover;  // Rebalancing cycle turnover rate (the subclass needs to implement it
                          // itself, it is absent if not implemented)

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
        ar& BOOST_SERIALIZATION_NVP(m_se);
        ar& BOOST_SERIALIZATION_NVP(m_af);
        ar& BOOST_SERIALIZATION_NVP(m_query);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_se);
        ar& BOOST_SERIALIZATION_NVP(m_af);
        ar& BOOST_SERIALIZATION_NVP(m_query);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HAYAKU_SUPPORT_SERIALIZATION */
};

#define PORTFOLIO_IMP(classname)                                                                 \
public:                                                                                          \
    virtual PortfolioPtr _clone() override {                                                     \
        return std::make_shared<classname>();                                                    \
    }                                                                                            \
    virtual void _reset() override;                                                              \
    virtual void _readyForRun() override;                                                        \
    virtual void _runMomentOnOpen(const Datetime& date, const Datetime& nextCycle, bool adjust)  \
      override;                                                                                  \
    virtual void _runMomentOnClose(const Datetime& date, const Datetime& nextCycle, bool adjust) \
      override;

/**
 * Client programs should all use this pointer type
 * @ingroup Selector
 */
typedef shared_ptr<Portfolio> PortfolioPtr;
typedef shared_ptr<Portfolio> PFPtr;

HAYAKU_API std::ostream& operator<<(std::ostream&, const Portfolio&);
HAYAKU_API std::ostream& operator<<(std::ostream&, const PortfolioPtr&);

inline const string& Portfolio::name() const {
    return m_name;
}

inline void Portfolio::name(const string& name) {
    m_name = name;
}

inline void Portfolio::setQuery(const KQuery& query) {
    if (m_query != query) {
        m_query = query;
        m_need_calculate = true;
    }
}

inline const KQuery& Portfolio::getQuery() const {
    return m_query;
}

inline internal::PortfolioAccountPortPtr Portfolio::getAccount() const {
    return m_account;
}

inline void Portfolio::setAccount(const internal::PortfolioAccountPortPtr& tm) {
    if (m_account != tm) {
        m_account = tm;
        m_need_calculate = true;
    }
}

inline SEPtr Portfolio::getSE() const {
    return m_se;
}

inline void Portfolio::setSE(const SEPtr& se) {
    if (m_se != se) {
        m_se = se;
        m_need_calculate = true;
    }
}

inline AFPtr Portfolio::getAF() const {
    return m_af;
}

inline void Portfolio::setAF(const AFPtr& af) {
    if (m_af != af) {
        m_af = af;
        m_need_calculate = true;
    }
}

inline const internal::StrategyRuntimeList& Portfolio::getRealSystemList() const {
    return m_real_sys_list;
}

inline const DatetimeList& Portfolio::getRunningDates() const noexcept {
    return m_dates;
}

inline const std::vector<std::pair<Datetime, double>>& Portfolio::getAdjustTurnover()
  const noexcept {
    return m_adjust_turnover;
}

} /* namespace hayaku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hayaku::Portfolio> : ostream_formatter {};

template <>
struct fmt::formatter<hayaku::PortfolioPtr> : ostream_formatter {};
#endif
