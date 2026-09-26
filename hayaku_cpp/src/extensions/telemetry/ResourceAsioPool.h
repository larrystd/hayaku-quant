#pragma once

/*
 * ResourceAsioPool.h
 *
 *  Copyright (c) 2025, hikyuu.org
 *
 *  Created on: 2025-03-17
 *      Author: fasiondog
 */

#include <atomic>
#include <boost/lockfree/queue.hpp>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

#include "common/Expected.h"
#include "common/Log.h"
#include "common/Net.h"
#include "common/Parameter.h"
#include "common/ResourceVersionTraits.h"

namespace hayaku {

using net::awaitable;
using net::co_spawn;
using net::detached;
using net::use_awaitable;
namespace this_coro = net::this_coro;

namespace rap {
class NullLock {
 public:
  void lock() {}
  void unlock() {}
  bool try_lock() { return true; }
};

}  // namespace rap

/**
 * General shared resource pool - it is suitable for the coroutine environment
 * It uses the boost lock free queue and acquires the resources asynchronously
 * in a coroutine
 * @ingroup Utilities
 *
 * @tparam ResourceType the resource type, it must support the constructor
 * ResourceType(const Parameter&)
 * @tparam MutexType the mutex type, std::mutex by default (thread safe)
 */
template <typename ResourceType, typename MutexType = std::mutex>
class ResourceAsioPool {
 private:
  struct WaiterNode {
    std::shared_ptr<net::steady_timer> timer;
    ResourceType *reserved_resource = nullptr;  // The reserved resource pointer
    WaiterNode() = default;
  };

 public:
  ResourceAsioPool() = delete;
  ResourceAsioPool(const ResourceAsioPool &) = delete;
  ResourceAsioPool &operator=(const ResourceAsioPool &) = delete;

  /**
   * Constructor
   * @param param connection parameters
   * @param max_count the maximum resource upper limit, 0 means unlimited
   */
  explicit ResourceAsioPool(const Parameter &param, size_t max_count = 128,
                            size_t max_waiters = 1000)
      : max_count_(max_count),
        param_(param),
        resource_list_(max_count == 0 ? 128 : max_count),
        max_waiters_(max_waiters > 0 ? max_waiters : 1000),
        waiter_queue_(max_waiters_) {
    if (max_waiters == 0) {
      HAYAKU_WARN("ResourceAsioPool: max_waiters is 0, using default 1000");
    } else if (max_waiters_ > 10000) {
      double estimated_mb = max_waiters_ * 150.0 / 1024 / 1024;
      HAYAKU_WARN(
          "ResourceAsioPool: large max_waiters={}, "
          "estimated memory usage at full capacity: ~{:.1f}MB",
          max_waiters_, estimated_mb);
    }
  }

  /**
   * Destructor, it releases all the cached resources
   */
  virtual ~ResourceAsioPool() {
    // Mark that it is being destructed, preventing new resource acquisitions
    // and returns
    is_destroying_.store(true, std::memory_order_release);

    // Cancel and clean up all the waiters
    WaiterNode *waiter = nullptr;
    while (waiter_queue_.pop(waiter)) {
      if (waiter) {
        // Try to cancel the timer to judge whether the coroutine is still
        // waiting
        std::size_t cancelled_ops = 0;
        if (waiter->timer) {
          cancelled_ops = waiter->timer->cancel();
        }

        // Only on a successful cancel is it safe to delete the reserved
        // resource If cancel returns 0, the coroutine may have woken up and
        // used the resource
        if (cancelled_ops > 0 && waiter->reserved_resource) {
          delete waiter->reserved_resource;
          count_.fetch_sub(1);
        }

        delete waiter;
      }
    }

    // Wait for all the active resources to be returned
    // When m_count == m_idleCount all the resources have been returned to the
    // idle queue
    std::unique_lock<MutexType> lock(destroy_mutex_);
    destroy_cv_.wait(lock, [this]() {
      return count_.load(std::memory_order_relaxed) ==
             idle_count_.load(std::memory_order_relaxed);
    });

    // At this time all the resources are in the idle queue, release them
    ResourceType *p = nullptr;
    while (resource_list_.pop(p)) {
      if (p) {
        delete p;
      }
    }
  }

