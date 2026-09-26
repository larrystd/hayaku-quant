/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-xx-xx
 *      Author: fasiondog
 */

#include "MultiFactorSelector2.h"

#include "strategy/selection/MultiFactors.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::MultiFactorSelector2)
#endif

namespace hayaku {

MultiFactorSelector2::MultiFactorSelector2() : SelectorBase("SE_MultiFactor2") {
  setParam<int>("ic_n", 5);
  setParam<int>("ic_rolling_n", 120);
  setParam<Stock>("ref_stk", Stock());
  setParam<bool>("use_spearman", true);
  setParam<string>("mode", "MF_ICIRWeight");
  setParam<int>(
      "mf_recover_type",
      KQuery::INVALID_RECOVER_TYPE);  // The MF calculation adjustment type
}

MultiFactorSelector2::MultiFactorSelector2(const MFPtr& mf)
    : SelectorBase("SE_MultiFactor2") {
  HAYAKU_CHECK(mf, "mf is null!");
  mf_ = mf;
  setParam<int>("ic_n", mf->getParam<int>("ic_n"));
  setParam<Stock>("ref_stk", mf->getRefStock());
  if (mf->haveParam("ic_rolling_n")) {
    setParam<int>("ic_rolling_n", mf->getParam<int>("ic_rolling_n"));
  } else {
    setParam<int>("ic_rolling_n", 120);
  }
  setParam<bool>("use_spearman", mf->getParam<bool>("use_spearman"));
  setParam<string>("mode", "CUSTOM");
  setParam<int>(
      "mf_recover_type",
      KQuery::INVALID_RECOVER_TYPE);  // The MF calculation adjustment type
  // setIndicators(mf->getRefIndicators());
  setFactorSet(mf->getRefFactorSet());
}

MultiFactorSelector2::~MultiFactorSelector2() {}

void MultiFactorSelector2::_checkParam(const string& name) const {
  if ("ic_n" == name) {
    HAYAKU_ASSERT(getParam<int>("ic_n") >= 1);
  } else if ("ic_rolling_n" == name) {
    HAYAKU_ASSERT(getParam<int>("ic_rolling_n") >= 1);
  } else if ("mode" == name) {
    auto mode = getParam<string>("mode");
    HAYAKU_ASSERT("MF_ICIRWeight" == mode || "MF_ICWeight" == mode ||
                  "MF_EqualWeight" == mode || "CUSTOM" == mode);
  } else if ("mf_recover_type" == name) {
    int recover_type = getParam<int>("mf_recover_type");
    HAYAKU_ASSERT(recover_type >= KQuery::NO_RECOVER &&
                  recover_type <= KQuery::INVALID_RECOVER_TYPE);
  }
}

void MultiFactorSelector2::_reset() {
  if (mf_) {
    mf_->reset();
  }
  stk_sys_dict_.clear();
}

SelectorPtr MultiFactorSelector2::_clone() {
  auto p = make_shared<MultiFactorSelector2>();
  p->mf_ = mf_->clone();
  p->stk_sys_dict_ = stk_sys_dict_;
  p->factorset_ = factorset_;
  return p;
}

bool MultiFactorSelector2::isMatchAF(const AFPtr& af) { return true; }

StrategyWeightList MultiFactorSelector2::_getSelected(Datetime date) {
  ScoreRecordList scores =
      mf_->getScores(date, 0, Null<size_t>(), sc_filter_);
  StrategyWeightList ret;
  for (const auto& sc : scores) {
    ret.emplace_back(stk_sys_dict_[sc.stock], sc.value);
  }
  return ret;
}

void MultiFactorSelector2::_calculate() {
  Stock ref_stk = getParam<Stock>("ref_stk");
  StockList stks;
  for (const auto& sys : pro_sys_list_) {
    stks.emplace_back(sys->getStock());
  }

  KQuery query = query_;
  if (getParam<int>("mf_recover_type") != KQuery::INVALID_RECOVER_TYPE) {
    query.recoverType(
        static_cast<KQuery::RecoverType>(getParam<int>("mf_recover_type")));
  }

  auto ic_n = getParam<int>("ic_n");
  auto ic_rolling_n = getParam<int>("ic_rolling_n");
  bool spearman = getParam<bool>("use_spearman");
  auto mode = getParam<string>("mode");

  if (!mf_) {
    if ("MF_ICIRWeight" == mode) {
      mf_ = MF_ICIRWeight(factorset_, stks, query, ref_stk, ic_n,
                           ic_rolling_n, spearman);
    } else if ("MF_ICWeight" == mode) {
      mf_ = MF_ICWeight(factorset_, stks, query, ref_stk, ic_n, ic_rolling_n,
                         spearman);
    } else if ("MF_EqualWeight" == mode) {
      mf_ = MF_EqualWeight(factorset_, stks, query, ref_stk, ic_n, spearman);
    } else {
      HAYAKU_THROW("Invalid mode: {}", mode);
    }
  } else {
    mf_->setQuery(query);
    mf_->setRefFactorSet(factorset_);
    mf_->setRefStock(ref_stk);
    mf_->setStockList(stks);
    mf_->setParam<int>("ic_n", ic_n);
    mf_->setParam<bool>("use_spearman", spearman);
    if (mf_->haveParam("ic_rolling_n")) {
      mf_->setParam<int>("ic_rolling_n", ic_rolling_n);
    }
  }

  mf_->calculate();

  for (const auto& sys : real_sys_list_) {
    stk_sys_dict_.insert({sys->getStock(), sys});
  }
}

SelectorPtr HAYAKU_API SE_MultiFactor2(const MFPtr& mf,
                                       const ScoresFilterPtr& filter) {
  auto p = make_shared<MultiFactorSelector2>(mf);
  p->setScoresFilter(filter);
  return p;
}

SelectorPtr HAYAKU_API SE_MultiFactor2(const FactorSet& factorset, int ic_n,
                                       int ic_rolling_n, const Stock& ref_stk,
                                       bool spearman, const string& mode,
                                       const ScoresFilterPtr& filter) {
  auto p = make_shared<MultiFactorSelector2>();
  p->setFactorSet(factorset);
  p->setParam<int>("ic_n", ic_n);
  p->setParam<int>("ic_rolling_n", ic_rolling_n);
  p->setParam<Stock>("ref_stk", ref_stk);
  p->setParam<bool>("use_spearman", spearman);
  p->setParam<string>("mode", mode);
  p->setScoresFilter(filter);
  return p;
}

}  // namespace hayaku
