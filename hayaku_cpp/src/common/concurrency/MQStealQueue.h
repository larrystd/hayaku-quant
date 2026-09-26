#pragma once

/*
 * MQStealQueue.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-11-17
 *      Author: fasiondog
 */

#include <condition_variable>
#include <queue>
#include <thread>

namespace hayaku {

template <typename T>
class MQStealQueue {
 public:
  MQStealQueue() {}

  void push(T&& item) {
    std::lock_guard<std::mutex> lk(mutex_);
    queue_.push_back(std::move(item));
    cond_.notify_one();
  }

  /** Insert the data into the head of the queue */
  void push_front(T&& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push_front(std::move(data));
    cond_.notify_one();
  }

  void wait_and_pop(T& value) {
    std::unique_lock<std::mutex> lk(mutex_);
    cond_.wait(lk, [this] { return !queue_.empty(); });
    value = std::move(queue_.front());
    queue_.pop_front();
  }

  std::shared_ptr<T> wait_and_pop() {
    std::unique_lock<std::mutex> lk(mutex_);
    cond_.wait(lk, [this] { return !queue_.empty(); });
    std::shared_ptr<T> res(std::make_shared<T>(std::move(queue_.front())));
    queue_.pop_front();
    return res;
  }

  bool try_pop(T& value) {
    std::lock_guard<std::mutex> lk(mutex_);
    if (queue_.empty()) {
      return false;
    }
    value = std::move(queue_.front());
    queue_.pop_front();
    return true;
  }

  std::shared_ptr<T> try_pop() {
    std::lock_guard<std::mutex> lk(mutex_);
    if (queue_.empty()) {
      return std::shared_ptr<T>();
    }
    std::shared_ptr<T> res(std::make_shared<T>(std::move(queue_.front())));
    queue_.pop();
    return res;
  }

  /**
   * Try to steal a piece of data from the tail of the queue
   * @param res stores the stolen data
   * @return false is returned if the queue was originally empty, otherwise true
   */
  bool try_steal(T& res) {
    std::lock_guard<std::mutex> lock(mutex_);
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

  bool empty() const {
    std::lock_guard<std::mutex> lk(mutex_);
    return queue_.empty();
  }

  // Queue size, lock free
  size_t size() const { return queue_.size(); }

  void clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto tmp = std::deque<T>();
    queue_.swap(tmp);
  }

 private:
  mutable std::mutex mutex_;
  std::deque<T> queue_;
  std::condition_variable cond_;
};

} /* namespace hayaku */
