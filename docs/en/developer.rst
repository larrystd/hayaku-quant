The C++ source is built with Bazel on macOS and Linux. From the repository
root, run ``./op.sh build`` and ``./op.sh test``. C++ formatting and static
analysis commands are documented in ``tools/cpp-style.md``.

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

.. _developer:


Build Prerequisites
-------------------

Install Bazelisk, CMake, a C++20 compiler and Python 3.10. Bazelisk reads the
pinned Bazel version from ``.bazelversion``. Clone the source repository:

.. code-block:: shell

    git clone https://github.com/larrystd/hayaku-quant.git
    cd hayaku-quant

The Bazel configuration supports macOS and Linux. Native dependencies are
pinned in ``MODULE.bazel`` and the lockfile. See ``BAZEL.md`` for target details.

Building and Installing
-----------------------

.. code-block:: shell

    python3.10 -m pip install -r requirements.txt
    ./op.sh build
    ./op.sh import-test
    ./op.sh test
    ./op.sh python-test

``./op.sh build`` stages the Python 3.10 native modules in the source tree.
Use ``./op.sh wheel``, ``./op.sh wheel-ingest`` and
``./op.sh wheel-realtime`` to package the core and optional modules.

Set the PYTHONPATH environment variable
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

On Linux, for example, add the following line to the end of ~/.bashrc (pointing to the source directory):

.. code-block:: shell

    export PYTHONPATH=/path/to/hayaku:$PYTHONPATH


IDE code hints do not work
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. Install pybind11-stubgen with pip install pybind11-stubgen
2. Run pybind11-stubgen hayaku -o .; code hints and help information will then work correctly.


Using the plugin
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you build from source and want to use the Hayaku plugin, install the standalone plugin package: pip install hayaku-plugin

Note that the plugin version must match your build: it is best to build from the release branch (or a release tag), so that a version mismatch does not render the plugin unusable.


Docker Build
------------

The docker directory in the source tree contains Dockerfile_dev files based on Ubuntu, Debian and Fedora, which can be used to quickly set up a Hayaku build environment.

.. code-block:: shell

    cd docker
    docker build -t hayaku_dev -f Dockerfile_dev .

    docker run -it hayaku_dev /bin/bash

Enter the hayaku directory; the remaining steps are the same as the source build instructions above.

There is also a Dockerfile that installs Hayaku via pip; see /docker/Dockerfile_miniconda .

Hayaku requires data to be imported before use. The Docker image does not include the GUI; run ``python -m hayaku.application.gui.importdata`` to import the data.

The Hayaku configuration file is located in /root/.hayaku, and the data files (HDF5) are stored in /root/stocks; you can specify your own mount directories when creating the Docker container.
