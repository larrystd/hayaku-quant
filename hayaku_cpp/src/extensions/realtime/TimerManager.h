#pragma once

/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-08
 *     Author: fasiondog
 */

#include <forward_list>
#include <functional>
#include <unordered_map>

#include "common/CppDef.h"
#include "common/Log.h"
#include "common/concurrency/ThreadPool.h"
#include "common/time/Datetime.h"

namespace hayaku {

/**
 * Timer management and scheduling
 * @ingroup Utilities
 */
class TimerManager {
 public:
  TimerManager(const TimerManager&) = delete;
  TimerManager(TimerManager&) = delete;
  TimerManager(TimerManager&&) = delete;
  TimerManager& operator=(const TimerManager&) = delete;
  TimerManager& operator=(TimerManager&) = delete;
  TimerManager& operator=(TimerManager&&) = delete;

  /**
   * Constructor
   * @param work_num the number of the threads in the thread pool executing the
   * timer tasks
   */
  explicit TimerManager(size_t work_num = 1)
      : stop_(true),
        current_timer_id_(-1),
        work_num_(work_num),
        tg_(nullptr),
        use_extend_tg_(false) {
    HAYAKU_ASSERT(work_num >= 1);
    start();
  }

  /**
   * Constructor with the given thread pool, so as to share the other thread
   * pools
   * @note Please guarantee by yourself that the lifetime of tg is always valid
   * while TimerManager is alive
   * @param tg the given task group thread pool
   */
  explicit TimerManager(ThreadPool* tg)
      : stop_(true),
        current_timer_id_(-1),
        work_num_(1),
        tg_(tg),
        use_extend_tg_(true) {
    HAYAKU_ASSERT(tg_);
    start();
  }

  /** Destructor */
  ~TimerManager() {
    stop();
    for (auto iter = timers_.begin(); iter != timers_.end(); ++iter) {
      delete iter->second;
    }
  }

  /** Start the scheduling, it can be restarted after a stop */
  void start() {
    // It is already in the executing state, return directly
    HAYAKU_IF_RETURN(!stop_, void());

    // Set the executing state
    stop_ = false;

    std::unique_lock<std::mutex> lock(mutex_);

    std::priority_queue<IntervalS> new_queue;
    queue_.swap(new_queue);
    if (!tg_) {
      tg_ = new ThreadPool(work_num_);
    }

    /*
     * Rebuild the execution queue according to the existing timers and delete
     * the invalid ones
     */

    std::forward_list<int> invalid_timers;  // Records the invalid timers
    for (auto iter = timers_.begin(); iter != timers_.end(); ++iter) {
      int time_id = iter->first;
      const Timer* timer = iter->second;
      Datetime now = Datetime::now();

      // Record the invalid timer id
      if (timer->repeat_num_ <= 0 ||
          (timer->end_date_ != Datetime::max() &&
           timer->end_date_ + timer->end_time_ < now)) {
        invalid_timers.push_front(time_id);
        continue;
      }

      IntervalS s;
      s.timer_id_ = time_id;
      if (timer->start_time_ < TimeDelta()) {
        Datetime first_start_time = timer->start_date_ + timer->end_time_;
        if (first_start_time >= now) {
          s.time_point_ = first_start_time;
        } else {
          if (timer->repeat_num_ <= 1) {
            invalid_timers.push_front(time_id);
            continue;
          }
          s.time_point_ = now.startOfDay() + timer->end_time_;
          if (s.time_point_ < now) {
            s.time_point_ = s.time_point_ + TimeDelta(1);
          }
        }

      } else {
        s.time_point_ =
            timer->start_date_ >= now.startOfDay()
                ? timer->start_date_ + timer->start_time_ + timer->duration_
                : now + timer->duration_;
        if (timer->start_time_ != timer->end_time_) {
          Datetime point_date = s.time_point_.startOfDay();
          TimeDelta point = s.time_point_ - point_date;
          if (point < timer->start_time_) {
            s.time_point_ = point_date + timer->start_time_;
          } else if (point > timer->end_time_) {
            s.time_point_ = point_date + timer->start_time_ + TimeDelta(1);
          } else {
            TimeDelta gap = point - timer->start_time_;
            if (gap % timer->duration_ != TimeDelta()) {
              int x = int(gap / timer->duration_) + 1;
              s.time_point_ = point_date + timer->start_time_ +
                               timer->duration_ * double(x);
            }
          }
        }
      }

      queue_.push(s);
    }

    // Clear the invalid timers
    for (auto id : invalid_timers) {
      _removeTimer(id);
    }

    lock.unlock();
    cond_.notify_all();

    detect_thread_ = std::thread([this]() { detectThread(); });
  }

