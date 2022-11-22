//========================================================================
// GLFW 3.4 Wayland - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2014 Jonas Ådahl <jadahl@gmail.com>
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
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#include "internal.h"

#include <errno.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <time.h>

#include "wayland-client-protocol.h"
#include "wayland-xdg-shell-client-protocol.h"
#include "wayland-xdg-decoration-client-protocol.h"
#include "wayland-viewporter-client-protocol.h"
#include "wayland-relative-pointer-unstable-v1-client-protocol.h"
#include "wayland-pointer-constraints-unstable-v1-client-protocol.h"
#include "wayland-idle-inhibit-unstable-v1-client-protocol.h"

// NOTE: Versions of wayland-scanner prior to 1.17.91 named every global array of
//       wl_interface pointers 'types', making it impossible to combine several unmodified
//       private-code files into a single compilation unit
// HACK: We override this name with a macro for each file, allowing them to coexist

#define types _glfw_wayland_types
#include "wayland-client-protocol-code.h"
#undef types

#define types _glfw_xdg_shell_types
#include "wayland-xdg-shell-client-protocol-code.h"
#undef types

#define types _glfw_xdg_decoration_types
#include "wayland-xdg-decoration-client-protocol-code.h"
#undef types

#define types _glfw_viewporter_types
#include "wayland-viewporter-client-protocol-code.h"
#undef types

#define types _glfw_relative_pointer_types
#include "wayland-relative-pointer-unstable-v1-client-protocol-code.h"
#undef types

#define types _glfw_pointer_constraints_types
#include "wayland-pointer-constraints-unstable-v1-client-protocol-code.h"
#undef types

#define types _glfw_idle_inhibit_types
#include "wayland-idle-inhibit-unstable-v1-client-protocol-code.h"
#undef types

static void wmBaseHandlePing(void* userData,
                             struct xdg_wm_base* wmBase,
                             uint32_t serial)
{
    xdg_wm_base_pong(wmBase, serial);
}

static const struct xdg_wm_base_listener wmBaseListener =
{
    wmBaseHandlePing
};

static void registryHandleGlobal(void* userData,
                                 struct wl_registry* registry,
                                 uint32_t name,
                                 const char* interface,
                                 uint32_t version)
{
    if (strcmp(interface, "wl_compositor") == 0)
    {
        _glfw.wl.compositorVersion = _glfw_min(3, version);
        _glfw.wl.compositor =
            wl_registry_bind(registry, name, &wl_compositor_interface,
                             _glfw.wl.compositorVersion);
    }
    else if (strcmp(interface, "wl_subcompositor") == 0)
    {
        _glfw.wl.subcompositor =
            wl_registry_bind(registry, name, &wl_subcompositor_interface, 1);
    }
    else if (strcmp(interface, "wl_shm") == 0)
    {
        _glfw.wl.shm =
            wl_registry_bind(registry, name, &wl_shm_interface, 1);
    }
    else if (strcmp(interface, "wl_output") == 0)
    {
        _glfwAddOutputWayland(name, version);
    }
    else if (strcmp(interface, "wl_seat") == 0)
    {
        if (!_glfw.wl.seat)
        {
            _glfw.wl.seatVersion = _glfw_min(4, version);
            _glfw.wl.seat =
                wl_registry_bind(registry, name, &wl_seat_interface,
                                 _glfw.wl.seatVersion);
            _glfwAddSeatListenerWayland(_glfw.wl.seat);
        }
    }
    else if (strcmp(interface, "wl_data_device_manager") == 0)
    {
        if (!_glfw.wl.dataDeviceManager)
        {
            _glfw.wl.dataDeviceManager =
                wl_registry_bind(registry, name,
                                 &wl_data_device_manager_interface, 1);
        }
    }
    else if (strcmp(interface, "xdg_wm_base") == 0)
    {
        _glfw.wl.wmBase =
            wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(_glfw.wl.wmBase, &wmBaseListener, NULL);
    }
    else if (strcmp(interface, "zxdg_decoration_manager_v1") == 0)
    {
        _glfw.wl.decorationManager =
            wl_registry_bind(registry, name,
                             &zxdg_decoration_manager_v1_interface,
                             1);
    }
    else if (strcmp(interface, "wp_viewporter") == 0)
    {
        _glfw.wl.viewporter =
            wl_registry_bind(registry, name, &wp_viewporter_interface, 1);
    }
    else if (strcmp(interface, "zwp_relative_pointer_manager_v1") == 0)
    {
        _glfw.wl.relativePointerManager =
            wl_registry_bind(registry, name,
                             &zwp_relative_pointer_manager_v1_interface,
                             1);
    }
    else if (strcmp(interface, "zwp_pointer_constraints_v1") == 0)
    {
        _glfw.wl.pointerConstraints =
            wl_registry_bind(registry, name,
                             &zwp_pointer_constraints_v1_interface,
                             1);
    }
    else if (strcmp(interface, "zwp_idle_inhibit_manager_v1") == 0)
    {
        _glfw.wl.idleInhibitManager =
            wl_registry_bind(registry, name,
                             &zwp_idle_inhibit_manager_v1_interface,
                             1);
    }
}

