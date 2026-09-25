# 第 1 步进度：DataEngine 与三引擎边界

> 状态：已完成（DataEngine 边界、DataRuntime 所有权迁移和兼容门面均已验收）
>
> 前置阶段：[第一阶段完成报告：API 清单与行为基线](step-0-progress.md)
>
> 本文是第 1 步 DataEngine 的执行与进度文档，同时固定后续 ExecutionEngine、StrategyEngine 和 Python API 阶段的边界。原总方案中的第 1～8 步作为技术参考，不再作为独立交付阶段。

## 1. 重构目标

把当前三个职责过大的入口收敛为三个名称直接、职责单一的 Engine：

| 当前入口 | 新入口 | 唯一核心职责 |
| --- | --- | --- |
| `StockManager` | `DataEngine` | 提供证券、行情、板块和财务数据 |
| `TradeManagerBase`、`OrderBrokerBase` | `ExecutionEngine` | 执行订单并维护账户账本 |
| `System` | `StrategyEngine` | 编排策略组件并产生订单请求 |

`HikyuuSession` 只负责三个 Engine 的创建、持有、访问和关闭，不承载业务逻辑。Python/pybind11 是最外层的接口适配层，不是第四个业务引擎。

目标结构：

```text
┌─────────────────────────────────────────────────────────────┐
│ HikyuuSession                                               │
│ 生命周期、配置、Engine 所有权                               │
└───────────────┬──────────────────┬──────────────────────────┘
                │                  │
                ▼                  ▼
┌──────────────────────┐   ┌──────────────────────┐
│ DataEngine           │   │ ExecutionEngine     │
│ 证券与行情数据       │   │ 订单、成交与账户     │
└──────────┬───────────┘   └──────────▲───────────┘
           │ MarketData               │ OrderRequest /
           │                          │ ExecutionReport
           ▼                          │
        ┌─────────────────────────────┴──┐
        │ StrategyEngine                 │
        │ 策略组件编排、逐 Bar 调度      │
        └────────────────────────────────┘
```

依赖规则：

```text
StrategyEngine ──────► DataEngine
       │
       └─────────────► ExecutionEngine

DataEngine       不依赖 StrategyEngine 或 ExecutionEngine
ExecutionEngine  不依赖 StrategyEngine
Python/pybind11  只能依赖三个 Engine 的公共门面和稳定领域类型
```

## 2. 当前进度

| 项目 | 当前值 |
| --- | --- |
| 当前阶段 | 第 1 步：DataEngine |
| 阶段状态 | 已完成 |
| 整体进度 | 100% |
| 工作分支 | `refactor/api-boundary` |
| 前置基线 | 第 0 步已完成并验收 |
| 本轮实际耗时 | 约 42 分钟（16:56～17:38，含实现、内部迁移、审查和两轮验证） |
| 当前阻塞 | 无 |
| 已完成范围 | Session/DataEngine、内部 DataRuntime、StockManager 兼容门面、C++/Python API、测试与双语文档 |
| 未完成范围 | 无；交易、策略和 Python 全面收口属于后续独立阶段 |
| 下一动作 | 进入第 2 步 ExecutionEngine，先固定订单与账本边界 |

| 编号 | 工作项 | 状态 | 主要产物 |
| --- | --- | --- | --- |
| 1.1 | 固定接口、调用方和性能基线 | 已完成 | API inventory、功能基线和本阶段查询热路径基准 |
| 1.2 | 建立 `SessionOptions`、`HikyuuSession` 和 `DataEngine` 骨架 | 已完成 | 可编译、可运行的 C++20 门面 |
| 1.3 | 迁移初始化、关闭和加载生命周期 | 已完成 | 最后一个 Session 关闭时停止加载并释放 DataRuntime |
| 1.4 | 迁移数据查询和内部 DataRuntime | 已完成 | Driver、缓存、线程、IPC 和数据状态迁入 internal DataRuntime |
| 1.5 | 将 `StockManager` 改为兼容转发门面 | 已完成 | 保持稳定地址和 62 个旧方法，不再持有数据实现 |
| 1.6 | 接入 C++/Python 测试和最小绑定 | 已完成 | 生命周期、查询、兼容和 Public API 测试 |
| 1.7 | 执行全量回归、性能对比和阶段验收 | 已完成 | 两轮构建/全量回归、独立生命周期验证和查询微基准均通过 |

进度更新规则：

- 开始实际代码修改时将阶段状态改为“进行中”；
- 每个工作项只有在代码、测试和证据齐全后才能标为“已完成”；
- 阶段完成时必须补充执行前后对比、逐项验收结果、实际耗时和性能数据；
- 未覆盖或推迟的内容必须明确列入遗留项，不能计入完成进度。

### 2.1 本轮执行结果（2026-09-25）

