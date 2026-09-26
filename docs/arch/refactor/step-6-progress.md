# 第 6 步：Python 绑定层与包结构收敛

> 状态：目标结构与边界已确定，尚未开始代码迁移
>
> 前置条件：第 5 步完成并通过结构、构建、API 与测试验收
>
> 决策日期：2026-09-26
>
> 本步范围：`hayaku_pywrap`、`hayaku`，以及因路径变化必须同步调整的构建、打包、测试、示例和中英文文档
>
> 品牌前置批次已于 2026-09-26 原子切换为 `hayaku`；本步直接以新包名、namespace、宏、库名和路径为基线，不再安排第二次改名

## 1. 核心结论

本步骤只解决 Python 暴露链路的物理结构和职责边界问题：

```text
hayaku_cpp/src
      │
      ▼
hayaku_pywrap              按 C++ 所有权组织绑定源码
      │
      ▼
core / ingest / realtime   保持三个现有原生模块边界
      │
      ▼
hayaku                     按 Python 用户接口组织公共包
      │
      ▼
GUI / CLI / draw / ingest  只作为显式使用的上层能力
```

两棵目标树采用不同的组织原则：

- `hayaku_pywrap` 是 C++ 适配层，必须镜像第 5 步确定的八个 C++ 业务域。
- `hayaku` 是 Python 用户接口，必须使用已经稳定的 Python 领域词汇，不能机械复制 C++ 目录名。
- 原生 `core` 仍是一个扁平 pybind11 模块；本步不拆成多个 Python 原生子模块。
- `ingest` 和 `realtime` 继续是可选原生模块，不重新并入 `core`。
- 绑定按“能力域”聚合，不按 C++ 类型逐文件镜像；默认一个领域只有一个主
  `Bindings.cpp`，从当前 82 个绑定 `.cpp` 收敛到约 15 个。
- `import hayaku` 继续保持小型、显式且无运行期副作用的公共入口。
- 本步不改变指标、策略、成交、数据导入和实时服务的业务语义。

## 2. 当前基线与主要问题

统计口径为当前 Git 跟踪文件，不包含 `__pycache__`、本地动态库和其他构建产物：

| 指标 | 当前值 |
|---|---:|
| `hayaku_pywrap` 跟踪文件 | 92 |
| 其中 `.cpp` / `.h` / 其他 | 82 / 9 / 1 |
| `hayaku` 跟踪文件 | 318 |
| 其中 `.py` / `.pyi` / `.sql` / 其他 | 159 / 1 / 71 / 87 |
| `hayaku` 顶层公共 API | 30 项 |
| 原生 Python 模块 | `core`、`ingest`、`realtime` |
| Step 5 pybind 导出清单 | 888 项 |

当前结构已经完成部分接口收敛，但物理归属仍停留在旧模型：

1. `hayaku_pywrap/data/indicator` 和 `hayaku_pywrap/data/factor` 实际绑定的是
   `operators`，仍挂在旧 `data` 目录下。
2. `hayaku_pywrap/analysis`、`app`、`advanced` 与 C++ 的
   `metrics`、`application`、`extensions` 不一致。
3. `ingest_main.cpp`、`realtime_main.cpp` 位于根目录，可选模块的入口和实现分离。
4. `advanced` 同时容纳应用插件、导入器和实时服务，无法表达唯一职责。
5. `hayaku/data` 一方面是 `DataEngine`、`Stock`、`KData` 的公共门面，另一方面又容纳
   数据下载、数据库连接、导入作业和 71 个 SQL 文件，存在明显的职责冲突。
6. `fetcher`、`flat`、`util`、`extend.py` 和 `gui/data` 是实现方式或历史命名，不能直接说明
   所属能力。
7. `main.cpp` 直接声明并编排大量叶级 `export_*` 函数，且存在
   `KReord`、`TimeLineReord`、`StrategeContext`、`Normlize`、
   `_KDataToMySQLImporte.cpp` 等内部拼写问题。
8. xmake 的 unity build 同时使用宽泛递归 glob 和子目录 glob，业务归属、编译目标归属与
   unity 分组没有形成一份清晰清单。

