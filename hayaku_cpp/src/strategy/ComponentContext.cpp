/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "ComponentContext.h"

#include "StrategyRuntime.h"

namespace hayaku::internal {

ComponentContext::ComponentContext(StrategyRuntime& runtime) noexcept
    : runtime_(runtime) {}

const KData& ComponentContext::kdata() const noexcept {
  return runtime_.kdata_;
}

const KData& ComponentContext::rawKData() const noexcept {
  return runtime_.raw_k_data_;
}

const Stock& ComponentContext::stock() const noexcept {
  return runtime_.stock_;
}

void ComponentContext::prepare() {
  HAYAKU_CHECK(runtime_.account_, "Strategy has no execution account: {}",
               runtime_.name_);
  HAYAKU_CHECK(runtime_.mm_, "Strategy has no MoneyManager: {}",
               runtime_.name_);
  HAYAKU_CHECK(runtime_.sg_, "Strategy has no Signal: {}", runtime_.name_);

  if (runtime_.ev_) {
    runtime_.pre_environment_valid_ = false;
  }

  if (runtime_.cn_) {
    runtime_.cn_->setAccount(runtime_.account_);
    runtime_.cn_->setSG(runtime_.sg_);
    runtime_.pre_condition_valid_ = false;
  }

  runtime_.mm_->setAccount(runtime_.account_);
  if (runtime_.pg_) {
    runtime_.pg_->setAccount(runtime_.account_);
  }
  if (runtime_.st_) {
    runtime_.st_->setAccount(runtime_.account_);
  }
  if (runtime_.tp_) {
    runtime_.tp_->setAccount(runtime_.account_);
  }
}

void ComponentContext::bind(const KData& kdata) {
  if (runtime_.kdata_ != kdata) {
    runtime_.calculated_ = false;
    runtime_.kdata_ = kdata;
  }

  HAYAKU_TRACE_IF_RETURN(runtime_.calculated_, void(),
                         "No need to calculate!");

  runtime_.stock_ = runtime_.kdata_.getStock();
  KQuery query = runtime_.kdata_.getQuery();
  if (runtime_.stock_.isNull() || query.recoverType() == KQuery::NO_RECOVER) {
    runtime_.raw_k_data_ = runtime_.kdata_;
  } else {
    KQuery noRecoverQuery = query;
    noRecoverQuery.recoverType(KQuery::NO_RECOVER);
    runtime_.raw_k_data_ = runtime_.stock_.getKData(noRecoverQuery);
  }
  HAYAKU_ASSERT(runtime_.kdata_.size() == runtime_.raw_k_data_.size());

  HAYAKU_WARN_IF(query.recoverType() == KQuery::FORWARD ||
                     query.recoverType() == KQuery::EQUAL_FORWARD,
                 htr("You are using forward or equal_forward adjusted K-line "
                     "data, which introduces "
                     "look-ahead bias!"));

  if (runtime_.sg_) {
    runtime_.sg_->setTO(runtime_.kdata_);
  }
  if (runtime_.cn_) {
    runtime_.cn_->setTO(runtime_.kdata_);
  }
  if (runtime_.st_) {
    runtime_.st_->setTO(runtime_.kdata_);
  }
  if (runtime_.tp_) {
    runtime_.tp_->setTO(runtime_.kdata_);
  }
  if (runtime_.pg_) {
    runtime_.pg_->setTO(runtime_.raw_k_data_);
  }
  if (runtime_.sp_) {
    runtime_.sp_->setTO(runtime_.raw_k_data_);
  }
  if (runtime_.ev_) {
    runtime_.ev_->setQuery(query);
  }
  if (runtime_.mm_) {
    runtime_.mm_->setQuery(query);
  }
}

}  // namespace hayaku::internal
