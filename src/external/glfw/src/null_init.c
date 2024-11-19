//========================================================================
// GLFW 3.4 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2016 Google Inc.
// Copyright (c) 2016-2017 Camilla Löwy <elmindreda@glfw.org>
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

#include "internal.h"

#include <stdlib.h>
#include <string.h>


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwConnectNull(int platformID, _GLFWplatform* platform)
{
    const _GLFWplatform null =
    {
        .platformID = GLFW_PLATFORM_NULL,
        .init = _glfwInitNull,
        .terminate = _glfwTerminateNull,
        .getCursorPos = _glfwGetCursorPosNull,
        .setCursorPos = _glfwSetCursorPosNull,
        .setCursorMode = _glfwSetCursorModeNull,
        .setRawMouseMotion = _glfwSetRawMouseMotionNull,
        .rawMouseMotionSupported = _glfwRawMouseMotionSupportedNull,
        .createCursor = _glfwCreateCursorNull,
        .createStandardCursor = _glfwCreateStandardCursorNull,
        .destroyCursor = _glfwDestroyCursorNull,
        .setCursor = _glfwSetCursorNull,
        .getScancodeName = _glfwGetScancodeNameNull,
        .getKeyScancode = _glfwGetKeyScancodeNull,
        .setClipboardString = _glfwSetClipboardStringNull,
        .getClipboardString = _glfwGetClipboardStringNull,
        .initJoysticks = _glfwInitJoysticksNull,
        .terminateJoysticks = _glfwTerminateJoysticksNull,
        .pollJoystick = _glfwPollJoystickNull,
        .getMappingName = _glfwGetMappingNameNull,
        .updateGamepadGUID = _glfwUpdateGamepadGUIDNull,
        .freeMonitor = _glfwFreeMonitorNull,
        .getMonitorPos = _glfwGetMonitorPosNull,
        .getMonitorContentScale = _glfwGetMonitorContentScaleNull,
        .getMonitorWorkarea = _glfwGetMonitorWorkareaNull,
        .getVideoModes = _glfwGetVideoModesNull,
        .getVideoMode = _glfwGetVideoModeNull,
        .getGammaRamp = _glfwGetGammaRampNull,
        .setGammaRamp = _glfwSetGammaRampNull,
        .createWindow = _glfwCreateWindowNull,
        .destroyWindow = _glfwDestroyWindowNull,
        .setWindowTitle = _glfwSetWindowTitleNull,
        .setWindowIcon = _glfwSetWindowIconNull,
        .getWindowPos = _glfwGetWindowPosNull,
        .setWindowPos = _glfwSetWindowPosNull,
        .getWindowSize = _glfwGetWindowSizeNull,
        .setWindowSize = _glfwSetWindowSizeNull,
        .setWindowSizeLimits = _glfwSetWindowSizeLimitsNull,
        .setWindowAspectRatio = _glfwSetWindowAspectRatioNull,
        .getFramebufferSize = _glfwGetFramebufferSizeNull,
        .getWindowFrameSize = _glfwGetWindowFrameSizeNull,
        .getWindowContentScale = _glfwGetWindowContentScaleNull,
        .iconifyWindow = _glfwIconifyWindowNull,
        .restoreWindow = _glfwRestoreWindowNull,
        .maximizeWindow = _glfwMaximizeWindowNull,
        .showWindow = _glfwShowWindowNull,
        .hideWindow = _glfwHideWindowNull,
        .requestWindowAttention = _glfwRequestWindowAttentionNull,
        .focusWindow = _glfwFocusWindowNull,
        .setWindowMonitor = _glfwSetWindowMonitorNull,
        .windowFocused = _glfwWindowFocusedNull,
        .windowIconified = _glfwWindowIconifiedNull,
        .windowVisible = _glfwWindowVisibleNull,
        .windowMaximized = _glfwWindowMaximizedNull,
        .windowHovered = _glfwWindowHoveredNull,
        .framebufferTransparent = _glfwFramebufferTransparentNull,
        .getWindowOpacity = _glfwGetWindowOpacityNull,
        .setWindowResizable = _glfwSetWindowResizableNull,
        .setWindowDecorated = _glfwSetWindowDecoratedNull,
        .setWindowFloating = _glfwSetWindowFloatingNull,
        .setWindowOpacity = _glfwSetWindowOpacityNull,
        .setWindowMousePassthrough = _glfwSetWindowMousePassthroughNull,
        .pollEvents = _glfwPollEventsNull,
        .waitEvents = _glfwWaitEventsNull,
        .waitEventsTimeout = _glfwWaitEventsTimeoutNull,
        .postEmptyEvent = _glfwPostEmptyEventNull,
        .getEGLPlatform = _glfwGetEGLPlatformNull,
        .getEGLNativeDisplay = _glfwGetEGLNativeDisplayNull,
        .getEGLNativeWindow = _glfwGetEGLNativeWindowNull,
        .getRequiredInstanceExtensions = _glfwGetRequiredInstanceExtensionsNull,
        .getPhysicalDevicePresentationSupport = _glfwGetPhysicalDevicePresentationSupportNull,
        .createWindowSurface = _glfwCreateWindowSurfaceNull
    };

    *platform = null;
    return GLFW_TRUE;
}