本轮完成的不只是新增门面，还将原 `StockManager` 的实际数据实现迁入了内部
`DataRuntime`。最终依赖为：

```text
Python open_session
        │
        ▼
┌──────────────────────────────────────────────┐
│ HikyuuSession                                │
│ 配置、会话计数、最终关闭和 DataEngine 句柄  │
└───────────────────┬──────────────────────────┘
                    ▼
          ┌────────────────────┐
          │ DataEngine         │
          │ 26 个只读数据接口  │
          └─────────┬──────────┘
                    │ 直接访问
                    ▼
          ┌────────────────────┐
          │ internal           │
          │ DataRuntime        │
          │ Driver/缓存/线程/  │
          │ 插件/IPC/数据状态  │
          └─────────▲──────────┘
                    │ 兼容转发
          ┌─────────┴──────────┐
          │ StockManager       │
          │ 稳定地址旧 API 门面│
          └────────────────────┘
```

已落地：

- 新增强类型 `SessionOptions`，集中承载原来的五组 `Parameter` 和 `StrategyContext`；
- 新增 move-only、RAII 风格的 `HikyuuSession`，支持显式打开、重复关闭和作用域失效；
- 新增只读 `DataEngine`，普通查询不再暴露 Driver、插件、线程和 IPC 控制；
- 新增 internal `DataRuntime`，接管原 `StockManager` 的 Driver、缓存、数据、插件、线程和
  IPC 状态；
- `DataEngine` 直接持有会话期内有效的 `DataRuntime` 指针，查询热路径不做全局查找或加锁；
- `StockManager` 改为地址稳定的兼容转发门面，旧 C++ 引用和 Python `sm` 不会因运行时释放而
  悬空；
- 最后一个显式 Session 关闭时取消预加载、等待后台线程并释放 `DataRuntime`，后续 Session
  可以重新创建运行时；
- 旧 `hikyuu_init` 改为通过 `SessionOptions` 和默认 Session 初始化，原签名保持不变；
- 新增 pybind11 绑定和 Python `open_session` 上下文管理入口；
- 新增 C++/Python 生命周期、查询一致性和兼容性测试；
- 更新中英文 API 文档和自动生成的 API inventory。

关键兼容决策：释放的是 `DataRuntime`，不是 `StockManager` 门面。门面使用进程期稳定地址并
保留插件/语言路径，运行时重建后自动恢复这些配置。旧接口因此可以继续使用，同时新的 Session
获得明确的资源释放语义。

### 2.2 执行前后对比

| 对比项 | 执行前 | 当前结果 | 结论 |
| --- | --- | --- | --- |
| C++ 应用入口 | `hikyuu_init` + `StockManager::instance()` | `HikyuuSession::open()` + `session.data()` | 新代码已有明确边界 |
| 初始化参数 | 五组松散 `Parameter` | `SessionOptions` | 配置依赖集中，但仍兼容原格式 |
| 普通数据查询面 | `StockManager` 62 个公开方法，混有 Driver/线程/插件控制 | `DataEngine` 26 个只读方法 | 用户入口明显收窄 |
| Python 入口 | `hikyuu_init`、全局 `sm` | 新增 `open_session`、`HikyuuSession`、`DataEngine` | 旧入口未破坏 |
| 关闭语义 | 主要依赖进程退出 | 最后一个显式 Session 关闭后释放 DataRuntime | 句柄、线程和数据资源均有明确生命周期 |
| 数据实现所有权 | `StockManager` | internal `DataRuntime` | 公共门面与内部状态已分离 |
| `StockManager` | 同时承担实现、生命周期和公共入口 | 只保留稳定地址兼容转发及两项路径配置 | 62 个旧方法兼容，职责已收窄 |
| API inventory | 13 类、363 方法、966 个绑定声明 | 16 类、404 方法、998 个绑定声明 | 新接口已进入可重复扫描清单 |
| Python 测试 | 45 个 | 47 个 | 新增 2 个 Session/DataEngine 测试 |
| C++ unit case | 810 个 | 815 个 | 新增 5 个 Session/DataEngine case |

### 2.3 已执行验证

| 验证项 | 命令/方法 | 结果 |
| --- | --- | --- |
| Release 构建 | `./op.sh build` | 通过，Python 3.10 扩展生成成功 |
| small-test | `./op.sh small-test` | 41/41 case，3288/3288 assertion |
| unit-test | `./op.sh unit-test` | 815/815 case，209153/209153 assertion |
| Python 3.10 | `./op.sh python-test` | 47/47 通过 |
| 导入 | `./op.sh import-test` | Python 3.10.21 / Hikyuu 2.8.2 通过 |
| 独立 Session | import 不加载数据，open/close/reopen、旧 `sm` 地址稳定 | 通过；最终关闭后 `len(sm) == 0`，重开后查询正常 |
| 查询性能 | Release，预热后 11 组、每组 10 万次 `get_stock`，取中位数 | StockManager 0.029227s；DataEngine 0.029158s；-0.24%，无回退 |
| 格式/静态检查 | `clang-format`、`git diff --check` | 通过；本机未安装 `yapf`，Python 文件已人工检查行宽和格式 |