static void registryHandleGlobalRemove(void* userData,
                                       struct wl_registry* registry,
                                       uint32_t name)
{
    for (int i = 0; i < _glfw.monitorCount; ++i)
    {
        _GLFWmonitor* monitor = _glfw.monitors[i];
        if (monitor->wl.name == name)
        {
            _glfwInputMonitor(monitor, GLFW_DISCONNECTED, 0);
            return;
        }
    }
}


static const struct wl_registry_listener registryListener =
{
    registryHandleGlobal,
    registryHandleGlobalRemove
};

// Create key code translation tables
//
static void createKeyTables(void)
{
    memset(_glfw.wl.keycodes, -1, sizeof(_glfw.wl.keycodes));
    memset(_glfw.wl.scancodes, -1, sizeof(_glfw.wl.scancodes));

    _glfw.wl.keycodes[KEY_GRAVE]      = GLFW_KEY_GRAVE_ACCENT;
    _glfw.wl.keycodes[KEY_1]          = GLFW_KEY_1;
    _glfw.wl.keycodes[KEY_2]          = GLFW_KEY_2;
    _glfw.wl.keycodes[KEY_3]          = GLFW_KEY_3;
    _glfw.wl.keycodes[KEY_4]          = GLFW_KEY_4;
    _glfw.wl.keycodes[KEY_5]          = GLFW_KEY_5;
    _glfw.wl.keycodes[KEY_6]          = GLFW_KEY_6;
    _glfw.wl.keycodes[KEY_7]          = GLFW_KEY_7;
    _glfw.wl.keycodes[KEY_8]          = GLFW_KEY_8;
    _glfw.wl.keycodes[KEY_9]          = GLFW_KEY_9;
    _glfw.wl.keycodes[KEY_0]          = GLFW_KEY_0;
    _glfw.wl.keycodes[KEY_SPACE]      = GLFW_KEY_SPACE;
    _glfw.wl.keycodes[KEY_MINUS]      = GLFW_KEY_MINUS;
    _glfw.wl.keycodes[KEY_EQUAL]      = GLFW_KEY_EQUAL;
    _glfw.wl.keycodes[KEY_Q]          = GLFW_KEY_Q;
    _glfw.wl.keycodes[KEY_W]          = GLFW_KEY_W;
    _glfw.wl.keycodes[KEY_E]          = GLFW_KEY_E;
    _glfw.wl.keycodes[KEY_R]          = GLFW_KEY_R;
    _glfw.wl.keycodes[KEY_T]          = GLFW_KEY_T;
    _glfw.wl.keycodes[KEY_Y]          = GLFW_KEY_Y;
    _glfw.wl.keycodes[KEY_U]          = GLFW_KEY_U;
    _glfw.wl.keycodes[KEY_I]          = GLFW_KEY_I;
    _glfw.wl.keycodes[KEY_O]          = GLFW_KEY_O;
    _glfw.wl.keycodes[KEY_P]          = GLFW_KEY_P;
    _glfw.wl.keycodes[KEY_LEFTBRACE]  = GLFW_KEY_LEFT_BRACKET;
    _glfw.wl.keycodes[KEY_RIGHTBRACE] = GLFW_KEY_RIGHT_BRACKET;
    _glfw.wl.keycodes[KEY_A]          = GLFW_KEY_A;
    _glfw.wl.keycodes[KEY_S]          = GLFW_KEY_S;
    _glfw.wl.keycodes[KEY_D]          = GLFW_KEY_D;
    _glfw.wl.keycodes[KEY_F]          = GLFW_KEY_F;
    _glfw.wl.keycodes[KEY_G]          = GLFW_KEY_G;
    _glfw.wl.keycodes[KEY_H]          = GLFW_KEY_H;
    _glfw.wl.keycodes[KEY_J]          = GLFW_KEY_J;
    _glfw.wl.keycodes[KEY_K]          = GLFW_KEY_K;
    _glfw.wl.keycodes[KEY_L]          = GLFW_KEY_L;
    _glfw.wl.keycodes[KEY_SEMICOLON]  = GLFW_KEY_SEMICOLON;
    _glfw.wl.keycodes[KEY_APOSTROPHE] = GLFW_KEY_APOSTROPHE;
    _glfw.wl.keycodes[KEY_Z]          = GLFW_KEY_Z;
    _glfw.wl.keycodes[KEY_X]          = GLFW_KEY_X;
    _glfw.wl.keycodes[KEY_C]          = GLFW_KEY_C;
    _glfw.wl.keycodes[KEY_V]          = GLFW_KEY_V;
    _glfw.wl.keycodes[KEY_B]          = GLFW_KEY_B;
    _glfw.wl.keycodes[KEY_N]          = GLFW_KEY_N;
    _glfw.wl.keycodes[KEY_M]          = GLFW_KEY_M;
    _glfw.wl.keycodes[KEY_COMMA]      = GLFW_KEY_COMMA;
    _glfw.wl.keycodes[KEY_DOT]        = GLFW_KEY_PERIOD;
    _glfw.wl.keycodes[KEY_SLASH]      = GLFW_KEY_SLASH;
    _glfw.wl.keycodes[KEY_BACKSLASH]  = GLFW_KEY_BACKSLASH;
    _glfw.wl.keycodes[KEY_ESC]        = GLFW_KEY_ESCAPE;
    _glfw.wl.keycodes[KEY_TAB]        = GLFW_KEY_TAB;
    _glfw.wl.keycodes[KEY_LEFTSHIFT]  = GLFW_KEY_LEFT_SHIFT;
    _glfw.wl.keycodes[KEY_RIGHTSHIFT] = GLFW_KEY_RIGHT_SHIFT;
    _glfw.wl.keycodes[KEY_LEFTCTRL]   = GLFW_KEY_LEFT_CONTROL;
    _glfw.wl.keycodes[KEY_RIGHTCTRL]  = GLFW_KEY_RIGHT_CONTROL;
    _glfw.wl.keycodes[KEY_LEFTALT]    = GLFW_KEY_LEFT_ALT;
    _glfw.wl.keycodes[KEY_RIGHTALT]   = GLFW_KEY_RIGHT_ALT;
    _glfw.wl.keycodes[KEY_LEFTMETA]   = GLFW_KEY_LEFT_SUPER;
    _glfw.wl.keycodes[KEY_RIGHTMETA]  = GLFW_KEY_RIGHT_SUPER;
    _glfw.wl.keycodes[KEY_COMPOSE]    = GLFW_KEY_MENU;
    _glfw.wl.keycodes[KEY_NUMLOCK]    = GLFW_KEY_NUM_LOCK;
    _glfw.wl.keycodes[KEY_CAPSLOCK]   = GLFW_KEY_CAPS_LOCK;
    _glfw.wl.keycodes[KEY_PRINT]      = GLFW_KEY_PRINT_SCREEN;
    _glfw.wl.keycodes[KEY_SCROLLLOCK] = GLFW_KEY_SCROLL_LOCK;
    _glfw.wl.keycodes[KEY_PAUSE]      = GLFW_KEY_PAUSE;
    _glfw.wl.keycodes[KEY_DELETE]     = GLFW_KEY_DELETE;
    _glfw.wl.keycodes[KEY_BACKSPACE]  = GLFW_KEY_BACKSPACE;
    _glfw.wl.keycodes[KEY_ENTER]      = GLFW_KEY_ENTER;
    _glfw.wl.keycodes[KEY_HOME]       = GLFW_KEY_HOME;
    _glfw.wl.keycodes[KEY_END]        = GLFW_KEY_END;
    _glfw.wl.keycodes[KEY_PAGEUP]     = GLFW_KEY_PAGE_UP;
    _glfw.wl.keycodes[KEY_PAGEDOWN]   = GLFW_KEY_PAGE_DOWN;
    _glfw.wl.keycodes[KEY_INSERT]     = GLFW_KEY_INSERT;
    _glfw.wl.keycodes[KEY_LEFT]       = GLFW_KEY_LEFT;
    _glfw.wl.keycodes[KEY_RIGHT]      = GLFW_KEY_RIGHT;
    _glfw.wl.keycodes[KEY_DOWN]       = GLFW_KEY_DOWN;
    _glfw.wl.keycodes[KEY_UP]         = GLFW_KEY_UP;
    _glfw.wl.keycodes[KEY_F1]         = GLFW_KEY_F1;
    _glfw.wl.keycodes[KEY_F2]         = GLFW_KEY_F2;
    _glfw.wl.keycodes[KEY_F3]         = GLFW_KEY_F3;
    _glfw.wl.keycodes[KEY_F4]         = GLFW_KEY_F4;
    _glfw.wl.keycodes[KEY_F5]         = GLFW_KEY_F5;
    _glfw.wl.keycodes[KEY_F6]         = GLFW_KEY_F6;
    _glfw.wl.keycodes[KEY_F7]         = GLFW_KEY_F7;
    _glfw.wl.keycodes[KEY_F8]         = GLFW_KEY_F8;
    _glfw.wl.keycodes[KEY_F9]         = GLFW_KEY_F9;
    _glfw.wl.keycodes[KEY_F10]        = GLFW_KEY_F10;
    _glfw.wl.keycodes[KEY_F11]        = GLFW_KEY_F11;
    _glfw.wl.keycodes[KEY_F12]        = GLFW_KEY_F12;
    _glfw.wl.keycodes[KEY_F13]        = GLFW_KEY_F13;
    _glfw.wl.keycodes[KEY_F14]        = GLFW_KEY_F14;
    _glfw.wl.keycodes[KEY_F15]        = GLFW_KEY_F15;
    _glfw.wl.keycodes[KEY_F16]        = GLFW_KEY_F16;
    _glfw.wl.keycodes[KEY_F17]        = GLFW_KEY_F17;
    _glfw.wl.keycodes[KEY_F18]        = GLFW_KEY_F18;
    _glfw.wl.keycodes[KEY_F19]        = GLFW_KEY_F19;
    _glfw.wl.keycodes[KEY_F20]        = GLFW_KEY_F20;
    _glfw.wl.keycodes[KEY_F21]        = GLFW_KEY_F21;
    _glfw.wl.keycodes[KEY_F22]        = GLFW_KEY_F22;
    _glfw.wl.keycodes[KEY_F23]        = GLFW_KEY_F23;
    _glfw.wl.keycodes[KEY_F24]        = GLFW_KEY_F24;
    _glfw.wl.keycodes[KEY_KPSLASH]    = GLFW_KEY_KP_DIVIDE;
    _glfw.wl.keycodes[KEY_KPASTERISK] = GLFW_KEY_KP_MULTIPLY;
    _glfw.wl.keycodes[KEY_KPMINUS]    = GLFW_KEY_KP_SUBTRACT;
    _glfw.wl.keycodes[KEY_KPPLUS]     = GLFW_KEY_KP_ADD;
    _glfw.wl.keycodes[KEY_KP0]        = GLFW_KEY_KP_0;
    _glfw.wl.keycodes[KEY_KP1]        = GLFW_KEY_KP_1;
    _glfw.wl.keycodes[KEY_KP2]        = GLFW_KEY_KP_2;
    _glfw.wl.keycodes[KEY_KP3]        = GLFW_KEY_KP_3;
    _glfw.wl.keycodes[KEY_KP4]        = GLFW_KEY_KP_4;
    _glfw.wl.keycodes[KEY_KP5]        = GLFW_KEY_KP_5;
    _glfw.wl.keycodes[KEY_KP6]        = GLFW_KEY_KP_6;
    _glfw.wl.keycodes[KEY_KP7]        = GLFW_KEY_KP_7;
    _glfw.wl.keycodes[KEY_KP8]        = GLFW_KEY_KP_8;
    _glfw.wl.keycodes[KEY_KP9]        = GLFW_KEY_KP_9;
    _glfw.wl.keycodes[KEY_KPDOT]      = GLFW_KEY_KP_DECIMAL;
    _glfw.wl.keycodes[KEY_KPEQUAL]    = GLFW_KEY_KP_EQUAL;
    _glfw.wl.keycodes[KEY_KPENTER]    = GLFW_KEY_KP_ENTER;
    _glfw.wl.keycodes[KEY_102ND]      = GLFW_KEY_WORLD_2;

    for (int scancode = 0;  scancode < 256;  scancode++)
    {
        if (_glfw.wl.keycodes[scancode] > 0)
            _glfw.wl.scancodes[_glfw.wl.keycodes[scancode]] = scancode;
    }
}

