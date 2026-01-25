-- Start VoxelFactory Project
project "VoxelFactory"
    kind "ConsoleApp"
    staticruntime "off"
    systemversion "latest"

    language "C++"
    cppdialect "C++23"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "src/**.c",
        "src/**.h",
        "src/**.cpp",
        "src/**.hpp",
        "src/**.inl",
    }
    includedirs {
        "src",
        "%{wks.location}/vendor/glfw/include",
        "%{wks.location}/vendor/glad/include",
        "%{wks.location}/vendor/glm",
        "%{wks.location}/vendor/stb/include"
    }

    defines {
        "GLFW_INCLUDE_NONE",
        "GLM_ENABLE_EXPERIMENTAL",
        "STB_IMAGE_IMPLEMENTATION"
    }
    links {
        "glad",
        "glfw" 
    }

    filter "system:windows"
        defines { "PLATFORM_WINDOWS" }

    filter "configurations:Debug"
        defines "DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "RELEASE"
        optimize "On"
