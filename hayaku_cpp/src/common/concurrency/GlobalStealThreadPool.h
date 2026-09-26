#pragma once

/*
 * GlobalStealThreadPool.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-9-16
 *      Author: fasiondog
 */

#include <future>
#include <thread>
#include <vector>

#include "../Log.h"
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
class HAYAKU_UTILS_API GlobalStealThreadPool {
 public:
  /**
   * Default constructor, it creates the number of the threads equal to the
   * number of the CPUs of the current system
   */
  GlobalStealThreadPool()
      : GlobalStealThreadPool(std::thread::hardware_concurrency()) {}

  /**
   * Constructor, it creates the given number of the threads
   * @param n the given number of the threads
   * @param until_empty it stops running automatically when the task queue is
   * empty
   */
  explicit GlobalStealThreadPool(size_t n, bool until_empty = true)
      : done_(false),
        worker_num_(n),
        running_until_empty_(until_empty),
        sleep_count_(0) {
    try {
      interrupt_flags_.resize(worker_num_, nullptr);
      for (int i = 0; i < worker_num_; i++) {
        // Create the worker threads and their task queues
        queues_.emplace_back(new WorkStealQueue);
      }
      // The threads are started after all the thread resources have been
      // initialized
      for (int i = 0; i < worker_num_; i++) {
        threads_.emplace_back(&GlobalStealThreadPool::worker_thread, this, i);
      }
    } catch (...) {
      done_.store(true, std::memory_order_release);
      throw;
    }
  }

  /**
   * Destructor, it waits and blocks until all the tasks in the thread pool are
   * finished
   */
  ~GlobalStealThreadPool() {
    if (!done_.load(std::memory_order_acquire)) {
      join();
    }
  }

  /** Get the number of the worker threads */
  size_t worker_num() const { return worker_num_; }

  /** Get the number of the currently sleeping worker threads */
  int sleep_count() const {
    return sleep_count_.load(std::memory_order_acquire);
  }

  /**
   * Intelligently wake up the sleeping threads
   * It adaptively judges whether a wake-up is needed according to the current
   * number of the remaining tasks and the number of the sleeping threads
   * @return the number of the actually woken threads
   */
  int wake_up() {
    HAYAKU_IF_RETURN(done_.load(std::memory_order_acquire), 0);
    int sleeping_count = sleep_count_.load(std::memory_order_acquire);
    if (sleeping_count <= 0) {
      return 0;
    }

    // Get the current number of the remaining tasks
    size_t remaining_tasks = remain_task_count();
    if (remaining_tasks == 0) {
      // There is no remaining task, no wake-up is needed
      return 0;
    }

    // The intelligent wake-up strategy:
    // 1. If the number of the tasks is greater than or equal to the number of
    // the sleeping
    //    threads, use notify_all to wake up all the threads (more efficient)
    // 2. If the number of the tasks is less than the number of the sleeping
    // threads, wake up
    //    precisely the needed number of the threads
    int threads_to_wake = 0;
    if (remaining_tasks >= static_cast<size_t>(sleeping_count)) {
      // The tasks are sufficient, use notify_all to wake up all the sleeping
      // threads (better performance)
      cv_.notify_all();
      threads_to_wake = sleeping_count;
    } else {
      // There are few tasks, wake up precisely as needed
      threads_to_wake = static_cast<int>(remaining_tasks);
      // Ensure that at least one thread is woken up to handle the task
      threads_to_wake = std::max(threads_to_wake, 1);

      // Wake up precisely the given number of the threads
      for (int i = 0; i < threads_to_wake; ++i) {
        cv_.notify_one();
      }
    }

    return threads_to_wake;
  }

  /** Number of the remaining tasks */
  size_t remain_task_count() const {
    if (done_.load(std::memory_order_acquire)) {
      return 0;
    }
    size_t total = master_work_queue_.size();
    for (size_t i = 0; i < worker_num_; i++) {
      total += queues_[i]->size();
    }
    return total;
  }

