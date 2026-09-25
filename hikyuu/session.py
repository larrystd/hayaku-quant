#!/usr/bin/python
# -*- coding: utf8 -*-

"""Explicit Hikyuu runtime session helpers."""

from pathlib import Path

from .core import HikyuuSession, StrategyContext


def open_session(filename=None, ignore_preload=False, context=None, account_config=None):
    """Open a Hikyuu runtime session.

    The returned object is a context manager. Pass an ``AccountConfig`` to install the native
    execution engine while opening the session. Closing it invalidates its scoped engine handles;
    closing the final session also releases the internal data runtime.
    """
    if filename is None:
        filename = Path.home() / ".hikyuu" / "hikyuu.ini"
    if context is None:
        context = StrategyContext(["all"])
    filename = str(Path(filename).expanduser())
    if account_config is None:
        return HikyuuSession.open(filename, ignore_preload, context)
    return HikyuuSession.open(filename, account_config, ignore_preload, context)