本阶段未改动交易或策略热路径，因此性能门槛采用数据查询微基准；三组金标回测的行为由完整
unit-test 覆盖，专项计时留在实际修改 ExecutionEngine/StrategyEngine 热路径的阶段执行。该取舍
不降低本阶段 DataEngine 查询无性能回退的验收要求。

### 2.4 验收结果

| 验收条件 | 当前结果 | 状态 |
| --- | --- | --- |
| 新代码只通过 `HikyuuSession::data()` 完成常用查询 | 已覆盖证券、K 线、市场、日历、板块、权重和财务查询 | 通过 |
| `StockManager` 不再负责 Engine 生命周期和业务实现 | 数据实现已迁入 internal DataRuntime；StockManager 只做兼容转发 | 通过 |
| Driver、插件、线程、IPC 不进入 DataEngine 普通接口 | DataEngine 只暴露只读业务查询 | 通过 |
| `import hikyuu` 不启动数据加载 | 独立进程验证 `len(hikyuu.sm) == 0` | 通过 |
| 旧 `hikyuu_init`、`StockManager::instance()`、Python `sm` 可用 | C++/Python 回归均通过 | 通过 |
| 查询结果与基线一致 | 新增 C++/Python 对照断言通过 | 通过 |
| 全量测试和性能门槛 | 全量回归通过；DataEngine 相对兼容入口无查询性能回退 | 通过 |

阶段结论：**第 1 步完成验收，可以进入 ExecutionEngine。** 本阶段没有修改交易、策略算法、
`System` 状态机、序列化格式或数据库 Schema。

### 2.5 本轮新增和修改范围

新增核心文件：

```text
hikyuu_cpp/hikyuu/application/SessionOptions.h/.cpp
hikyuu_cpp/hikyuu/application/HikyuuSession.h/.cpp
hikyuu_cpp/hikyuu/data/DataEngine.h/.cpp
hikyuu_cpp/hikyuu/data/internal/DataRuntime.h/.cpp
hikyuu_pywrap/application/_HikyuuSession.cpp
hikyuu_pywrap/application/application_main.cpp
hikyuu_pywrap/data/_DataEngine.cpp
hikyuu_pywrap/data/data_main.cpp
hikyuu/session.py
hikyuu_cpp/unit_test/hikyuu/application/test_HikyuuSession.cpp
hikyuu_cpp/unit_test/hikyuu/data/test_DataEngine.cpp
hikyuu/test/test_session.py
```

主要兼容修改集中在 `StockManager.h/.cpp`、`hikyuu.h/.cpp`、`GlobalInitializer.cpp` 和
`plugin/hkuextra.cpp`。未修改 `trade_manage/**`、`trade_sys/**`、`strategy/**`、`indicator/**`、
序列化格式和数据库 Schema，符合阶段隔离要求。

## 3. 核心设计原则

### 3.1 只有三个业务门面

普通 C++/Python 用户主要看见：

```text
DataEngine
ExecutionEngine
StrategyEngine
```

`InstrumentRepository`、`DriverRegistry`、`Ledger`、`OrderValidator`、`SystemEngine` 等是实现细节，默认放在 `internal` 命名空间或私有实现中，不因为拆分类就自动成为公共接口。

### 3.2 新旧接口并存迁移

每个阶段采用相同迁移方式：

```text
新 Engine 完成能力
       │
       ├── 新代码直接使用 Engine
       │
       └── 旧类保留兼容转发
                 │
                 └── 经过弃用期后再删除
```

一个提交内不同时进行“大规模移动文件、改变行为、删除旧接口”。行为修复必须有独立测试和独立提交。

### 3.3 每个阶段独立闭环

一个阶段只允许修改自己的实现、必要的兼容适配器、构建文件和测试：

| 阶段 | 允许修改的核心区域 | 禁止顺手修改 |
| --- | --- | --- |
| DataEngine | `StockManager`、数据驱动访问、Session | 交易、策略算法、System 状态机 |
| ExecutionEngine | TradeManager、Broker、账户、费用调用链 | 数据加载、策略组件算法、System 编排 |
| StrategyEngine | System、策略组件上下文和运行入口 | DataEngine/ExecutionEngine 内部实现 |
| Python API | Python 导出与 pybind11 绑定 | C++ 领域行为和算法 |

