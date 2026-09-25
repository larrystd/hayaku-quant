
target("hikyuu")
    -- set_kind("$(kind)")
    set_kind("shared")

    if is_mode("coverage") then 
        add_cxflags("-fprofile-update=atomic")
    end

    if get_config("leak_check") then
        if is_plat("macosx") then
            set_policy("build.sanitizer.address", true)
        elseif is_plat("linux") then
            -- 需要 export LD_PRELOAD=libasan.so
            set_policy("build.sanitizer.address", true)
            set_policy("build.sanitizer.leak", true)
            -- set_policy("build.sanitizer.memory", true)
            -- set_policy("build.sanitizer.thread", true)
        end
    end

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
    add_configfiles("$(projectdir)/config_utils.h.in", {prefixdir="common", filename="config.h"})

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
            add_defines("HKU_API=__declspec(dllexport)")
            add_defines("HKU_UTILS_API=__declspec(dllexport)")
        end
    end

    if is_plat("linux", "cross") then
        add_cxflags("-fPIC")
    end

    if is_plat("macosx") then
        -- macosx下boost序列化需要
        if is_kind("shared") then 
            add_defines("HKU_API=__attribute__((visibility(\"default\")))")
            add_defines("HKU_UTILS_API=__attribute__((visibility(\"default\")))")
        end
        add_cxflags("-frtti")
        add_shflags("-Wl,-flat_namespace,-undefined,dynamic_lookup")
        add_links("sqlite3")
        add_frameworks("CoreFoundation")
    end

    add_headerfiles("./**.h")

    -- set_policy("build.optimization.lto", true)
    add_rules("c++.unity_build", {batchsize = 0})
    add_files("./**.cpp|data/driver/**.cpp|common/db_connect/mysql/**.cpp|data/indicator_talib/**.cpp")
   
    add_files("./data/*.cpp", "./common/serialization/*.cpp", "./data/factor/**.cpp", {unity_group="base"})
    add_files("./data/indicator/**.cpp", {unity_group="indicator"})

    add_files("./analysis/**.cpp", {unity_group="analysis"})
    add_files("./app/*.cpp", "./app/runtime/**.cpp", {unity_group="app"})
    add_files("./app/plugin/**.cpp", {unity_group="plugin"})
    add_files("./execution/**.cpp", {unity_group="execution"})
    add_files("./strategy/**.cpp", {unity_group="strategy"})

    add_files("./strategy/allocatefunds/**.cpp", {unity_group="allocatefunds"})
    add_files("./strategy/condition/*.cpp", "./strategy/condition/imp/logic/*.cpp", {unity_group="condition"})
    add_files("./strategy/condition/imp/*.cpp", {unity_group="condition_imp"})
    add_files("./strategy/environment/*.cpp", "./strategy/environment/imp/logic/*.cpp", {unity_group="environment"})
    add_files("./strategy/environment/imp/*.cpp", {unity_group="environment_imp"})
    add_files("./strategy/moneymanager/**.cpp", {unity_group="moneymanager"})
    add_files("./strategy/multifactor/*.cpp|StyleRegression.cpp", "./strategy/multifactor/imp/*.cpp", {unity_group="multifactor"})
    -- StyleRegression.cpp 单独编译：包含 Eigen 头，避免 unity build 拼接后
    -- Eigen 内联实现泄漏给同组其他源码（编译变慢 + 潜在 ODR 风险）
    add_files("./strategy/multifactor/StyleRegression.cpp", {unity_group = false})
    add_files("./strategy/multifactor/filter/*.cpp", {unity_group="multifactor_filter"})
    add_files("./strategy/multifactor/normalize/*.cpp", {unity_group="multifactor_norm"})
    add_files("./strategy/portfolio/**.cpp", {unity_group="portfolio"})
    add_files("./strategy/profitgoal/**.cpp", {unity_group="profitgoal"})
    add_files("./strategy/selector/*.cpp", "./strategy/selector/imp/logic/*.cpp", {unity_group="selector"})
    add_files("./strategy/selector/imp/optimal/*.cpp", {unity_group="selector"})
    add_files("./strategy/selector/imp/*.cpp", {unity_group="selector_imp"})
    add_files("./strategy/signal/*.cpp", "./strategy/signal/crt/*.cpp", "./strategy/signal/imp/logic/*.cpp", {unity_group="signal"})
    add_files("./strategy/signal/imp/*.cpp", {unity_group="signal_imp"})
    add_files("./strategy/slippage/**.cpp", {unity_group="slippage"})
    add_files("./strategy/stoploss/**.cpp", {unity_group="stoploss"})

    add_files("./common/*.cpp", "./common/datetime/*.cpp", "./common/ini_parser/*.cpp", {unity_group="common"})
    add_files("./common/thread/*.cpp", "./common/db_connect/*.cpp", "./common/http_client/*.cpp", {unity_group="common"})

    add_files("./data/driver/*.cpp", {unity_group="data_driver"})
    add_files("./data/driver/ipc/**.cpp", {unity_group="data_driver"})
    add_files("./data/driver/block_info/qianlong/**.cpp", {unity_group="data_driver"})
    add_files("./data/driver/kdata/cvs/**.cpp", {unity_group="data_driver"})
    if get_config("hdf5") or get_config("sqlite") then
        add_files("./common/db_connect/sqlite/**.cpp", {unity_group="sqlite"})
        add_files("./data/driver/base_info/sqlite/**.cpp", {unity_group="sqlite"})
        add_files("./data/driver/block_info/sqlite/**.cpp", {unity_group="sqlite"})
    end
    if get_config("mysql") then
        add_files("./data/driver/base_info/mysql/**.cpp", {unity_group="mysql"})
        add_files("./data/driver/block_info/mysql/**.cpp", {unity_group="mysql"})
    end
    if get_config("sqlite") or get_config("hdf5") then
        add_files("./data/driver/kdata/sqlite/**.cpp", {unity_group="sqlite"})
    end
    if get_config("hdf5") then
        add_files("./data/driver/kdata/hdf5/**.cpp", {unity_group="hdf5"})
    end
    if get_config("mysql") then
        add_files("./data/driver/kdata/mysql/**.cpp", {unity_group="mysql"})
    end
    if get_config("tdx") then
        add_files("./data/driver/kdata/tdx/**.cpp", {unity_group="tdx"})
    end
    if get_config("mysql") then
        add_files("./common/db_connect/mysql/mysql_imp.cpp")
    end
    if has_config("ta_lib") then
        add_files("./data/indicator_talib/**.cpp", {unity_group="talib"})
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
        local dst_path = target:installdir() .. "/include/hikyuu/python/"
        os.cp("$(projectdir)/hikyuu_pywrap/common/pybind_utils.h", dst_path)
        os.cp("$(projectdir)/hikyuu_pywrap/common/pickle_support.h", dst_path)

        local destpath = target:installdir()
        import("core.project.task")
        task.run("copy_dependents", {}, target, destpath, true)
    end)
target_end()
