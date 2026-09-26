/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "ComponentContext.h"

#include "StrategyRuntime.h"

namespace hayaku::internal {

ComponentContext::ComponentContext(StrategyRuntime& runtime) noexcept
    : m_runtime(runtime) {}

const KData& ComponentContext::kdata() const noexcept {
  return m_runtime.m_kdata;
}

const KData& ComponentContext::rawKData() const noexcept {
  return m_runtime.m_rawKData;
}

const Stock& ComponentContext::stock() const noexcept {
  return m_runtime.m_stock;
}

void ComponentContext::prepare() {
  HAYAKU_CHECK(m_runtime.m_account, "Strategy has no execution account: {}",
               m_runtime.m_name);
  HAYAKU_CHECK(m_runtime.m_mm, "Strategy has no MoneyManager: {}",
               m_runtime.m_name);
  HAYAKU_CHECK(m_runtime.m_sg, "Strategy has no Signal: {}", m_runtime.m_name);

  if (m_runtime.m_ev) {
    m_runtime.m_preEnvironmentValid = false;
  }

  if (m_runtime.m_cn) {
    m_runtime.m_cn->setAccount(m_runtime.m_account);
    m_runtime.m_cn->setSG(m_runtime.m_sg);
    m_runtime.m_preConditionValid = false;
  }

  m_runtime.m_mm->setAccount(m_runtime.m_account);
  if (m_runtime.m_pg) {
    m_runtime.m_pg->setAccount(m_runtime.m_account);
  }
  if (m_runtime.m_st) {
    m_runtime.m_st->setAccount(m_runtime.m_account);
  }
  if (m_runtime.m_tp) {
    m_runtime.m_tp->setAccount(m_runtime.m_account);
  }
}

void ComponentContext::bind(const KData& kdata) {
  if (m_runtime.m_kdata != kdata) {
    m_runtime.m_calculated = false;
    m_runtime.m_kdata = kdata;
  }

  HAYAKU_TRACE_IF_RETURN(m_runtime.m_calculated, void(),
                         "No need to calculate!");

  m_runtime.m_stock = m_runtime.m_kdata.getStock();
  KQuery query = m_runtime.m_kdata.getQuery();
  if (m_runtime.m_stock.isNull() || query.recoverType() == KQuery::NO_RECOVER) {
    m_runtime.m_rawKData = m_runtime.m_kdata;
  } else {
    KQuery noRecoverQuery = query;
    noRecoverQuery.recoverType(KQuery::NO_RECOVER);
    m_runtime.m_rawKData = m_runtime.m_stock.getKData(noRecoverQuery);
  }
  HAYAKU_ASSERT(m_runtime.m_kdata.size() == m_runtime.m_rawKData.size());

  HAYAKU_WARN_IF(query.recoverType() == KQuery::FORWARD ||
                     query.recoverType() == KQuery::EQUAL_FORWARD,
                 htr("You are using forward or equal_forward adjusted K-line "
                     "data, which introduces "
                     "look-ahead bias!"));

  if (m_runtime.m_sg) {
    m_runtime.m_sg->setTO(m_runtime.m_kdata);
  }
  if (m_runtime.m_cn) {
    m_runtime.m_cn->setTO(m_runtime.m_kdata);
  }
  if (m_runtime.m_st) {
    m_runtime.m_st->setTO(m_runtime.m_kdata);
  }
  if (m_runtime.m_tp) {
    m_runtime.m_tp->setTO(m_runtime.m_kdata);
  }
  if (m_runtime.m_pg) {
    m_runtime.m_pg->setTO(m_runtime.m_rawKData);
  }
  if (m_runtime.m_sp) {
    m_runtime.m_sp->setTO(m_runtime.m_rawKData);
  }
  if (m_runtime.m_ev) {
    m_runtime.m_ev->setQuery(query);
  }
  if (m_runtime.m_mm) {
    m_runtime.m_mm->setQuery(query);
  }
}

}  // namespace hayaku::internal
