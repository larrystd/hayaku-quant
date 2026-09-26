#include "SeriesOperators.h"

// ---- Merged implementation type from ICval.h ----
/*
 * ConstantValue.h
 *
 *  Created on: 2017-6-25
 *      Author: Administrator
 */
#include "Indicator.h"

namespace hayaku {

class ICval : public IndicatorImp {
  INDICATOR_IMP(ICval)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ICval();
  ICval(double value, size_t discard);
  virtual ~ICval() override;
  virtual void _checkParam(const string& name) const override;

  virtual bool selfAlike(const IndicatorImp& other) const noexcept override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IPriceList.h ----
/*
 * IPriceList.h
 *
 *  Created on: 2013-2-12
 *      Author: fasiondog
 */

namespace hayaku {

// Find the last Null<price_t> in the array and set all the preceding data to
// Null
class IPriceList : public IndicatorImp {
  INDICATOR_IMP(IPriceList)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IPriceList();
  IPriceList(const PriceList&, int discard);
  IPriceList(PriceList&&, int discard);
  IPriceList(size_t size, double value, int discard);
  virtual ~IPriceList() override;
  virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IContext.h ----
/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-28
 *      Author: fasiondog
 */

namespace hayaku {

class IContext : public IndicatorImp {
 public:
  IContext();
  explicit IContext(const Indicator& ref_ind);
  virtual ~IContext() override;

  virtual string str() const override;
  virtual string formula() const override;
  virtual void _calculate(const Indicator& data) override;
  virtual IndicatorImpPtr _clone() override;

  virtual bool selfAlike(const IndicatorImp& other) const noexcept override;

  KData getContextKdata() const;

  // Forcefully set its own context data
  void setSelfContext(const KData& kdata) { m_ref_ind.setContext(kdata); }

  // Get its own context data
  KData getSelfContext() const { return m_ref_ind.getContext(); }

