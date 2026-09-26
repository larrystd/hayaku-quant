# 第 4 步：数据层、应用层与可选能力收口

> 状态：执行中；`data → app` 反向依赖已切断，`ingest` 与 `realtime` 可选构建/安装边界已验证；完整 Step 4 验收仍未通过
>
> 前置阶段：[第 3 步：最终架构冻结与旧体系退役](step-3-progress.md)
>
> 决策日期：2026-09-25
>
> 本文件先定义目标、边界、处理顺序和验收标准，实施与验证结果持续记录在第 12 节。

## 1. 阶段背景

第 3 步已经把研究、回测和模拟执行收口到 `DataEngine`、`ExecutionEngine`、
`StrategyEngine` 与 `HayakuSession`。但 `hayaku_cpp/src/data` 与 `hayaku_cpp/src/app` 的
职责仍未完全分开。

`hayaku_cpp/src/data` 当前同时包含：

- 证券、K 线、查询、权息、财务等数据领域类型；
- `DataEngine`、Driver、缓存、预加载和数据运行期；
- 指标和因子计算；
- HDF5、SQLite、MySQL、TDX、CSV 等存储或读取实现；
- IPC、共享内存客户端和实时更新路径；
- 对应用初始化、调度器、遥测、设备授权、VIP 扩展和商业因子插件的反向依赖。

`hayaku_cpp/src/app` 当前同时承担：

- 核心进程与 Session 生命周期；
- 动态插件装载和卸载；
- HDF5、MySQL、ClickHouse 数据导入门面；
- 数据检查；
- 实时行情接收、共享内存服务和远程数据服务；
- 设备授权、VIP 指标、扩展功能和交易报告；
- 调度器、系统信息和遥测上报。

一部分可选能力被统一放在 `app/plugin` 下，另一部分实时和插件逻辑已经进入 `data/internal`、
`data/driver/ipc` 和数据领域对象。这不表示它们具有相同的产品职责，也不表示每个接口都应该继续
留在核心库中。

当前最重要的结构问题不是文件数量，而是依赖方向：`app` 需要组合 `data`，但 `data` 又直接包含
`app/GlobalInitializer.h`、`app/runtime/**` 和 `app/plugin/**`，形成双向耦合。第 4 步必须先切断
这种反向依赖。

当前 `hayaku_cpp/src/xmake.lua` 只有一个 `target("hayaku")`。`src` 下新增任何子目录，默认仍会
进入同一个共享库。因此，本步骤明确禁止用新建 `src/ingest`、`src/realtime` 等目录来假装完成
模块化。**源码目录调整必须晚于产品边界和构建边界确定。**

## 2. 阶段目标

本步骤的目标是同时收紧数据层、应用层和插件面，使默认安装只承担研究、回测和模拟执行，同时
保留少量、边界清晰的官方可选能力。

最终产品只保留三个用户可理解的能力面：

| 能力面 | 定位 | 是否默认启用 |
| --- | --- | --- |
| `core` | 研究、指标/因子计算、回测、分析和模拟执行 | 是 |
| `ingest` | 下载、导入、校验、转换和修复历史数据 | 否 |
| `realtime` | 接收实时行情、更新运行期行情并按需发布行情服务 | 否 |

这里的 `core`、`ingest` 和 `realtime` 是产品职责名称，不预设为 `src` 下的物理目录。
不再使用含义宽泛的 `tools` 或 `advanced` 作为新能力的归属名称。

MySQL 与 ClickHouse 继续属于**官方支持的存储后端**，但它们是存储适配器，不增加新的产品能力面：

```text
core / ingest / realtime
            │
            ▼
统一的存储契约
    ├── HDF5 / SQLite（默认实现）
    ├── MySQL（官方可选适配器）
    └── ClickHouse（官方可选适配器）
```

独立构建、独立安装或独立仓库均不代表停止支持。官方支持至少要求版本兼容表、持续集成、基础读写
测试和升级说明。具体采用同仓库独立 target 还是独立扩展仓库，在接口收口后另行决定，本步骤不先
搬迁实现。

## 3. 核心边界

### 3.1 `core` 必须包含

- `HayakuSession`、`SessionOptions` 和必要的进程级生命周期管理；
- `DataEngine` 及研究和回测所需的只读市场数据能力；
- `ExecutionEngine`、账户、订单、费用、账本和模拟 Broker；
- `StrategyEngine`、策略组件、组合、选股和回测运行期；
- 基础指标、因子计算和只读分析；
- 默认存储实现及统一存储扩展契约；
- 日志、线程、序列化等无业务含义的基础设施。

回测已经属于 `StrategyEngine` 的核心职责，不再额外保留一套 BackTest 动态插件入口。

### 3.2 `core` 明确不包含

- 历史数据下载、批量导入、检查和修复流程；
- 实时行情接收、常驻行情服务器及共享内存发布；
- 设备授权、许可证校验和商业功能发现；
- VIP 指标、VIP 扩展、商业报告和私有数据源实现；
- MySQL、ClickHouse 客户端依赖的强制加载；
- 自动遥测、启动时网络访问、后台反馈线程；
- 与研究、回测和模拟执行无关的常驻调度任务。

`import hayaku`、构造 Session 和只运行回测时，不得连接网络、启动行情服务、写入设备 UID、加载
商业插件或要求安装 MySQL/ClickHouse 客户端。

### 3.3 两个可选能力的边界

`ingest` 只负责把外部历史数据准备成 Hayaku 可读取的存储数据，包括：

- 数据下载和数据源接入；
- K 线、分时、逐笔、财务、权息和板块数据导入；
- 完整性校验、索引更新、修复和格式转换；
- 向 HDF5、SQLite、MySQL、ClickHouse 等后端写入。

`realtime` 只负责运行时行情流，包括：

