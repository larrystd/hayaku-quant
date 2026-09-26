/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-05-20
 *     Author: fasiondog
 */

#include "DBCondition.h"

namespace hayaku {

DBCondition& DBCondition::operator&(const DBCondition& other) {
  if (this == &other) {
    return *this;
  }

  if (condition_.empty()) {
    condition_ = other.condition_;
  } else {
    condition_ = fmt::format("({} and {})", condition_, other.condition_);
  }

  return *this;
}

DBCondition& DBCondition::operator|(const DBCondition& other) {
  if (this == &other) {
    return *this;
  }

  if (condition_.empty()) {
    condition_ = other.condition_;
  } else {
    condition_ = fmt::format("({} or {})", condition_, other.condition_);
  }

  return *this;
}

}  // namespace hayaku