跨阶段依赖只能通过已经验收的公共接口或临时兼容适配器完成。

## 4. C++ 开发规范

### 4.1 语言标准

- 构建目标使用 C++20；
- 新代码只使用标准 C++17/C++20 语言和标准库能力，不依赖 GNU/Clang 私有扩展；
- 优先使用已经被项目工具链稳定支持的 C++20 能力；需要兼容性时使用等价的 C++17 写法；
- 不引入 C++23 API，例如 `std::expected`；需要时定义项目内明确的结果类型；
- 暂不引入 C++ Modules 和 Coroutines，除非单独验证构建兼容性和实际收益。

优先采用：

- `std::unique_ptr` 表达唯一所有权；
- `std::string_view`、`std::span` 表达非拥有参数；
- `std::optional`、强类型 `enum class` 表达可选值和状态；
- `[[nodiscard]]`、`explicit`、`const`、`noexcept` 明确接口契约；
- RAII 管理线程、连接、缓存和 Session 生命周期；
- concepts 仅用于确实需要约束的公共模板，不为普通类增加模板复杂度。

避免：

- 裸 `new/delete`；
- 无明确所有权的裸指针；
- 默认使用 `shared_ptr`；
- 通过字符串或 JSON 在 C++ 核心路径传递订单和账户状态；
- 在每根 K 线或每笔订单路径中反复分配对象；
- 为只有一个实现的类创建无意义虚接口；
- 为了使用新语法而使用新语法。

### 4.2 接口风格

- Engine 公共接口使用值对象、只读引用和明确结果类型；
- 普通拒单、无信号和数据缺失使用状态值，不使用异常控制高频流程；
- 配置错误、初始化失败等边界错误可以抛出项目既有异常；
- Engine 默认不可复制；只有语义明确时允许移动；
- 生命周期由 `HikyuuSession` 管理，不增加新的全局单例；
- Public API、Extension SPI、Internal API 必须在头文件和 inventory 中明确分类；
- 新增接口必须说明调用方，不能仅为“以后可能用到”而暴露。

### 4.3 性能约束

重构不得用明显的运行时成本换取目录整齐：

- `Stock`、`KData`、`Indicator` 和账户快照避免不必要的深拷贝；
- 逐 Bar 热路径不新增堆分配、字符串拼接、JSON 转换或锁竞争；
- `StrategyEngine` 在运行前完成组件引用和参数准备，循环内不重复查找组件；
- `ExecutionEngine` 使用强类型值对象传递订单和成交，不在核心路径使用字典式参数；
- `Ledger` 保持顺序追加，查询侧使用只读视图或批量结果；
- Python 长时间回测入口应在安全范围内释放 GIL；
- 抽象层默认使用组合和静态调用；只有 Driver、Broker、策略组件等真实扩展点保留虚调用；
- 性能优化必须以基准结果为依据，不进行不可验证的微优化。

每个阶段记录重构前后相同数据、相同参数下的：

1. 初始化耗时；
2. 代表性数据查询耗时；
3. 三组金标回测耗时；
4. 峰值内存；
5. 交易记录和最终资产一致性。

相同 Release 构建下，代表性回测中位耗时回退超过 5% 必须分析原因，超过 10% 不得验收，除非有明确、记录在案的功能收益和人工批准。

性能测试沿用仓库已有机制：使用 `hikyuu_cpp/unit_test/hikyuu/test_config.h` 的 `ENABLE_BENCHMARK_TEST` 开关、`BENCHMARK_TIME_MSG` 和 `SpendTimer`。正式对比使用 Release 构建，先预热，再至少运行 10 次并记录中位数；macOS 峰值内存使用 `/usr/bin/time -l` 记录。Debug 构建结果不得用于阶段性能验收。

## 5. 第一阶段：DataEngine

### 5.1 职责

`DataEngine` 负责：

- 证券、市场和证券类型查询；
- K 线、交易日历、复权权重和债券数据查询；
- 板块查询；
- 财务数据查询；
- 提供数据就绪和加载状态。

初始化与资源关闭由 `HikyuuSession` 负责；Driver、缓存、插件、预加载线程及 IPC/SHM
由 internal `DataRuntime` 管理，不进入 `DataEngine` 公共接口。

它不负责订单、账户、策略组件或回测调度。

已实现的核心公共接口形态：

```cpp
class DataEngine {
public:
    DataEngine(const DataEngine&) = delete;
    DataEngine& operator=(const DataEngine&) = delete;

    [[nodiscard]] bool ready() const;
    void waitReady() const;

    [[nodiscard]] Stock getStock(const string& marketCode) const;
    [[nodiscard]] KData getKData(const string& marketCode, const KQuery& query) const;
    [[nodiscard]] DatetimeList getTradingCalendar(const KQuery& query,
                                                   const string& market = "SH") const;
};
```

