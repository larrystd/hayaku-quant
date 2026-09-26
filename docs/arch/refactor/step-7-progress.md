# 第 7 步：C++ 质量基线与无效文件清理

> 状态：规划中；本文件只冻结目标、边界和验收标准，尚未执行全量格式化、静态检查修复、ASan 验收或文件删除
>
> 前置条件：第 6 步的 Python 绑定层与包结构收敛完成，构建、公共 API 和测试基线通过
>
> 决策日期：2026-09-26
>
> 后续顺序：第 8 步接口收敛；品牌已在第 6 步前置批次统一为 `hayaku`

## 1. 目标与边界

本步在结构迁移之后建立可重复的代码质量基线，不重新设计业务对象：

1. 用同一版本的 `clang-format` 将手写 C/C++ 代码一次性对齐 Google 格式，并把检查和显式格式化入口接入 `op.sh`。
2. 用 `clang-tidy` 检查项目代码，记录并处理真实缺陷，形成可持续执行的规则和告警基线。
3. 用 AddressSanitizer（ASan）检查核心库、测试及可选原生模块；在平台支持时补充泄漏检查。
4. 按当前产品目标审计内容价值，清理过时、重复或无意义的文件、测试数据和文档；同步修复其消费者。

本步允许因格式化暴露的缺失直接 `#include`、静态检查确认的缺陷和 ASan 确认的内存问题，
分别以可审查的小批次修复；不得把行为修复混入机械格式化提交。

本步**不做** GoogleTest 迁移、业务算法重写、公共类型/方法批量改名、三个原生模块边界调整，
也不再调整已经完成的 `hayaku` 品牌边界。`clang-format` 只改变排版，不负责命名规范；
可能影响 C++ ABI、Python 导出或序列化标识的命名问题，登记到第 8 步逐项决策。

## 2. `clang-format`：一次性对齐 Google

### 2.1 规则和文件范围

- `.clang-format` 收敛为 `BasedOnStyle: Google` 的简明配置，移除当前 4 空格、100 列等项目特例；固定并记录执行所用的 LLVM 主版本，避免本机和 CI 输出漂移。
- 范围是仓库内手写的 C/C++ 源码、头文件和测试：`hayaku_cpp/src`、`hayaku_cpp/test`、`hayaku_cpp/demo`、`hayaku_pywrap`、`hayaku_ingest_native`、`hayaku_realtime_native`。执行前重新生成精确文件清单。
- 不格式化 `build/`、`.xmake/`、第三方包、FlatBuffers 等生成代码和二进制产物。`*.h.in` 等模板先验证是否能被正确解析，再决定是否纳入。
- 先只读预览文件数、差异规模和包含顺序变化，再执行一次性格式化。旧代码不因当前工作树脏而被跳过，但结构迁移与机械排版须分开审查。

Google 默认包含头文件排序。若排序后编译失败，应补足文件自身所需的直接 `#include` 并单独记录，
不能依赖原有偶然的传递包含，也不能把编译失败解释为纯格式差异。

### 2.2 命令入口

在 `op.sh` 中增加并写入 `help`、`doctor`：

| 计划命令 | 行为 |
|---|---|
| `fmt-check [files...]` | 只读检查指定文件；无参数时按明确清单检查全部手写 C/C++ 文件 |
| `fmt <files...>` | 只格式化显式指定的文件 |
| `fmt-all` | 明确触发一次性全范围改写，不作为 `build` 或 `test` 的副作用 |
| `tidy <files...>` | 使用 Xmake 编译数据库检查指定编译单元，不自动应用修复 |
| `tidy-strict <files...>` | 对已纳入质量门禁的文件把报告的告警视为失败 |
| `asan-test` | 在隔离的 sanitizer 配置下构建并运行指定测试矩阵 |

这些命令应能发现固定版本的 LLVM 工具；`fmt-check`、`fmt` 和 `help` 不应被无关的
Python 扩展或 Xmake 构建前置检查阻断。同步校验 `tools/cpp-style.md` 中的
`hayaku_cpp` 示例。

## 3. `clang-tidy`：检查而非批量改写