`trade_manage`、`trade_sys`、旧 Manager/System Python 门面已经在前序步骤删除，本步不得
重新建立同名兼容包。

## 3. 分层边界

### 3.1 依赖方向

本步完成后，依赖只能自下而上：

```text
C++ public headers
    ↓
pybind leaf bindings
    ↓
native module composition roots
    ↓
Python domain facades
    ↓
opt-in features and applications
```

禁止出现以下反向依赖：

- C++ 或 pybind11 代码依赖 `hayaku` Python 包。
- `hayaku.common/data/execution/strategy` 依赖 GUI、绘图、抓取或数据库导入作业。
- `hayaku.__init__` 导入 pandas、matplotlib、SQLAlchemy、PyQt、数据抓取器或可选原生模块。
- `hayaku.ingest` 的普通导入立即连接数据库、加载具体导入插件或启动线程。
- GUI、CLI、示例直接依赖 `hayaku.core` 中的内部 Runtime/Ledger 类型。

### 3.2 “物理归属”与“发布目标”分离

绑定文件的目录由它绑定的 C++ 能力决定，编译进哪个原生模块由显式 target 清单决定。
例如：

- `SpotRecord` 物理归属 `extensions/realtime`，但为保持类型注册关系可以继续编译进
  `core`。
- `SpotAgent`、data server 和 shm server 控制绑定归属 `extensions/realtime`，只编译进
  `realtime`。
- TA-Lib 绑定归属 `extensions/talib`，但仍按 `HAYAKU_ENABLE_TA_LIB` 条件注册到 `core`。
- 三种历史数据导入器归属 `extensions/ingest`，只编译进 `ingest`。

不得为了让目录和 target 一一对应而复制绑定实现。

## 4. `hayaku_pywrap` 目标结构

### 4.1 最终目录

```text
hayaku_pywrap/
├── Bindings.h                       所有领域级注册入口的唯一声明表
├── main.cpp                         core 模块的唯一 composition root
├── xmake.lua
│
├── common/
│   ├── Bindings.cpp                  时间、参数、常量、算术、日志、STL 与 IO
│   ├── PybindSupport.{h,cpp}         跨领域真正复用的 pybind 工具
│   ├── PickleSupport.h
│   └── AnyConversion.h
│
├── data/
│   ├── Bindings.cpp                  DataEngine、值类型和 storage 协议
│   └── DataFrameConversion.{h,cpp}   仅保留确实被多处调用的转换实现
│
├── operators/
│   ├── Bindings.cpp                  Indicator、IndParam、Factor 与 FactorSet
│   └── BuiltinBindings.cpp           大批无状态内置算子集中注册
│
├── execution/
│   └── Bindings.cpp                  Engine、订单、Broker、定价与记录
│
├── metrics/
│   └── Bindings.cpp                  Performance 与只读结果分析
│
├── strategy/
│   └── Bindings.cpp                  Engine、decision、risk、selection、portfolio
│
├── application/
│   └── Bindings.cpp                  Session 与核心插件宿主
│
└── extensions/
    ├── talib/
    │   └── Bindings.cpp
    ├── ingest/
    │   ├── Module.cpp
    │   └── Bindings.cpp               CheckData 与全部 importer
    └── realtime/
        ├── Module.cpp
        └── Bindings.cpp               Spot、data server 与 shm server
```

这棵树以约 15 个 `.cpp` 为正常目标，验收上限为 20 个；不是每个 C++ 类都对应一个
binding 文件。每个 `Bindings.cpp` 内用匿名 namespace 或 `static` 叶级函数维持可读性，
领域外只看见 `Bindings.h` 中的一个注册入口。只有以下情况才允许继续拆文件，并须在
xmake 清单旁写明原因：

1. 同一份实现必须分别进入两个原生 target，且无法通过公共函数复用；
2. 单个领域在非 unity 构建中稳定触发编译器内存或对象文件限制；
3. 平台或功能开关要求独立编译，合并会把可选依赖带入默认 `core`。

