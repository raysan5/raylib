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

#define GLFW_NULL_WINDOW_STATE          _GLFWwindowNull null;
#define GLFW_NULL_LIBRARY_WINDOW_STATE  _GLFWlibraryNull null;
#define GLFW_NULL_MONITOR_STATE         _GLFWmonitorNull null;

#define GLFW_NULL_CONTEXT_STATE
#define GLFW_NULL_CURSOR_STATE
#define GLFW_NULL_LIBRARY_CONTEXT_STATE


// Null-specific per-window data
//
typedef struct _GLFWwindowNull
{
    int             xpos;
    int             ypos;
    int             width;
    int             height;
    char*           title;
    GLFWbool        visible;
    GLFWbool        iconified;
    GLFWbool        maximized;
    GLFWbool        resizable;
    GLFWbool        decorated;
    GLFWbool        floating;
    GLFWbool        transparent;
    float           opacity;
} _GLFWwindowNull;

// Null-specific per-monitor data
//
typedef struct _GLFWmonitorNull
{
    GLFWgammaramp   ramp;
} _GLFWmonitorNull;

// Null-specific global data
//
typedef struct _GLFWlibraryNull
{
    int             xcursor;
    int             ycursor;
    char*           clipboardString;
    _GLFWwindow*    focusedWindow;
} _GLFWlibraryNull;

void _glfwPollMonitorsNull(void);

GLFWbool _glfwConnectNull(int platformID, _GLFWplatform* platform);
int _glfwInitNull(void);
void _glfwTerminateNull(void);

void _glfwFreeMonitorNull(_GLFWmonitor* monitor);
void _glfwGetMonitorPosNull(_GLFWmonitor* monitor, int* xpos, int* ypos);
void _glfwGetMonitorContentScaleNull(_GLFWmonitor* monitor, float* xscale, float* yscale);
void _glfwGetMonitorWorkareaNull(_GLFWmonitor* monitor, int* xpos, int* ypos, int* width, int* height);
GLFWvidmode* _glfwGetVideoModesNull(_GLFWmonitor* monitor, int* found);
void _glfwGetVideoModeNull(_GLFWmonitor* monitor, GLFWvidmode* mode);
GLFWbool _glfwGetGammaRampNull(_GLFWmonitor* monitor, GLFWgammaramp* ramp);
void _glfwSetGammaRampNull(_GLFWmonitor* monitor, const GLFWgammaramp* ramp);

GLFWbool _glfwCreateWindowNull(_GLFWwindow* window, const _GLFWwndconfig* wndconfig, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig);
void _glfwDestroyWindowNull(_GLFWwindow* window);
void _glfwSetWindowTitleNull(_GLFWwindow* window, const char* title);
void _glfwSetWindowIconNull(_GLFWwindow* window, int count, const GLFWimage* images);
void _glfwSetWindowMonitorNull(_GLFWwindow* window, _GLFWmonitor* monitor, int xpos, int ypos, int width, int height, int refreshRate);
void _glfwGetWindowPosNull(_GLFWwindow* window, int* xpos, int* ypos);
void _glfwSetWindowPosNull(_GLFWwindow* window, int xpos, int ypos);
void _glfwGetWindowSizeNull(_GLFWwindow* window, int* width, int* height);
void _glfwSetWindowSizeNull(_GLFWwindow* window, int width, int height);
void _glfwSetWindowSizeLimitsNull(_GLFWwindow* window, int minwidth, int minheight, int maxwidth, int maxheight);
void _glfwSetWindowAspectRatioNull(_GLFWwindow* window, int n, int d);
void _glfwGetFramebufferSizeNull(_GLFWwindow* window, int* width, int* height);
void _glfwGetWindowFrameSizeNull(_GLFWwindow* window, int* left, int* top, int* right, int* bottom);
void _glfwGetWindowContentScaleNull(_GLFWwindow* window, float* xscale, float* yscale);
void _glfwIconifyWindowNull(_GLFWwindow* window);
void _glfwRestoreWindowNull(_GLFWwindow* window);
void _glfwMaximizeWindowNull(_GLFWwindow* window);
GLFWbool _glfwWindowMaximizedNull(_GLFWwindow* window);
GLFWbool _glfwWindowHoveredNull(_GLFWwindow* window);
GLFWbool _glfwFramebufferTransparentNull(_GLFWwindow* window);
void _glfwSetWindowResizableNull(_GLFWwindow* window, GLFWbool enabled);
void _glfwSetWindowDecoratedNull(_GLFWwindow* window, GLFWbool enabled);
void _glfwSetWindowFloatingNull(_GLFWwindow* window, GLFWbool enabled);
void _glfwSetWindowMousePassthroughNull(_GLFWwindow* window, GLFWbool enabled);
float _glfwGetWindowOpacityNull(_GLFWwindow* window);
void _glfwSetWindowOpacityNull(_GLFWwindow* window, float opacity);
void _glfwSetRawMouseMotionNull(_GLFWwindow *window, GLFWbool enabled);
GLFWbool _glfwRawMouseMotionSupportedNull(void);
void _glfwShowWindowNull(_GLFWwindow* window);
void _glfwRequestWindowAttentionNull(_GLFWwindow* window);
void _glfwRequestWindowAttentionNull(_GLFWwindow* window);
void _glfwHideWindowNull(_GLFWwindow* window);
void _glfwFocusWindowNull(_GLFWwindow* window);
GLFWbool _glfwWindowFocusedNull(_GLFWwindow* window);
GLFWbool _glfwWindowIconifiedNull(_GLFWwindow* window);
GLFWbool _glfwWindowVisibleNull(_GLFWwindow* window);
void _glfwPollEventsNull(void);
void _glfwWaitEventsNull(void);
void _glfwWaitEventsTimeoutNull(double timeout);
void _glfwPostEmptyEventNull(void);
void _glfwGetCursorPosNull(_GLFWwindow* window, double* xpos, double* ypos);
void _glfwSetCursorPosNull(_GLFWwindow* window, double x, double y);
void _glfwSetCursorModeNull(_GLFWwindow* window, int mode);
GLFWbool _glfwCreateCursorNull(_GLFWcursor* cursor, const GLFWimage* image, int xhot, int yhot);
GLFWbool _glfwCreateStandardCursorNull(_GLFWcursor* cursor, int shape);
void _glfwDestroyCursorNull(_GLFWcursor* cursor);
void _glfwSetCursorNull(_GLFWwindow* window, _GLFWcursor* cursor);
void _glfwSetClipboardStringNull(const char* string);
const char* _glfwGetClipboardStringNull(void);
const char* _glfwGetScancodeNameNull(int scancode);
int _glfwGetKeyScancodeNull(int key);

EGLenum _glfwGetEGLPlatformNull(EGLint** attribs);
EGLNativeDisplayType _glfwGetEGLNativeDisplayNull(void);
EGLNativeWindowType _glfwGetEGLNativeWindowNull(_GLFWwindow* window);

void _glfwGetRequiredInstanceExtensionsNull(char** extensions);
GLFWbool _glfwGetPhysicalDevicePresentationSupportNull(VkInstance instance, VkPhysicalDevice device, uint32_t queuefamily);
VkResult _glfwCreateWindowSurfaceNull(VkInstance instance, _GLFWwindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);

void _glfwPollMonitorsNull(void);

