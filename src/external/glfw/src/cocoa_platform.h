//========================================================================
// GLFW 3.4 macOS - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2009-2019 Camilla Löwy <elmindreda@glfw.org>
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

#include <stdint.h>

#include <Carbon/Carbon.h>
#include <IOKit/hid/IOHIDLib.h>

// NOTE: All of NSGL was deprecated in the 10.14 SDK
//       This disables the pointless warnings for every symbol we use
#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif

#if defined(__OBJC__)
#import <Cocoa/Cocoa.h>
#else
typedef void* id;
#endif

// NOTE: Many Cocoa enum values have been renamed and we need to build across
//       SDK versions where one is unavailable or deprecated.
//       We use the newer names in code and replace them with the older names if
//       the base SDK does not provide the newer names.

#if MAC_OS_X_VERSION_MAX_ALLOWED < 101400
 #define NSOpenGLContextParameterSwapInterval NSOpenGLCPSwapInterval
 #define NSOpenGLContextParameterSurfaceOpacity NSOpenGLCPSurfaceOpacity
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED < 101200
 #define NSBitmapFormatAlphaNonpremultiplied NSAlphaNonpremultipliedBitmapFormat
 #define NSEventMaskAny NSAnyEventMask
 #define NSEventMaskKeyUp NSKeyUpMask
 #define NSEventModifierFlagCapsLock NSAlphaShiftKeyMask
 #define NSEventModifierFlagCommand NSCommandKeyMask
 #define NSEventModifierFlagControl NSControlKeyMask
 #define NSEventModifierFlagDeviceIndependentFlagsMask NSDeviceIndependentModifierFlagsMask
 #define NSEventModifierFlagOption NSAlternateKeyMask
 #define NSEventModifierFlagShift NSShiftKeyMask
 #define NSEventTypeApplicationDefined NSApplicationDefined
 #define NSWindowStyleMaskBorderless NSBorderlessWindowMask
 #define NSWindowStyleMaskClosable NSClosableWindowMask
 #define NSWindowStyleMaskMiniaturizable NSMiniaturizableWindowMask
 #define NSWindowStyleMaskResizable NSResizableWindowMask
 #define NSWindowStyleMaskTitled NSTitledWindowMask
#endif

// NOTE: Many Cocoa dynamically linked constants have been renamed and we need
//       to build across SDK versions where one is unavailable or deprecated.
//       We use the newer names in code and replace them with the older names if
//       the deployment target is older than the newer names.

#if MAC_OS_X_VERSION_MIN_REQUIRED < 101300
 #define NSPasteboardTypeURL NSURLPboardType
#endif

typedef VkFlags VkMacOSSurfaceCreateFlagsMVK;
typedef VkFlags VkMetalSurfaceCreateFlagsEXT;

typedef struct VkMacOSSurfaceCreateInfoMVK
{
    VkStructureType                 sType;
    const void*                     pNext;
    VkMacOSSurfaceCreateFlagsMVK    flags;
    const void*                     pView;
} VkMacOSSurfaceCreateInfoMVK;

typedef struct VkMetalSurfaceCreateInfoEXT
{
    VkStructureType                 sType;
    const void*                     pNext;
    VkMetalSurfaceCreateFlagsEXT    flags;
    const void*                     pLayer;
} VkMetalSurfaceCreateInfoEXT;

typedef VkResult (APIENTRY *PFN_vkCreateMacOSSurfaceMVK)(VkInstance,const VkMacOSSurfaceCreateInfoMVK*,const VkAllocationCallbacks*,VkSurfaceKHR*);
typedef VkResult (APIENTRY *PFN_vkCreateMetalSurfaceEXT)(VkInstance,const VkMetalSurfaceCreateInfoEXT*,const VkAllocationCallbacks*,VkSurfaceKHR*);

#define GLFW_COCOA_WINDOW_STATE         _GLFWwindowNS  ns;
#define GLFW_COCOA_LIBRARY_WINDOW_STATE _GLFWlibraryNS ns;
#define GLFW_COCOA_MONITOR_STATE        _GLFWmonitorNS ns;
#define GLFW_COCOA_CURSOR_STATE         _GLFWcursorNS  ns;

#define GLFW_NSGL_CONTEXT_STATE         _GLFWcontextNSGL nsgl;
#define GLFW_NSGL_LIBRARY_CONTEXT_STATE _GLFWlibraryNSGL nsgl;

// HIToolbox.framework pointer typedefs
#define kTISPropertyUnicodeKeyLayoutData _glfw.ns.tis.kPropertyUnicodeKeyLayoutData
typedef TISInputSourceRef (*PFN_TISCopyCurrentKeyboardLayoutInputSource)(void);
#define TISCopyCurrentKeyboardLayoutInputSource _glfw.ns.tis.CopyCurrentKeyboardLayoutInputSource
typedef void* (*PFN_TISGetInputSourceProperty)(TISInputSourceRef,CFStringRef);
#define TISGetInputSourceProperty _glfw.ns.tis.GetInputSourceProperty
typedef UInt8 (*PFN_LMGetKbdType)(void);
#define LMGetKbdType _glfw.ns.tis.GetKbdType


