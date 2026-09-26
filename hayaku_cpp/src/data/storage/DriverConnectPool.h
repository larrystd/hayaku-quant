#pragma once

/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-19
 *     Author: fasiondog
 */

#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include "common/Parameter.h"

namespace hayaku {

/**
 * Driver resource pool
 * @tparam DriverConnectT driver type, it is required to have the DriverType
 * *clone() method
 * @ingroup DataDriver
 */
template <class DriverConnectT>
class DriverConnectPool {
 public:
  DriverConnectPool() = delete;
  DriverConnectPool(const DriverConnectPool &) = delete;
  DriverConnectPool &operator=(const DriverConnectPool &) = delete;

  typedef typename DriverConnectT::DriverTypePtr DriverPtr;
  typedef std::shared_ptr<DriverConnectT> DriverConnectPtr;

  /**
   * Constructor
   * @param prototype driver prototype, the ownership is transferred to this
   * pool
   * @param maxConnect the maximum number of connections allowed, 0 means
   * unlimited
   * @param maxIdleConnect the maximum number of idle connections allowed, 0
   * means releasing immediately, the default is the number of CPUs
   */
  explicit DriverConnectPool(
      const DriverPtr &prototype, size_t maxConnect = 0,
      size_t maxIdleConnect = std::thread::hardware_concurrency())
      : max_size_(maxConnect),
        max_idel_size_(maxIdleConnect),
        count_(0),
        prototype_(prototype),
        closer_(this) {}

  /**
   * Destructor, it releases all the cached connections
   */
  virtual ~DriverConnectPool() {
    while (!driver_list_.empty()) {
      DriverConnectT *p = driver_list_.front();
      driver_list_.pop();
      if (p) {
        delete p;
      }
    }
  }

  /** Get an available connection; if the maximum number of connections allowed
   * is exceeded, it blocks and waits until an idle resource is obtained */
  DriverConnectPtr getConnect() noexcept {
    std::unique_lock<std::mutex> lock(mutex_);
    if (driver_list_.empty()) {
      if (max_size_ > 0 && count_ >= max_size_) {
        cond_.wait(lock, [this] { return !driver_list_.empty(); });
      } else {
        count_++;
        return DriverConnectPtr(new DriverConnectT(prototype_->clone()),
                                closer_);
      }
    }
    DriverConnectT *p = driver_list_.front();
    driver_list_.pop();
    return DriverConnectPtr(p, closer_);
  }

  DriverPtr getPrototype() { return prototype_; }

  /** Number of the currently active connections */
  size_t count() const { return count_; }

  /** Number of the currently idle resources */
  size_t idleCount() const { return driver_list_.size(); }

  /** Release all the currently idle resources */
  void releaseIdleConnect() {
    std::lock_guard<std::mutex> lock(mutex_);
    while (!driver_list_.empty()) {
      DriverConnectT *p = driver_list_.front();
      driver_list_.pop();
      count_--;
      if (p) {
        delete p;
      }
    }
  }

 private:
  /** Return it to the connection pool */
  void returnDriver(DriverConnectT *p) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (p) {
      if (driver_list_.size() < max_idel_size_) {
        driver_list_.push(p);
        cond_.notify_all();
      } else {
        delete p;
        count_--;
      }
    } else {
      count_--;
      HAYAKU_WARN("Trying to return an empty pointer!");
    }
  }

 private:
  size_t max_size_;       // The maximum number of connections allowed
  size_t max_idel_size_;   // The maximum number of idle connections allowed
  size_t count_;         // The number of currently active connections
  DriverPtr prototype_;  // Driver prototype
  std::mutex mutex_;
  std::condition_variable cond_;
  std::queue<DriverConnectT *> driver_list_;

  class DriverCloser {
   public:
    explicit DriverCloser(DriverConnectPool *pool) : pool_(pool) {}
    void operator()(DriverConnectT *conn) {
      if (pool_ && conn) {
        pool_->returnDriver(conn);
      }
    }

   private:
    DriverConnectPool *pool_;
  };

  DriverCloser closer_;
};

}  // namespace hayaku
