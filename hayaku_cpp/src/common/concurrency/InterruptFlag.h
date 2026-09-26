#pragma once

/**
 *  Copyright (c) 2021 hikyuu.org
 *
 *  Created on: 2021/07/13
 *      Author: fasiondog
 */

#include <atomic>

namespace hayaku {

class InterruptFlag {
 public:
  InterruptFlag() : flag_(false) {}

  explicit InterruptFlag(bool initial) : flag_(initial) {}

  InterruptFlag(const InterruptFlag& other)
      : flag_(other.flag_.load(std::memory_order_relaxed)) {}

  // Assignment operator
  InterruptFlag& operator=(const InterruptFlag& other) {
    flag_.store(other.flag_.load(std::memory_order_relaxed),
                 std::memory_order_relaxed);
    return *this;
  }

  // Convert to the bool type
  operator bool() const { return flag_.load(std::memory_order_relaxed); }

  void set() { flag_ = true; }

  bool isSet() const { return flag_; }

 private:
  std::atomic_bool flag_;
};

}  // namespace hayaku
