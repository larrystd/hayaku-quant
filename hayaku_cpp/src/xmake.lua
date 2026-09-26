
local function generate_spot_schema(target)
    local schema = path.join(os.projectdir(), "hayaku_cpp/src/extensions/realtime/spot.fbs")
    local outputdir = path.join(os.projectdir(), get_config("builddir"), "autogen",
                                "extensions/realtime")
    local outputfile = path.join(outputdir, "spot_generated.h")
    if not os.isfile(outputfile) or os.mtime(outputfile) < os.mtime(schema) then
        local flatbuffers = assert(target:pkg("flatbuffers"), "flatbuffers package is required")
        local flatc = path.join(flatbuffers:installdir(), "bin",
                                is_host("windows") and "flatc.exe" or "flatc")
        os.mkdir(outputdir)
        cprint("${color.build.object}generating.flatbuffers %s", schema)
        os.vrunv(flatc, {"--cpp", "-o", outputdir, schema})
    end
end

local function enable_project_asan()
    if get_config("leak_check") and is_plat("macosx", "linux") then
        set_symbols("debug")
        set_policy("build.sanitizer.address", true)
        if is_plat("linux") then
            set_policy("build.sanitizer.leak", true)
        end
    end
end

target("hayaku")
    -- set_kind("$(kind)")
    set_kind("shared")

    if is_mode("coverage") then
        add_cxflags("-fprofile-update=atomic")
    end

    enable_project_asan()

    if has_config("http_client_ssl") or has_config("mysql") then
        add_packages("openssl3")
    end

    add_packages("boost", "fmt", "spdlog", "tl_expected", "nlohmann_json", "xxhash", "eigen", "utf8proc")
    add_packages("flatbuffers", "nng")
    if is_plat("windows", "linux", "cross", "macosx") then
        if get_config("sqlite") or get_config("hdf5") then
            add_packages("sqlite3")
        end
    end

    if has_config("mysql") and not has_config("disable_libmysqlclient") then
        add_packages("mysql")
    end

    if (not has_config("leak_check")) and is_plat("windows", "linux", "cross") then
        add_packages("mimalloc")
    end

    if has_config("omp") then
        add_packages("openmp")
        if is_plat("macosx") then
            add_packages("libomp")
        end
    end

    if has_config("http_client_zip") then
        add_packages("gzip-hpp")
    end

    if has_config("ta_lib") then
        add_packages("ta-lib")
    end

    add_options("mysql")
    add_includedirs(".")

    -- set version for release
    set_configdir("./")
    add_configfiles("$(projectdir)/config.h.in")
    add_configfiles("$(projectdir)/version.h.in")
    add_configfiles("$(projectdir)/config_utils.h.in", {prefixdir="common", filename="Config.h"})

    if is_plat("windows") then
        add_cxflags("/bigobj")
        add_cxflags("-wd4819")
        add_cxflags("-wd4251") -- template dll export warning
        add_cxflags("-wd4267")
        add_cxflags("-wd4834") -- C++17 discarding return value of function with 'nodiscard' attribute
        add_cxflags("-wd4244") -- discable double to int
        add_links("bcrypt")
    else
        add_rpathdirs("$ORIGIN")
        add_cxflags("-Wno-sign-compare", "-Wno-missing-braces")
    end

    if get_config("hdf5") then
        add_packages("hdf5")
    end

    if is_plat("windows") then
        if is_kind("shared") then
            add_defines("HAYAKU_API=__declspec(dllexport)")
            add_defines("HAYAKU_UTILS_API=__declspec(dllexport)")
        end
    end

    if is_plat("linux", "cross") then
        add_cxflags("-fPIC")
    end

    if is_plat("macosx") then
        -- macosx下boost序列化需要
        if is_kind("shared") then
            add_defines("HAYAKU_API=__attribute__((visibility(\"default\")))")
            add_defines("HAYAKU_UTILS_API=__attribute__((visibility(\"default\")))")
        end
        add_cxflags("-frtti")
        add_shflags("-Wl,-flat_namespace,-undefined,dynamic_lookup")
        add_links("sqlite3")
        add_frameworks("CoreFoundation")
    end

    add_headerfiles("./**.h|extensions/ingest/KDataTo*Importer.h|extensions/ingest/CheckData.h|extensions/ingest/IngestExport.h|extensions/realtime/DataServerPlugin.h|extensions/realtime/ShmServerPlugin.h|extensions/realtime/GlobalSpotAgent.h|extensions/realtime/SpotAgent.h|extensions/realtime/RealtimeExport.h")

    -- set_policy("build.optimization.lto", true)
    add_rules("c++.unity_build", {batchsize = 0})
    add_files("./data/*.cpp", "./common/serialization/*.cpp", {unity_group="base"})
    add_files("./operators/*.cpp", {unity_group="operators"})

    add_files("./metrics/**.cpp", {unity_group="metrics"})
    add_files("./application/*.cpp", "./extensions/realtime/RealtimePort.cpp",
              "./extensions/realtime/ScheduledTasks.cpp", "./extensions/realtime/Scheduler.cpp",
              {unity_group="app"})
    add_files("./application/plugins/DevicePlugin.cpp",
              "./application/plugins/ExtendIndicatorsPlugin.cpp",
              "./application/plugins/FactorPlugin.cpp",
              "./application/plugins/ExtraPlugins.cpp", {unity_group="plugin"})
    add_files("./execution/**.cpp", {unity_group="execution"})
    add_files("./strategy/*.cpp", {unity_group="strategy"})
    add_files("./strategy/decision/*.cpp", {unity_group="decision"})
    add_files("./strategy/risk/*.cpp", {unity_group="risk"})
    add_files("./strategy/selection/*.cpp|StyleRegression.cpp", {unity_group="selection"})
    -- StyleRegression.cpp 单独编译：包含 Eigen 头，避免 unity build 拼接后
    -- Eigen 内联实现泄漏给同组其他源码（编译变慢 + 潜在 ODR 风险）
    add_files("./strategy/selection/StyleRegression.cpp", {unity_group = false})
    add_files("./strategy/portfolio/*.cpp", {unity_group="portfolio"})

    add_files("./common/*.cpp", "./common/time/*.cpp", "./common/concurrency/*.cpp",
              "./extensions/telemetry/*.cpp", {unity_group="common"})
    add_files("./common/database/DBCondition.cpp", "./common/database/DBUpgrade.cpp",
              {unity_group="common"})

    add_files("./data/storage/BaseInfoDriver.cpp", "./data/storage/BlockInfoDriver.cpp",
              "./data/storage/DataDriverFactory.cpp", "./data/storage/HistoryFinanceReader.cpp",
              "./data/storage/KDataDriver.cpp", {unity_group="data_driver"})
    add_files("./extensions/realtime/KDataShmBufferImp.cpp",
              "./extensions/realtime/ShmClientHook.cpp",
              "./extensions/realtime/ShmMirrorSink.cpp", {unity_group="data_driver"})
    add_files("./extensions/ingest/QLBlockInfoDriver.cpp",
              "./extensions/ingest/KDataTempCsvDriver.cpp", {unity_group="data_driver"})
    if get_config("hdf5") or get_config("sqlite") then
        add_files("./common/database/AsyncSQLiteConnect.cpp",
                  "./common/database/AsyncSQLiteStatement.cpp",
                  "./common/database/SQLiteConnect.cpp",
                  "./common/database/SQLiteStatement.cpp",
                  "./common/database/SQLiteUtil.cpp", {unity_group="sqlite"})
        add_files("./data/storage/SQLiteBaseInfoDriver.cpp",
                  "./data/storage/SQLiteBlockInfoDriver.cpp", {unity_group="sqlite"})
    end
    if get_config("mysql") then
        add_files("./extensions/mysql/MySQLBaseInfoDriver.cpp",
                  "./extensions/mysql/MySQLBlockInfoDriver.cpp", {unity_group="mysql"})
    end
    if get_config("sqlite") or get_config("hdf5") then
        add_files("./data/storage/SQLiteKDataDriver.cpp", {unity_group="sqlite"})
    end
    if get_config("hdf5") then
        add_files("./extensions/hdf5/H5KDataDriver.cpp", {unity_group="hdf5"})
    end
    if get_config("mysql") then
        add_files("./extensions/mysql/MySQLKDataDriver.cpp", {unity_group="mysql"})
    end
    if get_config("tdx") then
        add_files("./extensions/ingest/TdxKDataDriver.cpp", {unity_group="tdx"})
    end
    if get_config("mysql") then
        add_files("./extensions/mysql/MySQLSupport.cpp")
    end
    if has_config("ta_lib") then
        add_files("./extensions/talib/*.cpp", {unity_group="talib"})
    end

    before_build(function(target)
        import("lib.detect.find_library")
        if is_plat("linux") then
            -- boost.mysql 依赖的 charconv 会自动检测包含__float128
            local quadmath = find_library("quadmath*",{
                "/usr/lib",
                "/usr/lib64",
                "/usr/local/lib",
                "/usr/lib/x86_64-linux-gnu",
                "/usr/lib/aarch64-linux-gnu",
                "/usr/lib/arm-linux-gnueabihf",
                "/usr/lib/gcc/x86_64-linux-gnu/**",
                "/usr/lib/gcc/aarch64-linux-gnu/**",
                "/usr/lib/gcc/arm-linux-gnueabihf/**"
            })
            -- print(quadmath)
            if quadmath ~= nil then
                target:add("syslinks", "quadmath")
            end
        end
    end)

    after_build(function(target)
        local destpath = get_config("builddir") .. "/" .. get_config("mode") .. "/" .. get_config("plat") .. "/" .. get_config("arch")
        print(destpath)
        import("core.project.task")
        task.run("copy_dependents", {}, target, destpath, true)
    end)

    after_install(function(target)
        local dst_path = target:installdir() .. "/include/hayaku/python/"
        os.cp("$(projectdir)/hayaku_pywrap/common/pybind_utils.h", dst_path)
        os.cp("$(projectdir)/hayaku_pywrap/common/pickle_support.h", dst_path)

        local destpath = target:installdir()
        import("core.project.task")
        task.run("copy_dependents", {}, target, destpath, true)
    end)
