

.. note::

    For a successful build, do not compile from a source package downloaded directly from GitHub: when files are uploaded to git, some line endings are converted to Linux-style line endings, so parts of a directly downloaded source package may fail to compile on Windows.

The C++ source is built with Xmake. From the repository root, use
``./op.sh configure`` followed by ``./op.sh build``. Run ``./op.sh test``
for the C++ and Python regression suites. C++ formatting and static-analysis
commands are documented in ``tools/cpp-style.md`` in the repository.

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

1. Install a C++ compiler
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The codebase is gradually migrating to C++20, so a compiler that supports the C++20 language features is required.

- Windows: Visual C++ 2022
- Linux: g++ >= 13, clang >= 15


2. Install the xmake build tool
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

xmake >= 2.8.2. Website: `<https://github.com/xmake-io/xmake>`_

See: `<https://xmake.io/#/zh-cn/guide/installation>`_


3. Clone the Hayaku source code
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Run the following command to clone the Hayaku source code (do not clone into a directory whose path contains Chinese characters):

.. code-block:: shell

    git clone https://github.com/larrystd/hayaku-quant.git

.. note::

    **Donor users who need the plugin should install the hayaku_plugin package: pip install hayaku_plugin**

    If the plugin crashes when used with the latest code, check out the release branch or the corresponding version branch and build from that.


4. Install the dependency packages on Linux
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

On Linux, the required development packages must be installed. On Ubuntu, for example, run:

.. code-block:: shell
    
    sudo apt-get install -y libsqlite3-dev   


5. Install the Xcode command-line tools on macOS
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Install Xcode and its command-line tools before building.
    

Building and Installing
-----------------------

1. Install the Python dependencies
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: shell

    pip install -r requirements.txt  or pip install -r requirements.txt -U  (upgrade the dependencies periodically)


2. Build
^^^^^^^^^^

.. note::

    **Note**: if you have not built for a while, run python setup.py clear before rebuilding after updating the code, so that the previous build cache is fully cleared. Also update the Python dependencies: pip install -r requirements.txt


From the source directory, run python setup.py build -j 10. Other supported commands:

- python setup.py help        -- show the help
- python setup.py build       -- run the build
- python setup.py install     -- build and install (into Python's site-packages directory)
- python setup.py uninstall   -- remove the installed Hayaku
- python setup.py test        -- run the unit tests (optionally pass --compile=1 to build first)
- python setup.py clear       -- clear the local build artifacts
- python setup.py wheel       -- generate a wheel package


For the options of each command, run python setup.py <command> --help, for example: python setup.py build --help



3. Set the PYTHONPATH environment variable
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

On Linux, for example, add the following line to the end of ~/.bashrc (pointing to the source directory):

.. code-block:: shell

    export PYTHONPATH=/path/to/hayaku:$PYTHONPATH


4. Generate a Visual Studio project on Windows
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Run a complete build with python setup.py build first, and generate the project only afterwards.

On Windows, if you prefer to debug with MSVC, run xmake project -k vsxmake -m "debug,release" to generate a Visual Studio project. After the command finishes, a subdirectory such as vsxmake2022 is created in the current directory, and the Visual Studio project is inside it.

In Visual Studio, you can set the demo as the startup project for debugging.


5. IDE code hints do not work
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. Install pybind11-stubgen with pip install pybind11-stubgen
2. Run pybind11-stubgen hayaku -o .; code hints and help information will then work correctly.


6. Using the plugin
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