  /** Terminate the scheduling */
  void stop() {
    if (!stop_) {
      std::unique_lock<std::mutex> lock(mutex_);
      std::priority_queue<IntervalS> queue;
      queue_.swap(queue);
      stop_ = true;
      lock.unlock();
      cond_.notify_all();
    }

    if (detect_thread_.joinable()) {
      detect_thread_.join();
    }

    if (!use_extend_tg_ && tg_) {
      tg_->stop();
      delete tg_;
      tg_ = nullptr;
    }
  }

  /** Get the current number of the timer tasks */
  size_t size() {
    std::lock_guard<std::mutex> lock(mutex_);
    return timers_.size();
  }

  /** Whether it is currently empty */
  bool empty() { return size() == 0; }

  /** Return the current stop state */
  bool stopped() const { return stop_; }

  /**
   * Add a scheduled task; an exception is thrown when the addition fails
   * @tparam F the task type
   * @tparam Args the task parameters
   * @param start_date the start date allowed to run
   * @param end_date the end date allowed to run
   * @param start_time the start time allowed to run
   * @param end_time the end time allowed to run
   * @param repeat_num the number of the repetitions, it must be greater than 0;
   * it means an infinite loop when it equals std::numeric_limits<int>::max()
   * @param duration the interval, it needs to be greater than TimeDelta(0)
   * @param f the delayed task to be executed
   * @param args the concrete task parameters
   * @return timer id
   */
  template <typename F, typename... Args>
  int addFunc(Datetime start_date, Datetime end_date, TimeDelta start_time,
              TimeDelta end_time, int repeat_num, TimeDelta duration, F&& f,
              Args&&... args) {
    HAYAKU_CHECK(!start_date.isNull(), "Invalid start_date!");
    HAYAKU_CHECK(!end_date.isNull(), "Invalid end_date!");
    Datetime start = start_date.startOfDay();
    Datetime end = end_date.startOfDay();
    HAYAKU_CHECK(end >= start, "end_date({}) need > start_date({})!", end,
                 start);
    HAYAKU_CHECK(start_time >= TimeDelta(0) &&
                     start_time <= TimeDelta(0, 23, 59, 59, 999, 999),
                 "Invalid start_time: {}", start_time.repr());
    HAYAKU_CHECK(end_time >= TimeDelta(0) &&
                     end_time <= TimeDelta(0, 23, 59, 59, 999, 999),
                 "Invalid end_time: {}", end_time.repr());
    HAYAKU_CHECK(end_time >= start_time, "end_time({}) need >= start_time({})!",
                 end_time, start_time);
    HAYAKU_CHECK(repeat_num > 0, "Invalid repeat_num: {}", repeat_num);
    HAYAKU_CHECK(duration > TimeDelta(0), "Invalid duration: {}",
                 duration.repr());
    return _addFunc(start, end, start_time, end_time, repeat_num, duration,
                    std::forward<F>(f), std::forward<Args>(args)...);
  }

