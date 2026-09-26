#pragma once

/*
 * ResourcePool.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-8-5
 *      Author: fasiondog
 */

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_set>

#include "common/Log.h"
#include "common/Parameter.h"
#include "common/ResourceVersionTraits.h"

namespace hayaku {

/**
 * Resource acquisition timeout exception
 */
class GetResourceTimeoutException : public hayaku::exception {
 public:
  GetResourceTimeoutException(const char *msg)
      : hayaku::exception(fmt::format("GetResourceTimeoutException {}", msg)) {}

  GetResourceTimeoutException(const std::string &msg)
      : hayaku::exception(fmt::format("GetResourceTimeoutException {}", msg)) {}

  virtual ~GetResourceTimeoutException() {}
};

/**
 * New resource creation failure exception
 */
class CreateResourceException : public hayaku::exception {
 public:
  CreateResourceException(const char *msg)
      : hayaku::exception(fmt::format("CreateResourceException {}", msg)) {}

  CreateResourceException(const std::string &msg)
      : hayaku::exception(fmt::format("CreateResourceException {}", msg)) {}

  virtual ~CreateResourceException() {}
};

/**
 * General shared resource pool
 * @ingroup Utilities
 */
template <typename ResourceType>
class ResourcePool {
 public:
  ResourcePool() = delete;
  ResourcePool(const ResourcePool &) = delete;
  ResourcePool &operator=(const ResourcePool &) = delete;

  /**
   * Constructor
   * @param param connection parameters
   * @param maxPoolSize the maximum number of the shared resources allowed, 0
   * means unlimited
   * @param maxIdleNum the maximum number of the idle resources allowed; 0 means
   * releasing immediately after use without a cache
   */
  explicit ResourcePool(const Parameter &param, size_t maxPoolSize = 0,
                        size_t maxIdleNum = 100)
      : max_pool_size_(maxPoolSize),
        max_idel_size_(maxIdleNum),
        count_(0),
        param_(param) {}

  /**
   * Destructor, it releases all the cached resources
   */
  virtual ~ResourcePool() {
    std::unique_lock<std::mutex> lock(mutex_);

    // Unbind the closer of all the allocated resources from the pool
    for (auto iter = closer_set_.begin(); iter != closer_set_.end(); ++iter) {
      (*iter)->unbind();
    }

    // Delete all the idle resources
    while (!resource_list_.empty()) {
      ResourceType *p = resource_list_.front();
      resource_list_.pop();
      if (p) {
        delete p;
      }
    }
  }

  /** Get the current maximum number of the resources allowed */
  size_t maxPoolSize() const { return max_idel_size_; }

  /** Get the current maximum number of the idle resources allowed */
  size_t maxIdleSize() const { return max_idel_size_; }

  /** Set the maximum number of the resources */
  void maxPoolSize(size_t num) {
    std::lock_guard<std::mutex> lock(mutex_);
    max_pool_size_ = num;
  }

  /** Set the maximum number of the idle resources allowed */
  void maxIdleSize(size_t num) {
    std::lock_guard<std::mutex> lock(mutex_);
    max_idel_size_ = num;
  }

  /** Resource instance pointer type */
  typedef std::shared_ptr<ResourceType> ResourcePtr;

  /**
   * Get an available resource; a null pointer is returned when the maximum
   * number of the resources allowed is exceeded
   * @exception CreateResourceException the new resource creation may throw an
   * exception
   */
  ResourcePtr get() {
    std::lock_guard<std::mutex> lock(mutex_);
    ResourcePtr result;
    ResourceType *p = nullptr;
    if (resource_list_.empty()) {
      if (max_pool_size_ > 0 && count_ >= max_pool_size_) {
        return result;
      }
      try {
        p = new ResourceType(param_);
      } catch (const std::exception &e) {
        HAYAKU_THROW_EXCEPTION(CreateResourceException,
                               "Failed create a new Resource! {}", e.what());
      } catch (...) {
        HAYAKU_THROW_EXCEPTION(CreateResourceException,
                               "Failed create a new Resource! Unknown error!");
      }
      count_++;
      result = ResourcePtr(p, ResourceCloser(this));
      closer_set_.insert(std::get_deleter<ResourceCloser>(result));
      return result;
    }
    p = resource_list_.front();
    resource_list_.pop();
    result = ResourcePtr(p, ResourceCloser(this));
    closer_set_.insert(std::get_deleter<ResourceCloser>(result));
    return result;
  }