1. 以 Xmake 生成的 `compile_commands.json` 为唯一编译参数来源；更换构建选项、目录或 target 后重新生成，并核对 `core`、`ingest`、`realtime` 的编译单元覆盖。
2. 从 `clang-analyzer`、`bugprone`、`performance` 和适用的 `google-*` 检查建立规则集。`abseil-*` 主要检查 Abseil API 用法，不因采用 Google 格式而全量启用。
3. 只报告项目手写代码及其自有头文件的诊断，排除系统、第三方和生成文件；记录检查耗时、规则版本、告警位置和处理结论。
4. `clang-tidy --fix` 不作为全仓自动步骤。尤其是公开方法、成员和 Python 可见名称的命名规则，只先形成清单；涉及兼容性时交由第 8 步处理。
5. 对确认有效的问题修复并补测试；误报使用最小范围的说明或抑制。已有告警未清零前，不把全仓 `WarningsAsErrors: '*'` 直接设为 CI 门禁；先约束新增/修改代码，再逐批消化基线。

## 4. ASan 与泄漏检查

仓库已有 `leak_check` Xmake 选项、core/test 的 sanitizer 策略和 `asan.sup`，但这不等于
已经完成独立可重复的 ASan 验收。本步需要：

1. 明确 macOS 与 Linux 可用的编译器、运行时和启动方式；使用独立的 debug/sanitizer 构建配置，避免把 sanitizer 产物混进普通 wheel 或发布库。
2. 核对 `hayaku` 核心库、`small-test`、`unit-test`、插件 ABI 测试以及可选的 ingest/realtime target 是否都使用一致的 sanitizer 编译和链接参数。
3. 优先运行不依赖真实行情和网络服务的测试；环境不允许 IPC、数据库或监听端口时，单独记录受限用例，不将其误判为内存问题。
4. 在支持的环境中运行泄漏检查；逐条复核 `asan.sup` 的 OpenSSL 抑制项，不用宽泛抑制掩盖项目自身泄漏。
5. 保存可复现的触发命令、栈和修复测试；修复完成后恢复普通构建配置，再执行常规回归。

ASan、LeakSanitizer 和平台运行时能力分别记录，不以某个平台无法执行泄漏检查推断全平台通过。

## 5. 无效文件与配置清理

**先判断内容是否还有意义，再检查引用。**被代码、CI 或文档引用，只说明删除时需要修改消费者，
不构成保留理由；没有引用，也不自动等于无效。判断顺序是：

1. 是否服务于当前的 `core` 研究/回测、可选 `ingest` 数据准备、可选 `realtime` 行情能力，或必要的构建、测试、发布？
2. 内容是否与当前 API 和产品承诺一致，能否覆盖独有的错误场景；还是旧版本、旧品牌、已移出仓库的商业能力或重复样本？
3. 若删掉或缩小它，用户说明、回归覆盖和可复现性是否仍完整？若答案为否，应重写、迁出或补最小替代物，而不是机械保留旧内容。
4. 决定删除后再定位代码、测试、打包、CI、README、Sphinx 导航等消费者，同批清除失效引用并验证。

以下是本步的**内容清理目标**，不因文件当前被引用而豁免；“保留”也以内容更新和最小化为前提。

| 首批候选 | 内容判断 | 本步处理 |
|---|---|---|
| 根目录 `doxyfile` | `INPUT` 为空、递归扫当前目录且还生成 LaTeX，未定义现在需要交付的 C++ API 文档范围 | 默认删除这份泛化配置；若 C++ API 参考文档仍是产品交付物，则另建只覆盖公开头文件的有效配置，不保留旧文件充数 |
| `hayaku_cpp/test/Doxyfile` 及开发文档引用 | 配置的项目名是 `test_doc`、输入是测试目录；中英文 `developer.rst` 还指向不存在的 `hayaku_cpp/Doxygen`，均不能说明当前 C++ API | 本步删除测试源码文档配置，并删除或重写失效命令；若将来确需 C++ API 参考文档，重新定义范围和配置 |

本步只处理下文列出的文件；其他根目录配置（如 `cppcheck.cppcheck`）不因名称可疑而顺带删除。

### 5.1 `i18n/`：保留本地化能力，清掉失效词条

`zh_CN.po` 是中文译文源，`zh_CN/hayaku.mo` 是运行时/安装包使用的编译产物；业绩报告等中文输出仍有
实际用户价值，因此不按“资源目录”整目录删除。但现有 `hayaku.pot` 和 `.po` 头部还指向不存在的
`hayaku_cpp/hayaku`，模板还列着已删除的旧 BackTest 文件，不能把这些旧词条当作现行翻译资产。

| 处理 | 边界 |
|---|---|
| 保留并校正 | 以当前源码的可翻译字符串为准，清理 `.po` 中不再出现的词条，补齐仍有价值的中文翻译；重新生成并验证 `.mo`，确保 core 的中文报告和可选模块有正确回退 |
| 删除 | 现有 `hayaku.pot` 是旧路径和旧 BackTest 文案的重复模板，且没有可重复的抽取流程；本步删除。以后需要模板时从当前源码重新生成，不沿用这份文件 |
| 保留并修正 | `update_translation.sh` 仍提供从 `.po` 编译 `.mo` 的明确入口，本步保留并检查输出路径。不得先删 `.mo` 再让现有中文运行路径失效 |

