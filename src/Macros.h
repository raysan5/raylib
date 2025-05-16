#ifdef ENVIRONMENT64

#define _CRT_SECURE_NO_WARNINGS
#define WIN32
#define _WINDOWS
#define NDEBUG
#define BUILD_LIBTYPE_SHARED
#define CMAKE_INTDIR = "Release"
#define raylib_EXPORTS

// RCORE

#define PLATFORM_DESKTOP
#define GRAPHICS_API_OPENGL_33

// RAUDIO

#define SUPPORT_FILEFORMAT_FLAC

// LIBS

#pragma comment(lib, "winmm.lib")

#endif

#ifdef ENVIRONMENT32

#define BUILD_LIBTYPE_SHARED
#define WIN32
#define _WINDOWS
#define NDEBUG
#define _CRT_SECURE_NO_WARNINGS
#define CMAKE_INTDIR = "Release"
#define raylib_EXPORTS

// RCORE

#define PLATFORM_DESKTOP
#define GRAPHICS_API_OPENGL_21

// RAUDIO

#define SUPPORT_FILEFORMAT_FLAC

#pragma comment(lib, "winmm.lib")

#endif