#pragma once

/*
 * Environment.h
 *
 *  Created on: 2013-2-28
 *      Author: fasiondog
 */

#include <set>
#include <shared_mutex>

#include "common/Parameter.h"
#include "data/KQuery.h"
#include "operators/Indicator.h"

namespace hayaku {

/**
 * Base class of the market environment strategy
 * @note The external environment should have nothing to do with the concrete
 * trading object
 * @ingroup Environment
 */
class HAYAKU_API EnvironmentBase
    : public enable_shared_from_this<EnvironmentBase> {
  PARAMETER_SUPPORT_WITH_CHECK

 public:
  EnvironmentBase();
  explicit EnvironmentBase(const string& name);
  virtual ~EnvironmentBase();

  // Used for the python clone, but it is not thread safe because of the mutex
  EnvironmentBase(const EnvironmentBase&);

  /** Get the name */
  const string& name() const { return name_; }

  /** Set the name */
  void name(const string& name) { name_ = name; }

  /** Reset */
  void reset();

  /** Set the query condition */
  void setQuery(const KQuery& query);

  /** Get the query condition */
  const KQuery& getQuery() const { return query_; }

  typedef shared_ptr<EnvironmentBase> EnvironmentPtr;
  /**
   * Clone operation
   * @note Unlike the other trading-system parts, the Environment is not bound
   * to a specific instrument and can be shared, so essentially the clone
   * operation is not needed here; it exists only for the consistency and the
   * possibly existing special scenarios.
   */
  EnvironmentPtr clone();

  /**
   * Add a valid time, it is called in _calculate
   * @param datetime the valid date of the system
   * @param value 1.0 by default; greater than 0 means valid and less than or
   * equal to 0 means invalid
   */
  void _addValid(const Datetime& datetime, price_t value = 1.0);

  /**
   * Judge whether the external environment of the given date is valid
   * @param datetime the given date
   * @return true valid | false invalid
   */
  bool isValid(const Datetime& datetime) const;

  price_t getValue(const Datetime& datetime) const;

  /**
   * Get the actual value in the form of an indicator, it is as long as the
   * trading object; <=0 means invalid and >0 means the system is valid
   * @note A time series indicator with the dates
   */
  Indicator getValues() const;

  /** Subclass calculation interface */
  virtual void _calculate() = 0;

  /** Subclass reset interface */
  virtual void _reset() {}

  /** Subclass clone interface */
  virtual EnvironmentPtr _clone() = 0;

  bool isPythonObject() const noexcept { return is_python_object_; }

 protected:
  string name_;
  KQuery query_;
  map<Datetime, size_t> date_index_;
  vector<price_t> values_;
  mutable std::shared_mutex mutex_;

  bool is_python_object_{false};

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
    // ev may be shared by multiple systems; m_query is kept and may be used for
    // the troubleshooting
    ar& boost::serialization::make_nvp("m_query", query_);
    ar& boost::serialization::make_nvp("m_date_index", date_index_);
    ar& boost::serialization::make_nvp("m_values", values_);
    ar& boost::serialization::make_nvp("m_is_python_object", is_python_object_);
  }

  template <class Archive>
  void load(Archive& ar, const unsigned int version) {
    ar& boost::serialization::make_nvp("m_name", name_);
    ar& boost::serialization::make_nvp("m_params", params_);
    ar& boost::serialization::make_nvp("m_query", query_);
    ar& boost::serialization::make_nvp("m_date_index", date_index_);
    ar& boost::serialization::make_nvp("m_values", values_);
    ar& boost::serialization::make_nvp("m_is_python_object", is_python_object_);
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HAYAKU_SUPPORT_SERIALIZATION */
};

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(EnvironmentBase)
#endif

#if HAYAKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used
 * directly for the serialization
 * @code
 * class Drived: public EnvironmentBase {
 *     ENVIRONMENT_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Environment
 */
#define ENVIRONMENT_NO_PRIVATE_MEMBER_SERIALIZATION           \
 private:                                                     \
  friend class boost::serialization::access;                  \
  template <class Archive>                                    \
  void serialize(Archive& ar, const unsigned int version) {   \
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase); \
  }
#else
#define ENVIRONMENT_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

/**
 * Client programs should all use this pointer type
 * @ingroup Environment
 */
typedef shared_ptr<EnvironmentBase> EnvironmentPtr;
typedef shared_ptr<EnvironmentBase> EVPtr;

#define ENVIRONMENT_IMP(classname)           \
 public:                                     \
  virtual EnvironmentPtr _clone() override { \
    return std::make_shared<classname>();    \
  }                                          \
  virtual void _calculate() override;

/**
 * Output the Environment information, e.g. Environment(name, params[...])
 * @ingroup Environment
 */
HAYAKU_API std::ostream& operator<<(std::ostream& os, const EnvironmentPtr&);
HAYAKU_API std::ostream& operator<<(std::ostream& os, const EnvironmentBase&);

} /* namespace hayaku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hayaku::EnvironmentBase> : ostream_formatter {};

template <>
struct fmt::formatter<hayaku::EnvironmentPtr> : ostream_formatter {};
#endif
