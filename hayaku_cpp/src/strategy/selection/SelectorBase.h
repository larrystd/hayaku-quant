#pragma once

/*
 * SelectorBase.h
 *
 *  Created on: 2016-2-21
 *      Author: fasiondog
 */

#include "StrategyWeight.h"
#include "common/Parameter.h"
#include "data/KData.h"
#include "strategy/portfolio/AllocateFundsBase.h"
#include "strategy/selection/MultiFactorBase.h"

namespace hayaku {

class HAYAKU_API Portfolio;

/**
 * Trading object selection module
 * @ingroup Selector
 */
class HAYAKU_API SelectorBase : public enable_shared_from_this<SelectorBase> {
  PARAMETER_SUPPORT_WITH_CHECK

 public:
  /** Default constructor */
  SelectorBase();
  SelectorBase(const SelectorBase&) = default;

  /**
   * Constructor, it also gives the algorithm name
   * @param name the given name
   */
  explicit SelectorBase(const string& name);

  /** Destructor */
  virtual ~SelectorBase();

  /** Get the algorithm name */
  const string& name() const;

  /** Set the algorithm name */
  void name(const string& name);

  using PFPtr = shared_ptr<Portfolio>;
  PFPtr getPF() const { return pf_.lock(); }

  void setPF(const PFPtr& pf) { pf_ = pf; }

  /**
   * Add a candidate stock and its trading strategy prototype
   * @param stock candidate stock
   * @param protoSys trading system strategy prototype
   */
  void addStock(const Stock& stock,
                const internal::StrategyRuntimePtr& protoSys);

  /**
   * Add a group of stocks with the same trading strategy
   * @note An invalid stock is ignored automatically, false is not returned
   * @param stkList candidate stock list
   * @param protoSys trading system strategy prototype
   */
  void addStockList(const StockList& stkList,
                    const internal::StrategyRuntimePtr& protoSys);

  /**
   * Add an existing system strategy instance directly
   * @note The stock should already be bound
   * @param sys
   */
  void addSystem(const internal::StrategyRuntimePtr& sys);

  /**
   * Add an existing system strategy instance directly
   * @note The stock should already be bound
   * @param sys
   */
  void addSystemList(const internal::StrategyRuntimeList& sys);

  /**
   * @brief Get the prototype system list
   * @return const internal::StrategyRuntimeList&
   */
  const internal::StrategyRuntimeList& getProtoSystemList() const;

  /**
   * @brief Get the system list actually run by PF
   * @return const internal::StrategyRuntimeList&
   */
  const internal::StrategyRuntimeList& getRealSystemList() const;

  /** Get the targets selected at the close of the given moment */
  StrategyWeightList getSelected(Datetime date);

  /**
   * @brief Reset
   * @note The reset does not clear the existing prototype systems
   */
  void reset();

  /**
   * Clear the existing system prototypes
   */
  void removeAll();

  typedef shared_ptr<SelectorBase> SelectorPtr;
  SelectorPtr clone();

  /** Subclass reset interface */
  virtual void _reset() {}

  /** Subclass clone interface */
  virtual SelectorPtr _clone() = 0;

  /** Subclass calculation interface */
  virtual void _calculate() = 0;

  /** Subclass interface to get the targets selected at the close of the given
   * moment */
  virtual StrategyWeightList _getSelected(Datetime date) = 0;

  virtual bool isMatchAF(const AFPtr& af) = 0;

  /** Add a prototype system in the subclass used for the logical operations; it
   * generally does not need to be implemented by the subclass */
  virtual void _addSystem(const internal::StrategyRuntimePtr& sys) {}

  /** Add a prototype system in the subclass used for the logical operations; it
   * generally does not need to be implemented by the subclass */
  virtual void _removeAll() {}

  /* Called by PF only; PF notifies it of the system list it actually runs and
   * starts the calculation */
  virtual void calculate(const internal::StrategyRuntimeList& pf_realSysList,
                         const KQuery& query);

  /* Called by PF only, it builds the mapping from the actual systems to the
   * prototype systems */
  virtual void bindRealToProto(const internal::StrategyRuntimePtr& real,
                               const internal::StrategyRuntimePtr& proto) {}

  void calculate_proto(const KQuery& query);

  virtual string str() const;

 public:
  //------------------------------------------------------------------------
  // It is useful for the Selector related to MF only; it is placed here mainly
  // so that SEPtr can get the MF related information directly It is useless for
  // the Selector not related to MF
  //------------------------------------------------------------------------
  MFPtr getMF() const { return mf_; }