target_end()

-- Optional live market-data transport and server facades. The core only owns RealtimePort.
target("hayaku-realtime")
    enable_project_asan()
    set_kind("shared")
    set_default(false)
    add_deps("hayaku")
    add_includedirs(".", "$(builddir)/autogen", {public = true})
    add_packages("boost", "fmt", "spdlog", "tl_expected", "nlohmann_json", "xxhash",
                 "eigen", "utf8proc", "flatbuffers", "nng")
    add_extrafiles("./extensions/realtime/spot.fbs")
    add_defines("HAYAKU_REALTIME_BUILD")
    add_files("./extensions/realtime/SpotAgent.cpp", "./extensions/realtime/GlobalSpotAgent.cpp",
              "./extensions/realtime/DataServerPlugin.cpp",
              "./extensions/realtime/ShmServerPlugin.cpp")
    add_headerfiles("./extensions/realtime/RealtimeExport.h",
                    "./extensions/realtime/GlobalSpotAgent.h",
                    "./extensions/realtime/SpotAgent.h",
                    "./extensions/realtime/DataServerPlugin.h",
                    "./extensions/realtime/ShmServerPlugin.h")
    before_build(generate_spot_schema)
    if is_plat("windows") then
        add_defines("HAYAKU_API=__declspec(dllimport)", "HAYAKU_UTILS_API=__declspec(dllimport)")
        add_cxflags("/bigobj")
    else
        add_rpathdirs("$ORIGIN", "$ORIGIN/../hayaku/cpp")
        add_cxflags("-Wno-sign-compare", "-Wno-missing-braces")
    end
    if is_plat("linux", "cross") then
        add_cxflags("-fPIC")
    end
    if is_plat("macosx") then
        add_shflags("-headerpad_max_install_names")
    end