  /**
   * Add a repeated timer task; an exception is thrown when the addition fails
   * @tparam F the task type
   * @tparam Args the task parameters
   * @param repeat_num the number of the repetitions, it must be greater than 0;
   * it means an infinite loop when it equals std::numeric_limits<int>::max()
   * @param duration the interval, it needs to be greater than TimeDelta(0)
   * @param f the delayed task to be executed
   * @param args the concrete task parameters
   * @return timer id
   */
  template <typename F, typename... Args>
  int addDurationFunc(int repeat_num, TimeDelta duration, F&& f,
                      Args&&... args) {
    HAYAKU_CHECK(repeat_num > 0, "Invalid repeat_num: {}, must > 0",
                 repeat_num);
    HAYAKU_CHECK(duration > TimeDelta(),
                 "Invalid duration: {}, must > TimeDelta(0)!", duration.repr());
    return _addFunc(Datetime::min(), Datetime::max(), TimeDelta(), TimeDelta(),
                    repeat_num, duration, std::forward<F>(f),
                    std::forward<Args>(args)...);
  }

  /**
   * Add a delayed task (executed once only); an exception is thrown when the
   * addition fails
   * @tparam F the task type
   * @tparam Args the task parameters
   * @param delay the delay time, it needs to be greater than TimeDelta(0)
   * @param f the delayed task to be executed
   * @param args the concrete task parameters
   * @return timer id
   */
  template <typename F, typename... Args>
  int addDelayFunc(TimeDelta delay, F&& f, Args&&... args) {
    HAYAKU_CHECK(delay > TimeDelta(), "Invalid delay: {}, must > TimeDelta(0)!",
                 delay);
    return _addFunc(Datetime::min(), Datetime::max(), TimeDelta(), TimeDelta(),
                    1, delay, std::forward<F>(f), std::forward<Args>(args)...);
  }

  /**
   * Execute the task at the given moment (once only); an exception is thrown
   * when the addition fails
   * @tparam F the task type
   * @tparam Args the task parameters
   * @param time_point the given running moment (including the concrete day,
   * hour, minute, second
   *                   ...)
   * @param f the delayed task to be executed
   * @param args the concrete task parameters
   * @return timer id
   */
  template <typename F, typename... Args>
  int addFuncAtTime(Datetime time_point, F&& f, Args&&... args) {
    Datetime now = Datetime::now();
    HAYAKU_CHECK(time_point > now, "You want run at {}, but now is {}",
                 time_point, now);
    Datetime point_date = time_point.startOfDay();
    TimeDelta point = time_point - point_date;
    return _addFunc(time_point.startOfDay(), Datetime::max(), TimeDelta(-1),
                    point, 1, TimeDelta(), std::forward<F>(f),
                    std::forward<Args>(args)...);
  }

  /**
   * Execute the task at the given time within the day; an exception is thrown
   * when the addition fails
   * @tparam F the task type
   * @tparam Args the task parameters
   * @param start_date the start date allowed to be executed
   * @param end_date the end date allowed to be executed
   * @param time the given running time within the day
   * @param f the delayed task to be executed
   * @param args the concrete task parameters
   * @return timer id
   */
  template <typename F, typename... Args>
  int addFuncAtTimeEveryDay(Datetime start_date, Datetime end_date,
                            TimeDelta time, F&& f, Args&&... args) {
    HAYAKU_CHECK(!start_date.isNull() && !end_date.isNull(),
                 "Invalid start_date({}) or end_date({})!", start_date,
                 end_date);
    HAYAKU_CHECK(
        time >= TimeDelta() && time <= TimeDelta(0, 23, 59, 59, 999, 999),
        "Invalid time {}", time.repr());
    Datetime start = start_date.startOfDay();
    Datetime end = end_date.startOfDay();
    HAYAKU_CHECK(end >= start, "Invalid range of date! ({} - {})", start, end);
    return _addFunc(Datetime::min(), Datetime::max(), TimeDelta(-1), time,
                    std::numeric_limits<int>::max(), TimeDelta(),
                    std::forward<F>(f), std::forward<Args>(args)...);
  }