  /** Resource instance pointer type */
  typedef std::shared_ptr<ResourceType> ResourcePtr;

  /**
   * Acquire an available resource synchronously (without waiting; it returns a
   * failure directly when there is no idle resource)
   *
   * @return stdx::expected<ResourcePtr, std::string>
   *         it contains the resource pointer on success and the error
   * information on failure
   *
   * @note The acquisition strategy:
   *       1. Acquire the resource from the idle queue preferentially and check
   * its version
   *       2. If the resource version is too old, destroy the resource and try
   * to create a new one
   *       3. If there is no idle resource but the upper limit has not been
   * reached, create a new version resource
   *       4. If the upper limit has been reached and there is no idle resource,
   * return a failure immediately (without waiting)
   *       5. Use asyncGet() if an asynchronous waiting is needed
   *
   * @example
   * @code
   * auto result = pool.get();
   * if (result) {
   *     auto resource = result.value();
   *     // The resource is guaranteed to be of the current latest version
   *     resource->doWork();
   * } else {
   *     HAYAKU_ERROR("Failed to get resource: {}", result.error());
   * }
   * @endcode
   */
  stdx::expected<ResourcePtr, std::string> get() {
    // 1. Try to acquire a resource from the idle queue
    ResourceType *p = nullptr;
    if (resource_list_.pop(p)) {
      idle_count_.fetch_sub(1);
      return stdx::expected<ResourcePtr, std::string>(
          ResourcePtr(p, ResourceCloser(this)));
    }

    // 2. There is no idle resource but the upper limit has not been reached ->
    // create a new
    //    resource
    if (max_count_ == 0 ||
        count_.load(std::memory_order_relaxed) < max_count_) {
      try {
        p = new ResourceType(param_);
      } catch (const std::exception &e) {
        return stdx::unexpected(std::string(
            fmt::format("Failed create a new Resource! {}", e.what())));
      } catch (...) {
        return stdx::unexpected(
            std::string("Failed create a new Resource! Unknown error!"));
      }
      count_.fetch_add(1);
      return stdx::expected<ResourcePtr, std::string>(
          ResourcePtr(p, ResourceCloser(this)));
    }

    // 3. The upper limit has been reached and there is no idle resource ->
    // return a failure
    //    directly
    return stdx::unexpected(
        fmt::format("No available resource, max_count={}, current_count={}",
                    max_count_, count_.load(std::memory_order_relaxed)));
  }

  /**
   * Acquire an available resource in a coroutine way (with a timeout)
   * @param timeout timeout
   * @return awaitable<stdx::expected<ResourcePtr, std::string>>
   * an awaitable result, it contains the resource pointer on success and the
   * error information on failure
   */
  awaitable<stdx::expected<ResourcePtr, std::string>> asyncGet(
      std::chrono::steady_clock::duration timeout = std::chrono::seconds(5)) {
    // Try to acquire a resource from the idle queue
    ResourceType *p = nullptr;
    if (resource_list_.pop(p)) {
      idle_count_.fetch_sub(1);
      co_return stdx::expected<ResourcePtr, std::string>(
          ResourcePtr(p, ResourceCloser(this)));
    }

    // There is no idle resource but the upper limit has not been reached ->
    // create a new resource
    if (max_count_ == 0 ||
        count_.load(std::memory_order_relaxed) < max_count_) {
      try {
        p = new ResourceType(param_);
      } catch (const std::exception &e) {
        co_return stdx::unexpected(std::string(
            fmt::format("Failed create a new Resource! {}", e.what())));
      } catch (...) {
        co_return stdx::unexpected(
            std::string("Failed create a new Resource! Unknown error!"));
      }
      count_.fetch_add(1);
      co_return stdx::expected<ResourcePtr, std::string>(
          ResourcePtr(p, ResourceCloser(this)));
    }

    // The upper limit has been reached -> enter the waiting queue
    auto executor = co_await this_coro::executor;
    auto waiter = new WaiterNode();
    waiter->timer = std::make_shared<net::steady_timer>(executor);
    waiter->timer->expires_after(timeout);

    // Join the waiting queue in a lock free way
    if (!waiter_queue_.push(waiter)) {
      delete waiter;
      co_return stdx::unexpected(
          fmt::format("Waiter queue is full (max={})", max_waiters_));
    }

    // Wait to be woken up or to time out
    net::error_code ec;
    try {
      co_await waiter->timer->async_wait(
          net::redirect_error(net::use_awaitable, ec));
    } catch (...) {
      ec = net::error::operation_aborted;
    }

    if (ec == net::error::operation_aborted && waiter->reserved_resource) {
      // It has been woken up and there is a reserved resource, use it directly
      // (success is guaranteed)
      ResourceType *res = waiter->reserved_resource;
      delete waiter;  // Clean up the node

      co_return stdx::expected<ResourcePtr, std::string>(
          ResourcePtr(res, ResourceCloser(this)));
    } else {
      // Timeout; note: the waiter is still in the queue and is cleaned up by
      // the returning thread or the destructor
      co_return stdx::unexpected(fmt::format(
          "ResourceAsioPool get timeout, max_count={}, current_count={}",
          max_count_, count_.load()));
    }
  }

