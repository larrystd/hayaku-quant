开发者指南
==========

.. _developer:

Hayaku 使用 Xmake 构建 C++ 库和 Python 扩展。core、ingest 和 realtime 是独立
目标；后两者在运行时是可选的。构建需要支持 C++20 的编译器、Xmake 3.0.8，以及
带开发头文件的 Python。直接使用 Xmake 时，可通过 ``HAYAKU_PYTHON`` 指定扩展
模块对应的 Python 可执行文件。

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

macOS 本地流程
--------------

``op.sh`` 默认选择 Homebrew Python 3.10。在仓库根目录执行：

.. code-block:: shell

   ./op.sh configure
   ./op.sh build
   ./op.sh build-optional
   ./op.sh test

``build`` 构建 core 扩展；``build-optional`` 构建 ingest 和 realtime 扩展。
``test`` 在运行 C++ 与 Python 测试前构建 C++ 测试目标和两个可选扩展。可使用
``./op.sh doctor`` 查看工具链和产物路径。

Linux 本地流程
--------------

先安装 C++20 编译器、Python 开发头文件，以及当前 Xmake 选项所需的系统依赖，
再执行：

.. code-block:: shell

   export HAYAKU_PYTHON="$(command -v python3)"
   xmake f -m release -k shared --feedback=n -y
   xmake -b core
   xmake -b ingest
   xmake -b realtime
   xmake r small-test
   xmake r unit-test
   python3 tests/python/test.py

代码质量
--------

全仓 C++ Google 格式检查和静态分析使用 LLVM 20。源码清单与编译数据库生成方法见
``tools/cpp-style.md``。

.. code-block:: shell

   ./op.sh fmt-check
   ./op.sh asan-test

ASan 命令在 ``build/asan`` 中构建项目目标，退出时恢复原 Xmake 配置。之后应
重新构建普通 Python 扩展，再运行未启用 sanitizer 的 Python。macOS 的 ASan
运行不包含 LeakSanitizer；Linux 在运行时支持时还检查泄漏。
