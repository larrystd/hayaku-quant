/*
 * Indicator.cpp
 *
 *  Created on: 2012-10-15
 *      Author: fasiondog
 */

#include "Indicator.h"

#include "SeriesOperators.h"

namespace hayaku {

HAYAKU_API std::ostream& operator<<(std::ostream& os,
                                    const Indicator& indicator) {
  os << indicator.str();
  return os;
}

string Indicator::str() const { return m_imp ? m_imp->str() : "Indicator{}"; }

Indicator::Indicator(const IndicatorImpPtr& imp) noexcept : m_imp(imp) {}

Indicator::Indicator(const Indicator& indicator) noexcept
    : m_imp(indicator.m_imp) {}

Indicator::Indicator(Indicator&& ind) noexcept : m_imp(std::move(ind.m_imp)) {}

Indicator::~Indicator() {}

string Indicator::formula() const {
  return m_imp ? m_imp->formula() : "Indicator";
}

Indicator Indicator::operator()(const KData& k) const {
  Indicator result = clone();
  result.setContext(k);
  return result;
}

void Indicator::setContext(const Stock& stock, const KQuery& query) {
  if (m_imp) m_imp->setContext(stock, query);
}

void Indicator::setContext(const KData& k) {
  if (m_imp) m_imp->setContext(k);
}

void Indicator::extend() {
  if (m_imp) {
    auto k = m_imp->getContext();
    const auto& stk = k.getStock();
    HAYAKU_WARN_IF_RETURN(stk.isNull(), void(), "stock is null!");
    const auto& query = k.getQuery();
    if (query.queryType() == KQuery::INDEX) {
      k = stk.getKData(KQuery(query.start(), Null<int64_t>(), query.kType(),
                              query.recoverType()));
    } else if (query.queryType() == KQuery::DATE) {
      k = stk.getKData(KQueryByDate(query.startDatetime(), Null<Datetime>(),
                                    query.kType(), query.recoverType()));
    } else {
      HAYAKU_WARN("query type ({}) error!",
                  static_cast<int>(query.queryType()));
    }
    m_imp->setContext(k);
  }
}

KData Indicator::getContext() const {
  return m_imp ? m_imp->getContext() : KData();
}

bool Indicator::alike(const Indicator& other) const {
  HAYAKU_IF_RETURN(m_imp == other.m_imp, true);
  return m_imp->alike(*other.m_imp);
}

bool Indicator::equal(const Indicator& other) const noexcept {
  HAYAKU_IF_RETURN(this == &other || m_imp == other.m_imp, true);
  HAYAKU_IF_RETURN(size() != other.size() || discard() != other.discard() ||
                       getResultNumber() != other.getResultNumber(),
                   false);

  for (size_t r = 0, result_num = getResultNumber(); r < result_num; r++) {
    auto const* d1 = this->data(r);
    auto const* d2 = other.data(r);
    for (size_t i = 0, total = size(); i < total; i++) {
      HAYAKU_IF_RETURN((std::isnan(d1[i]) && !std::isnan(d2[i])) ||
                           (!std::isnan(d1[i]) && std::isnan(d2[i])),
                       false);
      HAYAKU_IF_RETURN((!std::isnan(d1[i]) && !std::isnan(d2[i])) &&
                           (std::abs(d1[i] - d2[i]) >= 0.0001),
                       false);
    }
  }
  return true;
}

Indicator& Indicator::operator=(const Indicator& indicator) noexcept {
  HAYAKU_IF_RETURN(this == &indicator, *this);
  m_imp = indicator.m_imp;
  return *this;
}

Indicator& Indicator::operator=(Indicator&& indicator) noexcept {
  HAYAKU_IF_RETURN(this == &indicator, *this);
  m_imp = std::move(indicator.m_imp);
  return *this;
}

PriceList Indicator::getResultAsPriceList(size_t num) const {
  HAYAKU_WARN_IF_RETURN(!m_imp, PriceList(), "indicator imptr is null!");
  return m_imp->getResultAsPriceList(num);
}

Indicator Indicator::getResult(size_t num) const {
  HAYAKU_WARN_IF_RETURN(!m_imp, Indicator(), "indicator imptr is null!");
  return Indicator(m_imp->getResult(num));
}

HAYAKU_API Indicator operator+(const Indicator& ind1, const Indicator& ind2) {
  HAYAKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
  IndicatorImpPtr p = make_shared<IndicatorImp>();
  p->add(IndicatorImp::ADD, ind1.getImp(), ind2.getImp());
  return p->calculate();
}

HAYAKU_API Indicator operator-(const Indicator& ind1, const Indicator& ind2) {
  HAYAKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
  IndicatorImpPtr p = make_shared<IndicatorImp>();
  p->add(IndicatorImp::SUB, ind1.getImp(), ind2.getImp());
  return p->calculate();
}

HAYAKU_API Indicator operator*(const Indicator& ind1, const Indicator& ind2) {
  HAYAKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
  IndicatorImpPtr p = make_shared<IndicatorImp>();
  p->add(IndicatorImp::MUL, ind1.getImp(), ind2.getImp());
  return p->calculate();
}

HAYAKU_API Indicator operator/(const Indicator& ind1, const Indicator& ind2) {
  HAYAKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
  IndicatorImpPtr p = make_shared<IndicatorImp>();
  p->add(IndicatorImp::DIV, ind1.getImp(), ind2.getImp());
  return p->calculate();
}

HAYAKU_API Indicator operator%(const Indicator& ind1, const Indicator& ind2) {
  HAYAKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
  IndicatorImpPtr p = make_shared<IndicatorImp>();
  p->add(IndicatorImp::MOD, ind1.getImp(), ind2.getImp());
  return p->calculate();
}

HAYAKU_API Indicator operator==(const Indicator& ind1, const Indicator& ind2) {
  HAYAKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
  IndicatorImpPtr p = make_shared<IndicatorImp>();
  p->add(IndicatorImp::EQ, ind1.getImp(), ind2.getImp());
  return p->calculate();
}

HAYAKU_API Indicator operator!=(const Indicator& ind1, const Indicator& ind2) {
  HAYAKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
  IndicatorImpPtr p = make_shared<IndicatorImp>();
  p->add(IndicatorImp::NE, ind1.getImp(), ind2.getImp());
  return p->calculate();
}

HAYAKU_API Indicator operator>(const Indicator& ind1, const Indicator& ind2) {
  HAYAKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
  IndicatorImpPtr p = make_shared<IndicatorImp>();
  p->add(IndicatorImp::GT, ind1.getImp(), ind2.getImp());
  return p->calculate();
}

HAYAKU_API Indicator operator<(const Indicator& ind1, const Indicator& ind2) {
  HAYAKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
  IndicatorImpPtr p = make_shared<IndicatorImp>();
  p->add(IndicatorImp::LT, ind1.getImp(), ind2.getImp());
  return p->calculate();
}

HAYAKU_API Indicator operator>=(const Indicator& ind1, const Indicator& ind2) {
  HAYAKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
  IndicatorImpPtr p = make_shared<IndicatorImp>();
  p->add(IndicatorImp::GE, ind1.getImp(), ind2.getImp());
  return p->calculate();
}

HAYAKU_API Indicator operator<=(const Indicator& ind1, const Indicator& ind2) {
  HAYAKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
  IndicatorImpPtr p = make_shared<IndicatorImp>();
  p->add(IndicatorImp::LE, ind1.getImp(), ind2.getImp());
  return p->calculate();
}

HAYAKU_API Indicator operator&(const Indicator& ind1, const Indicator& ind2) {
  HAYAKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
  IndicatorImpPtr p = make_shared<IndicatorImp>();
  p->add(IndicatorImp::AND, ind1.getImp(), ind2.getImp());
  return p->calculate();
}

HAYAKU_API Indicator operator|(const Indicator& ind1, const Indicator& ind2) {
  HAYAKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
  IndicatorImpPtr p = make_shared<IndicatorImp>();
  p->add(IndicatorImp::OR, ind1.getImp(), ind2.getImp());
  return p->calculate();
}

HAYAKU_API Indicator operator+(const Indicator& ind, Indicator::value_t val) {
  return ind + CVAL(ind, val);
}

HAYAKU_API Indicator operator+(Indicator::value_t val, const Indicator& ind) {
  return CVAL(ind, val) + ind;
}

HAYAKU_API Indicator operator-(const Indicator& ind, Indicator::value_t val) {
  return ind - CVAL(ind, val);
}

HAYAKU_API Indicator operator-(Indicator::value_t val, const Indicator& ind) {
  return CVAL(ind, val) - ind;
}

HAYAKU_API Indicator operator-(const Indicator& ind) {
  return CVAL(ind, -1.0) * ind;
}

HAYAKU_API Indicator operator*(const Indicator& ind, Indicator::value_t val) {
  return ind * CVAL(ind, val);
}

HAYAKU_API Indicator operator*(Indicator::value_t val, const Indicator& ind) {
  return CVAL(ind, val) * ind;
}

HAYAKU_API Indicator operator/(const Indicator& ind, Indicator::value_t val) {
  return ind / CVAL(ind, val);
}

HAYAKU_API Indicator operator/(Indicator::value_t val, const Indicator& ind) {
  return CVAL(ind, val) / ind;
}

HAYAKU_API Indicator operator%(const Indicator& ind, Indicator::value_t val) {
  return ind % CVAL(ind, val);
}

HAYAKU_API Indicator operator%(Indicator::value_t val, const Indicator& ind) {
  return CVAL(ind, val) % ind;
}

HAYAKU_API Indicator operator==(const Indicator& ind, Indicator::value_t val) {
  return ind == CVAL(ind, val);
}

HAYAKU_API Indicator operator==(Indicator::value_t val, const Indicator& ind) {
  return CVAL(ind, val) == ind;
}

HAYAKU_API Indicator operator!=(const Indicator& ind, Indicator::value_t val) {
  return ind != CVAL(ind, val);
}

HAYAKU_API Indicator operator!=(Indicator::value_t val, const Indicator& ind) {
  return CVAL(ind, val) != ind;
}

HAYAKU_API Indicator operator>(const Indicator& ind, Indicator::value_t val) {
  return ind > CVAL(ind, val);
}

HAYAKU_API Indicator operator>(Indicator::value_t val, const Indicator& ind) {
  return CVAL(ind, val) > ind;
}

HAYAKU_API Indicator operator<(const Indicator& ind, Indicator::value_t val) {
  return ind < CVAL(ind, val);
}

HAYAKU_API Indicator operator<(Indicator::value_t val, const Indicator& ind) {
  return CVAL(ind, val) < ind;
}

HAYAKU_API Indicator operator>=(const Indicator& ind, Indicator::value_t val) {
  return ind >= CVAL(ind, val);
}

HAYAKU_API Indicator operator>=(Indicator::value_t val, const Indicator& ind) {
  return CVAL(ind, val) >= ind;
}

HAYAKU_API Indicator operator<=(const Indicator& ind, Indicator::value_t val) {
  return ind <= CVAL(ind, val);
}

HAYAKU_API Indicator operator<=(Indicator::value_t val, const Indicator& ind) {
  return CVAL(ind, val) <= ind;
}

HAYAKU_API Indicator operator&(const Indicator& ind, Indicator::value_t val) {
  return ind & CVAL(ind, val);
}

HAYAKU_API Indicator operator&(Indicator::value_t val, const Indicator& ind) {
  return CVAL(ind, val) & ind;
}

HAYAKU_API Indicator operator|(const Indicator& ind, Indicator::value_t val) {
  return ind | CVAL(ind, val);
}

HAYAKU_API Indicator operator|(Indicator::value_t val, const Indicator& ind) {
  return CVAL(ind, val) | ind;
}

Indicator HAYAKU_API WEAVE(const Indicator& ind1, const Indicator& ind2) {
  HAYAKU_ERROR_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator(),
                         "ind1 or ind2 is Null Indicator!");
  IndicatorImpPtr p = make_shared<IndicatorImp>("WEAVE");
  p->add(IndicatorImp::WEAVE, ind1.getImp(), ind2.getImp());
  return p->calculate();
}