  /** The number of the currently active resources, i.e. all the resources
   * (including the idle and the used ones) */
  size_t count() const { return count_.load(); }

  /**
   * The current number of the idle resources (the exact value)
   * It is tracked with an atomic counter to avoid operating the queue itself
   */
  size_t idleCount() const { return idle_count_.load(); }

  /** Release all the currently idle resources */
  void releaseIdleResource() {
    ResourceType *p = nullptr;
    while (resource_list_.pop(p)) {
      if (p) {
        idle_count_.fetch_sub(1);  // Decrease the idle count
        delete p;
        count_.fetch_sub(1);  // Decrease the count

        // Notify the destructor: the resource count has changed
        destroy_cv_.notify_one();
      }
    }
  }

 private:
  class ResourceCloser {
   public:
    explicit ResourceCloser(ResourceAsioPool *pool) : pool_(pool) {}

    void operator()(ResourceType *conn) {
      if (conn) {
        // If the pool is bound, the resource is returned; otherwise it is
        // deleted
        if (pool_) {
          pool_->returnResource(conn, this);
        } else {
          delete conn;
        }
      }
    }

   private:
    ResourceAsioPool *pool_;
  };

  /** Return it to the resource pool */
  void returnResource(ResourceType *p, ResourceCloser *closer) {
    if (!p) [[unlikely]] {
      HAYAKU_WARN("ResourceAsioPool::returnResource: nullptr");
      return;
    }

    // If it is being destructed, delete the resource directly
    if (is_destroying_.load(std::memory_order_acquire)) {
      delete p;
      count_.fetch_sub(1);
      destroy_cv_.notify_one();
      return;
    }

    // Try to wake up a waiter that has not timed out (the resource reservation
    // mechanism)
    WaiterNode *waiter = nullptr;

    // Pop in a loop until a waiter that has not timed out is found or the queue
    // is empty
    while (waiter_queue_.pop(waiter)) {
      // Reserve the resource first and then cancel the timer (key: ensure the
      // resource is ready before the wake-up)
      waiter->reserved_resource = p;

      // Try to cancel the timer and judge the success through the return value
      std::size_t cancelled_ops = waiter->timer->cancel();

      if (cancelled_ops > 0) {
        // The cancel succeeded; the coroutine is woken up and uses the reserved
        // resource directly
        destroy_cv_.notify_one();
        return;
      } else {
        // cancel returns 0, meaning the timer has expired naturally (a timeout)
        // Cancel the reservation, clean up the node and continue to find the
        // next waiter
        waiter->reserved_resource = nullptr;
        delete waiter;
      }
    }

    // There is no valid waiter, put it back into the idle queue
    if (!resource_list_.push(p)) {
      // The queue is full, delete it directly
      delete p;
      count_.fetch_sub(1);
      destroy_cv_.notify_one();
      return;
    }

    idle_count_.fetch_add(1);
    destroy_cv_.notify_one();
  }

