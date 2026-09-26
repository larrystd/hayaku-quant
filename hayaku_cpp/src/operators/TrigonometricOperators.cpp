#include "TrigonometricOperators.h"

// ---- Merged implementation type from IAcos.h ----
/*
 * IAcos.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */
#include "Indicator.h"

namespace hayaku {

class IAcos : public IndicatorImp {
  INDICATOR_IMP(IAcos)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IAcos();
  virtual ~IAcos() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IAsin.h ----
/*
 * IAsin.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

namespace hayaku {

class IAsin : public IndicatorImp {
  INDICATOR_IMP(IAsin)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IAsin();
  virtual ~IAsin() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IAtan.h ----
/*
 * IAtan.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

namespace hayaku {

class IAtan : public IndicatorImp {
  INDICATOR_IMP(IAtan)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IAtan();
  virtual ~IAtan() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from ICos.h ----
/*
 * ICos.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

namespace hayaku {

class ICos : public IndicatorImp {
  INDICATOR_IMP(ICos)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ICos();
  virtual ~ICos() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from ISin.h ----
/*
 * ISin.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

namespace hayaku {

class ISin : public IndicatorImp {
  INDICATOR_IMP(ISin)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ISin();
  virtual ~ISin() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from ITan.h ----
/*
 * ITan.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

namespace hayaku {

class ITan : public IndicatorImp {
  INDICATOR_IMP(ITan)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ITan();
  virtual ~ITan() override;
};

} /* namespace hayaku */

// ---- Merged implementation from IAcos.cpp ----
/*
 * IAcos.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IAcos)
#endif

namespace hayaku {

IAcos::IAcos() : IndicatorImp("ACOS", 1) {}

IAcos::~IAcos() {}

void IAcos::_calculate(const Indicator& data) {
  size_t total = data.size();
  discard_ = data.discard();
  if (discard_ >= total) {
    discard_ = total;
    return;
  }
  _increment_calculate(data, discard_);
}

void IAcos::_increment_calculate(const Indicator& data, size_t start_pos) {
  auto const* src = data.data();
  auto* dst = this->data();
  for (size_t i = discard_, end = data.size(); i < end; ++i) {
    dst[i] = std::acos(src[i]);
  }
}

Indicator HAYAKU_API ACOS() { return Indicator(make_shared<IAcos>()); }

} /* namespace hayaku */

// ---- Merged implementation from IAsin.cpp ----
/*
 * IAsin.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IAsin)
#endif

namespace hayaku {

IAsin::IAsin() : IndicatorImp("ASIN", 1) {}

IAsin::~IAsin() {}

void IAsin::_calculate(const Indicator& data) {
  size_t total = data.size();
  discard_ = data.discard();
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  _increment_calculate(data, discard_);
}

void IAsin::_increment_calculate(const Indicator& data, size_t start_pos) {
  auto const* src = data.data();
  auto* dst = this->data();
  for (size_t i = start_pos, end = data.size(); i < end; ++i) {
    dst[i] = std::asin(src[i]);
  }
}

Indicator HAYAKU_API ASIN() { return Indicator(make_shared<IAsin>()); }

} /* namespace hayaku */

// ---- Merged implementation from IAtan.cpp ----
/*
 * IAtan.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IAtan)
#endif

namespace hayaku {

IAtan::IAtan() : IndicatorImp("ATAN", 1) {}

IAtan::~IAtan() {}

void IAtan::_calculate(const Indicator& data) {
  size_t total = data.size();
  discard_ = data.discard();
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  _increment_calculate(data, discard_);
}

void IAtan::_increment_calculate(const Indicator& data, size_t start_pos) {
  auto const* src = data.data();
  auto* dst = this->data();
  for (size_t i = start_pos, end = data.size(); i < end; ++i) {
    dst[i] = std::atan(src[i]);
  }
}

Indicator HAYAKU_API ATAN() { return Indicator(make_shared<IAtan>()); }

} /* namespace hayaku */

// ---- Merged implementation from ICos.cpp ----
/*
 * ICos.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ICos)
#endif

namespace hayaku {

ICos::ICos() : IndicatorImp("COS", 1) {}

ICos::~ICos() {}

void ICos::_calculate(const Indicator& data) {
  size_t total = data.size();
  discard_ = data.discard();
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  _increment_calculate(data, discard_);
}

void ICos::_increment_calculate(const Indicator& data, size_t start_pos) {
  auto const* src = data.data();
  auto* dst = this->data();
  for (size_t i = start_pos, end = data.size(); i < end; ++i) {
    dst[i] = std::cos(src[i]);
  }
}

Indicator HAYAKU_API COS() { return Indicator(make_shared<ICos>()); }

} /* namespace hayaku */

// ---- Merged implementation from ISin.cpp ----
/*
 * ISin.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ISin)
#endif

namespace hayaku {

ISin::ISin() : IndicatorImp("SIN", 1) {}

ISin::~ISin() {}

void ISin::_calculate(const Indicator& data) {
  size_t total = data.size();
  discard_ = data.discard();
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  _increment_calculate(data, discard_);
}

void ISin::_increment_calculate(const Indicator& data, size_t start_pos) {
  auto const* src = data.data();
  auto* dst = this->data();
  for (size_t i = start_pos, total = data.size(); i < total; ++i) {
    dst[i] = std::sin(src[i]);
  }
}

Indicator HAYAKU_API SIN() { return Indicator(make_shared<ISin>()); }

} /* namespace hayaku */

// ---- Merged implementation from ITan.cpp ----
/*
 * ITan.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ITan)
#endif

namespace hayaku {

ITan::ITan() : IndicatorImp("TAN", 1) {}

ITan::~ITan() {}

void ITan::_calculate(const Indicator& data) {
  size_t total = data.size();
  discard_ = data.discard();
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  _increment_calculate(data, discard_);
}

void ITan::_increment_calculate(const Indicator& data, size_t start_pos) {
  auto const* src = data.data();
  auto* dst = this->data();
  for (size_t i = start_pos, total = data.size(); i < total; ++i) {
    dst[i] = std::tan(src[i]);
  }
}

Indicator HAYAKU_API TAN() { return Indicator(make_shared<ITan>()); }

} /* namespace hayaku */
