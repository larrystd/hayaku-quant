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
    sg1_ = sg1->clone();
  }
  if (sg2) {
    sg2_ = sg2->clone();
  }
}

OperatorSignal::~OperatorSignal() {}

void OperatorSignal::_reset() {
  if (sg1_) {
    sg1_->reset();
  }
  if (sg2_) {
    sg2_->reset();
  }
}

void OperatorSignal::sub_sg_calculate(SignalPtr& sg, const KData& kdata) {
  HAYAKU_IF_RETURN(!sg, void());
  bool cycle = sg->getParam<bool>("cycle");
  if (cycle) {
    sg->startCycle(cycle_start_, cycle_end_);
  }
  sg->_calculate(kdata);
}

SignalPtr OperatorSignal::_clone() {
  return make_shared<OperatorSignal>(name_, sg1_, sg2_);
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
    : SignalBase(name), value_(value), mode_(mode) {
  if (sg) {
    sg_ = sg->clone();
    // The cycle attribute stays consistent with the child sg
    setParam<bool>("cycle", sg_->getParam<bool>("cycle"));
  }
  if (std::isnan(value_)) {
    value_ = 0.0;
  }
}

OperatorValueSignal::~OperatorValueSignal() {}

void OperatorValueSignal::_reset() {
  if (sg_) {
    sg_->reset();
  }
}

SignalPtr OperatorValueSignal::_clone() {
  return make_shared<OperatorValueSignal>(name_, sg_, value_);
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
  HAYAKU_IF_RETURN(!sg1_ && !sg2_, void());

  auto const* ks = kdata.data();
  size_t total = kdata.size();

  if (sg1_ && !sg2_) {
    sub_sg_calculate(sg1_, kdata);
    for (size_t i = 0; i < total; ++i) {
      _addSignal(ks[i].datetime, sg1_->getValue(ks[i].datetime));
    }
    return;
  }

  if (!sg1_ && sg2_) {
    sub_sg_calculate(sg2_, kdata);
    for (size_t i = 0; i < total; i++) {
      _addSignal(ks[i].datetime, sg2_->getValue(ks[i].datetime));
    }
    return;
  }

  sub_sg_calculate(sg1_, kdata);
  sub_sg_calculate(sg2_, kdata);
  for (size_t i = 0; i < total; ++i) {
    double value =
        sg1_->getValue(ks[i].datetime) + sg2_->getValue(ks[i].datetime);
    _addSignal(ks[i].datetime, value);
  }
}

SignalPtr operator+(const SignalPtr& sg1, const SignalPtr& sg2) {
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
  HAYAKU_IF_RETURN(!sg_ || std::isnan(value_), void());

  auto const* ks = kdata.data();
  size_t total = kdata.size();

  sg_->_calculate(kdata);
  if (value_ == 0.0) {
    for (size_t i = 0; i < total; ++i) {
      _addSignal(ks[i].datetime, sg_->getValue(ks[i].datetime));
    }
  } else {
    for (size_t i = 0; i < total; ++i) {
      double buy_value = sg_->getBuyValue(ks[i].datetime);
      if (buy_value > 0.0) {
        buy_value += value_;
      }
      double sell_value = sg_->getSellValue(ks[i].datetime);
      if (sell_value < 0.0) {
        sell_value -= value_;
      }
      _addSignal(ks[i].datetime, buy_value + sell_value);
    }
  }
}

SignalPtr operator+(const SignalPtr& sg, double value) {
  return make_shared<AddValueSignal>(sg, value);
}

SignalPtr operator+(double value, const SignalPtr& sg) {
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
  HAYAKU_IF_RETURN(!sg1_ || !sg2_, void());

  auto const* ks = kdata.data();
  size_t total = kdata.size();

  sub_sg_calculate(sg1_, kdata);
  sub_sg_calculate(sg2_, kdata);
  for (size_t i = 0; i < total; ++i) {
    double buy_value1 = sg1_->getBuyValue(ks[i].datetime);
    double buy_value2 = sg2_->getBuyValue(ks[i].datetime);
    double buy_value = (buy_value2 != 0.0) ? buy_value1 / buy_value2 : 0.0;
    double sell_value1 = sg1_->getSellValue(ks[i].datetime);
    double sell_value2 = sg2_->getSellValue(ks[i].datetime);
    double sell_value = (sell_value2 != 0.0) ? sell_value1 / sell_value2 : 0.0;
    double value = buy_value - sell_value;
    _addSignal(ks[i].datetime, value);
  }
}

SignalPtr operator/(const SignalPtr& sg1, const SignalPtr& sg2) {
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
  HAYAKU_IF_RETURN(!sg_ || value_ == 0.0 || std::isnan(value_), void());

  auto const* ks = kdata.data();
  size_t total = kdata.size();

  sg_->_calculate(kdata);
  if (mode_ == 0) {
    for (size_t i = 0; i < total; ++i) {
      _addSignal(ks[i].datetime, sg_->getValue(ks[i].datetime) / value_);
    }
  } else {
    for (size_t i = 0; i < total; ++i) {
      double sg_value = sg_->getValue(ks[i].datetime);
      if (sg_value != 0.0) {
        _addSignal(ks[i].datetime, value_ / sg_value);
      }
    }
  }
}

SignalPtr operator/(const SignalPtr& sg, double value) {
  return make_shared<DivValueSignal>(sg, value);
}

SignalPtr operator/(double value, const SignalPtr& sg) {
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
  HAYAKU_IF_RETURN(!sg1_ || !sg2_, void());

  auto const* ks = kdata.data();
  size_t total = kdata.size();

  sub_sg_calculate(sg1_, kdata);
  sub_sg_calculate(sg2_, kdata);
  for (size_t i = 0; i < total; ++i) {
    double buy_value =
        sg1_->getBuyValue(ks[i].datetime) * sg2_->getBuyValue(ks[i].datetime);
    double sell_value = 0.0 - sg1_->getSellValue(ks[i].datetime) *
                                  sg2_->getSellValue(ks[i].datetime);
    _addSignal(ks[i].datetime, buy_value + sell_value);
  }
}

SignalPtr operator*(const SignalPtr& sg1, const SignalPtr& sg2) {
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
  HAYAKU_IF_RETURN(!sg_ || value_ == 0.0 || std::isnan(value_), void());

  auto const* ks = kdata.data();
  size_t total = kdata.size();

  sg_->_calculate(kdata);
  for (size_t i = 0; i < total; ++i) {
    _addSignal(ks[i].datetime, sg_->getValue(ks[i].datetime) * value_);
  }
}

SignalPtr operator*(const SignalPtr& sg, double value) {
  return make_shared<MulValueSignal>(sg, value);
}

SignalPtr operator*(double value, const SignalPtr& sg) {
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
  HAYAKU_IF_RETURN(!sg1_ && !sg2_, void());

  auto const* ks = kdata.data();
  size_t total = kdata.size();

  if (sg1_ && !sg2_) {
    sub_sg_calculate(sg1_, kdata);
    for (size_t i = 0; i < total; ++i) {
      _addSignal(ks[i].datetime, sg1_->getValue(ks[i].datetime));
    }
    return;
  }

  if (!sg1_ && sg2_) {
    sub_sg_calculate(sg2_, kdata);
    for (size_t i = 0; i < total; i++) {
      double value = 0.0 - sg2_->getValue(ks[i].datetime);
      _addSignal(ks[i].datetime, value);
    }
    return;
  }

  sub_sg_calculate(sg1_, kdata);
  sub_sg_calculate(sg2_, kdata);
  for (size_t i = 0; i < total; ++i) {
    double value =
        sg1_->getValue(ks[i].datetime) - sg2_->getValue(ks[i].datetime);
    _addSignal(ks[i].datetime, value);
  }
}

SignalPtr operator-(const SignalPtr& sg1, const SignalPtr& sg2) {
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
  HAYAKU_IF_RETURN(!sg_ || std::isnan(value_), void());

  auto const* ks = kdata.data();
  size_t total = kdata.size();

  sg_->_calculate(kdata);
  if (value_ == 0.0) {
    if (mode_ == 0) {
      for (size_t i = 0; i < total; ++i) {
        _addSignal(ks[i].datetime, sg_->getValue(ks[i].datetime));
      }
    } else {
      for (size_t i = 0; i < total; ++i) {
        _addSignal(ks[i].datetime, 0.0 - (sg_->getValue(ks[i].datetime)));
      }
    }

  } else {
    if (mode_ == 0) {
      for (size_t i = 0; i < total; ++i) {
        double buy_value = sg_->getBuyValue(ks[i].datetime);
        if (buy_value > 0.0) {
          buy_value -= value_;
        }
        double sell_value = sg_->getSellValue(ks[i].datetime);
        if (sell_value < 0.0) {
          sell_value -= value_;
        }
        _addSignal(ks[i].datetime, buy_value + sell_value);
      }
    } else {
      for (size_t i = 0; i < total; ++i) {
        double buy_value = sg_->getBuyValue(ks[i].datetime);
        if (buy_value > 0.0) {
          buy_value = value_ - buy_value;
        }
        double sell_value = sg_->getSellValue(ks[i].datetime);
        if (sell_value < 0.0) {
          sell_value = value_ - sell_value;
        }
        _addSignal(ks[i].datetime, buy_value + sell_value);
      }
    }
  }
}

SignalPtr operator-(const SignalPtr& sg, double value) {
  return make_shared<SubValueSignal>(sg, value);
}

SignalPtr operator-(double value, const SignalPtr& sg) {
  return make_shared<SubValueSignal>(value, sg);
}

} /* namespace hayaku */
