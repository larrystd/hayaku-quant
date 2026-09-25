"""Opt-in broad research environment.

Unlike importing :mod:`hikyuu`, importing this module intentionally loads indicator and strategy
helpers and opens the default data session.
"""

from pathlib import Path

from hikyuu.analysis import *
from hikyuu.core import *
from hikyuu.extend import *
from hikyuu.indicator import *
from hikyuu.execution import *
from hikyuu.session import open_session
from hikyuu.strategy import *


session = None
data = None


def load_hikyuu(config_file=None, ignore_preload=False, context=None):
    """Open and retain the interactive data session, returning it to the caller."""

    global session, data
    if config_file is None:
        config_file = Path.home() / ".hikyuu" / "hikyuu.ini"
        if not config_file.exists():
            from hikyuu.data.hku_config_template import generate_default_config
            generate_default_config()
    if session is not None and session.opened:
        session.close()
    session = open_session(config_file, ignore_preload=ignore_preload, context=context)
    session.wait_ready()
    data = session.data
    return session


load_hikyuu()