  /** Whether the current thread is a worker thread */
  static bool is_work_thread() { return local_work_queue_ != nullptr; }

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
    if (thread_need_stop_.isSet() || done_.load(std::memory_order_acquire)) {
      throw std::logic_error(
          "You can't submit a task to the stopped GlobalStealThreadPool!!");
    }

    typedef typename std::invoke_result<FunctionType>::type result_type;
    std::packaged_task<result_type()> task(std::forward<FunctionType>(f));
    task_handle<result_type> res(task.get_future());

    std::thread::id id = std::this_thread::get_id();
    if (local_work_queue_ && id == thread_id_) {
      // The local thread tasks enter the queue from the front (recursion
      // becomes a stack)
      local_work_queue_->push_front(std::move(task));
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
  bool done() const { return done_.load(std::memory_order_acquire); }

  /**
   * It waits for every thread to finish the currently executed task and then
   * exits immediately
   */
  void stop() {
    if (done_.exchange(true, std::memory_order_acq_rel)) {
      return;
    }

    // Reset the sleep count
    sleep_count_.store(0, std::memory_order_release);

    // At the same time the end task indication is added, so that it can also be
    // terminated when the dll exits
    for (size_t i = 0; i < worker_num_; i++) {
      if (interrupt_flags_[i]) {
        interrupt_flags_[i]->set();
      }
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
    threads_.clear();
  }

  /**
   * It waits and blocks until all the tasks in the thread pool are finished
   * @note From then on the thread pool cannot be used after the worker threads
   * are ended
   */
  void join() {
    if (done_.load(std::memory_order_acquire)) {
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
            if (queues_[i]->size() != 0) {
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

      done_.store(true, std::memory_order_release);
      for (size_t i = 0; i < worker_num_; i++) {
        if (interrupt_flags_[i]) {
          interrupt_flags_[i]->set();
        }
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

    done_.store(true, std::memory_order_release);
    master_work_queue_.clear();
    for (size_t i = 0; i < worker_num_; i++) {
      queues_[i]->clear();
    }
    threads_.clear();
  }

  struct ExecutorWrapper {
    GlobalStealThreadPool* pool;
    template <typename Function>
    void execute(Function f) {
      pool->submit(std::move(f));
    }
  };

  /** Coroutine executor */
  ExecutorWrapper executor() { return ExecutorWrapper{this}; }

 public:
  bool run_available_task_once() {
    HAYAKU_IF_RETURN(
        done_.load(std::memory_order_acquire) || thread_need_stop_.isSet(),
        false);
    bool task_run = false;
    task_type task;
    if (local_work_queue_) {
      if (pop_task_from_local_queue(task)) {
        if (!task.isNullTask()) {
          task();
          task_run = true;
        } else {
          thread_need_stop_.set();
        }
      } else if (pop_task_from_other_thread_queue(task)) {
        task();
        task_run = true;
      } else if (pop_task_from_master_queue(task)) {
        if (!task.isNullTask()) {
          task();
          task_run = true;
        } else {
          thread_need_stop_.set();
        }
      }
    } else if (pop_task_from_master_queue(task)) {
      if (!task.isNullTask()) {
        task();
        task_run = true;
      }
    }
    return task_run;
  }

 private:
  typedef FuncWrapper task_type;
  std::atomic_bool
      done_;           // The global termination indication of the thread pool
  size_t worker_num_;  // Number of the worker threads
  bool running_until_empty_;    // It stops running automatically when the task
                                 // queue is empty
  std::condition_variable cv_;  // Semaphore, it blocks the threads and waits
                                 // when there is no task
  std::mutex cv_mutex_;  // The mutex working together with the semaphore
  std::atomic<int> sleep_count_;  // Sleep count

  std::vector<InterruptFlag*> interrupt_flags_;  // Worker thread states
  ThreadSafeQueue<task_type>
      master_work_queue_;  // Task queue of the master thread
  std::vector<std::unique_ptr<WorkStealQueue> >
      queues_;                        // Task queues (one for every worker
                                       // thread)
  std::vector<std::thread> threads_;  // Worker threads

// Thread local variables
#if HAYAKU_OS_WINDOWS
  static WorkStealQueue* local_work_queue_;  // Local task queue
  static int index_;                         // The index in the thread pool
  static InterruptFlag
      thread_need_stop_;  // The indication for stopping the thread
  static std::thread::id thread_id_;

#else
#if CPP_STANDARD >= CPP_STANDARD_17 && !defined(__clang__)
  inline static thread_local WorkStealQueue* local_work_queue_ =
      nullptr;                                  // Local task queue
  inline static thread_local int index_ = -1;  // The index in the thread pool
  inline static thread_local InterruptFlag
      thread_need_stop_;  // The indication for stopping the
                           // thread
  inline static thread_local std::thread::id thread_id_;
#else
  static thread_local WorkStealQueue* local_work_queue_;  // Local task queue
  static thread_local int index_;  // The index in the thread pool
  static thread_local InterruptFlag
      thread_need_stop_;  // The indication for stopping the thread
  static thread_local std::thread::id thread_id_;
#endif
#endif

  void worker_thread(int index) {
    thread_id_ = std::this_thread::get_id();
    interrupt_flags_[index] = &thread_need_stop_;
    index_ = index;
    local_work_queue_ = queues_[index].get();
    while (!thread_need_stop_.isSet() &&
           !done_.load(std::memory_order_acquire)) {
      run_pending_task();
    }
    local_work_queue_ = nullptr;
    interrupt_flags_[index] = nullptr;
  }

  void run_pending_task() {
    // Take the work task from the local queue first; if there is no local task,
    // take it from the master queue If the task taken from the master queue is
    // an empty task, this thread is considered to be ended; otherwise a task is
    // stolen from the other work queues
    task_type task;
    if (pop_task_from_local_queue(task)) {
      if (!task.isNullTask()) {
        task();
      } else {
        thread_need_stop_.set();
      }
    } else if (pop_task_from_master_queue(task)) {
      if (!task.isNullTask()) {
        task();
      } else {
        thread_need_stop_.set();
      }
    } else if (pop_task_from_other_thread_queue(task)) {
      task();
    } else {
      // Increase the sleep count before entering the waiting state
      sleep_count_.fetch_add(1, std::memory_order_acq_rel);

      // std::this_thread::yield();
      std::unique_lock<std::mutex> lk(cv_mutex_);
      cv_.wait(lk, [this] {
        return this->done_.load(std::memory_order_acquire) ||
               !this->master_work_queue_.empty() ||
               (local_work_queue_ && !local_work_queue_->empty()) ||
               has_other_remain_task();
      });

      // Decrease the sleep count after being woken up
      sleep_count_.fetch_sub(1, std::memory_order_acq_rel);
    }
  }

  bool pop_task_from_master_queue(task_type& task) {
    return master_work_queue_.try_pop(task);
  }

  // cppcheck-suppress functionStatic  // Suppress the cppcheck suggestion of
  // converting it into a static function
  bool pop_task_from_local_queue(task_type& task) {
    return local_work_queue_ && local_work_queue_->try_pop(task);
  }

  bool pop_task_from_other_thread_queue(task_type& task) {
    for (int i = 0; i < worker_num_; ++i) {
      int index = (index_ + i + 1) % worker_num_;
      if (index != index_ && queues_[index]->try_steal(task)) {
        return true;
      }
    }
    return false;
  }

  bool has_other_remain_task() {
    for (int i = 0; i < worker_num_; ++i) {
      if (i != index_ && queues_[i] && !queues_[i]->empty()) {
        return true;
      }
    }
    return false;
  }
};

} /* namespace hayaku */

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
