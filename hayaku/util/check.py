#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

# ==========================================================
# History
# 1. 20200821, Added by fasiondog
# ==========================================================

import sys
import traceback
import functools
import asyncio
from .mylog import hayaku_logger, g_hayaku_logger_lock


class HAYAKUCheckError(Exception):
    def __init__(self, expression, message):
        self.expression = expression
        self.message = message

    def __str__(self):
        return str(self.message)


class HAYAKUIngoreError(Exception):
    def __init__(self, expression, message=None):
        self.expression = expression
        self.message = message if message is not None else 'ignore'

    def __str__(self):
        return str(self.message)


def checkif(expression, message, excepion=None, **kwargs):
    """Raise an exception if the expression is True. Note: the judgment of this function is the opposite of assert.

    :param boolean expression: the judgment condition
    :param str message: the exception annotation message
    :param Exception exception: the specified exception class; when None, the default HAYAKUCheckError exception is used
    """
    if expression:
        if excepion is None:
            raise HAYAKUCheckError(expression, message)
        else:
            raise excepion(message, **kwargs)


def hayaku_check(exp, msg, *args, **kwargs):
    if not exp:
        st = traceback.extract_stack()[-2]
        check_exp = st._line.split(',')[0]
        errmsg = "{}) {} [{}] [{}:{}]".format(check_exp, msg.format(*args, **kwargs), st.name, st.filename, st.lineno)
        raise HAYAKUCheckError(exp, errmsg)


def hayaku_check_throw(expression, message, excepion=None, **kwargs):
    """Raise an exception if the expression is False.

    :param boolean expression: the judgment condition
    :param str message: the exception annotation message
    :param Exception exception: the specified exception class; when None, the default HAYAKUCheckError exception is used
    """
    if not expression:
        st = traceback.extract_stack()[-2]
        check_exp = st._line.split(',')[0]
        errmsg = "{}) {} [{}] [{}:{}]".format(check_exp, message, st.name, st.filename, st.lineno)
        if excepion is None:
            raise HAYAKUCheckError(expression, errmsg)
        else:
            raise excepion(errmsg, **kwargs)


def hayaku_check_ignore(exp, *args, **kwargs):
    """An ignorable check"""
    if not exp:
        st = traceback.extract_stack()[-2]
        check_exp = st._line.split(',')[0]
        msg = kwargs.pop("msg") if "msg" in kwargs else ''
        if msg:
            errmsg = "{}) {} [{}] [{}:{}]".format(
                check_exp, msg.format(*args, **kwargs), st.name, st.filename, st.lineno
            )
        elif args:
            msg = args[0]
            errmsg = "{}) {} [{}] [{}:{}]".format(
                check_exp, msg.format(*args[1:], **kwargs), st.name, st.filename, st.lineno
            )
        raise HAYAKUIngoreError(exp, errmsg)


def get_exception_info():
    info = sys.exc_info()
    return "{}: {}".format(info[0].__name__, info[1])


def hayaku_catch(ret=None, trace=False, callback=None, retry=1, with_msg=False, re_raise=False):
    """Catch the raised exceptions, wrapped as: @hayaku_catch()
    :param ret: the return value when an exception occurs; when with_msg is True, the return is (ret, errmsg)
    :param boolean trace: print the exception stack information
    :param func callback: the callback function after an exception occurs, with the same input as func
    :param int retry: the number of the execution attempts
    :param boolean with_msg: whether to return the exception error message; when True, the function returns (ret, errmsg)
    :param boolean re_raise: whether to raise the error message again as an exception
    """
    def hayaku_catch_wrap(func):
        @functools.wraps(func)
        def wrappedFunc(*args, **kargs):
            for i in range(retry):
                errmsg = ""
                try:
                    val = func(*args, **kargs)
                    return (val, errmsg) if with_msg else val
                except HAYAKUIngoreError:
                    errmsg = "{} [{}.{}]".format(get_exception_info(), func.__module__, func.__name__)
                    hayaku_logger.debug(errmsg)
                except Exception:
                    errmsg = "{} [{}.{}]".format(get_exception_info(), func.__module__, func.__name__)
                    with g_hayaku_logger_lock:
                        hayaku_logger.error(errmsg)
                        if trace:
                            hayaku_logger.error(traceback.format_exc())
                    if i == (retry - 1):
                        if callback is not None:
                            callback(*args, **kargs)
                        if re_raise:
                            raise Exception(errmsg)
                except KeyboardInterrupt:
                    raise KeyboardInterrupt()
                except:
                    errmsg = "Unknown error! {} [{}.{}]".format(get_exception_info(), func.__module__, func.__name__)
                    with g_hayaku_logger_lock:
                        hayaku_logger.error(errmsg)
                        if trace:
                            hayaku_logger.error(traceback.format_exc())
                    if i == (retry - 1):
                        if callback is not None:
                            callback(*args, **kargs)
                        if re_raise:
                            raise Exception(errmsg)
                return (ret, errmsg) if with_msg else ret

        return wrappedFunc

    return hayaku_catch_wrap


def hayaku_to_async(func):
    @functools.wraps(func)
    async def async_func(*args, **kwargs):
        loop = asyncio.get_event_loop()
        return await loop.run_in_executor(None, func, *args, **kwargs)
    return async_func


def hayaku_run_ignore_exception(func, *args, **kwargs):
    """Run the function and ignore the exceptions"""
    try:
        return func(*args, **kwargs)
    except:
        return None
