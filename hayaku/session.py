#!/usr/bin/python
# -*- coding: utf8 -*-

"""Explicit Hayaku runtime session helpers."""

from pathlib import Path

from .core import HayakuSession, StrategyContext


def open_session(filename=None, ignore_preload=False, context=None, account_config=None):
    """Open a Hayaku runtime session.

    The returned object is a context manager. Pass an ``AccountConfig`` to install the native
    execution engine while opening the session. Closing it invalidates its scoped engine handles;
    closing the final session also releases the internal data runtime.
    """
    if filename is None:
        filename = Path.home() / ".hayaku" / "hayaku.ini"
    if context is None:
        context = StrategyContext(["all"])
    filename = str(Path(filename).expanduser())
    if account_config is None:
        return HayakuSession.open(filename, ignore_preload, context)
    return HayakuSession.open(filename, account_config, ignore_preload, context)
