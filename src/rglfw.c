/**********************************************************************************************
*
*   raylib GLFW single file compilation
*
*   This file includes GLFW sources to be compiled together with raylib for all supported
*   platforms, this way, no external dependencies are required.
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2017 Ramon Santamaria (@raysan5)
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

#include "external/glfw/src/context.c"
#include "external/glfw/src/init.c"
#include "external/glfw/src/input.c"
#include "external/glfw/src/monitor.c"
#include "external/glfw/src/vulkan.c"
#include "external/glfw/src/window.c"

// Required compilation defines: -D_GLFW_WIN32
#ifdef _WIN32
#include "external/glfw/src/win32_init.c"
#include "external/glfw/src/win32_joystick.c"
#include "external/glfw/src/win32_monitor.c"
#include "external/glfw/src/win32_time.c"
#include "external/glfw/src/win32_thread.c"
#include "external/glfw/src/win32_window.c"
#include "external/glfw/src/wgl_context.c"
#include "external/glfw/src/egl_context.c"
#include "external/glfw/src/osmesa_context.c"
#endif

// Required compilation defines: -D_GLFW_X11
#ifdef __linux__
#include "external/glfw/src/x11_init.c"
#include "external/glfw/src/x11_monitor.c"
#include "external/glfw/src/x11_window.c"
#include "external/glfw/src/xkb_unicode.c"
#include "external/glfw/src/linux_joystick.c"
#include "external/glfw/src/posix_time.c"
#include "external/glfw/src/posix_thread.c"
#include "external/glfw/src/glx_context.c"
#include "external/glfw/src/egl_context.c"
#include "external/glfw/src/osmesa_context.c"
#endif

// Required compilation defines: -D_GLFW_COCOA -D_GLFW_USE_CHDIR -D_GLFW_USE_MENUBAR -D_GLFW_USE_RETINA
#ifdef __APPLE__
#include "external/glfw/src/cocoa_init.m"
#include "external/glfw/src/cocoa_joystick.m"
#include "external/glfw/src/cocoa_monitor.m"
#include "external/glfw/src/cocoa_window.m"
#include "external/glfw/src/cocoa_time.c"
#include "external/glfw/src/posix_thread.c"
#include "external/glfw/src/nsgl_context.m" 
#include "external/glfw/src/egl_context.c" 
#include "external/glfw/src/osmesa_context.c.m"
#endif
