/*
 * MySQLConnect_boost.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-8-17
 *      Author: fasiondog
 */

#include <boost/asio.hpp>
#include <boost/mysql.hpp>
#include <memory>

#include "MySQLConnect.h"
#include "common/Config.h"
#include "extensions/mysql/LruCache.h"

namespace hayaku {

// Helper function: print the diagnostics information
static void printDiagHelper(const boost::mysql::error_code& ec,
                            const boost::mysql::diagnostics& diag,
                            const std::string& context) {
  if (!diag.server_message().empty()) {
    HAYAKU_ERROR("{} Server error: {}", context, diag.server_message());
  } else if (!diag.client_message().empty()) {
    HAYAKU_ERROR("{} Client error: {}", context, diag.client_message());
  } else {
    HAYAKU_ERROR("{} Error code {}: {}", context, ec.value(), ec.message());
  }
}

// The Pimpl implementation struct
struct MySQLConnect::Impl {
  boost::asio::io_context io_context;
  std::unique_ptr<boost::mysql::tcp_connection> conn;
  std::unique_ptr<
      LruCache<std::string, std::shared_ptr<boost::mysql::statement>>>
      statement_cache;

  std::shared_ptr<boost::mysql::statement> get_statement(
      const std::string& sql, boost::mysql::error_code& ec,
      boost::mysql::diagnostics& diag) {
    std::shared_ptr<boost::mysql::statement> ret;
    if (statement_cache->tryGet(sql, ret)) {
      return ret;
    }

    // Create the statement and prepare the lambda for the closing
    auto* connection_ptr = conn.get();
    auto deleter = [connection_ptr](boost::mysql::statement* stmt) {
      if (stmt && connection_ptr) {
        connection_ptr->close_statement(*stmt);
        // Ignore the error at the closing, because the connection may have been
        // lost already
      }
      delete stmt;
    };

    ret = std::shared_ptr<boost::mysql::statement>(
        new boost::mysql::statement(conn->prepare_statement(sql, ec, diag)),
        deleter);

    if (!ec) {
      statement_cache->insert(sql, ret);
    } else {
      ret.reset();
    }

    return ret;
  }
};

MySQLConnect::MySQLConnect(const Parameter& param)
    : DBConnectBase(param), impl_(std::make_unique<Impl>()) {
  // Get the prepared statement cache size and create the cache
  int64_t cache_size = tryGetParam<int64_t>("statement_cache_size", 3);
  params_.set("statement_cache_size", cache_size);
  impl_->statement_cache = std::make_unique<
      LruCache<std::string, std::shared_ptr<boost::mysql::statement>>>(
      cache_size);
  connect();
}

MySQLConnect::~MySQLConnect() { close(); }

void* MySQLConnect::getRawConnection() const noexcept {
  return impl_->conn.get();
}

bool MySQLConnect::tryConnect() noexcept {
  bool success = false;
  try {
    close();
    connect();
    success = true;
  } catch (const std::exception& e) {
    HAYAKU_WARN(e.what());
  }
  return success;
}

void MySQLConnect::connect() {
  try {
    std::string host = tryGetParam<std::string>("host", "127.0.0.1");
    std::string usr = tryGetParam<std::string>("usr", "root");
    std::string pwd = tryGetParam<std::string>("pwd", "");
    std::string database = tryGetParam<std::string>("db", "");
    unsigned short port =
        static_cast<unsigned short>(tryGetParam<int>("port", 3306));

    impl_->conn =
        std::make_unique<boost::mysql::tcp_connection>(impl_->io_context);
    boost::mysql::handshake_params params(usr, pwd, database);

    boost::mysql::error_code ec;
    boost::mysql::diagnostics diag;
    impl_->conn->connect(boost::asio::ip::tcp::endpoint(
                              boost::asio::ip::make_address(host), port),
                          params, ec, diag);

    if (ec) {
      printDiagHelper(ec, diag, "MySQL connect");
      HAYAKU_THROW("{}, {}", ec.value(), ec.message());
    }

  } catch (const hayaku::exception& e) {
    close();
    HAYAKU_ERROR(e.what());
    HAYAKU_THROW("Failed create MySQLConnect! {}", e.what());

  } catch (const std::exception& e) {
    close();
    HAYAKU_ERROR(e.what());
    HAYAKU_THROW("Failed create MySQLConnent instance! {}", e.what());

  } catch (...) {
    close();
    const char* errmsg = "Failed create MySQLConnect instance! Unknown error";
    HAYAKU_ERROR(errmsg);
    HAYAKU_THROW("{}", errmsg);
  }
}

void MySQLConnect::close() {
  if (impl_ && impl_->conn) {
    impl_->statement_cache->clear();
    impl_->conn->close();
    impl_->conn.reset();
  }
}

bool MySQLConnect::ping() {
  HAYAKU_ERROR_IF_RETURN((!impl_ || !impl_->conn) && !tryConnect(), false,
                         "Failed connect to mysql!");

  try {
    boost::mysql::error_code ec;
    boost::mysql::diagnostics diag;
    boost::mysql::results results;
    impl_->conn->execute("SELECT 1", results, ec, diag);

    // Try to reconnect when the ping fails
    if (ec && !tryConnect()) [[unlikely]] {
      printDiagHelper(ec, diag, "MySQL ping failed!");
      return false;
    }
    return true;
  } catch (const std::exception& e) {
    // Try to reconnect on an exception as well
    HAYAKU_ERROR_IF_RETURN(!tryConnect(), false, "MySQL ping exception! {}",
                           e.what());
    return true;
  }
}

int64_t MySQLConnect::exec(const std::string& sql_string) {
#if HAYAKU_SQL_TRACE
  HAYAKU_DEBUG(sql_string);
#endif

  if (!impl_ || !impl_->conn) {
    SQL_CHECK(tryConnect(), -1, "Failed connect to mysql!");
  }

  boost::mysql::error_code ec;
  boost::mysql::diagnostics diag;
  boost::mysql::results results;
  impl_->conn->execute(sql_string, results, ec, diag);

  if (ec) [[unlikely]] {
    // The execution failed, try to reconnect and execute again
    if (ping()) {
      impl_->conn->execute(sql_string, results, ec, diag);
    }

    if (ec) {
      printDiagHelper(ec, diag, "MySQL execute sql");
      SQL_THROW(ec.value(), "SQL error: {}! error msg: {}", sql_string,
                ec.message());
    }
  }

  // Get the number of the affected rows
  return results.affected_rows();
}

SQLStatementPtr MySQLConnect::getStatement(const std::string& sql_statement) {
  return std::make_shared<MySQLStatement>(this, sql_statement);
}

bool MySQLConnect::tableExist(const std::string& tablename) {
  bool result = false;
  try {
    SQLStatementPtr st =
        getStatement(fmt::format("SELECT 1 FROM {} LIMIT 1;", tablename));
    st->exec();
    result = true;
  } catch (...) {
    result = false;
  }
  return result;
}

void MySQLConnect::resetAutoIncrement(const std::string& tablename) {
  int64_t count =
      queryNumber<int64_t>(fmt::format("select count(1) from {}", tablename));
  SQL_CHECK(count == 0, -1,
            "The ID cannot be reset when data is present in table({})",
            tablename);
  exec(fmt::format("alter {} auto_increment=1", tablename));
}

void MySQLConnect::transaction() { exec("BEGIN"); }

void MySQLConnect::commit() { exec("COMMIT"); }

void MySQLConnect::rollback() noexcept {
  try {
    exec("ROLLBACK");
  } catch (const std::exception& e) {
    HAYAKU_ERROR("Failed transaction! {}", e.what());
  } catch (...) {
    HAYAKU_ERROR("Unknown error!");
  }
}

}  // namespace hayaku
