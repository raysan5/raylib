//========================================================================
// GLFW 3.3 Mir - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2014-2017 Brandon Schaefer <brandon.schaefer@canonical.com>
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

#include <sys/queue.h>
#include <pthread.h>
#include <dlfcn.h>

#include <mir_toolkit/mir_client_library.h>

typedef VkFlags VkMirWindowCreateFlagsKHR;

typedef struct VkMirWindowCreateInfoKHR
{
    VkStructureType             sType;
    const void*                 pNext;
    VkMirWindowCreateFlagsKHR   flags;
    MirConnection*              connection;
    MirWindow*                  mirWindow;
} VkMirWindowCreateInfoKHR;

typedef VkResult (APIENTRY *PFN_vkCreateMirWindowKHR)(VkInstance,const VkMirWindowCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
typedef VkBool32 (APIENTRY *PFN_vkGetPhysicalDeviceMirPresentationSupportKHR)(VkPhysicalDevice,uint32_t,MirConnection*);

#include "posix_thread.h"
#include "posix_time.h"
#include "linux_joystick.h"
#include "xkb_unicode.h"
#include "egl_context.h"
#include "osmesa_context.h"

#define _glfw_dlopen(name) dlopen(name, RTLD_LAZY | RTLD_LOCAL)
#define _glfw_dlclose(handle) dlclose(handle)
#define _glfw_dlsym(handle, name) dlsym(handle, name)

#define _GLFW_EGL_NATIVE_WINDOW  ((EGLNativeWindowType) window->mir.nativeWindow)
#define _GLFW_EGL_NATIVE_DISPLAY ((EGLNativeDisplayType) _glfw.mir.display)

#define _GLFW_PLATFORM_WINDOW_STATE         _GLFWwindowMir  mir
#define _GLFW_PLATFORM_MONITOR_STATE        _GLFWmonitorMir mir
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE _GLFWlibraryMir mir
#define _GLFW_PLATFORM_CURSOR_STATE         _GLFWcursorMir  mir

#define _GLFW_PLATFORM_CONTEXT_STATE
#define _GLFW_PLATFORM_LIBRARY_CONTEXT_STATE


// Mir-specific Event Queue
//
typedef struct EventQueue
{
    TAILQ_HEAD(, EventNode) head;
} EventQueue;

// Mir-specific per-window data
//
typedef struct _GLFWwindowMir
{
    MirWindow*              window;
    int                     width;
    int                     height;
    MirEGLNativeWindowType  nativeWindow;
    _GLFWcursor*            currentCursor;

} _GLFWwindowMir;

// Mir-specific per-monitor data
//
typedef struct _GLFWmonitorMir
{
    int curMode;
    int outputId;
    int x;
    int y;

} _GLFWmonitorMir;

// Mir-specific global data
//
typedef struct _GLFWlibraryMir
{
    MirConnection*          connection;
    MirEGLNativeDisplayType display;
    EventQueue* eventQueue;

    short int keycodes[256];
    short int scancodes[GLFW_KEY_LAST + 1];

    pthread_mutex_t eventMutex;
    pthread_cond_t  eventCond;

    // The window whose disabled cursor mode is active
    _GLFWwindow* disabledCursorWindow;

} _GLFWlibraryMir;

// Mir-specific per-cursor data
// TODO: Only system cursors are implemented in Mir atm. Need to wait for support.
//
typedef struct _GLFWcursorMir
{
    MirCursorConfiguration* conf;
    MirBufferStream*        customCursor;
    char const*             cursorName; // only needed for system cursors
} _GLFWcursorMir;


extern void _glfwPollMonitorsMir(void);
extern void _glfwInitEventQueueMir(EventQueue* queue);
extern void _glfwDeleteEventQueueMir(EventQueue* queue);

