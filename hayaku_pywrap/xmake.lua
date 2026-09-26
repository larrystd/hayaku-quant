
add_requires("pybind11", {system = false, alias = "pybind11"})

local function python_program()
    local configured = os.getenv("HAYAKU_PYTHON")
    if configured and #configured > 0 then
        return configured
    end

    return "python3"
end

target("core")
    set_kind("shared")
    set_default(false)
    -- if is_mode("debug") then 
    --     set_default(false) --会默认禁用这个target的编译，除非显示指定xmake build _hayaku才会去编译，但是target还存在，里面的files会保留到vcproj
    --     --set_enable(false) --set_enable(false)会彻底禁用这个target，连target的meta也不会被加载，vcproj不会保留它
    -- end

    add_deps("hayaku")
    add_packages("boost", "fmt", "spdlog", "flatbuffers", "pybind11", "utf8proc", "nlohmann_json",
                 "tl_expected", "nng")
    if is_plat("windows") then
        set_filename("core.pyd")
        add_cxflags("/bigobj")
        add_cxflags("-wd4251")
    else 
        set_filename("core.so")
    end

    if is_plat("windows") and get_config("kind") == "shared" then
        add_defines("HAYAKU_API=__declspec(dllimport)")
        add_defines("HAYAKU_UTILS_API=__declspec(dllimport)")
        add_cxflags("-wd4566")
    end
    
    local cc = get_config("cc")
    local cxx = get_config("cxx")
    if (cc and string.find(cc, "clang")) or (cxx and string.find(cxx, "clang")) then
        add_cxflags("-Wno-error=parentheses-equality -Wno-error=missing-braces")
    end

    if is_plat("linux", "cross") then 
        add_rpathdirs("$ORIGIN", "$ORIGIN/cpp")
    end

    if is_plat("macosx") then
        add_linkdirs("/usr/lib")

        -- macosx 下不能主动链接 python，所以需要使用如下编译选项
        add_shflags("-undefined dynamic_lookup", "-headerpad_max_install_names")
    end    

    add_includedirs(".", "../hayaku_cpp/src")

    -- set_policy("build.optimization.lto", true)
    add_rules("c++.unity_build", {batchsize = 0})
    add_files("./main.cpp", "./common/**.cpp", "./data/factor/**.cpp", {unity_group="base"})
    add_files("./analysis/**.cpp", {unity_group="analysis"})
    add_files("./app/application_main.cpp", "./app/_HayakuSession.cpp",
              "./app/runtime/agent_main.cpp", "./app/runtime/_SpotRecord.cpp",
              {unity_group="app"})
    add_files("./data/**.cpp", {unity_group="data"})
    add_files("./data/driver/**.cpp", {unity_group="data_driver"})
    add_files("./data/indicator/**.cpp", {unity_group="indicator"})
    add_files("./advanced/_device.cpp",
              "./advanced/_extind.cpp", "./advanced/_hayakuextra.cpp",
              "./advanced/_plugin_main.cpp",
              {unity_group="advanced"})
    add_files("./execution/**.cpp", {unity_group="execution"})
    add_files("./strategy/**.cpp", {unity_group="strategy"})

    on_load("windows", "linux", "macosx", function(target)
        import("lib.detect.find_tool")
        if is_plat("windows") then
            -- detect installed python3
            local python = assert(find_tool("python", {version = true}), "python not found, please install it first! note: python version must > 3.0")
            assert(python.version > "3", python.version .. " python version must > 3.0, please use python3.0 or later!")
            -- find python include and libs directory
            local pydir
            if os.getenv("CONDA_PREFIX") ~= nil then
                pydir = os.iorun("python -c \"import sys; print(sys.executable)\"")
            else
                -- 直接用 python.program 在 conda 环境下切换有问题
                pydir = os.iorunv(python.program, {"-c", "import sys; print(sys.executable)"})
            end
            pydir = path.directory(pydir)
            if pydir:endswith("Scripts") then
                -- if venv is activated, find the real python directory
                file = io.open(pydir .. "/../pyvenv.cfg", "r")
                for line in file:lines() do
                    if string.find(line, "home =") then
                        -- 使用 string.gmatch 函数抽取路径
                        for path in string.gmatch(line, "home = (.*)") do
                            pydir = path
                        end
                    end
                end
                file:close()
            end            
            target:add("includedirs", pydir .. "/include")
            target:add("linkdirs", pydir .. "/libs")
            return
        end
    
        -- get python include directory.
        local pydir_include = nil;
        local pydir_lib = nil;
        if os.getenv("CONDA_PREFIX") ~= nil and os.getenv("HAYAKU_PYTHON") == nil then
            print("CONDA_PREFIX: " .. os.getenv("CONDA_PREFIX"))
            local py3config = os.getenv("CONDA_PREFIX") .. "/bin/python3-config"
            pydir_include = os.iorun(py3config .. " --includes"):trim()
            pydir_lib = os.iorun(py3config .. " --libs"):trim()
        else
            local python_version = os.iorunv(python_program(), {
                "-c", "import sys; v = sys.version_info; print(str(v.major) + '.' + str(v.minor))"
            }):trim()
            local py3config = "python" .. python_version .. "-config"
            -- print("py3config: " .. py3config)
            pydir_include = os.iorun(py3config .. " --includes"):trim()
            pydir_lib = os.iorun(py3config .. " --libs"):trim()
        end
        assert(pydir_include, "python3-config not found!")
        print("pydir_include: " .. pydir_include)
        -- print("pydir_lib: " .. pydir_lib)
        target:add("cxflags", pydir_include, pydir_lib)    
    end)

    after_build(function(target)
        local dst_dir = "$(projectdir)/hayaku/cpp/"
        local dst_obj = dst_dir .. "core.so"

        if not is_plat("cross") then
          local python_version = os.iorunv(python_program(), {
              "-c", "import sys; v = sys.version_info; print(str(v.major) + str(v.minor))"
          }):trim()
          print("python_version: " .. python_version)
          dst_obj = dst_dir .. "core" ..  python_version
        end

        if is_plat("windows") then
            os.cp(target:targetdir() .. '/core.pyd', dst_obj .. ".pyd")
            for _, dll in ipairs(os.files(target:targetdir() .. '/*.dll')) do
                if path.filename(dll) ~= "hayaku-ingest.dll" and
                   path.filename(dll) ~= "hayaku-realtime.dll" then
                    os.cp(dll, dst_dir)
                end
            end
            os.cp(target:targetdir() .. '/hayaku.lib', dst_dir)
        elseif is_plat("macosx") then
            os.cp(target:targetdir() .. '/core.so', dst_obj .. ".so")
            for _, dylib in ipairs(os.files(target:targetdir() .. '/*.dylib')) do
                if path.filename(dylib) ~= "libhayaku-ingest.dylib" and
                   path.filename(dylib) ~= "libhayaku-realtime.dylib" then
                    os.cp(dylib, dst_dir)
                end
            end
        else
            for _, so in ipairs(os.files(target:targetdir() .. '/*.so')) do
                local filename = path.filename(so)
                if filename ~= "ingest.so" and filename ~= "libhayaku-ingest.so" and
                   filename ~= "realtime.so" and filename ~= "libhayaku-realtime.so" then
                    os.cp(so, dst_dir)
                end
            end
            for _, so in ipairs(os.files(target:targetdir() .. '/*.so.*')) do
                if not path.filename(so):startswith("libhayaku-ingest.so.") and
                   not path.filename(so):startswith("libhayaku-realtime.so.") then
                    os.cp(so, dst_dir)
                end
            end
            if not is_plat("cross") then
                os.trymv(dst_dir .. '/core.so', dst_obj .. ".so")
            end
        end

        if is_plat("macosx") then
            dst_obj = dst_obj .. ".so"
            for _, filepath in ipairs(os.files(dst_dir .. "/*.dylib")) do
                -- print(path.filename(filepath))
                local filename = path.filename(filepath)
                os.run(format("install_name_tool -change @rpath/%s @loader_path/%s %s", filename, filename, dst_obj))
                os.run(format("install_name_tool -change %s @loader_path/%s %s", filename, filename, dst_obj))
            end
            os.run(format("install_name_tool -change libssl.3.dylib @loader_path/libssl.3.dylib %s", dst_obj))
            os.run(format("install_name_tool -change libcrypto.3.dylib @loader_path/libcrypto.3.dylib %s", dst_obj))

            if get_config("mysql") then
                if get_config("kind") == "shared" then
                    dst_obj = dst_dir .. "libhayaku.dylib"
                    os.run(format("install_name_tool -change libssl.3.dylib @loader_path/libssl.3.dylib %s", dst_obj))
                    os.run(format("install_name_tool -change libcrypto.3.dylib @loader_path/libcrypto.3.dylib %s", dst_obj))
                else
                    os.cp(target:targetdir() .. '/*.a', dst_dir)
                end

                if not get_config("disable_libmysqlclient") then
                    filename = "libmysqlclient.21.dylib"
                    os.run(format("install_name_tool -change @loader_path/../lib/libssl.3.dylib @loader_path/libssl.3.dylib %s", dst_dir .. filename))
                    os.run(format("install_name_tool -change @loader_path/../lib/libcrypto.3.dylib @loader_path/libcrypto.3.dylib %s", dst_dir .. filename))
                end
            end

            -- 添加 macosx 签名
            local projectdir = os.projectdir() 
            local scan_dir = path.join(projectdir, "hayaku/cpp")
            print("Start signing dynamic libraries in: " .. scan_dir)
            local ok, err = os.execv("find", {
                scan_dir,
                "-type", "f",
                "(", 
                "-name", "*.dylib", 
                "-o", 
                "-name", "*.so", 
                ")",
                "-exec", "codesign", "-s", "-", "--force", "--deep", "{}", ";"
            })
            if not ok then
                raise("Failed to sign libraries: " .. (err or "unknown error"))
            end
            print("Signing completed.")
         end

        os.cp("$(projectdir)/i18n/zh_CN/*.mo", "$(projectdir)/hayaku/cpp/i18n/zh_CN/")
    end)
