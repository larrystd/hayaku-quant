#include "WindowOperators.h"

/*
 * IBarsCount.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-12
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IBARSCOUNT_H_
#define INDICATOR_IMP_IBARSCOUNT_H_

#include "Indicator.h"

namespace hayaku {

class IBarsCount : public IndicatorImp {
  INDICATOR_IMP(IBarsCount)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IBarsCount();
  virtual ~IBarsCount() override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_IBARSCOUNT_H_ */

/*
 * IBarsLast.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IBARSLAST_H_
#define INDICATOR_IMP_IBARSLAST_H_

namespace hayaku {

class IBarsLast : public IndicatorImp {
  INDICATOR_IMP(IBarsLast)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IBarsLast();
  virtual ~IBarsLast() override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_IBARSLAST_H_ */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-06-01
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IBARSLASTCOUNT_H_
#define INDICATOR_IMP_IBARSLASTCOUNT_H_

namespace hayaku {

/* Count the number of the consecutive periods satisfying the condition */
class IBarsLastCount : public IndicatorImp {
  INDICATOR_IMP(IBarsLastCount)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IBarsLastCount();
  virtual ~IBarsLastCount() override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_IBARSLASTCOUNT_H_ */

/*
 * IBarsLasts.h
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-17
 *      Author: hayaku
 */

#pragma once
#ifndef INDICATOR_IMP_IBARSLASTS_H_
#define INDICATOR_IMP_IBARSLASTS_H_

namespace hayaku {

class IBarsLasts : public IndicatorImp {
  INDICATOR_IMP(IBarsLasts)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IBarsLasts();
  virtual ~IBarsLasts() override;

