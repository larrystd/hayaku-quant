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
  Portfolio(const string& name, const internal::PortfolioAccountPortPtr& tm,
            const SelectorPtr& se, const AFPtr& af);

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
  const std::vector<std::pair<Datetime, double>>& getAdjustTurnover()
      const noexcept;

  /** Used for the printing output */
  virtual string str() const;

  virtual void _reset() {}
  virtual PortfolioPtr _clone() { return std::make_shared<Portfolio>(); }

  virtual void _readyForRun() {}
  virtual void _runMomentOnOpen(const Datetime& date, const Datetime& nextCycle,
                                bool adjust) {}
  virtual void _runMomentOnClose(const Datetime& date,
                                 const Datetime& nextCycle, bool adjust) {}

  /**
   * After the backtest is finished, return the trade record of the last day
   * together with the delayed buy and sell requests
   */
  virtual json lastSuggestion() const;

  bool isPythonObject() const noexcept { return is_python_object_; }

 private:
  void initParam();

  // Calculate the rebalancing dates
  void _calculateAdjustDate();
  void _calculateAdjustDateOnMode(int adjust_cycle, const string& mode);
  void _calculateAdjustDateOnModeDelayToTradingDay(int adjust_cycle,
                                                   const string& mode);

 protected:
  // Track and print the current TM positions
  void traceMomentTMAfterRunAtOpen(const Datetime& date);
  void traceMomentTMAfterRunAtClose(const Datetime& date);

 protected:
  string name_;
  internal::PortfolioAccountPortPtr account_;
  internal::PortfolioAccountPortPtr
      cash_account_;  // It is responsible for the internal fund management only
                      // (i.e. it only needs to checkout to the sub-accounts and
                      // check in cash from the accounts)
  SEPtr se_;
  AFPtr af_;

  KQuery query_;         // The associated query condition
  bool need_calculate_;  // Flag of whether the calculation is needed
  bool is_python_object_{false};

  internal::StrategyRuntimeList
      real_sys_list_;  // List of all the actually running sub-systems

  // Temporary data used for the intermediate calculation
  std::unordered_set<internal::StrategyRuntimePtr> running_sys_set_;
  DatetimeList dates_;            // Running date list
  vector<uint8_t> adjust_flags_;  // Rebalancing day flags
  DatetimeList cycle_end_dates_;  // Rebalancing cycle end dates

  std::vector<std::pair<Datetime, double>>
      adjust_turnover_;  // Rebalancing cycle turnover rate (the subclass needs
                          // to implement it itself, it is absent if not
                          // implemented)

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
 private:
  friend class boost::serialization::access;
  template <class Archive>
  void save(Archive& ar, const unsigned int version) const {
    ar& boost::serialization::make_nvp("m_name", name_);
    ar& boost::serialization::make_nvp("m_params", params_);
    ar& boost::serialization::make_nvp("m_se", se_);
    ar& boost::serialization::make_nvp("m_af", af_);
    ar& boost::serialization::make_nvp("m_query", query_);
    ar& boost::serialization::make_nvp("m_is_python_object", is_python_object_);
  }

  template <class Archive>
  void load(Archive& ar, const unsigned int version) {
    ar& boost::serialization::make_nvp("m_name", name_);
    ar& boost::serialization::make_nvp("m_params", params_);
    ar& boost::serialization::make_nvp("m_se", se_);
    ar& boost::serialization::make_nvp("m_af", af_);
    ar& boost::serialization::make_nvp("m_query", query_);
    ar& boost::serialization::make_nvp("m_is_python_object", is_python_object_);
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HAYAKU_SUPPORT_SERIALIZATION */
};

#define PORTFOLIO_IMP(classname)                                              \
 public:                                                                      \
  virtual PortfolioPtr _clone() override {                                    \
    return std::make_shared<classname>();                                     \
  }                                                                           \
  virtual void _reset() override;                                             \
  virtual void _readyForRun() override;                                       \
  virtual void _runMomentOnOpen(                                              \
      const Datetime& date, const Datetime& nextCycle, bool adjust) override; \
  virtual void _runMomentOnClose(                                             \
      const Datetime& date, const Datetime& nextCycle, bool adjust) override;

/**
 * Client programs should all use this pointer type
 * @ingroup Selector
 */
typedef shared_ptr<Portfolio> PortfolioPtr;
typedef shared_ptr<Portfolio> PFPtr;

HAYAKU_API std::ostream& operator<<(std::ostream&, const Portfolio&);
HAYAKU_API std::ostream& operator<<(std::ostream&, const PortfolioPtr&);

inline const string& Portfolio::name() const { return name_; }

inline void Portfolio::name(const string& name) { name_ = name; }

inline void Portfolio::setQuery(const KQuery& query) {
  if (query_ != query) {
    query_ = query;
    need_calculate_ = true;
  }
}

inline const KQuery& Portfolio::getQuery() const { return query_; }

inline internal::PortfolioAccountPortPtr Portfolio::getAccount() const {
  return account_;
}

inline void Portfolio::setAccount(const internal::PortfolioAccountPortPtr& tm) {
  if (account_ != tm) {
    account_ = tm;
    need_calculate_ = true;
  }
}

inline SEPtr Portfolio::getSE() const { return se_; }

inline void Portfolio::setSE(const SEPtr& se) {
  if (se_ != se) {
    se_ = se;
    need_calculate_ = true;
  }
}

inline AFPtr Portfolio::getAF() const { return af_; }

inline void Portfolio::setAF(const AFPtr& af) {
  if (af_ != af) {
    af_ = af;
    need_calculate_ = true;
  }
}

inline const internal::StrategyRuntimeList& Portfolio::getRealSystemList()
    const {
  return real_sys_list_;
}

inline const DatetimeList& Portfolio::getRunningDates() const noexcept {
  return dates_;
}

inline const std::vector<std::pair<Datetime, double>>&
Portfolio::getAdjustTurnover() const noexcept {
  return adjust_turnover_;
}

} /* namespace hayaku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hayaku::Portfolio> : ostream_formatter {};

template <>
struct fmt::formatter<hayaku::PortfolioPtr> : ostream_formatter {};
#endif
