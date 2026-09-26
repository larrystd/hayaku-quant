#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include "strategy/decision/SignalBase.h"

namespace hayaku {

class OperatorSignal : public SignalBase {
 public:
  OperatorSignal();
  explicit OperatorSignal(const string& name);
  OperatorSignal(const string& name, const SignalPtr& sg1,
                 const SignalPtr& sg2);
  virtual ~OperatorSignal();

  virtual void _reset() override final;

  virtual SignalPtr _clone() override;
  virtual void _calculate(const KData& kdata) override {}

 protected:
  void sub_sg_calculate(SignalPtr& sg, const KData& kdata);

 protected:
  SignalPtr m_sg1;
  SignalPtr m_sg2;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);
    ar& BOOST_SERIALIZATION_NVP(m_sg1);
    ar& BOOST_SERIALIZATION_NVP(m_sg2);
  }
#endif
};

#define OPERATOR_SIGNAL_IMP(classname, name)            \
 public:                                                \
  classname() : OperatorSignal(name) {}                 \
  classname(const SignalPtr& sg1, const SignalPtr& sg2) \
      : OperatorSignal(name, sg1, sg2) {}               \
  virtual ~classname() override {}                      \
  virtual SignalPtr _clone() override {                 \
    return make_shared<classname>(m_sg1, m_sg2);        \
  }                                                     \
  virtual void _calculate(const KData&) override;

#if HAYAKU_SUPPORT_SERIALIZATION
#define OPERATOR_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION      \
 private:                                                    \
  friend class boost::serialization::access;                 \
  template <class Archive>                                   \
  void serialize(Archive& ar, const unsigned int version) {  \
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);     \
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(OperatorSignal); \
  }
#else
#define OPERATOR_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

namespace hayaku {

class OperatorValueSignal : public SignalBase {
 public:
  OperatorValueSignal();
  explicit OperatorValueSignal(const string& name);
  OperatorValueSignal(const string& name, const SignalPtr& sg, double value,
                      int mode = 0);
  virtual ~OperatorValueSignal();

  virtual void _reset() override;
  virtual SignalPtr _clone() override;
  virtual void _calculate(const KData& kdata) override {}

 protected:
  double m_value{0.0};
  SignalPtr m_sg;
  int m_mode{
      0};  // It is valid for - and / only: 0: (sg, value), 1: (value, sg)

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);
    ar& BOOST_SERIALIZATION_NVP(m_sg);
    ar& BOOST_SERIALIZATION_NVP(m_value);
    ar& BOOST_SERIALIZATION_NVP(m_mode);
  }
#endif
};

#define OPERATOR_VALUE_SIGNAL_IMP(classname, name) \
 public:                                           \
  classname() : OperatorValueSignal(name) {}       \
  classname(const SignalPtr& sg, double value)     \
      : OperatorValueSignal(name, sg, value) {}    \
  virtual ~classname() override {}                 \
  virtual SignalPtr _clone() override {            \
    return make_shared<classname>(m_sg, m_value);  \
  }                                                \
  virtual void _calculate(const KData&) override;

#if HAYAKU_SUPPORT_SERIALIZATION
#define OPERATOR_VALUE_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION     \
 private:                                                         \
  friend class boost::serialization::access;                      \
  template <class Archive>                                        \
  void serialize(Archive& ar, const unsigned int version) {       \
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SignalBase);          \
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(OperatorValueSignal); \
  }
#else
#define OPERATOR_VALUE_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

namespace hayaku {

class AddSignal : public OperatorSignal {
  OPERATOR_SIGNAL_IMP(AddSignal, "SG_Add")
  OPERATOR_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

namespace hayaku {

class AddValueSignal : public OperatorValueSignal {
  OPERATOR_VALUE_SIGNAL_IMP(AddValueSignal, "SG_AddValue")
  OPERATOR_VALUE_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

namespace hayaku {

class AndSignal : public OperatorSignal {
  OPERATOR_SIGNAL_IMP(AndSignal, "SG_And")
  OPERATOR_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

namespace hayaku {

class DivSignal : public OperatorSignal {
  OPERATOR_SIGNAL_IMP(DivSignal, "SG_Div")
  OPERATOR_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

namespace hayaku {

class DivValueSignal : public OperatorValueSignal {
  OPERATOR_VALUE_SIGNAL_IMP(DivValueSignal, "SG_DivValue")
  OPERATOR_VALUE_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  DivValueSignal(double value, const SignalPtr& sg);
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

namespace hayaku {

class MulSignal : public OperatorSignal {
  OPERATOR_SIGNAL_IMP(MulSignal, "SG_Mul")
  OPERATOR_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

namespace hayaku {

class MulValueSignal : public OperatorValueSignal {
  OPERATOR_VALUE_SIGNAL_IMP(MulValueSignal, "SG_MulValue")
  OPERATOR_VALUE_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

namespace hayaku {

class OrSignal : public OperatorSignal {
  OPERATOR_SIGNAL_IMP(OrSignal, "SG_Or")
  OPERATOR_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

namespace hayaku {

class SubSignal : public OperatorSignal {
  OPERATOR_SIGNAL_IMP(SubSignal, "SG_Sub")
  OPERATOR_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

namespace hayaku {

class SubValueSignal : public OperatorValueSignal {
  OPERATOR_VALUE_SIGNAL_IMP(SubValueSignal, "SG_AddValue")
  OPERATOR_VALUE_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  SubValueSignal(double value, const SignalPtr& sg);
};

} /* namespace hayaku */