Indicator HAYAKU_API IF(const Indicator& ind1, const Indicator& ind2,
                        const Indicator& ind3) {
  HAYAKU_ERROR_IF_RETURN(!ind1.getImp() || !ind2.getImp() || !ind3.getImp(),
                         Indicator(), "Exists null indicator!");
  IndicatorImpPtr p = make_shared<IndicatorImp>();
  p->add_if(ind1.getImp(), ind2.getImp(), ind3.getImp());
  return p->calculate();
}

Indicator HAYAKU_API IF(const Indicator& x, Indicator::value_t a,
                        const Indicator& b) {
  return IF(x, CVAL(b, a), b);
}

Indicator HAYAKU_API IF(const Indicator& x, const Indicator& a,
                        Indicator::value_t b) {
  return IF(x, a, CVAL(a, b));
}

Indicator HAYAKU_API IF(const Indicator& x, Indicator::value_t a,
                        Indicator::value_t b) {
  return IF(x, CVAL(x, a), CVAL(x, b));
}

IndicatorList HAYAKU_API combineCalculateIndicators(
    const IndicatorList& indicators, const KData& kdata, bool tovalue) {
  IndicatorList ret;
  ret.reserve(indicators.size());
  for (const auto& ind : indicators) {
    ret.push_back(ind.clone());
  }

  vector<IndicatorImpPtr> sub_nodes;
  for (const auto& ind : ret) {
    vector<IndicatorImpPtr> nodes;
    ind.getImp()->getAllSubNodes(nodes);
    sub_nodes.insert(sub_nodes.end(), nodes.begin(), nodes.end());
  }

  IndicatorImp::inner_repeatALikeNodes(sub_nodes);
  for (const auto& ind : ret) {
    ind.getImp()->repeatSeparateKTypeLeafALikeNodes();
  }

  if (tovalue) {
    for (auto& ind : ret) {
      ind.setContext(kdata);
      ind = ind.getResult(0);
    }
  } else {
    for (auto& ind : ret) {
      ind.setContext(kdata);
    }
  }

  return ret;
}

} /* namespace hayaku */
