
.. note::

    为了顺利编译代码， 请勿使用从 github 直接下载源码包的方式编译。 原因是 git 上传时部分文件的换行符被置换为Linux式的换行符，将导致直接下载的部分代码在Windows下无法顺利编译。

C++ 源码使用 Xmake 构建。在仓库根目录运行 ``./op.sh configure`` 和
``./op.sh build``；使用 ``./op.sh test`` 执行 C++ 与 Python 回归测试。
C++ 格式化和静态检查命令见仓库内的 ``tools/cpp-style.md``。

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

1、安装C++编译器
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

已开始逐步向 c++20 迁移，编译需要支持 c++20 语言特性。

- Windows 平台: Visual C++ 2022
- Linux 平台: g++ > = 13 、 clang >= 15


2、安装构建工具 xmake
^^^^^^^^^^^^^^^^^^^^^^^^^^^

xmake >= 2.8.2，网址：`<https://github.com/xmake-io/xmake>`_

参见：`<https://xmake.io/#/zh-cn/guide/installation>`_


3、克隆 Hayaku 源码
^^^^^^^^^^^^^^^^^^^^^^^^

执行以下命令克隆 hayaku 源码：（请勿在中文目录下克隆代码）

.. code-block:: shell

    git clone https://github.com/larrystd/hayaku-quant.git

.. note::

    **捐赠用户如需使用插件，请安装 hayaku_plugin 包: pip install hayaku_plugin**

    如最新代码使用插件发生崩溃，建议 checkout release 分支或对应版本分支进行编译。


4、Linux下安装依赖软件包
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Linux下需安装依赖的开发软件包。如 Ubuntu 下，执行以下命令：

.. code-block:: shell
    
    sudo apt-get install -y libsqlite3-dev   


5、macOS 下安装 Xcode 命令行工具
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

编译前请安装 Xcode 及其命令行工具。
    

编译与安装
------------

1. 安装 python 依赖包
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: shell

    pip install -r requirements.txt  或 pip install -r requirements.txt -U  (定期升级依赖包)


2. 编译
^^^^^^^^^^

.. note::

    **注意** ：长时间没编译，更新代码后进行重编译前，请先执行 python setup.py clear 彻底清除之前的编译缓存。并更新 python 依赖，pip install -r requirements.txt


进入源码目录下，执行 python setup.py build -j 10 , 其他支持的 command：

- python setup.py help        -- 查看帮助
- python setup.py build       -- 执行编译
- python setup.py install     -- 编译并执行安装（安装到 python 的 site-packages 目录下）
- python setup.py uninstall   -- 删除已安装的Hayaku
- python setup.py test        -- 执行单元测试（可带参数 --compile=1，先执行编译）
- python setup.py clear       -- 清除本地编译结果
- python setup.py wheel       -- 生成wheel安装包


各命令参数，可以 执行 python setup.py commond --help 查看, 如: python setup.py build --help



3. 设置 PYTHONPATH 环境变量
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Linux 下如修改 ~/.bashrc 文件，在末尾添加如下内容 （指向源码目录） ：

.. code-block:: shell

    export PYTHONPATH=/path/to/hayaku:$PYTHONPATH


4、Windows 下转 Visual Studio 工程
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

请先使用 python setup.py build 直接编译过一次后，在转换。

Windows 下，习惯用 msvc 调试的，可以使用  xmake project -k vsxmake -m "debug,release" 命令生成 VS 工程。命令执行后，会在当前目录下生成如 vsxmake2022 的子目录，VS工程位于其内。

在 VS 内，可以将 demo 设为启动工程，进行调试。


5、IDE无法正常提示
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. 安装 pybind11-stubgen，使用命令 pip install pybind11-stubgen
2. 运行 pybind11-stubgen hayaku -o . 命令，即可正常提示帮助信息。


6、使用插件
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