  std::atomic<size_t> count_{
      0};  // The number of the currently active resources
  std::atomic<size_t> idle_count_{
      0};             // The current number of the idle resources
  size_t max_count_;  // The maximum resource upper limit
  Parameter param_;
  boost::lockfree::queue<ResourceType *> resource_list_;

  size_t max_waiters_;  // The runtime logical upper limit: the
                        // maximum number of the waiters
  boost::lockfree::queue<WaiterNode *>
      waiter_queue_;          // The lock free waiting queue
  MutexType destroy_mutex_;  // The mutex protecting the destructor waiting
                              // condition variable
  std::condition_variable_any
      destroy_cv_;  // Used to notify the destructor that a
                     // resource has been returned
  std::atomic<bool> is_destroying_{
      false};  // Marks whether it is being destructed
};

/**
 * @brief Versioned resource pool (the resource type is required to support the
 * version interfaces)
 * @details It uses boost::lockfree::queue to implement the lock free idle queue
 * and supports the asynchronous resource acquisition in a coroutine. When the
 * parameters change, the version number is increased automatically and all the
 * idle old version resources are released.
 *
 *          **Important constraint**: ResourceType must implement the
 * getVersion() and setVersion(int) methods.
 *
 * @tparam ResourceType the resource type, it must implement the getVersion()
 * and setVersion(int) methods
 * @tparam MutexType the mutex type, std::mutex by default (thread safe)
 * @ingroup Utilities
 */
template <typename ResourceType, typename MutexType = std::mutex>
class ResourceAsioVersionPool {
 private:
  struct WaiterNode {
    std::shared_ptr<net::steady_timer> timer;
    ResourceType *reserved_resource = nullptr;  // The reserved resource pointer
    WaiterNode() = default;
  };

 public:
  // Compile-time check: ResourceType must support getVersion and setVersion
  static_assert(hayaku::detail::has_resource_getVersion_v<ResourceType>,
                "ResourceType must implement getVersion() method.");
  static_assert(hayaku::detail::has_resource_setVersion_v<ResourceType>,
                "ResourceType must implement setVersion(int) method.");

  ResourceAsioVersionPool() = delete;
  ResourceAsioVersionPool(const ResourceAsioVersionPool &) = delete;
  ResourceAsioVersionPool &operator=(const ResourceAsioVersionPool &) = delete;

  /**
   * Constructor
   * @param param connection parameters
   * @param max_count the maximum resource upper limit, 0 means unlimited
   * @param max_waiters the maximum number of the waiters
   */
  explicit ResourceAsioVersionPool(const Parameter &param, size_t max_count = 0,
                                   size_t max_waiters = 1000)
      : max_count_(max_count),
        param_(param),
        resource_list_(128),
        max_waiters_(max_waiters > 0 ? max_waiters : 1000),
        waiter_queue_(max_waiters_) {
    if (max_waiters == 0) {
      HAYAKU_WARN(
          "ResourceAsioVersionPool: max_waiters is 0, using default 1000");
    } else if (max_waiters_ > 10000) {
      double estimated_mb = max_waiters_ * 150.0 / 1024 / 1024;
      HAYAKU_WARN(
          "ResourceAsioVersionPool: large max_waiters={}, "
          "estimated memory usage at full capacity: ~{:.1f}MB",
          max_waiters_, estimated_mb);
    }
  }