### 5.2 `test_data/`：保留测试场景，缩小而非继承历史数据包

目录约 55 MB，主要是分钟线 HDF5、`stock.db`、CSV 和通达信样本。当前测试启动会复制整个目录，
但“测试会复制”不证明每个文件、每条历史记录都必要。目标是**最小、确定、可复现的夹具集合**：

| 处理 | 边界 |
|---|---|
| 直接清理 | `test_data/tmp/*.plk` 等运行生成物不作为源数据；清理现有文件并保证测试在空 `tmp/` 下可重建 |
| 计划删除 | `trader.db`、`downloads/finance/gpcw20110930.dat`、未配置且未表达独立断言的五份旧 `block` 文件；具体路径见 5.4。执行前做一次场景核对，若有独有覆盖，改为最小夹具而不是保留整个旧文件 |
| 压缩/重造候选 | `sh_1min.h5`、`sz_1min.h5`、两份 `5min` HDF5、`test_min_data.csv` 占据主要体积；为日线、分钟、分笔/分时、两交易所及导入边界各保留足够触发断言的最小数据，不因旧测试断言依赖大样本就原样保留 |
| 必须保住的意义 | `stock.db`、必要的 HDF5/CSV、`block` 和 `vipdoc` 通达信样本应按“它验证什么”留存；若可用更小的生成夹具替代，就重写测试并删除冗余二进制。Linux/Windows 配置若可统一也应收敛 |

每个保留夹具需登记“测试场景 → 必需字段/记录 → 文件”，而不是仅登记“哪个测试打开了文件”。
缩减前后比较断言覆盖，并运行 C++ 单测、小测试、Python 测试及相关导入测试；不引入需要公网下载
才能运行默认单测的替代方案。

### 5.3 `docs/en/`、`docs/zh/`：删无效内容，不因 Sphinx 正在构建而保留旧手册

两棵目录是中英文用户手册源码，仍被 CI 构建；这只决定清理时必须同步改构建/导航，**不代表
当前每一页都有价值**。保留一套与现行 `core`、`ingest`、`realtime` 对应的精简双语入口，
`docs/arch/refactor/` 的架构决策不在此次清理范围内。

| 处理 | 内容边界 |
|---|---|
| 保留但重写 | `overview`、`install`、`quickstart`、`python_api` 和仍有效的研究/回测 API 说明；安装、初始化和示例必须以当前公开包及显式 Session 为准。旧 GUI 导入流程应改为可选 `ingest` 场景，不再充当 core 入门主线 |
| 从主手册移出 | `vip/**` 的捐赠、授权及商业功能细节，以及 `shm_server.rst` 中仅属于外部 VIP 插件的操作手册；仍维护的扩展文档迁至对应扩展仓库/独立站点，主手册只保留必要的可选能力边界和链接 |
| 删除或重写 | `stock_manager.rst` 中与已移除的全局 `sm`/旧 `StockManager` Python API 冲突的段落；`developer.rst` 的失效 Doxygen/旧构建命令；`install.rst` 中带旧时间点的版本建议；`overview.rst` 的旧项目地址。无法验证的用法不继续发布 |
| 清理历史与素材 | 两份 `release_history.rst` 从当前手册删除，历史内容仍可在 Git 历史中查阅；英文 `release.md` 不再作为空壳发布，补齐与中文页对应的有效内容。旧 GUI/安装/捐赠截图按 5.4 的精确清单删除；其余 logo 和插图不顺带清理 |

对其余 `hub`、`interactive`、`indicator`、`factor`、`strategy`、`reference` 等页面逐页核对现行 API，
可用的内容保留并修正，不可用且无迁移价值的内容删掉；成对处理中文、英文导航和页面。
如果产品决定**不再发布用户手册**，才整体退役 `docs/en`、`docs/zh`，且必须同时给 README、安装包、
CI、Read the Docs 配置和贡献规则提供替代说明；本步默认不是整目录删除。

实际删除时列出精确路径，记录“内容失效原因 → 替代/迁出位置 → 受影响消费者 → 回归结果”。
同批修复 README、CI、打包规则及中英文导航。被删除的跟踪文件仍可从 Git 历史恢复。

### 5.4 执行文件清单

以下是本步拟处理的**现有精确路径**，不是 `find`/glob 的批量删除规则。执行前如文件已变化，
先复核内容；不得自动扩大到同目录的其他文件。本节是计划，尚未删除这些文件。

