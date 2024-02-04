function raylib_link( pathToRaylib )
	links { "raylib" }
	dependson { "raylib" }
	includedirs { pathToRaylib .. "src" }

	libdirs { pathToRaylib .. "bin/%{cfg.buildcfg}" }
	filter { "system:linux" }
		links { "m", "GL", "pthread", "dl", "rt", "X11" } -- only supporting X11 with linux rn
	filter { "system:macosx" }
		links { "OpenGL.framework", "Cocoa.framework", "IOKit.framework", "CoreFoundation.framework", "CoreAudio.framework", "CoreVideo.framework", "AudioToolbox.framework" }
	filter { "system:windows" }
		links { "winmm", "kernel32", "opengl32", "gdi32" }
		defines { "_WIN32" }
	filter { "action:" }
	filter {}
end

newoption {
	trigger = "platform", default = "desktop", value = "Platform type",
	description = "what platform youre using",
	allowed = {
		{ "desktop", "Desktop OSs" },
		{ "desktop-sdl", "Desktop OS with SDL backend" },
		{ "web", "HTML5 with WebAssembly"},
		{ "drm", "RaspberryPi and such" },
		{ "android", "Android (ARM, ARM64)" }
	}
}

-- Choose what opengl versoin you want to work with

--desktopGL = "GRAPHICS_API_OPENGL_11"
--desktopGL = "GRAPHICS_API_OPENGL_21"
desktopGL = "GRAPHICS_API_OPENGL_33"
--desktopGL = "GRAPHICS_API_OPENGL_43"

embededGL = "GRAPHICS_API_OPENGL_ES2"

libType = "StaticLib"
--libType = "SharedLib"

project "raylib"
	language "C"
	cdialect "C99"
	kind (libType)
	
	objdir "obj/%{cfg.buildcfg}"
	targetdir "bin/%{cfg.buildcfg}"

	files {
		"src/*.c",
		"src/*.h"
	}
	includedirs { "src/external/glfw/include" }

	filter { "options:platform=desktop" }
		defines { desktopGL, "PLATFORM_DESKTOP" }
	filter { "options:platform=desktop-sdl" }
		defines { desktopGL, "PLATFORM_DESKTOP_SDL" }
	filter { "options:platform=web" }
		defines { embededGL, "PLATFORM_WEB", "_DEFAULT_SOURCE" }
	filter { "options:platform=drm" }
		defines { embededGL, "PLATFORM_DRM" }
	filter { "options:platform=android" }
		defines { embededGL, "PLATFORM_ANDROID" }
	filter {}

	filter { "system:linux" }
		defines { "_GNU_SOURCE", "PLATFORM_OS=LINUX" }
	filter { "system:macosx" }
		disablewarnings {"deprecated-declarations" }
		defines { "PLATOFRM_OS=OSX" }
	filter { "system:bsd" }
		defines { "PLATFORM_OS=BSD" }
	filter { "system:windows" }
		defines { "PLATFORM_OS=WINDOWS" }
	filter {}