  /**
   * Destructor, it releases all the cached resources
   */
  virtual ~ResourceAsioVersionPool() {
    // Mark that it is being destructed, preventing new resource acquisitions
    // and returns
    is_destroying_.store(true, std::memory_order_release);

    // Cancel and clean up all the waiters
    WaiterNode *waiter = nullptr;
    while (waiter_queue_.pop(waiter)) {
      if (waiter) {
        // Try to cancel the timer to judge whether the coroutine is still
        // waiting
        std::size_t cancelled_ops = 0;
        if (waiter->timer) {
          cancelled_ops = waiter->timer->cancel();
        }

        // Only on a successful cancel is it safe to delete the reserved
        // resource If cancel returns 0, the coroutine may have woken up and
        // used the resource
        if (cancelled_ops > 0 && waiter->reserved_resource) {
          delete waiter->reserved_resource;
          count_.fetch_sub(1);
        }

        delete waiter;
      }
    }

    // Wait for all the active resources to be returned
    std::unique_lock<MutexType> lock(destroy_mutex_);
    destroy_cv_.wait(lock, [this]() {
      return count_.load(std::memory_order_relaxed) == idle_count_.load();
    });

    // At this time all the resources are in the idle queue, release them
    ResourceType *p = nullptr;
    while (resource_list_.pop(p)) {
      if (p) {
        delete p;
      }
    }
  }

  /** Whether the given parameter exists */
  bool haveParam(const std::string &name) {
    std::lock_guard<MutexType> lock(mutex_);
    return param_.have(name);
  }

  /** Get the value of the given parameter; an exception is thrown when the
   * parameter does not exist or the type does not match */
  template <typename ValueType>
  ValueType getParam(const std::string &name) {
    std::lock_guard<MutexType> lock(mutex_);
    return param_.get<ValueType>(name);
  }

  /**
   * @brief Set the value of the given parameter; the parameter takes effect
   * only when a new resource is created
   * @details When the parameter already exists, the type of the newly set value
   * must be the same as that of the original parameter, otherwise an exception
   * is thrown
   * @param name parameter name
   * @param value parameter value
   * @exception std::logic_error
   */
  template <typename ValueType>
  void setParam(const std::string &name, const ValueType &value) {
    std::lock_guard<MutexType> lock(mutex_);
    // If the parameter has not actually changed, return directly
    if (param_.have(name) && value == param_.get<ValueType>(name)) {
      return;
    }
    param_.set<ValueType>(name, value);
    version_.fetch_add(1);
    releaseIdleResource();  // Release the current idle resources so that the
                            // new parameter values take effect
  }

  /**
   * @brief Set the resource parameters; they take effect only when a new
   * resource is created
   * @param param the parameter object
   */
  void setParameter(const Parameter &param) {
    std::lock_guard<MutexType> lock(mutex_);
    param_ = param;
    version_.fetch_add(1);
    releaseIdleResource();  // Release the current idle resources so that the
                            // new parameter values take effect
  }

  /**
   * @brief Set the resource parameters; they take effect only when a new
   * resource is created
   * @param param the parameter object
   */
  void setParameter(Parameter &&param) {
    std::lock_guard<MutexType> lock(mutex_);
    param_ = std::move(param);
    version_.fetch_add(1);
    releaseIdleResource();  // Release the current idle resources so that the
                            // new parameter values take effect
  }

  /** Get the current version of the resource pool */
  int getVersion() { return version_.load(); }

  /** Increase the current version of the resource pool, equivalent to notifying
   * the resource pool that the resource version has changed */
  void incVersion(int version) { version_.fetch_add(1); }

  /** Resource instance pointer type */
  typedef std::shared_ptr<ResourceType> ResourcePtr;

  /**
   * Acquire an available resource synchronously (without waiting; it returns a
   * failure directly when there is no idle resource)
   * @return stdx::expected<ResourcePtr, std::string>
   * It contains the resource pointer on success and the error information on
   * failure
   */
  stdx::expected<ResourcePtr, std::string> get() {
    // 1. Try to acquire a resource from the idle queue
    ResourceType *p = nullptr;
    if (resource_list_.pop(p)) {
      idle_count_.fetch_sub(1);

      // Check the resource version and destroy it if it is too old
      if (p->getVersion() != version_.load()) {
        delete p;
        count_.fetch_sub(1);
        p = nullptr;
      } else {
        return stdx::expected<ResourcePtr, std::string>(
            ResourcePtr(p, ResourceCloser(this)));
      }
    }

    // 2. The upper limit has not been reached, create a new resource
    if (max_count_ == 0 ||
        count_.load(std::memory_order_relaxed) < max_count_) {
      try {
        Parameter current_param;
        {
          std::lock_guard<MutexType> lock(mutex_);
          current_param = param_;
        }

        p = new ResourceType(current_param);
        p->setVersion(version_.load());
      } catch (const std::exception &e) {
        return stdx::unexpected(std::string(
            fmt::format("Failed create a new Resource! {}", e.what())));
      } catch (...) {
        return stdx::unexpected(
            std::string("Failed create a new Resource! Unknown error!"));
      }

      count_.fetch_add(1);
      return stdx::expected<ResourcePtr, std::string>(
          ResourcePtr(p, ResourceCloser(this)));
    }

    // 3. The upper limit has been reached and there is no idle resource ->
    // return a failure
    //    directly
    return stdx::unexpected(
        fmt::format("No available resource, max_count={}, current_count={}",
                    max_count_, count_.load(std::memory_order_relaxed)));
  }