  /**
   * Execute the task at the given time every day; an exception is thrown when
   * the addition fails
   * @tparam F the task type
   * @tparam Args the task parameters
   * @param time the given running time within the day
   * @param f the delayed task to be executed
   * @param args the concrete task parameters
   * @return timer id
   */
  template <typename F, typename... Args>
  int addFuncAtTimeEveryDay(TimeDelta time, F&& f, Args&&... args) {
    return addFuncAtTimeEveryDay(Datetime::min(), Datetime::max(), time,
                                 std::forward<F>(f),
                                 std::forward<Args>(args)...);
  }

  /**
   * Remove a timer task
   * @param timerid timer id
   */
  void removeTimer(int timerid) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto iter = timers_.find(timerid);
    if (iter != timers_.end()) {
      iter->second->repeat_num_ = 0;
    }
  }

 private:
  void _removeTimer(int id) {
    delete timers_[id];
    timers_.erase(id);
  }

  void detectThread() {
    while (!stop_) {
      Datetime now = Datetime::now();
      std::unique_lock<std::mutex> lock(mutex_);
      if (queue_.empty()) {
        cond_.wait(lock);
        continue;
      }

      IntervalS s = queue_.top();
      if (s.time_point_ == Datetime::min()) {
        break;  // End the detection thread so that the dll can exit safely,
                // because the atomic may be invalid when the dll exits
      }

      TimeDelta diff = s.time_point_ - now;
      if (diff > TimeDelta()) {
        cond_.wait_for(
            lock, std::chrono::duration<int64_t, std::micro>(diff.ticks()));
        continue;
      }

      queue_.pop();

      // Get the current time again
      now = Datetime::now();

      auto timer_iter = timers_.find(s.timer_id_);
      if (timer_iter == timers_.end()) {
        continue;
      }

      auto timer = timer_iter->second;
      tg_->submit(timer->func_);

      if (timer->repeat_num_ != std::numeric_limits<int>::max()) {
        timer->repeat_num_--;
      }

      if (timer->repeat_num_ <= 0) {
        _removeTimer(s.timer_id_);
        continue;
      }

      // Calculate the time point of the next execution
      Datetime today = now.startOfDay();
      if (timer->start_time_ >= TimeDelta()) {
        // The timer not executed at the given moment
        s.time_point_ = s.time_point_ + timer->duration_;
        if (s.time_point_ < now) {
          // The system time is adjusted forward
          s.time_point_ = now;
        }

        // If the executable time range of the day is limited and the next
        // execution moment exceeds the limit of the day
        if (timer->start_time_ != timer->end_time_ &&
            s.time_point_ > today + timer->end_time_) {
          s.time_point_ = today + timer->start_time_ + TimeDelta(1);
        }

      } else {
        // The timer with the given daily running time
        s.time_point_ = s.time_point_ +
                         (today - s.time_point_.startOfDay() + TimeDelta(1));
      }

      if (timer->end_date_ != Datetime::max() &&
          s.time_point_ > timer->end_date_ + timer->end_time_) {
        _removeTimer(s.timer_id_);
        continue;
      }

      // Push the next running time into the queue
      queue_.push(s);
    }
  }

  // Allocate the timer_id
  int getNewTimerId() {
    int max_int = std::numeric_limits<int>::max();
    HAYAKU_WARN_IF_RETURN(timers_.size() >= size_t(max_int), -1,
                          "Timer queue is full!");

    if (current_timer_id_ >= max_int) {
      current_timer_id_ = 0;
    } else {
      current_timer_id_++;
    }

    while (true) {
      if (timers_.find(current_timer_id_) != timers_.end()) {
        if (current_timer_id_ >= max_int) {
          current_timer_id_ = 0;
        } else {
          current_timer_id_++;
        }
      } else {
        break;
      }
    }
    return current_timer_id_;
  }

 private:
  class Timer {
   public:
    void operator()() { func_(); }

