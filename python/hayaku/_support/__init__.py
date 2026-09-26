#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

# ===============================================================================
# Author: fasiondog
# History: 1) 20090527, Added by fasiondog
# ===============================================================================

# from singleton import Singleton

from .logging import *
from .checks import *
from .timeout import *
from .notebook import *

__all__ = [
    'spend_time',
    'hayaku_benchmark',
    'timeout',
    'hayaku_logger',
    'class_logger',
    'add_class_logger_handler',
    'HAYAKUCheckError',
    'hayaku_check',
    'hayaku_check_throw',
    'hayaku_check_ignore',
    'hayaku_catch',
    'hayaku_to_async',
    "hayaku_run_ignore_exception",
    'hayaku_trace',
    'hayaku_debug',
    'hayaku_info',
    'hayaku_warn',
    'hayaku_error',
    'hayaku_fatal',
    'hayaku_trace_if',
    'hayaku_debug_if',
    'hayaku_info_if',
    'hayaku_warn_if',
    'hayaku_info_if',
    'hayaku_warn_if',
    'hayaku_error_if',
    'hayaku_fatal_if',
    'with_trace',
    'set_my_logger_file',
    'capture_multiprocess_all_logger',
    'LoggingContext',
    'in_interactive_session',
    'in_ipython_frontend',
]
