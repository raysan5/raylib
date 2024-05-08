//========================================================================
// GLFW 3.4 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2018 Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#if defined(GLFW_BUILD_WIN32_TIMER) || \
    defined(GLFW_BUILD_WIN32_MODULE) || \
    defined(GLFW_BUILD_WIN32_THREAD) || \
    defined(GLFW_BUILD_COCOA_TIMER) || \
    defined(GLFW_BUILD_POSIX_TIMER) || \
    defined(GLFW_BUILD_POSIX_MODULE) || \
    defined(GLFW_BUILD_POSIX_THREAD) || \
    defined(GLFW_BUILD_POSIX_POLL) || \
    defined(GLFW_BUILD_LINUX_JOYSTICK)
 #error "You must not define these; define zero or more _GLFW_<platform> macros instead"
#endif

#include "null_platform.h"
#define GLFW_EXPOSE_NATIVE_EGL
#define GLFW_EXPOSE_NATIVE_OSMESA

#if defined(_GLFW_WIN32)
 #include "win32_platform.h"
 #define GLFW_EXPOSE_NATIVE_WIN32
 #define GLFW_EXPOSE_NATIVE_WGL
#else
 #define GLFW_WIN32_WINDOW_STATE
 #define GLFW_WIN32_MONITOR_STATE
 #define GLFW_WIN32_CURSOR_STATE
 #define GLFW_WIN32_LIBRARY_WINDOW_STATE
 #define GLFW_WGL_CONTEXT_STATE
 #define GLFW_WGL_LIBRARY_CONTEXT_STATE
#endif

#if defined(_GLFW_COCOA)
 #include "cocoa_platform.h"
 #define GLFW_EXPOSE_NATIVE_COCOA
 #define GLFW_EXPOSE_NATIVE_NSGL
#else
 #define GLFW_COCOA_WINDOW_STATE
 #define GLFW_COCOA_MONITOR_STATE
 #define GLFW_COCOA_CURSOR_STATE
 #define GLFW_COCOA_LIBRARY_WINDOW_STATE
 #define GLFW_NSGL_CONTEXT_STATE
 #define GLFW_NSGL_LIBRARY_CONTEXT_STATE
#endif

#if defined(_GLFW_WAYLAND)
 #include "wl_platform.h"
 #define GLFW_EXPOSE_NATIVE_WAYLAND
#else
 #define GLFW_WAYLAND_WINDOW_STATE
 #define GLFW_WAYLAND_MONITOR_STATE
 #define GLFW_WAYLAND_CURSOR_STATE
 #define GLFW_WAYLAND_LIBRARY_WINDOW_STATE
#endif

#if defined(_GLFW_X11)
 #include "x11_platform.h"
 #define GLFW_EXPOSE_NATIVE_X11
 #define GLFW_EXPOSE_NATIVE_GLX
#else
 #define GLFW_X11_WINDOW_STATE
 #define GLFW_X11_MONITOR_STATE
 #define GLFW_X11_CURSOR_STATE
 #define GLFW_X11_LIBRARY_WINDOW_STATE
 #define GLFW_GLX_CONTEXT_STATE
 #define GLFW_GLX_LIBRARY_CONTEXT_STATE
#endif

#include "null_joystick.h"

#if defined(_GLFW_WIN32)
 #include "win32_joystick.h"
#else
 #define GLFW_WIN32_JOYSTICK_STATE
 #define GLFW_WIN32_LIBRARY_JOYSTICK_STATE
#endif

#if defined(_GLFW_COCOA)
 #include "cocoa_joystick.h"
#else
 #define GLFW_COCOA_JOYSTICK_STATE
 #define GLFW_COCOA_LIBRARY_JOYSTICK_STATE
#endif

#if (defined(_GLFW_X11) || defined(_GLFW_WAYLAND)) && defined(__linux__)
 #define GLFW_BUILD_LINUX_JOYSTICK
#endif

#if defined(GLFW_BUILD_LINUX_JOYSTICK)
 #include "linux_joystick.h"
#else
 #define GLFW_LINUX_JOYSTICK_STATE
 #define GLFW_LINUX_LIBRARY_JOYSTICK_STATE
#endif

#define GLFW_PLATFORM_WINDOW_STATE \
        GLFW_WIN32_WINDOW_STATE \
        GLFW_COCOA_WINDOW_STATE \
        GLFW_WAYLAND_WINDOW_STATE \
        GLFW_X11_WINDOW_STATE \
        GLFW_NULL_WINDOW_STATE \