  /**
   * Acquire an available resource in a coroutine way (with a timeout)
   * @param timeout timeout, 5 seconds by default
   * @return awaitable<stdx::expected<ResourcePtr, std::string>>
   * an awaitable result, it contains the resource pointer on success and the
   * error information on failure
   */
  awaitable<stdx::expected<ResourcePtr, std::string>> asyncGet(
      std::chrono::steady_clock::duration timeout = std::chrono::seconds(5)) {
    auto executor = co_await this_coro::executor;

    // Try to acquire a resource from the idle queue
    ResourceType *p = nullptr;
    if (resource_list_.pop(p)) {
      idle_count_.fetch_sub(1);

      // Check the resource version and destroy it if it is too old; a new
      // resource is created below
      if (p->getVersion() != version_.load()) {
        delete p;
        count_.fetch_sub(1);
        p = nullptr;
      } else {
        co_return stdx::expected<ResourcePtr, std::string>(
            ResourcePtr(p, ResourceCloser(this)));
      }
    }

    // The upper limit has not been reached, create a new resource
    if (max_count_ == 0 ||
        count_.load(std::memory_order_relaxed) < max_count_) {
      try {
        Parameter current_param;
        int current_version;
        {
          std::lock_guard<MutexType> lock(mutex_);
          current_param = param_;
          current_version = version_.load();
        }

        p = new ResourceType(current_param);
        p->setVersion(current_version);
      } catch (const std::exception &e) {
        co_return stdx::unexpected(std::string(
            fmt::format("Failed create a new Resource! {}", e.what())));
      } catch (...) {
        co_return stdx::unexpected(
            std::string("Failed create a new Resource! Unknown error!"));
      }

      count_.fetch_add(1);
      co_return stdx::expected<ResourcePtr, std::string>(
          ResourcePtr(p, ResourceCloser(this)));
    }

    // The upper limit has been reached, enter the waiting queue
    auto waiter = new WaiterNode();
    waiter->timer = std::make_shared<net::steady_timer>(executor);
    waiter->timer->expires_after(timeout);

    // Join the waiting queue in a lock free way
    if (!waiter_queue_.push(waiter)) {
      delete waiter;
      co_return stdx::unexpected(
          fmt::format("Waiter queue is full (max={})", max_waiters_));
    }

    // Wait to be woken up or to time out
    net::error_code ec;
    try {
      co_await waiter->timer->async_wait(
          net::redirect_error(net::use_awaitable, ec));
    } catch (...) {
      ec = net::error::operation_aborted;
    }

    if (ec == net::error::operation_aborted && waiter->reserved_resource) {
      // It has been woken up and there is a reserved resource, use it directly
      // (success is guaranteed)
      ResourceType *res = waiter->reserved_resource;
      delete waiter;  // Clean up the node

      co_return stdx::expected<ResourcePtr, std::string>(
          ResourcePtr(res, ResourceCloser(this)));
    } else {
      // Timeout
      // Note: the waiter is still in the queue and is cleaned up by the
      // returning thread or the destructor
      co_return stdx::unexpected(fmt::format(
          "ResourceAsioVersionPool get timeout, max_count={}, current_count={}",
          max_count_, count_.load(std::memory_order_relaxed)));
    }
  }

  /** The number of the currently active resources, i.e. all the resources
   * (including the idle and the used ones) */
  size_t count() const { return count_.load(); }