static GLFWbool loadCursorTheme(void)
{
    int cursorSize = 32;

    const char* sizeString = getenv("XCURSOR_SIZE");
    if (sizeString)
    {
        errno = 0;
        const long cursorSizeLong = strtol(sizeString, NULL, 10);
        if (errno == 0 && cursorSizeLong > 0 && cursorSizeLong < INT_MAX)
            cursorSize = (int) cursorSizeLong;
    }

    const char* themeName = getenv("XCURSOR_THEME");

    _glfw.wl.cursorTheme = wl_cursor_theme_load(themeName, cursorSize, _glfw.wl.shm);
    if (!_glfw.wl.cursorTheme)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to load default cursor theme");
        return GLFW_FALSE;
    }

    // If this happens to be NULL, we just fallback to the scale=1 version.
    _glfw.wl.cursorThemeHiDPI =
        wl_cursor_theme_load(themeName, cursorSize * 2, _glfw.wl.shm);

    _glfw.wl.cursorSurface = wl_compositor_create_surface(_glfw.wl.compositor);
    _glfw.wl.cursorTimerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    return GLFW_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwConnectWayland(int platformID, _GLFWplatform* platform)
{
    const _GLFWplatform wayland =
    {
        GLFW_PLATFORM_WAYLAND,
        _glfwInitWayland,
        _glfwTerminateWayland,
        _glfwGetCursorPosWayland,
        _glfwSetCursorPosWayland,
        _glfwSetCursorModeWayland,
        _glfwSetRawMouseMotionWayland,
        _glfwRawMouseMotionSupportedWayland,
        _glfwCreateCursorWayland,
        _glfwCreateStandardCursorWayland,
        _glfwDestroyCursorWayland,
        _glfwSetCursorWayland,
        _glfwGetScancodeNameWayland,
        _glfwGetKeyScancodeWayland,
        _glfwSetClipboardStringWayland,
        _glfwGetClipboardStringWayland,
#if defined(__linux__)
        _glfwInitJoysticksLinux,
        _glfwTerminateJoysticksLinux,
        _glfwPollJoystickLinux,
        _glfwGetMappingNameLinux,
        _glfwUpdateGamepadGUIDLinux,
#else
        _glfwInitJoysticksNull,
        _glfwTerminateJoysticksNull,
        _glfwPollJoystickNull,
        _glfwGetMappingNameNull,
        _glfwUpdateGamepadGUIDNull,
#endif
        _glfwFreeMonitorWayland,
        _glfwGetMonitorPosWayland,
        _glfwGetMonitorContentScaleWayland,
        _glfwGetMonitorWorkareaWayland,
        _glfwGetVideoModesWayland,
        _glfwGetVideoModeWayland,
        _glfwGetGammaRampWayland,
        _glfwSetGammaRampWayland,
        _glfwCreateWindowWayland,
        _glfwDestroyWindowWayland,
        _glfwSetWindowTitleWayland,
        _glfwSetWindowIconWayland,
        _glfwGetWindowPosWayland,
        _glfwSetWindowPosWayland,
        _glfwGetWindowSizeWayland,
        _glfwSetWindowSizeWayland,
        _glfwSetWindowSizeLimitsWayland,
        _glfwSetWindowAspectRatioWayland,
        _glfwGetFramebufferSizeWayland,
        _glfwGetWindowFrameSizeWayland,
        _glfwGetWindowContentScaleWayland,
        _glfwIconifyWindowWayland,
        _glfwRestoreWindowWayland,
        _glfwMaximizeWindowWayland,
        _glfwShowWindowWayland,
        _glfwHideWindowWayland,
        _glfwRequestWindowAttentionWayland,
        _glfwFocusWindowWayland,
        _glfwSetWindowMonitorWayland,
        _glfwWindowFocusedWayland,
        _glfwWindowIconifiedWayland,
        _glfwWindowVisibleWayland,
        _glfwWindowMaximizedWayland,
        _glfwWindowHoveredWayland,
        _glfwFramebufferTransparentWayland,
        _glfwGetWindowOpacityWayland,
        _glfwSetWindowResizableWayland,
        _glfwSetWindowDecoratedWayland,
        _glfwSetWindowFloatingWayland,
        _glfwSetWindowOpacityWayland,
        _glfwSetWindowMousePassthroughWayland,
        _glfwPollEventsWayland,
        _glfwWaitEventsWayland,
        _glfwWaitEventsTimeoutWayland,
        _glfwPostEmptyEventWayland,
        _glfwGetEGLPlatformWayland,
        _glfwGetEGLNativeDisplayWayland,
        _glfwGetEGLNativeWindowWayland,
        _glfwGetRequiredInstanceExtensionsWayland,
        _glfwGetPhysicalDevicePresentationSupportWayland,
        _glfwCreateWindowSurfaceWayland,
    };

    void* module = _glfwPlatformLoadModule("libwayland-client.so.0");
    if (!module)
    {
        if (platformID == GLFW_PLATFORM_WAYLAND)
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "Wayland: Failed to load libwayland-client");
        }

        return GLFW_FALSE;
    }

    PFN_wl_display_connect wl_display_connect = (PFN_wl_display_connect)
        _glfwPlatformGetModuleSymbol(module, "wl_display_connect");
    if (!wl_display_connect)
    {
        if (platformID == GLFW_PLATFORM_WAYLAND)
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "Wayland: Failed to load libwayland-client entry point");
        }

        _glfwPlatformFreeModule(module);
        return GLFW_FALSE;
    }

    struct wl_display* display = wl_display_connect(NULL);
    if (!display)
    {
        if (platformID == GLFW_PLATFORM_WAYLAND)
            _glfwInputError(GLFW_PLATFORM_ERROR, "Wayland: Failed to connect to display");

        _glfwPlatformFreeModule(module);
        return GLFW_FALSE;
    }

    _glfw.wl.display = display;
    _glfw.wl.client.handle = module;

    *platform = wayland;
    return GLFW_TRUE;
}

