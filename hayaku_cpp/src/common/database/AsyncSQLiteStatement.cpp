/*
 * AsyncSQLiteStatement.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2026-05-09
 *      Author: fasiondog
 */

#include "AsyncSQLiteStatement.h"

#include <sqlite3.h>

#include "AsyncSQLiteConnect.h"
#include "common/Log.h"
#include "common/concurrency/ParallelAlgorithms.h"

namespace hayaku {

// The Pimpl implementation struct
struct AsyncSQLiteStatement::Impl {
  sqlite3 *db_ = nullptr;
  sqlite3_stmt *stmt_ = nullptr;
  bool needs_reset_ = false;
  int step_status_ = SQLITE_DONE;
  bool at_first_step_ = true;
  AsyncSQLiteConnect *connect_ =
      nullptr;  // Hold the connection pointer to get the thread pool executor

  Impl(AsyncSQLiteConnect *connect, sqlite3 *db, sqlite3_stmt *stmt)
      : db_(db), stmt_(stmt), connect_(connect) {}

  ~Impl() {
    if (stmt_) {
      sqlite3_finalize(stmt_);
    }
  }

  void reset() {
    if (needs_reset_) {
      int status = sqlite3_reset(stmt_);
      if (status != SQLITE_OK) {
        step_status_ = SQLITE_DONE;
        SQL_THROW(status, "{}", sqlite3_errmsg(db_));
      }
      needs_reset_ = false;
      step_status_ = SQLITE_DONE;
      at_first_step_ = true;
    }
  }

  // Get the thread pool executor from the connection
  ThreadPool::ExecutorWrapper getExecutor() const {
    return connect_->getThreadPoolExecutor();
  }
};

AsyncSQLiteStatement::AsyncSQLiteStatement(AsyncSQLiteConnect *connect,
                                           const std::string &sql)
    : AsyncSQLStatementBase(connect, sql), impl_(nullptr) {
  HAYAKU_CHECK(connect != nullptr, "Invalid AsyncSQLiteConnect");

  // Make sure the connection is initialized (a synchronous operation)
  connect->_connect();

  // Prepare the statement in the constructor (a synchronous operation, because
  // it is a local memory operation only)
  auto *raw_conn = connect->getRawConnection();
  sqlite3 *db = static_cast<sqlite3 *>(raw_conn);

  sqlite3_stmt *stmt = nullptr;
  int status = sqlite3_prepare_v2(
      db, sql.c_str(), static_cast<int>(sql.size() + 1), &stmt, nullptr);
  if (status != SQLITE_OK) {
    if (stmt) {
      sqlite3_finalize(stmt);
    }
    SQL_THROW(status, "Failed prepare sql statement: {}! error msg: {}", sql,
              sqlite3_errmsg(db));
  }

  HAYAKU_CHECK(stmt != nullptr, "Invalid SQL statement: {}", sql);

  impl_ = std::make_unique<Impl>(connect, db, stmt);
}

AsyncSQLiteStatement::~AsyncSQLiteStatement() {
  // m_impl cleans up the sqlite3_stmt automatically
}

void AsyncSQLiteStatement::_reset() {
  if (impl_) {
    impl_->reset();
  }
}

net::awaitable<void> AsyncSQLiteStatement::sub_exec() {
  if (!impl_) {
    throw exception("AsyncSQLiteStatement is not initialized");
  }

  // Merge reset and step into a single co_run
  auto exec_func = [this]() -> int {
    // 1. Reset the statement
    if (impl_->needs_reset_) {
      int status = sqlite3_reset(impl_->stmt_);
      if (status != SQLITE_OK) {
        return status;
      }
      impl_->needs_reset_ = false;
      impl_->step_status_ = SQLITE_DONE;
      impl_->at_first_step_ = true;
    }

    // 2. Execute the first step
    impl_->step_status_ = sqlite3_step(impl_->stmt_);
    impl_->needs_reset_ = true;

    if (impl_->step_status_ != SQLITE_DONE &&
        impl_->step_status_ != SQLITE_ROW) {
      return impl_->step_status_;
    }
    return SQLITE_OK;
  };

  int status = co_await co_run(impl_->getExecutor(), exec_func);

  if (status != SQLITE_OK) {
    SQL_THROW(status, "{}", sqlite3_errmsg(impl_->db_));
  }
  co_return;
}

net::awaitable<bool> AsyncSQLiteStatement::sub_moveNext() {
  if (!impl_) {
    throw exception("AsyncSQLiteStatement is not initialized");
  }

  // moveNext is a local state check and can return synchronously directly
  if (impl_->step_status_ == SQLITE_ROW) {
    if (impl_->at_first_step_) {
      impl_->at_first_step_ = false;
      co_return true;
    } else {
      // sqlite3_step needs to be executed, this is an I/O operation
      auto step_func = [this]() -> int {
        impl_->step_status_ = sqlite3_step(impl_->stmt_);
        return impl_->step_status_;
      };

      int status = co_await co_run(impl_->getExecutor(), step_func);

      if (status == SQLITE_DONE) {
        co_return false;
      } else if (status == SQLITE_ROW) {
        co_return true;
      } else {
        SQL_THROW(status, "{}", sqlite3_errmsg(impl_->db_));
      }
    }
  } else {
    co_return false;
  }
}

uint64_t AsyncSQLiteStatement::sub_getLastRowid() {
  if (!impl_) {
    throw exception("AsyncSQLiteStatement is not initialized");
  }
  return sqlite3_last_insert_rowid(impl_->db_);
}

int AsyncSQLiteStatement::sub_getNumColumns() const {
  if (!impl_) {
    return 0;
  }
  return (impl_->at_first_step_ == false) &&
                 (impl_->step_status_ == SQLITE_ROW)
             ? sqlite3_column_count(impl_->stmt_)
             : 0;
}

void AsyncSQLiteStatement::sub_bindNull(int idx) {
  if (!impl_) {
    throw exception("AsyncSQLiteStatement is not initialized");
  }
  _reset();
  int status = sqlite3_bind_null(impl_->stmt_, idx + 1);
  SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(impl_->db_));
}