  void setMF(const MFPtr& mf) {
    mf_ = mf;
    calculated_ = false;
  }

  ScoresFilterPtr getScoresFilter() const { return sc_filter_; }

  /** Set the cross-section score record filter, it is used for the MF related
   * Selector only, to filter when the Score list is got from MF */
  void setScoresFilter(const ScoresFilterPtr& filter);

  /** Append a filter on the basis of the existing filters, it is used for the
   * MF related Selector only, to filter when the Score list is got from MF */
  void addScoresFilter(const ScoresFilterPtr& filter);

  bool isPythonObject() const noexcept { return is_python_object_; }

 private:
  void initParam();

 protected:
  ScoresFilterPtr sc_filter_;
  MFPtr mf_;

 protected:
  string name_;
  bool is_python_object_{false};
  bool calculated_{false};  // Whether it has been calculated
  bool proto_calculated_{false};
  KQuery query_;
  KQuery proto_query_;

  internal::StrategyRuntimeList pro_sys_list_;  // Prototype system list
  internal::StrategyRuntimeList
      real_sys_list_;  // The systems actually run in the PF portfolio, set
                        // when PF executes, in the same order as the prototype
                        // list

  std::weak_ptr<Portfolio>
      pf_;  // Stored but not serialized, the reference to PF

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
 private:
  friend class boost::serialization::access;
  template <class Archive>
  void save(Archive& ar, const unsigned int version) const {
    ar& boost::serialization::make_nvp("m_name", name_);
    ar& boost::serialization::make_nvp("m_is_python_object", is_python_object_);
    ar& boost::serialization::make_nvp("m_params", params_);
    ar& boost::serialization::make_nvp("m_sc_filter", sc_filter_);
    ar& boost::serialization::make_nvp("m_mf", mf_);
  }

  template <class Archive>
  void load(Archive& ar, const unsigned int version) {
    ar& boost::serialization::make_nvp("m_name", name_);
    ar& boost::serialization::make_nvp("m_is_python_object", is_python_object_);
    ar& boost::serialization::make_nvp("m_params", params_);
    pro_sys_list_.clear();
    ar& boost::serialization::make_nvp("m_sc_filter", sc_filter_);
    ar& boost::serialization::make_nvp("m_mf", mf_);
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HAYAKU_SUPPORT_SERIALIZATION */
};

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(SelectorBase)
#endif

#if HAYAKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used
 * directly for the serialization
 * @code
 * class Drived: public SelectorBase {
 *     SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Selector
 */
#define SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION            \
 private:                                                   \
  friend class boost::serialization::access;                \
  template <class Archive>                                  \
  void serialize(Archive& ar, const unsigned int version) { \
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase);  \
  }
#else
#define SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define SELECTOR_IMP(classname)                                    \
 public:                                                           \
  virtual SelectorPtr _clone() override {                          \
    return std::make_shared<classname>();                          \
  }                                                                \
  virtual StrategyWeightList _getSelected(Datetime date) override; \
  virtual bool isMatchAF(const AFPtr& af) override;                \
  virtual void _calculate() override;

/**
 * Client programs should all use this pointer type
 * @ingroup Selector
 */
typedef shared_ptr<SelectorBase> SelectorPtr;
typedef shared_ptr<SelectorBase> SEPtr;

HAYAKU_API std::ostream& operator<<(std::ostream&, const SelectorBase&);
HAYAKU_API std::ostream& operator<<(std::ostream&, const SelectorPtr&);

inline const string& SelectorBase::name() const { return name_; }

inline void SelectorBase::name(const string& name) { name_ = name; }

inline const internal::StrategyRuntimeList& SelectorBase::getRealSystemList()
    const {
  return real_sys_list_;
}

inline const internal::StrategyRuntimeList& SelectorBase::getProtoSystemList()
    const {
  return pro_sys_list_;
}

inline void SelectorBase::setScoresFilter(const ScoresFilterPtr& filter) {
  sc_filter_ = filter;
  calculated_ = false;
}

inline void SelectorBase::addScoresFilter(const ScoresFilterPtr& filter) {
  sc_filter_ = sc_filter_ | filter;
  calculated_ = false;
}

} /* namespace hayaku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hayaku::SelectorBase> : ostream_formatter {};

template <>
struct fmt::formatter<hayaku::SelectorPtr> : ostream_formatter {};
#endif
