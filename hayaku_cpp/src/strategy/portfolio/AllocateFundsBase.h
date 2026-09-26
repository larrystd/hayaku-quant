#pragma once

/*
 * AllocateMoney.h
 *
 *  Created on: 2018-1-30
 *      Author: fasiondog
 */

#include "common/Parameter.h"
#include "execution/PortfolioAccountPort.h"
#include "strategy/selection/StrategyWeight.h"

namespace hayaku {

/**
 * Asset allocation adjustment algorithm
 * @details It allocates and adjusts the asset proportions according to the
 * asset market value. For a pure fund adjustment, please use the money
 * management algorithm.
 * @ingroup AllocateFunds
 */
class HAYAKU_API AllocateFundsBase
    : public enable_shared_from_this<AllocateFundsBase> {
  PARAMETER_SUPPORT_WITH_CHECK

 public:
  /** Default constructor */
  AllocateFundsBase();
  AllocateFundsBase(const AllocateFundsBase&) = default;

  /**
   * Constructor
   * @param name algorithm name
   */
  explicit AllocateFundsBase(const string& name);

  /** Destructor */
  virtual ~AllocateFundsBase();

  /** Get the algorithm name */
  const string& name() const;

  /** Modify the algorithm name */
  void name(const string& name);

  /**
   * Execute the asset allocation adjustment, it is called by PF only
   * @param date the given date
   * @param se_list the system instances selected by the system instance
   * selector
   * @param running_list the currently running system instances
   * @return the system list that needs a delayed sell operation, where the
   * weight is the corresponding quantity to be sold
   */
  StrategyWeightList adjustFunds(
      const Datetime& date, const StrategyWeightList& se_list,
      const std::unordered_set<internal::StrategyRuntimePtr>& running_list);

  /** Get the trade account */
  const internal::PortfolioAccountPortPtr& getAccount() const;

  /** Set the trade account, it is set by PF */
  void setAccount(const internal::PortfolioAccountPortPtr& account);

  /** Set the shadow account of Portfolio, it is called by Portfolio only */
  void setCashAccount(const internal::PortfolioAccountPortPtr& account);

  const internal::PortfolioAccountPortPtr& getCashAccount() const;

  /** Get the associated query condition */
  const KQuery& getQuery() const;

  /** Set the query condition, it is set by PF */
  void setQuery(const KQuery& query);

  /** Reset */
  void reset();

  typedef shared_ptr<AllocateFundsBase> AFPtr;

  /** Clone operation */
  AFPtr clone();

  /** Subclass reset interface */
  virtual void _reset() {}

  /** Interface for the subclass to clone its private variables */
  virtual AFPtr _clone() = 0;

  /**
   * Subclass weight allocation interface, it gets the system instances actually
   * allocated the assets and their weights
   * @details It actually calls the subclass interface _allocateWeight
   * @param date the given date
   * @param se_list the system instances selected by the system instance
   * selector
   * @return the subclass only needs to return the relative proportion of every
   * system
   */
  virtual StrategyWeightList _allocateWeight(
      const Datetime& date, const StrategyWeightList& se_list) = 0;

 public:
  /*
   * An internal function, it is set to public for the testing only.
   * It adjusts the planned weights allocated by the subclass according to the
   * internal parameter settings
   */
  static void adjustWeight(StrategyWeightList& sw_list,
                           double can_allocate_weight, bool auto_adjust,
                           bool ignore_zero);

  bool isPythonObject() const noexcept { return m_is_python_object; }

 private:
  void initParam();

  /* It also adjusts the sub-systems already running (the ones already allocated
   * funds or holding positions) */
  StrategyWeightList _adjust_with_running(
      const Datetime& date, const StrategyWeightList& se_list,
      const std::unordered_set<internal::StrategyRuntimePtr>& running_list);

  /* It does not adjust the sub-systems already running */
  void _adjust_without_running(
      const Datetime& date, const StrategyWeightList& se_list,
      const std::unordered_set<internal::StrategyRuntimePtr>& running_list);

 protected:
  bool m_is_python_object{false};

 private:
  string m_name;   // Component name
  KQuery m_query;  // Query condition
  internal::PortfolioAccountPortPtr
      m_account;  // Set by PF at runtime, the actual account of PF
  internal::PortfolioAccountPortPtr
      m_cashAccount;  // Set by PF at runtime, the shadow account of tm, used to
                      // coordinate the fund allocation

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
 private:
  friend class boost::serialization::access;
  template <class Archive>
  void save(Archive& ar, const unsigned int version) const {
    ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
    ar& BOOST_SERIALIZATION_NVP(m_name);
    ar& BOOST_SERIALIZATION_NVP(m_params);
    ar& BOOST_SERIALIZATION_NVP(m_query);
  }

  template <class Archive>
  void load(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
    ar& BOOST_SERIALIZATION_NVP(m_name);
    ar& BOOST_SERIALIZATION_NVP(m_params);
    ar& BOOST_SERIALIZATION_NVP(m_query);
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HAYAKU_SUPPORT_SERIALIZATION */
};

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(AllocateFundsBase)
#endif

#if HAYAKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used
 * directly for the serialization
 * @code
 * class Drived: public AllocateFundsBase {
 *     ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Selector
 */
#define ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION           \
 private:                                                       \
  friend class boost::serialization::access;                    \
  template <class Archive>                                      \
  void serialize(Archive& ar, const unsigned int version) {     \
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(AllocateFundsBase); \
  }
#else
#define ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define ALLOCATEFUNDS_IMP(classname)                                        \
 public:                                                                    \
  virtual AFPtr _clone() override { return std::make_shared<classname>(); } \
  virtual StrategyWeightList _allocateWeight(                               \
      const Datetime&, const StrategyWeightList&) override;

typedef shared_ptr<AllocateFundsBase> AllocateFundsPtr;
typedef shared_ptr<AllocateFundsBase> AFPtr;

HAYAKU_API std::ostream& operator<<(std::ostream&, const AllocateFundsBase&);
HAYAKU_API std::ostream& operator<<(std::ostream&, const AFPtr&);

inline const string& AllocateFundsBase::name() const { return m_name; }

inline void AllocateFundsBase::name(const string& name) { m_name = name; }

inline const internal::PortfolioAccountPortPtr& AllocateFundsBase::getAccount()
    const {
  return m_account;
}

inline void AllocateFundsBase::setAccount(
    const internal::PortfolioAccountPortPtr& account) {
  m_account = account;
}

inline void AllocateFundsBase::setCashAccount(
    const internal::PortfolioAccountPortPtr& account) {
  m_cashAccount = account;
}

inline const internal::PortfolioAccountPortPtr&
AllocateFundsBase::getCashAccount() const {
  return m_cashAccount;
}

inline const KQuery& AllocateFundsBase::getQuery() const { return m_query; }

inline void AllocateFundsBase::setQuery(const KQuery& query) {
  m_query = query;
}

} /* namespace hayaku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hayaku::AllocateFundsBase> : ostream_formatter {};

template <>
struct fmt::formatter<hayaku::AFPtr> : ostream_formatter {};
#endif
