/*
 * MySQLStatement_libmysqlclient.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-8-17
 *      Author: fasiondog
 */

#include <cstring>
#include <vector>

#include "MySQLConnect.h"
#include "MySQLStatement.h"

#if defined(_MSC_VER)
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4267)
#endif

namespace hayaku {

// The Pimpl implementation struct
struct MySQLStatement::Impl {
  MYSQL* db{nullptr};
  MYSQL_STMT* stmt{nullptr};
  MYSQL_RES* meta_result{nullptr};
  bool needs_reset{false};
  bool has_bind_result{false};

  std::vector<MYSQL_BIND> param_bind;
  std::vector<MYSQL_BIND> result_bind;
  std::vector<boost::any> param_buffer;
  std::vector<boost::any> result_buffer;
  std::vector<unsigned long> result_length;
  std::vector<char> result_is_null;
  std::vector<char> result_error;
};

MySQLStatement::MySQLStatement(DBConnectBase* driver,
                               const std::string& sql_statement)
    : SQLStatementBase(driver, sql_statement),
      impl_(std::make_unique<Impl>()) {
  const MySQLConnect* connect = dynamic_cast<MySQLConnect*>(driver);
  SQL_CHECK(connect, -1,
            "Failed create statement: {}! Failed dynamic_cast<MySQLConnect*>!",
            sql_statement);

  impl_->db = static_cast<MYSQL*>(connect->getRawConnection());
  _prepare();

  auto param_count = mysql_stmt_param_count(impl_->stmt);
  if (param_count > 0) {
    impl_->param_bind.resize(param_count);
    memset(impl_->param_bind.data(), 0, param_count * sizeof(MYSQL_BIND));
  }

  impl_->meta_result = mysql_stmt_result_metadata(impl_->stmt);
  if (impl_->meta_result) {
    int column_count = mysql_num_fields(impl_->meta_result);
    impl_->result_bind.resize(column_count);
    memset(impl_->result_bind.data(), 0, column_count * sizeof(MYSQL_BIND));
    impl_->result_length.resize(column_count, 0);
    impl_->result_is_null.resize(column_count, 0);
    impl_->result_error.resize(column_count, 0);
  }
}

MySQLStatement::~MySQLStatement() {
  if (impl_->meta_result) {
    mysql_free_result(impl_->meta_result);
  }
  if (impl_->stmt) {
    mysql_stmt_close(impl_->stmt);
  }
}

void MySQLStatement::_prepare() {
  impl_->stmt = mysql_stmt_init(impl_->db);
  HAYAKU_CHECK(impl_->stmt, "Failed mysql_stmt_init! SQL: {}", sql_string_);

  int ret = mysql_stmt_prepare(impl_->stmt, sql_string_.c_str(),
                               sql_string_.size());
  HAYAKU_IF_RETURN(0 == ret, void());

  mysql_stmt_close(impl_->stmt);
  impl_->stmt = nullptr;

  // On a server exception, try to reconnect to the server
  // 1 is "Lost connection to MySQL server during query", but MYSQL has no error
  // code definition for it
  if (1 == ret || CR_SERVER_LOST == ret || CR_SERVER_GONE_ERROR == ret) {
    MySQLConnect* connect = dynamic_cast<MySQLConnect*>(driver_);
    if (connect && connect->ping()) {
      impl_->db = static_cast<MYSQL*>(connect->getRawConnection());
    } else {
      HAYAKU_THROW("Failed reconnect mysql! SQL: {}", sql_string_);
    }
  } else if (CR_OUT_OF_MEMORY == ret) {
    HAYAKU_THROW("Out of memory! SQL: {}", sql_string_);
  }

  impl_->stmt = mysql_stmt_init(impl_->db);
  ret = mysql_stmt_prepare(impl_->stmt, sql_string_.c_str(),
                           sql_string_.size());
  HAYAKU_IF_RETURN(0 == ret, void());

  std::string stmt_errorstr(mysql_stmt_error(impl_->stmt));
  mysql_stmt_close(impl_->stmt);
  impl_->stmt = nullptr;
  HAYAKU_THROW("Failed prepare statement: {}! ret: {}, error msg: {}!",
               sql_string_, ret, stmt_errorstr);
}

void MySQLStatement::_reset() {
  if (impl_->needs_reset) {
    int ret = mysql_stmt_reset(impl_->stmt);
    SQL_CHECK(ret == 0, ret, "Failed reset statement! {}",
              mysql_stmt_error(impl_->stmt));
    impl_->result_buffer.clear();
    impl_->needs_reset = false;
    impl_->has_bind_result = false;
  }
}

void MySQLStatement::sub_exec() {
  _reset();
  impl_->needs_reset = true;
  int ret = 0;
  if (impl_->param_bind.size() > 0) {
    ret = mysql_stmt_bind_param(impl_->stmt, impl_->param_bind.data());
    SQL_CHECK(ret == 0, ret, "Failed mysql_stmt_bind_param! {}",
              mysql_stmt_error(impl_->stmt));
  }
  ret = mysql_stmt_execute(impl_->stmt);
  SQL_CHECK(ret == 0, ret, "Failed mysql_stmt_execute: {}",
            mysql_stmt_error(impl_->stmt));
}

void MySQLStatement::_bindResult() {
  HAYAKU_IF_RETURN(!impl_->meta_result, void());
  MYSQL_FIELD* field;
  int idx = 0;
  while ((field = mysql_fetch_field(impl_->meta_result))) {
    impl_->result_bind[idx].buffer_type = field->type;
#if MYSQL_VERSION_ID >= 80000
    impl_->result_bind[idx].is_null = (bool*)&impl_->result_is_null[idx];
    impl_->result_bind[idx].error = (bool*)&impl_->result_error[idx];
#else
    impl_->result_bind[idx].is_null = &impl_->result_is_null[idx];
    impl_->result_bind[idx].error = &impl_->result_error[idx];
#endif
    impl_->result_bind[idx].length = &impl_->result_length[idx];

    if (field->type == MYSQL_TYPE_LONGLONG) {
      int64_t item = 0;
      impl_->result_buffer.push_back(item);
      auto& buf = impl_->result_buffer.back();
      impl_->result_bind[idx].buffer = boost::any_cast<int64_t>(&buf);
    } else if (field->type == MYSQL_TYPE_LONG ||
               field->type == MYSQL_TYPE_INT24) {
      int32_t item = 0;
      impl_->result_buffer.push_back(item);
      auto& buf = impl_->result_buffer.back();
      impl_->result_bind[idx].buffer = boost::any_cast<int32_t>(&buf);
    } else if (field->type == MYSQL_TYPE_DOUBLE) {
      double item = 0;
      impl_->result_buffer.push_back(item);
      auto& buf = impl_->result_buffer.back();
      impl_->result_bind[idx].buffer = boost::any_cast<double>(&buf);
    } else if (field->type == MYSQL_TYPE_FLOAT) {
      float item = 0;
      impl_->result_buffer.push_back(item);
      auto& buf = impl_->result_buffer.back();
      impl_->result_bind[idx].buffer = boost::any_cast<float>(&buf);
    } else if (field->type == MYSQL_TYPE_VAR_STRING ||
               field->type == MYSQL_TYPE_STRING ||
               field->type == MYSQL_TYPE_BLOB ||
               field->type == MYSQL_TYPE_TINY_BLOB ||
               field->type == MYSQL_TYPE_VARCHAR ||
               field->type == MYSQL_TYPE_DECIMAL ||
               field->type == MYSQL_TYPE_NEWDECIMAL) {
      unsigned long length = field->length + 1;
      impl_->result_bind[idx].buffer_length = length;
      impl_->result_buffer.emplace_back(std::vector<char>(length));
      auto& buf = impl_->result_buffer.back();
      std::vector<char>* p = boost::any_cast<std::vector<char>>(&buf);
      impl_->result_bind[idx].buffer = p->data();
    } else if (field->type == MYSQL_TYPE_TINY) {
      int8_t item = 0;
      impl_->result_buffer.push_back(item);
      auto& buf = impl_->result_buffer.back();
      impl_->result_bind[idx].buffer = boost::any_cast<int8_t>(&buf);
    } else if (field->type == MYSQL_TYPE_SHORT ||
               field->type == MYSQL_TYPE_YEAR) {
      short item = 0;
      impl_->result_buffer.push_back(item);
      auto& buf = impl_->result_buffer.back();
      impl_->result_bind[idx].buffer_type = MYSQL_TYPE_SHORT;
      impl_->result_bind[idx].buffer = boost::any_cast<short>(&buf);
    } else if (field->type == MYSQL_TYPE_DATETIME ||
               field->type == MYSQL_TYPE_DATE ||
               field->type == MYSQL_TYPE_TIMESTAMP ||
               field->type == MYSQL_TYPE_TIME ||
               field->type == MYSQL_TYPE_TIME2) {
      MYSQL_TIME item;
      memset(&item, 0, sizeof(item));
      impl_->result_buffer.push_back(item);
      auto& buf = impl_->result_buffer.back();
      impl_->result_bind[idx].buffer = boost::any_cast<MYSQL_TIME>(&buf);
    } else {
      HAYAKU_THROW("Unsupport field type: {}, field name: {}", int(field->type),
                   field->name);
    }

    idx++;
  }
}

bool MySQLStatement::sub_moveNext() {
  int ret = 0;
  if (!impl_->has_bind_result) {
    _bindResult();
    impl_->has_bind_result = true;

    ret = mysql_stmt_bind_result(impl_->stmt, impl_->result_bind.data());
    SQL_CHECK(ret == 0, ret, "Failed mysql_stmt_bind_result! {}",
              mysql_stmt_error(impl_->stmt));

    ret = mysql_stmt_store_result(impl_->stmt);
    SQL_CHECK(ret == 0, ret, "Failed mysql_stmt_store_result! {}",
              mysql_stmt_error(impl_->stmt));
  }

  ret = mysql_stmt_fetch(impl_->stmt);
  if (ret == 0) {
    return true;
  } else if (ret == 1) {
    SQL_THROW(ret, "Error occurred in mysql_stmt_fetch! {}",
              mysql_stmt_error(impl_->stmt));
  }
  return false;
}

void MySQLStatement::sub_bindNull(int idx) {
  SQL_CHECK(idx < static_cast<int>(impl_->param_bind.size()), -1,
            "idx out of range! idx: {}, total: {}", idx,
            impl_->param_bind.size());
  impl_->param_bind[idx].buffer_type = MYSQL_TYPE_NULL;
}

void MySQLStatement::sub_bindInt(int idx, int64_t value) {
  SQL_CHECK(idx < static_cast<int>(impl_->param_bind.size()), -1,
            "idx out of range! idx: {}, total: {}", idx,
            impl_->param_bind.size());
  impl_->param_buffer.push_back(value);
  auto& buf = impl_->param_buffer.back();
  impl_->param_bind[idx].buffer_type = MYSQL_TYPE_LONGLONG;
  impl_->param_bind[idx].buffer = boost::any_cast<int64_t>(&buf);
}

void MySQLStatement::sub_bindDouble(int idx, double item) {
  SQL_CHECK(idx < static_cast<int>(impl_->param_bind.size()), -1,
            "idx out of range! idx: {}, total: {}", idx,
            impl_->param_bind.size());
  impl_->param_buffer.push_back(item);
  auto& buf = impl_->param_buffer.back();
  impl_->param_bind[idx].buffer_type = MYSQL_TYPE_DOUBLE;
  impl_->param_bind[idx].buffer = boost::any_cast<double>(&buf);
}

void MySQLStatement::sub_bindDatetime(int idx, const Datetime& item) {
  if (item == Null<Datetime>()) {
    sub_bindNull(idx);
    return;
  }

  SQL_CHECK(idx < static_cast<int>(impl_->param_bind.size()), -1,
            "idx out of range! idx: {}, total: {}", idx,
            impl_->param_bind.size());
  MYSQL_TIME tm;
  tm.year = static_cast<unsigned int>(item.year());
  tm.month = static_cast<unsigned int>(item.month());
  tm.day = static_cast<unsigned int>(item.day());
  tm.hour = static_cast<unsigned int>(item.hour());
  tm.minute = static_cast<unsigned int>(item.minute());
  tm.second = static_cast<unsigned int>(item.second());
  tm.second_part = static_cast<unsigned long>(item.millisecond() * 1000 +
                                              item.microsecond());
  tm.time_type = MYSQL_TIMESTAMP_DATETIME;
  impl_->param_buffer.push_back(tm);
  auto& buf = impl_->param_buffer.back();
  MYSQL_TIME* p = boost::any_cast<MYSQL_TIME>(&buf);
  impl_->param_bind[idx].buffer_type = MYSQL_TYPE_DATETIME;
  impl_->param_bind[idx].buffer = p;
  impl_->param_bind[idx].buffer_length = sizeof(MYSQL_TIME);
  impl_->param_bind[idx].is_null = 0;
}

void MySQLStatement::sub_bindText(int idx, const std::string& item) {
  SQL_CHECK(idx < static_cast<int>(impl_->param_bind.size()), -1,
            "idx out of range! idx: {}, total: {}", idx,
            impl_->param_bind.size());
  impl_->param_buffer.push_back(item);
  auto& buf = impl_->param_buffer.back();
  std::string* p = boost::any_cast<std::string>(&buf);
  impl_->param_bind[idx].buffer_type = MYSQL_TYPE_VAR_STRING;
  impl_->param_bind[idx].buffer = (void*)p->data();
  impl_->param_bind[idx].buffer_length = item.size();
  impl_->param_bind[idx].is_null = 0;
}

void MySQLStatement::sub_bindText(int idx, const char* item, size_t len) {
  SQL_CHECK(idx < static_cast<int>(impl_->param_bind.size()), -1,
            "idx out of range! idx: {}, total: {}", idx,
            impl_->param_bind.size());
  impl_->param_buffer.push_back(std::string(item));
  auto& buf = impl_->param_buffer.back();
  std::string* p = boost::any_cast<std::string>(&buf);
  impl_->param_bind[idx].buffer_type = MYSQL_TYPE_VAR_STRING;
  impl_->param_bind[idx].buffer = (void*)p->data();
  impl_->param_bind[idx].buffer_length = p->size();
  impl_->param_bind[idx].is_null = 0;
}

void MySQLStatement::sub_bindBlob(int idx, const std::string& item) {
  SQL_CHECK(idx < static_cast<int>(impl_->param_bind.size()), -1,
            "idx out of range! idx: {}, total: {}", idx,
            impl_->param_bind.size());
  impl_->param_buffer.push_back(item);
  auto& buf = impl_->param_buffer.back();
  std::string* p = boost::any_cast<std::string>(&buf);
  impl_->param_bind[idx].buffer_type = MYSQL_TYPE_BLOB;
  impl_->param_bind[idx].buffer = (void*)p->data();
  impl_->param_bind[idx].buffer_length = item.size();
  impl_->param_bind[idx].is_null = 0;
}

void MySQLStatement::sub_bindBlob(int idx, const std::vector<char>& item) {
  SQL_CHECK(idx < static_cast<int>(impl_->param_bind.size()), -1,
            "idx out of range! idx: {}, total: {}", idx,
            impl_->param_bind.size());
  impl_->param_buffer.push_back(item);
  auto& buf = impl_->param_buffer.back();
  std::vector<char>* p = boost::any_cast<std::vector<char>>(&buf);
  impl_->param_bind[idx].buffer_type = MYSQL_TYPE_BLOB;
  impl_->param_bind[idx].buffer = (void*)p->data();
  impl_->param_bind[idx].buffer_length = p->size();
  impl_->param_bind[idx].is_null = 0;
}

int MySQLStatement::sub_getNumColumns() const {
  return mysql_stmt_field_count(impl_->stmt);
}

void MySQLStatement::sub_getColumnAsInt64(int idx, int64_t& item) {
  SQL_CHECK(idx < static_cast<int>(impl_->result_buffer.size()), -1,
            "idx out of range! idx: {}, total: {}", idx,
            impl_->result_buffer.size());

  SQL_CHECK(impl_->result_error[idx] == 0, -1,
            "Error occurred in sub_getColumnAsint64_t! idx: {}", idx);

  if (impl_->result_is_null[idx]) {
    item = 0;
    return;
  }

  try {
    if (impl_->result_bind[idx].buffer_type == MYSQL_TYPE_LONGLONG) {
      item = boost::any_cast<int64_t>(impl_->result_buffer[idx]);
    } else if (impl_->result_bind[idx].buffer_type == MYSQL_TYPE_LONG ||
               impl_->result_bind[idx].buffer_type == MYSQL_TYPE_INT24) {
      item = boost::any_cast<int32_t>(impl_->result_buffer[idx]);
    } else if (impl_->result_bind[idx].buffer_type == MYSQL_TYPE_TINY) {
      item = boost::any_cast<int8_t>(impl_->result_buffer[idx]);
    } else if (impl_->result_bind[idx].buffer_type == MYSQL_TYPE_SHORT ||
               impl_->result_bind[idx].buffer_type == MYSQL_TYPE_YEAR) {
      item = boost::any_cast<short>(impl_->result_buffer[idx]);
    } else {
      HAYAKU_THROW("Field type mismatch! idx: {}", idx);
    }
  } catch (const hayaku::exception&) {
    throw;
  } catch (const std::exception& e) {
    HAYAKU_THROW("Failed get column idx: {}! {}", idx, e.what());
  } catch (...) {
    HAYAKU_THROW("Failed get columon idx: {}! Unknown error!", idx);
  }
}

void MySQLStatement::sub_getColumnAsDouble(int idx, double& item) {
  SQL_CHECK(idx < static_cast<int>(impl_->result_buffer.size()), -1,
            "idx out of range! idx: {}, total: {}", idx,
            impl_->result_buffer.size());

  SQL_CHECK(impl_->result_error[idx] == 0, -1,
            "Error occurred in sub_getColumnAsDouble! idx: {}", idx);

  if (impl_->result_is_null[idx]) {
    item = 0.0;
    return;
  }

  try {
    if (impl_->result_bind[idx].buffer_type == MYSQL_TYPE_DOUBLE) {
      item = boost::any_cast<double>(impl_->result_buffer[idx]);
    } else if (impl_->result_bind[idx].buffer_type == MYSQL_TYPE_FLOAT) {
      item = boost::any_cast<float>(impl_->result_buffer[idx]);
    } else if (impl_->result_bind[idx].buffer_type == MYSQL_TYPE_LONGLONG) {
      item = boost::any_cast<int64_t>(impl_->result_buffer[idx]);
    } else if (impl_->result_bind[idx].buffer_type == MYSQL_TYPE_LONG ||
               impl_->result_bind[idx].buffer_type == MYSQL_TYPE_INT24) {
      item = boost::any_cast<int32_t>(impl_->result_buffer[idx]);
    } else if (impl_->result_bind[idx].buffer_type == MYSQL_TYPE_TINY) {
      item = boost::any_cast<int8_t>(impl_->result_buffer[idx]);
    } else if (impl_->result_bind[idx].buffer_type == MYSQL_TYPE_SHORT ||
               impl_->result_bind[idx].buffer_type == MYSQL_TYPE_YEAR) {
      item = boost::any_cast<short>(impl_->result_buffer[idx]);
    } else if (impl_->result_bind[idx].buffer_type == MYSQL_TYPE_DECIMAL ||
               impl_->result_bind[idx].buffer_type == MYSQL_TYPE_NEWDECIMAL) {
      std::vector<char>* p =
          boost::any_cast<std::vector<char>>(&(impl_->result_buffer[idx]));
      std::ostringstream buf;
      for (unsigned long i = 0; i < impl_->result_length[idx]; i++) {
        buf << (*p)[i];
      }
      item = std::stod(buf.str());
    } else {
      HAYAKU_THROW("Field type({}) mismatch! idx: {}",
                   int(impl_->result_bind[idx].buffer_type), idx);
    }
  } catch (const hayaku::exception&) {
    throw;
  } catch (const std::exception& e) {
    HAYAKU_THROW("Failed get column idx: {}! {}", idx, e.what());
  } catch (...) {
    HAYAKU_THROW("Failed get columon idx: {}! Unknown error!", idx);
  }
}

void MySQLStatement::sub_getColumnAsDatetime(int idx, Datetime& item) {
  SQL_CHECK(idx < static_cast<int>(impl_->result_buffer.size()), -1,
            "idx out of range! idx: {}, total: {}", idx,
            impl_->result_buffer.size());

  SQL_CHECK(impl_->result_error[idx] == 0, -1,
            "Error occurred in sub_getColumnAsDatetime! idx: {}", idx);

  if (impl_->result_is_null[idx]) {
    item = Null<Datetime>();
    return;
  }

  try {
    const MYSQL_TIME* tm =
        boost::any_cast<MYSQL_TIME>(&(impl_->result_buffer[idx]));
    if (tm->time_type == MYSQL_TIMESTAMP_DATETIME) {
      long millisec = tm->second_part / 1000;
      long microsec = tm->second_part - millisec * 1000;
      item = Datetime(tm->year, tm->month, tm->day, tm->hour, tm->minute,
                      tm->second, millisec, microsec);
    } else if (tm->time_type == MYSQL_TIMESTAMP_DATE) {
      item = Datetime(tm->year, tm->month, tm->day);
    } else {
      HAYAKU_THROW("Unsupported type: {}, Field type mismatch! idx: {}",
                   int(impl_->result_bind[idx].buffer_type), idx);
    }
  } catch (const hayaku::exception&) {
    throw;
  } catch (...) {
    HAYAKU_THROW("Field type mismatch! idx: {}", idx);
  }
}

void MySQLStatement::sub_getColumnAsText(int idx, std::string& item) {
  SQL_CHECK(idx < static_cast<int>(impl_->result_buffer.size()), -1,
            "idx out of range! idx: {}, total: {}", idx,
            impl_->result_buffer.size());

  SQL_CHECK(impl_->result_error[idx] == 0, -1,
            "Error occurred in sub_getColumnAsText! idx: {}", idx);

  if (impl_->result_is_null[idx]) {
    item.clear();
    return;
  }

  try {
    if (impl_->result_bind[idx].buffer_type == MYSQL_TYPE_DATETIME ||
        impl_->result_bind[idx].buffer_type == MYSQL_TYPE_TIMESTAMP ||
        impl_->result_bind[idx].buffer_type == MYSQL_TYPE_DATE ||
        impl_->result_bind[idx].buffer_type == MYSQL_TYPE_TIME) {
      const MYSQL_TIME* tm =
          boost::any_cast<MYSQL_TIME>(&(impl_->result_buffer[idx]));
      if (tm->time_type == MYSQL_TIMESTAMP_DATETIME) {
        long millisec = tm->second_part / 1000;
        long microsec = tm->second_part - millisec * 1000;
        item = Datetime(tm->year, tm->month, tm->day, tm->hour, tm->minute,
                        tm->second, millisec, microsec)
                   .str();
      } else if (tm->time_type == MYSQL_TIMESTAMP_DATE) {
        item = Datetime(tm->year, tm->month, tm->day).str();
      } else if (tm->time_type == MYSQL_TIMESTAMP_TIME) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", tm->hour, tm->minute,
                 tm->second);
        item = std::string(buf);
      } else {
        HAYAKU_THROW("Unsupported type: {}, Field type mismatch! idx: {}",
                     int(impl_->result_bind[idx].buffer_type), idx);
      }
      return;
    }

