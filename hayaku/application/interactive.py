"""Opt-in broad research environment.

Unlike importing :mod:`hayaku`, importing this module intentionally loads indicator and strategy
helpers and opens the default data session.
"""

from pathlib import Path

from hayaku.metrics import *
from hayaku.core import *
from hayaku.common._extensions import *
from hayaku.data._extensions import *
from hayaku.operators import *
from hayaku.execution import *
from hayaku.application.session import open_session
from hayaku.strategy import *


session = None
data = None


def load_hayaku(config_file=None, ignore_preload=False, context=None):
    """Open and retain the interactive data session, returning it to the caller."""

    global session, data
    if config_file is None:
        config_file = Path.home() / ".hayaku" / "hayaku.ini"
        if not config_file.exists():
            from hayaku.application.config.generator import generate_default_config
            generate_default_config()
    if session is not None and session.opened:
        session.close()
    session = open_session(config_file, ignore_preload=ignore_preload, context=context)
    session.wait_ready()
    data = session.data
    return session


load_hayaku()