- 接收实时行情；
- 更新进程内实时行情和 K 线；
- 按明确配置发布共享内存或进程间行情；
- 显式启动、停止和健康检查。

`ingest` 不启动常驻行情服务，`realtime` 不承担历史数据批量修复。两者均不得成为核心库初始化的
隐式前置条件。

## 4. 本步骤实施范围

### 4.1 纳入范围

本步骤以 `hayaku_cpp/src/data` 和 `hayaku_cpp/src/app` 为两个主要审计对象，并同步处理它们的
直接公共面：

- `hayaku_cpp/src/data/**`；
- `hayaku_cpp/src/app/**`；
- `DataEngine`、`DataRuntime`、Driver Factory、存储实现和 IPC 路径；
- 指标、因子中对应用插件、授权和遥测的依赖；
- `hayaku_pywrap/advanced/**` 中对应绑定；
- `hayaku_pywrap/data/**` 与 `hayaku/advanced/**`、`hayaku/data/**`、GUI 中的直接调用；
- xmake 构建选项、安装规则和插件加载规则；
- 对应 C++、Python、数据读取、生命周期和导入边界测试；
- 对应中英文用户文档。

跨目录修改只能用于完成上述公共面收口，不能借机重排 `execution`、`strategy` 或其他无关目录。

### 4.2 明确不纳入范围

- 不重新设计三个 Engine 的业务模型；
- 不重新迁移 `hayaku_cpp/src` 的顶层目录；
- 不重写已有指标、因子、复权和 K 线算法；
- 不因 `data` 文件数量多就合并或删除具有独立公式和测试的指标实现；
- 不为了分类整齐而创建 `src/ingest`、`src/realtime`；
- 不在本步骤内决定独立扩展仓库的最终地址和发布流程；
- 不重写外部商业插件的内部实现；
- 不把 Python 数据下载脚本整体改写为 C++；
- 不新增第四个 Engine 或新的全局 Manager；
- 不以保留所有历史入口为目标；确认无用的旧接口允许删除。

### 4.3 `data` 的目标职责

`data` 是核心研究与回测的数据领域，不等同于“数据导入工具”。最终只负责以下四类能力：

1. 数据值类型和查询模型；
2. `DataEngine` 提供的稳定只读查询；
3. 数据 Driver 契约、默认本地读取实现、缓存和预加载；
4. 指标和因子的纯计算能力。

当前各部分的目标如下：

| 当前目录或类型 | 是否属于核心 `data` | 处理目标 |
| --- | --- | --- |
| `Stock`、`KData`、`KQuery`、`KRecord`、`Block` 等领域类型 | 是 | 保留稳定语义，禁止依赖应用插件和进程初始化 |
| `DataEngine` | 是 | 作为普通用户读取数据的唯一稳定门面，保持只读 |
| `indicator/**` | 是 | 保留研究所需的指标计算；移除对 `app/runtime/sysinfo` 的依赖 |
| `indicator_talib/**` | 可选计算适配器 | 保持按构建选项启用，不增加新的产品能力面 |
| `factor/**` | 是 | 保留因子计算；商业持久化、授权检查和 ClickHouse 调用移出核心计算路径 |
| `driver/*.h` | 是 | 保留最小 BaseInfo、BlockInfo、KData 读取契约 |
| SQLite、HDF5 Driver | 是 | 作为默认本地存储实现，不能依赖插件系统 |
| MySQL Driver | 官方可选适配器 | 继续支持，但核心未启用 MySQL 时不得包含其客户端依赖 |
| ClickHouse Driver/插件 | 官方可选适配器 | 接入统一存储契约，不通过过宽的商业插件接口侵入 DataRuntime |
| TDX、CSV、千龙实现 | 待逐项确认 | 有直接研究读取场景则作为适配器保留；只用于导入时归入 `ingest` |
| `KDataPrivatedBufferImp`、`KDataSharedBufferImp` | 是 | 保留进程内 KData 缓冲和共享所有权语义 |
| `KDataShmBufferImp`、`driver/ipc/**` | `realtime` 边界 | 不参与默认数据初始化；由实时组件显式装配 |
| `DataRuntime` | 是，但必须瘦身 | 只管理 Driver、核心数据状态、缓存和预加载，不管理通用插件和服务 |
| `StrategyContext` | 待澄清命名 | 若实际只控制数据加载范围，应收口为数据加载配置；不得在 data 中承载策略运行状态 |

文件多不是删除依据。`indicator/**` 当前包含大量独立公式实现，只要它们属于研究公共面、拥有测试
且不引入反向依赖，就仍是核心数据能力。需要收缩的是运行期耦合和可选后端，而不是机械减少源码
数量。

### 4.4 `DataRuntime` 的收口目标

`DataRuntime` 当前同时管理数据、线程、通用插件、共享内存协商和商业扩展，职责过宽。最终边界为：

```text
HayakuSession / app
        │ 创建、配置、关闭
        ▼
DataEngine
        │ 只读门面
        ▼
DataRuntime
        ├── Driver 选择与连接池
        ├── 证券、市场、板块、权息、财务数据状态
        ├── 缓存和预加载
        └── 数据加载事件（核心内最小契约）
```

`DataRuntime` 最终不得继续负责：

- 持有通用 `PluginManager` 或按插件名发现商业功能；
- 包含 `app/plugin/interface/**`；
- 决定 ShmServer/DataServer 的服务角色；
- 启动应用调度任务、遥测或设备授权；
- 自动加载 VIP 指标、HayakuExtra、TMReport 或商业因子存储；
- 通过全局访问器在 Session 关闭后重新创建自己。

存储适配器通过明确的 Driver 注册或构造注入进入数据层。`app` 负责组合和所有权，`data` 不负责
寻找、授权或启动外部产品能力。

### 4.5 `data` 与 `app` 的依赖方向