 private:
  Indicator m_ref_ind;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
 private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(IndicatorImp);
    ar& BOOST_SERIALIZATION_NVP(m_ref_ind);
  }
#endif
};

}  // namespace hayaku

namespace hayaku {

Indicator Indicator::operator()(const Indicator& ind) {
  HAYAKU_IF_RETURN(!m_imp, Indicator());
  HAYAKU_IF_RETURN(!ind.getImp(), Indicator(m_imp));

  IndicatorImp const* context_ptr = dynamic_cast<IContext const*>(m_imp.get());
  if (context_ptr != nullptr) {
    auto p = make_shared<IContext>(ind);
    return p->calculate();
  }

  // AST node pruning: when the operator (m_imp) is semantically equivalent to
  // the operand (ind), reuse the already calculated ind (which holds a valid
  // buffer), instead of returning an empty clone shell of m_imp (the size of
  // m_imp is 0 when it has not been calculated). Note: reusing ind makes the
  // return value share the underlying node with ind, which relies on the
  // immutable parameter semantics of hayaku (alike has verified that m_params
  // are the same, setParam triggers an in-place recalculation and the sharing
  // is safe).
  if (m_imp->alike(*ind.getImp())) {
    return ind;
  }

  IndicatorImpPtr p = m_imp->clone();
  p->add(IndicatorImp::OP, IndicatorImpPtr(), ind.getImp());
  return p->calculate();
}

}  // namespace hayaku

// ---- Merged implementation type from IResult.h ----
/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-12
 *      Author: fasiondog
 */

namespace hayaku {

class IResult : public IndicatorImp {
  INDICATOR_IMP(IResult)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IResult();
  explicit IResult(int reuslt_ix);
  virtual ~IResult() override = default;

  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

// ---- Merged implementation from ICval.cpp ----
/*
 * ConstantValue.cpp
 *
 *  Created on: 2017-6-25
 *      Author: Administrator
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ICval)
#endif

namespace hayaku {

ICval::ICval() : IndicatorImp("CVAL", 1) {
  m_need_self_alike_compare = true;
  setParam<double>("value", 0.0);
  setParam<int>("discard", 0);
}

ICval::ICval(double value, size_t discard) : IndicatorImp("CVAL", 1) {
  m_need_self_alike_compare = true;
  setParam<double>("value", value);
  setParam<int>("discard", discard);
}

ICval::~ICval() {}

void ICval::_checkParam(const string& name) const {
  if ("discard" == name) {
    HAYAKU_ASSERT(getParam<int>("discard") >= 0);
  }
}

bool ICval::selfAlike(const IndicatorImp& other) const noexcept {
  HAYAKU_IF_RETURN(isLeaf() && other.isLeaf(), true);
  return m_right && m_right->alike(*other.getRightNode());
}

void ICval::_calculate(const Indicator& data) {
  double value = getParam<double>("value");
  int discard = getParam<int>("discard");

  size_t total = 0;
  if (isLeaf()) {
    // Leaf node
    const KData& k = getContext();
    if (k.getStock().isNull()) {
      _readyBuffer(1, 1);
      if (discard < 1) {
        m_discard = 0;
        _set(value, 0, 0);
      } else {
        m_discard = 1;
      }
      return;
    }

    total = k.size();
    if (0 == total) {
      return;
    }

    _readyBuffer(total, 1);
    auto* dst = this->data(0);
    for (size_t i = 0; i < total; ++i) {
      dst[i] = value;
    }
    return;

  } else {
    // Non-leaf node
    total = data.size();
    discard = data.discard() > discard ? data.discard() : discard;
  }

  m_discard = discard > total ? total : discard;

  size_t ret_num = data.getResultNumber();
  if (ret_num == 0) {
    ret_num = 1;
  }
  _readyBuffer(total, ret_num);

  for (size_t r = 0; r < ret_num; ++r) {
    auto* dst = this->data(r);
    for (size_t i = m_discard; i < total; ++i) {
      dst[i] = value;
    }
  }
}

void ICval::_increment_calculate(const Indicator& data, size_t start_pos) {
  double value = getParam<double>("value");

  size_t total = 0;
  if (isLeaf()) {
    // Leaf node
    const KData& k = getContext();
    total = k.size();
    if (0 == total) {
      return;
    }

  } else {
    // Non-leaf node
    total = data.size();
  }

  for (size_t r = 0; r < m_result_num; ++r) {
    auto* dst = this->data(r);
    for (size_t i = start_pos; i < total; ++i) {
      dst[i] = value;
    }
  }
}

Indicator HAYAKU_API CVAL(double value, size_t discard) {
  return make_shared<ICval>(value, discard)->calculate();
}

Indicator HAYAKU_API CVAL(const Indicator& ind, double value, int discard) {
  auto p = make_shared<ICval>(value, discard);
  if (ind.getContext() == Null<KData>()) {
    // When the passed ind has no context and the ignored data length equals the
    // input data length, it is treated as a leaf node directly because the
    // underlying ind may contain other CVAL, but a CVAL without a context has
    // the size 1; PRICELIST is similar
    return ind.discard() == ind.size() ? Indicator(p) : Indicator(p)(ind);
  }

  p->setContext(ind.getContext());
  return Indicator(p);
}

} /* namespace hayaku */

// ---- Merged implementation from IKData.cpp ----
/*
 * IKData.cpp
 *
 *  Created on: 2013-2-11
 *      Author: fasiondog
 */
#include <boost/algorithm/string.hpp>

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IKData)
#endif

namespace hayaku {

IKData::IKData() : IndicatorImp("KDATA") {
  m_need_context = true;
  setParam<string>("kpart", "KDATA");
}

IKData::~IKData() {}

void IKData::_checkParam(const string& name) const {
  if ("kpart" == name) {
    string part = getParam<string>("kpart");
    HAYAKU_ASSERT("KDATA" == part || "OPEN" == part || "HIGH" == part ||
                  "LOW" == part || "CLOSE" == part || "AMO" == part ||
                  "VOL" == part);
  }
}

// Support a KDATA Indicator as the parameter
void IKData::_calculate(const Indicator& ind) {
  HAYAKU_WARN_IF(!isLeaf() && !ind.empty(),
                 "The input is ignored because {} depends on the context!",
                 getParam<string>("kpart"));

  m_name = getParam<string>("kpart");
  const KData& kdata = getContext();
  size_t total = kdata.size();
  HAYAKU_IF_RETURN(total == 0, void());

  if ("KDATA" == m_name) {
    _readyBuffer(total, 6);
  } else {
    _readyBuffer(total, 1);
  }
  _increment_calculate(ind, 0);
}

void IKData::_increment_calculate(const Indicator& data, size_t start_pos) {
  const KData& kdata = getContext();
  size_t total = kdata.size();
  HAYAKU_IF_RETURN(total == 0, void());

  auto const* ks = kdata.data();
  if ("KDATA" == m_name) {
    auto* dst0 = this->data(0);
    auto* dst1 = this->data(1);
    auto* dst2 = this->data(2);
    auto* dst3 = this->data(3);
    auto* dst4 = this->data(4);
    auto* dst5 = this->data(5);
    for (size_t i = start_pos; i < total; ++i) {
      dst0[i] = ks[i].openPrice;
      dst1[i] = ks[i].highPrice;
      dst2[i] = ks[i].lowPrice;
      dst3[i] = ks[i].closePrice;
      dst4[i] = ks[i].transAmount;
      dst5[i] = ks[i].transCount;
    }
  } else if ("OPEN" == m_name) {
    auto* dst = this->data();
    for (size_t i = start_pos; i < total; ++i) {
      dst[i] = ks[i].openPrice;
    }
  } else if ("HIGH" == m_name) {
    auto* dst = this->data();
    for (size_t i = start_pos; i < total; ++i) {
      dst[i] = ks[i].highPrice;
    }
  } else if ("LOW" == m_name) {
    auto* dst = this->data();
    for (size_t i = start_pos; i < total; ++i) {
      dst[i] = ks[i].lowPrice;
    }

  } else if ("CLOSE" == m_name) {
    auto* dst = this->data();
    for (size_t i = start_pos; i < total; ++i) {
      dst[i] = ks[i].closePrice;
    }
  } else if ("AMO" == m_name) {
    auto* dst = this->data();
    for (size_t i = start_pos; i < total; ++i) {
      dst[i] = ks[i].transAmount;
    }

  } else if ("VOL" == m_name) {
    auto* dst = this->data();
    for (size_t i = start_pos; i < total; ++i) {
      dst[i] = ks[i].transCount;
    }
  }
}

Indicator HAYAKU_API KDATA(const KData& kdata) {
  auto p = make_shared<IKData>();
  p->setParam<string>("kpart", "KDATA");
  p->setContext(kdata);
  return Indicator(p);
}

Indicator HAYAKU_API OPEN(const KData& kdata) {
  auto p = make_shared<IKData>();
  p->setParam<string>("kpart", "OPEN");
  p->setContext(kdata);
  return Indicator(p);
}

Indicator HAYAKU_API HIGH(const KData& kdata) {
  auto p = make_shared<IKData>();
  p->setParam<string>("kpart", "HIGH");
  p->setContext(kdata);
  return Indicator(p);
}

Indicator HAYAKU_API LOW(const KData& kdata) {
  auto p = make_shared<IKData>();
  p->setParam<string>("kpart", "LOW");
  p->setContext(kdata);
  return Indicator(p);
}

Indicator HAYAKU_API CLOSE(const KData& kdata) {
  auto p = make_shared<IKData>();
  p->setParam<string>("kpart", "CLOSE");
  p->setContext(kdata);
  return Indicator(p);
}

Indicator HAYAKU_API AMO(const KData& kdata) {
  auto p = make_shared<IKData>();
  p->setParam<string>("kpart", "AMO");
  p->setContext(kdata);
  return Indicator(p);
}

Indicator HAYAKU_API VOL(const KData& kdata) {
  auto p = make_shared<IKData>();
  p->setParam<string>("kpart", "VOL");
  p->setContext(kdata);
  return Indicator(p);
}

Indicator HAYAKU_API KDATA_PART(const KData& kdata, const string& part) {
  auto p = make_shared<IKData>();
  p->setParam<string>("kpart", part);
  p->setContext(kdata);
  return Indicator(p);
}

//-----------------------------------------------------------
Indicator HAYAKU_API KDATA() {
  IndicatorImpPtr p = make_shared<IKData>();
  p->setParam<string>("kpart", "KDATA");
  p->name("KDATA");
  return p->calculate();
}

Indicator HAYAKU_API OPEN() {
  IndicatorImpPtr p = make_shared<IKData>();
  p->setParam<string>("kpart", "OPEN");
  p->name("OPEN");
  return p->calculate();
}

Indicator HAYAKU_API HIGH() {
  IndicatorImpPtr p = make_shared<IKData>();
  p->setParam<string>("kpart", "HIGH");
  p->name("HIGH");
  return p->calculate();
}

Indicator HAYAKU_API LOW() {
  IndicatorImpPtr p = make_shared<IKData>();
  p->setParam<string>("kpart", "LOW");
  p->name("LOW");
  return p->calculate();
}

Indicator HAYAKU_API CLOSE() {
  IndicatorImpPtr p = make_shared<IKData>();
  p->setParam<string>("kpart", "CLOSE");
  p->name("CLOSE");
  return p->calculate();
}

Indicator HAYAKU_API AMO() {
  IndicatorImpPtr p = make_shared<IKData>();
  p->setParam<string>("kpart", "AMO");
  p->name("AMO");
  return p->calculate();
}

Indicator HAYAKU_API VOL() {
  IndicatorImpPtr p = make_shared<IKData>();
  p->setParam<string>("kpart", "VOL");
  p->name("VOL");
  return p->calculate();
}

Indicator HAYAKU_API KDATA_PART(const string& part) {
  IndicatorImpPtr p = make_shared<IKData>();
  p->setParam<string>("kpart", part);
  p->name("KDATA_PART");
  return p->calculate();
}
} /* namespace hayaku */

// ---- Merged implementation from IPriceList.cpp ----
/*
 * IPriceList.cpp
 *
 *  Created on: 2013-2-12
 *      Author: fasiondog
 */

#include "operators/SeriesOperators.h"
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IPriceList)
#endif

namespace hayaku {

IPriceList::IPriceList() : IndicatorImp("PRICELIST", 1) {
  setParam<PriceList>("data", PriceList());
  setParam<int>("discard", 0);
}

IPriceList::IPriceList(const PriceList& data, int in_discard)
    : IndicatorImp("PRICELIST", 1) {
  setParam<PriceList>("data", data);
  setParam<int>("discard", in_discard);
}

IPriceList::IPriceList(PriceList&& data, int in_discard)
    : IndicatorImp("PRICELIST", 1) {
  setParam<PriceList>("data", std::move(data));
  setParam<int>("discard", in_discard);
}

IPriceList::IPriceList(size_t size, double value, int discard) {
  setParam<PriceList>("data", PriceList(size, value));
  setParam<int>("discard", discard);
}

IPriceList::~IPriceList() {}

void IPriceList::_checkParam(const string& name) const {
  if ("discard" == name) {
    HAYAKU_ASSERT(getParam<int>("discard") >= 0);
  }
}

void IPriceList::_calculate(const Indicator& data) {
  HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                 "The PRICELIST indicator is not leaf node");