    Datetime start_date_ =
        Datetime::min().startOfDay();  // The start date allowed to be
                                       // executed (inclusive)
    Datetime end_date_ =
        Datetime::max().startOfDay();  // The end date allowed to be executed
                                       // (inclusive)
    /*
     * Note: if m_start_time < TimeDelta(0), m_end_time represents the given
     * daily running time, and m_duration
     */
    TimeDelta start_time_;  // The start time of the day allowed to be executed
                             // (inclusive)
    TimeDelta end_time_;    // The end time of the day allowed to be executed
                             // (inclusive)
    TimeDelta duration_;    // The delay or the interval
    int repeat_num_ =
        1;  // The number of the repetitions, max means an infinite loop
    std::function<void()> func_;
  };

  struct IntervalS {
    Datetime time_point_;  // The exact time point of the execution
    int timer_id_ =
        -1;  // The corresponding Timer, a negative value is invalid
    bool operator<(const IntervalS& other) const {
      return time_point_ > other.time_point_;
    }
  };

  template <typename F, typename... Args>
  int _addFunc(Datetime start_date, Datetime end_date, TimeDelta start_time,
               TimeDelta end_time, int repeat_num, TimeDelta duration, F&& f,
               Args&&... args) {
    Datetime now = Datetime::now();
    Datetime today = now.startOfDay();
    HAYAKU_CHECK(end_date >= today, "Invalid end_date {}, because today is {}",
                 end_date, today);
    if (end_date != Datetime::max()) {
      HAYAKU_CHECK(end_date + end_time >= now,
                   "Invalid param! You want end time is {}, but now is {}",
                   end_date + end_time, now);
    }

    Timer* timer = new Timer;
    timer->start_date_ = start_date;
    timer->end_date_ = end_date;
    timer->start_time_ = start_time;
    timer->end_time_ = end_time;
    timer->repeat_num_ = repeat_num;
    timer->duration_ = duration;
    timer->func_ = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

    IntervalS s;
    if (start_time < TimeDelta()) {
      Datetime first_start_time = start_date + end_time;
      if (first_start_time >= now) {
        s.time_point_ = first_start_time;
      } else {
        HAYAKU_CHECK(repeat_num > 1,
                     "The time has expired! expect time {}, but now is {}",
                     first_start_time, now);
        s.time_point_ = today + end_time;
        if (s.time_point_ < now) {
          s.time_point_ = s.time_point_ + TimeDelta(1);
        }
      }

    } else {
      s.time_point_ = start_date >= today ? start_date + start_time + duration
                                           : now + duration;
      if (timer->start_time_ != timer->end_time_) {
        Datetime point_date = s.time_point_.startOfDay();
        TimeDelta point = s.time_point_ - point_date;
        if (point < timer->start_time_) {
          s.time_point_ = point_date + timer->start_time_;
        } else if (point > timer->end_time_) {
          s.time_point_ = point_date + timer->start_time_ + TimeDelta(1);
        } else {
          TimeDelta gap = point - timer->start_time_;
          if (gap % timer->duration_ != TimeDelta()) {
            int x = int(gap / timer->duration_) + 1;
            s.time_point_ = point_date + timer->start_time_ +
                             timer->duration_ * double(x);
          }
        }
      }
    }

    std::unique_lock<std::mutex> lock(mutex_);
    int id = getNewTimerId();
    if (id < 0) {
      delete timer;
      lock.unlock();
      HAYAKU_THROW("Failed to get new id, maybe too timers!");
    }

    timers_[id] = timer;
    s.timer_id_ = id;
    // HAYAKU_TRACE("s.m_time_point: {}", s.m_time_point.repr());
    queue_.push(s);
    lock.unlock();
    cond_.notify_all();
    return id;
  }

 private:
  std::priority_queue<IntervalS> queue_;
  std::atomic_bool stop_;
  std::mutex mutex_;
  std::condition_variable cond_;
  std::thread detect_thread_;

  std::unordered_map<int, Timer*> timers_;
  int current_timer_id_;
  size_t work_num_;  // The number of the threads in the task execution thread
                      // pool
  ThreadPool* tg_{nullptr};
  bool use_extend_tg_{false};
};

}  // namespace hayaku