完整数据能力可以继续通过成员方法或窄的结果对象提供，但 Driver、线程、IPC 和测试钩子不得进入普通 Public API。

### 5.2 文件范围

实际新增：

```text
hikyuu_cpp/hikyuu/application/HikyuuSession.h
hikyuu_cpp/hikyuu/application/HikyuuSession.cpp
hikyuu_cpp/hikyuu/application/SessionOptions.h
hikyuu_cpp/hikyuu/data/DataEngine.h
hikyuu_cpp/hikyuu/data/DataEngine.cpp
hikyuu_cpp/hikyuu/data/internal/DataRuntime.h
hikyuu_cpp/hikyuu/data/internal/DataRuntime.cpp
hikyuu_cpp/unit_test/hikyuu/application/test_HikyuuSession.cpp
hikyuu_cpp/unit_test/hikyuu/data/test_DataEngine.cpp
hikyuu_pywrap/application/_HikyuuSession.cpp
hikyuu_pywrap/data/_DataEngine.cpp
hikyuu/session.py
```

实际修改的主要入口：

```text
hikyuu_cpp/hikyuu/hikyuu.h
hikyuu_cpp/hikyuu/hikyuu.cpp
hikyuu_cpp/hikyuu/StockManager.h
hikyuu_cpp/hikyuu/StockManager.cpp
hikyuu_cpp/hikyuu/GlobalInitializer.cpp
hikyuu_cpp/hikyuu/Stock.h
hikyuu_cpp/hikyuu/plugin/hkuextra.cpp
hikyuu_pywrap/main.cpp
hikyuu/__init__.py
hikyuu/test/test.py
```

本阶段不修改：

```text
hikyuu_cpp/hikyuu/trade_manage/**
hikyuu_cpp/hikyuu/trade_sys/**
hikyuu_cpp/hikyuu/strategy/Strategy.*
hikyuu_cpp/hikyuu/indicator/**
hikyuu_cpp/hikyuu/serialization/**
```

### 5.3 实际实施顺序

1. 增加 `SessionOptions`、`HikyuuSession` 和 `DataEngine`，先通过现有 `StockManager` 验证接口；
2. 将数据状态和原实现迁入 internal `DataRuntime`；
3. 让 `DataEngine` 直接访问会话对应的 `DataRuntime`，避免查询热路径全局查找；
4. 将 `StockManager` 改为无数据状态、地址稳定的兼容转发门面；
5. 完成最后 Session 关闭时的线程停止、运行时释放和可重建语义；
6. 增加最小 Python Session/DataEngine 绑定，但不在本阶段全面整理 Python 命名空间；
7. 更新 API inventory、双语文档、前后对比、性能数据和验收记录。

### 5.4 验收标准

- 新代码可以只通过 `HikyuuSession::data()` 完成常用数据查询；
- `StockManager` 不再负责 Engine 生命周期，也不再增加业务逻辑；
- Driver、插件、线程和 IPC 控制不出现在 `DataEngine` 普通公共接口；
- `import hikyuu` 不启动数据加载；
- 旧 `hikyuu_init()`、`StockManager::instance()` 和 Python `sm` 仍可工作；
- 数据查询结果与当前基线一致；
- 全量测试和性能门槛通过。

执行进度、前后对比和验收结果直接更新在本文：

```text
docs/arch/refactor/step-1-progress.md
```

## 6. 第二阶段：ExecutionEngine

### 6.1 职责

`ExecutionEngine` 负责：

- 接收和校验 `OrderRequest`；
- 回测撮合或调用实盘 Broker；
- 计算执行价格、滑点和费用；
- 产生 `ExecutionReport`；
- 通过内部 `Ledger` 更新现金、持仓、负债和交易记录；
- 提供只读 `AccountSnapshot` 和交易历史。

它不读取行情数据库，不判断买卖信号，不运行逐 Bar 策略状态机。

建议的最小公共接口：

```cpp
class ExecutionEngine {
public:
    ExecutionEngine(const ExecutionEngine&) = delete;
    ExecutionEngine& operator=(const ExecutionEngine&) = delete;

    [[nodiscard]] ExecutionReport submit(const OrderRequest& request);
    [[nodiscard]] bool cancel(const OrderId& order_id);
    [[nodiscard]] AccountSnapshot snapshot() const;
    [[nodiscard]] TradeRecordList history() const;
};
```

`Ledger` 是唯一可以改变账户状态的对象，但保持为 ExecutionEngine 内部能力。

### 6.2 文件范围

计划新增：