  DatetimeList align_dates = haveParam("align_date_list")
                                 ? getParam<DatetimeList>("align_date_list")
                                 : DatetimeList();
  const PriceList& x = getParam<const PriceList&>("data");
  int x_discard = getParam<int>("discard");
  size_t x_total = x.size();

  const auto& k = getContext();
  size_t total = x_total;
  if (k.size() > 0) {
    total = k.size();
  }

  _readyBuffer(total, 1);

  if (k != Null<KData>() && align_dates.size() > 0) {
    // If it is a time series itself, align it by time
    auto tmp = ALIGN(PRICELIST(x, std::move(align_dates), x_discard), k);
    HAYAKU_ASSERT(tmp.size() == total);
    auto* dst = this->data();
    auto* src = x.data();
    for (size_t i = tmp.discard(); i < total; ++i) {
      dst[i] = src[i];
    }
    m_discard = tmp.discard();
    return;
  }

  // If a context is given, align at the right end by the context values,
  // keeping the same length as the context
  if (x_discard >= x_total) {
    m_discard = total;
    return;
  }

  size_t x_start = x_discard;
  auto* dst = this->data();
  if (x_total < total) {
    dst = dst + total + x_discard - x_total;
  } else if (x_total > total) {
    x_start = x_total - total;
    dst = dst - x_start;
    if (x_discard > x_start) {
      x_start = x_discard;
      dst = dst + x_discard - x_start;
    }
  }