    std::vector<char>* p =
        boost::any_cast<std::vector<char>>(&(impl_->result_buffer[idx]));
    std::ostringstream buf;
    for (unsigned long i = 0; i < impl_->result_length[idx]; i++) {
      buf << (*p)[i];
    }
    item = buf.str();
  } catch (...) {
    HAYAKU_THROW("Field type mismatch! idx: {}", idx);
  }
}

void MySQLStatement::sub_getColumnAsBlob(int idx, std::string& item) {
  SQL_CHECK(idx < static_cast<int>(impl_->result_buffer.size()), -1,
            "idx out of range! idx: {}, total: {}", idx,
            impl_->result_buffer.size());

  SQL_CHECK(impl_->result_error[idx] == 0, -1,
            "Error occurred in sub_getColumnAsBlob! idx: {}", idx);

  if (impl_->result_is_null[idx]) {
    item.clear();
    return;
  }

  try {
    std::vector<char>* p =
        boost::any_cast<std::vector<char>>(&impl_->result_buffer[idx]);
    std::ostringstream buf;
    for (unsigned long i = 0; i < impl_->result_length[idx]; i++) {
      buf << (*p)[i];
    }
    item = buf.str();
  } catch (...) {
    HAYAKU_THROW("Field type mismatch! idx: {}", idx);
  }
}

void MySQLStatement::sub_getColumnAsBlob(int idx, std::vector<char>& item) {
  SQL_CHECK(idx < static_cast<int>(impl_->result_buffer.size()), -1,
            "idx out of range! idx: {}, total: {}", idx,
            impl_->result_buffer.size());

  SQL_CHECK(impl_->result_error[idx] == 0, -1,
            "Error occurred in sub_getColumnAsBlob! idx: {}", idx);

  if (impl_->result_is_null[idx]) {
    item.clear();
    return;
  }

  try {
    unsigned long len = impl_->result_length[idx];
    std::vector<char>* p =
        boost::any_cast<std::vector<char>>(&impl_->result_buffer[idx]);
    item.resize(len);
    memcpy(item.data(), p->data(), len);

  } catch (...) {
    HAYAKU_THROW("Field type mismatch! idx: {}", idx);
  }
}

uint64_t MySQLStatement::sub_getLastRowid() {
  return mysql_stmt_insert_id(impl_->stmt);
}

}  // namespace hayaku

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