“一个类型一个文件”、只含一个 `m.def`/`py::class_` 的薄文件、以及仅转发另一个
`export_*` 的文件都不属于例外。最终不得重新出现 `advanced`、`analysis`、`app`、
`data/indicator`、`data/factor` 等旧归属。

### 4.2 旧目录到新目录的唯一映射

| 当前路径 | 目标路径 | 说明 |
|---|---|---|
| `common/**` | `common/**` | 统一文件命名和注册入口 |
| `data/*.cpp` | `data/**` | 数据值类型和 `DataEngine` |
| `data/driver/**` | `data/storage/**` | 与 C++ `data/storage` 对齐 |
| `data/indicator/**` | `operators/**` | 指标属于算子域，不属于数据存储 |
| `data/factor/**` | `operators/**` | Factor 与 Indicator 同属算子域 |
| `analysis/**` | `metrics/**` | 与 C++ `metrics` 对齐 |
| `app/_HayakuSession.cpp` | `application/**` | Session 属于应用装配 |
| `advanced/_device.cpp` 等核心插件绑定 | `application/plugins/**` | 与 C++ 插件宿主归属一致 |
| `_ta_lib.cpp` | `extensions/talib/**` | 可选算法扩展 |
| 导入器、`_checkdata.cpp`、`ingest_main.cpp` | `extensions/ingest/**` | 一个完整的可选模块边界 |
| Spot/data server/shm server、`realtime_main.cpp` | `extensions/realtime/**` | 一个完整的可选模块边界 |

### 4.3 注册边界

1. `Bindings.h` 只声明一级业务域注册入口，每个领域只暴露一个
   `bindXxx(py::module_&)`。
2. `main.cpp` 只负责模块创建、全局异常/运行状态初始化和调用领域级注册入口。
3. 叶级注册函数只存在于所属 `Bindings.cpp` 内，不建叶级头文件，也不由 `main.cpp`
   跨目录逐一声明。
4. 可选 `ingest`、`realtime` 各自保留一个 `Module.cpp`，这是模块初始化边界，不是按类型拆分。
5. 可以修正 C++ 内部注册函数和文件名拼写，但 Python 可见名称不得随之改变。
6. pybind 类注册顺序、基类注册顺序和跨模块类型 identity 必须保持有效。
7. `core`、`ingest`、`realtime` 三个 target 使用显式且互不重叠的文件清单；unity group
   按业务域划分，不用递归 glob 隐式重复收集同一文件。

## 5. `hayaku` Python 包目标结构

### 5.1 组织原则

Python 目录使用用户认知，而不是照抄 C++ 名称。对应关系固定如下：

| C++ / pywrap 领域 | Python 公共入口 | 决策 |
|---|---|---|
| `common` | `hayaku.common` | 保留 |
| `data` | `hayaku.data` | 保留，但只允许数据查询和值类型 |
| `operators` | `hayaku.indicator` | 保留成熟的 Python 术语 |
| `execution` | `hayaku.execution` | 保留 |
| `metrics` | `hayaku.analysis` | 保留用户面对的“分析”术语 |
| `strategy` | `hayaku.strategy` | 保留 |
| `application` | `hayaku.session`、`hayaku.interactive` | 保留显式运行入口 |
| extension protocols | `hayaku.spi` | 保留 |
| ingest extension | `hayaku.ingest` | 保留，普通导入必须惰性 |
| realtime extension | `hayaku.advanced` | 本步保留既有公共路径，接口收敛步骤再评估命名 |

### 5.2 最终目录