  virtual void _dyn_calculate(const Indicator&) override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_IBARSLASTS_H_ */

/*
 * IBarsSince.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IBARSSINCE_H_
#define INDICATOR_IMP_IBARSSINCE_H_

namespace hayaku {

/*
 * The position where the condition first holds within N periods; when N is 0 it
 * is the whole sequence Usage: BARSSINCEN(X,N): the number of periods from the
 * first time X is not 0 within N periods until now, N is a constant
 * BARSSINCEN(X,N): For example: BARSSINCEN(HIGH>10,10) gives the number of
 * periods from the time the stock price exceeds 10 yuan within 10 periods until
 * now
 */
class IBarsSince : public IndicatorImp {
  INDICATOR_IMP(IBarsSince)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IBarsSince();
  virtual ~IBarsSince() override;
  virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_IBARSSINCE_H_ */

/*
 * ISumBars.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-5
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ISUMBARS_H_
#define INDICATOR_IMP_ISUMBARS_H_

namespace hayaku {

class ISumBars : public IndicatorImp {
  INDICATOR_IMP(ISumBars)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ISumBars();
  virtual ~ISumBars() override;

  virtual void _dyn_calculate(const Indicator&) override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_ISUMBARS_H_ */

/*
 * IBarsCount.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-12
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IBarsCount)
#endif

namespace hayaku {

IBarsCount::IBarsCount() : IndicatorImp("BARSCOUNT", 1) {}

IBarsCount::~IBarsCount() {}

void IBarsCount::_calculate(const Indicator& ind) {
  KData k = ind.getContext();
  Stock stk = k.getStock();

  size_t total = ind.size();
  discard_ = ind.discard();
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  auto* dst = this->data();

  // If there is no context, take the discard of this indicator for the
  // calculation directly
  if (stk.isNull()) {
    for (size_t i = discard_; i < total; i++) {
      dst[i] = i + 1 - discard_;
    }

    return;
  }

  // For the 1-minute line, get the number of the trading minutes of the day
  KQuery q = k.getQuery();
  auto const* krecords = k.data();
  if (q.kType() == KQuery::MIN) {
    Datetime pre_d = krecords[discard_].datetime.startOfDay();
    size_t count = 0;
    for (size_t i = discard_; i < total; i++) {
      Datetime d = krecords[i].datetime.startOfDay();
      if (d != pre_d) {
        pre_d = d;
        count = 0;
      }
      dst[i] = ++count;
    }

    return;
  }

  // Get the total number of the trading days since the listing
  size_t k_start_pos = k.startPos();
  if (k_start_pos != Null<size_t>()) {
    for (size_t i = discard_; i < total; i++) {
      dst[i] = 1 + k_start_pos + i;
    }
  }

  return;
}

Indicator BARSCOUNT() { return Indicator(make_shared<IBarsCount>()); }

} /* namespace hayaku */

/*
 * IBarsLast.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IBarsLast)
#endif

namespace hayaku {

IBarsLast::IBarsLast() : IndicatorImp("BARSLAST", 1) {}

IBarsLast::~IBarsLast() {}

void IBarsLast::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  discard_ = ind.discard();
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  auto const* src = ind.data();
  auto* dst = this->data();

  if (total == discard_ + 1) {
    if (src[discard_] != 0.0) {
      dst[discard_] = 0.0;
    } else {
      discard_ = total;
    }
    return;
  }

  size_t pos = total;
  for (size_t i = total - 1; i != discard_; i--) {
    if (src[i] != 0.0) {
      for (size_t j = i; j < pos; j++) {
        dst[j] = j - i;
      }
      pos = i;
    }
  }

  if (src[discard_] != 0.0) {
    for (size_t i = discard_; i < pos; i++) {
      dst[i] = i - discard_;
    }
  } else {
    discard_ = pos;
  }
}

Indicator BARSLAST() { return Indicator(make_shared<IBarsLast>()); }

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-06-01
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IBarsLastCount)
#endif

namespace hayaku {

IBarsLastCount::IBarsLastCount() : IndicatorImp("BARSLASTCOUNT", 1) {}

IBarsLastCount::~IBarsLastCount() {}

void IBarsLastCount::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  discard_ = ind.discard();
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  auto const* src = ind.data();
  auto* dst = this->data();
  if (src[discard_] > 0.0) {
    dst[discard_] = 1.0;
  } else {
    dst[discard_] = 0;
  }

  for (size_t i = discard_ + 1; i < total; ++i) {
    if (src[i] > 0.0) {
      dst[i] = dst[i - 1] + 1.0;
    } else {
      dst[i] = 0.0;
    }
  }
}

Indicator BARSLASTCOUNT() { return Indicator(make_shared<IBarsLastCount>()); }

} /* namespace hayaku */

/*
 * IBarsLasts.cpp
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-17
 *      Author: hayaku
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IBarsLasts)
#endif

namespace hayaku {

IBarsLasts::IBarsLasts() : IndicatorImp("BARSLASTS", 1) {
  setParam<int>("n", 0);
}

IBarsLasts::~IBarsLasts() {}

void IBarsLasts::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  size_t ind_discard = ind.discard();
  if (ind_discard >= total) {
    discard_ = total;
    return;
  }

  // Get the parameter N
  int n = getParam<int>("n");

  // Parameter validation: if n <= 0, return a sequence of all NaN
  if (n <= 0) {
    discard_ = total;
    return;
  }

  auto const* src = ind.data();
  auto* dst = this->data();

  // Special case: there is only one valid data point
  if (total == ind_discard + 1) {
    if (src[ind_discard] != 0.0 && n == 1) {
      dst[ind_discard] = 0.0;
      discard_ = ind_discard;
    } else {
      discard_ = total;
    }
    return;
  }

  // Record the positions where the condition holds
  std::vector<size_t> true_positions;
  for (size_t i = ind_discard; i < total; i++) {
    if (src[i] != 0.0) {
      true_positions.push_back(i);
    }
  }

  // If the condition holds fewer than N times, return NaN for everything
  if (true_positions.size() < n) {
    discard_ = total;
    return;
  }

  // Find the position where the condition holds for the N-th time; it is valid
  // from that position
  size_t first_valid_pos = true_positions[n - 1];

  // Traverse in reverse order, similar to the BARSLAST logic
  size_t pos = total;
  size_t count = 0;  // Records how many times the condition has held

  for (size_t i = total - 1; i >= first_valid_pos; i--) {
    if (src[i] != 0.0) {
      count++;
      size_t target_idx = true_positions.size() - count;

      size_t target_pos = true_positions[target_idx];
      size_t base_pos = true_positions[target_idx + 1 - n];
      size_t target_pos_end = (target_idx + 1) >= true_positions.size()
                                  ? pos
                                  : true_positions[target_idx + 1];
      size_t pos_diff = target_pos - base_pos;
      for (size_t j = target_pos; j < target_pos_end; j++) {
        dst[j] = j + pos_diff - target_pos;
      }
      pos = i;
    }

    if (i == first_valid_pos) {
      break;
    }
  }

  updateDiscard();
}

void IBarsLasts::_dyn_calculate(const Indicator& ind) {
  // Get the dynamic parameter n
  Indicator ind_param(getIndParamImp("n"));
  HAYAKU_CHECK(ind_param.size() == ind.size(),
               "ind_param->size()={}, ind.size()={}!", ind_param.size(),
               ind.size());

  size_t total = ind.size();
  size_t ind_discard = ind.discard();
  discard_ = std::max(ind_discard, ind_param.discard());

  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  auto const* src = ind.data();
  auto* dst = this->data();
  auto const* n_data = ind_param.data();

  // Calculate for every position separately
  for (size_t i = discard_; i < total; i++) {
    int n = static_cast<int>(n_data[i]);

    // Parameter validation: if n <= 0, return NaN
    if (n <= 0) {
      dst[i] = Null<price_t>();
      continue;
    }

    // Search backward from the current position for the position where the
    // condition holds for the n-th time
    int count = 0;
    size_t target_pos = Null<size_t>();

    for (size_t j = i; j >= ind_discard; j--) {
      if (src[j] != 0.0) {
        count++;
        if (count == n) {
          target_pos = j;
          break;
        }
      }
      if (j == ind_discard) {
        break;
      }
    }

    // If the position where the condition holds for the n-th time is found,
    // calculate the distance
    if (target_pos != Null<size_t>()) {
      dst[i] = static_cast<price_t>(i - target_pos);
    } else {
      dst[i] = Null<price_t>();
    }
  }

  updateDiscard();
}

Indicator BARSLASTS(int n) {
  auto p = make_shared<IBarsLasts>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator BARSLASTS(const IndParam& n) {
  auto p = make_shared<IBarsLasts>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 * IBarsSince.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IBarsSince)
#endif

namespace hayaku {

IBarsSince::IBarsSince() : IndicatorImp("BARSSINCE", 1) {
  setParam<int>("n", 0);
}

IBarsSince::~IBarsSince() {}

void IBarsSince::_checkParam(const string& name) const {
  if (name == "n") {
    HAYAKU_CHECK(getParam<int>("n") >= 0, "n must >= 0!");
  }
}

void IBarsSince::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  discard_ = ind.discard();
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  auto const* src = ind.data();
  auto* dst = this->data();

  int n = getParam<int>("n");
  if (0 == n) {
    bool found = false;
    size_t pos = discard_;
    for (size_t i = discard_; i < total; ++i) {
      if (found) {
        dst[i] = i - pos;
      } else {
        if (!std::isnan(src[i]) && src[i] != 0.0) {
          found = true;
          pos = i;
          dst[i] = 0.0;
        }
      }
    }

    discard_ = pos;
    return;
  }

  if (1 == n) {
    for (size_t i = discard_; i < total; ++i) {
      if (!std::isnan(src[i]) && src[i] != 0.0) {
        dst[i] = 0.0;
      } else {
        dst[i] = Null<value_t>();
      }
    }
    updateDiscard();
    return;
  }

  size_t first = discard_ + n - 1;
  for (size_t i = first; i < total; ++i) {
    size_t pos = 0;
    bool found = false;
    for (size_t j = i + 1 - n; j < i; j++) {
      if (!std::isnan(src[j]) && src[j] != 0.0) {
        pos = i - j;
        found = true;
        break;
      }
    }
    dst[i] = found ? pos : Null<value_t>();
  }
  updateDiscard();
}

Indicator BARSSINCE() { return Indicator(make_shared<IBarsSince>()); }

Indicator BARSSINCEN(int n) {
  auto p = make_shared<IBarsSince>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 * ISumBars.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-5
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ISumBars)
#endif

namespace hayaku {

ISumBars::ISumBars() : IndicatorImp("SUMBARS", 1) { setParam<double>("a", 0); }

ISumBars::~ISumBars() {}

void ISumBars::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  discard_ = ind.discard();
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  auto const* src = ind.data();
  auto* dst = this->data();

  double a = getParam<double>("a");
  if (total == discard_ + 1) {
    if (src[discard_] >= a) {
      dst[discard_] = 0.0;
    } else {
      discard_ = total;
    }
    return;
  }

  size_t null_pos = Null<size_t>();
  size_t start = total - 1;
  size_t pos = start;
  size_t last_pos = start;
  double sum = src[pos];
  for (size_t i = start; i >= discard_; i--) {
    if (i != start) {
      sum = sum - src[i + 1];
    }

    if (i < pos) {
      sum = src[i];
      pos = i;
    }

    if (sum < a) {
      if (pos >= 1) {
        for (size_t j = pos - 1; j >= discard_; j--) {
          sum += src[j];
          if (sum >= a) {
            pos = j;
            break;
          }

          if (j == discard_) {
            pos = null_pos;
            break;
          }
        }
      } else {
        pos = null_pos;
      }
    }

    if (pos != null_pos) {
      dst[i] = i - pos;
    }

    if (i == discard_ || pos == null_pos) {
      last_pos = i;
      break;
    }
  }

  discard_ = pos == null_pos ? last_pos + 1 : last_pos;
}

void ISumBars::_dyn_calculate(const Indicator& ind) {
  Indicator ind_param(getIndParamImp("a"));
  HAYAKU_CHECK(ind_param.size() == ind.size(),
               "ind_param->size()={}, ind.size()={}!", ind_param.size(),
               ind.size());
  discard_ = std::max(ind.discard(), ind_param.discard());
  size_t total = ind.size();
  HAYAKU_IF_RETURN(0 == total || discard_ >= total, void());

  for (size_t i = discard_; i < total; i++) {
    price_t a = ind_param[i];
    price_t sum = 0.0;
    price_t n = Null<price_t>();
    for (size_t j = i; j >= ind.discard(); j--) {
      sum += ind[j];
      if (sum >= a) {
        n = price_t(i - j);  // Same as dst[i] = i - pos of the static version
                             // (periods >= 0)
        break;
      }
      if (j == ind.discard()) {
        break;
      }
    }
    _set(n, i);
  }
}

Indicator SUMBARS(double a) {
  IndicatorImpPtr p = make_shared<ISumBars>();
  p->setParam<double>("a", a);
  return Indicator(p);
}

Indicator SUMBARS(const IndParam& a) {
  IndicatorImpPtr p = make_shared<ISumBars>();
  p->setIndParam("a", a);
  return Indicator(p);
}

} /* namespace hayaku */
