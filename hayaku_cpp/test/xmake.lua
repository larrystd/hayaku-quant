add_requires("doctest")

local function enable_project_asan()
    if get_config("leak_check") and is_plat("macosx", "linux") then
        set_symbols("debug")
        set_policy("build.sanitizer.address", true)
        if is_plat("linux") then
            set_policy("build.sanitizer.leak", true)
        end
    end
end

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
                'test/*' 'src/data/storage/*' 'src/extensions/*' 'src/application/plugins/*' 'src/common/*' \
                -o cover-final.info")
        
        -- 生成的html及相关文件的目录名称，--legend 简单的统计信息说明
        os.exec("genhtml -o cover_report --legend --title 'hayaku' cover-final.info")

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
      os.cp("$(projectdir)/hayaku/plugin", "$(builddir)/$(mode)/$(plat)/$(arch)/lib/")
      os.cp("$(projectdir)/i18n", "$(builddir)/$(mode)/$(plat)/$(arch)/lib/")
      print("copy finished")
    end
end


target("unit-test")
    enable_project_asan()
    set_kind("binary")
    set_default(false)

    if is_mode("coverage") then 
        add_cxflags("-fprofile-update=atomic")
    end    

    if has_config("mysql") and not has_config("disable_libmysqlclient") then
        add_packages("mysql")
    end    


    add_packages("boost", "fmt", "spdlog", "doctest", "sqlite3", "nlohmann_json", "tl_expected", "eigen", "nng", "flatbuffers", "utf8proc")
    if has_config("ta_lib") then
        add_packages("ta-lib")
    end

    add_includedirs(".", "../src", "$(builddir)/autogen")

    if is_plat("windows") then
        add_cxflags("/bigobj")
        add_cxflags("-wd4267", "-wd4996", "-wd4251", "-wd4244", "-wd4805", "-wd4566")
    else
        add_cxflags("-Wno-unused-variable",  "-Wno-missing-braces")
        add_cxflags("-Wno-sign-compare", "-Wno-self-assign-overloaded")
    end
    
    if is_plat("windows") and get_config("kind") == "shared" then
        add_defines("HAYAKU_API=__declspec(dllimport)")
        add_defines("HAYAKU_UTILS_API=__declspec(dllimport)")
    end

    add_deps("hayaku", "hayaku-realtime", "hayaku_abi_valid", "hayaku_abi_legacy",
             "hayaku_abi_wrong_version", "hayaku_abi_wrong_id", "hayaku_abi_no_destroy",
             "hayaku_abi_wrong_interface")

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
    add_files("./**.cpp|data/real_data/**|extensions/talib/**.cpp|application/plugin_fixtures/**.cpp")
    
    add_files("./application/*.cpp", "./application/plugin_fixtures/plugin_valid.cpp",
              "./extensions/realtime/*.cpp", "./extensions/mysql/*.cpp", {unity_group="app"})
    add_files("./data/*.cpp", {unity_group="data"})
    add_files("./operators/*.cpp", {unity_group="operators"})
    add_files("./common/serialization/**.cpp", {unity_group="serialization"})
    add_files("./execution/**.cpp", {unity_group="execution"})
    add_files("./strategy/*.cpp", {unity_group="strategy"})
    add_files("./strategy/decision/*.cpp", {unity_group="decision"})
    add_files("./strategy/risk/*.cpp", {unity_group="risk"})
    add_files("./strategy/selection/*.cpp|test_style_regression.cpp", {unity_group="selection"})
    -- 测试包含 Eigen 头，单独编译避免 unity build 泄漏 Eigen 内联实现
    add_files("./strategy/selection/test_style_regression.cpp", {unity_group = false})
    add_files("./strategy/portfolio/*.cpp", {unity_group="portfolio"})
    add_files("./common/**.cpp", {unity_group="common"})

    if has_config("ta_lib") then
        add_files("extensions/talib/*.cpp", {unity_group="talib"})
    end

    before_run(prepare_run)
    after_run(coverage_report)
target_end()

target("small-test")
    enable_project_asan()
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
        add_defines("HAYAKU_API=__declspec(dllimport)")
        add_defines("HAYAKU_UTILS_API=__declspec(dllimport)")
    end

    add_deps("hayaku", "hayaku_abi_valid", "hayaku_abi_legacy", "hayaku_abi_wrong_version", "hayaku_abi_wrong_id", "hayaku_abi_no_destroy", "hayaku_abi_wrong_interface")

    if is_plat("linux") or is_plat("macosx") then
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
    end

    -- add files
    if get_config("hdf5") then
        add_files("./data/*.cpp")
    end
    add_files("./application/test_PluginLoader.cpp",
              "./application/plugin_fixtures/plugin_valid.cpp", "./test_main.cpp")

    before_run(prepare_run)
    after_run(coverage_report)
target_end()

for _, fixture in ipairs({"valid", "legacy", "wrong_version", "wrong_id", "no_destroy", "wrong_interface"}) do
    target("hayaku_abi_" .. fixture)
        enable_project_asan()
        set_kind("shared")
        set_default(false)
        add_includedirs("../src")
        add_packages("fmt", "spdlog")
        add_deps("hayaku")
        add_files("./application/plugin_fixtures/" .. fixture .. ".cpp")
        if is_plat("windows") then
            add_defines("HAYAKU_API=__declspec(dllimport)", "HAYAKU_UTILS_API=__declspec(dllimport)")
        end
    target_end()
end

target("plugin-abi-test")
    enable_project_asan()
    set_kind("binary")
    set_default(false)
    add_packages("doctest", "spdlog", "fmt")
    add_includedirs(".", "../src")
    add_deps("hayaku", "hayaku_abi_valid", "hayaku_abi_legacy", "hayaku_abi_wrong_version", "hayaku_abi_wrong_id", "hayaku_abi_no_destroy", "hayaku_abi_wrong_interface")
    add_files("./application/test_PluginLoader.cpp",
              "./application/plugin_fixtures/test_main.cpp")
    if is_plat("windows") and get_config("kind") == "shared" then
        add_defines("HAYAKU_API=__declspec(dllimport)", "HAYAKU_UTILS_API=__declspec(dllimport)")
    end
    if is_plat("linux") or is_plat("macosx") then
        add_shflags("-Wl,-rpath=$ORIGIN", "-Wl,-rpath=$ORIGIN/../lib")
    end
target_end()

target("real-test")
    enable_project_asan()
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
        add_defines("HAYAKU_API=__declspec(dllimport)")
        add_defines("HAYAKU_UTILS_API=__declspec(dllimport)")
    end

    add_defines("HAYAKU_USE_REAL_DATA_TEST")
    add_deps("hayaku")

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