```text
hayaku/
├── __init__.py                     冻结的 30 项顶层公共 API
├── _public_api.py                  声明式公共边界
├── core.py                         原生 core 加载与原始绑定命名空间
├── session.py                      显式 Session 入口
├── interactive.py                  显式宽接口和默认数据会话入口
├── hub.py                          策略仓库应用能力
│
├── common/
│   ├── __init__.py                 共享值类型公共门面
│   └── _extensions.py              Datetime/TimeDelta/Parameter 的可选 Python 增强
├── data/
│   ├── __init__.py                 仅 DataEngine、Stock、KData、Query 等公共门面
│   └── _extensions.py              Query/KData 等可选 Python 增强
├── indicator/
│   ├── __init__.py                 算子、Indicator 与 Factor 用户入口
│   ├── _extensions.py              Indicator Python 增强
│   └── formulas.py                 纯 Python 指标公式
├── execution/
│   ├── __init__.py
│   ├── brokers.py
│   ├── records.py
│   ├── broker_easytrader.py
│   └── broker_mail.py
├── analysis/
│   ├── __init__.py
│   └── results.py                  只读结果与报表转换
├── strategy/
│   ├── __init__.py
│   ├── components.py
│   └── examples.py                 小型策略构造示例；不在导入时运行
├── spi/
│   └── __init__.py                 显式扩展协议
│
├── ingest/
│   ├── __init__.py                 可选原生导入器的惰性入口
│   ├── sources/                    TDX、PyTDX、QMT、行情与板块抓取
│   ├── backends/                   SQLite、MySQL、HDF5、ClickHouse 适配
│   ├── jobs/                       K 线、权重、财务、板块、国债导入作业
│   └── schema/
│       ├── sqlite/
│       ├── mysql/
│       ├── clickhouse/
│       └── memory/
├── advanced/
│   └── __init__.py                 可选实时服务控制的惰性入口
├── draw/
│   ├── __init__.py
│   ├── backends/                   matplotlib、bokeh、echarts
│   └── studies/                    elder、kaufman、volume
│
├── gui/
│   ├── __init__.py
│   ├── HayakuTDX.py
│   ├── importdata.py
│   ├── dataserver.py
│   ├── shmserver.py
│   ├── spot_server.py
│   ├── start_qmt.py
│   ├── tasks/                      原 `gui/data` 中的导入任务与线程
│   ├── generated/                  Qt 生成文件
│   ├── flat/                       实时 FlatBuffers Python 生成代码
│   └── resources/                  UI、图标和图片
├── shell/
│   ├── __init__.py
│   ├── hayakucmd.py
│   ├── hayakushell.py
│   └── cmdserver.py
├── config/
│   ├── __init__.py
│   ├── generator.py                原 `data/hayaku_config_template.py`
│   └── block/                      内置板块配置
├── _support/                       非公共 Python 基础设施
│   ├── checks.py
│   ├── logging.py
│   ├── notebook.py
│   ├── singleton.py
│   ├── slicing.py
│   └── timeout.py
│
├── cpp/                            core 本地构建产物与 i18n 资源落点
├── plugin/                         运行期插件产物落点
├── test/                           Python 回归测试
└── examples/                       Python 示例和 notebook
```

`test` 和 `examples` 本步继续保留在现有位置，避免把测试发现、notebook 资源和包结构迁移
混成第三项工程；打包脚本必须显式排除不应进入 wheel 的测试缓存和本地产物。

### 5.3 必须消失的历史归属

| 当前内容 | 目标归属 |
|---|---|
| `hayaku/data` 下的数据导入实现 | `hayaku/ingest/sources|backends|jobs` |
| `hayaku/data/*_upgrade`、`*_sql` | `hayaku/ingest/schema` |
| `hayaku/fetcher` | `hayaku/ingest/sources` |
| `hayaku/util` | 私有 `hayaku/_support` |
| `hayaku/flat` | `hayaku/gui/flat` |
| `hayaku/gui/data` | `hayaku/gui/tasks` 或 `gui/generated` |
| `hayaku/data/hayaku_config_template.py` | `hayaku/config/generator.py` |
| `hayaku/extend.py` | 所属领域的私有 `_extensions.py` |

迁移完成后不保留上述旧路径的转发模块。仓库内 GUI、CLI、测试、示例和文档必须在同一批次
迁到新路径。

## 6. 公共接口边界

### 6.1 本步必须保持不变

1. `hayaku.__all__` 仍为当前冻结的 30 项，名称集合和对象 identity 不变。
2. `hayaku.common`、`data`、`indicator`、`execution`、`analysis`、`strategy`、`spi`、
   `ingest`、`advanced`、`draw` 的公共名称集合不因物理移动而扩大。
