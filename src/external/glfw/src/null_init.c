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
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#include "internal.h"

#include <stdlib.h>


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwConnectNull(int platformID, _GLFWplatform* platform)
{
    const _GLFWplatform null =
    {
        GLFW_PLATFORM_NULL,
        _glfwInitNull,
        _glfwTerminateNull,
        _glfwGetCursorPosNull,
        _glfwSetCursorPosNull,
        _glfwSetCursorModeNull,
        _glfwSetRawMouseMotionNull,
        _glfwRawMouseMotionSupportedNull,
        _glfwCreateCursorNull,
        _glfwCreateStandardCursorNull,
        _glfwDestroyCursorNull,
        _glfwSetCursorNull,
        _glfwGetScancodeNameNull,
        _glfwGetKeyScancodeNull,
        _glfwSetClipboardStringNull,
        _glfwGetClipboardStringNull,
        _glfwInitJoysticksNull,
        _glfwTerminateJoysticksNull,
        _glfwPollJoystickNull,
        _glfwGetMappingNameNull,
        _glfwUpdateGamepadGUIDNull,
        _glfwFreeMonitorNull,
        _glfwGetMonitorPosNull,
        _glfwGetMonitorContentScaleNull,
        _glfwGetMonitorWorkareaNull,
        _glfwGetVideoModesNull,
        _glfwGetVideoModeNull,
        _glfwGetGammaRampNull,
        _glfwSetGammaRampNull,
        _glfwCreateWindowNull,
        _glfwDestroyWindowNull,
        _glfwSetWindowTitleNull,
        _glfwSetWindowIconNull,
        _glfwGetWindowPosNull,
        _glfwSetWindowPosNull,
        _glfwGetWindowSizeNull,
        _glfwSetWindowSizeNull,
        _glfwSetWindowSizeLimitsNull,
        _glfwSetWindowAspectRatioNull,
        _glfwGetFramebufferSizeNull,
        _glfwGetWindowFrameSizeNull,
        _glfwGetWindowContentScaleNull,
        _glfwIconifyWindowNull,
        _glfwRestoreWindowNull,
        _glfwMaximizeWindowNull,
        _glfwShowWindowNull,
        _glfwHideWindowNull,
        _glfwRequestWindowAttentionNull,
        _glfwFocusWindowNull,
        _glfwSetWindowMonitorNull,
        _glfwWindowFocusedNull,
        _glfwWindowIconifiedNull,
        _glfwWindowVisibleNull,
        _glfwWindowMaximizedNull,
        _glfwWindowHoveredNull,
        _glfwFramebufferTransparentNull,
        _glfwGetWindowOpacityNull,
        _glfwSetWindowResizableNull,
        _glfwSetWindowDecoratedNull,
        _glfwSetWindowFloatingNull,
        _glfwSetWindowOpacityNull,
        _glfwSetWindowMousePassthroughNull,
        _glfwPollEventsNull,
        _glfwWaitEventsNull,
        _glfwWaitEventsTimeoutNull,
        _glfwPostEmptyEventNull,
        _glfwGetEGLPlatformNull,
        _glfwGetEGLNativeDisplayNull,
        _glfwGetEGLNativeWindowNull,
        _glfwGetRequiredInstanceExtensionsNull,
        _glfwGetPhysicalDevicePresentationSupportNull,
        _glfwCreateWindowSurfaceNull,
    };

    *platform = null;
    return GLFW_TRUE;
}

int _glfwInitNull(void)
{
    _glfwPollMonitorsNull();
    return GLFW_TRUE;
}

void _glfwTerminateNull(void)
{
    free(_glfw.null.clipboardString);
    _glfwTerminateOSMesa();
    _glfwTerminateEGL();
}

