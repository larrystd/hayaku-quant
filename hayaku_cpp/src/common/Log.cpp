/*
 * Log.cpp
 *
 *  Created on: 2013-2-1
 *      Author: fasiondog
 */

#include <thread>
#include <mutex>
#include "Os.h"
#include "Log.h"

// With stdout_color the log output cannot be redirected to python
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/null_sink.h>
#include <iostream>
#include "spdlog/sinks/ostream_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"

#if HAYAKU_USE_SPDLOG_ASYNC_LOGGER
#include <spdlog/async.h>
#endif /* HAYAKU_USE_SPDLOG_ASYNC_LOGGER */

namespace hayaku {

static LOG_LEVEL g_log_level = LOG_LEVEL::LOG_TRACE;
static std::mutex g_log_mutex;

LOG_LEVEL get_log_level() {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    return g_log_level;
}

void set_log_level(LOG_LEVEL level) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    g_log_level = level;
    getHayakuLogger()->set_level((spdlog::level::level_enum)level);
}

std::shared_ptr<spdlog::logger> getHayakuLogger() {
    auto logger = spdlog::get("hayaku");
    if (logger) {
        return logger;
    }
    logger = spdlog::default_logger();
    if (logger) {
        return logger;
    }

    // SPDLOG_DISABLE_DEFAULT_LOGGER leaves the default logger null until a session opens.
    // Logging before that point must remain safe without creating a file or process-wide logger.
    static auto bootstrapLogger = std::make_shared<spdlog::logger>(
      "hayaku-bootstrap", std::make_shared<spdlog::sinks::null_sink_mt>());
    return bootstrapLogger;
}

void HAYAKU_UTILS_API initLogger(bool not_use_color, const std::string& filename) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    std::string logname("hayaku");
    spdlog::drop(logname);
    std::shared_ptr<spdlog::logger> logger = spdlog::get(logname);
    if (logger) {
        spdlog::drop(logname);
    }

    spdlog::sink_ptr stdout_sink;
    if (not_use_color) {
        stdout_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(std::cout, true);
    } else {
        stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    }
    stdout_sink->set_level(spdlog::level::trace);

    std::string logfile = filename.empty() ? "./hayaku.log" : filename;
    auto rotating_sink =
      std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logfile, 1024 * 1024 * 10, 3);
    rotating_sink->set_level(spdlog::level::warn);

    std::vector<spdlog::sink_ptr> sinks{stdout_sink};
    if (rotating_sink) {
        sinks.emplace_back(rotating_sink);
    }

#if HAYAKU_USE_SPDLOG_ASYNC_LOGGER
    spdlog::init_thread_pool(8192, 1);
    logger = std::make_shared<spdlog::async_logger>(logname, sinks.begin(), sinks.end(),
                                                    spdlog::thread_pool(),
                                                    spdlog::async_overflow_policy::block);
#else
    logger = std::make_shared<spdlog::logger>(logname, sinks.begin(), sinks.end());
#endif

    logger->set_level((spdlog::level::level_enum)g_log_level);
    logger->flush_on(spdlog::level::trace);
    logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%^HAYAKU-%L%$] - %v (%s:%#)");
    // logger->set_pattern("%^%Y-%m-%d %H:%M:%S.%e [HAYAKU-%L] - %v (%s:%#)%$");
    // spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
}

}  // namespace hayaku
