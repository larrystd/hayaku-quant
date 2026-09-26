#pragma once

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */

#include "strategy/decision/ConditionBase.h"

namespace hayaku {

class AddCondition : public ConditionBase {
 public:
  AddCondition();
  AddCondition(const ConditionPtr& cond1, const ConditionPtr& cond2);
  virtual ~AddCondition();

  virtual void _calculate() override;
  virtual void _reset() override;
  virtual ConditionPtr _clone() override;

 private:
  ConditionPtr cond1_;
  ConditionPtr cond2_;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
    ar& boost::serialization::make_nvp("m_cond1", cond1_);
    ar& boost::serialization::make_nvp("m_cond2", cond2_);
  }
#endif
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-02-16
 *      Author: fasiondog
 */

namespace hayaku {

class AndCondition : public ConditionBase {
 public:
  AndCondition();
  AndCondition(const ConditionPtr& cond1, const ConditionPtr& cond2);
  virtual ~AndCondition();

  virtual void _calculate() override;
  virtual void _reset() override;
  virtual ConditionPtr _clone() override;

 private:
  ConditionPtr cond1_;
  ConditionPtr cond2_;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
    ar& boost::serialization::make_nvp("m_cond1", cond1_);
    ar& boost::serialization::make_nvp("m_cond2", cond2_);
  }
#endif
};

}  // namespace hayaku

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */

namespace hayaku {

class DivCondition : public ConditionBase {
 public:
  DivCondition();
  DivCondition(const ConditionPtr& cond1, const ConditionPtr& cond2);
  virtual ~DivCondition();

  virtual void _calculate() override;
  virtual void _reset() override;
  virtual ConditionPtr _clone() override;

 private:
  ConditionPtr cond1_;
  ConditionPtr cond2_;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
    ar& boost::serialization::make_nvp("m_cond1", cond1_);
    ar& boost::serialization::make_nvp("m_cond2", cond2_);
  }
#endif
};

}  // namespace hayaku

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */

namespace hayaku {

class MultiCondition : public ConditionBase {
 public:
  MultiCondition();
  MultiCondition(const ConditionPtr& cond1, const ConditionPtr& cond2);
  virtual ~MultiCondition();

  virtual void _calculate() override;
  virtual void _reset() override;
  virtual ConditionPtr _clone() override;

 private:
  ConditionPtr cond1_;
  ConditionPtr cond2_;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
    ar& boost::serialization::make_nvp("m_cond1", cond1_);
    ar& boost::serialization::make_nvp("m_cond2", cond2_);
  }
#endif
};

}  // namespace hayaku

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */

namespace hayaku {

class OrCondition : public ConditionBase {
 public:
  OrCondition();
  OrCondition(const ConditionPtr& cond1, const ConditionPtr& cond2);
  virtual ~OrCondition();

  virtual void _calculate() override;
  virtual void _reset() override;
  virtual ConditionPtr _clone() override;

 private:
  ConditionPtr cond1_;
  ConditionPtr cond2_;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
    ar& boost::serialization::make_nvp("m_cond1", cond1_);
    ar& boost::serialization::make_nvp("m_cond2", cond2_);
  }
#endif
};

}  // namespace hayaku

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240223 added by fasiondog
 */

namespace hayaku {

class SubCondition : public ConditionBase {
 public:
  SubCondition();
  SubCondition(const ConditionPtr& cond1, const ConditionPtr& cond2);
  virtual ~SubCondition();

  virtual void _calculate() override;
  virtual void _reset() override;
  virtual ConditionPtr _clone() override;

 private:
  ConditionPtr cond1_;
  ConditionPtr cond2_;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
    ar& boost::serialization::make_nvp("m_cond1", cond1_);
    ar& boost::serialization::make_nvp("m_cond2", cond2_);
  }
#endif
};

}  // namespace hayaku