// NSGL-specific per-context data
//
typedef struct _GLFWcontextNSGL
{
    id                pixelFormat;
    id                object;
} _GLFWcontextNSGL;

// NSGL-specific global data
//
typedef struct _GLFWlibraryNSGL
{
    // dlopen handle for OpenGL.framework (for glfwGetProcAddress)
    CFBundleRef     framework;
} _GLFWlibraryNSGL;

// Cocoa-specific per-window data
//
typedef struct _GLFWwindowNS
{
    id              object;
    id              delegate;
    id              view;
    id              layer;

    GLFWbool        maximized;
    GLFWbool        occluded;
    GLFWbool        scaleFramebuffer;

    // Cached window properties to filter out duplicate events
    int             width, height;
    int             fbWidth, fbHeight;
    float           xscale, yscale;

    // The total sum of the distances the cursor has been warped
    // since the last cursor motion event was processed
    // This is kept to counteract Cocoa doing the same internally
    double          cursorWarpDeltaX, cursorWarpDeltaY;
} _GLFWwindowNS;

// Cocoa-specific global data
//
typedef struct _GLFWlibraryNS
{
    CGEventSourceRef    eventSource;
    id                  delegate;
    GLFWbool            cursorHidden;
    TISInputSourceRef   inputSource;
    IOHIDManagerRef     hidManager;
    id                  unicodeData;
    id                  helper;
    id                  keyUpMonitor;
    id                  nibObjects;

    char                keynames[GLFW_KEY_LAST + 1][17];
    short int           keycodes[256];
    short int           scancodes[GLFW_KEY_LAST + 1];
    char*               clipboardString;
    CGPoint             cascadePoint;
    // Where to place the cursor when re-enabled
    double              restoreCursorPosX, restoreCursorPosY;
    // The window whose disabled cursor mode is active
    _GLFWwindow*        disabledCursorWindow;

    struct {
        CFBundleRef     bundle;
        PFN_TISCopyCurrentKeyboardLayoutInputSource CopyCurrentKeyboardLayoutInputSource;
        PFN_TISGetInputSourceProperty GetInputSourceProperty;
        PFN_LMGetKbdType GetKbdType;
        CFStringRef     kPropertyUnicodeKeyLayoutData;
    } tis;
} _GLFWlibraryNS;

// Cocoa-specific per-monitor data
//
typedef struct _GLFWmonitorNS
{
    CGDirectDisplayID   displayID;
    CGDisplayModeRef    previousMode;
    uint32_t            unitNumber;
    id                  screen;
    double              fallbackRefreshRate;
} _GLFWmonitorNS;

// Cocoa-specific per-cursor data
//
typedef struct _GLFWcursorNS
{
    id              object;
} _GLFWcursorNS;


GLFWbool _glfwConnectCocoa(int platformID, _GLFWplatform* platform);
int _glfwInitCocoa(void);
void _glfwTerminateCocoa(void);

GLFWbool _glfwCreateWindowCocoa(_GLFWwindow* window, const _GLFWwndconfig* wndconfig, const _GLFWctxconfig* ctxconfig, const _GLFWfbconfig* fbconfig);
void _glfwDestroyWindowCocoa(_GLFWwindow* window);
void _glfwSetWindowTitleCocoa(_GLFWwindow* window, const char* title);
void _glfwSetWindowIconCocoa(_GLFWwindow* window, int count, const GLFWimage* images);
void _glfwGetWindowPosCocoa(_GLFWwindow* window, int* xpos, int* ypos);
void _glfwSetWindowPosCocoa(_GLFWwindow* window, int xpos, int ypos);
void _glfwGetWindowSizeCocoa(_GLFWwindow* window, int* width, int* height);
void _glfwSetWindowSizeCocoa(_GLFWwindow* window, int width, int height);
void _glfwSetWindowSizeLimitsCocoa(_GLFWwindow* window, int minwidth, int minheight, int maxwidth, int maxheight);
void _glfwSetWindowAspectRatioCocoa(_GLFWwindow* window, int numer, int denom);
void _glfwGetFramebufferSizeCocoa(_GLFWwindow* window, int* width, int* height);
void _glfwGetWindowFrameSizeCocoa(_GLFWwindow* window, int* left, int* top, int* right, int* bottom);
void _glfwGetWindowContentScaleCocoa(_GLFWwindow* window, float* xscale, float* yscale);
void _glfwIconifyWindowCocoa(_GLFWwindow* window);
void _glfwRestoreWindowCocoa(_GLFWwindow* window);
void _glfwMaximizeWindowCocoa(_GLFWwindow* window);
void _glfwShowWindowCocoa(_GLFWwindow* window);
void _glfwHideWindowCocoa(_GLFWwindow* window);
void _glfwRequestWindowAttentionCocoa(_GLFWwindow* window);
void _glfwFocusWindowCocoa(_GLFWwindow* window);
void _glfwSetWindowMonitorCocoa(_GLFWwindow* window, _GLFWmonitor* monitor, int xpos, int ypos, int width, int height, int refreshRate);
GLFWbool _glfwWindowFocusedCocoa(_GLFWwindow* window);
GLFWbool _glfwWindowIconifiedCocoa(_GLFWwindow* window);
GLFWbool _glfwWindowVisibleCocoa(_GLFWwindow* window);
GLFWbool _glfwWindowMaximizedCocoa(_GLFWwindow* window);
GLFWbool _glfwWindowHoveredCocoa(_GLFWwindow* window);
GLFWbool _glfwFramebufferTransparentCocoa(_GLFWwindow* window);
void _glfwSetWindowResizableCocoa(_GLFWwindow* window, GLFWbool enabled);
void _glfwSetWindowDecoratedCocoa(_GLFWwindow* window, GLFWbool enabled);
void _glfwSetWindowFloatingCocoa(_GLFWwindow* window, GLFWbool enabled);
float _glfwGetWindowOpacityCocoa(_GLFWwindow* window);
void _glfwSetWindowOpacityCocoa(_GLFWwindow* window, float opacity);
void _glfwSetWindowMousePassthroughCocoa(_GLFWwindow* window, GLFWbool enabled);