int _glfwInitNull(void)
{
    int scancode;

    memset(_glfw.null.keycodes, -1, sizeof(_glfw.null.keycodes));
    memset(_glfw.null.scancodes, -1, sizeof(_glfw.null.scancodes));

    _glfw.null.keycodes[GLFW_NULL_SC_SPACE]         = GLFW_KEY_SPACE;
    _glfw.null.keycodes[GLFW_NULL_SC_APOSTROPHE]    = GLFW_KEY_APOSTROPHE;
    _glfw.null.keycodes[GLFW_NULL_SC_COMMA]         = GLFW_KEY_COMMA;
    _glfw.null.keycodes[GLFW_NULL_SC_MINUS]         = GLFW_KEY_MINUS;
    _glfw.null.keycodes[GLFW_NULL_SC_PERIOD]        = GLFW_KEY_PERIOD;
    _glfw.null.keycodes[GLFW_NULL_SC_SLASH]         = GLFW_KEY_SLASH;
    _glfw.null.keycodes[GLFW_NULL_SC_0]             = GLFW_KEY_0;
    _glfw.null.keycodes[GLFW_NULL_SC_1]             = GLFW_KEY_1;
    _glfw.null.keycodes[GLFW_NULL_SC_2]             = GLFW_KEY_2;
    _glfw.null.keycodes[GLFW_NULL_SC_3]             = GLFW_KEY_3;
    _glfw.null.keycodes[GLFW_NULL_SC_4]             = GLFW_KEY_4;
    _glfw.null.keycodes[GLFW_NULL_SC_5]             = GLFW_KEY_5;
    _glfw.null.keycodes[GLFW_NULL_SC_6]             = GLFW_KEY_6;
    _glfw.null.keycodes[GLFW_NULL_SC_7]             = GLFW_KEY_7;
    _glfw.null.keycodes[GLFW_NULL_SC_8]             = GLFW_KEY_8;
    _glfw.null.keycodes[GLFW_NULL_SC_9]             = GLFW_KEY_9;
    _glfw.null.keycodes[GLFW_NULL_SC_SEMICOLON]     = GLFW_KEY_SEMICOLON;
    _glfw.null.keycodes[GLFW_NULL_SC_EQUAL]         = GLFW_KEY_EQUAL;
    _glfw.null.keycodes[GLFW_NULL_SC_A]             = GLFW_KEY_A;
    _glfw.null.keycodes[GLFW_NULL_SC_B]             = GLFW_KEY_B;
    _glfw.null.keycodes[GLFW_NULL_SC_C]             = GLFW_KEY_C;
    _glfw.null.keycodes[GLFW_NULL_SC_D]             = GLFW_KEY_D;
    _glfw.null.keycodes[GLFW_NULL_SC_E]             = GLFW_KEY_E;
    _glfw.null.keycodes[GLFW_NULL_SC_F]             = GLFW_KEY_F;
    _glfw.null.keycodes[GLFW_NULL_SC_G]             = GLFW_KEY_G;
    _glfw.null.keycodes[GLFW_NULL_SC_H]             = GLFW_KEY_H;
    _glfw.null.keycodes[GLFW_NULL_SC_I]             = GLFW_KEY_I;
    _glfw.null.keycodes[GLFW_NULL_SC_J]             = GLFW_KEY_J;
    _glfw.null.keycodes[GLFW_NULL_SC_K]             = GLFW_KEY_K;
    _glfw.null.keycodes[GLFW_NULL_SC_L]             = GLFW_KEY_L;
    _glfw.null.keycodes[GLFW_NULL_SC_M]             = GLFW_KEY_M;
    _glfw.null.keycodes[GLFW_NULL_SC_N]             = GLFW_KEY_N;
    _glfw.null.keycodes[GLFW_NULL_SC_O]             = GLFW_KEY_O;
    _glfw.null.keycodes[GLFW_NULL_SC_P]             = GLFW_KEY_P;
    _glfw.null.keycodes[GLFW_NULL_SC_Q]             = GLFW_KEY_Q;
    _glfw.null.keycodes[GLFW_NULL_SC_R]             = GLFW_KEY_R;
    _glfw.null.keycodes[GLFW_NULL_SC_S]             = GLFW_KEY_S;
    _glfw.null.keycodes[GLFW_NULL_SC_T]             = GLFW_KEY_T;
    _glfw.null.keycodes[GLFW_NULL_SC_U]             = GLFW_KEY_U;
    _glfw.null.keycodes[GLFW_NULL_SC_V]             = GLFW_KEY_V;
    _glfw.null.keycodes[GLFW_NULL_SC_W]             = GLFW_KEY_W;
    _glfw.null.keycodes[GLFW_NULL_SC_X]             = GLFW_KEY_X;
    _glfw.null.keycodes[GLFW_NULL_SC_Y]             = GLFW_KEY_Y;
    _glfw.null.keycodes[GLFW_NULL_SC_Z]             = GLFW_KEY_Z;
    _glfw.null.keycodes[GLFW_NULL_SC_LEFT_BRACKET]  = GLFW_KEY_LEFT_BRACKET;
    _glfw.null.keycodes[GLFW_NULL_SC_BACKSLASH]     = GLFW_KEY_BACKSLASH;
    _glfw.null.keycodes[GLFW_NULL_SC_RIGHT_BRACKET] = GLFW_KEY_RIGHT_BRACKET;
    _glfw.null.keycodes[GLFW_NULL_SC_GRAVE_ACCENT]  = GLFW_KEY_GRAVE_ACCENT;
    _glfw.null.keycodes[GLFW_NULL_SC_WORLD_1]       = GLFW_KEY_WORLD_1;
    _glfw.null.keycodes[GLFW_NULL_SC_WORLD_2]       = GLFW_KEY_WORLD_2;
    _glfw.null.keycodes[GLFW_NULL_SC_ESCAPE]        = GLFW_KEY_ESCAPE;
    _glfw.null.keycodes[GLFW_NULL_SC_ENTER]         = GLFW_KEY_ENTER;
    _glfw.null.keycodes[GLFW_NULL_SC_TAB]           = GLFW_KEY_TAB;
    _glfw.null.keycodes[GLFW_NULL_SC_BACKSPACE]     = GLFW_KEY_BACKSPACE;
    _glfw.null.keycodes[GLFW_NULL_SC_INSERT]        = GLFW_KEY_INSERT;
    _glfw.null.keycodes[GLFW_NULL_SC_DELETE]        = GLFW_KEY_DELETE;
    _glfw.null.keycodes[GLFW_NULL_SC_RIGHT]         = GLFW_KEY_RIGHT;
    _glfw.null.keycodes[GLFW_NULL_SC_LEFT]          = GLFW_KEY_LEFT;
    _glfw.null.keycodes[GLFW_NULL_SC_DOWN]          = GLFW_KEY_DOWN;
    _glfw.null.keycodes[GLFW_NULL_SC_UP]            = GLFW_KEY_UP;
    _glfw.null.keycodes[GLFW_NULL_SC_PAGE_UP]       = GLFW_KEY_PAGE_UP;
    _glfw.null.keycodes[GLFW_NULL_SC_PAGE_DOWN]     = GLFW_KEY_PAGE_DOWN;
    _glfw.null.keycodes[GLFW_NULL_SC_HOME]          = GLFW_KEY_HOME;
    _glfw.null.keycodes[GLFW_NULL_SC_END]           = GLFW_KEY_END;
    _glfw.null.keycodes[GLFW_NULL_SC_CAPS_LOCK]     = GLFW_KEY_CAPS_LOCK;
    _glfw.null.keycodes[GLFW_NULL_SC_SCROLL_LOCK]   = GLFW_KEY_SCROLL_LOCK;
    _glfw.null.keycodes[GLFW_NULL_SC_NUM_LOCK]      = GLFW_KEY_NUM_LOCK;
    _glfw.null.keycodes[GLFW_NULL_SC_PRINT_SCREEN]  = GLFW_KEY_PRINT_SCREEN;
    _glfw.null.keycodes[GLFW_NULL_SC_PAUSE]         = GLFW_KEY_PAUSE;
    _glfw.null.keycodes[GLFW_NULL_SC_F1]            = GLFW_KEY_F1;
    _glfw.null.keycodes[GLFW_NULL_SC_F2]            = GLFW_KEY_F2;
    _glfw.null.keycodes[GLFW_NULL_SC_F3]            = GLFW_KEY_F3;
    _glfw.null.keycodes[GLFW_NULL_SC_F4]            = GLFW_KEY_F4;
    _glfw.null.keycodes[GLFW_NULL_SC_F5]            = GLFW_KEY_F5;
    _glfw.null.keycodes[GLFW_NULL_SC_F6]            = GLFW_KEY_F6;
    _glfw.null.keycodes[GLFW_NULL_SC_F7]            = GLFW_KEY_F7;
    _glfw.null.keycodes[GLFW_NULL_SC_F8]            = GLFW_KEY_F8;
    _glfw.null.keycodes[GLFW_NULL_SC_F9]            = GLFW_KEY_F9;
    _glfw.null.keycodes[GLFW_NULL_SC_F10]           = GLFW_KEY_F10;
    _glfw.null.keycodes[GLFW_NULL_SC_F11]           = GLFW_KEY_F11;
    _glfw.null.keycodes[GLFW_NULL_SC_F12]           = GLFW_KEY_F12;
    _glfw.null.keycodes[GLFW_NULL_SC_F13]           = GLFW_KEY_F13;
    _glfw.null.keycodes[GLFW_NULL_SC_F14]           = GLFW_KEY_F14;
    _glfw.null.keycodes[GLFW_NULL_SC_F15]           = GLFW_KEY_F15;
    _glfw.null.keycodes[GLFW_NULL_SC_F16]           = GLFW_KEY_F16;
    _glfw.null.keycodes[GLFW_NULL_SC_F17]           = GLFW_KEY_F17;
    _glfw.null.keycodes[GLFW_NULL_SC_F18]           = GLFW_KEY_F18;
    _glfw.null.keycodes[GLFW_NULL_SC_F19]           = GLFW_KEY_F19;
    _glfw.null.keycodes[GLFW_NULL_SC_F20]           = GLFW_KEY_F20;
    _glfw.null.keycodes[GLFW_NULL_SC_F21]           = GLFW_KEY_F21;
    _glfw.null.keycodes[GLFW_NULL_SC_F22]           = GLFW_KEY_F22;
    _glfw.null.keycodes[GLFW_NULL_SC_F23]           = GLFW_KEY_F23;
    _glfw.null.keycodes[GLFW_NULL_SC_F24]           = GLFW_KEY_F24;
    _glfw.null.keycodes[GLFW_NULL_SC_F25]           = GLFW_KEY_F25;
    _glfw.null.keycodes[GLFW_NULL_SC_KP_0]          = GLFW_KEY_KP_0;
    _glfw.null.keycodes[GLFW_NULL_SC_KP_1]          = GLFW_KEY_KP_1;
    _glfw.null.keycodes[GLFW_NULL_SC_KP_2]          = GLFW_KEY_KP_2;
    _glfw.null.keycodes[GLFW_NULL_SC_KP_3]          = GLFW_KEY_KP_3;
    _glfw.null.keycodes[GLFW_NULL_SC_KP_4]          = GLFW_KEY_KP_4;
    _glfw.null.keycodes[GLFW_NULL_SC_KP_5]          = GLFW_KEY_KP_5;
    _glfw.null.keycodes[GLFW_NULL_SC_KP_6]          = GLFW_KEY_KP_6;
    _glfw.null.keycodes[GLFW_NULL_SC_KP_7]          = GLFW_KEY_KP_7;
    _glfw.null.keycodes[GLFW_NULL_SC_KP_8]          = GLFW_KEY_KP_8;
    _glfw.null.keycodes[GLFW_NULL_SC_KP_9]          = GLFW_KEY_KP_9;
    _glfw.null.keycodes[GLFW_NULL_SC_KP_DECIMAL]    = GLFW_KEY_KP_DECIMAL;
    _glfw.null.keycodes[GLFW_NULL_SC_KP_DIVIDE]     = GLFW_KEY_KP_DIVIDE;
    _glfw.null.keycodes[GLFW_NULL_SC_KP_MULTIPLY]   = GLFW_KEY_KP_MULTIPLY;
    _glfw.null.keycodes[GLFW_NULL_SC_KP_SUBTRACT]   = GLFW_KEY_KP_SUBTRACT;
    _glfw.null.keycodes[GLFW_NULL_SC_KP_ADD]        = GLFW_KEY_KP_ADD;
    _glfw.null.keycodes[GLFW_NULL_SC_KP_ENTER]      = GLFW_KEY_KP_ENTER;
    _glfw.null.keycodes[GLFW_NULL_SC_KP_EQUAL]      = GLFW_KEY_KP_EQUAL;
    _glfw.null.keycodes[GLFW_NULL_SC_LEFT_SHIFT]    = GLFW_KEY_LEFT_SHIFT;
    _glfw.null.keycodes[GLFW_NULL_SC_LEFT_CONTROL]  = GLFW_KEY_LEFT_CONTROL;
    _glfw.null.keycodes[GLFW_NULL_SC_LEFT_ALT]      = GLFW_KEY_LEFT_ALT;
    _glfw.null.keycodes[GLFW_NULL_SC_LEFT_SUPER]    = GLFW_KEY_LEFT_SUPER;
    _glfw.null.keycodes[GLFW_NULL_SC_RIGHT_SHIFT]   = GLFW_KEY_RIGHT_SHIFT;
    _glfw.null.keycodes[GLFW_NULL_SC_RIGHT_CONTROL] = GLFW_KEY_RIGHT_CONTROL;
    _glfw.null.keycodes[GLFW_NULL_SC_RIGHT_ALT]     = GLFW_KEY_RIGHT_ALT;
    _glfw.null.keycodes[GLFW_NULL_SC_RIGHT_SUPER]   = GLFW_KEY_RIGHT_SUPER;
    _glfw.null.keycodes[GLFW_NULL_SC_MENU]          = GLFW_KEY_MENU;

    for (scancode = GLFW_NULL_SC_FIRST;  scancode < GLFW_NULL_SC_LAST;  scancode++)
    {
        if (_glfw.null.keycodes[scancode] > 0)
            _glfw.null.scancodes[_glfw.null.keycodes[scancode]] = scancode;
    }

    _glfwPollMonitorsNull();
    return GLFW_TRUE;
}

void _glfwTerminateNull(void)
{
    free(_glfw.null.clipboardString);
    _glfwTerminateOSMesa();
    _glfwTerminateEGL();
}