最终只允许以下方向：

```text
app ───────────────► data
ingest ────────────► data 的写入/存储契约
realtime ──────────► data 的实时更新契约
storage adapters ─► data 的 Driver 契约

data ──X──► app
```

因此需要逐项消除当前反向依赖：

- `IndicatorImp.cpp` 不再包含 `app/runtime/sysinfo.h`；
- `DataDriverFactory.cpp`、`Stock.cpp`、`DataRuntime.cpp` 不再依赖 `GlobalInitializer`；
- `DataRuntime` 不再包含 app 调度器、`plugins.h`、device、hayakuextra、extind 和 sysinfo；
- `Factor`、`FactorSet` 不再直接调用 app 中的授权和商业 factor façade；
- `KQuery`、`KData`、`Stock` 不再直接调用 `app/plugin/hayakuextra`；
- 共享内存客户端通过最小实时端口接入，不让核心 data 包含 ShmServer 插件接口。

若移除反向依赖需要新增接口，该接口必须由被依赖的一侧定义：数据读取/更新接口定义在 data，
实现和装配留给 app、realtime 或存储适配器。禁止再建立新的跨层全局单例。

## 5. 现有 `app/plugin` 的目标归属

下表是进入实施前的目标分类。它用于确定职责和去留，不表示立即按表移动文件。

| 当前能力/文件 | 目标归属 | 处理目标 |
| --- | --- | --- |
| `backtest.*`、`BackTestPluginInterface.h` | 删除候选 | 回测已归 `StrategyEngine`；确认无外部 ABI 使用后删除重复入口 |
| 三个 `KDataTo*Importer.*` 及接口 | `ingest` | 统一导入契约，后端差异由存储适配器承担，避免三套重复门面 |
| `checkdata.*`、`CheckDataPluginInterface.h` | `ingest` | 保留数据校验能力，但不从核心顶层 API 自动暴露 |
| `shmserver.*`、`ShmServerPluginInterface.h` | `realtime` | 纳入显式生命周期，消除插件卸载后的裸指针风险 |
| `dataserver.*`、`DataServerPluginInterface.h` | 待确认 | 只有存在独立远程服务场景、调用方和测试时才保留；否则删除 |
| `factor.*`、`DataDriverPluginInterface.h` | 存储适配器 | 拆分过宽接口；因子读写不得与 Driver 注册、计算和商业发现混在一个接口 |
| `device.*`、`DevicePluginInterface.h` | 商业扩展 | 不进入核心初始化，不作为第四个产品能力面 |
| `extind.*`、`ExtendIndicatorsPluginInterface.h` | 商业扩展 | 通过明确扩展接口按需加载，不污染基础指标 API |
| `hayakuextra.*`、`HayakuExtraPluginInterface.h` | 商业扩展 | 从核心公共面和默认初始化移除 |
| `TMReportPluginInterface.h` | 商业扩展 | 分析层通过可选报告扩展使用，核心分析不依赖其存在 |
| `plugins.h` | 删除 | 删除全量聚合头；调用方只包含实际使用的最小接口 |

商业/VIP 能力不继续拆成多个用户可见模块。它们统一视为外部扩展，只保留必要的最小扩展契约；
没有核心调用方的契约不为“以后可能使用”而保留。

## 6. `app` 运行期的处理边界

`app` 最终只负责配置解析、对象组合和生命周期，不承载具体数据模型、数据导入、行情协议或商业
业务。`app` 可以依赖 `data` 的稳定契约，`data` 不得反向调用 `app` 完成初始化或扩展发现。

### 6.1 Session 与进程生命周期

- 明确区分进程级资源和 Session 级资源；
- 移除头文件静态对象触发的隐式全局初始化；
- 初始化和关闭必须成对、幂等，并支持同进程内重新创建 Session；
- `HayakuSession::close()` 必须关闭它启动的线程、服务和运行期对象；
- 多 Session 使用不同配置时不得静默复用第一个 Session 的配置；
- 进程运行期对象只管理日志、线程池、Driver 注册和指标等核心基础能力。

运行期对象的最终类名和文件名在实现时确定，不因本方案预先创建 `ProcessRuntime.*`。

### 6.2 插件所有权

- 插件实例和动态库句柄必须由同一个所有者管理；
- 任何 façade、服务或导入器不得在插件卸载后继续保存裸接口指针；
- 停止服务必须发生在卸载对应动态库之前；
- 可选插件加载失败只能使对应能力不可用，不得破坏核心 Session；
- 插件 ABI 至少包含插件标识、接口版本、兼容性校验和明确的销毁路径；
- 不再通过 `plugins.h` 一次性引入全部商业和服务接口。

### 6.3 网络、调度与遥测

- 核心初始化期间禁止自动发送反馈或遥测；
- 遥测若继续保留，必须显式选择加入，并具有可等待、可取消的生命周期；
- 禁止 detached 线程在 Session 或全局运行期销毁后继续访问资源；
- 调度器必须可重复创建和释放，不使用“只能初始化一次但可以删除”的状态组合；
- SpotAgent、DataServer 和 ShmServer 只能由 `realtime` 的显式入口启动。

## 7. 物理目录和仓库决策规则

本步骤先确定依赖和构建边界，再决定物理位置。只有满足以下条件，能力才可以从当前位置迁移：

1. 已经拥有独立、命名明确的构建 target；
2. 核心库不再反向依赖该 target；
3. 公共头、运行库和 Python 包的安装边界已经确定；
4. 单独安装和未安装两种测试均可执行；
5. 生命周期和 ABI 契约已有自动化测试；
6. 文档能够说明用户何时需要安装该能力。

在满足上述条件前，允许文件暂时保留原位，但必须通过构建开关和调用边界消除隐式依赖。

