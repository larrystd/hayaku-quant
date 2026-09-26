#include "BooleanOperators.h"

// ---- Merged implementation type from IJumpDown.h ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */
#include "Indicator.h"

namespace hayaku {

/* Edge jump, jumping from greater than 0.0 to <= 0.0 */
class IJumpDown : public IndicatorImp {
  INDICATOR_IMP(IJumpDown)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IJumpDown();
  virtual ~IJumpDown() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IJumpUp.h ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */

namespace hayaku {

/* Edge jump, jumping from less than or equal to 0.0 to > 0.0 */
class IJumpUp : public IndicatorImp {
  INDICATOR_IMP(IJumpUp)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IJumpUp();
  virtual ~IJumpUp() override;
};

} /* namespace hayaku */

// ---- Merged implementation from IJumpDown.cpp ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IJumpDown)
#endif

namespace hayaku {

IJumpDown::IJumpDown() : IndicatorImp("JUMPDOWN", 1) {}

IJumpDown::~IJumpDown() {}

void IJumpDown::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  discard_ = ind.discard() + 1;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  _increment_calculate(ind, discard_);
}

void IJumpDown::_increment_calculate(const Indicator& ind, size_t start_pos) {
  const auto* src = ind.data();
  auto* dst = data();
  for (size_t i = start_pos, total = ind.size(); i < total; ++i) {
    if (src[i - 1] > 0.0 && src[i] <= 0.0) {
      dst[i] = 1.0;
    } else {
      dst[i] = 0.0;
    }
  }
}

Indicator JUMPDOWN() { return Indicator(make_shared<IJumpDown>()); }

} /* namespace hayaku */

// ---- Merged implementation from IJumpUp.cpp ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IJumpUp)
#endif

namespace hayaku {

IJumpUp::IJumpUp() : IndicatorImp("JUMPUP", 1) {}

IJumpUp::~IJumpUp() {}

void IJumpUp::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  discard_ = ind.discard() + 1;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  _increment_calculate(ind, discard_);
}

void IJumpUp::_increment_calculate(const Indicator& ind, size_t start_pos) {
  const auto* src = ind.data();
  auto* dst = data();
  for (size_t i = start_pos, total = ind.size(); i < total; ++i) {
    if (src[i - 1] <= 0.0 && src[i] > 0.0) {
      dst[i] = 1.0;
    } else {
      dst[i] = 0.0;
    }
  }
}

Indicator JUMPUP() { return Indicator(make_shared<IJumpUp>()); }

} /* namespace hayaku */