3. `hayaku.core` 继续加载与当前 Python 版本对应的 `core3xx` 原生模块。
4. `open_session`、`hayaku.session`、`hayaku.interactive` 的导入和生命周期语义不变。
5. `ingest` 和 `advanced` 缺少可选原生包时继续给出可操作的 `ImportError`，普通导入不失败。
6. `HayakuTDX`、`importdata`、`dataserver`、`shmserver` console script 名称不变。
7. pybind11 导出的 Python 类名、函数名、参数、默认值、枚举、pickle 和异常类型不变。
8. `core`、`ingest`、`realtime` 的原生模块名、安装位置和跨模块类型注册关系不变。

### 6.2 本步允许删除的内部路径

以下路径不属于冻结公共面，完成仓库内消费者迁移后直接删除，不增加 shim：

- `hayaku.fetcher.*`
- `hayaku.util.*`
- `hayaku.flat.*`
- `hayaku.extend`
- `hayaku.data` 下除公共门面外的导入脚本和数据库模块
- `hayaku.gui.data.*`
- `hayaku_pywrap` 中所有旧目录和旧内部注册函数名

删除内部路径前仍须扫描第三方文档和发布说明；如果发现它已经被正式记录为公共接口，应先补入
冻结清单并调整目标，而不是临时保留转发层。

## 7. 本步变更边界

### 7.1 现在必须做

1. 按第 4 节重组 `hayaku_pywrap`，建立八域所有权和三个显式原生 target 清单。
2. 按第 5 节重组 `hayaku`，消除 `data`、`advanced`、`util` 等当前职责混杂。
3. 同步修改 C++ include、内部注册函数、Python import、xmake、setup、console entry point。
4. 同步迁移 SQL、ini、Qt、FlatBuffers、图片等包资源，并验证 wheel 中的实际路径。
5. 为公共模块建立显式 `__all__` 或导出快照，不再依赖无边界的星号导入决定公共面。
6. 保持 `import hayaku` 无数据会话、无网络、无数据库、无 GUI/绘图库副作用。
7. 删除旧路径，不建立长期转发目录或双份实现。
8. 路径稳定后，为本步新增或修改的 C++ 绑定头统一使用单一 `#pragma once`。

### 7.2 允许的最小代码变化

- 文件和目录移动、文件名修正。
- 内部 C++ 注册函数改名与域级注册入口提取。
- Python 内部 import、资源定位和 console entry point 更新。
- 将 `extend.py` 按所有权拆为私有增强模块，但不得改变增强加载时机。
- 为防止导入副作用而把重依赖改为函数内导入或惰性导入。
- 将同一职责的薄 Python 转发文件或薄绑定注册文件合并。
- 删除无导出、无引用、无构建归属的空绑定文件。
- 为验证边界补充结构扫描、导出快照、wheel 内容和 import 副作用测试。

### 7.3 本步明确不做

| 延期内容 | 原因 | 后续处理 |
|---|---|---|
| 修改 `hayaku_cpp/src` 的公共接口或对象职责 | 本步消费第 5 步稳定的 C++ 面，不重新设计核心 | 第 8 步接口收敛 |
| 合并 `DataRuntime`、`ExecutionRuntime`、`StrategyRuntime` 与 Engine | 会改变生命周期和调用协议 | 第 8 步接口收敛 |
| 拆分原生 `core` 为多个 pybind11 子模块 | 会影响类型 identity、pickle、安装和导入顺序 | 独立设计后再做 |
| 改变 888 项 pybind 导出或 30 项顶层公共 API | 属于接口变化，不是物理整理 | 第 8 步逐项评估 |
| 修改指标、策略、成交、手续费、滑点、复权和数据导入算法 | 属于业务行为 | 本轮禁止 |
| 修改 SQL schema、序列化、pickle 或配置格式 | 可能破坏已有数据 | 后续带迁移方案处理 |
| 引入新第三方依赖或更换 GUI/绘图库 | 与目录整理无关 | 独立步骤 |
| 手工生成或批量改写 `.pyi` | stub 按项目发布流程自动生成 | 验收时只校验一致性 |
| 再增加旧品牌兼容包、namespace alias 或双份动态库 | 品牌切换已作为前置原子批次完成；双轨兼容会重新扩大边界 | 本轮禁止 |
| 为旧内部路径增加兼容包、转发模块或软链接 | 会永久保留两套结构 | 本步禁止 |