可能的发布单元名称如下，仅作为发布语义，不作为本阶段目录创建清单：

```text
hayaku                    # core
hayaku-ingest             # 可选的数据准备能力
hayaku-realtime           # 可选的实时行情能力
hayaku-storage-mysql      # 官方 MySQL 适配器
hayaku-storage-clickhouse # 官方 ClickHouse 适配器
```

MySQL 和 ClickHouse 可以与主仓库分开发布，但必须继续参与官方兼容性和回归测试。

## 8. 删除与保留判定

每个现有插件接口都必须回答以下问题：

1. 是否存在仓库内真实调用方？
2. 是否存在文档化的用户场景？
3. 是否存在公共 API 或已知外部 ABI 使用方？
4. 是否有独立测试覆盖成功、失败和关闭路径？
5. 是否必须使用 C++ 动态插件，而不能由 Python 或普通 Driver 完成？
6. 是否能在不初始化该能力时保持核心零副作用？

判定规则：

- 有明确场景、调用方和测试：保留并归入对应能力；
- 能力有价值但接口重复或过宽：先设计最小契约，再迁移调用方；
- 只剩历史入口、无调用方且被新 Engine 覆盖：列入删除批次；
- 无法确认外部 ABI 使用：先登记兼容风险和迁移说明，不静默删除；
- 仅因“未来可能需要”而存在：不作为保留理由。

## 9. 执行计划

### 9.1 执行总目标

执行过程始终以以下五个结果为准，不以移动文件数量或减少目录数量为准：

1. `data` 只负责数据领域、读取、缓存、指标和因子计算，不依赖 `app`；
2. `app` 只负责配置、对象装配和生命周期；
3. 默认核心不加载导入、实时服务、商业插件、遥测或 MySQL/ClickHouse 客户端；
4. 用户可见的可选能力只有 `ingest` 和 `realtime`，存储后端统一作为适配器；
5. 研究、回测、模拟执行和现有数据查询结果保持不变。

整体执行顺序如下：

```text
0. 冻结基线
      ▼
1. 纯化 data 领域代码
      ▼
2. 收窄 DataRuntime 与 Driver
      ▼
3. 收口 app 生命周期
      ▼
4. 清理 app/plugin
      ▼
5. 统一 ingest 能力
      ▼
6. 隔离 realtime 能力
      ▼
7. 建立可选构建和发布边界
      ▼
8. 全量验收并冻结架构
```

每个阶段必须独立通过对应门槛后再进入下一阶段。不允许先大规模移动文件，再集中修复编译和行为。

### 9.2 阶段 0：冻结现状和行为基线

**目标**：在删除或解耦前，明确每项代码的用途和不能改变的行为。

**处理范围**：

- `src/data/**` 的领域类型、Driver、DataRuntime、IPC、指标和因子；
- `src/app/**` 的 Session、初始化器、运行期、服务和插件 façade；
- pybind11、Python、GUI、文档和外部插件 ABI 入口。

**交付结果**：

- `data → app` 反向依赖清单；
- 插件接口、插件 ID、动态库、调用方和测试覆盖清单；
- 核心、`ingest`、`realtime`、存储适配器、商业扩展和删除候选六类归属表；
- HDF5/SQLite 查询、指标、因子、复权、回测和 Session 生命周期行为基线；
- 计划删除 API 的兼容风险和拒绝列表。

**通过门槛**：每个待处理入口都有明确归属；未知外部 ABI 使用已登记；基线测试可以稳定重复。

### 9.3 阶段 1：纯化 `data` 领域代码

**目标**：先让证券、K 线、查询、指标和因子成为不依赖应用层的纯数据与计算代码。

**处理范围**：

- `Stock`、`KData`、`KQuery` 等直接调用 `hayakuextra` 的路径；
- `IndicatorImp` 对 sysinfo/遥测的依赖；
- `Factor`、`FactorSet` 对 device 和商业 factor façade 的依赖；
- `DataDriverFactory`、`Stock` 对 `GlobalInitializer` 的依赖。

**交付结果**：

- 数据领域类型不负责触发全局初始化或发现插件；
- 指标和因子核心计算不检查设备授权、不访问商业存储；
- 商业增强能力通过外部扩展接口调用核心数据类型，而不是让核心数据类型反向调用插件；
- Driver Factory 不依赖静态应用初始化器才能工作。

**通过门槛**：上述文件不再包含 `app/**`；指标、因子、复权和数据对象单元测试结果不变。

### 9.4 阶段 2：收窄 `DataRuntime` 与 Driver

**目标**：使 DataRuntime 成为纯粹的数据读取运行期，而不是整个系统的插件和服务中心。

**处理范围**：

- `data/internal/DataRuntime.*`；
- `data/driver/DataDriverFactory.*` 和 Driver 注册流程；
- `data/driver/ipc/**`、`KDataShmBufferImp.*`；
- HDF5、SQLite、MySQL、ClickHouse 等存储实现的接入方式。

**交付结果**：

- DataRuntime 只管理 Driver、数据状态、缓存、连接池和预加载线程；
- 通用 PluginManager、商业插件预加载、服务角色、调度和遥测移出 DataRuntime；
- HDF5/SQLite 作为默认本地实现，通过统一 Driver 契约工作；
- MySQL/ClickHouse 通过同一契约注册，缺失时不影响核心；
- IPC/Shm 只通过最小数据读取或实时更新端口接入。

**通过门槛**：`src/data/**` 对 `src/app/**` 的直接包含为 0；关闭 Session 后 DataRuntime 不会被
全局访问器意外重建；默认配置不探测可选插件或实时服务。

### 9.5 阶段 3：收口 `app` 生命周期

**目标**：建立唯一、显式、可重复的初始化和关闭路径。

**处理范围**：