  /**
   * Get an available resource within the given timeout
   * @param ms_timeout the timeout in milliseconds
   * @exception GetResourceTimeoutException, CreateResourceException
   */
  ResourcePtr getWaitFor(uint64_t ms_timeout) {  // NOSONAR
    std::unique_lock<std::mutex> lock(mutex_);
    ResourcePtr result;
    ResourceType *p = nullptr;
    if (resource_list_.empty()) {
      if (max_pool_size_ > 0 && count_ >= max_pool_size_) {
        // HAYAKU_TRACE("The maximum number of the resources is exceeded,
        // waiting for an idle resource");
        if (ms_timeout > 0) {
          if (cond_.wait_for(
                  lock, std::chrono::duration<uint64_t, std::milli>(ms_timeout),
                  [&] { return !resource_list_.empty(); })) {
            HAYAKU_CHECK_THROW(!resource_list_.empty(),
                               GetResourceTimeoutException,
                               "Failed get resource!");
          } else {
            HAYAKU_THROW_EXCEPTION(GetResourceTimeoutException,
                                   "Failed get resource!");
          }
        } else {
          cond_.wait(lock, [this] { return !resource_list_.empty(); });
        }
      } else {
        try {
          p = new ResourceType(param_);
        } catch (const std::exception &e) {
          HAYAKU_THROW_EXCEPTION(CreateResourceException,
                                 "Failed create a new Resource! {}", e.what());
        } catch (...) {
          HAYAKU_THROW_EXCEPTION(
              CreateResourceException,
              "Failed create a new Resource! Unknown error!");
        }
        count_++;
        result = ResourcePtr(p, ResourceCloser(this));
        closer_set_.insert(std::get_deleter<ResourceCloser>(result));
        return result;
      }
    }
    p = resource_list_.front();
    resource_list_.pop();
    result = ResourcePtr(p, ResourceCloser(this));
    closer_set_.insert(std::get_deleter<ResourceCloser>(result));
    return result;
  }

  /**
   * Get an available resource; it blocks and waits until an idle resource is
   * obtained when the maximum number of the resources allowed is exceeded
   * @exception CreateResourceException the new resource creation may throw an
   * exception
   */
  ResourcePtr getAndWait() { return getWaitFor(0); }

  /** The number of the currently active resources, i.e. all the resources
   * (including the idle and the used ones) */
  size_t count() const { return count_; }

  /** The current number of the idle resources */
  size_t idleCount() const { return resource_list_.size(); }

  /** Release all the currently idle resources */
  void releaseIdleResource() {
    std::lock_guard<std::mutex> lock(mutex_);
    _releaseIdleResourceNoLock();
  }

 private:
  void _releaseIdleResourceNoLock() {
    while (!resource_list_.empty()) {
      ResourceType *p = resource_list_.front();
      resource_list_.pop();
      count_--;
      if (p) {
        delete p;
      }
    }
  }

 private:
  size_t max_pool_size_;  // The maximum number of the shared resources allowed
  size_t max_idel_size_;  // The maximum number of the idle resources allowed
  size_t count_;        // The number of the currently active resources
  Parameter param_;
  std::mutex mutex_;
  std::condition_variable cond_;
  std::queue<ResourceType *> resource_list_;

  class ResourceCloser {
   public:
    explicit ResourceCloser(ResourcePool *pool) : pool_(pool) {  // NOSONAR
    }

