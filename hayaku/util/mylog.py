#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import os
import logging
import logging.handlers
import traceback
import time
import functools
import multiprocessing


# Count the function running time
def spend_time(func):
    @functools.wraps(func)
    def wrappedFunc(*args, **kargs):
        starttime = time.perf_counter()
        try:
            print("\nCalling: %s" % func.__name__)
            return func(*args, **kargs)
        finally:
            endtime = time.perf_counter()
            print("spend time: %.6fs, %.4fm" % (endtime - starttime, (endtime - starttime) / 60))

    return wrappedFunc

# Count the function running time


def hayaku_benchmark(count=10):
    def hayaku_benchmark_wrap(func):
        @functools.wraps(func)
        def wrappedFunc(*args, **kargs):
            starttime = time.perf_counter()
            try:
                print(f"\nstart benchmark: {func.__name__}")
                for i in range(count):
                    ret = func(*args, **kargs)
                return ret
            finally:
                endtime = time.perf_counter()
                total_sec = endtime - starttime
                total_min = total_sec / 60.0
                seconds = total_sec / count
                minutes = seconds / 60.0
                print("+--------------------------------------------------------------------------")
                print(f"|  run count: {count}")
                print(f"|  total time: {total_sec: <.6f}s, {total_min: <.4f}m")
                print(f"|  mean time: {seconds: <.6f}s, {minutes: <.4f}m")
                print("+--------------------------------------------------------------------------")
        return wrappedFunc
    return hayaku_benchmark_wrap


FORMAT = '%(asctime)-15s [%(levelname)s] %(message)s [%(name)s::%(funcName)s]'
logging.basicConfig(format=FORMAT, level=logging.INFO)
hayaku_logger_name = 'hayaku'
hayaku_logger = logging.getLogger(hayaku_logger_name)

_usrdir = os.path.expanduser("~")
if not os.path.lexists(f"{_usrdir}/.hayaku"):
    os.makedirs(f"{_usrdir}/.hayaku")
_logfile = logging.handlers.RotatingFileHandler(
    f"{_usrdir}/.hayaku/hayaku_py.log", maxBytes=10240, backupCount=3, encoding="utf-8")
_logfile.setFormatter(logging.Formatter(FORMAT))
_logfile.setLevel(logging.WARN)
hayaku_logger.addHandler(_logfile)

g_hayaku_logger_lock = multiprocessing.Lock()


def set_my_logger_file(file_name):
    global _logfile
    with g_hayaku_logger_lock:
        hayaku_logger.removeHandler(_logfile)
        _logfile = logging.handlers.RotatingFileHandler(
            file_name, maxBytes=10240, backupCount=3, encoding="utf-8")
        _logfile.setFormatter(logging.Formatter(FORMAT))
        _logfile.setLevel(logging.WARN)
        hayaku_logger.addHandler(_logfile)


def get_default_logger():
    return logging.getLogger(hayaku_logger_name)


def class_logger(cls, enable=False):
    # logger = logging.getLogger("{}.{}".format(cls.__module__, cls.__name__))
    logger = logging.getLogger("{}".format(cls.__name__))
    if enable == 'debug':
        logger.setLevel(logging.DEBUG)
    elif enable == 'info':
        logger.setLevel(logging.INFO)
    cls.logger = logger


def add_class_logger_handler(class_list, level=logging.INFO, handler=None):
    """Add a log handler to the specified classes and set the level

    :param class_list: the class list
    :param level: the log level
    :param handler: logging handler
    """
    for cls in class_list:
        # logger = logging.getLogger("{}.{}".format(cls.__module__, cls.__name__))
        logger = logging.getLogger("{}".format(cls.__name__))
        if handler:
            logger.addHandler(handler)
        logger.setLevel(level)


def hayaku_debug(msg, *args, **kwargs):
    st = traceback.extract_stack()[-2]
    logger = kwargs.pop("logger") if "logger" in kwargs else None
    if logger:
        logger.debug("{} [{}] ({}:{})".format(msg.format(*args, **kwargs), st.name, st.filename, st.lineno))
    else:
        hayaku_logger.debug("{} [{}] ({}:{})".format(msg.format(*args, **kwargs), st.name, st.filename, st.lineno))


hayaku_trace = hayaku_debug


def hayaku_info(msg, *args, **kwargs):
    st = traceback.extract_stack()[-2]
    logger = kwargs.pop("logger") if "logger" in kwargs else None
    if logger is not None:
        logger.info("{} [{}] ({}:{})".format(msg.format(*args, **kwargs), st.name, st.filename, st.lineno))
    else:
        hayaku_logger.info("{} [{}] ({}:{})".format(msg.format(*args, **kwargs), st.name, st.filename, st.lineno))


def hayaku_warn(msg, *args, **kwargs):
    st = traceback.extract_stack()[-2]
    logger = kwargs.pop("logger") if "logger" in kwargs else None
    if logger:
        logger.warning("{} [{}] ({}:{})".format(msg.format(*args, **kwargs), st.name, st.filename, st.lineno))
    else:
        with g_hayaku_logger_lock:
            hayaku_logger.warning("{} [{}] ({}:{})".format(msg.format(*args, **kwargs), st.name, st.filename, st.lineno))


