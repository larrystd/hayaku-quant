#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#include "common/Parameter.h"

namespace hayaku {

/**
 * Base class of the time cross-section data normalization
 */
class NormalizeBase {
  PARAMETER_SUPPORT_WITH_CHECK

 public:
  NormalizeBase() = default;
  NormalizeBase(const string& name) : name_(name) {}

  NormalizeBase(const NormalizeBase& other)
      : params_(other.params_), name_(other.name_) {}

  virtual ~NormalizeBase() = default;

  /** Get the name */
  const string& name() const { return name_; }

  /** Set the name */
  void name(const string& name) { name_ = name; }

  typedef std::shared_ptr<NormalizeBase> NormPtr;
  NormPtr clone();

  virtual NormPtr _clone() = 0;

  virtual PriceList normalize(const PriceList& data) = 0;

  bool isPythonObject() const noexcept { return is_python_object_; }

 protected:
  string name_;
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
    ar& boost::serialization::make_nvp("m_is_python_object", is_python_object_);
    ar& boost::serialization::make_nvp("m_params", params_);
  }

  template <class Archive>
  void load(Archive& ar, const unsigned int version) {
    ar& boost::serialization::make_nvp("m_name", name_);
    ar& boost::serialization::make_nvp("m_is_python_object", is_python_object_);
    ar& boost::serialization::make_nvp("m_params", params_);
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HAYAKU_SUPPORT_SERIALIZATION */
};

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(NormalizeBase)
#endif

#if HAYAKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used
 * directly for the serialization
 * @code
 * class Drived: public NormalizeBase {
 *     NORMALIZE_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup MultiFactor
 */
#define NORMALIZE_NO_PRIVATE_MEMBER_SERIALIZATION           \
 private:                                                   \
  friend class boost::serialization::access;                \
  template <class Archive>                                  \
  void serialize(Archive& ar, const unsigned int version) { \
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(NormalizeBase); \
  }
#else
#define NORMALIZE_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

typedef std::shared_ptr<NormalizeBase> NormPtr;
typedef std::shared_ptr<NormalizeBase> NormalizePtr;

#define NORMALIZE_IMP(classname)           \
 public:                                   \
  virtual NormalizePtr _clone() override { \
    return std::make_shared<classname>();  \
  }                                        \
  PriceList normalize(const PriceList& data) override;

std::ostream& operator<<(std::ostream&, const NormalizeBase&);
std::ostream& operator<<(std::ostream&, const NormalizePtr&);

}  // namespace hayaku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hayaku::NormalizeBase> : ostream_formatter {};

template <>
struct fmt::formatter<hayaku::NormalizePtr> : ostream_formatter {};
#endif