void _glfwSetRawMouseMotionCocoa(_GLFWwindow *window, GLFWbool enabled);
GLFWbool _glfwRawMouseMotionSupportedCocoa(void);

void _glfwPollEventsCocoa(void);
void _glfwWaitEventsCocoa(void);
void _glfwWaitEventsTimeoutCocoa(double timeout);
void _glfwPostEmptyEventCocoa(void);

void _glfwGetCursorPosCocoa(_GLFWwindow* window, double* xpos, double* ypos);
void _glfwSetCursorPosCocoa(_GLFWwindow* window, double xpos, double ypos);
void _glfwSetCursorModeCocoa(_GLFWwindow* window, int mode);
const char* _glfwGetScancodeNameCocoa(int scancode);
int _glfwGetKeyScancodeCocoa(int key);
GLFWbool _glfwCreateCursorCocoa(_GLFWcursor* cursor, const GLFWimage* image, int xhot, int yhot);
GLFWbool _glfwCreateStandardCursorCocoa(_GLFWcursor* cursor, int shape);
void _glfwDestroyCursorCocoa(_GLFWcursor* cursor);
void _glfwSetCursorCocoa(_GLFWwindow* window, _GLFWcursor* cursor);
void _glfwSetClipboardStringCocoa(const char* string);
const char* _glfwGetClipboardStringCocoa(void);

EGLenum _glfwGetEGLPlatformCocoa(EGLint** attribs);
EGLNativeDisplayType _glfwGetEGLNativeDisplayCocoa(void);
EGLNativeWindowType _glfwGetEGLNativeWindowCocoa(_GLFWwindow* window);

void _glfwGetRequiredInstanceExtensionsCocoa(char** extensions);
GLFWbool _glfwGetPhysicalDevicePresentationSupportCocoa(VkInstance instance, VkPhysicalDevice device, uint32_t queuefamily);
VkResult _glfwCreateWindowSurfaceCocoa(VkInstance instance, _GLFWwindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);

void _glfwFreeMonitorCocoa(_GLFWmonitor* monitor);
void _glfwGetMonitorPosCocoa(_GLFWmonitor* monitor, int* xpos, int* ypos);
void _glfwGetMonitorContentScaleCocoa(_GLFWmonitor* monitor, float* xscale, float* yscale);
void _glfwGetMonitorWorkareaCocoa(_GLFWmonitor* monitor, int* xpos, int* ypos, int* width, int* height);
GLFWvidmode* _glfwGetVideoModesCocoa(_GLFWmonitor* monitor, int* count);
GLFWbool _glfwGetVideoModeCocoa(_GLFWmonitor* monitor, GLFWvidmode* mode);
GLFWbool _glfwGetGammaRampCocoa(_GLFWmonitor* monitor, GLFWgammaramp* ramp);
void _glfwSetGammaRampCocoa(_GLFWmonitor* monitor, const GLFWgammaramp* ramp);

void _glfwPollMonitorsCocoa(void);
void _glfwSetVideoModeCocoa(_GLFWmonitor* monitor, const GLFWvidmode* desired);
void _glfwRestoreVideoModeCocoa(_GLFWmonitor* monitor);

float _glfwTransformYCocoa(float y);

void* _glfwLoadLocalVulkanLoaderCocoa(void);

GLFWbool _glfwInitNSGL(void);
void _glfwTerminateNSGL(void);
GLFWbool _glfwCreateContextNSGL(_GLFWwindow* window,
                                const _GLFWctxconfig* ctxconfig,
                                const _GLFWfbconfig* fbconfig);
void _glfwDestroyContextNSGL(_GLFWwindow* window);