  for (size_t i = x_start; i < x_total; ++i) {
    dst[i] = x[i];
  }
  m_discard = total + x_start - x_total;
  return;
}

Indicator HAYAKU_API PRICELIST(const PriceList& data, int discard) {
  return make_shared<IPriceList>(data, discard)->calculate();
}

Indicator HAYAKU_API PRICELIST(PriceList&& data, int discard) {
  return make_shared<IPriceList>(std::move(data), discard)->calculate();
}

Indicator HAYAKU_API PRICELIST(const PriceList& data, const DatetimeList& ds,
                               int discard) {
  HAYAKU_CHECK(data.size() == ds.size(),
               "The data length must be the same as the length of the "
               "reference date list");
  auto ret = PRICELIST(data, discard);
  ret.setParam<DatetimeList>("align_date_list", ds);
  return ret;
}

Indicator HAYAKU_API PRICELIST(PriceList&& data, const DatetimeList&& ds,
                               int discard) {
  HAYAKU_CHECK(data.size() == ds.size(),
               "The data length must be the same as the length of the "
               "reference date list");
  auto ret = PRICELIST(std::move(data), discard);
  ret.setParam<DatetimeList>("align_date_list", std::move(ds));
  return ret;
}

Indicator HAYAKU_API PRICELIST() {
  auto p = make_shared<IPriceList>();
  return Indicator(p);
}

Indicator HAYAKU_API PRICELIST(size_t size, double value, int discard) {
  return make_shared<IPriceList>(size, value, discard)->calculate();
}

Indicator HAYAKU_API PRICELIST(const DatetimeList& dates, double value,
                               int discard) {
  IndicatorImpPtr ptr = make_shared<IPriceList>(dates.size(), value, discard);
  ptr->setParam<DatetimeList>("align_date_list", dates);
  return ptr->calculate();
}

Indicator HAYAKU_API PRICELIST(DatetimeList&& dates, double value,
                               int discard) {
  IndicatorImpPtr ptr = make_shared<IPriceList>(dates.size(), value, discard);
  ptr->setParam<DatetimeList>("align_date_list", std::move(dates));
  return ptr->calculate();
}

} /* namespace hayaku */

// ---- Merged implementation from IContext.cpp ----
/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-28
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IContext)
#endif

namespace hayaku {

IContext::IContext() : IndicatorImp("CONTEXT") {
  m_need_self_alike_compare = true;
  setParam<bool>("fill_null", false);
  setParam<bool>("use_self_ktype",
                 false);  // Use the K-line type of its own context
  setParam<bool>("use_self_recover_type",
                 false);  // Use the adjustment type of its own context
}

IContext::IContext(const Indicator& ref_ind)
    : IndicatorImp("CONTEXT"), m_ref_ind(ref_ind) {
  m_need_self_alike_compare = true;
  setParam<bool>("fill_null", false);
  setParam<bool>("use_self_ktype", false);
  setParam<bool>("use_self_recover_type", false);
}

IContext::~IContext() {}

IndicatorImpPtr IContext::_clone() {
  auto p = make_shared<IContext>();
  p->m_ref_ind = m_ref_ind.clone();
  return p;
}

string IContext::str() const {
  std::ostringstream os;
  os << "Indicator{\n"
     << "  context: " << m_ref_ind.getContext().getStock().market_code()
     << "\n  name: " << name() << "\n  size: " << size()
     << "\n  discard: " << discard() << "\n  result sets: " << getResultNumber()
     << "\n  params: " << getParameter();
  os << "\n  formula: " << formula();
  for (size_t r = 0; r < getResultNumber(); ++r) {
    if (m_pBuffer[r]) {
      os << "\n  values" << r << ": " << *m_pBuffer[r];
    }
  }
  os << "\n}";
  return os.str();
}

string IContext::formula() const {
  return fmt::format("CONTEXT({})", m_ref_ind.formula());
}

KData IContext::getContextKdata() const { return m_ref_ind.getContext(); }

bool IContext::selfAlike(const IndicatorImp& other) const noexcept {
  const auto* other_ctx = dynamic_cast<const IContext*>(&other);
  HAYAKU_IF_RETURN(other_ctx == nullptr, false);
  return m_ref_ind.getImp()->alike(*(other_ctx->m_ref_ind.getImp()));
}

void IContext::_calculate(const Indicator& ind) {
  HAYAKU_ASSERT(isLeaf());

  auto null_k = Null<KData>();
  const auto& in_k = getContext();
  auto self_k = m_ref_ind.getContext();
  HAYAKU_IF_RETURN((self_k == in_k || in_k == null_k) && this->size() != 0,
                   void());

  auto self_dates = m_ref_ind.getDatetimeList();
  // HAYAKU_WARN_IF((self_k == null_k && m_ref_ind.empty() &&
  // self_dates.empty()),
  //             "The data length of context is zero! ");

  auto ref = m_ref_ind;

  if (in_k != null_k && in_k != self_k) {
    if (self_dates.empty() && self_k.getStock().isNull()) {
      // The context is invalid and there is no align date, calculate as a time
      // independent sequence and align
      if (ref.size() > in_k.size()) {
        ref = SLICE(ref, ref.size() - in_k.size(), ref.size());
      } else if (ref.size() < in_k.size()) {
        // Align at the right end
        ref = CVAL(0.)(in_k) + ref;
      }  // else the lengths are equal, no more processing is needed

    } else if (self_k != null_k) {
      // If the reference indicator is a time series, query it with the date
      // query condition of the current context and then align by date
      bool use_self_ktype = getParam<bool>("use_self_ktype");
      bool use_self_recover_type = getParam<bool>("use_self_recover_type");
      auto self_stk = self_k.getStock();
      if (use_self_ktype || use_self_recover_type) {
        const auto& self_query = self_k.getQuery();
        const auto& in_query = in_k.getQuery();
        auto ktype = use_self_ktype ? self_query.kType() : in_query.kType();
        auto recover_type = use_self_recover_type ? self_query.recoverType()
                                                  : in_query.recoverType();
        KQuery query;
        if (in_query.queryType() == KQuery::DATE) {
          query = KQueryByDate(in_query.startDatetime(), in_query.endDatetime(),
                               ktype, recover_type);
        } else {
          query = KQueryByIndex(in_query.start(), in_query.end(), ktype,
                                recover_type);
        }
        // ref = m_ref_ind(self_stk.getKData(query));
        // Make its reference indicator use the incremental calculation
        ref.setContext(self_stk.getKData(query));

      } else {
        // ref = m_ref_ind(self_stk.getKData(in_k.getQuery()));
        // Make its reference indicator use the incremental calculation
        ref.setContext(self_stk.getKData(in_k.getQuery()));
      }
      ref = ALIGN(ref, in_k, getParam<bool>("fill_null"));
    } else if (self_dates.size() > 1) {
      // A time series without a context
      ref = ALIGN(ref, in_k, getParam<bool>("fill_null"));
    }
  }

  size_t total = ref.size();
  size_t rtotal = ref.getResultNumber();
  _readyBuffer(total, rtotal);

  m_discard = ref.discard();
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  size_t len = sizeof(value_t) * (total - m_discard);
  for (size_t r = 0; r < rtotal; ++r) {
    const auto* src = ref.data(r) + m_discard;
    auto* dst = this->data(r) + m_discard;
    memcpy(dst, src, len);
  }
}

Indicator HAYAKU_API CONTEXT(bool fill_null, bool use_self_ktype,
                             bool use_self_recover_type) {
  auto p = make_shared<IContext>();
  p->setParam<bool>("fill_null", fill_null);
  p->setParam<bool>("use_self_ktype", use_self_ktype);
  p->setParam<bool>("use_self_recover_type", use_self_recover_type);
  return Indicator(p);
}

Indicator HAYAKU_API CONTEXT(const Indicator& ind, bool fill_null,
                             bool use_self_ktype, bool use_self_recover_type) {
  auto p = make_shared<IContext>(ind);
  p->setParam<bool>("fill_null", fill_null);
  p->setParam<bool>("use_self_ktype", use_self_ktype);
  p->setParam<bool>("use_self_recover_type", use_self_recover_type);
  return p->calculate();
}

Indicator HAYAKU_API CONTEXT(const Indicator& ind, const Stock& stk,
                             bool fill_null) {
  HAYAKU_WARN_IF(ind.getContext() != Null<KData>(),
                 "The context of input indicator will be ignored!");
  KData kdata = stk.isNull() ? Null<KData>() : stk.getKData(KQuery(0, 0));
  Indicator ref = ind.clone();
  ref.setContext(kdata);
  auto p = make_shared<IContext>(ref);
  p->setParam<bool>("fill_null", fill_null);
  p->setParam<bool>("use_self_ktype", false);
  p->setParam<bool>("use_self_recover_type", false);
  return p->calculate();
}

KData HAYAKU_API CONTEXT_K(const Indicator& ind) {
  auto imp = ind.getImp();
  IContext const* p = dynamic_cast<IContext const*>(imp.get());
  if (p != nullptr) {
    return p->getContextKdata();
  }
  return ind.getContext();
}

bool HAYAKU_API is_standalone_context(const Indicator& ind) {
  auto imp = ind.getImp();
  IContext const* p = dynamic_cast<IContext const*>(imp.get());
  return p != nullptr;
}

}  // namespace hayaku

// ---- Merged implementation from IResult.cpp ----
/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-12
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IResult)
#endif

