#include "SignalExpressions.h"

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OperatorSignal)
#endif

namespace hayaku {

OperatorSignal::OperatorSignal() : SignalBase("SG_Operator") {}
OperatorSignal::OperatorSignal(const string& name) : SignalBase(name) {}

OperatorSignal::OperatorSignal(const string& name, const SignalPtr& sg1,
                               const SignalPtr& sg2)
    : SignalBase(name) {
  if (sg1) {
    m_sg1 = sg1->clone();
  }
  if (sg2) {
    m_sg2 = sg2->clone();
  }
}

OperatorSignal::~OperatorSignal() {}

void OperatorSignal::_reset() {
  if (m_sg1) {
    m_sg1->reset();
  }
  if (m_sg2) {
    m_sg2->reset();
  }
}

void OperatorSignal::sub_sg_calculate(SignalPtr& sg, const KData& kdata) {
  HAYAKU_IF_RETURN(!sg, void());
  bool cycle = sg->getParam<bool>("cycle");
  if (cycle) {
    sg->startCycle(m_cycle_start, m_cycle_end);
  }
  sg->_calculate(kdata);
}

SignalPtr OperatorSignal::_clone() {
  return make_shared<OperatorSignal>(m_name, m_sg1, m_sg2);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OperatorValueSignal)
#endif

namespace hayaku {

OperatorValueSignal::OperatorValueSignal() : SignalBase("SG_OperatorValue") {}
OperatorValueSignal::OperatorValueSignal(const string& name)
    : SignalBase(name) {}

OperatorValueSignal::OperatorValueSignal(const string& name,
                                         const SignalPtr& sg, double value,
                                         int mode)
    : SignalBase(name), m_value(value), m_mode(mode) {
  if (sg) {
    m_sg = sg->clone();
    // The cycle attribute stays consistent with the child sg
    setParam<bool>("cycle", m_sg->getParam<bool>("cycle"));
  }
  if (std::isnan(m_value)) {
    m_value = 0.0;
  }
}

OperatorValueSignal::~OperatorValueSignal() {}

void OperatorValueSignal::_reset() {
  if (m_sg) {
    m_sg->reset();
  }
}

SignalPtr OperatorValueSignal::_clone() {
  return make_shared<OperatorValueSignal>(m_name, m_sg, m_value);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::AddSignal)
#endif

namespace hayaku {

void AddSignal::_calculate(const KData& kdata) {
  HAYAKU_IF_RETURN(!m_sg1 && !m_sg2, void());

  auto const* ks = kdata.data();
  size_t total = kdata.size();

  if (m_sg1 && !m_sg2) {
    sub_sg_calculate(m_sg1, kdata);
    for (size_t i = 0; i < total; ++i) {
      _addSignal(ks[i].datetime, m_sg1->getValue(ks[i].datetime));
    }
    return;
  }

  if (!m_sg1 && m_sg2) {
    sub_sg_calculate(m_sg2, kdata);
    for (size_t i = 0; i < total; i++) {
      _addSignal(ks[i].datetime, m_sg2->getValue(ks[i].datetime));
    }
    return;
  }

  sub_sg_calculate(m_sg1, kdata);
  sub_sg_calculate(m_sg2, kdata);
  for (size_t i = 0; i < total; ++i) {
    double value =
        m_sg1->getValue(ks[i].datetime) + m_sg2->getValue(ks[i].datetime);
    _addSignal(ks[i].datetime, value);
  }
}

HAYAKU_API SignalPtr operator+(const SignalPtr& sg1, const SignalPtr& sg2) {
  return make_shared<AddSignal>(sg1, sg2);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::AddValueSignal)
#endif

namespace hayaku {

void AddValueSignal::_calculate(const KData& kdata) {
  HAYAKU_IF_RETURN(!m_sg || std::isnan(m_value), void());

  auto const* ks = kdata.data();
  size_t total = kdata.size();

  m_sg->_calculate(kdata);
  if (m_value == 0.0) {
    for (size_t i = 0; i < total; ++i) {
      _addSignal(ks[i].datetime, m_sg->getValue(ks[i].datetime));
    }
  } else {
    for (size_t i = 0; i < total; ++i) {
      double buy_value = m_sg->getBuyValue(ks[i].datetime);
      if (buy_value > 0.0) {
        buy_value += m_value;
      }
      double sell_value = m_sg->getSellValue(ks[i].datetime);
      if (sell_value < 0.0) {
        sell_value -= m_value;
      }
      _addSignal(ks[i].datetime, buy_value + sell_value);
    }
  }
}

HAYAKU_API SignalPtr operator+(const SignalPtr& sg, double value) {
  return make_shared<AddValueSignal>(sg, value);
}

HAYAKU_API SignalPtr operator+(double value, const SignalPtr& sg) {
  return make_shared<AddValueSignal>(sg, value);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::DivSignal)
#endif

namespace hayaku {

void DivSignal::_calculate(const KData& kdata) {
  HAYAKU_IF_RETURN(!m_sg1 || !m_sg2, void());

  auto const* ks = kdata.data();
  size_t total = kdata.size();

  sub_sg_calculate(m_sg1, kdata);
  sub_sg_calculate(m_sg2, kdata);
  for (size_t i = 0; i < total; ++i) {
    double buy_value1 = m_sg1->getBuyValue(ks[i].datetime);
    double buy_value2 = m_sg2->getBuyValue(ks[i].datetime);
    double buy_value = (buy_value2 != 0.0) ? buy_value1 / buy_value2 : 0.0;
    double sell_value1 = m_sg1->getSellValue(ks[i].datetime);
    double sell_value2 = m_sg2->getSellValue(ks[i].datetime);
    double sell_value = (sell_value2 != 0.0) ? sell_value1 / sell_value2 : 0.0;
    double value = buy_value - sell_value;
    _addSignal(ks[i].datetime, value);
  }
}

HAYAKU_API SignalPtr operator/(const SignalPtr& sg1, const SignalPtr& sg2) {
  return make_shared<DivSignal>(sg1, sg2);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::DivValueSignal)
#endif

namespace hayaku {

DivValueSignal::DivValueSignal(double value, const SignalPtr& sg)
    : OperatorValueSignal("SG_DivVlaue", sg, value, 1) {}

void DivValueSignal::_calculate(const KData& kdata) {
  HAYAKU_IF_RETURN(!m_sg || m_value == 0.0 || std::isnan(m_value), void());

  auto const* ks = kdata.data();
  size_t total = kdata.size();

  m_sg->_calculate(kdata);
  if (m_mode == 0) {
    for (size_t i = 0; i < total; ++i) {
      _addSignal(ks[i].datetime, m_sg->getValue(ks[i].datetime) / m_value);
    }
  } else {
    for (size_t i = 0; i < total; ++i) {
      double sg_value = m_sg->getValue(ks[i].datetime);
      if (sg_value != 0.0) {
        _addSignal(ks[i].datetime, m_value / sg_value);
      }
    }
  }
}

HAYAKU_API SignalPtr operator/(const SignalPtr& sg, double value) {
  return make_shared<DivValueSignal>(sg, value);
}

HAYAKU_API SignalPtr operator/(double value, const SignalPtr& sg) {
  return make_shared<DivValueSignal>(value, sg);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::MulSignal)
#endif

namespace hayaku {

void MulSignal::_calculate(const KData& kdata) {
  HAYAKU_IF_RETURN(!m_sg1 || !m_sg2, void());

  auto const* ks = kdata.data();
  size_t total = kdata.size();

  sub_sg_calculate(m_sg1, kdata);
  sub_sg_calculate(m_sg2, kdata);
  for (size_t i = 0; i < total; ++i) {
    double buy_value =
        m_sg1->getBuyValue(ks[i].datetime) * m_sg2->getBuyValue(ks[i].datetime);
    double sell_value = 0.0 - m_sg1->getSellValue(ks[i].datetime) *
                                  m_sg2->getSellValue(ks[i].datetime);
    _addSignal(ks[i].datetime, buy_value + sell_value);
  }
}

HAYAKU_API SignalPtr operator*(const SignalPtr& sg1, const SignalPtr& sg2) {
  return make_shared<MulSignal>(sg1, sg2);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::MulValueSignal)
#endif

namespace hayaku {

void MulValueSignal::_calculate(const KData& kdata) {
  HAYAKU_IF_RETURN(!m_sg || m_value == 0.0 || std::isnan(m_value), void());

  auto const* ks = kdata.data();
  size_t total = kdata.size();

  m_sg->_calculate(kdata);
  for (size_t i = 0; i < total; ++i) {
    _addSignal(ks[i].datetime, m_sg->getValue(ks[i].datetime) * m_value);
  }
}

HAYAKU_API SignalPtr operator*(const SignalPtr& sg, double value) {
  return make_shared<MulValueSignal>(sg, value);
}

HAYAKU_API SignalPtr operator*(double value, const SignalPtr& sg) {
  return make_shared<MulValueSignal>(sg, value);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::SubSignal)
#endif

namespace hayaku {

void SubSignal::_calculate(const KData& kdata) {
  HAYAKU_IF_RETURN(!m_sg1 && !m_sg2, void());

  auto const* ks = kdata.data();
  size_t total = kdata.size();

  if (m_sg1 && !m_sg2) {
    sub_sg_calculate(m_sg1, kdata);
    for (size_t i = 0; i < total; ++i) {
      _addSignal(ks[i].datetime, m_sg1->getValue(ks[i].datetime));
    }
    return;
  }

  if (!m_sg1 && m_sg2) {
    sub_sg_calculate(m_sg2, kdata);
    for (size_t i = 0; i < total; i++) {
      double value = 0.0 - m_sg2->getValue(ks[i].datetime);
      _addSignal(ks[i].datetime, value);
    }
    return;
  }

  sub_sg_calculate(m_sg1, kdata);
  sub_sg_calculate(m_sg2, kdata);
  for (size_t i = 0; i < total; ++i) {
    double value =
        m_sg1->getValue(ks[i].datetime) - m_sg2->getValue(ks[i].datetime);
    _addSignal(ks[i].datetime, value);
  }
}

HAYAKU_API SignalPtr operator-(const SignalPtr& sg1, const SignalPtr& sg2) {
  return make_shared<SubSignal>(sg1, sg2);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::SubValueSignal)
#endif

namespace hayaku {

SubValueSignal::SubValueSignal(double value, const SignalPtr& sg)
    : OperatorValueSignal("SG_SubVlaue", sg, value, 1) {}

void SubValueSignal::_calculate(const KData& kdata) {
  HAYAKU_IF_RETURN(!m_sg || std::isnan(m_value), void());

  auto const* ks = kdata.data();
  size_t total = kdata.size();

  m_sg->_calculate(kdata);
  if (m_value == 0.0) {
    if (m_mode == 0) {
      for (size_t i = 0; i < total; ++i) {
        _addSignal(ks[i].datetime, m_sg->getValue(ks[i].datetime));
      }
    } else {
      for (size_t i = 0; i < total; ++i) {
        _addSignal(ks[i].datetime, 0.0 - (m_sg->getValue(ks[i].datetime)));
      }
    }

  } else {
    if (m_mode == 0) {
      for (size_t i = 0; i < total; ++i) {
        double buy_value = m_sg->getBuyValue(ks[i].datetime);
        if (buy_value > 0.0) {
          buy_value -= m_value;
        }
        double sell_value = m_sg->getSellValue(ks[i].datetime);
        if (sell_value < 0.0) {
          sell_value -= m_value;
        }
        _addSignal(ks[i].datetime, buy_value + sell_value);
      }
    } else {
      for (size_t i = 0; i < total; ++i) {
        double buy_value = m_sg->getBuyValue(ks[i].datetime);
        if (buy_value > 0.0) {
          buy_value = m_value - buy_value;
        }
        double sell_value = m_sg->getSellValue(ks[i].datetime);
        if (sell_value < 0.0) {
          sell_value = m_value - sell_value;
        }
        _addSignal(ks[i].datetime, buy_value + sell_value);
      }
    }
  }
}

HAYAKU_API SignalPtr operator-(const SignalPtr& sg, double value) {
  return make_shared<SubValueSignal>(sg, value);
}

HAYAKU_API SignalPtr operator-(double value, const SignalPtr& sg) {
  return make_shared<SubValueSignal>(value, sg);
}

} /* namespace hayaku */
