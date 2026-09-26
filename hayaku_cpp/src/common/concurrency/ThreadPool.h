#pragma once

/*
 * StealThreadPool.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-9-16
 *      Author: fasiondog
 */

#include <cstdio>
#include <future>
#include <thread>
#include <vector>

#include "../Log.h"
#include "FuncWrapper.h"
#include "InterruptFlag.h"
#include "ThreadSafeQueue.h"
#include "common/CppDef.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

namespace hayaku {

/**
 * @brief An ordinary centralized task queue thread pool; the tasks are
 * independent of each other and cannot wait for each other
 * @note If the tasks have a sequential order, please use StealThreadPool.
 * @details
 * @ingroup ThreadPool
 */
class ThreadPool {
 public:
  /**
   * Default constructor, it creates the number of the threads equal to the
   * number of the CPUs of the current system
   */
  ThreadPool() : ThreadPool(std::thread::hardware_concurrency()) {}

  /**
   * Constructor, it creates the given number of the threads
   * @param n the given number of the threads
   * @param until_empty when joining, it waits for the task queue to be empty
   * and then stops running
   */
  explicit ThreadPool(size_t n, bool until_empty = true)
      : done_(false), worker_num_(n), running_until_empty_(until_empty) {
    try {
      // The threads are started after all the thread resources have been
      // initialized
      for (int i = 0; i < worker_num_; i++) {
        // Create the worker threads and their task queues
        threads_.emplace_back(&ThreadPool::worker_thread, this, i);
      }
    } catch (...) {
      done_ = true;
      throw;
    }
  }

  /**
   * Destructor, it waits and blocks until all the tasks in the thread pool are
   * finished
   */
  ~ThreadPool() {
    if (!done_) {
      join();
    }
    threads_.clear();
  }

  /** Get the number of the worker threads */
  size_t worker_num() const { return worker_num_; }

  /** The type of the corresponding future returned after submitting a task to
   * the thread pool */
  template <typename ResultType>
  using task_handle = std::future<ResultType>;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

  /** Submit a task to the thread pool */
  template <typename FunctionType>
  auto submit(FunctionType&& f) {
    if (done_) {
      throw std::logic_error(
          "You can't submit a task to the stopped task group!");
    }
    typedef typename std::invoke_result<FunctionType>::type result_type;
    std::packaged_task<result_type()> task(std::forward<FunctionType>(f));
    task_handle<result_type> res(task.get_future());
    master_work_queue_.push(std::move(task));
    return res;
  }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

  /** Return the end state of the thread pool */
  bool done() const { return done_; }

  /** Number of the remaining tasks */
  size_t remain_task_count() const { return master_work_queue_.size(); }

  /**
   * It waits for every thread to finish the currently executed task and then
   * exits immediately
   */
  void stop() {
    if (done_.exchange(true, std::memory_order_relaxed)) {
      return;
    }

    // At the same time the end task indication is added, so that it can also be
    // terminated when the dll exits
    for (size_t i = 0; i < worker_num_; i++) {
      master_work_queue_.push(FuncWrapper());
    }

    master_work_queue_.notify_all();

    {
      std::lock_guard<std::mutex> lock(mutex_join_);
      for (size_t i = 0; i < worker_num_; i++) {
        if (threads_[i].joinable()) {
          threads_[i].join();
        }
      }
    }

    master_work_queue_.clear();
  }

  /**
   * It waits and blocks until all the tasks in the thread pool are finished
   * @note From then on the thread pool cannot be used after the worker threads
   * are ended
   */
  void join() {
    if (done_) {
      return;
    }

    // It instructs every worker thread to stop running when no work task is got
    if (!running_until_empty_) {
      done_ = true;
    }

    // It is still possible that some thread does not get it and thus is not
    // terminated
    for (size_t i = 0; i < 2 * worker_num_; i++) {
      master_work_queue_.push(FuncWrapper());
    }

    master_work_queue_.notify_all();

    {
      std::lock_guard<std::mutex> lock(mutex_join_);
      for (size_t i = 0; i < worker_num_; i++) {
        if (threads_[i].joinable()) {
          threads_[i].join();
        }
      }
    }

    done_ = true;
    master_work_queue_.clear();
  }

  struct ExecutorWrapper {
    ThreadPool* pool;
    template <typename Function>
    void execute(Function f) {
      pool->submit(std::move(f));
    }
  };

  /** Coroutine executor */
  ExecutorWrapper executor() { return ExecutorWrapper{this}; }

 private:
  typedef FuncWrapper task_type;
  std::atomic_bool
      done_;           // The global termination indication of the thread pool
  size_t worker_num_;  // Number of the worker threads
  bool running_until_empty_;  // It stops running automatically when the task
                               // queue is empty

  ThreadSafeQueue<task_type>
      master_work_queue_;             // Task queue of the master thread
  std::vector<std::thread> threads_;  // Worker threads
  std::mutex mutex_join_;             // Used to protect joinable

  void worker_thread(int index) {
    while (!done_) {
      task_type task;
      master_work_queue_.wait_and_pop(task);
      if (task.isNullTask()) {
        break;
      }
      task();
    }
  }

};  // namespace hayaku

} /* namespace hayaku */

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