int _glfwInitWayland(void)
{
    // These must be set before any failure checks
    _glfw.wl.keyRepeatTimerfd = -1;
    _glfw.wl.cursorTimerfd = -1;

    _glfw.wl.client.display_flush = (PFN_wl_display_flush)
        _glfwPlatformGetModuleSymbol(_glfw.wl.client.handle, "wl_display_flush");
    _glfw.wl.client.display_cancel_read = (PFN_wl_display_cancel_read)
        _glfwPlatformGetModuleSymbol(_glfw.wl.client.handle, "wl_display_cancel_read");
    _glfw.wl.client.display_dispatch_pending = (PFN_wl_display_dispatch_pending)
        _glfwPlatformGetModuleSymbol(_glfw.wl.client.handle, "wl_display_dispatch_pending");
    _glfw.wl.client.display_read_events = (PFN_wl_display_read_events)
        _glfwPlatformGetModuleSymbol(_glfw.wl.client.handle, "wl_display_read_events");
    _glfw.wl.client.display_disconnect = (PFN_wl_display_disconnect)
        _glfwPlatformGetModuleSymbol(_glfw.wl.client.handle, "wl_display_disconnect");
    _glfw.wl.client.display_roundtrip = (PFN_wl_display_roundtrip)
        _glfwPlatformGetModuleSymbol(_glfw.wl.client.handle, "wl_display_roundtrip");
    _glfw.wl.client.display_get_fd = (PFN_wl_display_get_fd)
        _glfwPlatformGetModuleSymbol(_glfw.wl.client.handle, "wl_display_get_fd");
    _glfw.wl.client.display_prepare_read = (PFN_wl_display_prepare_read)
        _glfwPlatformGetModuleSymbol(_glfw.wl.client.handle, "wl_display_prepare_read");
    _glfw.wl.client.proxy_marshal = (PFN_wl_proxy_marshal)
        _glfwPlatformGetModuleSymbol(_glfw.wl.client.handle, "wl_proxy_marshal");
    _glfw.wl.client.proxy_add_listener = (PFN_wl_proxy_add_listener)
        _glfwPlatformGetModuleSymbol(_glfw.wl.client.handle, "wl_proxy_add_listener");
    _glfw.wl.client.proxy_destroy = (PFN_wl_proxy_destroy)
        _glfwPlatformGetModuleSymbol(_glfw.wl.client.handle, "wl_proxy_destroy");
    _glfw.wl.client.proxy_marshal_constructor = (PFN_wl_proxy_marshal_constructor)
        _glfwPlatformGetModuleSymbol(_glfw.wl.client.handle, "wl_proxy_marshal_constructor");
    _glfw.wl.client.proxy_marshal_constructor_versioned = (PFN_wl_proxy_marshal_constructor_versioned)
        _glfwPlatformGetModuleSymbol(_glfw.wl.client.handle, "wl_proxy_marshal_constructor_versioned");
    _glfw.wl.client.proxy_get_user_data = (PFN_wl_proxy_get_user_data)
        _glfwPlatformGetModuleSymbol(_glfw.wl.client.handle, "wl_proxy_get_user_data");
    _glfw.wl.client.proxy_set_user_data = (PFN_wl_proxy_set_user_data)
        _glfwPlatformGetModuleSymbol(_glfw.wl.client.handle, "wl_proxy_set_user_data");
    _glfw.wl.client.proxy_get_version = (PFN_wl_proxy_get_version)
        _glfwPlatformGetModuleSymbol(_glfw.wl.client.handle, "wl_proxy_get_version");
    _glfw.wl.client.proxy_marshal_flags = (PFN_wl_proxy_marshal_flags)
        _glfwPlatformGetModuleSymbol(_glfw.wl.client.handle, "wl_proxy_marshal_flags");

    if (!_glfw.wl.client.display_flush ||
        !_glfw.wl.client.display_cancel_read ||
        !_glfw.wl.client.display_dispatch_pending ||
        !_glfw.wl.client.display_read_events ||
        !_glfw.wl.client.display_disconnect ||
        !_glfw.wl.client.display_roundtrip ||
        !_glfw.wl.client.display_get_fd ||
        !_glfw.wl.client.display_prepare_read ||
        !_glfw.wl.client.proxy_marshal ||
        !_glfw.wl.client.proxy_add_listener ||
        !_glfw.wl.client.proxy_destroy ||
        !_glfw.wl.client.proxy_marshal_constructor ||
        !_glfw.wl.client.proxy_marshal_constructor_versioned ||
        !_glfw.wl.client.proxy_get_user_data ||
        !_glfw.wl.client.proxy_set_user_data)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to load libwayland-client entry point");
        return GLFW_FALSE;
    }

    _glfw.wl.cursor.handle = _glfwPlatformLoadModule("libwayland-cursor.so.0");
    if (!_glfw.wl.cursor.handle)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to load libwayland-cursor");
        return GLFW_FALSE;
    }

    _glfw.wl.cursor.theme_load = (PFN_wl_cursor_theme_load)
        _glfwPlatformGetModuleSymbol(_glfw.wl.cursor.handle, "wl_cursor_theme_load");
    _glfw.wl.cursor.theme_destroy = (PFN_wl_cursor_theme_destroy)
        _glfwPlatformGetModuleSymbol(_glfw.wl.cursor.handle, "wl_cursor_theme_destroy");
    _glfw.wl.cursor.theme_get_cursor = (PFN_wl_cursor_theme_get_cursor)
        _glfwPlatformGetModuleSymbol(_glfw.wl.cursor.handle, "wl_cursor_theme_get_cursor");
    _glfw.wl.cursor.image_get_buffer = (PFN_wl_cursor_image_get_buffer)
        _glfwPlatformGetModuleSymbol(_glfw.wl.cursor.handle, "wl_cursor_image_get_buffer");

    _glfw.wl.egl.handle = _glfwPlatformLoadModule("libwayland-egl.so.1");
    if (!_glfw.wl.egl.handle)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to load libwayland-egl");
        return GLFW_FALSE;
    }

    _glfw.wl.egl.window_create = (PFN_wl_egl_window_create)
        _glfwPlatformGetModuleSymbol(_glfw.wl.egl.handle, "wl_egl_window_create");
    _glfw.wl.egl.window_destroy = (PFN_wl_egl_window_destroy)
        _glfwPlatformGetModuleSymbol(_glfw.wl.egl.handle, "wl_egl_window_destroy");
    _glfw.wl.egl.window_resize = (PFN_wl_egl_window_resize)
        _glfwPlatformGetModuleSymbol(_glfw.wl.egl.handle, "wl_egl_window_resize");

    _glfw.wl.xkb.handle = _glfwPlatformLoadModule("libxkbcommon.so.0");
    if (!_glfw.wl.xkb.handle)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to load libxkbcommon");
        return GLFW_FALSE;
    }

    _glfw.wl.xkb.context_new = (PFN_xkb_context_new)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_context_new");
    _glfw.wl.xkb.context_unref = (PFN_xkb_context_unref)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_context_unref");
    _glfw.wl.xkb.keymap_new_from_string = (PFN_xkb_keymap_new_from_string)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_keymap_new_from_string");
    _glfw.wl.xkb.keymap_unref = (PFN_xkb_keymap_unref)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_keymap_unref");
    _glfw.wl.xkb.keymap_mod_get_index = (PFN_xkb_keymap_mod_get_index)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_keymap_mod_get_index");
    _glfw.wl.xkb.keymap_key_repeats = (PFN_xkb_keymap_key_repeats)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_keymap_key_repeats");
    _glfw.wl.xkb.keymap_key_get_syms_by_level = (PFN_xkb_keymap_key_get_syms_by_level)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_keymap_key_get_syms_by_level");
    _glfw.wl.xkb.state_new = (PFN_xkb_state_new)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_state_new");
    _glfw.wl.xkb.state_unref = (PFN_xkb_state_unref)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_state_unref");
    _glfw.wl.xkb.state_key_get_syms = (PFN_xkb_state_key_get_syms)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_state_key_get_syms");
    _glfw.wl.xkb.state_update_mask = (PFN_xkb_state_update_mask)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_state_update_mask");
    _glfw.wl.xkb.state_key_get_layout = (PFN_xkb_state_key_get_layout)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_state_key_get_layout");
    _glfw.wl.xkb.state_mod_index_is_active = (PFN_xkb_state_mod_index_is_active)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_state_mod_index_is_active");
    _glfw.wl.xkb.compose_table_new_from_locale = (PFN_xkb_compose_table_new_from_locale)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_compose_table_new_from_locale");
    _glfw.wl.xkb.compose_table_unref = (PFN_xkb_compose_table_unref)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_compose_table_unref");
    _glfw.wl.xkb.compose_state_new = (PFN_xkb_compose_state_new)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_compose_state_new");
    _glfw.wl.xkb.compose_state_unref = (PFN_xkb_compose_state_unref)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_compose_state_unref");
    _glfw.wl.xkb.compose_state_feed = (PFN_xkb_compose_state_feed)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_compose_state_feed");
    _glfw.wl.xkb.compose_state_get_status = (PFN_xkb_compose_state_get_status)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_compose_state_get_status");
    _glfw.wl.xkb.compose_state_get_one_sym = (PFN_xkb_compose_state_get_one_sym)
        _glfwPlatformGetModuleSymbol(_glfw.wl.xkb.handle, "xkb_compose_state_get_one_sym");

    _glfw.wl.registry = wl_display_get_registry(_glfw.wl.display);
    wl_registry_add_listener(_glfw.wl.registry, &registryListener, NULL);

    createKeyTables();

    _glfw.wl.xkb.context = xkb_context_new(0);
    if (!_glfw.wl.xkb.context)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to initialize xkb context");
        return GLFW_FALSE;
    }

    // Sync so we got all registry objects
    wl_display_roundtrip(_glfw.wl.display);

    // Sync so we got all initial output events
    wl_display_roundtrip(_glfw.wl.display);