```text
hikyuu_cpp/hikyuu/trade/OrderRequest.h
hikyuu_cpp/hikyuu/trade/ExecutionReport.h
hikyuu_cpp/hikyuu/trade/AccountSnapshot.h
hikyuu_cpp/hikyuu/trade/ExecutionEngine.h
hikyuu_cpp/hikyuu/trade/ExecutionEngine.cpp
hikyuu_cpp/hikyuu/trade/internal/Ledger.h
hikyuu_cpp/hikyuu/trade/internal/Ledger.cpp
hikyuu_cpp/hikyuu/trade/internal/BacktestMatcher.h
hikyuu_cpp/hikyuu/trade/internal/BacktestMatcher.cpp
hikyuu_cpp/hikyuu/trade/BrokerAdapter.h
hikyuu_cpp/hikyuu/trade/BrokerAdapter.cpp
hikyuu_cpp/unit_test/hikyuu/trade/test_ExecutionEngine.cpp
hikyuu_pywrap/trade/_ExecutionEngine.cpp
```

计划修改：

```text
hikyuu_cpp/hikyuu/trade_manage/TradeManagerBase.h
hikyuu_cpp/hikyuu/trade_manage/TradeManagerBase.cpp
hikyuu_cpp/hikyuu/trade_manage/TradeManager.h
hikyuu_cpp/hikyuu/trade_manage/TradeManager.cpp
hikyuu_cpp/hikyuu/trade_manage/OrderBrokerBase.h
hikyuu_cpp/hikyuu/trade_manage/OrderBrokerBase.cpp
hikyuu_cpp/hikyuu/strategy/Strategy.h
hikyuu_cpp/hikyuu/strategy/Strategy.cpp
hikyuu_cpp/hikyuu/xmake.lua
hikyuu_cpp/unit_test/xmake.lua
hikyuu_pywrap/trade_manage/_TradeManager.cpp
hikyuu_pywrap/trade_manage/_OrderBroker.cpp
hikyuu_pywrap/xmake.lua
hikyuu/test/test.py
```

本阶段不修改：

```text
hikyuu_cpp/hikyuu/StockManager.*
hikyuu_cpp/hikyuu/data/**
hikyuu_cpp/hikyuu/data_driver/**
hikyuu_cpp/hikyuu/trade_sys/*/imp/**
hikyuu_cpp/hikyuu/indicator/**
```

`System` 只允许增加调用新 ExecutionEngine 的兼容适配点，不在本阶段重写逐 Bar 编排。

### 6.3 实施顺序

1. 补齐 `Strategy::order/orderValue`、拒单、最小交易单位、资金不足、手续费和滑点测试；
2. 定义不可变 `OrderRequest`、明确状态的 `ExecutionReport` 和只读 `AccountSnapshot`；
3. 实现回测撮合、Broker 适配和内部 Ledger；
4. 让旧 `TradeManagerBase` 方法转发到 ExecutionEngine；
5. 修复已登记的下单数量缺陷，使用独立 bugfix 提交；
6. 对比新旧路径的逐笔成交、现金、持仓和性能；
7. 更新 API inventory 和完成报告。

### 6.4 验收标准

- 只有 Ledger 能修改账户状态；
- `ExecutionEngine` 不依赖 `StrategyEngine` 或具体策略组件；
- 回测和实盘适配器返回相同结构的 `ExecutionReport`；
- `TradeManagerBase` 只承担兼容转发，不再增加职责；
- `Strategy::order/orderValue` 的数量和价格行为有完整测试；
- 三组金标的新旧路径结果一致；
- 全量测试和性能门槛通过。

完成记录写入：

```text
docs/arch/refactor/step-2-execution-engine-progress.md
```

## 7. 第三阶段：StrategyEngine

### 7.1 职责

`StrategyEngine` 是当前 `System` 的明确替代名称，负责：

- 持有不可随意变更的 `StrategyConfig`；
- 从 DataEngine 获取回测数据；
- 编排 EV、CN、SG、MM、ST、TP、PG、SP 等组件；
- 运行逐 Bar 状态机和延迟订单状态；
- 产生 `OrderRequest` 并交给 ExecutionEngine；
- 接收 `ExecutionReport`，形成回测结果。

它不直接访问 Driver、Broker 或写入账户。

建议的最小公共接口：

```cpp
class StrategyEngine {
public:
    StrategyEngine(const StrategyEngine&) = delete;
    StrategyEngine& operator=(const StrategyEngine&) = delete;

    [[nodiscard]] BacktestResult run(const StrategyConfig& config);
    void stop() noexcept;
    [[nodiscard]] bool running() const noexcept;
};
```

逐 Bar、延迟订单、Portfolio/AllocateFunds 协作方法属于内部实现，不进入普通公共接口。

### 7.2 文件范围

计划新增：

