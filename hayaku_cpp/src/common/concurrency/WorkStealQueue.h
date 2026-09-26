#pragma once

/*
 * WorkStealQueue.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-9-16
 *      Author: fasiondog
 */

#include <deque>
#include <shared_mutex>

#include "FuncWrapper.h"

namespace hayaku {

/**
 * Task stealing queue
 */
class WorkStealQueue {
 private:
  typedef FuncWrapper data_type;
  std::deque<data_type> queue_;
  mutable std::shared_mutex mutex_;

 public:
  /** Constructor */
  WorkStealQueue() {}

  // The copy constructor and the assignment overload are disabled
  WorkStealQueue(const WorkStealQueue& other) = delete;
  WorkStealQueue& operator=(const WorkStealQueue& other) = delete;

  /** Insert the data into the head of the queue */
  void push_front(data_type&& data) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    queue_.push_front(std::move(data));
  }

  /** Insert the data into the tail of the queue */
  void push_back(data_type&& data) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    queue_.push_back(std::move(data));
  }

  /** Whether the queue is empty */
  bool empty() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return queue_.empty();
  }

  /** Queue size */
  size_t size() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return queue_.size();
  }

  void clear() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto tmp = std::deque<data_type>();
    queue_.swap(tmp);
  }

  /**
   * Try to pop a piece of data from the head of the queue
   * @param res stores the popped data
   * @return false is returned if the queue was originally empty, otherwise true
   */
  bool try_pop(data_type& res) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (queue_.empty()) {
      return false;
    }

    res = std::move(queue_.front());
    queue_.pop_front();
    return true;
  }

  /**
   * Try to steal a piece of data from the tail of the queue
   * @param res stores the stolen data
   * @return false is returned if the queue was originally empty, otherwise true
   */
  bool try_steal(data_type& res) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (queue_.empty()) {
      return false;
    }

    if (queue_.back().isNullTask()) {
      return false;
    }

    res = std::move(queue_.back());
    queue_.pop_back();
    return true;
  }
};

} /* namespace hayaku */