#define GLFW_PLATFORM_MONITOR_STATE \
        GLFW_WIN32_MONITOR_STATE \
        GLFW_COCOA_MONITOR_STATE \
        GLFW_WAYLAND_MONITOR_STATE \
        GLFW_X11_MONITOR_STATE \
        GLFW_NULL_MONITOR_STATE \

#define GLFW_PLATFORM_CURSOR_STATE \
        GLFW_WIN32_CURSOR_STATE \
        GLFW_COCOA_CURSOR_STATE \
        GLFW_WAYLAND_CURSOR_STATE \
        GLFW_X11_CURSOR_STATE \
        GLFW_NULL_CURSOR_STATE \

#define GLFW_PLATFORM_JOYSTICK_STATE \
        GLFW_WIN32_JOYSTICK_STATE \
        GLFW_COCOA_JOYSTICK_STATE \
        GLFW_LINUX_JOYSTICK_STATE

#define GLFW_PLATFORM_LIBRARY_WINDOW_STATE \
        GLFW_WIN32_LIBRARY_WINDOW_STATE \
        GLFW_COCOA_LIBRARY_WINDOW_STATE \
        GLFW_WAYLAND_LIBRARY_WINDOW_STATE \
        GLFW_X11_LIBRARY_WINDOW_STATE \
        GLFW_NULL_LIBRARY_WINDOW_STATE \

#define GLFW_PLATFORM_LIBRARY_JOYSTICK_STATE \
        GLFW_WIN32_LIBRARY_JOYSTICK_STATE \
        GLFW_COCOA_LIBRARY_JOYSTICK_STATE \
        GLFW_LINUX_LIBRARY_JOYSTICK_STATE

#define GLFW_PLATFORM_CONTEXT_STATE \
        GLFW_WGL_CONTEXT_STATE \
        GLFW_NSGL_CONTEXT_STATE \
        GLFW_GLX_CONTEXT_STATE

#define GLFW_PLATFORM_LIBRARY_CONTEXT_STATE \
        GLFW_WGL_LIBRARY_CONTEXT_STATE \
        GLFW_NSGL_LIBRARY_CONTEXT_STATE \
        GLFW_GLX_LIBRARY_CONTEXT_STATE

#if defined(_WIN32)
 #define GLFW_BUILD_WIN32_THREAD
#else
 #define GLFW_BUILD_POSIX_THREAD
#endif

#if defined(GLFW_BUILD_WIN32_THREAD)
 #include "win32_thread.h"
 #define GLFW_PLATFORM_TLS_STATE    GLFW_WIN32_TLS_STATE
 #define GLFW_PLATFORM_MUTEX_STATE  GLFW_WIN32_MUTEX_STATE
#elif defined(GLFW_BUILD_POSIX_THREAD)
 #include "posix_thread.h"
 #define GLFW_PLATFORM_TLS_STATE    GLFW_POSIX_TLS_STATE
 #define GLFW_PLATFORM_MUTEX_STATE  GLFW_POSIX_MUTEX_STATE
#endif

#if defined(_WIN32)
 #define GLFW_BUILD_WIN32_TIMER
#elif defined(__APPLE__)
 #define GLFW_BUILD_COCOA_TIMER
#else
 #define GLFW_BUILD_POSIX_TIMER
#endif

#if defined(GLFW_BUILD_WIN32_TIMER)
 #include "win32_time.h"
 #define GLFW_PLATFORM_LIBRARY_TIMER_STATE  GLFW_WIN32_LIBRARY_TIMER_STATE
#elif defined(GLFW_BUILD_COCOA_TIMER)
 #include "cocoa_time.h"
 #define GLFW_PLATFORM_LIBRARY_TIMER_STATE  GLFW_COCOA_LIBRARY_TIMER_STATE
#elif defined(GLFW_BUILD_POSIX_TIMER)
 #include "posix_time.h"
 #define GLFW_PLATFORM_LIBRARY_TIMER_STATE  GLFW_POSIX_LIBRARY_TIMER_STATE
#endif

#if defined(_WIN32)
 #define GLFW_BUILD_WIN32_MODULE
#else
 #define GLFW_BUILD_POSIX_MODULE
#endif

#if defined(_GLFW_WAYLAND) || defined(_GLFW_X11)
 #define GLFW_BUILD_POSIX_POLL
#endif

