project "glad"
    kind "StaticLib"
	staticruntime "off"
	systemversion "latest"
    language "C"

    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

	files {
		"glad/include/glad/glad.h",
		"glad/include/KHR/khrplatform.h",
		"glad/src/glad.c"
	}

	includedirs {
		"glad/include"
	}

project "glfw"
    kind "StaticLib"
	staticruntime "off"
	systemversion "latest"
    language "C"

    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    files {
		"glfw/include/GLFW/glfw3.h",
		"glfw/include/GLFW/glfw3native.h",
		"glfw/src/glfw_config.h",
		"glfw/src/context.c",
		"glfw/src/init.c",
		"glfw/src/input.c",
		"glfw/src/monitor.c",

		"glfw/src/null_init.c",
		"glfw/src/null_joystick.c",
		"glfw/src/null_monitor.c",
		"glfw/src/null_window.c",
		"glfw/src/null_platform.h",
		"glfw/src/null_joystick.h",

		"glfw/src/platform.c",
		"glfw/src/vulkan.c",
		"glfw/src/window.c",

		"glfw/src/internal.h",
		"glfw/src/platform.h",
		"glfw/src/mappings.h",
	}

    filter "system:windows"
        files {
			"GLFW/src/win32_init.c",
			"GLFW/src/win32_joystick.c",
			"GLFW/src/win32_module.c",
			"GLFW/src/win32_monitor.c",
			"GLFW/src/win32_time.c",
			"GLFW/src/win32_thread.c",
			"GLFW/src/win32_window.c",
			"GLFW/src/wgl_context.c",
			"GLFW/src/egl_context.c",
			"GLFW/src/osmesa_context.c"
		}
		defines { 
			"_GLFW_WIN32",
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "system:linux"
		pic "On"

		files {
			"GLFW/src/x11_init.c",
			"GLFW/src/x11_monitor.c",
			"GLFW/src/x11_window.c",
			"GLFW/src/x11_platform.h",
			"GLFW/src/xkb_unicode.c",
			"GLFW/src/posix_time.h",
			"GLFW/src/posix_time.c",
			"GLFW/src/posix_thread.h",
			"GLFW/src/posix_thread.c",
			"GLFW/src/posix_module.c",
			"GLFW/src/posix_poll.c",
			"GLFW/src/posix_poll.h",
			"GLFW/src/glx_context.c",
			"GLFW/src/egl_context.c",
			"GLFW/src/osmesa_context.c",
			"GLFW/src/linux_joystick.c",
		}
		includedirs {
			"GLFW/src"
		}
		defines {
			"_GLFW_X11",
			"_CRT_SECURE_NO_WARNINGS"
		}


-- Configurations
    filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter { "system:windows", "configurations:Debug-AS" }	
		runtime "Debug"
		symbols "on"
		sanitize { "Address" }
		flags { "NoRuntimeChecks", "NoIncrementalLink" }

	filter "configurations:Release"
		runtime "Release"
		optimize "speed"