    void operator()(ResourceType *conn) {  // NOSONAR
      if (conn) {
        // If the pool is bound, the resource is returned; otherwise it is
        // deleted
        if (pool_) {
          // HAYAKU_DEBUG("retuan to pool");
          pool_->returnResource(conn, this);
        } else {
          // HAYAKU_DEBUG("delete resource not in pool");
          delete conn;
        }
      }
    }

    // Unbind the resource pool
    void unbind() { pool_ = nullptr; }

   private:
    ResourcePool *pool_;
  };

  /** Return it to the resource pool */
  void returnResource(ResourceType *p, ResourceCloser *closer) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (p) {
      if (resource_list_.size() < max_idel_size_) {
        resource_list_.push(p);
        cond_.notify_all();
      } else {
        delete p;
        count_--;
      }
    } else {
      count_--;
      // HAYAKU_WARN("Trying to return an empty pointer!");
    }
    if (closer) {
      closer_set_.erase(closer);  // Remove this closer
    }
  }

  std::unordered_set<ResourceCloser *>
      closer_set_;  // The closers occupying the resources
};

/**
 * @brief Versioned resource pool (the resource type is required to support the
 * version interfaces)
 * @details The resource class is required to have the two interface functions
 * int getVersion() and void setVersion(int). When the parameters change, the
 * version number is increased automatically and all the idle old version
 * resources are released.
 *
 *          **Important constraint**: ResourceType must implement the
 * getVersion() and setVersion(int) methods.
 *
 * @tparam ResourceType the resource type, it must implement the getVersion()
 * and setVersion(int) methods
 * @ingroup Utilities
 */
template <typename ResourceType>
class ResourceVersionPool {
 public:
  // Compile-time check: ResourceType must support getVersion and setVersion
  static_assert(hayaku::detail::has_resource_getVersion_v<ResourceType>,
                "ResourceType must implement getVersion() method.");
  static_assert(hayaku::detail::has_resource_setVersion_v<ResourceType>,
                "ResourceType must implement setVersion(int) method.");

  ResourceVersionPool() = delete;
  ResourceVersionPool(const ResourceVersionPool &) = delete;
  ResourceVersionPool &operator=(const ResourceVersionPool &) = delete;

  /**
   * Constructor
   * @param param connection parameters
   * @param maxPoolSize the maximum number of the shared resources allowed, 0
   * means unlimited
   * @param maxIdleNum the maximum number of the idle resources allowed; 0 means
   * releasing immediately after use without a cache
   */
  explicit ResourceVersionPool(const Parameter &param, size_t maxPoolSize = 0,
                               size_t maxIdleNum = 100)
      : max_pool_size_(maxPoolSize),
        max_idel_size_(maxIdleNum),
        count_(0),
        param_(param),
        version_(0) {}

  /**
   * Destructor, it releases all the cached resources
   */
  virtual ~ResourceVersionPool() {
    std::unique_lock<std::mutex> lock(mutex_);

    // Unbind the closer of all the allocated resources from the pool
    for (auto iter = closer_set_.begin(); iter != closer_set_.end(); ++iter) {
      (*iter)->unbind();
    }

    // Delete all the idle resources
    while (!resource_list_.empty()) {
      ResourceType *p = resource_list_.front();
      resource_list_.pop();
      if (p) {
        delete p;
      }
    }
  }

  /** Get the current maximum number of the resources allowed */
  size_t maxPoolSize() const { return max_idel_size_; }

  /** Get the current maximum number of the idle resources allowed */
  size_t maxIdleSize() const { return max_idel_size_; }

  /** Set the maximum number of the resources */
  void maxPoolSize(size_t num) {
    std::lock_guard<std::mutex> lock(mutex_);
    max_pool_size_ = num;
  }

  /** Set the maximum number of the idle resources allowed */
  void maxIdleSize(size_t num) {
    std::lock_guard<std::mutex> lock(mutex_);
    max_idel_size_ = num;
  }

  /** Whether the given parameter exists */
  bool haveParam(const std::string &name) {
    std::lock_guard<std::mutex> lock(mutex_);
    return param_.have(name);
  }

