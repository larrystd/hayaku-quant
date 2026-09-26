#pragma once

/*
 * AutoTransAction.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-7-11
 *      Author: fasiondog
 */

#include "DBConnectBase.h"

namespace hayaku {

/**
 * Automatic transaction handling: it starts a transaction automatically in the
 * code block and commits it automatically after the code block exits
 * @note When there are multiple data changes, an exception thrown in the middle
 * of the program processing may cause the data to be partially committed
 * @ingroup DBConnect
 */
class AutoTransAction {
 public:
  /**
   * Constructor
   * @param driver the database connection pointer
   */
  explicit AutoTransAction(const DBConnectPtr& driver) : driver_(driver) {
    driver_->transaction();
  }

  /** Destructor */
  ~AutoTransAction() {
    try {
      driver_->commit();
    } catch (...) {
      HAYAKU_ERROR("Transaction commit failed!");
      driver_->rollback();
      driver_.reset();
    }
  }

  /** Get the database connection */
  const DBConnectPtr& connect() const { return driver_; }

 private:
  AutoTransAction() = delete;
  AutoTransAction(const AutoTransAction&) = delete;
  AutoTransAction& operator=(const AutoTransAction&) = delete;

 private:
  DBConnectPtr driver_;
};

/**
 * Manual transaction handling; it allows the nested starting of the transaction
 * and requires a manual start and commit; on the automatic exit it does not
 * commit automatically but rolls back!
 * @details There must be one effective manual transaction start; multiple
 * nested starts are regarded as one transaction handling. After a manual
 * commit, the transaction must be started manually again if there is new
 * transaction handling.
 * @note When nested, it may happen that the inner transaction has been
 * committed but the outer transaction handling fails and is rolled back (only
 * the part of the corresponding transaction handling is rolled back)
 * @ingroup DBConnect
 */
class TransAction {
 public:
  /**
   * Constructor
   * @param driver the database connection pointer
   */
  explicit TransAction(const DBConnectPtr& driver)
      : driver_(driver), committed_(false), started_(true) {
    HAYAKU_CHECK(driver, "Null DBConnectPtr!");
    driver_->transaction();
  }

  /** Destructor */
  ~TransAction() {
    // If the transaction has not been committed actively it is regarded as
    // needing a rollback
    if (started_ && !committed_) {
      HAYAKU_WARN("The transaction is rolled back!");
      driver_->rollback();
    } else if (!committed_) {
      HAYAKU_WARN("Not manul begin transaction!");
    }
  }

  /** Get the database connection */
  const DBConnectPtr& connect() const { return driver_; }

  /** Start the transaction */
  void begin() {
    if (!started_) {
      driver_->transaction();
      committed_ = false;
      started_ = true;
    }
  }

  /** End and commit the transaction */
  void end() {
    HAYAKU_CHECK(started_, "No transaction has started!");
    if (!committed_) {
      driver_->commit();
      committed_ = true;
      started_ = false;
    }
  }

 private:
  TransAction() = delete;
  TransAction(const AutoTransAction&) = delete;
  TransAction& operator=(const AutoTransAction&) = delete;

 private:
  DBConnectPtr driver_;
  bool committed_;
  bool started_;
};

}  // namespace hayaku
