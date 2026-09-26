/*
 * SQLiteStatement.cpp
 *
 *  Copyright (c) 2019, hiyuu.org
 *
 *  Created on: 2019-7-11
 *      Author: fasiondog
 */

#include "SQLiteStatement.h"

#include "SQLiteConnect.h"

namespace hayaku {

SQLiteStatement::SQLiteStatement(DBConnectBase *driver,
                                 const std::string &sql_statement)
    : SQLStatementBase(driver, sql_statement),
      needs_reset_(false),
      step_status_(SQLITE_DONE),
      at_first_step_(true),
      db_((dynamic_cast<SQLiteConnect *>(driver))->db_),
      stmt_(NULL) {
  int status = sqlite3_prepare_v2(db_, sql_string_.c_str(),
                                  int(sql_string_.size() + 1), &stmt_, NULL);
  if (status != SQLITE_OK) {
    sqlite3_finalize(stmt_);
    SQL_THROW(status, "Failed prepare sql statement: {}! error msg: {}",
              sql_string_, sqlite3_errmsg(db_));
  }

  HAYAKU_CHECK(stmt_ != 0, "Invalid SQL statement: {}", sql_string_);
}

SQLiteStatement::~SQLiteStatement() {
  sqlite3_finalize(stmt_);
  // m_db comes from Connect and its lifetime is managed by Connect, so it must
  // not be released here
}

void SQLiteStatement::_reset() {
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

void SQLiteStatement::sub_exec() {
  _reset();
  step_status_ = sqlite3_step(stmt_);
  needs_reset_ = true;
  if (step_status_ != SQLITE_DONE && step_status_ != SQLITE_ROW) {
    SQL_THROW(step_status_, "{}", sqlite3_errmsg(db_));
  }
}

bool SQLiteStatement::sub_moveNext() {
  if (step_status_ == SQLITE_ROW) {
    if (at_first_step_) {
      at_first_step_ = false;
      return true;
    } else {
      step_status_ = sqlite3_step(stmt_);
      if (step_status_ == SQLITE_DONE) {
        return false;
      } else if (step_status_ == SQLITE_ROW) {
        return true;
      } else {
        SQL_THROW(step_status_, "{}", sqlite3_errmsg(db_));
      }
    }
  } else {
    return false;
  }
}

int SQLiteStatement::sub_getNumColumns() const {
  return (at_first_step_ == false) && (step_status_ == SQLITE_ROW)
             ? sqlite3_column_count(stmt_)
             : 0;
}

void SQLiteStatement::sub_bindNull(int idx) {
  _reset();
  int status = sqlite3_bind_null(stmt_, idx + 1);
  SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(db_));
}

void SQLiteStatement::sub_bindInt(int idx, int64_t value) {
  _reset();
  int status = sqlite3_bind_int64(stmt_, idx + 1, value);
  SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(db_));
}

void SQLiteStatement::sub_bindDatetime(int idx, const Datetime &item) {
  if (item == Null<Datetime>()) {
    sub_bindNull(idx);
  } else {
    sub_bindText(idx, item.str());
  }
}

void SQLiteStatement::sub_bindText(int idx, const std::string &item) {
  _reset();
  int status = sqlite3_bind_text(stmt_, idx + 1, item.c_str(),
                                 (int)item.size(), SQLITE_TRANSIENT);
  SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(db_));
}

void SQLiteStatement::sub_bindText(int idx, const char *item, size_t len) {
  _reset();
  int status =
      sqlite3_bind_text(stmt_, idx + 1, item, (int)len, SQLITE_TRANSIENT);
  SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(db_));
}

void SQLiteStatement::sub_bindDouble(int idx, double item) {
  _reset();
  int status = sqlite3_bind_double(stmt_, idx + 1, item);
  SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(db_));
}

void SQLiteStatement::sub_bindBlob(int idx, const std::string &item) {
  _reset();
  int status = sqlite3_bind_blob(stmt_, idx + 1, item.data(), (int)item.size(),
                                 SQLITE_TRANSIENT);
  SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(db_));
}

void SQLiteStatement::sub_bindBlob(int idx, const std::vector<char> &item) {
  _reset();
  int status = sqlite3_bind_blob(stmt_, idx + 1, item.data(), (int)item.size(),
                                 SQLITE_TRANSIENT);
  SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(db_));
}

void SQLiteStatement::sub_getColumnAsInt64(int idx, int64_t &item) {
  item = sqlite3_column_int64(stmt_, idx);
}

void SQLiteStatement::sub_getColumnAsDouble(int idx, double &item) {
  item = sqlite3_column_double(stmt_, idx);
}

void SQLiteStatement::sub_getColumnAsDatetime(int idx, Datetime &item) {
  std::string date_str;
  sub_getColumnAsText(idx, date_str);
  item = date_str.empty() ? Datetime() : Datetime(date_str);
}

void SQLiteStatement::sub_getColumnAsText(int idx, std::string &item) {
  const char *data =
      reinterpret_cast<const char *>(sqlite3_column_text(stmt_, idx));
  item = (data != 0) ? std::string(data) : std::string();
}

void SQLiteStatement::sub_getColumnAsBlob(int idx, std::string &item) {
  const char *data =
      static_cast<const char *>(sqlite3_column_blob(stmt_, idx));
  if (data == NULL) {
    throw null_blob_exception();
  }
  const int size = sqlite3_column_bytes(stmt_, idx);
  item = std::string(data, size);
}

void SQLiteStatement::sub_getColumnAsBlob(int idx, std::vector<char> &item) {
  const char *data =
      static_cast<const char *>(sqlite3_column_blob(stmt_, idx));
  if (data == NULL) {
    throw null_blob_exception();
  }
  const int size = sqlite3_column_bytes(stmt_, idx);
  item.resize(size);
  memcpy(item.data(), data, size);
}

uint64_t SQLiteStatement::sub_getLastRowid() {
  return sqlite3_last_insert_rowid(db_);
}

}  // namespace hayaku