def hayaku_error(msg, *args, **kwargs):
    st = traceback.extract_stack()[-2]
    logger = kwargs.pop("logger") if "logger" in kwargs else None
    if logger:
        logger.error("{} [{}] ({}:{})".format(msg.format(*args, **kwargs), st.name, st.filename, st.lineno))
    else:
        with g_hayaku_logger_lock:
            hayaku_logger.error("{} [{}] ({}:{})".format(msg.format(*args, **kwargs), st.name, st.filename, st.lineno))


def hayaku_fatal(msg, *args, **kwargs):
    st = traceback.extract_stack()[-2]
    logger = kwargs.pop("logger") if "logger" in kwargs else None
    if logger:
        logger.critical("{} [{}] ({}:{})".format(msg.format(*args, **kwargs), st.name, st.filename, st.lineno))
    else:
        with g_hayaku_logger_lock:
            hayaku_logger.critical("{} [{}] ({}:{})".format(msg.format(*args, **kwargs), st.name, st.filename, st.lineno))


def hayaku_debug_if(exp, msg, *args, **kwargs):
    if exp:
        st = traceback.extract_stack()[-2]
        logger = kwargs.pop("logger") if "logger" in kwargs else None
        callback = kwargs.pop("callback") if "callback" in kwargs else None
        if logger:
            logger.info("{} [{}] ({}:{})".format(msg.format(*args, **kwargs), st.name, st.filename, st.lineno))
        else:
            hayaku_logger.info("{} [{}] ({}:{})".format(msg.format(*args, **kwargs), st.name, st.filename, st.lineno))
        if callback:
            callback()


hayaku_trace_if = hayaku_debug_if


def hayaku_info_if(exp, msg, *args, **kwargs):
    if exp:
        st = traceback.extract_stack()[-2]
        logger = kwargs.pop("logger") if "logger" in kwargs else None
        callback = kwargs.pop("callback") if "callback" in kwargs else None
        if logger:
            logger.info("{} [{}] ({}:{})".format(msg.format(*args, **kwargs), st.name, st.filename, st.lineno))
        else:
            hayaku_logger.info("{} [{}] ({}:{})".format(msg.format(*args, **kwargs), st.name, st.filename, st.lineno))
        if callback:
            callback()


def hayaku_warn_if(exp, msg, *args, **kwargs):
    if exp:
        st = traceback.extract_stack()[-2]
        logger = kwargs.pop("logger") if "logger" in kwargs else None
        callback = kwargs.pop("callback") if "callback" in kwargs else None
        if logger:
            logger.warning("{} [{}] ({}:{})".format(msg.format(*args, **kwargs), st.name, st.filename, st.lineno))
        else:
            with g_hayaku_logger_lock:
                hayaku_logger.warning("{} [{}] ({}:{})".format(msg.format(
                    *args, **kwargs), st.name, st.filename, st.lineno))
        if callback:
            callback()


def hayaku_error_if(exp, msg, *args, **kwargs):
    if exp:
        st = traceback.extract_stack()[-2]
        logger = kwargs.pop("logger") if "logger" in kwargs else None
        callback = kwargs.pop("callback") if "callback" in kwargs else None
        if logger:
            logger.error("{} [{}] ({}:{})".format(msg.format(*args, **kwargs), st.name, st.filename, st.lineno))
        else:
            with g_hayaku_logger_lock:
                hayaku_logger.error("{} [{}] ({}:{})".format(msg.format(*args, **kwargs), st.name, st.filename, st.lineno))
        if callback:
            callback()


def hayaku_fatal_if(exp, msg, *args, **kwargs):
    if exp:
        st = traceback.extract_stack()[-2]
        logger = kwargs.pop("logger") if "logger" in kwargs else None
        callback = kwargs.pop("callback") if "callback" in kwargs else None
        if logger:
            logger.critical("{} [{}] ({}:{})".format(msg.format(*args, **kwargs), st.name, st.filename, st.lineno))
        else:
            with g_hayaku_logger_lock:
                hayaku_logger.critical("{} [{}] ({}:{})".format(
                    msg.format(*args, **kwargs), st.name, st.filename, st.lineno))
        if callback:
            callback()


# Trace the function running
def with_trace(level=logging.INFO):
    def with_trace_wrap(func):
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            hayaku_info('start run: %s' % func.__name__)
            result = func(*args, **kwargs)
            hayaku_info('completed: %s' % func.__name__)
            return result

        return wrapper

    return with_trace_wrap


def capture_multiprocess_all_logger(queue, level=None):
    """Reset the logger output of all the subprocesses to the specified queue, and reset the level

    @param multiprocessing.Queue queue the specified mp Queue
    @param level the log output level; None keeps the original level
    """
    if queue is None:
        return
    qh = logging.handlers.QueueHandler(queue)
    for name in logging.Logger.manager.loggerDict.keys():
        logger = logging.getLogger(name)
        logger.addHandler(qh)
        if level is not None:
            logger.setLevel(level)

# Temporary change logger level
# https://docs.python.org/3/howto/logging-cookbook.html


class LoggingContext:
    def __init__(self, logger, level=None, handler=None, close=True):
        self.logger = logger
        self.level = level
        self.handler = handler
        self.close = close

    def __enter__(self):
        if self.level is not None:
            self.old_level = self.logger.level
            self.logger.setLevel(self.level)
        if self.handler:
            self.logger.addHandler(self.handler)

    def __exit__(self, et, ev, tb):
        if self.level is not None:
            self.logger.setLevel(self.old_level)
        if self.handler:
            self.logger.removeHandler(self.handler)
        if self.handler and self.close:
            self.handler.close()
