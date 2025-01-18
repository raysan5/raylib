/**********************************************************************************************
*
*   rglfw - raylib GLFW single file compilation
*
*   This file includes latest GLFW sources (https://github.com/glfw/glfw) to be compiled together
*   with raylib for all supported platforms, this way, no external dependencies are required.
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2017-2025 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

//#define _GLFW_BUILD_DLL           // To build shared version
// Ref: http://www.glfw.org/docs/latest/compile.html#compile_manual

// Platform options:
// _GLFW_WIN32      to use the Win32 API
// _GLFW_X11        to use the X Window System
// _GLFW_WAYLAND    to use the Wayland API (experimental and incomplete)
// _GLFW_COCOA      to use the Cocoa frameworks
//
// On Linux, _GLFW_X11 and _GLFW_WAYLAND can be combined

//----------------------------------------------------------------------------------
// Feature Test Macros required for this module
//----------------------------------------------------------------------------------
#if (defined(__linux__) || defined(PLATFORM_WEB)) && (_POSIX_C_SOURCE < 199309L)
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 199309L // Required for: CLOCK_MONOTONIC if compiled with c99 without gnu ext.
#endif
#if (defined(__linux__) || defined(PLATFORM_WEB)) && !defined(_GNU_SOURCE)
    #undef _GNU_SOURCE
    #define _GNU_SOURCE // Required for: ppoll if compiled with c99 without gnu ext.
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
    #define _GLFW_WIN32
#endif
#if defined(__linux__)
    #if !defined(_GLFW_WAYLAND) && !defined(_GLFW_X11)
        #error "Cannot disable Wayland and X11 at the same time"
    #endif
#endif
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
    #define _GLFW_X11
#endif
#if defined(__APPLE__)
    #define _GLFW_COCOA
    #define _GLFW_USE_MENUBAR       // To create and populate the menu bar when the first window is created
    #define _GLFW_USE_RETINA        // To have windows use the full resolution of Retina displays
#endif
#if defined(__TINYC__)
    #define _WIN32_WINNT_WINXP      0x0501
#endif

// Common modules to all platforms
#include "external/glfw/src/init.c"
#include "external/glfw/src/platform.c"
#include "external/glfw/src/context.c"
#include "external/glfw/src/monitor.c"
#include "external/glfw/src/window.c"
#include "external/glfw/src/input.c"
#include "external/glfw/src/vulkan.c"

#if defined(_WIN32) || defined(__CYGWIN__)
    #include "external/glfw/src/win32_init.c"
    #include "external/glfw/src/win32_module.c"
    #include "external/glfw/src/win32_monitor.c"
    #include "external/glfw/src/win32_window.c"
    #include "external/glfw/src/win32_joystick.c"
    #include "external/glfw/src/win32_time.c"
    #include "external/glfw/src/win32_thread.c"
    #include "external/glfw/src/wgl_context.c"

    #include "external/glfw/src/egl_context.c"
    #include "external/glfw/src/osmesa_context.c"
#endif

#if defined(__linux__)
    #include "external/glfw/src/posix_module.c"
    #include "external/glfw/src/posix_thread.c"
    #include "external/glfw/src/posix_time.c"
    #include "external/glfw/src/posix_poll.c"
    #include "external/glfw/src/linux_joystick.c"
    #include "external/glfw/src/xkb_unicode.c"

    #include "external/glfw/src/egl_context.c"
    #include "external/glfw/src/osmesa_context.c"

    #if defined(_GLFW_WAYLAND)
        #include "external/glfw/src/wl_init.c"
        #include "external/glfw/src/wl_monitor.c"
        #include "external/glfw/src/wl_window.c"
    #endif
    #if defined(_GLFW_X11)
        #include "external/glfw/src/x11_init.c"
        #include "external/glfw/src/x11_monitor.c"
        #include "external/glfw/src/x11_window.c"
        #include "external/glfw/src/glx_context.c"
    #endif
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined( __NetBSD__) || defined(__DragonFly__)
    #include "external/glfw/src/posix_module.c"
    #include "external/glfw/src/posix_thread.c"
    #include "external/glfw/src/posix_time.c"
    #include "external/glfw/src/posix_poll.c"
    #include "external/glfw/src/null_joystick.c"
    #include "external/glfw/src/xkb_unicode.c"

    #include "external/glfw/src/x11_init.c"
    #include "external/glfw/src/x11_monitor.c"
    #include "external/glfw/src/x11_window.c"
    #include "external/glfw/src/glx_context.c"

    #include "external/glfw/src/egl_context.c"
    #include "external/glfw/src/osmesa_context.c"
#endif

#if defined(__APPLE__)
    #include "external/glfw/src/posix_module.c"
    #include "external/glfw/src/posix_thread.c"
    #include "external/glfw/src/cocoa_init.m"
    #include "external/glfw/src/cocoa_joystick.m"
    #include "external/glfw/src/cocoa_monitor.m"
    #include "external/glfw/src/cocoa_window.m"
    #include "external/glfw/src/cocoa_time.c"
    #include "external/glfw/src/nsgl_context.m"

    #include "external/glfw/src/egl_context.c"
    #include "external/glfw/src/osmesa_context.c"
#endif