- `GlobalInitializer.*`；
- `HayakuSession.*`、`SessionOptions.*`；
- 全局线程池、DataRuntime 和指标注册的所有权；
- `GlobalSpotAgent`、scheduler、sysinfo 和反馈线程。

**交付结果**：

- 移除头文件静态对象触发的自动初始化；
- 明确进程级资源和 Session 级资源各自由谁创建、关闭；
- Session 启动的线程和对象全部在 `close()` 中按逆序停止；
- 不再存在无法重新创建的 once-flag/已删除对象组合；
- 遥测默认关闭，保留时必须显式启用、可取消、可等待；
- 不同 Session 配置不会被静默忽略。

**通过门槛**：连续 open/close、多次创建、异常初始化、并发 stop 和不同配置用例全部通过；进程退出
时没有遗留线程或卸载后回调。

### 9.6 阶段 4：清理 `app/plugin`

**目标**：删除重复和无主入口，只保留有明确消费者、生命周期和测试的最小扩展契约。

**处理范围**：`app/plugin/**`、对应 pybind11、Python `advanced` 入口和调用方。

**交付结果**：

- 删除 `plugins.h` 聚合头，调用方只包含实际使用的接口；
- 核对并删除被 `StrategyEngine` 替代的 BackTest 插件入口；
- 删除无调用方、无文档场景且无测试的 façade；
- device、extind、hayakuextra、TMReport 等商业能力不再由核心初始化；
- `DataDriverPluginInterface` 拆成必要的最小存储/因子扩展契约；
- DataServer 必须在证明独立场景和维护价值后才能保留。

**通过门槛**：每个剩余插件接口都有唯一职责、真实调用方、版本契约、成功/失败/关闭测试；插件卸载
后没有悬空接口指针。

### 9.7 阶段 5：统一 `ingest` 能力

**目标**：保留数据准备功能，但不再向用户暴露三套按后端复制的工作流。

**处理范围**：三个 `KDataTo*Importer`、`checkdata`、`hayaku/data/**`、相关 GUI 导入任务和绑定。

**交付结果**：

- 建立一个后端无关的导入契约；
- HDF5、MySQL、ClickHouse 只作为目标存储适配器选择；
- 下载、导入、检查、修复和升级脚本统一归入 `ingest` 产品能力；
- 普通 `import hayaku` 不导入这些实现，也不要求其依赖存在；
- GUI 使用统一导入契约，不直接绑定某个商业插件类。

**通过门槛**：同一导入流程可以选择不同后端；未安装 `ingest` 时核心正常运行；安装后各后端冒烟
导入和数据校验通过。

### 9.8 阶段 6：隔离 `realtime` 能力

**目标**：把实时行情接收和发布形成一个可显式启动、停止的能力面。

**处理范围**：SpotAgent、GlobalSpotAgent、ShmServer、IPC/Shm 数据路径、DataServer 候选和实时更新
钩子。

**交付结果**：

- 一个明确的 realtime start/stop/health 契约；
- SpotAgent 输入长度、FlatBuffers 校验和可空字段处理完整；
- 服务对象持有插件生命周期，不保存可能悬空的裸接口指针；
- 停止顺序为停止接收、停止工作线程、注销回调、释放对象、最后卸载动态库；
- 未启用 realtime 时不连接 IPC、不协商 Shm、不启动后台线程；
- DataServer 若与 ShmServer 场景重叠且缺少独立消费者，则删除。

**通过门槛**：启动、重复启动、停止、重复停止、畸形报文、插件缺失和 Session 提前关闭测试通过；
核心回测路径没有 realtime 依赖。

### 9.9 阶段 7：建立构建和发布边界

**目标**：把已经验证的逻辑边界变成真正的可选安装边界，而不是只调整源码目录。

**处理范围**：xmake targets、依赖声明、安装规则、Python 包入口、插件搜索路径和发布文档。

**交付结果**：

- 核心 target 不链接 MySQL、ClickHouse、实时服务或商业插件依赖；
- `ingest`、`realtime` 和存储适配器可以独立启用、安装和测试；
- MySQL、ClickHouse 具有明确版本兼容表和官方测试矩阵；
- 只有在独立 target 和安装边界成立后，才决定是否移动文件或拆到独立仓库；
- 不创建只改变名称、没有独立构建意义的 `src/ingest` 或 `src/realtime`。

**通过门槛**：至少验证“仅核心”“核心 + ingest”“核心 + realtime”“核心 + MySQL”“核心 +
ClickHouse”五种构建/安装组合。

### 9.10 阶段 8：全量验收与架构冻结

**目标**：证明职责收缩没有改变研究、回测和模拟执行结果，并阻止旧依赖重新进入。

**交付结果**：

- 依赖扫描测试固定 `data ─X─► app` 规则；
- 公共 API inventory、拒绝列表、插件 ABI 表和可选安装说明更新；
- C++、Python、导入、生命周期、服务和后端测试全部通过；
- 数据查询、指标、因子、复权、回测和模拟执行金标保持一致；
- 代表性回测性能回退不超过 5%，无未解释内存增长；
- Step 4 文档从“方案草案”更新为包含实际结果的完成报告。

**通过门槛**：第 10 节全部验收项同时成立；任何仅靠兼容壳维持的旧双向依赖均已删除。

## 10. 验收标准

### 10.1 结构与依赖

- 默认核心只包含研究、回测、分析和模拟执行所需能力；
- 用户可见的可选能力只有 `ingest` 和 `realtime`，存储后端作为适配器存在；
- 未创建仅用于分类的 `src/ingest`、`src/realtime`；
- `hayaku_cpp/src/data/**` 不再包含 `app/**` 头文件；
- `DataRuntime` 不再持有通用 PluginManager，不再启动服务、调度器、遥测或商业插件；
- `DataEngine` 仍是稳定只读门面，Driver 和缓存实现不泄漏到普通公共 API；
- 指标和因子核心计算不依赖设备授权、商业 façade 或 ClickHouse；
- IPC、Shm 和实时更新只通过最小 data 端口接入，不参与默认数据初始化；
- `plugins.h` 不再存在，剩余插件接口均有明确所有者和调用方；
- 核心可以在没有 MySQL、ClickHouse、VIP 插件和实时服务的环境中构建和运行；
- MySQL、ClickHouse 继续有官方读写、升级和兼容性测试。