  /**
   * The current number of the idle resources (the exact value)
   * It is tracked with an atomic counter to avoid operating the queue itself
   */
  size_t idleCount() const { return idle_count_.load(); }

  /** Release all the currently idle resources */
  void releaseIdleResource() {
    ResourceType *p = nullptr;
    while (resource_list_.pop(p)) {
      if (p) {
        idle_count_.fetch_sub(1);  // Decrease the idle count
        delete p;
        count_.fetch_sub(1);  // Decrease the count

        // Notify the destructor: the resource count has changed
        destroy_cv_.notify_one();
      }
    }
  }

 private:
  class ResourceCloser {
   public:
    explicit ResourceCloser(ResourceAsioVersionPool *pool) : pool_(pool) {}

    void operator()(ResourceType *conn) {
      if (conn) {
        // If the pool is bound, the resource is returned; otherwise it is
        // deleted
        if (pool_) {
          pool_->returnResource(conn, this);
        } else {
          delete conn;
        }
      }
    }

   private:
    ResourceAsioVersionPool *pool_;
  };

  /** Return it to the resource pool */
  void returnResource(ResourceType *p, ResourceCloser *closer) {
    if (!p) [[unlikely]] {
      HAYAKU_WARN("ResourceAsioVersionPool::returnResource: nullptr");
      return;
    }

    // If it is being destructed, delete the resource directly
    if (is_destroying_.load(std::memory_order_acquire)) {
      delete p;
      count_.fetch_sub(1);
      destroy_cv_.notify_one();
      return;
    }

    // The returned resource is accepted only when its version equals the
    // resource pool version
    if (p->getVersion() == version_.load()) {
      // Try to wake up a waiter that has not timed out (the resource
      // reservation mechanism)
      WaiterNode *waiter = nullptr;

      // Pop in a loop until a waiter that has not timed out is found or the
      // queue is empty
      while (waiter_queue_.pop(waiter)) {
        // Reserve the resource first and then cancel the timer (key: ensure the
        // resource is ready before the wake-up)
        waiter->reserved_resource = p;

        // Try to cancel the timer and judge the success through the return
        // value
        std::size_t cancelled_ops = waiter->timer->cancel();

        if (cancelled_ops > 0) {
          // The cancel succeeded; the coroutine is woken up and uses the
          // reserved resource directly

          // The waiter is deleted by the coroutine after the resource is
          // acquired
          destroy_cv_.notify_one();
          return;
        } else {
          // cancel returns 0, meaning the timer has expired naturally (a
          // timeout) Cancel the reservation, clean up the node and continue to
          // find the next waiter
          waiter->reserved_resource = nullptr;
          delete waiter;
        }
      }

      // There is no valid waiter, put it back into the idle queue
      if (!resource_list_.push(p)) {
        // The queue is full, delete it directly
        delete p;
        count_.fetch_sub(1);
        destroy_cv_.notify_one();
        return;
      }

      idle_count_.fetch_add(1);
      destroy_cv_.notify_one();
    } else {
      delete p;
      count_.fetch_sub(1);
      destroy_cv_.notify_one();
    }
  }

  std::atomic<size_t> count_{
      0};  // The number of the currently active resources
  std::atomic<size_t> idle_count_{
      0};             // The current number of the idle resources
  size_t max_count_;  // The maximum resource upper limit
  Parameter param_;
  boost::lockfree::queue<ResourceType *> resource_list_;
  std::atomic<int> version_{0};  // The current resource pool version

  mutable MutexType mutex_;  // The mutex protecting the parameter access
  size_t max_waiters_;        // The maximum number of the waiters
  boost::lockfree::queue<WaiterNode *>
      waiter_queue_;          // The lock free waiting queue
  MutexType destroy_mutex_;  // The mutex protecting the destructor waiting
                              // condition variable
  std::condition_variable_any
      destroy_cv_;  // Used to notify the destructor that a
                     // resource has been returned
  std::atomic<bool> is_destroying_{
      false};  // Marks whether it is being destructed
};

}  // namespace hayaku
