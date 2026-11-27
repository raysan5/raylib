//========================================================================
// GLFW 3.5 - www.glfw.org
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

#define GLFW_NULL_SC_FIRST          GLFW_NULL_SC_SPACE
#define GLFW_NULL_SC_SPACE          1
#define GLFW_NULL_SC_APOSTROPHE     2
#define GLFW_NULL_SC_COMMA          3
#define GLFW_NULL_SC_MINUS          4
#define GLFW_NULL_SC_PERIOD         5
#define GLFW_NULL_SC_SLASH          6
#define GLFW_NULL_SC_0              7
#define GLFW_NULL_SC_1              8
#define GLFW_NULL_SC_2              9
#define GLFW_NULL_SC_3              10
#define GLFW_NULL_SC_4              11
#define GLFW_NULL_SC_5              12
#define GLFW_NULL_SC_6              13
#define GLFW_NULL_SC_7              14
#define GLFW_NULL_SC_8              15
#define GLFW_NULL_SC_9              16
#define GLFW_NULL_SC_SEMICOLON      17
#define GLFW_NULL_SC_EQUAL          18
#define GLFW_NULL_SC_LEFT_BRACKET   19
#define GLFW_NULL_SC_BACKSLASH      20
#define GLFW_NULL_SC_RIGHT_BRACKET  21
#define GLFW_NULL_SC_GRAVE_ACCENT   22
#define GLFW_NULL_SC_WORLD_1        23
#define GLFW_NULL_SC_WORLD_2        24
#define GLFW_NULL_SC_ESCAPE         25
#define GLFW_NULL_SC_ENTER          26
#define GLFW_NULL_SC_TAB            27
#define GLFW_NULL_SC_BACKSPACE      28
#define GLFW_NULL_SC_INSERT         29
#define GLFW_NULL_SC_DELETE         30
#define GLFW_NULL_SC_RIGHT          31
#define GLFW_NULL_SC_LEFT           32
#define GLFW_NULL_SC_DOWN           33
#define GLFW_NULL_SC_UP             34
#define GLFW_NULL_SC_PAGE_UP        35
#define GLFW_NULL_SC_PAGE_DOWN      36
#define GLFW_NULL_SC_HOME           37
#define GLFW_NULL_SC_END            38
#define GLFW_NULL_SC_CAPS_LOCK      39
#define GLFW_NULL_SC_SCROLL_LOCK    40
#define GLFW_NULL_SC_NUM_LOCK       41
#define GLFW_NULL_SC_PRINT_SCREEN   42
#define GLFW_NULL_SC_PAUSE          43
#define GLFW_NULL_SC_A              44
#define GLFW_NULL_SC_B              45
#define GLFW_NULL_SC_C              46
#define GLFW_NULL_SC_D              47
#define GLFW_NULL_SC_E              48
#define GLFW_NULL_SC_F              49
#define GLFW_NULL_SC_G              50
#define GLFW_NULL_SC_H              51
#define GLFW_NULL_SC_I              52
#define GLFW_NULL_SC_J              53
#define GLFW_NULL_SC_K              54
#define GLFW_NULL_SC_L              55
#define GLFW_NULL_SC_M              56
#define GLFW_NULL_SC_N              57
#define GLFW_NULL_SC_O              58
#define GLFW_NULL_SC_P              59
#define GLFW_NULL_SC_Q              60
#define GLFW_NULL_SC_R              61
#define GLFW_NULL_SC_S              62
#define GLFW_NULL_SC_T              63
#define GLFW_NULL_SC_U              64
#define GLFW_NULL_SC_V              65
#define GLFW_NULL_SC_W              66
#define GLFW_NULL_SC_X              67
#define GLFW_NULL_SC_Y              68
#define GLFW_NULL_SC_Z              69
#define GLFW_NULL_SC_F1             70
#define GLFW_NULL_SC_F2             71
#define GLFW_NULL_SC_F3             72
#define GLFW_NULL_SC_F4             73
#define GLFW_NULL_SC_F5             74
#define GLFW_NULL_SC_F6             75
#define GLFW_NULL_SC_F7             76
#define GLFW_NULL_SC_F8             77
#define GLFW_NULL_SC_F9             78
#define GLFW_NULL_SC_F10            79
#define GLFW_NULL_SC_F11            80
#define GLFW_NULL_SC_F12            81
#define GLFW_NULL_SC_F13            82
#define GLFW_NULL_SC_F14            83
#define GLFW_NULL_SC_F15            84
#define GLFW_NULL_SC_F16            85
#define GLFW_NULL_SC_F17            86
#define GLFW_NULL_SC_F18            87
#define GLFW_NULL_SC_F19            88
#define GLFW_NULL_SC_F20            89
#define GLFW_NULL_SC_F21            90
#define GLFW_NULL_SC_F22            91
#define GLFW_NULL_SC_F23            92
#define GLFW_NULL_SC_F24            93
#define GLFW_NULL_SC_F25            94
#define GLFW_NULL_SC_KP_0           95
#define GLFW_NULL_SC_KP_1           96
#define GLFW_NULL_SC_KP_2           97
#define GLFW_NULL_SC_KP_3           98
#define GLFW_NULL_SC_KP_4           99
#define GLFW_NULL_SC_KP_5           100
#define GLFW_NULL_SC_KP_6           101
#define GLFW_NULL_SC_KP_7           102
#define GLFW_NULL_SC_KP_8           103
#define GLFW_NULL_SC_KP_9           104
#define GLFW_NULL_SC_KP_DECIMAL     105
#define GLFW_NULL_SC_KP_DIVIDE      106
#define GLFW_NULL_SC_KP_MULTIPLY    107
#define GLFW_NULL_SC_KP_SUBTRACT    108
#define GLFW_NULL_SC_KP_ADD         109
#define GLFW_NULL_SC_KP_ENTER       110
#define GLFW_NULL_SC_KP_EQUAL       111
#define GLFW_NULL_SC_LEFT_SHIFT     112
#define GLFW_NULL_SC_LEFT_CONTROL   113
#define GLFW_NULL_SC_LEFT_ALT       114
#define GLFW_NULL_SC_LEFT_SUPER     115
#define GLFW_NULL_SC_RIGHT_SHIFT    116
#define GLFW_NULL_SC_RIGHT_CONTROL  117
#define GLFW_NULL_SC_RIGHT_ALT      118
#define GLFW_NULL_SC_RIGHT_SUPER    119
#define GLFW_NULL_SC_MENU           120
#define GLFW_NULL_SC_LAST           GLFW_NULL_SC_MENU

typedef VkFlags VkHeadlessSurfaceCreateFlagsEXT;

typedef struct VkHeadlessSurfaceCreateInfoEXT
{
    VkStructureType                 sType;
    const void*                     pNext;
    VkHeadlessSurfaceCreateFlagsEXT flags;
} VkHeadlessSurfaceCreateInfoEXT;

typedef VkResult (APIENTRY *PFN_vkCreateHeadlessSurfaceEXT)(VkInstance,const VkHeadlessSurfaceCreateInfoEXT*,const VkAllocationCallbacks*,VkSurfaceKHR*);

// Null-specific per-window data
//
typedef struct _GLFWwindowNull
{
    int             xpos;
    int             ypos;
    int             width;
    int             height;
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
    uint16_t        keycodes[GLFW_NULL_SC_LAST + 1];
    uint8_t         scancodes[GLFW_KEY_LAST + 1];
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
GLFWbool _glfwGetVideoModeNull(_GLFWmonitor* monitor, GLFWvidmode* mode);
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

