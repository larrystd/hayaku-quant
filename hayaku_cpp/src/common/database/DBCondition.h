#pragma once

/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-05-20
 *     Author: fasiondog
 */

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "common/Log.h"

#ifndef HAYAKU_UTILS_API
#define HAYAKU_UTILS_API
#endif

namespace hayaku {

struct ASC {
  explicit ASC(const char *name) : name(name) {}
  explicit ASC(const std::string &name) : name(name) {}
  std::string name;
};

struct DESC {
  explicit DESC(const char *name) : name(name) {}
  explicit DESC(const std::string &name) : name(name) {}
  std::string name;
};

struct LIMIT {
  explicit LIMIT(int limit) : limit(limit) {}
  int limit = 1;
};

class HAYAKU_UTILS_API DBCondition {
 public:
  DBCondition() = default;
  DBCondition(const DBCondition &) = default;
  DBCondition(DBCondition &&rv) : condition_(std::move(rv.condition_)) {}

  explicit DBCondition(const char *cond) : condition_(cond) {}
  explicit DBCondition(const std::string &cond) : condition_(cond) {}

  DBCondition &operator=(const DBCondition &) = default;
  DBCondition &operator=(DBCondition &&rv) {
    if (this != &rv) {
      condition_ = std::move(rv.condition_);
    }
    return *this;
  }

  DBCondition &operator&(const DBCondition &other);
  DBCondition &operator|(const DBCondition &other);

  enum ORDERBY { ORDER_ASC, ORDER_DESC };

  void orderBy(const std::string &field, ORDERBY order) {
    condition_ = order == ORDERBY::ORDER_ASC
                      ? fmt::format("{} order by {} ASC", condition_, field)
                      : fmt::format("{} order by {} DESC", condition_, field);
  }

  DBCondition &operator+(const ASC &asc) {
    orderBy(asc.name, ORDER_ASC);
    return *this;
  }

  DBCondition &operator+(const DESC &desc) {
    orderBy(desc.name, ORDER_DESC);
    return *this;
  }

  DBCondition &operator+(const LIMIT &limit) {
    condition_ = fmt::format("{} limit {}", condition_, limit.limit);
    return *this;
  }

  const std::string &str() const { return condition_; }

 private:
  std::string condition_;
};

struct Field {
  explicit Field(const char *name) : name(name) {}
  explicit Field(const std::string &name) : name(name) {}

  // in and not_in do not support strings; the SQL operation in ("stra", "strb")
  // is generally not used
  template <typename T>
  DBCondition in(const std::vector<T> &vals) {
    HAYAKU_CHECK(!vals.empty(), "input vals can't be empty!");
    return DBCondition(fmt::format("({} in ({}))", name, fmt::join(vals, ",")));
  }

  template <typename T>
  DBCondition not_in(const std::vector<T> &vals) {
    HAYAKU_CHECK(!vals.empty(), "input vals can't be empty!");
    return DBCondition(
        fmt::format("({} not in ({}))", name, fmt::join(vals, ",")));
  }

  DBCondition like(const std::string &pattern) {
    return DBCondition(fmt::format(R"(({} like "{}"))", name, pattern));
  }

  DBCondition like(const char *pattern) {
    return DBCondition(fmt::format(R"(({} like "{}"))", name, pattern));
  }

