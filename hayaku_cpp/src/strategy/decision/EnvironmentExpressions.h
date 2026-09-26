#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */

#include "strategy/decision/EnvironmentBase.h"

namespace hayaku {

class AddEnvironment : public EnvironmentBase {
 public:
  AddEnvironment();
  AddEnvironment(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
  virtual ~AddEnvironment();

  virtual void _calculate() override;
  virtual void _reset() override;
  virtual EnvironmentPtr _clone() override;

 private:
  EnvironmentPtr ev1_;
  EnvironmentPtr ev2_;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase);
    ar& boost::serialization::make_nvp("m_ev1", ev1_);
    ar& boost::serialization::make_nvp("m_ev2", ev2_);
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

class AndEnvironment : public EnvironmentBase {
 public:
  AndEnvironment();
  AndEnvironment(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
  virtual ~AndEnvironment();

  virtual void _calculate() override;
  virtual void _reset() override;
  virtual EnvironmentPtr _clone() override;

 private:
  EnvironmentPtr ev1_;
  EnvironmentPtr ev2_;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase);
    ar& boost::serialization::make_nvp("m_ev1", ev1_);
    ar& boost::serialization::make_nvp("m_ev2", ev2_);
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

class DivEnvironment : public EnvironmentBase {
 public:
  DivEnvironment();
  DivEnvironment(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
  virtual ~DivEnvironment();

  virtual void _calculate() override;
  virtual void _reset() override;
  virtual EnvironmentPtr _clone() override;

 private:
  EnvironmentPtr ev1_;
  EnvironmentPtr ev2_;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase);
    ar& boost::serialization::make_nvp("m_ev1", ev1_);
    ar& boost::serialization::make_nvp("m_ev2", ev2_);
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

class MultiEnvironment : public EnvironmentBase {
 public:
  MultiEnvironment();
  MultiEnvironment(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
  virtual ~MultiEnvironment();

  virtual void _calculate() override;
  virtual void _reset() override;
  virtual EnvironmentPtr _clone() override;

 private:
  EnvironmentPtr ev1_;
  EnvironmentPtr ev2_;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase);
    ar& boost::serialization::make_nvp("m_ev1", ev1_);
    ar& boost::serialization::make_nvp("m_ev2", ev2_);
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

class OrEnvironment : public EnvironmentBase {
 public:
  OrEnvironment();
  OrEnvironment(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
  virtual ~OrEnvironment();

  virtual void _calculate() override;
  virtual void _reset() override;
  virtual EnvironmentPtr _clone() override;

 private:
  EnvironmentPtr ev1_;
  EnvironmentPtr ev2_;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase);
    ar& boost::serialization::make_nvp("m_ev1", ev1_);
    ar& boost::serialization::make_nvp("m_ev2", ev2_);
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

class SubEnvironment : public EnvironmentBase {
 public:
  SubEnvironment();
  SubEnvironment(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
  virtual ~SubEnvironment();

  virtual void _calculate() override;
  virtual void _reset() override;
  virtual EnvironmentPtr _clone() override;

 private:
  EnvironmentPtr ev1_;
  EnvironmentPtr ev2_;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase);
    ar& boost::serialization::make_nvp("m_ev1", ev1_);
    ar& boost::serialization::make_nvp("m_ev2", ev2_);
  }
#endif
};

}  // namespace hayaku
