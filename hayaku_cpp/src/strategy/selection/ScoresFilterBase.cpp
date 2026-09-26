/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-13
 *      Author: fasiondog
 */

#include "ScoresFilterBase.h"

namespace hayaku {

HAYAKU_API std::ostream& operator<<(std::ostream& out,
                                    const ScoresFilterBase& scfilter) {
  out << "SCFilter{";
  out << scfilter.name() << "(params:" << scfilter.getParameter() << ")";
  auto child = scfilter.child_;
  while (child) {
    out << " -> " << child->name() << "(params:" << child->getParameter()
        << ")";
    child = child->child_;
  }
  out << "}";
  return out;
}

HAYAKU_API std::ostream& operator<<(std::ostream& out,
                                    const ScoresFilterPtr& scfilter) {
  if (scfilter) {
    out << *scfilter;
  } else {
    out << "SCFilter(NULL)";
  }
  return out;
}

void ScoresFilterBase::baseCheckParam(const string& name) const {}

void ScoresFilterBase::paramChanged() {}

ScoresFilterPtr ScoresFilterBase::clone() {
  auto p = _clone();
  p->params_ = params_;
  p->name_ = name_;
  p->is_python_object_ = is_python_object_;

  if (child_) {
    p->child_ = child_->clone();
  }
  return p;
}

ScoreRecordList ScoresFilterBase::filter(const ScoreRecordList& scores,
                                         const Datetime& date,
                                         const KQuery& query) {
  auto ret = _filter(scores, date, query);
  if (child_) {
    ret = child_->filter(ret, date, query);
  }
  return ret;
}

HAYAKU_API ScoresFilterPtr operator|(const ScoresFilterPtr& a,
                                     const ScoresFilterPtr& b) {
  ScoresFilterPtr ret;
  if (a && b) {
    auto node = a;
    while (node->child_) {
      node = node->child_;
    }
    node->child_ = b;
    ret = a;
  } else if (a) {
    HAYAKU_WARN("filter b is null, will be ignored.");
    ret = a;
  } else if (b) {
    HAYAKU_WARN("filter a is null, will be ignored.");
    ret = b;
  } else {
    HAYAKU_WARN("filter a and b are all null, will be returned null.");
  }
  return ret;
}

}  // namespace hayaku