  std::string name;
};

// Under linux the template specialization of a class member function must be
// implemented outside the class Otherwise the compilation reports: explicit
// specialization in non-namespace scope
template <>
inline DBCondition Field::in<std::string>(
    const std::vector<std::string> &vals) {
  HAYAKU_CHECK(!vals.empty(), "input vals can't be empty!");
  std::ostringstream out;
  out << "(" << name << " in (";
  size_t total = vals.size();
  for (size_t i = 0; i < total - 1; i++) {
    out << "\"" << vals[i] << "\",";
  }
  out << "\"" << vals[total - 1] << "\"))";
  return DBCondition(out.str());
}

template <>
inline DBCondition Field::not_in<std::string>(
    const std::vector<std::string> &vals) {
  HAYAKU_CHECK(!vals.empty(), "input vals can't be empty!");
  std::ostringstream out;
  out << "(" << name << " not in (";
  size_t total = vals.size();
  for (size_t i = 0; i < total - 1; i++) {
    out << "\"" << vals[i] << "\",";
  }
  out << "\"" << vals[total - 1] << "\"))";
  return DBCondition(out.str());
}

inline std::ostream &operator<<(std::ostream &out, const DBCondition &d) {
  out << d.str();
  return out;
}

template <typename T>
inline DBCondition operator==(const Field &field, T val) {
  std::ostringstream out;
  out << "(" << field.name << "=" << val << ")";
  return DBCondition(out.str());
}

template <typename T>
inline DBCondition operator!=(const Field &field, T val) {
  std::ostringstream out;
  out << "(" << field.name << "<>" << val << ")";
  return DBCondition(out.str());
}

template <typename T>
inline DBCondition operator>(const Field &field, T val) {
  std::ostringstream out;
  out << "(" << field.name << ">" << val << ")";
  return DBCondition(out.str());
}

template <typename T>
inline DBCondition operator>=(const Field &field, T val) {
  std::ostringstream out;
  out << "(" << field.name << ">=" << val << ")";
  return DBCondition(out.str());
}

template <typename T>
inline DBCondition operator<(const Field &field, T val) {
  std::ostringstream out;
  out << "(" << field.name << "<" << val << ")";
  return DBCondition(out.str());
}

template <typename T>
inline DBCondition operator<=(const Field &field, T val) {
  std::ostringstream out;
  out << "(" << field.name << "<=" << val << ")";
  return DBCondition(out.str());
}

template <>
inline DBCondition operator!=(const Field &field, const char *val) {
  return DBCondition(fmt::format(R"(({}<>"{}"))", field.name, val));
}

template <>
inline DBCondition operator>(const Field &field, const char *val) {
  return DBCondition(fmt::format(R"(({}>"{}"))", field.name, val));
}

template <>
inline DBCondition operator<(const Field &field, const char *val) {
  return DBCondition(fmt::format(R"(({}<"{}"))", field.name, val));
}

template <>
inline DBCondition operator>=(const Field &field, const char *val) {
  return DBCondition(fmt::format(R"(({}>="{}"))", field.name, val));
}

template <>
inline DBCondition operator<=(const Field &field, const char *val) {
  return DBCondition(fmt::format(R"(({}<="{}"))", field.name, val));
}

inline DBCondition operator==(const Field &field, const std::string &val) {
  return DBCondition(fmt::format(R"(({}="{}"))", field.name, val));
}

inline DBCondition operator!=(const Field &field, const std::string &val) {
  return DBCondition(fmt::format(R"(({}<>"{}"))", field.name, val));
}

inline DBCondition operator>(const Field &field, const std::string &val) {
  return DBCondition(fmt::format(R"(({}>"{}"))", field.name, val));
}

inline DBCondition operator<(const Field &field, const std::string &val) {
  return DBCondition(fmt::format(R"(({}<"{}"))", field.name, val));
}

inline DBCondition operator>=(const Field &field, const std::string &val) {
  return DBCondition(fmt::format(R"(({}>="{}"))", field.name, val));
}

inline DBCondition operator<=(const Field &field, const std::string &val) {
  return DBCondition(fmt::format(R"(({}<="{}"))", field.name, val));
}

inline DBCondition operator==(const Field &field, const char *val) {
  return DBCondition(fmt::format(R"(({}="{}"))", field.name, val));
}

inline DBCondition operator!=(const Field &field, const char *val) {
  return DBCondition(fmt::format(R"(({}<>"{}"))", field.name, val));
}

}  // namespace hayaku
