C++ 源码在 macOS 和 Linux 上使用 Bazel 构建。在仓库根目录运行
``./op.sh build`` 和 ``./op.sh test``。C++ 格式化与静态检查命令见
``tools/cpp-style.md``。

Python 包结构
-------------

``hayaku_pywrap`` 按八个 C++ 业务域组织绑定，``core``、``ingest`` 和
``realtime`` 原生模块各有独立源码清单。Python 包对应八个顶层业务域：
``hayaku.common``、``hayaku.data``、``hayaku.operators``、
``hayaku.execution``、``hayaku.metrics``、``hayaku.strategy``、
``hayaku.application`` 和 ``hayaku.extensions``。``hayaku.data`` 提供行情查询
和值类型；``hayaku.operators`` 提供指标公式，``hayaku.metrics`` 提供结果转换。
数据源、存储后端、导入作业及 schema 资源位于 ``hayaku.extensions.ingest``。
普通 ``import hayaku`` 不启动数据会话；运行时通过
``hayaku.application.session.open_session`` 显式打开。实时控制从
``hayaku.extensions.realtime`` 导入，绘图从
``hayaku.extensions.visualization`` 导入。GUI 与命令行模块分别位于
``hayaku.application.gui`` 和 ``hayaku.application.cli``；交互探索位于
``hayaku.application.interactive``，配置与 hub 辅助模块位于
``hayaku.application.config`` 和 ``hayaku.application.hub``。

Python 测试与示例位于仓库根目录的 ``tests/python`` 和 ``examples/python``，
不随安装包分发。使用 ``python3 tests/python/test.py`` 执行回归测试；教程
Notebook 位于 ``examples/python/notebook``。旧路径 ``hayaku.advanced``、
``hayaku.draw``、``hayaku.gui``、``hayaku.shell``、``hayaku.interactive``、
``hayaku.hub``、``hayaku.config``、``hayaku.test`` 和 ``hayaku.examples``
已删除。之前的内部路径 ``hayaku.fetcher``、``hayaku.util``、
``hayaku.flat``、``hayaku.extend`` 和 ``hayaku.gui.data`` 也已删除。
Step 6C 又删除了旧路径 ``hayaku.indicator``、``hayaku.analysis``、
``hayaku.apps``、``hayaku.session``、``hayaku.ingest``、
``hayaku.realtime``、``hayaku.visualization`` 和 ``hayaku.spi``。

.. _developer:


编译前准备
----------------

安装 Bazelisk、CMake、支持 C++20 的编译器及 Python 3.10。Bazelisk 从
``.bazelversion`` 读取固定的 Bazel 版本。克隆源码：

.. code-block:: shell

    git clone https://github.com/larrystd/hayaku-quant.git
    cd hayaku-quant

Bazel 配置支持 macOS 和 Linux。原生依赖固定在 ``MODULE.bazel`` 及锁文件中；
构建目标详见 ``BAZEL.md``。

编译与安装
------------

.. code-block:: shell

    python3.10 -m pip install -r requirements.txt
    ./op.sh build
    ./op.sh import-test
    ./op.sh test
    ./op.sh python-test

``./op.sh build`` 会把 Python 3.10 原生模块放入源码包目录。执行
``./op.sh wheel``、``./op.sh wheel-ingest`` 和
``./op.sh wheel-realtime`` 可分别制作核心与可选扩展的 wheel。

设置 PYTHONPATH 环境变量
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Linux 下如修改 ~/.bashrc 文件，在末尾添加如下内容 （指向源码目录） ：

.. code-block:: shell

    export PYTHONPATH=/path/to/hayaku:$PYTHONPATH


IDE 无法正常提示
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. 安装 pybind11-stubgen，使用命令 pip install pybind11-stubgen
2. 运行 pybind11-stubgen hayaku -o . 命令，即可正常提示帮助信息。


使用插件
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

如自行编译的希望使用 hayaku 插件的，请安装独立的插件包 pip install hayaku-plugin

但请注意插件版本需要配套，最好使用 release 分支（或标签）进行编译，避免版本不匹配无法使用。


Docker 构建
------------

源码 docker 目录下，提供了基于 Ubuntu/Debain/Fedora 的 Dockerfile_dev 文件，可以用来快速构建 Hayaku 的编译环境。

.. code-block:: shell

    cd docker
    docker build -t hayaku_dev -f Dockerfile_dev .

    docker run -it hayaku_dev /bin/bash

进入 hayaku 目录下，其他与源码编译步骤一致。

也可以使用基于 pip 安装 Hayaku 的 dockerfile, 见 /docker/Dockerfile_miniconda 。

Hayaku 使用前需要导入数据，Docker 镜像不包含界面，可以执行 ``python -m hayaku.application.gui.importdata`` 命令导入数据。

hayaku 配置文件在 /root/.hayaku 目录下, 数据文件存储(HDF5)在 /root/stocks 目录下，可自行在创建docker容器时指定挂载目录。