## 8. 结构验收条件

### 8.1 `hayaku_pywrap`

- [ ] 一级业务目录只有 `common`、`data`、`operators`、`execution`、`metrics`、
      `strategy`、`application`、`extensions`。
- [ ] 不存在 `advanced`、`analysis`、`app`、`data/indicator`、`data/factor`。
- [ ] `main.cpp` 不再直接声明叶级绑定注册函数。
- [ ] 绑定 `.cpp` 从当前 82 个收敛到不超过 20 个；任何超出一域一个主文件的拆分都有明确构建理由。
- [ ] `core`、`ingest`、`realtime` 文件清单显式、互斥且无重复编译源。
- [ ] 每个绑定文件只有一个业务所有者和一个明确 target 归属。
- [ ] 内部拼写错误文件名和注册函数名已经修正，Python 导出名称保持不变。
- [ ] 新增或修改的绑定头只使用单一 `#pragma once`。

### 8.2 `hayaku`

- [ ] `hayaku/data` 只包含数据查询和值类型公共门面及其私有增强，不含导入作业、连接器或 SQL。
- [ ] 数据源、后端、导入作业和 schema 全部归入 `hayaku/ingest`。
- [ ] 不存在顶层 `fetcher`、`util`、`flat`、`extend.py` 和 `gui/data`。
- [ ] `__init__.py` 仍只导出冻结的 30 项公共 API。
- [ ] 公共领域包都有显式导出边界，私有支持代码不会泄漏到包顶层。
- [ ] `cpp`、`plugin` 只作为构建/运行产物落点，不混入业务 Python 源码。
- [ ] console scripts、SQL、ini、Qt、FlatBuffers、图片和 i18n 资源在安装包中可定位。

### 8.3 行为

- [ ] Step 5 API inventory 中的 pybind 导出集合完全一致。
- [ ] 公共 Python 模块的 `__all__`、函数签名和对象 identity 与迁移前一致。
- [ ] `import hayaku` 不加载 pandas、matplotlib、SQLAlchemy、PyQt、ingest 或 realtime 原生模块。
- [ ] 缺少 ingest/realtime 可选包时，基础包可正常导入并给出既有错误语义。
- [ ] Python 3.10 核心测试、边界测试、完整 Python 测试和三个原生模块构建通过。
- [ ] 默认 wheel、ingest wheel 和 realtime wheel 的内容检查通过。
- [ ] `git diff --check` 通过。

## 9. 相邻步骤与品牌基线

品牌切换原计划放在更后阶段，但已按用户决定提前为第 6 步的前置原子批次完成。当前唯一
有效名称为 `hayaku`：顶层源码目录、Python import、C++ namespace、宏、动态库、xmake
target、CLI、配置目录、插件 ABI、测试和文档必须使用同一名称；旧名称只允许出现在版权
归属、上游项目链接和真实第三方依赖名中。

后续顺序保持为：

### 第 7 步：C++ 质量基线与无效文件清理

- 固定 Google 格式与 clang-tidy 规则，完成一次性机械格式化和静态检查基线。
- 建立 ASan/泄漏检查矩阵，审计并清理无效文件与过时文档入口。
- 目标和验收条件见 [第 7 步](step-7-progress.md)。

### 第 8 步：接口收敛

- 评估 Runtime 与三个 Engine 的最终关系。
- 删除重复 Base/Manager/Port 和无独立价值的公共接口。
- 整理 `Stock/KData/KQuery` 等公共类型和 Python 原始绑定面。
- 任何 Python 公共 API 变化都必须带迁移说明，而不是作为目录移动的副作用。

## 10. 执行计划状态

本文件当前冻结目标结构、聚合式 binding 粒度和变更边界。逐文件映射、批次顺序、回退点、
构建矩阵和预计时间，在目标树评审通过后补入；在此之前不开始 Step 6 批量移动。