  /** Get the value of the given parameter; an exception is thrown when the
   * parameter does not exist or the type does not match */
  template <typename ValueType>
  ValueType getParam(const std::string &name) {
    std::lock_guard<std::mutex> lock(mutex_);
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
    std::lock_guard<std::mutex> lock(mutex_);
    // If the parameter has not actually changed, return directly
    HAYAKU_IF_RETURN(
        param_.have(name) && value == param_.get<ValueType>(name), void());
    param_.set<ValueType>(name, value);
    version_++;
    _releaseIdleResourceNoLock();  // Release the current idle resources so that
                                   // the new parameter values take effect
  }

  /**
   * @brief Set the resource parameters; they take effect only when a new
   * resource is created
   * @param param the parameter object
   */
  void setParameter(const Parameter &param) {
    std::lock_guard<std::mutex> lock(mutex_);
    param_ = param;
    version_++;
    _releaseIdleResourceNoLock();  // Release the current idle resources so that
                                   // the new parameter values take effect
  }

  /**
   * @brief Set the resource parameters; they take effect only when a new
   * resource is created
   * @param param the parameter object
   */
  void setParameter(Parameter &&param) {
    std::lock_guard<std::mutex> lock(mutex_);
    param_ = std::move(param);
    version_++;
    _releaseIdleResourceNoLock();  // Release the current idle resources so that
                                   // the new parameter values take effect
  }

  /** Get the current version of the resource pool */
  int getVersion() {
    std::lock_guard<std::mutex> lock(mutex_);
    return version_;
  }

  /** Increase the current version of the resource pool, equivalent to notifying
   * the resource pool that the resource version has changed */
  void incVersion(int version) {
    std::lock_guard<std::mutex> lock(mutex_);
    version_++;
  }

  /** Resource instance pointer type */
  typedef std::shared_ptr<ResourceType> ResourcePtr;

  /**
   * Get an available resource; a null pointer is returned when the maximum
   * number of the resources allowed is exceeded
   * @exception CreateResourceException the new resource creation may throw an
   * exception
   */
  ResourcePtr get() {
    std::lock_guard<std::mutex> lock(mutex_);
    ResourcePtr result;
    ResourceType *p = nullptr;
    if (resource_list_.empty()) {
      if (max_pool_size_ > 0 && count_ >= max_pool_size_) {
        return result;
      }
      try {
        p = new ResourceType(param_);
        p->setVersion(version_);
      } catch (const std::exception &e) {
        HAYAKU_THROW_EXCEPTION(CreateResourceException,
                               "Failed create a new Resource! {}", e.what());
      } catch (...) {
        HAYAKU_THROW_EXCEPTION(CreateResourceException,
                               "Failed create a new Resource! Unknown error!");
      }
      count_++;
      result = ResourcePtr(p, ResourceCloser(this));
      closer_set_.insert(std::get_deleter<ResourceCloser>(result));
      return result;
    }
    p = resource_list_.front();
    resource_list_.pop();
    result = ResourcePtr(p, ResourceCloser(this));
    closer_set_.insert(std::get_deleter<ResourceCloser>(result));
    return result;
  }

