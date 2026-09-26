#include "SelectorExpressions.h"

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OperatorAddSelector)
#endif

namespace hayaku {

StrategyWeightList OperatorAddSelector::_getSelected(Datetime date) {
  return getUnionSelected(date, [](double w1, double w2) { return w1 + w2; });
}

HAYAKU_API SelectorPtr operator+(const SelectorPtr& se1,
                                 const SelectorPtr& se2) {
  return make_shared<OperatorAddSelector>(se1, se2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OperatorAddValueSelector)
#endif

namespace hayaku {

StrategyWeightList OperatorAddValueSelector::_getSelected(Datetime date) {
  StrategyWeightList ret;
  HAYAKU_IF_RETURN(!m_se, ret);

  ret = m_se->getSelected(date);
  for (auto& sw : ret) {
    sw.weight += m_value;
  }

  return ret;
}

HAYAKU_API SelectorPtr operator+(const SelectorPtr& se, double value) {
  return make_shared<OperatorAddValueSelector>(se, value);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OperatorDivSelector)
#endif

namespace hayaku {

StrategyWeightList OperatorDivSelector::_getSelected(Datetime date) {
  return getIntersectionSelected(date,
                                 [](double w1, double w2) { return w1 / w2; });
}

HAYAKU_API SelectorPtr operator/(const SelectorPtr& se1,
                                 const SelectorPtr& se2) {
  return make_shared<OperatorDivSelector>(se1, se2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OperatorDivValueSelector)
#endif

namespace hayaku {

StrategyWeightList OperatorDivValueSelector::_getSelected(Datetime date) {
  StrategyWeightList ret;
  HAYAKU_IF_RETURN(!m_se, ret);

  ret = m_se->getSelected(date);
  for (auto& sw : ret) {
    sw.weight /= m_value;
  }

  return ret;
}

HAYAKU_API SelectorPtr operator/(const SelectorPtr& se, double value) {
  return make_shared<OperatorDivValueSelector>(se, value);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OperatorInvertDivValueSelector)
#endif

namespace hayaku {

StrategyWeightList OperatorInvertDivValueSelector::_getSelected(Datetime date) {
  StrategyWeightList ret;
  HAYAKU_IF_RETURN(!m_se, ret);

  ret = m_se->getSelected(date);
  for (auto& sw : ret) {
    sw.weight = m_value / sw.weight;
  }

  return ret;
}

HAYAKU_API SelectorPtr operator/(double value, const SelectorPtr& se) {
  return make_shared<OperatorInvertDivValueSelector>(se, value);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OperatorInvertSubValueSelector)
#endif

namespace hayaku {

StrategyWeightList OperatorInvertSubValueSelector::_getSelected(Datetime date) {
  StrategyWeightList ret;
  HAYAKU_IF_RETURN(!m_se, ret);

  ret = m_se->getSelected(date);
  for (auto& sw : ret) {
    sw.weight = m_value - sw.weight;
  }

  return ret;
}

HAYAKU_API SelectorPtr operator-(double value, const SelectorPtr& se) {
  return make_shared<OperatorInvertSubValueSelector>(se, value);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OperatorMulSelector)
#endif

namespace hayaku {

StrategyWeightList OperatorMulSelector::_getSelected(Datetime date) {
  return getIntersectionSelected(date,
                                 [](double w1, double w2) { return w1 * w2; });
}

HAYAKU_API SelectorPtr operator*(const SelectorPtr& se1,
                                 const SelectorPtr& se2) {
  return make_shared<OperatorMulSelector>(se1, se2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OperatorMulValueSelector)
#endif

namespace hayaku {

StrategyWeightList OperatorMulValueSelector::_getSelected(Datetime date) {
  StrategyWeightList ret;
  HAYAKU_IF_RETURN(!m_se, ret);

  ret = m_se->getSelected(date);
  for (auto& sw : ret) {
    sw.weight *= m_value;
  }

  return ret;
}

HAYAKU_API SelectorPtr operator*(const SelectorPtr& se, double value) {
  return make_shared<OperatorMulValueSelector>(se, value);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OperatorSubSelector)
#endif

namespace hayaku {

StrategyWeightList OperatorSubSelector::_getSelected(Datetime date) {
  return getUnionSelected(date, [](double w1, double w2) { return w1 - w2; });
}

HAYAKU_API SelectorPtr operator-(const SelectorPtr& se1,
                                 const SelectorPtr& se2) {
  return make_shared<OperatorSubSelector>(se1, se2);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OperatorSubValueSelector)
#endif

namespace hayaku {

StrategyWeightList OperatorSubValueSelector::_getSelected(Datetime date) {
  StrategyWeightList ret;
  HAYAKU_IF_RETURN(!m_se, ret);

  ret = m_se->getSelected(date);
  for (auto& sw : ret) {
    sw.weight -= m_value;
  }

  return ret;
}

HAYAKU_API SelectorPtr operator-(const SelectorPtr& se, double value) {
  return make_shared<OperatorSubValueSelector>(se, value);
}

}  // namespace hayaku
