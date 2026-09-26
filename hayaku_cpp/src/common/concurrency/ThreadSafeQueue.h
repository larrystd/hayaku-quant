#pragma once

/*
 * ThreadSafeQueue.h
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

/**
 * Thread safe queue
 */
template <typename T>
class ThreadSafeQueue {
 public:
  /** Constructor */
  ThreadSafeQueue() {}

  /** Insert the element into the tail of the queue */
  void push(T&& item) {
    std::lock_guard<std::mutex> lk(mutex_);
    queue_.push(std::move(item));
    cond_.notify_one();
  }

  /** Wait until an element is taken from the head of the queue */
  void wait_and_pop(T& value) {
    std::unique_lock<std::mutex> lk(mutex_);
    cond_.wait(lk, [this] { return !queue_.empty(); });
    value = std::move(queue_.front());
    queue_.pop();
  }

  /** Wait until an element is taken from the head of the queue */
  std::shared_ptr<T> wait_and_pop() {
    std::unique_lock<std::mutex> lk(mutex_);
    cond_.wait(lk, [this] { return !queue_.empty(); });
    std::shared_ptr<T> res(std::make_shared<T>(std::move(queue_.front())));
    queue_.pop();
    return res;
  }

  /** Try to take an element from the head of the queue; true is returned on
   * success and false on failure */
  bool try_pop(T& value) {
    std::lock_guard<std::mutex> lk(mutex_);
    if (queue_.empty()) {
      return false;
    }
    value = std::move(queue_.front());
    queue_.pop();
    return true;
  }

  /** Try to take an element from the head of the queue; true is returned on
   * success and false on failure */
  std::shared_ptr<T> try_pop() {
    std::lock_guard<std::mutex> lk(mutex_);
    if (queue_.empty()) {
      return std::shared_ptr<T>();
    }
    std::shared_ptr<T> res(std::make_shared<T>(std::move(queue_.front())));
    queue_.pop();
    return res;
  }

  /** Whether the queue is empty */
  bool empty() const {
    std::lock_guard<std::mutex> lk(mutex_);
    return queue_.empty();
  }

  /** Queue size, ! it is not locked, use it with caution */
  size_t size() const { return queue_.size(); }

  /** Clear the task queue */
  void clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto tmp = std::queue<T>();
    queue_.swap(tmp);
  }

  void notify_all() { cond_.notify_all(); }

 private:
  mutable std::mutex mutex_;
  std::queue<T> queue_;
  std::condition_variable cond_;
};

} /* namespace hayaku */