  /**
   * Get an available resource within the given timeout
   * @param ms_timeout the timeout in milliseconds
   * @exception GetResourceTimeoutException, CreateResourceException
   */
  ResourcePtr getWaitFor(uint64_t ms_timeout) {  // NOSONAR
    std::unique_lock<std::mutex> lock(mutex_);
    ResourcePtr result;
    ResourceType *p = nullptr;
    if (resource_list_.empty()) {
      if (max_pool_size_ > 0 && count_ >= max_pool_size_) {
        // HAYAKU_TRACE("The maximum number of the resources is exceeded,
        // waiting for an idle resource");
        if (ms_timeout > 0) {
          if (cond_.wait_for(
                  lock, std::chrono::duration<uint64_t, std::milli>(ms_timeout),
                  [&] { return !resource_list_.empty(); })) {
            HAYAKU_CHECK_THROW(!resource_list_.empty(),
                               GetResourceTimeoutException,
                               "Failed get resource!");
          } else {
            HAYAKU_THROW_EXCEPTION(GetResourceTimeoutException,
                                   "Failed get resource!");
          }
        } else {
          cond_.wait(lock, [this] { return !resource_list_.empty(); });
        }
      } else {
        try {
          p = new ResourceType(param_);
          p->setVersion(version_);
        } catch (const std::exception &e) {
          HAYAKU_THROW_EXCEPTION(CreateResourceException,
                                 "Failed create a new Resource! {}", e.what());
        } catch (...) {
          HAYAKU_THROW_EXCEPTION(
              CreateResourceException,
              "Failed create a new Resource! Unknown error!");
        }
        count_++;
        result = ResourcePtr(p, ResourceCloser(this));
        closer_set_.insert(std::get_deleter<ResourceCloser>(result));
        return result;
      }
    }
    p = resource_list_.front();
    resource_list_.pop();
    result = ResourcePtr(p, ResourceCloser(this));
    closer_set_.insert(std::get_deleter<ResourceCloser>(result));
    return result;
  }

  /**
   * Get an available resource; it blocks and waits until an idle resource is
   * obtained when the maximum number of the resources allowed is exceeded
   * @exception CreateResourceException the new resource creation may throw an
   * exception
   */
  ResourcePtr getAndWait() { return getWaitFor(0); }

  /** The number of the currently active resources, i.e. all the resources
   * (including the idle and the used ones) */
  size_t count() const { return count_; }

  /** The current number of the idle resources */
  size_t idleCount() const { return resource_list_.size(); }

  /** Release all the currently idle resources */
  void releaseIdleResource() {
    std::lock_guard<std::mutex> lock(mutex_);
    _releaseIdleResourceNoLock();
  }

 private:
  void _releaseIdleResourceNoLock() {
    while (!resource_list_.empty()) {
      ResourceType *p = resource_list_.front();
      resource_list_.pop();
      count_--;
      if (p) {
        delete p;
      }
    }
  }

 private:
  size_t max_pool_size_;  // The maximum number of the shared resources allowed
  size_t max_idel_size_;  // The maximum number of the idle resources allowed
  size_t count_;        // The number of the currently active resources
  Parameter param_;
  std::mutex mutex_;
  std::condition_variable cond_;
  std::queue<ResourceType *> resource_list_;
  int version_;

  class ResourceCloser {
   public:
    explicit ResourceCloser(ResourceVersionPool *pool)
        : pool_(pool) {  // NOSONAR
    }

    void operator()(ResourceType *conn) {  // NOSONAR
      if (conn) {
        // If the pool is bound, the resource is returned; otherwise it is
        // deleted
        if (pool_) {
          // HAYAKU_DEBUG("retuan to pool");
          pool_->returnResource(conn, this);
        } else {
          // HAYAKU_DEBUG("delete resource not in pool");
          delete conn;
        }
      }
    }

    // Unbind the resource pool
    void unbind() { pool_ = nullptr; }

   private:
    ResourceVersionPool *pool_;
  };

  /** Return it to the resource pool */
  void returnResource(ResourceType *p, ResourceCloser *closer) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (p) {
      // When the version of the currently returned resource equals the resource
      // pool version and the idle resource list is less than the maximum number
      // of the idle resources, the returned resource is accepted
      if (p->getVersion() == version_ &&
          resource_list_.size() < max_idel_size_) {
        resource_list_.push(p);
        cond_.notify_all();
      } else {
        delete p;
        count_--;
      }
    } else {
      count_--;
      // HAYAKU_WARN("Trying to return an empty pointer!");
    }
    if (closer) {
      closer_set_.erase(closer);  // Remove this closer
    }
  }

  std::unordered_set<ResourceCloser *>
      closer_set_;  // The closers occupying the resources
};

}  // namespace hayaku
