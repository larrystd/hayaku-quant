#include "SeriesOperators.h"

// ---- Merged implementation type from IAlign.h ----
/*
 * IAlign.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-20
 *      Author: fasiondog
 */
#include "Indicator.h"

namespace hayaku {

class IAlign : public IndicatorImp {
  INDICATOR_IMP(IAlign)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IAlign();
  virtual ~IAlign() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IBackset.h ----
/*
 * IBackset.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-13
 *      Author: fasiondog
 */

namespace hayaku {

class IBackset : public IndicatorImp {
  INDICATOR_IMP(IBackset)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IBackset();
  virtual ~IBackset() override;

  virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IDiscard.h ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-31
 *      Author: fasiondog
 */

namespace hayaku {

/* Set the discard value in the way of an indicator formula */
class IDiscard : public IndicatorImp {
  INDICATOR_IMP(IDiscard)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IDiscard();
  virtual ~IDiscard() override;
  virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IDropna.h ----
/*
 * IDropna.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-28
 *      Author: fasiondog
 */

namespace hayaku {

class IDropna : public IndicatorImp {
  INDICATOR_IMP(IDropna)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IDropna();
  virtual ~IDropna() override;

  // Forbid the child node merging, it is already false by default at the upper
  // level virtual bool selfAlike(const IndicatorImp& other) const noexcept
  // override {
  //     return false;
  // }
};

} /* namespace hayaku */

// ---- Merged implementation type from IReplace.h ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-12
 *      Author: fasiondog
 */

namespace hayaku {

/*
 * Replace the given value, it is usually used to replace the Nan values
 */
class IReplace : public IndicatorImp {
  INDICATOR_IMP(IReplace)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IReplace();
  virtual ~IReplace() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IReverse.h ----
/*
 * IReverse.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

namespace hayaku {

class IReverse : public IndicatorImp {
  INDICATOR_IMP(IReverse)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IReverse();
  virtual ~IReverse() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from ISlice.h ----
/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-27
 *      Author: fasiondog
 */

namespace hayaku {

class ISlice : public IndicatorImp {
  INDICATOR_IMP(ISlice)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ISlice();
  ISlice(const PriceList&, int64_t start, int64_t end);
  virtual ~ISlice() override;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

// ---- Merged implementation from IAlign.cpp ----
/*
 * IAlign.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-20
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IAlign)
#endif

namespace hayaku {

IAlign::IAlign() : IndicatorImp("ALIGN") {
  setParam<DatetimeList>("align_date_list",
                         DatetimeList());  // The date sequence to align to
  setParam<bool>("fill_null",
                 true);  // Whether to fill the missing data with nan
}

IAlign::~IAlign() {}

void IAlign::_calculate(const Indicator& ind) {
  // The ref_date_list parameter affects the IndicatorImp globally, do not
  // modify it at will
  DatetimeList dates = getParam<DatetimeList>("align_date_list");
  size_t total = dates.size();

  // If align_date_list is invalid, try to use the dates in its own context as
  // the reference dates
  if (0 == total) {
    dates = getContext().getDatetimeList();
    total = dates.size();
  }

  m_result_num = ind.getResultNumber();
  _readyBuffer(total, m_result_num);

  size_t ind_total = ind.size();
  if (total == 0 || ind_total == 0) {
    m_discard = total;
    return;
  }

  bool fill_null = getParam<bool>("fill_null");

  // Handle the case where the passed indicator itself has no context dates and
  // cannot be aligned:
  // 1. ignore the fill_null parameter;
  // 2. if the data length is not greater than the date sequence length, align
  // at the right end,
  //    i.e. the last data corresponds to the last date and the missing data in
  //    front is discarded;
  // 3. if the data length is greater than the date sequence length, align at
  // the right end and
  //    discard the data in front that exceeds the date sequence.
  DatetimeList ind_dates = ind.getDatetimeList();
  if (ind_dates.size() == 0) {
    if (ind_total <= total) {
      size_t offset = total - ind_total;
      m_discard = offset + ind.discard();
      for (size_t r = 0; r < m_result_num; r++) {
        auto const* src = ind.data(r);
        auto* dst = this->data(r);
        memcpy(dst + m_discard, src + ind.discard(),
               sizeof(IndicatorImp::value_t) * (total - m_discard));
      }
      return;

    } else {
      // ind_total > total
      m_discard = 0;
      size_t offset = ind_total - total;
      if (ind.discard() > offset) {
        m_discard = ind.discard() - offset;
      }

      for (size_t r = 0; r < m_result_num; r++) {
        auto const* src = ind.data(r);
        auto* dst = this->data(r);
        for (size_t i = m_discard; i < total; i++) {
          dst[i] = src[i + offset];
        }
      }
      return;
    }
  }

  // The other indicator data that has the corresponding context dates
  // 1. If there is no exactly equal date, take the data of the closest date
  // earlier than the
  //    corresponding date;
  // 2. if there is a corresponding date, take the data of that date.
  if (fill_null) {
    size_t ind_idx = ind.discard();
    for (size_t i = 0; i < total; i++) {
      if (ind_idx >= ind_total) {
        break;
      }

      const Datetime& ind_date = ind_dates[ind_idx];
      if (ind_date == dates[i]) {
        for (size_t r = 0; r < m_result_num; r++) {
          _set(ind.get(ind_idx, r), i, r);
        }
        ind_idx++;

      } else if (ind_date < dates[i]) {
        size_t j = ind_idx + 1;
        while (j < ind_total && ind_dates[j] < dates[i]) {
          j++;
        }

        if (j >= ind_total) {
          break;
        }

        if (ind_dates[j] == dates[i]) {
          for (size_t r = 0; r < m_result_num; r++) {
            _set(ind.get(j, r), i, r);
          }
        }

        ind_idx = j + 1;
      }
    }

  } else {
    if (ind_dates[0] > dates[total - 1]) {
      // If the first data date > the last reference date, ignore everything
      m_discard = total;
      return;

    } else if (dates[0] > ind_dates[ind_total - 1]) {
      // If all the reference dates are later than the last date of ind_dates,
      // use the last data of ind_dates directly
      for (size_t r = 0; r < m_result_num; r++) {
        value_t val = ind.get(ind_total - 1, r);
        auto* dst = this->data(r);
        for (size_t i = 0; i < total; i++) {
          dst[i] = val;
        }
      }

    } else {
      size_t pos = 0;
      for (size_t i = 0; i < total; i++) {
        if (dates[i] >= ind_dates[0]) {
          pos = i;
          break;
        }
      }
      for (size_t ind_idx = 0; ind_idx < ind_total; ind_idx++) {
        const Datetime& ind_date = ind_dates[ind_idx];
        for (size_t i = pos; i < total; i++) {
          if (dates[i] < ind_date) {
            for (size_t r = 0; r < m_result_num; r++) {
              _set(ind.get(ind_idx - 1, r), i, r);
            }
          } else if (dates[i] == ind_date) {
            for (size_t r = 0; r < m_result_num; r++) {
              _set(ind.get(ind_idx, r), i, r);
            }
          } else {
            pos = i;
            break;
          }
        }
        if (dates[pos] >= ind_dates[ind_total - 1]) {
          break;
        }
      }
      if (pos < total) {
        if (dates[pos] >= ind_dates[ind_total - 1]) {
          for (size_t r = 0; r < m_result_num; r++) {
            for (size_t i = pos; i < total; i++) {
              _set(ind.get(ind_total - 1, r), i, r);
            }
          }
        } else {
          for (size_t r = 0; r < m_result_num; r++) {
            auto* dst = this->data(r);
            for (size_t i = pos; i < total; i++) {
              dst[i] = dst[i - 1];
            }
          }
        }
      }
    }
  }

  // Force updating m_discard again
  m_discard = 0;
  updateDiscard();
}

Indicator HAYAKU_API ALIGN(bool fill_null) {
  IndicatorImpPtr p = make_shared<IAlign>();
  p->setParam<bool>("fill_null", fill_null);
  return Indicator(p);
}

Indicator HAYAKU_API ALIGN(const DatetimeList& ref, bool fill_null) {
  IndicatorImpPtr p = make_shared<IAlign>();
  p->setParam<DatetimeList>("align_date_list", ref);
  p->setParam<bool>("fill_null", fill_null);
  return Indicator(p);
}

Indicator HAYAKU_API ALIGN(DatetimeList&& ref, bool fill_null) {
  IndicatorImpPtr p = make_shared<IAlign>();
  p->setParam<DatetimeList>("align_date_list", std::move(ref));
  p->setParam<bool>("fill_null", fill_null);
  return Indicator(p);
}

} /* namespace hayaku */

// ---- Merged implementation from IBackset.cpp ----
/*
 * IBackset.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-13
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IBackset)
#endif

namespace hayaku {

IBackset::IBackset() : IndicatorImp("BACKSET", 1) {
  m_is_serial = true;
  setParam<int>("n", 2);
}

IBackset::~IBackset() {}

void IBackset::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 1);
  }
}

void IBackset::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  int n = getParam<int>("n");
  m_discard = ind.discard();
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  auto const* src = ind.data();
  auto* dst = this->data();

  size_t i = total;
  size_t end_i = m_discard + n;
  if (end_i > total) {
    end_i = total;
  }
  while (i-- > end_i) {
    if (src[i] != 0.0) {
      dst[i] = 1.0;
      size_t j = i;
      size_t end_j = i - n + 1;
      while (j-- > end_j) {
        dst[j] = 1.0;
      }
    } else {
      if (dst[i] != 1.0) {
        dst[i] = 0.0;
      }
    }
  }

  // i = end_i - 1;
  while (true) {
    if (src[i] != 0.0) {
      for (size_t j = m_discard; j <= i; j++) {
        dst[j] = 1.0;
      }
      break;
    } else {
      dst[i] = 0.0;
      if (i == m_discard) {
        break;
      }
      i--;
    }
  }
}

void IBackset::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                                 size_t step) {
  size_t start = _get_step_start(curPos, step, ind.discard());
  if (ind[curPos] == 0.0) {
    for (size_t i = start; i <= curPos; i++) {
      _set(0.0, curPos);
    }
  } else {
    for (size_t i = start; i <= curPos; i++) {
      _set(1.0, curPos);
    }
  }
}

Indicator HAYAKU_API BACKSET(int n) {
  IndicatorImpPtr p = make_shared<IBackset>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API BACKSET(const IndParam& n) {
  IndicatorImpPtr p = make_shared<IBackset>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

// ---- Merged implementation from IDiscard.cpp ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-31
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IDiscard)
#endif

namespace hayaku {

IDiscard::IDiscard() : IndicatorImp("DISCARD") { setParam<int>("discard", 0); }

IDiscard::~IDiscard() {}

void IDiscard::_checkParam(const string& name) const {
  if (name == "discard") {
    HAYAKU_CHECK(getParam<int>(name) >= 0, "DISCARD: discard must >= 0!");
  }
}

void IDiscard::_calculate(const Indicator& data) {
  size_t total = data.size();
  HAYAKU_IF_RETURN(total == 0, void());

  size_t result_num = data.getResultNumber();
  _readyBuffer(total, result_num);

  m_discard = (size_t)getParam<int>("discard");
  if (m_discard < data.discard()) {
    m_discard = data.discard();
  }

  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  for (size_t r = 0; r < result_num; ++r) {
    auto const* src = data.data(r);
    auto* dst = this->data(r);
    memcpy(dst + m_discard, src + m_discard,
           (total - m_discard) * sizeof(value_t));
  }
}

Indicator HAYAKU_API DISCARD(int discard) {
  auto p = make_shared<IDiscard>();
  p->setParam<int>("discard", discard);
  return Indicator(p);
}

} /* namespace hayaku */

// ---- Merged implementation from IDropna.cpp ----
/*
 * IDropna.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-28
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IDropna)
#endif

namespace hayaku {

IDropna::IDropna() : IndicatorImp("DROPNA", 1) {
  m_need_self_alike_compare = true;
  setParam<DatetimeList>("align_date_list", DatetimeList());
}

IDropna::~IDropna() {}

void IDropna::_calculate(const Indicator& ind) {
  // The ref_date_list parameter affects the IndicatorImp globally, do not
  // modify it at will
  size_t total = ind.size();
  if (ind.discard() >= total) {
    m_discard = 0;
    setParam<DatetimeList>("align_date_list", DatetimeList());
    _readyBuffer(0, ind.getResultNumber());
    return;
  }

  m_result_num = ind.getResultNumber();
  size_t row_len = total - ind.discard();

#if CPP_STANDARD >= CPP_STANDARD_17
  std::unique_ptr<price_t[]> buf =
      std::make_unique<price_t[]>(m_result_num * row_len);
#else
  std::unique_ptr<price_t[]> buf(new price_t[m_result_num * row_len]);
#endif

  DatetimeList dates;
  size_t pos = 0;
  for (size_t i = ind.discard(); i < total; i++) {
    bool has_nan = false;
    for (size_t r = 0; r < m_result_num; r++) {
      if (std::isnan(ind.get(i, r))) {
        has_nan = true;
        break;
      }
    }

    if (!has_nan) {
      dates.push_back(ind.getDatetime(i));
      for (size_t r = 0; r < m_result_num; r++) {
        buf[pos + r * m_result_num] = ind.get(i, r);
      }
      pos++;
    }
  }

  _readyBuffer(pos / m_result_num, m_result_num);

  for (size_t r = 0; r < m_result_num; r++) {
    auto* dst = this->data(r);
    int start = r * m_result_num;
    for (size_t i = 0; i < pos; i++) {
      dst[i] = buf[start + i];
    }
  }

  m_discard = 0;
  setParam<DatetimeList>("align_date_list", dates);
}

Indicator HAYAKU_API DROPNA() { return Indicator(make_shared<IDropna>()); }

} /* namespace hayaku */

// ---- Merged implementation from IReplace.cpp ----
/*
 * IReplace.cpp
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IReplace)
#endif

namespace hayaku {

IReplace::IReplace() : IndicatorImp("REPLACE", 1) {
  setParam<double>("old_value", Null<double>());
  setParam<double>("new_value", 0.0);
  setParam<bool>("ignore_discard",
                 false);  // Ignore the discard of the passed indicator, i.e.
                          // replace all the data
}

IReplace::~IReplace() {}

void IReplace::_calculate(const Indicator& data) {
  size_t total = data.size();
  HAYAKU_IF_RETURN(total == 0, void());

  bool ignore_discard = getParam<bool>("ignore_discard");
  if (ignore_discard) {
    m_discard = 0;
  } else {
    m_discard = data.discard();
    if (m_discard >= total) {
      m_discard = total;
      return;
    }
  }

  value_t old_value = getParam<double>("old_value");
  value_t new_value = getParam<double>("new_value");

  auto const* src = data.data();
  auto* dst = this->data();

  if (std::isnan(old_value)) {
    for (size_t i = m_discard; i < total; ++i) {
      dst[i] = std::isnan(src[i]) ? new_value : src[i];
    }
  } else {
    value_t epsilon = std::numeric_limits<value_t>::epsilon();
    for (size_t i = m_discard; i < total; ++i) {
      dst[i] = (std::fabs(src[i] - old_value) < epsilon) ? new_value : src[i];
    }
  }

  // Update m_discard again
  updateDiscard();
}

Indicator HAYAKU_API REPLACE(double old_value, double new_value,
                             bool ignore_discard) {
  Indicator::value_t epsilon =
      std::numeric_limits<Indicator::value_t>::epsilon();
  HAYAKU_WARN_IF(std::fabs(old_value - new_value) < epsilon,
                 "The value to be replaced is equal to the replacement value! "
                 "Are you sure?");
  auto p = make_shared<IReplace>();
  p->setParam<double>("old_value", old_value);
  p->setParam<double>("new_value", new_value);
  p->setParam<bool>("ignore_discard", ignore_discard);
  return Indicator(p);
}

} /* namespace hayaku */

// ---- Merged implementation from IReverse.cpp ----
/*
 * IReverse.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IReverse)
#endif

namespace hayaku {

IReverse::IReverse() : IndicatorImp("REVERSE", 1) {}

IReverse::~IReverse() {}

void IReverse::_calculate(const Indicator& data) {
  size_t total = data.size();
  m_discard = data.discard();
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  _increment_calculate(data, m_discard);
}

void IReverse::_increment_calculate(const Indicator& data, size_t start_pos) {
  auto const* src = data.data();
  auto* dst = this->data();
  for (size_t i = start_pos, end = data.size(); i < end; ++i) {
    dst[i] = -src[i];
  }
}

Indicator HAYAKU_API REVERSE() { return Indicator(make_shared<IReverse>()); }

} /* namespace hayaku */

// ---- Merged implementation from ISlice.cpp ----
/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-27
 *      Author: fasiondog
 */

#include <cstring>
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ISlice)
#endif

namespace hayaku {

ISlice::ISlice() : IndicatorImp("SLICE", 1) {
  setParam<int>("result_index", -1);
  setParam<PriceList>("data", PriceList());
  setParam<int64_t>("start", 0);
  setParam<int64_t>("end", Null<int64_t>());
}

ISlice::ISlice(const PriceList& data, int64_t start, int64_t end)
    : IndicatorImp("SLICE", 1) {
  setParam<int>("result_index", 0);
  setParam<PriceList>("data", data);
  setParam<int64_t>("start", start);
  setParam<int64_t>("end", end);
}

ISlice::~ISlice() {}

void ISlice::_checkParam(const string& name) const {
  // if ("result_index" == name) {
  //     HAYAKU_ASSERT(getParam<int>("result_index") >= 0);
  // }
}

void ISlice::_calculate(const Indicator& data) {
  // On a leaf node, take its own data parameter directly
  if (isLeaf()) {
    m_discard = 0;
    const PriceList& x = getParam<const PriceList&>("data");
    size_t total = x.size();
    int64_t startix = getParam<int64_t>("start");
    if (startix < 0) {
      startix = total + startix;
    }
    HAYAKU_ERROR_IF_RETURN(startix < 0 || size_t(startix) >= total, void(),
                           "start {}, total {}", startix, total);

    int64_t endix = getParam<int64_t>("end");
    if (endix == Null<int64_t>()) {
      endix = total;
    } else if (endix < 0) {
      endix = total + endix;
    }
    HAYAKU_IF_RETURN(endix < 0 || size_t(endix) > total || startix == endix,
                     void());

    _readyBuffer(endix - startix, 1);
    auto* dst = this->data();
    for (int64_t i = startix; i < endix; ++i) {
      dst[i - startix] = x[i];
    }
    return;
  }

  // Not on a leaf node, ignore its own data parameter and regard its input as
  // the data in the function arguments
  int result_index = getParam<int>("result_index");
  HAYAKU_ERROR_IF_RETURN(
      result_index >= 0 && result_index >= data.getResultNumber(), void(),
      "result_index out of range!");

  size_t total = data.size();
  int64_t startix = getParam<int64_t>("start");
  if (startix < 0) {
    startix = total + startix;
  }
  HAYAKU_IF_RETURN(startix < 0 || size_t(startix) >= total, void());

  int64_t endix = getParam<int64_t>("end");
  if (endix == Null<int64_t>()) {
    endix = total;
  } else if (endix < 0) {
    endix = total + endix;
  }
  HAYAKU_IF_RETURN(endix < 0 || size_t(endix) > total || startix == endix,
                   void());

  if (result_index < 0) {
    size_t ret_num = data.getResultNumber();
    if (ret_num == 0) {
      ret_num = 1;
    }
    _readyBuffer(endix - startix, ret_num);
    for (size_t r = 0; r < ret_num; ++r) {
      auto const* src = data.data(r) + startix;
      auto* dst = this->data(r);
      memcpy(dst, src, (endix - startix) * sizeof(value_t));
    }
  } else {
    _readyBuffer(endix - startix, 1);
    auto const* src = data.data(result_index) + startix;
    auto* dst = this->data();
    memcpy(dst, src, (endix - startix) * sizeof(value_t));
  }

  // Update the discard number
  m_discard = data.discard() <= size_t(startix) ? 0 : data.discard() - startix;
}

Indicator HAYAKU_API SLICE(const PriceList& data, int64_t start, int64_t end) {
  return make_shared<ISlice>(data, start, end)->calculate();
}

Indicator HAYAKU_API SLICE(int64_t start, int64_t end, int result_index) {
  IndicatorImpPtr p = make_shared<ISlice>();
  p->setParam<int>("result_index", result_index);
  p->setParam<int64_t>("start", start);
  p->setParam<int64_t>("end", end);
  return Indicator(p);
}

}  // namespace hayaku