```text
hikyuu_cpp/hikyuu/trade_sys/engine/StrategyEngine.h
hikyuu_cpp/hikyuu/trade_sys/engine/StrategyEngine.cpp
hikyuu_cpp/hikyuu/trade_sys/engine/StrategyConfig.h
hikyuu_cpp/hikyuu/trade_sys/engine/BacktestResult.h
hikyuu_cpp/hikyuu/trade_sys/engine/internal/StrategyRuntime.h
hikyuu_cpp/hikyuu/trade_sys/engine/internal/StrategyRuntime.cpp
hikyuu_cpp/hikyuu/trade_sys/component/ComponentContext.h
hikyuu_cpp/unit_test/hikyuu/trade_sys/engine/test_StrategyEngine.cpp
hikyuu_pywrap/trade_sys/_StrategyEngine.cpp
```

计划修改：

```text
hikyuu_cpp/hikyuu/trade_sys/system/System.h
hikyuu_cpp/hikyuu/trade_sys/system/System.cpp
hikyuu_cpp/hikyuu/trade_sys/system/TradeRequest.h
hikyuu_cpp/hikyuu/trade_sys/system/TradeRequest.cpp
hikyuu_cpp/hikyuu/trade_sys/environment/EnvironmentBase.h
hikyuu_cpp/hikyuu/trade_sys/condition/ConditionBase.h
hikyuu_cpp/hikyuu/trade_sys/signal/SignalBase.h
hikyuu_cpp/hikyuu/trade_sys/moneymanager/MoneyManagerBase.h
hikyuu_cpp/hikyuu/trade_sys/stoploss/StoplossBase.h
hikyuu_cpp/hikyuu/trade_sys/profitgoal/ProfitGoalBase.h
hikyuu_cpp/hikyuu/trade_sys/slippage/SlippageBase.h
hikyuu_cpp/hikyuu/xmake.lua
hikyuu_cpp/unit_test/xmake.lua
hikyuu_pywrap/trade_sys/_System.cpp
hikyuu_pywrap/xmake.lua
hikyuu/test/test.py
```

本阶段不修改：

```text
hikyuu_cpp/hikyuu/data/**
hikyuu_cpp/hikyuu/data_driver/**
hikyuu_cpp/hikyuu/trade/internal/**
hikyuu_cpp/hikyuu/trade_sys/*/imp/**
hikyuu_cpp/hikyuu/indicator/**
hikyuu_cpp/hikyuu/serialization/**
```

### 7.3 实施顺序

1. 定义 `StrategyConfig`、`BacktestResult` 和只读 `ComponentContext`；
2. 把逐 Bar 和延迟订单状态机迁入内部 `StrategyRuntime`；
3. 使用 DataEngine 获取数据，使用 ExecutionEngine 提交订单；
4. 将现有 `System` 改为 StrategyEngine 的兼容门面；
5. 将 `_calculate/_reset/_clone` 等扩展钩子和普通用户接口分级，但暂不全面移动 Python 命名空间；
6. 对所有内置组件执行金标回归和性能比较；
7. 更新 API inventory 和完成报告。

### 7.4 验收标准

- 普通用户只需要配置并调用 `StrategyEngine::run()`；
- StrategyEngine 不直接访问数据库 Driver 或 Broker 实现；
- 只有 ExecutionEngine 能提交执行并修改账户；
- `runMoment*`、`pfProcessDelay*`、强制卖出和延迟状态接口不在普通公共门面；
- 旧 `System` 和 `SYS_*` 工厂在兼容期内继续工作；
- 所有现有内置策略组件的金标结果一致；
- 全量测试和性能门槛通过。

完成记录写入：

```text
docs/arch/refactor/step-3-strategy-engine-progress.md
```

## 8. 第四阶段：Python/pybind11 接口收口

### 8.1 职责

这一阶段只决定用户能看见什么，不改变三个 Engine 的 C++ 行为。

目标命名空间：

```text
hikyuu                 高频稳定类型、工厂和 open_session
├── data               DataEngine 及数据查询辅助
├── strategy           StrategyEngine、配置和回测结果
├── execution          ExecutionEngine、订单和账户快照
├── advanced           临时证券、低层控制和低频工具
└── spi                Driver、Broker 和策略组件扩展协议
```

### 8.2 文件范围

计划修改或新增：

```text
hikyuu/__init__.py
hikyuu/core.py
hikyuu/extend.py
hikyuu/session.py
hikyuu/data/__init__.py
hikyuu/strategy/__init__.py
hikyuu/execution/__init__.py
hikyuu/advanced/__init__.py
hikyuu/spi/__init__.py
hikyuu/indicator/__init__.py
hikyuu/trade_manage/__init__.py
hikyuu/trade_sys/__init__.py
hikyuu_pywrap/main.cpp
hikyuu_pywrap/application/**
hikyuu_pywrap/data/**
hikyuu_pywrap/trade/**
hikyuu_pywrap/trade_manage/**
hikyuu_pywrap/trade_sys/**
hikyuu/test/test_public_api.py
hikyuu/test/test.py
```

