#pragma once

/*
 * StealMQThreadPool.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-9-16
 *      Author: fasiondog
 */

#include <chrono>
#include <future>
#include <thread>
#include <vector>

#include "FuncWrapper.h"
#include "InterruptFlag.h"
#include "ThreadSafeQueue.h"
#include "common/CppDef.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

#ifndef HAYAKU_UTILS_API
#define HAYAKU_UTILS_API
#endif

namespace hayaku {

/**
 * @brief An ordinary multi task queue thread pool; the tasks are independent of
 * each other and cannot wait for each other
 * @note If the tasks have a sequential order, please use StealThreadPool.
 * @details
 * @ingroup ThreadPool
 */
#ifdef _MSC_VER
class MQThreadPool {
#else
class HAYAKU_UTILS_API MQThreadPool {
#endif
 public:
  /**
   * Default constructor, it creates the number of the threads equal to the
   * number of the CPUs of the current system
   */
  MQThreadPool() : MQThreadPool(std::thread::hardware_concurrency()) {}

  /**
   * Constructor, it creates the given number of the threads
   * @param n the given number of the threads
   * @param until_empty it stops running automatically when the task queue is
   * empty
   */
  explicit MQThreadPool(size_t n, bool until_empty = true)
      : done_(false), worker_num_(n), runnging_until_empty_(until_empty) {
    try {
      thread_need_stop_.resize(worker_num_);
      for (int i = 0; i < worker_num_; i++) {
        // Create the worker threads and their task queues
        queues_.push_back(std::unique_ptr<ThreadSafeQueue<task_type>>(
            new ThreadSafeQueue<task_type>));
      }
      // The threads are started after all the thread resources have been
      // initialized
      for (int i = 0; i < worker_num_; i++) {
        threads_.push_back(std::thread(&MQThreadPool::worker_thread, this, i));
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
  ~MQThreadPool() {
    if (!done_) {
      join();
    }
    threads_.clear();
  }

  /** Get the number of the worker threads */
  size_t worker_num() const { return worker_num_; }

  /** Number of the remaining tasks */
  size_t remain_task_count() const {
    size_t total = 0;
    for (size_t i = 0; i < worker_num_; i++) {
      total += queues_[i]->size();
    }
    return total;
  }

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
  auto submit(FunctionType &&f) {
    if (done_) {
      throw std::logic_error(
          "You can't submit a task to the stopped MQThreadPool!");
    }

    typedef typename std::invoke_result<FunctionType>::type result_type;
    std::packaged_task<result_type()> task(std::forward<FunctionType>(f));
    task_handle<result_type> res(task.get_future());

    // Add the task to the empty queue or the queue with the smallest number of
    // the tasks
    size_t min_count = std::numeric_limits<size_t>::max();
    int index = 0;
    for (int i = 0; i < worker_num_; ++i) {
      if (!thread_need_stop_[i].isSet()) {
        size_t cur_count = queues_[i]->size();
        if (cur_count == 0) {
          index = i;
          break;
        }

        if (cur_count < min_count) {
          min_count = cur_count;
          index = i;
        }
      }
    }

    queues_[index]->push(std::move(task));
    return res;
  }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

  /** Return the end state of the thread pool */
  bool done() const { return done_; }

  /**
   * It waits for every thread to finish the currently executed task and then
   * exits immediately
   */
  void stop() {
    if (done_.exchange(true, std::memory_order_relaxed)) {
      return;
    }

    for (size_t i = 0; i < worker_num_; i++) {
      thread_need_stop_[i].set();
      queues_[i]->push(FuncWrapper());
    }

    {
      std::lock_guard<std::mutex> lock(mutex_join_);
      for (size_t i = 0; i < worker_num_; i++) {
        if (threads_[i].joinable()) {
          threads_[i].join();
        }
      }
    }

    for (size_t i = 0; i < worker_num_; i++) {
      queues_[i]->clear();
    }
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
    if (!runnging_until_empty_) {
      done_ = true;
      for (size_t i = 0; i < worker_num_; i++) {
        thread_need_stop_[i].set();
      }
    }

    for (size_t i = 0; i < worker_num_; i++) {
      queues_[i]->push(FuncWrapper());
      queues_[i]->notify_all();
    }

    {  // Wait for the threads to be finished
      std::lock_guard<std::mutex> lock(mutex_join_);
      for (size_t i = 0; i < worker_num_; i++) {
        if (threads_[i].joinable()) {
          threads_[i].join();
        }
      }
    }

    done_ = true;
  }

  struct ExecutorWrapper {
    MQThreadPool *pool;
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
  bool runnging_until_empty_;  // It runs until the queue is empty and then
                                // stops

  std::vector<std::unique_ptr<ThreadSafeQueue<task_type>>>
      queues_;                                   // Thread task queues
  std::vector<InterruptFlag> thread_need_stop_;  // Thread termination flags
  std::vector<std::thread> threads_;             // Worker threads
  std::mutex mutex_join_;                        // Used to protect joinable

  void worker_thread(int index) {
    auto *local_queue = queues_[index].get();
    auto *local_stop_flag = &thread_need_stop_[index];
    while (!local_stop_flag->isSet() || !done_) {
      task_type task;
      local_queue->wait_and_pop(task);
      if (task.isNullTask()) {
        local_stop_flag->set();
        break;
      }
      task();
    }
  }
};

} /* namespace hayaku */

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
