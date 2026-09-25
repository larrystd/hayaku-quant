add_requires("doctest")

function coverage_report(target)
    if is_mode("coverage") and not is_plat("windows") and not (linuxos.name() == "ubuntu" and linuxos.version():lt("20.0")) then 
        -- 如需分支覆盖，须在下面 lcov, genhtml 命令后都加入: --rc lcov_branch_coverage=1
        print("Processing coverage info ...")
        
        if not os.isfile("cover-init.info") then
            -- 初始化并创建基准数据文件
            os.run("lcov -c -i -d ./ -o cover-init.info") 
        end

        -- 移除之前的结果，否则容易出错
        os.tryrm('cover.info')
        os.tryrm('cover-total.info')

        -- 收集当前测试文件运行后产生的覆盖率文件
        os.run("lcov -c -d ./ -o cover.info")

        -- 合并基准数据和执行测试文件后生成的覆盖率数据
        os.exec("lcov --ignore-errors empty --rc geninfo_unexecuted_blocks=1 -a cover-init.info -a cover.info -o cover-total.info")

        -- 删除统计信息中如下的代码或文件，支持正则
        os.exec("lcov --ignore-errors unused --remove cover-total.info '*/usr/include/*' \
                '*/usr/lib/*' '*/usr/local/include/*' '*/usr/local/lib/*' '*/usr/local/lib64/*' \
                '*/.xmake*' '*/boost/*' \
                'test/*' 'src/data/driver/*' 'src/app/runtime/*' 'src/app/plugin/*' 'src/common/*' \
                -o cover-final.info")
        
        -- 生成的html及相关文件的目录名称，--legend 简单的统计信息说明
        os.exec("genhtml -o cover_report --legend --title 'hikyuu' cover-final.info")

        -- 生成 sonar 可读取报告
        if is_plat("linux") then
            os.run("gcovr -r . -e cpp/test --xml -o coverage.xml")
        end
    end
end

function prepare_run(target)
    local targetname = target:name()
  
    if "unit-test" == targetname or "small-test" == targetname or "prepare-test" == targetname then
      print("copying test_data ...")
      os.rm("$(builddir)/$(mode)/$(plat)/$(arch)/lib/test_data")
      os.cp("$(projectdir)/test_data", "$(builddir)/$(mode)/$(plat)/$(arch)/lib/")
      os.cp("$(projectdir)/hikyuu/plugin", "$(builddir)/$(mode)/$(plat)/$(arch)/lib/")
      os.cp("$(projectdir)/i18n", "$(builddir)/$(mode)/$(plat)/$(arch)/lib/")
      print("copy finished")
    end
end


