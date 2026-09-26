#pragma once

/*
 * StealMQStealThreadPool.h
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
#include "MQStealQueue.h"
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
 * @brief Multi queue task stealing pool without a centralized queue
 * @ingroup ThreadPool
 */
#ifdef _MSC_VER
class MQStealThreadPool {
#else
class HAYAKU_UTILS_API MQStealThreadPool {
#endif
 public:
  /**
   * Default constructor, it creates the number of the threads equal to the
   * number of the CPUs of the current system
   */
  MQStealThreadPool()
      : MQStealThreadPool(std::thread::hardware_concurrency()) {}

  /**
   * Constructor, it creates the given number of the threads
   * @param n the given number of the threads
   * @param until_empty it stops running automatically when the task queue is
   * empty
   */
  explicit MQStealThreadPool(size_t n, bool until_empty = true)
      : done_(false), worker_num_(n), runnging_until_empty_(until_empty) {
    try {
      interrupt_flags_.resize(worker_num_);
      for (size_t i = 0; i < worker_num_; i++) {
        // Create the worker threads and their task queues
        queues_.emplace_back(new MQStealQueue<task_type>);
      }
      // The threads are started after all the thread resources have been
      // initialized
      for (int i = 0; i < worker_num_; i++) {
        threads_.emplace_back(&MQStealThreadPool::worker_thread, this, i);
        thread_index_[threads_.back().get_id()] = i;
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
  ~MQStealThreadPool() {
    if (!done_) {
      join();
    }
    threads_.clear();
  }

  /** Get the number of the worker threads */
  size_t worker_num() const { return worker_num_; }

  /** Number of the remaining tasks */
  size_t remain_task_count() const {
    if (done_) {
      return 0;
    }

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
  auto submit(FunctionType&& f) {
    if (done_) {
      throw std::logic_error(
          "You can't submit a task to the stopped MQStealThreadPool!");
    }

    int index = -1;
    auto iter = thread_index_.find(std::this_thread::get_id());
    if (iter != thread_index_.end()) {
      index = iter->second;
    }

    typedef typename std::invoke_result<FunctionType>::type result_type;
    std::packaged_task<result_type()> task(std::forward<FunctionType>(f));
    task_handle<result_type> res(task.get_future());

    // If it is the local thread and the thread has not been terminated, it is
    // added to its own queue
    if (index != -1 && interrupt_flags_[index]) {
      // The local thread tasks enter the queue from the front (recursion
      // becomes a stack)
      queues_[index]->push_front(std::move(task));
      return res;
    }

    queues_[current_index_]->push(std::move(task));
    current_index_++;
    if (current_index_ >= worker_num_) {
      current_index_ = 0;
    }
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
    if (done_) {
      return;
    }

    // At the same time the end task indication is added, so that it can also be
    // terminated when the dll exits
    for (size_t i = 0; i < worker_num_; i++) {
      if (interrupt_flags_[i]) {
        interrupt_flags_[i].set();
      }
      queues_[i]->push(FuncWrapper());
    }

    for (size_t i = 0; i < worker_num_; i++) {
      if (threads_[i].joinable()) {
        threads_[i].join();
      }
    }

    done_ = true;
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
    if (runnging_until_empty_) {
      while (true) {
        bool can_quit = true;
        for (size_t i = 0; i < worker_num_; i++) {
          if (!queues_[i]->empty()) {
            can_quit = false;
            break;
          }
        }

        if (can_quit) {
          break;
        }

        std::this_thread::yield();
      }

      done_ = true;
      for (size_t i = 0; i < worker_num_; i++) {
        if (interrupt_flags_[i]) {
          interrupt_flags_[i].set();
        }
      }
    }

    for (size_t i = 0; i < worker_num_; i++) {
      queues_[i]->push(FuncWrapper());
    }

    // Wait for the threads to be finished
    for (size_t i = 0; i < worker_num_; i++) {
      if (threads_[i].joinable()) {
        threads_[i].join();
      }
    }

    done_ = true;
    for (size_t i = 0; i < worker_num_; i++) {
      queues_[i]->clear();
    }
  }

  struct ExecutorWrapper {
    MQStealThreadPool* pool;
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

  std::vector<std::unique_ptr<MQStealQueue<task_type>>>
      queues_;                                  // Thread task queues
  std::vector<InterruptFlag> interrupt_flags_;  // Thread termination flags
  std::vector<std::thread> threads_;            // Worker threads

  std::unordered_map<std::thread::id, int> thread_index_;
  int current_index_ =
      0;  // The queue index used when a new task is placed currently

  void worker_thread(int index) {
    while (!interrupt_flags_[index].isSet() && !done_) {
      run_pending_task(index);
    }
  }

  void run_pending_task(int index) {
    task_type task;
    // Try to get a task from the local queue and execute it
    if (queues_[index]->try_pop(task)) {
      if (task.isNullTask()) {
        interrupt_flags_[index].set();
      } else {
        task();
      }
    } else if (pop_task_from_other_thread_queue(task, index)) {
      task();
    } else {
      // Block and wait for a new task in the local queue
      // Note: in the recursive case the tasks are preferentially added to the
      // local queue and depend on each other; if a task is stolen by another
      // thread, the other thread would be blocked and wait Therefore it waits
      // for the local queue here instead of continuing to steal in a loop
      queues_[index]->wait_and_pop(task);
      if (task.isNullTask()) {
        interrupt_flags_[index].set();
      } else {
        task();
      }
    }
  }

  bool pop_task_from_other_thread_queue(task_type& task, int index) {
    for (size_t i = 0; i < worker_num_; ++i) {
      size_t pos = (index + i + 1) % worker_num_;
      if (pos != index && queues_[pos]->try_steal(task)) {
        return true;
      }
    }
    return false;
  }
};  // namespace hayaku

} /* namespace hayaku */

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
