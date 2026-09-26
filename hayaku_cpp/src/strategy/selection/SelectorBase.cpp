/*
 * SelectorBase.cpp
 *
 *  Created on: 2016-2-21
 *      Author: fasiondog
 */

#include "SelectorBase.h"

#include "strategy/StrategyRuntime.h"

namespace hayaku {

std::ostream& operator<<(std::ostream& os, const SelectorBase& st) {
  os << st.str();
  return os;
}

std::ostream& operator<<(std::ostream& os, const SelectorPtr& st) {
  if (st) {
    os << st->str();
  } else {
    os << "Selector(NULL)";
  }

  return os;
}

string SelectorBase::str() const {
  std::ostringstream buf;
  buf << "Selector(" << name() << ", " << getParameter() << ", " << sc_filter_
      << ")";
  return buf.str();
}

SelectorBase::SelectorBase() : name_("SelectorBase") { initParam(); }

SelectorBase::SelectorBase(const string& name) : name_(name) { initParam(); }

SelectorBase::~SelectorBase() {}

void SelectorBase::initParam() {
  // Usually the prototype system does not participate in the calculation, but
  // in some special scenarios it needs to rely on the companion system
  // strategy; in that case the behavior of the actually executed system can be
  // considered to follow the buys and sells of the companion system, such as a
  // selection relying on the SG (however relying on the SG only is not
  // rigorous, because the SG of the prototype and of the actual system are the
  // same). In this case the prototype
  setParam<bool>("depend_on_proto_sys",
                 false);      // The prototype system must be able to run alone
  setParam<int>("get_n", 0);  // How many leading items getSelected returns
}

void SelectorBase::baseCheckParam(const string& name) const {}

void SelectorBase::paramChanged() {
  calculated_ = false;
  proto_calculated_ = false;
}

void SelectorBase::removeAll() {
  pro_sys_list_.clear();
  _removeAll();
  reset();
}

void SelectorBase::reset() {
  internal::StrategyRuntimeList::const_iterator iter = pro_sys_list_.begin();
  for (; iter != pro_sys_list_.end(); ++iter) {
    (*iter)->reset();
  }

  real_sys_list_.clear();
  _reset();

  calculated_ = false;
  proto_calculated_ = false;
}

SelectorPtr SelectorBase::clone() {
  SelectorPtr p = _clone();
  p->params_ = params_;
  p->name_ = name_;
  p->is_python_object_ = is_python_object_;
  p->query_ = query_;
  p->proto_query_ = proto_query_;
  p->calculated_ = calculated_;
  p->proto_calculated_ = proto_calculated_;

  p->real_sys_list_.reserve(real_sys_list_.size());
  for (const auto& sys : real_sys_list_) {
    p->real_sys_list_.emplace_back(sys->clone());
  }

  p->pro_sys_list_.reserve(pro_sys_list_.size());
  for (const auto& sys : pro_sys_list_) {
    p->pro_sys_list_.emplace_back(sys->clone());
  }

  if (sc_filter_) {
    p->sc_filter_ = sc_filter_->clone();
  }

  p->pf_ = pf_;  // A reference to PF only, not cloned

  return p;
}

void SelectorBase::calculate(
    const internal::StrategyRuntimeList& pf_realSysList, const KQuery& query) {
  HAYAKU_IF_RETURN(calculated_ && query_ == query, void());

  query_ = query;
  real_sys_list_ = pf_realSysList;

  // It depends on the running system and must be calculated before its own
  // calculation
  if (getParam<bool>("depend_on_proto_sys")) {
    calculate_proto(query);
  }

  _calculate();
  calculated_ = true;
}

void SelectorBase::calculate_proto(const KQuery& query) {
  if (proto_query_ != query && !proto_calculated_) {
    HAYAKU_WARN_IF_RETURN(pro_sys_list_.empty(), void(),
                          "m_pro_sys_list is empty!");
    for (auto& sys : pro_sys_list_) {
      sys->run(query);
    }
    proto_calculated_ = true;
    proto_query_ = query;
  }
}

void SelectorBase::addSystem(const internal::StrategyRuntimePtr& sys) {
  HAYAKU_CHECK(sys, "The input sys is null!");
  HAYAKU_CHECK(sys->getMM(), "protoSys missing MoneyManager!");
  HAYAKU_CHECK(sys->getSG(), "protoSys missing Siganl!");
  HAYAKU_CHECK(!sys->getParam<bool>("shared_account"),
               "A selector prototype cannot share an execution account");
  if (getParam<bool>("depend_on_proto_sys")) {
    HAYAKU_CHECK(
        sys->getAccount(),
        "Scenarios that depend on prototype systems need to specify a TM!");
  }

  sys->reset();
  _addSystem(sys);

  pro_sys_list_.emplace_back(sys);
  calculated_ = false;
  proto_calculated_ = false;
}

void SelectorBase::addSystemList(const internal::StrategyRuntimeList& sysList) {
  for (const auto& sys : sysList) {
    addSystem(sys);
  }
}

void SelectorBase::addStock(const Stock& stock,
                            const internal::StrategyRuntimePtr& protoSys) {
  HAYAKU_CHECK(!stock.isNull(), "The input stock is null!");
  HAYAKU_CHECK(protoSys, "The input protoSys is null!");
  HAYAKU_CHECK(protoSys->getMM(), "protoSys missing MoneyManager!");
  HAYAKU_CHECK(protoSys->getSG(), "protoSys missing Siganl!");
  HAYAKU_CHECK(!protoSys->getParam<bool>("shared_account"),
               "A selector prototype cannot share an execution account");
  if (getParam<bool>("depend_on_proto_sys")) {
    HAYAKU_CHECK(
        protoSys->getAccount(),
        "Scenarios that depend on prototype systems need to specify a TM!");
  }

  auto proto = protoSys;
  proto->forceResetAll();
  internal::StrategyRuntimePtr sys = proto->clone();
  sys->reset();
  sys->setStock(stock);
  _addSystem(sys);
  pro_sys_list_.emplace_back(sys);

  calculated_ = false;
  proto_calculated_ = false;
}

void SelectorBase::addStockList(const StockList& stkList,
                                const internal::StrategyRuntimePtr& protoSys) {
  for (const auto& stk : stkList) {
    addStock(stk, protoSys);
  }
}

StrategyWeightList SelectorBase::getSelected(Datetime date) {
  if (getParam<int>("get_n") <= 0) {
    return _getSelected(date);
  }

  StrategyWeightList ret = _getSelected(date);
  if (ret.size() > getParam<int>("get_n")) {
    ret.resize(getParam<int>("get_n"));
  }
  return ret;
}

} /* namespace hayaku */
