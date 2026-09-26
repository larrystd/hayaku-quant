#pragma once

/*
 * AsyncTransAction.h
 *
 *  Copyright (c) 2026, hikyuu.org
 *
 *  Created on: 2026-05-09
 *      Author: fasiondog
 */

#include <memory>

#include "AsyncDBConnectBase.h"

namespace hayaku {

/**
 * Asynchronous automatic transaction handling: it starts a transaction
 * automatically in the code block and commits it automatically after the code
 * block exits
 * @note When there are multiple data changes, an exception thrown in the middle
 * of the program processing may cause the data to be partially committed
 * @details It rolls back automatically when commit() fails, and rolls back
 * automatically in the destructor if it has not been committed (using a
 * detached coroutine)
 * @ingroup DBConnect
 */
class AsyncAutoTransAction final {
 public:
  AsyncAutoTransAction() = delete;
  AsyncAutoTransAction(const AsyncAutoTransAction&) = delete;
  AsyncAutoTransAction& operator=(const AsyncAutoTransAction&) = delete;
  AsyncAutoTransAction(AsyncAutoTransAction&&) = delete;
  AsyncAutoTransAction& operator=(AsyncAutoTransAction&&) = delete;

  /**
   * Factory method: create an instance and start the transaction automatically
   * @param driver the database connection pointer
   * @return asynchronous transaction object
   */
  static net::awaitable<std::shared_ptr<AsyncAutoTransAction>> create(
      const AsyncDBConnectPtr& driver) {
    auto action =
        std::shared_ptr<AsyncAutoTransAction>(new AsyncAutoTransAction(driver));
    co_await action->startTransaction();
    co_return action;
  }

  /** Get the database connection */
  const AsyncDBConnectPtr& connect() const { return driver_; }

  /** Destructor: it rolls back automatically if it has not been committed */
  ~AsyncAutoTransAction() {
    if (!committed_ && driver_ && io_context_) {
      // Start a detached coroutine to roll back (fire and forget)
      net::asio::co_spawn(
          *io_context_,
          [driver = driver_]() -> net::awaitable<void> {
            try {
              co_await driver->commit();
              co_return;
            } catch (const std::exception& e) {
              HAYAKU_ERROR("Failed to commit transaction! {}", e.what());
            } catch (...) {
              HAYAKU_ERROR("Failed to commit! Unknown exception!");
            }
            try {
              co_await driver->rollback();
              HAYAKU_INFO("Transaction rolled back successfully!");
            } catch (...) {
              HAYAKU_ERROR(
                  "Failed to rollback transaction! Unknown exception!");
            }
          },
          net::asio::detached);
    }
  }

 private:
  /* Private constructor */
  explicit AsyncAutoTransAction(const AsyncDBConnectPtr& driver)
      : driver_(driver), io_context_(nullptr), committed_(false) {
    HAYAKU_CHECK(driver_, "Null AsyncDBConnectPtr!");
  }

  /* Internal method: start the transaction */
  net::awaitable<void> startTransaction() {
    // Get the io_context of the current coroutine environment
    auto exec = co_await net::this_coro::executor;
    io_context_ = &static_cast<boost::asio::io_context&>(exec.context());

    // Start the transaction
    co_await driver_->transaction();
  }

 private:
  AsyncDBConnectPtr driver_;
  boost::asio::io_context* io_context_ = nullptr;
  bool committed_ = false;
};

/**
 * Asynchronous manual transaction handling; it allows the nested starting of
 * the transaction and requires a manual start and commit
 * @details There must be one effective manual transaction start; multiple
 * nested starts are regarded as one transaction handling. After a manual
 * commit, the transaction must be started manually again if there is new
 * transaction handling.
 * @note If it has been started but not committed at the destruction, it rolls
 * back automatically (using a detached coroutine)
 * @ingroup DBConnect
 */
class AsyncTransAction final {
 public:
  AsyncTransAction() = delete;
  AsyncTransAction(const AsyncTransAction&) = delete;
  AsyncTransAction& operator=(const AsyncTransAction&) = delete;
  AsyncTransAction(AsyncTransAction&&) = delete;
  AsyncTransAction& operator=(AsyncTransAction&&) = delete;

  /**
   * Factory method: create an instance and start the transaction automatically
   * @param driver the database connection pointer
   * @return asynchronous transaction object
   */
  static net::awaitable<std::shared_ptr<AsyncTransAction>> create(
      const AsyncDBConnectPtr& driver) {
    auto action =
        std::shared_ptr<AsyncTransAction>(new AsyncTransAction(driver));
    co_await action->begin();
    co_return action;
  }

  /** Get the database connection */
  const AsyncDBConnectPtr& connect() const { return driver_; }

  /**
   * Start the transaction (the nesting is supported)
   * @note It is not started repeatedly if it has already been started
   */
  net::awaitable<void> begin() {
    if (!started_) {
      // Get the io_context of the current coroutine environment
      auto exec = co_await net::this_coro::executor;
      io_context_ = &static_cast<boost::asio::io_context&>(exec.context());

      // Start the transaction
      co_await driver_->transaction();
      started_ = true;
      committed_ = false;
    }
    co_return;
  }

  /**
   * End and commit the transaction
   * @note begin() must be called first to start the transaction
   */
  net::awaitable<void> end() {
    HAYAKU_CHECK(started_, "No transaction has started!");
    if (!committed_) {
      co_await driver_->commit();
      committed_ = true;
      started_ = false;
    }
    co_return;
  }

  /** Roll back the transaction */
  net::awaitable<void> rollback() {
    if (started_ && !committed_) {
      co_await driver_->rollback();
      started_ = false;
      committed_ = false;
    }
    co_return;
  }

  /** Destructor: it rolls back automatically if it has been started but not
   * committed */
  ~AsyncTransAction() {
    // If the transaction has not been committed actively it is regarded as
    // needing a rollback
    if (started_ && !committed_ && driver_ && io_context_) {
      HAYAKU_WARN(
          "AsyncTransAction: The transaction is rolled back in destructor!");

      // Start a detached coroutine to roll back (fire and forget)
      boost::asio::co_spawn(
          *io_context_,
          [driver = driver_]() -> net::awaitable<void> {
            try {
              co_await driver->rollback();
            } catch (const std::exception& e) {
              HAYAKU_WARN("Failed to rollback transaction! {}", e.what());
            } catch (...) {
              HAYAKU_WARN("Failed to rollback transaction! Unknown exception!");
            }
          },
          boost::asio::detached);
    }
  }

 private:
  /** Private constructor */
  explicit AsyncTransAction(const AsyncDBConnectPtr& driver)
      : driver_(driver),
        io_context_(nullptr),
        committed_(false),
        started_(false) {
    HAYAKU_CHECK(driver_, "Null AsyncDBConnectPtr!");
  }

 private:
  AsyncDBConnectPtr driver_;
  net::asio::io_context* io_context_ = nullptr;
  bool committed_ = false;
  bool started_ = false;
};

}  // namespace hayaku