target_end()

-- Optional Python live extension. Its native library is not part of core.so.
target("realtime")
    set_kind("shared")
    set_default(false)
    add_deps("hayaku-realtime")
    add_packages("boost", "fmt", "spdlog", "flatbuffers", "pybind11", "utf8proc",
                 "nlohmann_json", "tl_expected", "nng")
    add_includedirs(".", "../hayaku_cpp/src")
    add_files("./realtime_main.cpp", "./app/runtime/_SpotAgent.cpp",
              "./advanced/_dataserver.cpp", "./advanced/_shmserver.cpp")

    if is_plat("windows") then
        set_filename("realtime.pyd")
        add_defines("HAYAKU_API=__declspec(dllimport)", "HAYAKU_UTILS_API=__declspec(dllimport)")
        add_cxflags("/bigobj")
    else
        set_filename("realtime.so")
    end
    if is_plat("linux", "cross") then
        add_rpathdirs("$ORIGIN", "$ORIGIN/../hayaku/cpp")
    end
    if is_plat("macosx") then
        add_shflags("-undefined dynamic_lookup", "-headerpad_max_install_names")
    end

    on_load("windows", "linux", "macosx", function(target)
        local include_dir = os.iorunv(python_program(), {
            "-c", "import sysconfig; print(sysconfig.get_path('include'))"
        }):trim()
        assert(#include_dir > 0, "Python include directory not found")
        target:add("includedirs", include_dir)
        if is_plat("windows") then
            local libs_dir = os.iorunv(python_program(), {
                "-c", "import os, sys; print(os.path.join(sys.base_prefix, 'libs'))"
            }):trim()
            target:add("linkdirs", libs_dir)
        end
    end)

    after_build(function(target)
        local dst_dir = "$(projectdir)/hayaku_realtime_native/"
        local core_cpp_dir = "$(projectdir)/hayaku/cpp/"
        local dst_obj = dst_dir .. "realtime"
        if not is_plat("cross") then
            local python_version = os.iorunv(python_program(), {
                "-c", "import sys; v = sys.version_info; print(str(v.major) + str(v.minor))"
            }):trim()
            dst_obj = dst_obj .. python_version
        end

        if is_plat("windows") then
            os.cp(target:targetfile(), dst_obj .. ".pyd")
            os.cp(target:targetdir() .. "/hayaku-realtime.dll", dst_dir)
        elseif is_plat("macosx") then
            os.cp(target:targetfile(), dst_obj .. ".so")
            os.cp(target:targetdir() .. "/libhayaku-realtime.dylib", dst_dir)
            local native_lib = dst_dir .. "libhayaku-realtime.dylib"
            os.run(format("install_name_tool -change @rpath/libhayaku-realtime.dylib @loader_path/libhayaku-realtime.dylib %s", dst_obj .. ".so"))
            for _, dylib in ipairs(os.files(core_cpp_dir .. "/*.dylib")) do
                local filename = path.filename(dylib)
                local location = "@loader_path/../hayaku/cpp/" .. filename
                os.run(format("install_name_tool -change @rpath/%s %s %s", filename, location, dst_obj .. ".so"))
                os.run(format("install_name_tool -change %s %s %s", filename, location, dst_obj .. ".so"))
                os.run(format("install_name_tool -change @rpath/%s %s %s", filename, location, native_lib))
                os.run(format("install_name_tool -change %s %s %s", filename, location, native_lib))
            end
            os.run(format("codesign -s - --force %s", native_lib))
            os.run(format("codesign -s - --force %s", dst_obj .. ".so"))
        else
            os.cp(target:targetfile(), dst_obj .. ".so")
            os.cp(target:targetdir() .. "/libhayaku-realtime.so", dst_dir)
        end
    end)
target_end()

-- Optional Python ingestion extension. core.so can be installed without it.
target("ingest")
    set_kind("shared")
    set_default(false)
    add_deps("hayaku-ingest")
    add_packages("boost", "fmt", "spdlog", "flatbuffers", "pybind11", "utf8proc",
                 "nlohmann_json", "tl_expected", "nng")
    add_includedirs(".", "../hayaku_cpp/src")
    add_files("./ingest_main.cpp", "./advanced/_KDataToHdf5Importer.cpp",
              "./advanced/_KDataToClickHouseImporter.cpp",
              "./advanced/_KDataToMySQLImporte.cpp", "./advanced/_checkdata.cpp")

    if is_plat("windows") then
        set_filename("ingest.pyd")
        add_defines("HAYAKU_API=__declspec(dllimport)", "HAYAKU_UTILS_API=__declspec(dllimport)")
        add_cxflags("/bigobj")
    else
        set_filename("ingest.so")
    end
    if is_plat("linux", "cross") then
        add_rpathdirs("$ORIGIN", "$ORIGIN/../hayaku/cpp")
    end
    if is_plat("macosx") then
        add_shflags("-undefined dynamic_lookup", "-headerpad_max_install_names")
    end

    on_load("windows", "linux", "macosx", function(target)
        local include_dir = os.iorunv(python_program(), {
            "-c", "import sysconfig; print(sysconfig.get_path('include'))"
        }):trim()
        assert(#include_dir > 0, "Python include directory not found")
        target:add("includedirs", include_dir)
        if is_plat("windows") then
            local libs_dir = os.iorunv(python_program(), {
                "-c", "import os, sys; print(os.path.join(sys.base_prefix, 'libs'))"
            }):trim()
            target:add("linkdirs", libs_dir)
        end
    end)

    after_build(function(target)
        local dst_dir = "$(projectdir)/hayaku_ingest_native/"
        local core_cpp_dir = "$(projectdir)/hayaku/cpp/"
        local dst_obj = dst_dir .. "ingest"
        if not is_plat("cross") then
            local python_version = os.iorunv(python_program(), {
                "-c", "import sys; v = sys.version_info; print(str(v.major) + str(v.minor))"
            }):trim()
            dst_obj = dst_obj .. python_version
        end

        if is_plat("windows") then
            os.cp(target:targetfile(), dst_obj .. ".pyd")
            os.cp(target:targetdir() .. "/hayaku-ingest.dll", dst_dir)
        elseif is_plat("macosx") then
            os.cp(target:targetfile(), dst_obj .. ".so")
            os.cp(target:targetdir() .. "/libhayaku-ingest.dylib", dst_dir)
            os.run(format("install_name_tool -change @rpath/libhayaku-ingest.dylib @loader_path/libhayaku-ingest.dylib %s", dst_obj .. ".so"))
            for _, dylib in ipairs(os.files(core_cpp_dir .. "/*.dylib")) do
                local filename = path.filename(dylib)
                local location = "@loader_path/../hayaku/cpp/" .. filename
                os.run(format("install_name_tool -change @rpath/%s %s %s", filename, location, dst_obj .. ".so"))
                os.run(format("install_name_tool -change %s %s %s", filename, location, dst_obj .. ".so"))
                local native_lib = dst_dir .. "libhayaku-ingest.dylib"
                os.run(format("install_name_tool -change @rpath/%s %s %s", filename, location, native_lib))
                os.run(format("install_name_tool -change %s %s %s", filename, location, native_lib))
            end
            os.run(format("codesign -s - --force %s", dst_dir .. "libhayaku-ingest.dylib"))
            os.run(format("codesign -s - --force %s", dst_obj .. ".so"))
        else
            os.cp(target:targetfile(), dst_obj .. ".so")
            os.cp(target:targetdir() .. "/libhayaku-ingest.so", dst_dir)
        end
    end)
target_end()
