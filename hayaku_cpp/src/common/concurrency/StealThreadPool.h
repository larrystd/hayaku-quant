#pragma once

/*
 * StealThreadPool.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-9-16
 *      Author: fasiondog
 */

#include <future>
#include <thread>
#include <vector>

#include "InterruptFlag.h"
#include "ThreadSafeQueue.h"
#include "WorkStealQueue.h"
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
 * @brief Distributed stealing thread pool
 * @note It is mainly used in the recursive case where a task creates further
 * tasks and adds them to the thread pool; otherwise an ordinary thread pool is
 * recommended
 * @details
 * @ingroup ThreadPool
 */
#ifdef _MSC_VER
class StealThreadPool {
#else
class HAYAKU_UTILS_API StealThreadPool {
#endif
 public:
  /**
   * Default constructor, it creates the number of the threads equal to the
   * number of the CPUs of the current system
   */
  StealThreadPool() : StealThreadPool(std::thread::hardware_concurrency()) {}

  /**
   * Constructor, it creates the given number of the threads
   * @param n the given number of the threads
   * @param until_empty it stops running automatically when the task queue is
   * empty
   */
  explicit StealThreadPool(size_t n, bool until_empty = true)
      : done_(false), worker_num_(n), running_until_empty_(until_empty) {
    try {
      interrupt_flags_.resize(worker_num_);
      for (int i = 0; i < worker_num_; i++) {
        // Create the worker threads and their task queues
        queues_.emplace_back(new WorkStealQueue);
      }
      // The threads are started after all the thread resources have been
      // initialized
      for (int i = 0; i < worker_num_; i++) {
        threads_.emplace_back(&StealThreadPool::worker_thread, this, i);
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
  ~StealThreadPool() {
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
    size_t total = master_work_queue_.size();
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
          "You can't submit a task to the stopped StealThreadPool!!");
    }

    int index = -1;
    auto iter = thread_index_.find(std::this_thread::get_id());
    if (iter != thread_index_.end()) {
      index = iter->second;
    }

    typedef typename std::invoke_result<FunctionType>::type result_type;
    std::packaged_task<result_type()> task(std::forward<FunctionType>(f));
    task_handle<result_type> res(task.get_future());
    if (index != -1 && !interrupt_flags_[index]) {
      // The local thread tasks enter the queue from the front (recursion
      // becomes a stack)
      queues_[index]->push_front(std::move(task));
    } else {
      master_work_queue_.push(std::move(task));
      cv_.notify_one();
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

    done_ = true;

    // At the same time the end task indication is added, so that it can also be
    // terminated when the dll exits
    for (size_t i = 0; i < worker_num_; i++) {
      interrupt_flags_[i].set();
      queues_[i]->push_front(FuncWrapper());
    }

    cv_.notify_all();  // Wake up all the worker threads
    for (size_t i = 0; i < worker_num_; i++) {
      if (threads_[i].joinable()) {
        threads_[i].join();
      }
    }

    master_work_queue_.clear();
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
    if (running_until_empty_) {
      while (true) {
        if (master_work_queue_.size() != 0) {
          std::this_thread::yield();
        } else {
          bool can_quit = true;
          for (size_t i = 0; i < worker_num_; i++) {
            if (!queues_[i]->empty()) {
              can_quit = false;
              break;
            }
          }
          if (can_quit) {
            break;
          } else {
            std::this_thread::yield();
          }
        }
      }

      done_ = true;
      for (size_t i = 0; i < worker_num_; i++) {
        interrupt_flags_[i].set();
      }
    }

    for (size_t i = 0; i < worker_num_; i++) {
      master_work_queue_.push(FuncWrapper());
    }

    // Wake up all the worker threads
    cv_.notify_all();

    // Wait for the threads to be finished
    for (size_t i = 0; i < worker_num_; i++) {
      if (threads_[i].joinable()) {
        threads_[i].join();
      }
    }

    done_ = true;
    master_work_queue_.clear();
    for (size_t i = 0; i < worker_num_; i++) {
      queues_[i]->clear();
    }
  }

  struct ExecutorWrapper {
    StealThreadPool* pool;
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
  std::condition_variable
      cv_;  // Semaphore, it blocks the threads and waits when there is no task
  std::mutex cv_mutex_;  // The mutex working together with the semaphore

  std::vector<InterruptFlag> interrupt_flags_;  // Worker thread states
  ThreadSafeQueue<task_type>
      master_work_queue_;  // Task queue of the master thread
  std::vector<std::unique_ptr<WorkStealQueue>>
      queues_;                        // Task queues (one for every worker
                                       // thread)
  std::vector<std::thread> threads_;  // Worker threads
  std::unordered_map<std::thread::id, int> thread_index_;

  void worker_thread(int index) {
    while (!done_ && !interrupt_flags_[index]) {
      run_pending_task(index);
    }
  }

  void run_pending_task(int index) {
    // Take the work task from the local queue first; if there is no local task,
    // take it from the master queue If the task taken from the master queue is
    // an empty task, this thread is considered to be ended; otherwise a task is
    // stolen from the other work queues
    task_type task;
    if (pop_task_from_local_queue(task, index)) {
      if (!task.isNullTask()) {
        task();
      } else {
        interrupt_flags_[index].set();
      }
    } else if (pop_task_from_master_queue(task)) {
      if (!task.isNullTask()) {
        task();
      } else {
        interrupt_flags_[index].set();
      }
    } else if (pop_task_from_other_thread_queue(task, index)) {
      task();
    } else {
      std::unique_lock<std::mutex> lk(cv_mutex_);
      cv_.wait(lk, [this] {
        return this->done_ || !this->master_work_queue_.empty();
      });
    }
  }

  bool pop_task_from_master_queue(task_type& task) {
    return master_work_queue_.try_pop(task);
  }

  // cppcheck-suppress functionStatic  // Suppress the cppcheck suggestion of
  // converting it into a static function
  bool pop_task_from_local_queue(task_type& task, int index) {
    return queues_[index]->try_pop(task);
  }

  bool pop_task_from_other_thread_queue(task_type& task, int index) {
    for (int i = 0; i < worker_num_; ++i) {
      int pos = (index + i + 1) % worker_num_;
      if (pos != index && !interrupt_flags_[pos] &&
          queues_[pos]->try_steal(task)) {
        return true;
      }
    }
    return false;
  }
};

} /* namespace hayaku */
