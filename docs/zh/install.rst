安装
====

使用 Python 3.10 或更新版本。安装已发布的 Python 包：

.. code-block:: shell

    python -m pip install hayaku

检查 Python 包及其原生 core：

.. code-block:: shell

    python -c "import hayaku; print(hayaku.__version__)"

行情数据需要另外配置。打开 Session 前应准备本地 ``hayaku.ini`` 和兼容的
数据源，参见 :ref:`quickstart`。可选的 ingest、realtime 能力还需要对应的
原生模块和依赖。

从源码构建
----------

在 Git 工作目录中安装 Python 依赖，并使用仓库脚本完成本地 Python 3.10 构建：

.. code-block:: shell

    python3.10 -m pip install -r requirements.txt
    ./op.sh configure
    ./op.sh build
    ./op.sh import-test
    ./op.sh test

运行 ``./op.sh doctor`` 可查看脚本选用的工具路径。工具安装在其他位置时，
可设置 ``PYTHON_PREFIX``、``PYTHON_BIN`` 或 ``XMAKE_BIN``。开发构建说明见
:ref:`developer`。
