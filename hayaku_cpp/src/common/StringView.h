#pragma once

/*
 *  Copyright (c) 2019~2021, hayaku
 *
 *  Created on: 2021/12/16
 *      Author: fasiondog
 */

#include "CppDef.h"

#if CPP_STANDARD >= CPP_STANDARD_17
#include <string>
#include <string_view>
namespace hayaku {

using std::string_view;

}

#else

#include <cstring>
#include <string>

#include "Log.h"

namespace hayaku {

class string_view {
 public:
  constexpr string_view() noexcept = default;

  string_view(const char *data, std::size_t size) : _data(data), _size(size) {
    HAYAKU_CHECK_THROW(data, std::invalid_argument, "Input null ptr!");
  };

  string_view(const char *data) : _data(data) {  // NOSONAR
    HAYAKU_CHECK_THROW(data, std::invalid_argument, "Input null ptr!");
    _size = std::strlen(data);
  }

  string_view(const std::string &str)
      : _data(str.data()), _size(str.size()) {}  // NOSONAR

  constexpr string_view(const string_view &) noexcept = default;

  string_view &operator=(const string_view &) noexcept = default;

  constexpr const char &operator[](std::size_t pos) const { return _data[pos]; }

  constexpr const char *data() const noexcept { return _data; }

  constexpr std::size_t size() const noexcept { return _size; }

 private:
  constexpr string_view(std::nullptr_t) = delete;

 private:
  const char *_data = nullptr;
  std::size_t _size = 0;
};

}  // namespace hayaku

#endif
