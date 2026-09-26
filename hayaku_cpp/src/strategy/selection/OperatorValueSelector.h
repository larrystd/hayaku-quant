#pragma once

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#include "strategy/selection/SelectorBase.h"

namespace hayaku {

class OperatorValueSelector : public SelectorBase {
 public:
  OperatorValueSelector();
  explicit OperatorValueSelector(const string& name);
  OperatorValueSelector(const string& name, const SelectorPtr& se,
                        double value);
  virtual ~OperatorValueSelector();

  virtual void _reset() override;
  virtual SelectorPtr _clone() override;
  virtual bool isMatchAF(const AFPtr& af) override;
  virtual void _calculate() override;
  virtual StrategyWeightList _getSelected(Datetime date) override {
    return StrategyWeightList();
  }

 protected:
  SelectorPtr se_;
  double value_{0.0};

 private:
  //============================================
  // Serialization support
  //============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  // template <class Archive>
  // void serialize(Archive& ar, const unsigned int version) {
  //     ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase);
  //     ar& boost::serialization::make_nvp("m_se", se_);
  //     ar& boost::serialization::make_nvp("m_value", value_);
  // }
  template <class Archive>
  void save(Archive& ar, const unsigned int version) const {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase);
    ar& boost::serialization::make_nvp("m_se", se_);
    ar& boost::serialization::make_nvp("m_value", value_);
  }

  template <class Archive>
  void load(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase);
    ar& boost::serialization::make_nvp("m_se", se_);
    ar& boost::serialization::make_nvp("m_value", value_);
    if (se_) {
      pro_sys_list_ = se_->getProtoSystemList();
    }
  }

#endif
};

#define OPERATOR_VALUE_SELECTOR_IMP(classname, name)               \
 public:                                                           \
  classname() : OperatorValueSelector(name) {}                     \
  classname(const SelectorPtr& se, double value)                   \
      : OperatorValueSelector(name, se, value) {}                  \
  virtual ~classname() override {}                                 \
                                                                   \
  virtual StrategyWeightList _getSelected(Datetime date) override; \
                                                                   \
  virtual SelectorPtr _clone() override {                          \
    auto p = std::make_shared<classname>();                        \
    if (se_) {                                                     \
      p->se_ = se_->clone();                                       \
    }                                                              \
    p->value_ = value_;                                            \
    return p;                                                      \
  }

#if HAYAKU_SUPPORT_SERIALIZATION
#define OPERATOR_VALUE_SELECTOR_SERIALIZATION                       \
 private:                                                           \
  friend class boost::serialization::access;                        \
  template <class Archive>                                          \
  void serialize(Archive& ar, const unsigned int version) {         \
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(OperatorValueSelector); \
  }
#else
#define OPERATOR_VALUE_SELECTOR_SERIALIZATION
#endif

}  // namespace hayaku