### 10.2 行为与生命周期

- `import hayaku` 无网络请求、遥测、设备 UID 写入和后台服务启动；
- HDF5/SQLite 下证券、K 线、板块、日历、财务和权息查询金标保持一致；
- 指标、因子、复权和回测结果保持一致；
- Session 连续 open/close、多次创建及异常关闭通过测试；
- SpotAgent、ShmServer、DataServer 和调度器均能显式停止并释放线程；
- 插件卸载后不存在 façade 或服务保存的悬空接口指针；
- 可选插件缺失或版本不兼容时，错误只限于对应能力且信息明确；
- 多 Session 不会静默忽略不同配置。

### 10.3 API 与测试

- 每个保留的 C++/Python 公共入口都有用途说明和自动化测试；
- 回测只通过 `StrategyEngine` 的稳定入口，不存在第二套 BackTest 插件 API；
- 数据导入不再为 HDF5、MySQL、ClickHouse 暴露三套重复的上层工作流；
- `ingest`、`realtime`、MySQL 和 ClickHouse 未安装时，核心导入测试通过；
- 安装各可选能力后，对应契约测试和端到端冒烟测试通过；
- small-test、unit-test、Python 3.10 测试、import-test 和 `git diff --check` 全部通过；
- 代表性回测性能不得因插件边界调整回退超过 5%。

## 11. 阶段完成定义

以下条件必须同时成立，才能把第 4 步标记为完成：

```text
核心职责已经收窄
        +
data 与 app 已形成 app → data 单向依赖
        +
DataRuntime 只管理核心数据状态和读取运行期
        +
可选能力只有 ingest 和 realtime 两个产品面
        +
MySQL / ClickHouse 作为官方存储适配器继续受支持
        +
无用插件入口和隐式副作用已经清理
        +
生命周期、可选安装和全量回归通过验收
```

只完成文件改名、只移动到新的 `src` 子目录、只增加构建开关，或只在文档中声明“可选”，均不算
完成本步骤。

## 12. 执行记录

### 2026-09-25：第一批实现

本批仅处理依赖方向、核心初始化副作用和运行期生命周期，不移动源码目录，也不宣称 Step 4 已完成。

**已实现**：

- 在 `data` 定义扩展 K 线、因子存储和实时数据源最小契约；插件解析与组合移到 `app`；
- `DataRuntime` 不再持有通用 `PluginManager`，数据域源码不再包含 `app/**` 头文件；共享内存服务角色由
  `app` 管理，数据层仅接收缓存回收策略；
- 引入进程级 `PluginRuntime`，默认 Session 不加载 HayakuExtra、授权、VIP 指标或商业报告；扩展 K 线
  仅在显式调用注册入口时装配；ClickHouse 仅在驱动配置明确选择时加载；
- 移除头文件静态初始化；`HayakuSession` 显式创建/释放 `DataRuntime`，并验证并发 Session 配置一致；
  Session 关闭后，数据运行期全局访问器不再隐式重建运行期对象；
- 默认不启动网络反馈和每日自动重载；INI 新增显式 `auto_reload` 开关，默认关闭；补充异常初始化回滚和
  NNG/HDF5 进程退出清理。
- Python 绑定构建支持通过 `HAYAKU_PYTHON` 指定解释器；本机使用 Python 3.10 完成 C++ 库与绑定构建。
- 新配置中的 MySQL 构建选项默认关闭；显式启用时仍使用当前内置 Driver 路径，独立适配器 target 尚待实现。

**尚未完成**：

- 尚未裁剪或删除 `app/plugin` 中的 BackTest、DataServer 和导入器，也未完成每个插件 ABI 的版本化；
- `plugins.h` 的仓库内调用方已改为包含具体接口，但兼容聚合头仍保留；尚未完成外部插件兼容性确认和删除；
- `ingest`、`realtime`、MySQL/ClickHouse 尚未形成独立 xmake target 或独立安装边界；显式开启 MySQL
  时仍会把客户端链接到核心共享库；
- 未完成全量插件调用方、Python/GUI 导入面、数据后端兼容性和生命周期审计；没有做源码移动；
- 尚未完成回归验收。基线阶段已有 small-test 44/44 用例、3300/3300 断言通过；本轮未运行测试。

**本轮验证**：

- `git diff --check` 通过；
- 本轮修改涉及的 34 个生产 C++ 源文件和 30 个已修改测试 C++ 源文件通过 `clang++ -fsyntax-only`；Python 绑定未纳入此项检查；
- 使用 `HAYAKU_PYTHON=/opt/homebrew/bin/python3.10`，`xmake -P . hayaku` 和 `xmake -P . core` 均编译、链接成功；
  Python 绑定已生成 `core310.so`；本轮未运行测试或 Python 导入检查；
- 此构建仍把 `app/plugin` 和 MySQL 编进 `hayaku`，只能证明当前整体构建可用，不能作为可选安装边界已完成的证据。
  `otool -L` 也显示本次 `libhayaku.dylib` 依赖 `libmysqlclient.21.dylib`。
- 显式配置 `mysql=false` 后，`hayaku` 编译、链接成功，`otool -L` 不再列出 MySQL 客户端；该配置仍编译
  `app/plugin`。验证后已恢复本机原有的 `mysql=true` 配置及 C++/Python 构建产物。

