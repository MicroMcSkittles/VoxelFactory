-- Start VoxelFactory Project
project "VoxelFactory"
    kind "ConsoleApp"
    staticruntime "off"
    systemversion "latest"

-- Configure C++
    language "C++"
    cppdialect "C++23"

-- Output Directories
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

-- Include all c/c++ files in project
    files {
        "src/**.c",
        "src/**.h",
        "src/**.cpp",
        "src/**.hpp",
        "src/**.inl",
    }

    includedirs {
        "src"
    }

    defines {
        "GLM_ENABLE_EXPERIMENTAL",
    }

-- Windows
    filter "system:windows"

        defines {
            "PLATFORM_WINDOWS"
        }

-- Configuations
    filter "configurations:Debug"
        defines "DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "RELEASE"
        optimize "On"
