#pragma once

/*
 * exception.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-6-23
 *      Author: fasiondog
 */

#include <exception>
#include <string>

#ifndef HAYAKU_UTILS_API
#define HAYAKU_UTILS_API
#endif

namespace hayaku {

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4275)
#endif

/**
 * @ingroup Utilities
 * @addtogroup Exception Exception handling
 * @{
 */

class HAYAKU_UTILS_API exception : public std::exception {
 public:
  exception() : m_msg("Unknown exception!") {}
  exception(const char *msg)
      : m_msg(msg) {}  // cppcheck-suppress noExplicitConstructor
  exception(const std::string &msg)
      : m_msg(msg) {}  // cppcheck-suppress noExplicitConstructor
  virtual ~exception() noexcept {}
  virtual const char *what() const noexcept;

 protected:
  std::string m_msg;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

} /* namespace hayaku */