void AsyncSQLiteStatement::sub_bindInt(int idx, int64_t value) {
  if (!impl_) {
    throw exception("AsyncSQLiteStatement is not initialized");
  }
  _reset();
  int status = sqlite3_bind_int64(impl_->stmt_, idx + 1, value);
  SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(impl_->db_));
}

void AsyncSQLiteStatement::sub_bindDouble(int idx, double item) {
  if (!impl_) {
    throw exception("AsyncSQLiteStatement is not initialized");
  }
  _reset();
  int status = sqlite3_bind_double(impl_->stmt_, idx + 1, item);
  SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(impl_->db_));
}

void AsyncSQLiteStatement::sub_bindDatetime(int idx, const Datetime &item) {
  if (item == Null<Datetime>()) {
    sub_bindNull(idx);
  } else {
    sub_bindText(idx, item.str());
  }
}

void AsyncSQLiteStatement::sub_bindText(int idx, const std::string &item) {
  if (!impl_) {
    throw exception("AsyncSQLiteStatement is not initialized");
  }
  _reset();
  int status =
      sqlite3_bind_text(impl_->stmt_, idx + 1, item.c_str(),
                        static_cast<int>(item.size()), SQLITE_TRANSIENT);
  SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(impl_->db_));
}

void AsyncSQLiteStatement::sub_bindText(int idx, const char *item, size_t len) {
  if (!impl_) {
    throw exception("AsyncSQLiteStatement is not initialized");
  }
  _reset();
  int status = sqlite3_bind_text(impl_->stmt_, idx + 1, item,
                                 static_cast<int>(len), SQLITE_TRANSIENT);
  SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(impl_->db_));
}

void AsyncSQLiteStatement::sub_bindBlob(int idx, const std::string &item) {
  if (!impl_) {
    throw exception("AsyncSQLiteStatement is not initialized");
  }
  _reset();
  int status =
      sqlite3_bind_blob(impl_->stmt_, idx + 1, item.data(),
                        static_cast<int>(item.size()), SQLITE_TRANSIENT);
  SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(impl_->db_));
}

void AsyncSQLiteStatement::sub_bindBlob(int idx,
                                        const std::vector<char> &item) {
  if (!impl_) {
    throw exception("AsyncSQLiteStatement is not initialized");
  }
  _reset();
  int status =
      sqlite3_bind_blob(impl_->stmt_, idx + 1, item.data(),
                        static_cast<int>(item.size()), SQLITE_TRANSIENT);
  SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(impl_->db_));
}

void AsyncSQLiteStatement::sub_getColumnAsInt64(int idx, int64_t &item) {
  if (!impl_) {
    throw exception("AsyncSQLiteStatement is not initialized");
  }
  item = sqlite3_column_int64(impl_->stmt_, idx);
}

void AsyncSQLiteStatement::sub_getColumnAsDouble(int idx, double &item) {
  if (!impl_) {
    throw exception("AsyncSQLiteStatement is not initialized");
  }
  item = sqlite3_column_double(impl_->stmt_, idx);
}

void AsyncSQLiteStatement::sub_getColumnAsDatetime(int idx, Datetime &item) {
  std::string date_str;
  sub_getColumnAsText(idx, date_str);
  item = date_str.empty() ? Datetime() : Datetime(date_str);
}

void AsyncSQLiteStatement::sub_getColumnAsText(int idx, std::string &item) {
  if (!impl_) {
    throw exception("AsyncSQLiteStatement is not initialized");
  }
  const char *data =
      reinterpret_cast<const char *>(sqlite3_column_text(impl_->stmt_, idx));
  item = (data != nullptr) ? std::string(data) : std::string();
}

void AsyncSQLiteStatement::sub_getColumnAsBlob(int idx, std::string &item) {
  if (!impl_) {
    throw exception("AsyncSQLiteStatement is not initialized");
  }
  const char *data =
      static_cast<const char *>(sqlite3_column_blob(impl_->stmt_, idx));
  if (data == nullptr) {
    throw null_blob_exception();
  }
  const int size = sqlite3_column_bytes(impl_->stmt_, idx);
  item = std::string(data, size);
}

void AsyncSQLiteStatement::sub_getColumnAsBlob(int idx,
                                               std::vector<char> &item) {
  if (!impl_) {
    throw exception("AsyncSQLiteStatement is not initialized");
  }
  const char *data =
      static_cast<const char *>(sqlite3_column_blob(impl_->stmt_, idx));
  if (data == nullptr) {
    throw null_blob_exception();
  }
  const int size = sqlite3_column_bytes(impl_->stmt_, idx);
  item.resize(size);
  memcpy(item.data(), data, size);
}

}  // namespace hayaku
