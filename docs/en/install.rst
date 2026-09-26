Installation
============

Use Python 3.10 or newer. Install the published package with:

.. code-block:: shell

    python -m pip install hayaku

Check the Python package and its native core:

.. code-block:: shell

    python -c "import hayaku; print(hayaku.__version__)"

Market data is configured separately. Opening a Session requires a local
``hayaku.ini`` and a compatible data source; see :ref:`quickstart`.
Optional ingest and realtime capabilities require their corresponding native
modules and dependencies.

Build from source
-----------------

From a Git checkout, install Python dependencies and use the repository
wrapper for a local Python 3.10 build:

.. code-block:: shell

    python3.10 -m pip install -r requirements.txt
    ./op.sh configure
    ./op.sh build
    ./op.sh import-test
    ./op.sh test

The wrapper's selected paths are shown by ``./op.sh doctor``. Set
``PYTHON_PREFIX``, ``PYTHON_BIN`` or ``XMAKE_BIN`` when those tools are
installed elsewhere. Developer build details are in :ref:`developer`.
