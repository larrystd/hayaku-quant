Developer Guide
===============

.. _developer:

Hayaku uses Xmake for the C++ library and Python extensions. The core, ingest,
and realtime targets are separate; the last two are optional at runtime. Use
a C++20 compiler, Xmake 3.0.8, and a Python installation with development
headers. Set ``HAYAKU_PYTHON`` to the Python executable used for the extension
modules when building with Xmake directly.

Python Package Layout
---------------------

The bindings under ``hayaku_pywrap`` follow the eight C++ domains. The
``core``, ``ingest``, and ``realtime`` native modules have separate source
lists. The Python domain roots are ``hayaku.common``, ``hayaku.data``,
``hayaku.operators``, ``hayaku.execution``, ``hayaku.metrics``,
``hayaku.strategy``, ``hayaku.application``, and ``hayaku.extensions``.
``hayaku.data`` exposes market data queries and value types;
``hayaku.operators`` provides indicator formulas; ``hayaku.metrics`` provides
result conversion helpers. Data sources, storage backends, import jobs, and
schema resources live in ``hayaku.extensions.ingest``. Importing ``hayaku``
does not start a data session; use ``hayaku.application.session.open_session``
for runtime work. Import realtime controls from ``hayaku.extensions.realtime``
and drawing from ``hayaku.extensions.visualization``. GUI and command-line
modules live in ``hayaku.application.gui`` and ``hayaku.application.cli``;
interactive exploration uses ``hayaku.application.interactive``. Configuration
and hub helpers live in ``hayaku.application.config`` and
``hayaku.application.hub``.

Python tests and examples are in ``tests/python`` and ``examples/python`` at
the repository root, outside the installed package. Run the regression suite
with ``python3 tests/python/test.py``. Tutorial notebooks are under
``examples/python/notebook``. The old paths ``hayaku.advanced``,
``hayaku.draw``, ``hayaku.gui``, ``hayaku.shell``, ``hayaku.interactive``,
``hayaku.hub``, ``hayaku.config``, ``hayaku.test``, and
``hayaku.examples`` have been removed. The previous internal paths
``hayaku.fetcher``, ``hayaku.util``, ``hayaku.flat``, ``hayaku.extend``,
and ``hayaku.gui.data`` are also removed.
The former ``hayaku.indicator``, ``hayaku.analysis``, ``hayaku.apps``,
``hayaku.session``, ``hayaku.ingest``, ``hayaku.realtime``,
``hayaku.visualization``, and ``hayaku.spi`` paths were removed in Step 6C.

macOS local workflow
--------------------

``op.sh`` selects the Homebrew Python 3.10 installation by default. From the
repository root:

.. code-block:: shell

   ./op.sh configure
   ./op.sh build
   ./op.sh build-optional
   ./op.sh test

``build`` creates the core extension. ``build-optional`` creates the ingest
and realtime extensions. ``test`` builds the C++ test targets and both
optional extensions before running the C++ and Python suites. Use
``./op.sh doctor`` to inspect the selected toolchain and artifact paths.

Linux local workflow
--------------------

Install a C++20 compiler, Python development headers, and the system packages
required by the enabled Xmake options. Then run:

.. code-block:: shell

   export HAYAKU_PYTHON="$(command -v python3)"
   xmake f -m release -k shared --feedback=n -y
   xmake -b core
   xmake -b ingest
   xmake -b realtime
   xmake r small-test
   xmake r unit-test
   python3 tests/python/test.py

Code quality
------------

Use LLVM 20 for the project-wide Google C++ format and the C++ static
analysis commands. The source list and compilation database instructions are
in ``tools/cpp-style.md``.

.. code-block:: shell

   ./op.sh fmt-check
   ./op.sh asan-test

The ASan command builds project targets in ``build/asan`` and restores the
prior Xmake configuration. Rebuild the ordinary Python extensions afterwards
before running a non-sanitized Python process. On macOS, ASan runs without
LeakSanitizer; Linux also checks leaks when supported by its runtime.