#ifdef WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION
    if (_glfw.wl.seatVersion >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
    {
        _glfw.wl.keyRepeatTimerfd =
            timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    }
#endif

    if (!_glfw.wl.wmBase)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to find xdg-shell in your compositor");
        return GLFW_FALSE;
    }

    if (!_glfw.wl.shm)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Wayland: Failed to find wl_shm in your compositor");
        return GLFW_FALSE;
    }

    if (!loadCursorTheme())
        return GLFW_FALSE;

    if (_glfw.wl.seat && _glfw.wl.dataDeviceManager)
    {
        _glfw.wl.dataDevice =
            wl_data_device_manager_get_data_device(_glfw.wl.dataDeviceManager,
                                                   _glfw.wl.seat);
        _glfwAddDataDeviceListenerWayland(_glfw.wl.dataDevice);
    }

    return GLFW_TRUE;
}

void _glfwTerminateWayland(void)
{
    _glfwTerminateEGL();
    _glfwTerminateOSMesa();

    if (_glfw.wl.egl.handle)
    {
        _glfwPlatformFreeModule(_glfw.wl.egl.handle);
        _glfw.wl.egl.handle = NULL;
    }

    if (_glfw.wl.xkb.composeState)
        xkb_compose_state_unref(_glfw.wl.xkb.composeState);
    if (_glfw.wl.xkb.keymap)
        xkb_keymap_unref(_glfw.wl.xkb.keymap);
    if (_glfw.wl.xkb.state)
        xkb_state_unref(_glfw.wl.xkb.state);
    if (_glfw.wl.xkb.context)
        xkb_context_unref(_glfw.wl.xkb.context);
    if (_glfw.wl.xkb.handle)
    {
        _glfwPlatformFreeModule(_glfw.wl.xkb.handle);
        _glfw.wl.xkb.handle = NULL;
    }

    if (_glfw.wl.cursorTheme)
        wl_cursor_theme_destroy(_glfw.wl.cursorTheme);
    if (_glfw.wl.cursorThemeHiDPI)
        wl_cursor_theme_destroy(_glfw.wl.cursorThemeHiDPI);
    if (_glfw.wl.cursor.handle)
    {
        _glfwPlatformFreeModule(_glfw.wl.cursor.handle);
        _glfw.wl.cursor.handle = NULL;
    }

    for (unsigned int i = 0; i < _glfw.wl.offerCount; i++)
        wl_data_offer_destroy(_glfw.wl.offers[i].offer);

    _glfw_free(_glfw.wl.offers);

    if (_glfw.wl.cursorSurface)
        wl_surface_destroy(_glfw.wl.cursorSurface);
    if (_glfw.wl.subcompositor)
        wl_subcompositor_destroy(_glfw.wl.subcompositor);
    if (_glfw.wl.compositor)
        wl_compositor_destroy(_glfw.wl.compositor);
    if (_glfw.wl.shm)
        wl_shm_destroy(_glfw.wl.shm);
    if (_glfw.wl.viewporter)
        wp_viewporter_destroy(_glfw.wl.viewporter);
    if (_glfw.wl.decorationManager)
        zxdg_decoration_manager_v1_destroy(_glfw.wl.decorationManager);
    if (_glfw.wl.wmBase)
        xdg_wm_base_destroy(_glfw.wl.wmBase);
    if (_glfw.wl.selectionOffer)
        wl_data_offer_destroy(_glfw.wl.selectionOffer);
    if (_glfw.wl.dragOffer)
        wl_data_offer_destroy(_glfw.wl.dragOffer);
    if (_glfw.wl.selectionSource)
        wl_data_source_destroy(_glfw.wl.selectionSource);
    if (_glfw.wl.dataDevice)
        wl_data_device_destroy(_glfw.wl.dataDevice);
    if (_glfw.wl.dataDeviceManager)
        wl_data_device_manager_destroy(_glfw.wl.dataDeviceManager);
    if (_glfw.wl.pointer)
        wl_pointer_destroy(_glfw.wl.pointer);
    if (_glfw.wl.keyboard)
        wl_keyboard_destroy(_glfw.wl.keyboard);
    if (_glfw.wl.seat)
        wl_seat_destroy(_glfw.wl.seat);
    if (_glfw.wl.relativePointerManager)
        zwp_relative_pointer_manager_v1_destroy(_glfw.wl.relativePointerManager);
    if (_glfw.wl.pointerConstraints)
        zwp_pointer_constraints_v1_destroy(_glfw.wl.pointerConstraints);
    if (_glfw.wl.idleInhibitManager)
        zwp_idle_inhibit_manager_v1_destroy(_glfw.wl.idleInhibitManager);
    if (_glfw.wl.registry)
        wl_registry_destroy(_glfw.wl.registry);
    if (_glfw.wl.display)
    {
        wl_display_flush(_glfw.wl.display);
        wl_display_disconnect(_glfw.wl.display);
    }

    if (_glfw.wl.keyRepeatTimerfd >= 0)
        close(_glfw.wl.keyRepeatTimerfd);
    if (_glfw.wl.cursorTimerfd >= 0)
        close(_glfw.wl.cursorTimerfd);

    _glfw_free(_glfw.wl.clipboardString);
}

