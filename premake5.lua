workspace "LZW"
    architecture "x86_64"

    configurations { 
        "Release", 
        "Debug"
    }

    targetdir "bin/%{cfg.buildcfg}/%{prj.name}"
    objdir "obj/%{cfg.buildcfg}/%{prj.name}"

    startproject "LZW_CLI"

    filter "system:Windows"
        systemversion "latest"

    filter "Release"
        defines { "NDEBUG", "RELEASE" }
        optimize "On"

    filter "Debug"
        defines { "DEBUG" }
        symbols "On"

    -- <filesystem> support for all projects
    filter "toolset:gcc"    -- GCC v8.x
        links "stdc++fs" 
    filter "toolset:clang"  -- clang
        if _OPTIONS["cc"] == "clang" then
            premake.warn("Clang is not supporterd yet. Problems with <filesystem> may occur.")
        end
        --links "c++fs"
        --buildoptions {"-stdlib=libc++"}
    filter {}

    -- LZW_Core project cotains logic and algorithms for LZW coding.
    -- It also manages filesystem for input and output data.
    project "LZW_Core"
        location "LZW_Core"
        kind "StaticLib"
        language "C++"
        cppdialect "C++17"
        targetname "lzw_core"

        files {
            "%{prj.name}/src/**.hpp",
            "%{prj.name}/src/**.cpp"
        }

        filter "Release"
            optimize "Speed"

    -- Simple Command-Line Interface for LZW_Core. 
    -- There should be no critical logic here.
    project "LZW_CLI"
        location "LZW_CLI"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"
        targetname "lzw"

        files {
            "%{prj.name}/src/**.hpp",
            "%{prj.name}/src/**.cpp",
            "%{prj.name}/vendor/CLI/CLI11.hpp"
        }

        includedirs {
            "LZW_Core/src",
            "%{prj.name}/vendor"
        }

        links {
            "LZW_Core"
        }


    group "Tests"

        -- Tests project for LZW_Core. It produces executable with
        -- embeded CLI from Catch2. Run this program with no arguments
        -- to simply run all the tests.
        project "LZW_Core_Tests"
            location "LZW_Core_Tests"
            kind "ConsoleApp"
            language "C++"
            cppdialect "C++17"
            targetname "lzw_core_tests"

            files {
                "%{prj.name}/src/**.hpp",
                "%{prj.name}/src/**.cpp",
                "%{prj.name}/vendor/catch2/catch.hpp"
            }

            includedirs {
                "LZW_Core/src",
                "%{prj.name}/vendor"
            }

            links {
                "LZW_Core"
            }

    group ""


-- Additional actions for maintaining project files.
newaction {
   trigger     = "clean",
   description = "Clean project from binaries and obj files",
   execute = function ()
      print("Removing bin/...")
	  os.rmdir("./bin")
	  print("Removing obj/...")
	  os.rmdir("./obj")
	  print("Done")
   end
}

newaction {
	trigger     = "reset",
	description = "Removes all files ignored by git",
	execute = function()
		print("Removing generated files...")
		os.execute("git clean -Xdf")
		print("Done")
	end
}