### 2026-09-25：`app/plugin` 仓库内调用方清点

此清点只覆盖当前仓库，不能代替外部 C++ 插件/用户的 ABI 使用调查；因此不据此删除兼容入口。

- 三种 `KDataTo*Importer` 均由 Python 绑定导出；HDF5/MySQL/ClickHouse 导入脚本和 QMT GUI 任务存在
  直接调用。应保留导入场景，并在统一 `ingest` 契约前避免删除现有入口。
- `check_data` 由 Python advanced 绑定导出，归入 `ingest`，不是核心 Session 的前置能力。
- `start_data_server` / `stop_data_server` 有 Python 绑定和 GUI 调用；`start_shm_server` /
  `stop_shm_server` 有中英文文档及 GUI 命令调用。它们是实际服务面，需在 realtime 统一入口设计中
  明确保留或合并，不能按“无调用方”删除。
- 仓库内未发现 `backtest(...)` façade 的调用点；其 C++ 入口和 `BackTestPluginInterface` 仍是潜在
  外部 API。现有 `StrategyEngine::run(BacktestRequest)` 仅接受单一 `KData`，无法等价承接旧入口的
  `StrategyContext`、逐 bar 回调、成交时机、做空和滑点参数；旧 VIP 回测实现又位于仓库外。因此需先补齐
  事件驱动能力并做结果对照，或明确 API 迁移策略，再移除旧插件入口。
- 仓库内没有其他文件包含 `plugins.h`；该聚合头仍暂留，待外部消费者清点后再删除。

### 2026-09-25：并行推进可选能力入口与安全性

以下是局部收口，不代表 `ingest`、`realtime` 已成为独立可安装模块。

- 新增 Python `hayaku.ingest.open_kdata_importer`，按 HDF5/MySQL/ClickHouse 后端创建并配置历史 K 线导入器；
  三个 pytdx 脚本与 QMT 导入任务改用统一入口。现有 C++ 导入器 façade 和 pybind 导出仍保留，尚未形成
  后端无关的 C++ 写入契约。QMT 在 `hdf5.enable=false` 时读取 MySQL 证券列表却仍写 HDF5 的旧行为未改。
- `PluginLoader` 在 `load(false)` 失败时安全返回，缺少入口符号时先卸载句柄，重复加载先释放旧实例；
  尚未补齐插件 ID、接口版本、兼容性校验与专用销毁路径。
- `SpotAgent` 增加报文长度与 FlatBuffers 有效负载检查、可空字段防护；停止时先结束接收，再排空处理队列。
  完整 realtime start/stop/health 契约、服务对象所有权和畸形报文测试仍未完成。
- 新 Python 入口通过 Python 3.10 语法检查，两个 C++ 修复通过单文件语法检查；本批未运行测试。
- 新增 `tools/arch/check_app_dependencies.py` 源码依赖扫描；当前覆盖 `data`、`analysis` 下 547 个 C/C++
  文件，未发现直接 `app` include/path 依赖。它不解析宏、条件编译或链接依赖；随后新增 3 个扫描器
  单元用例与 `.github/workflows/architecture.yml`，在 CI 固定此规则。
- 并行修改合入后，使用本机原有 `mysql=true` 配置再次构建 `xmake -P . core` 成功，生成 C++ 共享库及
  Python 3.10 绑定；这仍不是“仅核心”构建。
- 只读复核确认 SpotAgent 停止顺序可先排空接收任务再排空处理任务；随后修复 socket 初始化错误路径
  的资源释放、持续接收错误的忙循环，以及跨批次时间戳共享导致的数据竞争。处理回调中直接调用同一
  Agent 的 `stop()` 仍可能自等待，需在生命周期契约和测试中单独处理。
- SpotAgent 后续修复通过单文件 C++ 语法检查；整合后再次执行 `xmake -P . hayaku`，编译与链接成功。
  本轮未运行测试，且本机配置仍为显式 `mysql=true`。

**下一处构建边界的具体阻挡**：四个历史数据导入/检查 C++ façade（`KDataToHdf5Importer.cpp`、
`KDataToMySQLImporter.cpp`、`KDataToClickHouseImporter.cpp`、`checkdata.cpp`）没有仓库内核心 C++
直接调用者，适合作为首批 `ingest` target 候选。但 `hayaku_cpp/src/xmake.lua` 的广域 `./**.cpp`
规则会先把它们纳入 `hayaku`；Python 侧 `hayaku_pywrap/xmake.lua` 也广域收集源码，
`main.cpp` / `_plugin_main.cpp` 无条件注册可选绑定。因此只从 `app/plugin` 的显式文件组移除源码
并不能建立独立安装边界；必须连同 glob 排除、符号导出、Python 按需绑定和未安装路径一起设计验收。
`realtime` 还被 Session 的 `stopShmServer`、SpotAgent 和调度入口直接牵住；MySQL Driver 仍由
`DataDriverFactory` 编译期注册。它们不能仅靠 xmake 开关或移动文件完成独立安装。BackTest 则因旧回测
语义及外部 ABI 未澄清，暂不列入可直接删除批次。
此外广域 `add_headerfiles("./**.h")` 仍会安装可选头；macOS 链接使用 `-undefined dynamic_lookup`，
所以仅有 target 链接成功不足以证明边界，仍需检查仅安装 core 时的 Python 导入、符号和运行库依赖。

### 2026-09-25：可选导入构建边界与运行验收（继续执行中）

- C++/Python xmake 源码收集从通配改为显式分组，Python `core` 不再编入四个历史导入/检查绑定；
  `ingest` 为独立 Python 扩展 target。默认保留旧 C++ façade 符号以避免静默破坏外部二进制，
  严格拆分模式需显式设置 `legacy_ingest_abi=n`，其 C++/Python 组合尚未验收。