**删除配置、旧翻译模板和无独立场景的测试数据：**

| 文件 | 清理理由 |
|---|---|
| `doxyfile` | 泛化扫描仓库，未定义有效的 C++ API 文档交付物 |
| `hayaku_cpp/test/Doxyfile` | 只生成测试源码文档，不是当前用户或开发者需要的 API 参考 |
| `i18n/hayaku.pot` | 旧源码路径与旧 BackTest 词条；当前 `.po` 已是译文源 |
| `test_data/tmp/Datetime.plk` | 测试运行生成物，不是源夹具 |
| `test_data/tmp/KData.plk` | 同上 |
| `test_data/tmp/Stock.plk` | 同上 |
| `test_data/trader.db` | 旧 `td_*` 交易库样本，没有当前执行/回测测试场景 |
| `test_data/downloads/finance/gpcw20110930.dat` | 2011 年财务下载样本，没有对应的当前导入断言 |
| `test_data/block/Zhbblk.ini` | 不在当前测试配置的板块集合中 |
| `test_data/block/ggbk.ini` | 同上 |
| `test_data/block/hkzs.ini` | 同上 |
| `test_data/block/rdbk.ini` | 同上 |
| `test_data/block/sysblk.ini` | 同上 |

`test_data/tmp/` 目录本身保留为可重建的运行目录；其三个 `.plk` 当前未被 Git 跟踪。
若执行前发现上述数据文件有独有断言，先补一个更小且含义清楚的替代夹具，再删除旧文件。

**从当前双语手册移出：**下表每格都是完整路径；VIP 技术内容仍有价值的部分先迁到相应扩展的文档，
主手册只留可选能力和安装边界。不存在维护中的扩展时，删除过时内容而不保留营销空壳。

| 英文文件 | 中文文件 | 处理 |
|---|---|---|
| `docs/en/vip/index.rst` | `docs/zh/vip/index.rst` | 从主导航删除 |
| `docs/en/vip/vip-plan.md` | `docs/zh/vip/vip-plan.md` | 商业捐赠/授权细节迁出本仓 |
| `docs/en/vip/indicator.rst` | `docs/zh/vip/indicator.rst` | VIP 指标说明迁出本仓 |
| `docs/en/vip/dataserver.rst` | `docs/zh/vip/dataserver.rst` | VIP 数据服务说明迁出本仓 |
| `docs/en/vip/dynamick.rst` | `docs/zh/vip/dynamick.rst` | VIP 动态 K 线说明迁出本仓 |
| `docs/en/shm_server.rst` | `docs/zh/shm_server.rst` | 只属于外部 `shmserver` 插件的细节迁出；可选 `realtime` 的通用说明另写 |
| `docs/en/release_history.rst` | `docs/zh/release_history.rst` | 从当前手册删除旧版本历史；历史内容保留在 Git |

**删除随上述过时流程一起退出的图片：**两列均为完整路径；更新页面与导航后再删除图片，
不按“是否被引用”直接扫空 `_static/`。

| 英文图片 | 中文图片 | 对应失效内容 |
|---|---|
| `docs/en/_static/20000-install.png` | `docs/zh/_static/20000-install.png` | 旧安装图，改用当前安装步骤 |
| `docs/en/_static/20004-install-importdata-directx.png` | `docs/zh/_static/20004-install-importdata-directx.png` | 旧 GUI 导入/DirectX 流程 |
| `docs/en/_static/20004-install-importdata-err.png` | `docs/zh/_static/20004-install-importdata-err.png` | 旧 GUI 导入错误图 |
| `docs/en/_static/20005-install-config3.png` | `docs/zh/_static/20005-install-config3.png` | 旧配置界面 |
| `docs/en/_static/20006-install-config4.png` | `docs/zh/_static/20006-install-config4.png` | 旧配置界面 |
| `docs/en/_static/install-20190228.png` | `docs/zh/_static/install-20190228.png` | 2019 年安装/导入截图 |
| `docs/en/_static/install-2019022802.png` | `docs/zh/_static/install-2019022802.png` | 同上 |
| `docs/en/_static/quickstart_download.png` | `docs/zh/_static/quickstart_download.png` | 旧 GUI 下载截图 |
| `docs/en/_static/quickstart_examples.png` | `docs/zh/_static/quickstart_examples.png` | 旧入门示例截图 |
| `docs/en/_static/10003-phone.jpg` | `docs/zh/_static/10003-phone.jpg` | 旧远程 Jupyter 手机操作教程 |
| `docs/en/_static/dataserver_01.png` | `docs/zh/_static/dataserver_01.png` | VIP 数据服务页面插图 |
| `docs/en/_static/dataserver_02.png` | `docs/zh/_static/dataserver_02.png` | 同上 |
| `docs/en/_static/qun.png` | `docs/zh/_static/qun.png` | 旧社群宣传图，不属于产品使用说明 |

