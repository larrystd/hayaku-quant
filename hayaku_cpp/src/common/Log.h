#pragma once

/*
 * Log.h
 *
 *  Created on: 2013-2-1
 *      Author: fasiondog
 */


#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "common/Config.h"
#include "Exception.h"
#include "CppDef.h"

#ifndef HAYAKU_LOG_ACTIVE_LEVEL
#define HAYAKU_LOG_ACTIVE_LEVEL 0
#endif

// clang-format off
#ifndef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL HAYAKU_LOG_ACTIVE_LEVEL
#endif

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#if HAYAKU_USE_SPDLOG_ASYNC_LOGGER
    #include "spdlog/async.h"
#endif
// clang-format on

#include <fmt/ostream.h>
#include <fmt/format.h>
#include <fmt/chrono.h>

#ifndef HAYAKU_ENABLE_STACK_TRACE
#define HAYAKU_ENABLE_STACK_TRACE 0
#endif

#if HAYAKU_ENABLE_STACK_TRACE
#include <boost/stacktrace.hpp>
#endif

#ifndef HAYAKU_UTILS_API
#define HAYAKU_UTILS_API
#endif

namespace hayaku {

/**********************************************
 * Use SPDLOG for logging
 *********************************************/

/** Log level */
enum LOG_LEVEL {
    LOG_TRACE = SPDLOG_LEVEL_TRACE,     ///< Trace
    LOG_DEBUG = SPDLOG_LEVEL_DEBUG,     ///< Debug
    LOG_INFO = SPDLOG_LEVEL_INFO,       ///< General information
    LOG_WARN = SPDLOG_LEVEL_WARN,       ///< Warning
    LOG_ERROR = SPDLOG_LEVEL_ERROR,     ///< Error
    LOG_FATAL = SPDLOG_LEVEL_CRITICAL,  ///< Fatal
    LOG_OFF = SPDLOG_LEVEL_OFF,         ///< Turn off the log printing
};

/**
 * Initialize the logger
 * @param not_use_color do not use the colored output
 * @param filename the log file name; it is "./hayaku.log" in the current directory by default when
 *                 it is empty; you need to guarantee that the directory exists and is writable
 */
void HAYAKU_UTILS_API initLogger(bool not_use_color = false,
                              const std::string& filename = std::string());

/**
 * Get the current log level
 * @return
 */
LOG_LEVEL HAYAKU_UTILS_API get_log_level();

/**
 * Set the log level
 * @param level the given log level
 */
void HAYAKU_UTILS_API set_log_level(LOG_LEVEL level);

std::shared_ptr<spdlog::logger> HAYAKU_UTILS_API getHayakuLogger();

#define HAYAKU_TRACE(...) SPDLOG_LOGGER_TRACE(::hayaku::getHayakuLogger(), __VA_ARGS__)
#define HAYAKU_DEBUG(...) SPDLOG_LOGGER_DEBUG(::hayaku::getHayakuLogger(), __VA_ARGS__)
#define HAYAKU_INFO(...) SPDLOG_LOGGER_INFO(::hayaku::getHayakuLogger(), __VA_ARGS__)
#define HAYAKU_WARN(...) SPDLOG_LOGGER_WARN(::hayaku::getHayakuLogger(), __VA_ARGS__)
#define HAYAKU_ERROR(...) SPDLOG_LOGGER_ERROR(::hayaku::getHayakuLogger(), __VA_ARGS__)
#define HAYAKU_FATAL(...) SPDLOG_LOGGER_CRITICAL(::hayaku::getHayakuLogger(), __VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
//
// Under clang/gcc __PRETTY_FUNCTION__ contains the function parameters, it can be specified at
// compile time
// #define HAYAKU_FUNCTION __PRETTY_FUNCTION__
//
///////////////////////////////////////////////////////////////////////////////
#ifndef HAYAKU_FUNCTION
#define HAYAKU_FUNCTION __FUNCTION__
#endif

#if !HAYAKU_ENABLE_STACK_TRACE
#if CPP_STANDARD >= CPP_STANDARD_20
/**
 * If the expression is false an hayaku::exception is thrown with the passed information
 * @note Used for checking the external input parameters and the results
 */
#define HAYAKU_CHECK(expr, ...)                                                                     \
    do {                                                                                         \
        if (!(expr)) [[unlikely]] {                                                              \
            throw ::hayaku::exception(fmt::format("HAYAKU_CHECK({}) {} [{}] ({}:{})", #expr,           \
                                               fmt::format(__VA_ARGS__), HAYAKU_FUNCTION, __FILE__, \
                                               __LINE__));                                       \
        }                                                                                        \
    } while (0)

/**
 * If the expression is false the given exception is thrown with the passed information
 * @note Used for checking the external input parameters and the results
 */
#define HAYAKU_CHECK_THROW(expr, except, ...)                                                         \
    do {                                                                                           \
        if (!(expr)) [[unlikely]] {                                                                \
            throw except(fmt::format("HAYAKU_CHECK({}) {} [{}] ({}:{})", #expr,                       \
                                     fmt::format(__VA_ARGS__), HAYAKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                          \
    } while (0)
#else
#define HAYAKU_CHECK(expr, ...)                                                                     \
    do {                                                                                         \
        if (!(expr)) {                                                                           \
            throw ::hayaku::exception(fmt::format("HAYAKU_CHECK({}) {} [{}] ({}:{})", #expr,           \
                                               fmt::format(__VA_ARGS__), HAYAKU_FUNCTION, __FILE__, \
                                               __LINE__));                                       \
        }                                                                                        \
    } while (0)

#define HAYAKU_CHECK_THROW(expr, except, ...)                                                         \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            throw except(fmt::format("HAYAKU_CHECK({}) {} [{}] ({}:{})", #expr,                       \
                                     fmt::format(__VA_ARGS__), HAYAKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                          \
    } while (0)
#endif  // CPP_STANDARD >= CPP_STANDARD_20
#else
#define HAYAKU_CHECK(expr, ...)                                                                     \
    do {                                                                                         \
        if (!(expr)) {                                                                           \
            std::string errmsg = fmt::format(__VA_ARGS__);                                       \
            errmsg = fmt::format("{}\n {}", errmsg, to_string(boost::stacktrace::stacktrace())); \
            throw ::hayaku::exception(fmt::format("HAYAKU_CHECK({}) {} [{}] ({}:{})", #expr, errmsg,   \
                                               HAYAKU_FUNCTION, __FILE__, __LINE__));               \
        }                                                                                        \
    } while (0)

#define HAYAKU_CHECK_THROW(expr, except, ...)                                                         \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            std::string errmsg = fmt::format(__VA_ARGS__);                                         \
            errmsg = fmt::format("{}\n {}", errmsg, to_string(boost::stacktrace::stacktrace()));   \
            throw except(fmt::format("HAYAKU_CHECK({}) {} [{}] ({}:{})", #expr, errmsg, HAYAKU_FUNCTION, \
                                     __FILE__, __LINE__));                                         \
        }                                                                                          \
    } while (0)
#endif  // #if !HAYAKU_ENABLE_STACK_TRACE

#if HAYAKU_ENABLE_STACK_TRACE
/**
 * If the expression is false an hayaku::exception is thrown
 * @note Used for checking the internal input parameters only; it can be disabled at compile time
 *       with the HAYAKU_DISABLE_ASSERT macro
 */
#define HAYAKU_ASSERT(expr)                                                                  \
    do {                                                                                  \
        if (!(expr)) {                                                                    \
            std::string err_msg(fmt::format("HAYAKU_ASSERT({})\n{}", #expr,                  \
                                            to_string(boost::stacktrace::stacktrace()))); \
            throw ::hayaku::exception(                                                       \
              fmt::format("{} [{}] ({}:{})", err_msg, HAYAKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                 \
    } while (0)

#else
#if CPP_STANDARD >= CPP_STANDARD_20
#define HAYAKU_ASSERT(expr)                                                                  \
    do {                                                                                  \
        if (!(expr)) [[unlikely]] {                                                       \
            std::string err_msg(fmt::format("HAYAKU_ASSERT({})", #expr));                    \
            throw ::hayaku::exception(                                                       \
              fmt::format("{} [{}] ({}:{})", err_msg, HAYAKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                 \
    } while (0)
#else
#define HAYAKU_ASSERT(expr)                                                                  \
    do {                                                                                  \
        if (!(expr)) {                                                                    \
            std::string err_msg(fmt::format("HAYAKU_ASSERT({})", #expr));                    \
            throw ::hayaku::exception(                                                       \
              fmt::format("{} [{}] ({}:{})", err_msg, HAYAKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                 \
    } while (0)
#endif  // CPP_STANDARD >= CPP_STANDARD_20
#endif  // #if HAYAKU_ENABLE_STACK_TRACE

#if !HAYAKU_ENABLE_STACK_TRACE
/** Throw an hayaku::exception with the passed information */
#define HAYAKU_THROW(...)                                                                             \
    do {                                                                                           \
        throw ::hayaku::exception(fmt::format("EXCEPTION: {} [{}] ({}:{})", fmt::format(__VA_ARGS__), \
                                           HAYAKU_FUNCTION, __FILE__, __LINE__));                     \
    } while (0)

/** Throw the given exception with the passed information */
#define HAYAKU_THROW_EXCEPTION(except, ...)                                                 \
    do {                                                                                 \
        throw except(fmt::format("EXCEPTION: {} [{}] ({}:{})", fmt::format(__VA_ARGS__), \
                                 HAYAKU_FUNCTION, __FILE__, __LINE__));                     \
    } while (0)

#else
#define HAYAKU_THROW(...)                                                                          \
    do {                                                                                        \
        std::string errmsg(fmt::format("{}\n {}", fmt::format(__VA_ARGS__),                     \
                                       to_string(boost::stacktrace::stacktrace())));            \
        throw ::hayaku::exception(                                                                 \
          fmt::format("EXCEPTION: {} [{}] ({}:{})", errmsg, HAYAKU_FUNCTION, __FILE__, __LINE__)); \
    } while (0)

#define HAYAKU_THROW_EXCEPTION(except, ...)                                                        \
    do {                                                                                        \
        std::string errmsg(fmt::format("{}\n {}", fmt::format(__VA_ARGS__),                     \
                                       to_string(boost::stacktrace::stacktrace())));            \
        throw except(                                                                           \
          fmt::format("EXCEPTION: {} [{}] ({}:{})", errmsg, HAYAKU_FUNCTION, __FILE__, __LINE__)); \
    } while (0)
#endif  // #if !HAYAKU_ENABLE_STACK_TRACE

/**
 * Print the TRACE information when the given condition is satisfied
 * @param expr the given condition
 */
#define HAYAKU_TRACE_IF(expr, ...) \
    if (expr) {                 \
        HAYAKU_TRACE(__VA_ARGS__); \
    }

/**
 * Print the DEBUG information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 */
#define HAYAKU_DEBUG_IF(expr, ...) \
    if (expr) {                 \
        HAYAKU_DEBUG(__VA_ARGS__); \
    }

/**
 * Print the INFO information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 */
#define HAYAKU_INFO_IF(expr, ...) \
    if (expr) {                \
        HAYAKU_INFO(__VA_ARGS__); \
    }

/**
 * Print the WARN information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 */
#define HAYAKU_WARN_IF(expr, ...) \
    if (expr) {                \
        HAYAKU_WARN(__VA_ARGS__); \
    }

/**
 * Print the ERROR information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 */
#define HAYAKU_ERROR_IF(expr, ...) \
    if (expr) {                 \
        HAYAKU_ERROR(__VA_ARGS__); \
    }

/**
 * Print the FATAL information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 */
#define HAYAKU_FATAL_IF(expr, ...) \
    if (expr) {                 \
        HAYAKU_FATAL(__VA_ARGS__); \
    }

/**
 * Return the given value when the given condition is satisfied
 * @param expr the given condition
 * @param ret return value
 */
#define HAYAKU_IF_RETURN(expr, ret) \
    if (expr) {                  \
        return ret;              \
    }

/**
 * Print the TRACE information and return the given value when the given condition is satisfied
 * @param expr the given condition
 * @param ret return value
 */
#define HAYAKU_TRACE_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        HAYAKU_TRACE(__VA_ARGS__);             \
        return ret;                         \
    }

/**
 * Print the DEBUG information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 * @param ret return value
 */
#define HAYAKU_DEBUG_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        HAYAKU_DEBUG(__VA_ARGS__);             \
        return ret;                         \
    }

/**
 * Print the INFO information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 * @param ret return value
 */
#define HAYAKU_INFO_IF_RETURN(expr, ret, ...) \
    if (expr) {                            \
        HAYAKU_INFO(__VA_ARGS__);             \
        return ret;                        \
    }

/**
 * Print the WARN information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 * @param ret return value
 */
#define HAYAKU_WARN_IF_RETURN(expr, ret, ...) \
    if (expr) {                            \
        HAYAKU_WARN(__VA_ARGS__);             \
        return ret;                        \
    }

/**
 * Print the ERROR information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 * @param ret return value
 */
#define HAYAKU_ERROR_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        HAYAKU_ERROR(__VA_ARGS__);             \
        return ret;                         \
    }

/**
 * Print the FATAL information and return the given value when the given
 * condition is satisfied
 * @param expr the given condition
 * @param ret return value
 */
#define HAYAKU_FATAL_IF_RETURN(expr, ret, ...) \
    if (expr) {                             \
        HAYAKU_FATAL(__VA_ARGS__);             \
        return ret;                         \
    }

/** Used for the printing in catch (...), it reduces the size of the compiled code */
#define HAYAKU_THROW_UNKNOWN HAYAKU_THROW("Unknown error!")
#define HAYAKU_TRACE_UNKNOWN HAYAKU_TRACE("Unknown error!")
#define HAYAKU_DEBUG_UNKNOWN HAYAKU_DEBUG("Unknown error!")
#define HAYAKU_INFO_UNKNOWN HAYAKU_INFO("Unknown error!");
#define HAYAKU_ERROR_UNKNOWN HAYAKU_ERROR("Unknown error!");
#define HAYAKU_FATAL_UNKNOWN HAYAKU_FATAL("Unknown error!");

#if CPP_STANDARD >= CPP_STANDARD_20
#define CLASS_LOGGER_IMP(cls) \
protected:                    \
    static constexpr const char* ms_logger = #cls;
#elif CPP_STANDARD >= CPP_STANDARD_17
#define CLASS_LOGGER_IMP(cls) \
protected:                    \
    inline static const char* ms_logger = #cls;
#else
#define CLASS_LOGGER_IMP(cls) \
protected:                    \
    const char* ms_logger = #cls;
#endif

#define CLS_TRACE(...) HAYAKU_TRACE(fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_DEBUG(...) HAYAKU_DEBUG(fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_INFO(...) HAYAKU_INFO(fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_WARN(...) HAYAKU_WARN(fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_ERROR(...) HAYAKU_ERROR(fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_FATAL(...) HAYAKU_FATAL(fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))

#define CLS_TRACE_IF(expr, ...) \
    HAYAKU_TRACE_IF(expr, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_DEBUG_IF(expr, ...) \
    HAYAKU_DEBUG_IF(expr, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_INFO_IF(expr, ...) \
    HAYAKU_INFO_IF(expr, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_WARN_IF(expr, ...) \
    HAYAKU_WARN_IF(expr, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_ERROR_IF(expr, ...) \
    HAYAKU_ERROR_IF(expr, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_FATAL_IF(expr, ...) \
    HAYAKU_FATAL_IF(expr, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))

#define CLS_IF_RETURN(expr, ret) HAYAKU_IF_RETURN(expr, ret)
#define CLS_TRACE_IF_RETURN(expr, ret, ...) \
    HAYAKU_TRACE_IF_RETURN(expr, ret, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_DEBUG_IF_RETURN(expr, ret, ...) \
    HAYAKU_DEBUG_IF_RETURN(expr, ret, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_INFO_IF_RETURN(expr, ret, ...) \
    HAYAKU_INFO_IF_RETURN(expr, ret, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_WARN_IF_RETURN(expr, ret, ...) \
    HAYAKU_WARN_IF_RETURN(expr, ret, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_ERROR_IF_RETURN(expr, ret, ...) \
    HAYAKU_ERROR_IF_RETURN(expr, ret, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))
#define CLS_FATAL_IF_RETURN(expr, ret, ...) \
    HAYAKU_FATAL_IF_RETURN(expr, ret, fmt::format("[{}] {}", ms_logger, fmt::format(__VA_ARGS__)))

#define CLS_ASSERT HAYAKU_ASSERT

#if CPP_STANDARD >= CPP_STANDARD_20
#define CLS_CHECK(expr, ...)                                                                    \
    do {                                                                                        \
        if (!(expr)) [[unlikely]] {                                                             \
            throw ::hayaku::exception(fmt::format("[{}] CLS_CHECK({}) {} [{}] ({}:{})", ms_logger, \
                                               #expr, fmt::format(__VA_ARGS__), HAYAKU_FUNCTION,   \
                                               __FILE__, __LINE__));                            \
        }                                                                                       \
    } while (0)

#define CLS_CHECK_THROW(expr, except, ...)                                                         \
    do {                                                                                           \
        if (!(expr)) [[unlikely]] {                                                                \
            throw except(fmt::format("[{}] CLS_CHECK({}) {} [{}] ({}:{})", ms_logger, #expr,       \
                                     fmt::format(__VA_ARGS__), HAYAKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                          \
    } while (0)
#else
#define CLS_CHECK(expr, ...)                                                                    \
    do {                                                                                        \
        if (!(expr)) {                                                                          \
            throw ::hayaku::exception(fmt::format("[{}] CLS_CHECK({}) {} [{}] ({}:{})", ms_logger, \
                                               #expr, fmt::format(__VA_ARGS__), HAYAKU_FUNCTION,   \
                                               __FILE__, __LINE__));                            \
        }                                                                                       \
    } while (0)

#define CLS_CHECK_THROW(expr, except, ...)                                                         \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            throw except(fmt::format("[{}] CLS_CHECK({}) {} [{}] ({}:{})", ms_logger, #expr,       \
                                     fmt::format(__VA_ARGS__), HAYAKU_FUNCTION, __FILE__, __LINE__)); \
        }                                                                                          \
    } while (0)
#endif  // CPP_STANDARD >= CPP_STANDARD_20

#define CLS_THROW(...)                                                                       \
    do {                                                                                     \
        throw ::hayaku::exception(fmt::format("[{}] EXCEPTION: {} [{}] ({}:{})", ms_logger,     \
                                           fmt::format(__VA_ARGS__), HAYAKU_FUNCTION, __FILE__, \
                                           __LINE__));                                       \
    } while (0)

#define CLS_THROW_EXCEPTION(except, ...)                                                       \
    do {                                                                                       \
        throw except(fmt::format("[{}] EXCEPTION: {} [{}] ({}:{})", ms_logger,                 \
                                 fmt::format(__VA_ARGS__), HAYAKU_FUNCTION, __FILE__, __LINE__)); \
    } while (0)

/** @} */

} /* namespace hayaku */