本阶段不修改：

```text
hikyuu_cpp/hikyuu/data/** 的业务实现
hikyuu_cpp/hikyuu/trade/** 的业务实现
hikyuu_cpp/hikyuu/trade_sys/engine/** 的业务实现
hikyuu_cpp/hikyuu/trade_sys/*/imp/**
hikyuu_cpp/hikyuu/indicator/**
```

### 8.3 实施顺序

1. 明确顶层稳定白名单并建立 `__all__`；
2. 显式导入替代层层 `import *`；
3. 将扩展协议移入 `hikyuu.spi`；
4. 将低层控制移入 `hikyuu.advanced`；
5. 旧路径保留兼容转发并发出 `DeprecationWarning`；
6. 修复 `open_spend_time` 的错误绑定；
7. 更新文档示例，执行 notebook/示例冒烟测试；
8. 更新 API inventory 和完成报告。

### 8.4 验收标准

- `dir(hikyuu)` 和 `hikyuu.__all__` 只包含明确批准的稳定接口；
- 普通用户类不出现 Driver、线程、IPC 和 `_calculate/_reset/_clone` 等内部入口；
- `DataEngine`、`ExecutionEngine`、`StrategyEngine` 从规定路径稳定导入；
- 所有迁移符号都有新路径、兼容转发和弃用警告；
- Python 3.10 测试、示例和 import 测试通过；
- 不因包装层增加大对象复制或阻塞 GIL 的回测开销。

完成记录写入：

```text
docs/arch/refactor/step-4-python-api-progress.md
```

## 9. 兼容与清理策略

每个旧入口只有在同时满足以下条件时才能删除：

1. 新 Engine 已提供功能等价入口；
2. 仓库内部调用已经迁移；
3. C++ 和 Python 旧入口已至少经过一个小版本弃用期；
4. API inventory 已记录替代入口和删除版本；
5. `rg` 确认没有非兼容层调用；
6. release notes 已说明迁移方式；
7. 全量测试、金标和性能门槛全部通过。

物理移动文件和删除兼容层不是独立业务阶段，应在所属 Engine 稳定后用单独 cleanup 提交执行。

## 10. 每阶段统一验证

功能验证：

```bash
./op.sh build
./op.sh small-test
./op.sh unit-test
./op.sh python-test
./op.sh import-test
```

额外验证：

```text
1. 重复生成 API inventory，确认差异符合预期；
2. 运行三组回测金标，比较逐笔交易、现金和最终持仓；
3. 使用相同 Release 构建和数据执行性能基准；
4. 记录执行前后接口数量、测试数量、耗时和峰值内存；
5. 检查本阶段是否修改了禁止区域；
6. 确认工作树只包含本阶段文件。
```

阶段完成报告必须包含：

| 内容 | 要求 |
| --- | --- |
| 执行前 | 当前接口数量、依赖、测试和性能基线 |
| 执行后 | 新接口、兼容层、文件变化和测试结果 |
| 差异 | 删除、迁移、新增及行为变化 |
| 验收 | 每条标准的证据和通过/不通过结论 |
| 性能 | 同数据同参数的重构前后结果 |
| 遗留项 | 明确说明未完成范围，不能用“全部完成”掩盖 |

## 11. 提交边界

推荐提交序列：

```text
docs: define three-engine refactoring boundaries

refactor: introduce data engine facade
refactor: move data runtime behind data engine
test: validate data engine compatibility and performance

refactor: introduce execution contracts
refactor: move account mutations behind execution engine
fix: normalize strategy order quantities
test: validate execution engine compatibility and performance

refactor: introduce strategy engine
refactor: move system runtime behind strategy engine
test: validate strategy engine compatibility and performance

refactor: separate Python public API from extension SPI
docs: publish engine API migration guide
```

每个提交只做一种变化，保持可审查、可二分、可回滚。

## 12. 完成定义

三引擎重构完成时必须同时满足：

- 普通用户主要通过 `HikyuuSession` 和三个 Engine 完成任务；
- `StockManager`、`TradeManagerBase`、`System` 不再承担新业务逻辑；
- 三个旧类只剩兼容转发，并有明确弃用计划；
- DataEngine、ExecutionEngine、StrategyEngine 的依赖方向无循环；
- Internal API 不出现在普通 Python 自动补全和稳定 C++ 聚合头中；
- 新增功能不再要求给三个旧大类添加方法；
- 所有行为金标保持一致，已批准的 bugfix 除外；
- 全量测试通过，性能回退在验收门槛内；
- 每个阶段都有执行前后对比和验收报告。
