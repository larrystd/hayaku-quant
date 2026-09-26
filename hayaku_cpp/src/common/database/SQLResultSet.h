#pragma once

/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-01-10
 *      Author: fasiondog
 */

#include <iterator>

#include "DBConnectBase.h"
#include "common/Arithmetic.h"
#include "common/Log.h"
#include "common/OsDef.h"

namespace hayaku {

template <class TableT, size_t page_size>
class SQLResultSetIterator;

/**
 * SQL query result set
 * @tparam TableT data structure
 * @tparam page_size the number of the data contained in every page
 * @ingroup DBConnect
 */
template <class TableT, size_t page_size = 100>
class SQLResultSet {
  friend class SQLResultSetIterator<TableT, page_size>;

 public:
#if CPP_STANDARD >= CPP_STANDARD_17
  static constexpr int PSIZE = page_size;
#else
  static const int PSIZE = page_size;
#endif

  SQLResultSet() = default;

  /**
   * Build a new paged query result instance
   * @param connect
   * @param sql
   */
  SQLResultSet(const DBConnectPtr& connect, const std::string& sql)
      : connect_(connect),
        where_(sql),
        sql_template_(
            "id IN (SELECT id FROM {} WHERE {} {} LIMIT {} OFFSET {}) {}") {
    trim(where_);
    if (where_.empty()) {
      where_ = "1=1";
      orderby_inner_ = "ORDER BY id";
      // m_orderby_outer = "";
      return;
    }

    std::string tmp = utf8_to_upper(where_);
    size_t pos = tmp.rfind("ORDER");
    if (pos != std::string::npos) {
      orderby_inner_ = fmt::format("{}, id ASC", where_.substr(pos));
      orderby_outer_ = orderby_inner_;
      where_ = where_.erase(pos, std::string::npos);
    } else {
      orderby_inner_ = "ORDER BY id";
    }
  }

  /** Get its database connection */
  const DBConnectPtr& getConnect() const { return connect_; }

  using const_iterator = SQLResultSetIterator<TableT, page_size>;
  using iterator = SQLResultSetIterator<TableT, page_size>;

  const_iterator cbegin() { return const_iterator(this, 0); }

  const_iterator cend() { return const_iterator(this, Null<size_t>()); }

  iterator begin() { return iterator(this, 0); }

  iterator end() { return iterator(this, Null<size_t>()); }

  /**
   * @brief Get the data set size at the current moment
   * @note The data set size changes with the current database content, it is
   * not always constant
   * @return size_t
   */
  size_t size() const {
    HAYAKU_IF_RETURN(!connect_, 0);
    std::string sql = fmt::format("select count(1) from {} where {}",
                                  TableT::getTableName(), where_);
    return connect_->queryNumber<size_t>(sql, 0);
  }

  /**
   * @brief Whether the current data set is empty
   * @return true empty
   * @return false not empty
   */
  bool empty() const { return size() == 0; }

  /**
   * @brief Get the number of the pages of the current data set
   * @note It is the number of the pages of the corresponding data set obtained
   * at the calling moment only
   * @return size_t
   */
  size_t getPageCount() {
    size_t total = size();
    size_t n = total / page_size;
    return n * page_size > total ? n : n + 1;
  }

  /**
   * @brief Get all the data in the given page
   * @param page the given page
   * @return std::vector<TableT> all the valid data sets contained in this page
   */
  std::vector<TableT> getPage(size_t page) {
    std::vector<TableT> result;
    connect_->batchLoad(
        result, fmt::format(sql_template_, TableT::getTableName(), where_,
                            orderby_inner_, page_size, page * page_size,
                            orderby_outer_));
    return result;
  }

  TableT operator[](size_t index) { return get(index); }

  TableT at(size_t index) {
    TableT result = get(index);
    HAYAKU_CHECK_THROW(result.valid(), std::out_of_range, "Index is over");
    return result;
  }

 private:
  TableT get(size_t index) {
    TableT result{Null<TableT>()};
    HAYAKU_IF_RETURN(index == Null<size_t>(), result);

    size_t page = index / page_size;
    if (connect_ && page != current_page_) {
      buffer_.clear();
      connect_->batchLoad(
          buffer_,
          fmt::format(fmt::runtime(sql_template_), TableT::getTableName(),
                      where_, orderby_inner_, page_size, page * page_size,
                      orderby_outer_));
      current_page_ = page;
    }

    HAYAKU_IF_RETURN(buffer_.empty(), result);

    size_t pos = index - page * page_size;
    HAYAKU_IF_RETURN(pos >= buffer_.size(), result);

    result = buffer_[index - page * page_size];
    return result;
  }

 private:
  DBConnectPtr connect_;
  std::vector<TableT> buffer_;
  std::string where_;
  std::string sql_template_;
  std::string orderby_inner_;
  std::string orderby_outer_;
  size_t current_page_ = Null<size_t>();
};

template <class TableT, size_t page_size>
class SQLResultSetIterator {
 public:
  using ResultSet = SQLResultSet<TableT, page_size>;

  SQLResultSetIterator() = default;
  ~SQLResultSetIterator() = default;

  explicit SQLResultSetIterator(ResultSet* result_set, size_t index)
      : set_(result_set), index_(index) {
    if (index_ != Null<size_t>()) {
      value_ = std::move(set_->get(index));
      if (!value_.valid()) {
        index_ = Null<size_t>();
      }
    }
  }

  SQLResultSetIterator(const SQLResultSetIterator& other)
      : set_(other.set_), index_(other.index_) {}

  SQLResultSetIterator& operator=(const SQLResultSetIterator& other) {
    if (this == &other) return *this;
    index_ = other.index_;
    set_ = other.set_;
    return *this;
  }

  const TableT& operator*() const { return value_; }

  TableT& operator*() { return value_; }

  const TableT* const operator->() const { return &value_; }

  TableT* operator->() { return &value_; }

  // Prefix increment operator
  SQLResultSetIterator& operator++() {
    HAYAKU_CHECK_THROW(index_ != Null<size_t>(), std::logic_error,
                       "Cannot increment an end iterator.");
    index_++;
    value_ = std::move(set_->get(index_));
    if (!value_.valid()) {
      index_ = Null<size_t>();
    }
    return *this;
  }

  bool operator!=(const SQLResultSetIterator& iter) const {
    return index_ != iter.index_;
  }

 private:
  ResultSet* set_ = nullptr;
  size_t index_ = Null<size_t>();
  TableT value_;
};

}  // namespace hayaku
