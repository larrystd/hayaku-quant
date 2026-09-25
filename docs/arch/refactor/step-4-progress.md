# 第 4 步：数据层、应用层与可选能力收口

> 状态：方案草案，尚未执行
>
> 前置阶段：[第 3 步：最终架构冻结与旧体系退役](step-3-progress.md)
>
> 决策日期：2026-09-25
>
> 本步骤只定义目标、边界、处理顺序和验收标准；未授权前不移动或删除生产代码。

## 1. 阶段背景

第 3 步已经把研究、回测和模拟执行收口到 `DataEngine`、`ExecutionEngine`、
`StrategyEngine` 与 `HikyuuSession`。但 `hikyuu_cpp/src/data` 与 `hikyuu_cpp/src/app` 的
职责仍未完全分开。

`hikyuu_cpp/src/data` 当前同时包含：

- 证券、K 线、查询、权息、财务等数据领域类型；
- `DataEngine`、Driver、缓存、预加载和数据运行期；
- 指标和因子计算；
- HDF5、SQLite、MySQL、TDX、CSV 等存储或读取实现；
- IPC、共享内存客户端和实时更新路径；
- 对应用初始化、调度器、遥测、设备授权、VIP 扩展和商业因子插件的反向依赖。

`hikyuu_cpp/src/app` 当前同时承担：

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

当前 `hikyuu_cpp/src/xmake.lua` 只有一个 `target("hikyuu")`。`src` 下新增任何子目录，默认仍会
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

- `HikyuuSession`、`SessionOptions` 和必要的进程级生命周期管理；
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

`import hikyuu`、构造 Session 和只运行回测时，不得连接网络、启动行情服务、写入设备 UID、加载
商业插件或要求安装 MySQL/ClickHouse 客户端。

### 3.3 两个可选能力的边界

`ingest` 只负责把外部历史数据准备成 Hikyuu 可读取的存储数据，包括：

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

本步骤以 `hikyuu_cpp/src/data` 和 `hikyuu_cpp/src/app` 为两个主要审计对象，并同步处理它们的
直接公共面：

- `hikyuu_cpp/src/data/**`；
- `hikyuu_cpp/src/app/**`；
- `DataEngine`、`DataRuntime`、Driver Factory、存储实现和 IPC 路径；
- 指标、因子中对应用插件、授权和遥测的依赖；
- `hikyuu_pywrap/advanced/**` 中对应绑定；
- `hikyuu_pywrap/data/**` 与 `hikyuu/advanced/**`、`hikyuu/data/**`、GUI 中的直接调用；
- xmake 构建选项、安装规则和插件加载规则；
- 对应 C++、Python、数据读取、生命周期和导入边界测试；
- 对应中英文用户文档。

跨目录修改只能用于完成上述公共面收口，不能借机重排 `execution`、`strategy` 或其他无关目录。

### 4.2 明确不纳入范围

- 不重新设计三个 Engine 的业务模型；
- 不重新迁移 `hikyuu_cpp/src` 的顶层目录；
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
HikyuuSession / app
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
- 自动加载 VIP 指标、HkuExtra、TMReport 或商业因子存储；
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
- `DataRuntime` 不再包含 app 调度器、`plugins.h`、device、hkuextra、extind 和 sysinfo；
- `Factor`、`FactorSet` 不再直接调用 app 中的授权和商业 factor façade；
- `KQuery`、`KData`、`Stock` 不再直接调用 `app/plugin/hkuextra`；
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
| `hkuextra.*`、`HkuExtraPluginInterface.h` | 商业扩展 | 从核心公共面和默认初始化移除 |
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
- `HikyuuSession::close()` 必须关闭它启动的线程、服务和运行期对象；
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
hikyuu                    # core
hikyuu-ingest             # 可选的数据准备能力
hikyuu-realtime           # 可选的实时行情能力
hikyuu-storage-mysql      # 官方 MySQL 适配器
hikyuu-storage-clickhouse # 官方 ClickHouse 适配器
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

## 9. 实施顺序

### 9.1 冻结清单

- 记录 `data` 每个子目录、Driver、运行期入口及其 app 反向依赖；
- 记录每个 façade、接口、插件 ID、动态库名称、Python 绑定和调用方；
- 区分核心内部调用、官方可选能力、GUI 调用和未知外部调用；
- 为计划删除的 API 建立拒绝列表或编译负向测试。

### 9.2 先切断 `data` 对 `app` 的反向依赖

- 从领域类型、指标、因子、Driver Factory 和 DataRuntime 中移除 app 头文件；
- 将 Driver 注册、可选能力发现和服务启动改为由 app 显式装配；
- 把 IPC/共享内存协商放到 realtime 边界，不在默认数据初始化中执行；
- 逐批运行数据查询、指标、因子、复权和回测金标，禁止行为漂移。

### 9.3 收窄 DataRuntime 和 Driver

- 让 DataRuntime 只持有核心数据状态、Driver、缓存和预加载资源；
- 移出通用 PluginManager、商业插件预加载、服务角色和应用调度；
- 固定最小 Driver 契约，并以 HDF5/SQLite 作为默认实现验证；
- 分别验证 MySQL 和 ClickHouse 适配器可安装、可缺失且版本兼容。

### 9.4 删除确定无用的入口

- 核对并处理 BackTest 插件入口；
- 删除 `plugins.h`，改为最小显式包含；
- 删除无调用方、无文档场景和无测试的 façade；
- 每个删除批次独立通过构建和测试。

### 9.5 收紧生命周期

- 统一 Session、DataRuntime、SpotAgent、服务和插件管理器的关闭顺序；
- 修复裸插件指针、后台线程和一次性调度器问题；
- 将网络、遥测和常驻服务改为显式启动。

### 9.6 收口可选能力

- 为 `ingest` 建立一个后端无关的导入契约；
- 为 `realtime` 建立一个显式 start/stop 契约；
- 将 MySQL、ClickHouse 接到统一存储契约；
- 清理 Python 顶层及 `advanced` 中无边界的公共入口。

### 9.7 最后决定物理拆分

- 先验证无可选组件时核心可以独立构建和运行；
- 再增加独立 target、安装包或扩展仓库；
- 最后才移动文件，并同步构建、安装、文档和测试路径。

## 10. 验收标准

### 10.1 结构与依赖

- 默认核心只包含研究、回测、分析和模拟执行所需能力；
- 用户可见的可选能力只有 `ingest` 和 `realtime`，存储后端作为适配器存在；
- 未创建仅用于分类的 `src/ingest`、`src/realtime`；
- `hikyuu_cpp/src/data/**` 不再包含 `app/**` 头文件；
- `DataRuntime` 不再持有通用 PluginManager，不再启动服务、调度器、遥测或商业插件；
- `DataEngine` 仍是稳定只读门面，Driver 和缓存实现不泄漏到普通公共 API；
- 指标和因子核心计算不依赖设备授权、商业 façade 或 ClickHouse；
- IPC、Shm 和实时更新只通过最小 data 端口接入，不参与默认数据初始化；
- `plugins.h` 不再存在，剩余插件接口均有明确所有者和调用方；
- 核心可以在没有 MySQL、ClickHouse、VIP 插件和实时服务的环境中构建和运行；
- MySQL、ClickHouse 继续有官方读写、升级和兼容性测试。

### 10.2 行为与生命周期

- `import hikyuu` 无网络请求、遥测、设备 UID 写入和后台服务启动；
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
