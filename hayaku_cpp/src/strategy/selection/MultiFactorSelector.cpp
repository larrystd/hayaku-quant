/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#include "MultiFactorSelector.h"

#include "strategy/selection/MultiFactors.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::MultiFactorSelector)
#endif

namespace hayaku {

MultiFactorSelector::MultiFactorSelector() : SelectorBase("SE_MultiFactor") {
  setParam<bool>("only_should_buy",
                 false);  // Select the systems that also issue a buy signal
  setParam<bool>("ignore_null",
                 true);  // Ignore the securities whose score is nan in the MF
  setParam<bool>("ignore_le_zero",
                 false);  // Ignore the securities whose score is <= 0 in the MF
  setParam<int>("topn", 10);
  setParam<bool>(
      "reverse",
      false);  // Reverse order, topn means the last few (lowest values)
  setParam<int>("ic_n", 5);
  setParam<int>("ic_rolling_n", 120);
  setParam<Stock>("ref_stk", Stock());
  setParam<bool>("use_spearman", true);
  setParam<string>("mode", "MF_ICIRWeight");
  setParam<int>(
      "mf_recover_type",
      KQuery::INVALID_RECOVER_TYPE);  // The MF calculation adjustment type
}

MultiFactorSelector::MultiFactorSelector(const MFPtr& mf, int topn)
    : SelectorBase("SE_MultiFactor") {
  HAYAKU_CHECK(mf, "mf is null!");
  mf_ = mf;
  setParam<bool>("only_should_buy", false);
  setParam<bool>("ignore_null", true);
  setParam<bool>("ignore_le_zero", false);
  setParam<int>("topn", topn);
  setParam<bool>("reverse", false);

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
  setFactorSet(mf->getRefFactorSet());
}

MultiFactorSelector::~MultiFactorSelector() {}

void MultiFactorSelector::_checkParam(const string& name) const {
  if ("ic_n" == name) {
    HAYAKU_ASSERT(getParam<int>("ic_n") >= 1);
  } else if ("ic_rolling_n" == name) {
    HAYAKU_ASSERT(getParam<int>("ic_rolling_n") >= 1);
  } else if ("mode" == name) {
    auto mode = getParam<string>("mode");
    HAYAKU_ASSERT("MF_ICIRWeight" == mode || "MF_ICWeight" == mode ||
                  "MF_EqualWeight" == mode || "CUSTOM" == mode);
  }
}

void MultiFactorSelector::_reset() {
  if (mf_) {
    mf_->reset();
  }
  stk_sys_dict_.clear();
}

SelectorPtr MultiFactorSelector::_clone() {
  auto p = make_shared<MultiFactorSelector>();
  p->mf_ = mf_->clone();
  p->stk_sys_dict_ = stk_sys_dict_;
  p->factorset_ = factorset_;
  return p;
}

bool MultiFactorSelector::isMatchAF(const AFPtr& af) { return true; }

ScoreRecordList MultiFactorSelector::filterOnlyShouldBuy(
    Datetime date, const ScoreRecordList& scores, size_t topn) {
  ScoreRecordList ret;
  size_t i = 0, cnt = 0, total = scores.size();
  while (i < total && cnt < topn) {
    auto const& sc = scores[i];
    auto sys = stk_sys_dict_[sc.stock];
    if (sys->getSG()->shouldBuy(date)) {
      ret.emplace_back(sc);
      cnt++;
    }
    i++;
  }
  return ret;
}

ScoreRecordList MultiFactorSelector::filterTopNReverse(
    Datetime date, const ScoreRecordList& raw_scores, size_t topn,
    bool only_should_buy, bool ignore_null) {
  ScoreRecordList scores;
  auto iter = raw_scores.rbegin();
  for (size_t count = 0; count < topn && iter != raw_scores.rend(); ++iter) {
    if (!std::isnan(iter->value)) {
      scores.emplace_back(*iter);
      count++;
    }
  }
  if (scores.size() < topn && !ignore_null) {
    size_t lack = topn - scores.size();
    iter = raw_scores.rbegin();
    for (size_t count = 0; count < lack && iter != raw_scores.rend(); ++iter) {
      if (std::isnan(iter->value)) {
        scores.emplace_back(*iter);
        count++;
      } else {
        break;
      }
    }
  }
  if (only_should_buy) {
    scores = filterOnlyShouldBuy(date, scores, topn);
  }

  return scores;
}

ScoreRecordList MultiFactorSelector::filterTopN(
    Datetime date, const ScoreRecordList& raw_scores, size_t topn,
    bool only_should_buy) {
  ScoreRecordList scores;
  if (only_should_buy) {
    scores = filterOnlyShouldBuy(date, raw_scores, topn);
  } else {
    scores.assign(raw_scores.begin(),
                  raw_scores.begin() + std::min(topn, raw_scores.size()));
  }
  return scores;
}

StrategyWeightList MultiFactorSelector::_getSelected(Datetime date) {
  bool ignore_null = getParam<bool>("ignore_null");
  bool ignore_le_zero = getParam<bool>("ignore_le_zero");
  bool only_should_buy = getParam<bool>("only_should_buy");
  bool reverse = getParam<bool>("reverse");

  ScoreRecordList scores =
      mf_->getScores(date, 0, Null<size_t>(),
                     [ignore_null, ignore_le_zero](const ScoreRecord& sc) {
                       return !(ignore_null && std::isnan(sc.value)) &&
                              !(ignore_le_zero && sc.value <= 0.0);
                     });

  // MultiFactorSelector2 with filters is recommended; this is kept for the old
  // interface compatibility only Apply the user-defined score filters
  // (set_scores_filter / add_scores_filter are supported)
  if (sc_filter_) {
    scores = sc_filter_->filter(scores, date, query_);
  }

  int param_topn = getParam<int>("topn");
  size_t topn =
      param_topn > 0 ? static_cast<size_t>(param_topn) : scores.size();
  if (topn > scores.size()) {
    topn = scores.size();
  }

  if (!reverse) {
    // Sort in the ascending order
    scores = filterTopN(date, scores, topn, only_should_buy);

  } else {
    // Sort in the descending order
    scores =
        filterTopNReverse(date, scores, topn, only_should_buy, ignore_null);
  }

  StrategyWeightList ret;
  for (const auto& sc : scores) {
    ret.emplace_back(stk_sys_dict_[sc.stock], sc.value);
  }
  return ret;
}

void MultiFactorSelector::_calculate() {
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
      mf_ = MF_ICIRWeight(factorset_, stks, query, ref_stk, ic_n, ic_rolling_n,
                          spearman);
    } else if ("MF_ICWeight" == mode) {
      mf_ = MF_ICWeight(factorset_, stks, query, ref_stk, ic_n, ic_rolling_n,
                        spearman);
    } else if ("MF_EqualWeight" == mode) {
      mf_ = MF_EqualWeight(factorset_, stks, query, ref_stk, ic_n, spearman);
    } else {
      HAYAKU_THROW("Invalid mode: {}", mode);
    }
  } else {
    if (getParam<bool>("keep_mf_recover_type")) {
      auto mf_query = mf_->getQuery();
      auto new_query = query;
      new_query.recoverType(mf_query.recoverType());
      mf_->setQuery(new_query);
    } else {
      mf_->setQuery(query);
    }
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

SelectorPtr SE_MultiFactor(const MFPtr& mf, int topn) {
  return make_shared<MultiFactorSelector>(mf, topn);
}

SelectorPtr SE_MultiFactor(const FactorSet& factorset, int topn, int ic_n,
                           int ic_rolling_n, const Stock& ref_stk,
                           bool spearman, const string& mode) {
  auto p = make_shared<MultiFactorSelector>();
  p->setFactorSet(factorset);
  p->setParam<int>("topn", topn);
  p->setParam<int>("ic_n", ic_n);
  p->setParam<int>("ic_rolling_n", ic_rolling_n);
  p->setParam<Stock>("ref_stk", ref_stk);
  p->setParam<bool>("use_spearman", spearman);
  p->setParam<string>("mode", mode);
  return p;
}

}  // namespace hayaku