target_end()

target("hayaku-ingest")
    enable_project_asan()
    set_kind("shared")
    set_default(false)
    add_deps("hayaku")
    add_includedirs(".")
    add_packages("boost", "fmt", "spdlog", "tl_expected", "nlohmann_json", "xxhash", "eigen",
                 "utf8proc", "flatbuffers", "nng")
    add_defines("HAYAKU_INGEST_BUILD")
    add_files("./extensions/ingest/KDataToHdf5Importer.cpp",
              "./extensions/ingest/KDataToMySQLImporter.cpp",
              "./extensions/ingest/KDataToClickHouseImporter.cpp",
              "./extensions/ingest/CheckData.cpp")
    add_headerfiles("./extensions/ingest/IngestExport.h",
                    "./extensions/ingest/KDataToHdf5Importer.h",
                    "./extensions/ingest/KDataToMySQLImporter.h",
                    "./extensions/ingest/KDataToClickHouseImporter.h",
                    "./extensions/ingest/CheckData.h")
    if is_plat("windows") then
        add_defines("HAYAKU_API=__declspec(dllimport)")
        add_defines("HAYAKU_UTILS_API=__declspec(dllimport)")
        add_cxflags("/bigobj")
    else
        add_rpathdirs("$ORIGIN", "$ORIGIN/../hayaku/cpp")
        add_cxflags("-Wno-sign-compare", "-Wno-missing-braces")
    end
    if is_plat("linux", "cross") then
        add_cxflags("-fPIC")
    end
    if is_plat("macosx") then
        add_shflags("-headerpad_max_install_names")
    end
target_end()