namespace hayaku {

IResult::IResult() : IndicatorImp("RESULT", 1) {
  setParam<int>("result_ix", 0);
}

IResult::IResult(int result_ix) : IndicatorImp("RESULT", 1) {
  setParam<int>("result_ix", result_ix);
  checkParam("result_ix");
}

void IResult::_checkParam(const string& name) const {
  if ("result_ix" == name) {
    int result_ix = getParam<int>("result_ix");
    HAYAKU_ASSERT(result_ix >= 0 && result_ix < MAX_RESULT_NUM);
  }
}

void IResult::_calculate(const Indicator& ind) {
  int result_ix = getParam<int>("result_ix");
  HAYAKU_IF_RETURN(ind.empty(),
                   void());  // For a formula the ind may not be calculated yet
  HAYAKU_CHECK(result_ix < ind.getResultNumber(),
               "The input indicator has only {} results, but result_ix({}) is "
               "out_of range!",
               ind.getResultNumber(), result_ix);
  m_discard = ind.discard();
  HAYAKU_IF_RETURN(m_discard >= ind.size(), void());

  const auto* src = ind.data(result_ix);
  auto* dst = this->data();
  memcpy(dst + m_discard, src + m_discard,
         sizeof(value_t) * (ind.size() - m_discard));
}

void IResult::_increment_calculate(const Indicator& ind, size_t start_pos) {
  int result_ix = getParam<int>("result_ix");
  const auto* src = ind.data(result_ix);
  auto* dst = this->data();
  memcpy(dst + start_pos, src + start_pos,
         sizeof(value_t) * (ind.size() - start_pos));
}

Indicator HAYAKU_API RESULT(int result_ix) {
  return Indicator(make_shared<IResult>(result_ix));
}

}  // namespace hayaku