- `mysql=n`、默认兼容配置下，`xmake ingest` 与 `xmake core` 均编译链接成功；`otool -L` 显示
  `libhayaku.dylib` 不依赖 MySQL 客户端。实际 Python 3.10 `import hayaku` 成功，
  `test_ingest_boundary` 的 5 个用例通过，包括未加载可选扩展的核心导入路径。
- 移除静态初始化后暴露出预初始化日志空指针：Python core 与 C++ 测试入口曾在首次日志调用时崩溃。
  `Log.cpp` 增加无文件副作用的临时 logger，保留预设日志级别；Python 导入已恢复，C++ 全量测试仍待复跑。
- 插件 ABI 采用显式 v1 C 导出契约（插件 ID、ABI/接口版本、库内销毁），旧 `createPlugin`
  保持 legacy 加载。独立 `plugin-abi-test` 运行通过 7 个用例、56 条断言；这不保证跨编译器/STL ABI。
- 默认 wheel 的可选 ingest 二进制排除检查通过，且已修正原先被误标为 `py3-none-any` 的平台标签；
  独立可选 wheel 与真实安装/卸载组合还未验收。
- 实时路径已加入核心侧 `RealtimePort` 最小契约，Session/Strategy/自动重载不再直接调用 SpotAgent
  实现；`unit-test` 重新构建成功，真实 NNG inproc 定向测试 1/1 用例、14/14 断言通过，覆盖回调自停、
  回调内拒绝同步关闭 Session、外部 join 与重启。独立 realtime target 尚待完成。
- 插件采用进程级 pin：Session 关闭不会卸载已载入动态库。`PluginManager::clear()` 和直接 Loader
  重新加载会使已借出的裸接口指针失效，因此不支持运行中的热卸载；ClickHouse Driver 等插件对象
  也必须先从全局工厂释放，不能仅停止 façade 后就 `dlclose`。

### 2026-09-26：删除已确认的旧入口

- 仓库内无调用方的旧 `backtest` 插件 façade、`BackTestPluginInterface` 和 `HAYAKU_PLUGIN_BACKTEST`
  已删除；`StrategyEngine` 的现有回测能力保持不变。旧接口的逐 bar 回调、做空、滑点等语义未迁移，
  仓库外用户需自行迁移，属于明确的 C++ 源码和二进制不兼容变更。
- 无仓库内引用的 `plugins.h` 聚合头已删除；扩展代码应直接包含需要的接口头及 `PluginIds.h`。
- 删除 `legacy_ingest_abi` 兼容构建开关，导入/检查实现只进入独立 `hayaku-ingest` target；
  普通核心不再提供历史导入 façade 符号。导入能力本身保留在可选 target 和 Python `hayaku.ingest`。
- 删除 Python `hayaku.core` 和 `hayaku.advanced` 的旧导入别名；`hayaku.ingest` 成为唯一的
  Python 历史导入入口。同步更新公共 API 清单、边界测试和中英文迁移说明。
- 这些删除不等于整个 Step 4 完成。仍未完成：`realtime` 独立 target/安装验证，MySQL/ClickHouse
  独立适配器和真实后端矩阵，统一的后端无关 C++ 写入契约，旧 BackTest 行为等价迁移（如仍需要），
  以及可选安装组合、真实数据测试、性能与内存验收。
- 删除后的 macOS/Python 3.10、`mysql=n` 配置中，`xmake build core`、`xmake build ingest`、
  `xmake build unit-test` 均成功；`xmake run unit-test` 为 805/805 用例、208450/208450 断言通过，
  Python 主测试入口为 64/64 用例通过，ingest 专项为 5/5。`nm` 检查 core 中无历史导入 façade 符号，
  `otool -L` 检查 core 不依赖 MySQL 或独立 ingest 库；`git diff --check` 通过。

### 2026-09-26：可选安装矩阵与独立实时 target

- 核心构建排除 SpotAgent、GlobalSpotAgent、ShmServer 和 DataServer 实现；`RealtimePort` 仍留在 core。
  新的 `hayaku-realtime` C++ target 和 Python `realtime` target 复用原源码位置，没有新增 `src` 顶层目录。
  `core310.so`、`libhayaku.dylib` 均不链接 `libhayaku-realtime.dylib`；普通核心导入不加载实时扩展。
- Python 实时入口通过 `hayaku.advanced` 按需加载 `hayaku_realtime_native`；只装 core 时可导入
  `hayaku.advanced`，访问实时 API 才提示安装可选包。SpotAgent 跨动态库测试改用本机临时 TCP
  连接；原 NNG `inproc://` 只适用于同一动态库实例，不能作为拆库后的跨模块验收。
- 分别构建 `hayaku`、`hayaku-ingest`、`hayaku-realtime` 三个 macOS/Python 3.10 wheel；
  三者文件路径交集为 0。core wheel 不包含可选 native 文件、MySQL 客户端或 ABI 测试动态库。
  临时 venv 中验证“仅 core → 加装 ingest 与 realtime → 卸载 ingest → 卸载 realtime”：
  核心始终可导入，可选功能只在对应包已安装时加载，卸载可选包不会删除核心文件。
- 本轮完整 C++ 测试 805/805 用例、208452/208452 断言通过；SpotAgent 定向测试 1/1 用例、
  16/16 断言通过；Python 主测试入口 66/66 用例通过。NNG TCP 定向测试需要本机回环监听权限。
  单独过滤运行 Session 用例时，`ready()` 与异步 preload 有竞态；完整 C++ 套件本轮通过，
  该定向测试稳定性仍需另行处理。
- 未完成的原 Step 4 条件仍包括：MySQL/ClickHouse 独立适配器与真实后端矩阵、后端无关的 C++
  写入契约、真实数据/性能/内存验收，以及仓库外插件消费者的迁移验证。