**修改或缩减，但不直接删除：**

| 精确路径 | 目标 |
|---|---|
| `i18n/zh_CN.po`、`i18n/zh_CN/hayaku.mo`、`i18n/update_translation.sh` | 用当前源码清理译文源、重编译运行资源并校验脚本 |
| `test_data/sh_1min.h5`、`test_data/sz_1min.h5`、`test_data/sh_5min.h5`、`test_data/sz_5min.h5`、`test_data/test_min_data.csv` | 缩成足以覆盖两市场、各周期和导入边界的最小样本，保留断言语义 |
| `test_data/stock.db`、`test_data/hayaku_linux.ini`、`test_data/hayaku_win.ini` | 仅保留测试所需库表/配置；先验证再缩减，不能因体积直接删库 |
| `docs/en/index.rst`、`docs/zh/index.rst` | 移除 VIP、Shm 和旧发布历史导航，维持中英文结构一致 |
| `docs/en/overview.rst`、`docs/zh/overview.rst`、`docs/en/install.rst`、`docs/zh/install.rst` | 更新旧项目地址、安装版本说明和产品边界 |
| `docs/en/quickstart.rst`、`docs/zh/quickstart.rst` | 以当前 core + 可选 ingest 的实际流程重写，不再以旧 GUI/Jupyter 教程为主线 |
| `docs/en/stock_manager.rst`、`docs/zh/stock_manager.rst` | 删除已退出的全局 `sm`/旧 `StockManager` Python 用法，保留现行 Session 数据访问 |
| `docs/en/developer.rst`、`docs/zh/developer.rst` | 删除失效 Doxygen 和旧构建命令，改为可运行的当前开发流程 |
| `docs/en/release.md`、`docs/zh/release.md` | 英文页补真实内容，中文页更新当前版本；不再发布占位页 |

`test_data` 其余日线、分时、分笔、必要板块和 `vipdoc` 样本暂保留；`docs/en`、`docs/zh`
其余 API 页和品牌图暂保留。它们不因“被引用”永久豁免，只有在下一轮逐页/逐夹具审计给出
**新增的精确路径和内容理由**后，才进入删除清单。

## 6. 执行顺序与验收

1. 冻结第 6 步后的工作树基线，记录文件清单、LLVM 版本、现有格式差异、测试结果和待删候选。
2. 完成 `.clang-format`、`.clang-tidy`、`op.sh` 与工具说明；先验证命令只读模式和单文件模式。
3. 全量格式化作为独立机械批次；检查异常 diff、包含顺序和 `git diff --check`，再做构建与回归。
4. 执行 tidy 审计，按真实缺陷、风格债务、误报分类；修复与格式化分开提交。
5. 执行 ASan/泄漏测试矩阵，修复可复现问题，并恢复普通配置验证。
6. 按内容价值清理 Doxygen、`i18n` 旧词条、`test_data` 冗余夹具及双语手册；同批消除对应引用，验证翻译、测试、文档和打包内容。

完成条件：

- [ ] 固定 LLVM 版本下，手写 C/C++ 文件的全量 `fmt-check` 通过；生成和第三方文件未被改写。
- [ ] 默认构建、`small-test`、`unit-test`、相关可选模块与完整 Python 测试无新增失败，公共 API 清单和回测结果无非预期变化。
- [ ] tidy 规则、现存告警和新增代码门禁均有可复现记录；未用批量改名掩盖 API 变化。
- [ ] ASan 覆盖的 target、平台、测试和抑制项有清单；项目代码无未处理的可复现内存错误。
- [ ] 每个删除、缩减或迁移的对象都有内容价值结论、替代方式和消费者清单；Doxygen 文档入口不再指向不存在的配置。
- [ ] 中文 `.mo` 与当前译文源一致；测试夹具能解释对应断言且默认测试不依赖公网；中英文用户手册不再把旧 GUI、旧 Python API 或外部 VIP 细节作为核心功能发布。
- [ ] `git diff --check`、架构边界检查和适用的中英文文档检查通过；普通非 sanitizer 构建已恢复。

本步的工具配置、机械格式化、tidy/ASan 缺陷修复和文件删除分别作为可审查的变更批次，
不把所有差异混为一个难以定位的提交。