target("unit-test")
    set_kind("binary")
    set_default(false)

    if is_mode("coverage") then 
        add_cxflags("-fprofile-update=atomic")
    end    

    if get_config("leak_check") then
        if is_plat("macosx") then
            set_policy("build.sanitizer.address", true)
        elseif is_plat("linux") then
            -- 需要 export LD_PRELOAD=libasan.so
            -- 也可能需要抑制 openssl 泄露告警：export LSAN_OPTIONS="suppressions=asan.sup"
            -- asan.sup 需要copy到 unit-test执行目录
            set_policy("build.sanitizer.address", true)
            set_policy("build.sanitizer.leak", true)
            -- set_policy("build.sanitizer.memory", true)
            -- set_policy("build.sanitizer.thread", true)
        end
    end

    if has_config("mysql") and not has_config("disable_libmysqlclient") then
        add_packages("mysql")
    end    


    add_packages("boost", "fmt", "spdlog", "doctest", "sqlite3", "nlohmann_json", "tl_expected", "eigen", "nng")
    if has_config("ta_lib") then
        add_packages("ta-lib")
    end

    add_includedirs(".", "../src")

    if is_plat("windows") then
        add_cxflags("/bigobj")
        add_cxflags("-wd4267", "-wd4996", "-wd4251", "-wd4244", "-wd4805", "-wd4566")
    else
        add_cxflags("-Wno-unused-variable",  "-Wno-missing-braces")
        add_cxflags("-Wno-sign-compare", "-Wno-self-assign-overloaded")
    end
    
    if is_plat("windows") and get_config("kind") == "shared" then
        add_defines("HKU_API=__declspec(dllimport)")
        add_defines("HKU_UTILS_API=__declspec(dllimport)")
    end

    add_deps("hikyuu")

    if is_plat("linux") or is_plat("macosx") then
        add_links("sqlite3")
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
    end

    if is_plat("macosx") then
        -- boost序列化需要
        add_cxflags("-frtti -fvisibility=default")
        add_ldflags("-Wl,-flat_namespace,-undefined,dynamic_lookup")
    end

    -- set_policy("build.optimization.lto", true)
    add_rules("c++.unity_build", {batchsize = 0})
    add_files("./**.cpp|data/real_data/**|data/indicator_talib/**.cpp")
    
    add_files("./analysis/**.cpp", {unity_group="analysis"})
    add_files("./app/**.cpp", {unity_group="app"})
    add_files("./data/*.cpp", {unity_group="data"})
    add_files("./data/indicator/**.cpp", {unity_group="indicator"})
    add_files("./data/factor/**.cpp", {unity_group="factor"})
    add_files("./common/serialization/**.cpp", {unity_group="serialization"})
    add_files("./execution/**.cpp", {unity_group="execution"})
    add_files("./strategy/allocatefunds/**.cpp", {unity_group="allocatefunds"})
    add_files("./strategy/condition/**.cpp", {unity_group="condition"})
    add_files("./strategy/environment/**.cpp", {unity_group="environment"})
    add_files("./strategy/moneymanager/**.cpp", {unity_group="moneymanager"})
    add_files("./strategy/multifactor/**.cpp|test_style_regression.cpp", {unity_group="multifactor"})
    -- 测试包含 Eigen 头，单独编译避免 unity build 泄漏 Eigen 内联实现
    add_files("./strategy/multifactor/test_style_regression.cpp", {unity_group = false})
    add_files("./strategy/portfolio/**.cpp", {unity_group="portfolio"})
    add_files("./strategy/profitgoal/**.cpp", {unity_group="profitgoal"})
    add_files("./strategy/selector/**.cpp", {unity_group="selector"})
    add_files("./strategy/signal/**.cpp", {unity_group="signal"})
    add_files("./strategy/slippage/**.cpp", {unity_group="slippage"})
    add_files("./strategy/stoploss/**.cpp", {unity_group="stoploss"})
    add_files("./common/**.cpp", {unity_group="common"})

    if has_config("ta_lib") then
        add_files("data/indicator_talib/**.cpp", {unity_group="talib"})
    end

    before_run(prepare_run)
    after_run(coverage_report)
target_end()

target("small-test")
    set_kind("binary")
    set_default(false)
    
    if has_config("mysql") and not has_config("disable_libmysqlclient") then
        add_packages("mysql")
    end    

    add_packages("boost", "fmt", "spdlog", "doctest", "sqlite3", "nlohmann_json", "tl_expected", "nng")

    add_includedirs(".", "../src")

    if is_plat("windows") then
        add_cxflags("-wd4267")
        add_cxflags("-wd4251")
        add_cxflags("-wd4244")
        add_cxflags("-wd4805")
        add_cxflags("-wd4566")
    else
        add_cxflags("-Wno-unused-variable",  "-Wno-missing-braces")
        add_cxflags("-Wno-sign-compare")
    end
    
    if is_plat("windows") and get_config("kind") == "shared" then
        add_defines("HKU_API=__declspec(dllimport)")
        add_defines("HKU_UTILS_API=__declspec(dllimport)")
    end

    add_deps("hikyuu")

    if is_plat("linux") or is_plat("macosx") then
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
    end

    -- add files
    add_files("./data/*.cpp", "./app/plugin/*.cpp", "./test_main.cpp")

    before_run(prepare_run)
    after_run(coverage_report)
target_end()

target("real-test")
    set_kind("binary")
    set_default(false)

    if has_config("mysql") and not has_config("disable_libmysqlclient") then
        add_packages("mysql")
    end
    
    add_packages("boost", "fmt", "spdlog", "doctest", "sqlite3", "mysql", "nlohmann_json", "tl_expected", "nng")
    
    add_includedirs(".", "../src")

    if is_plat("windows") then
        add_cxflags("-wd4267", "-wd4996", "-wd4251", "-wd4244", "-wd4805", "-wd4566")
    else
        add_cxflags("-Wno-unused-variable",  "-Wno-missing-braces")
        add_cxflags("-Wno-sign-compare")
    end
    
    if is_plat("windows") and get_config("kind") == "shared" then
        add_defines("HKU_API=__declspec(dllimport)")
        add_defines("HKU_UTILS_API=__declspec(dllimport)")
    end

    add_defines("HKU_USE_REAL_DATA_TEST")
    add_deps("hikyuu")

    if is_plat("linux") or is_plat("macosx") then
        add_links("sqlite3")
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
    end

    -- add files
    add_files("./data/real_data/**.cpp")
    add_files("./test_main.cpp")

    before_run(prepare_run)
    after_run(coverage_report)
target_end()
