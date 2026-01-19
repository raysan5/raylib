/**********************************************************************************************
*
*   rcore_desktop_glfw - Functions to manage window, graphics device and inputs
*
*   PLATFORM: DESKTOP: GLFW
*       - Windows (Win32, Win64)
*       - Linux (X11/Wayland desktop mode)
*       - FreeBSD, OpenBSD, NetBSD, DragonFly (X11 desktop)
*       - OSX/macOS (x64, arm64)
*
*   LIMITATIONS:
*       - Limitation 01
*       - Limitation 02
*
*   POSSIBLE IMPROVEMENTS:
*       - Improvement 01
*       - Improvement 02
*
*   CONFIGURATION:
*       #define RCORE_PLATFORM_CUSTOM_FLAG
*           Custom flag for rcore on target platform -not used-
*
*   DEPENDENCIES:
*       - rglfw: Manage graphic device, OpenGL context and inputs (Windows, Linux, OSX/macOS, FreeBSD...)
*       - gestures: Gestures system for touch-ready devices (or simulated from mouse inputs)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2026 Ramon Santamaria (@raysan5) and contributors
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

#define GLFW_INCLUDE_NONE       // Disable the standard OpenGL header inclusion on GLFW3
                                // NOTE: Already provided by rlgl implementation (on glad.h)
#include "GLFW/glfw3.h"         // GLFW3 library: Windows, OpenGL context and Input management
                                // NOTE: GLFW3 already includes gl.h (OpenGL) headers

// Support retrieving native window handlers
#if defined(_WIN32)
    #if !defined(HWND) && !defined(_MSVC_LANG)
        #define HWND void*
    #elif !defined(HWND) && defined(_MSVC_LANG)
        typedef struct HWND__ *HWND;
    #endif

    #include "../external/win32_clipboard.h" // Clipboard image copy-paste

    #define GLFW_EXPOSE_NATIVE_WIN32
    #define GLFW_NATIVE_INCLUDE_NONE // To avoid some symbols re-definition in windows.h
    #include "GLFW/glfw3native.h"

    #if defined(SUPPORT_WINMM_HIGHRES_TIMER) && !defined(SUPPORT_BUSY_WAIT_LOOP)
        // NOTE: Those functions require linking with winmm library
        //#pragma warning(disable: 4273)
        __declspec(dllimport) unsigned int __stdcall timeEndPeriod(unsigned int uPeriod);
        //#pragma warning(default: 4273)
    #endif
#endif
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
    #include <sys/time.h>               // Required for: timespec, nanosleep(), select() - POSIX

    #if defined(_GLFW_X11) || defined(_GLFW_WAYLAND)
                                        // Set appropriate expose macros based on available backends
        #if defined(_GLFW_X11)
            #define GLFW_EXPOSE_NATIVE_X11
                #define Font X11Font    // Hack to fix 'Font' name collision
                                        // The definition and references to the X11 Font type will be replaced by 'X11Font'
                                        // Works as long as the current file consistently references any X11 Font as X11Font
                                        // Since it is never referenced (as of writing), this does not pose an issue
        #endif

        #if defined(_GLFW_WAYLAND)
            #define GLFW_EXPOSE_NATIVE_WAYLAND
        #endif

        #include "GLFW/glfw3native.h"   // Include native header only once, regardless of how many backends are defined
                                        // Required for: glfwGetX11Window() and glfwGetWaylandWindow()
        #if defined(_GLFW_X11)          // Clean up X11-specific hacks
            #undef Font                 // Revert hack and allow normal raylib Font usage
        #endif
    #endif
#endif
#if defined(__APPLE__)
    #include <unistd.h>                 // Required for: usleep()

    //#define GLFW_EXPOSE_NATIVE_COCOA    // WARNING: Fails due to type redefinition
    void *glfwGetCocoaWindow(GLFWwindow* handle);
    #include "GLFW/glfw3native.h"       // Required for: glfwGetCocoaWindow()
#endif

#include <stddef.h>  // Required for: size_t

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    GLFWwindow *handle;                 // GLFW window handle (graphic device)
} PlatformData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern CoreData CORE;                   // Global CORE state context

static PlatformData platform = { 0 };   // Platform specific data

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
int InitPlatform(void);          // Initialize platform (graphics, inputs and more)
void ClosePlatform(void);        // Close platform

// Error callback event
static void ErrorCallback(int error, const char *description);                          // GLFW3 Error Callback, runs on GLFW3 error

// Window callbacks events
static void WindowSizeCallback(GLFWwindow *window, int width, int height);              // GLFW3 WindowSize Callback, runs when window is resized
static void FramebufferSizeCallback(GLFWwindow *window, int width, int height);         // GLFW3 FramebufferSize Callback, runs when window is resized
static void WindowContentScaleCallback(GLFWwindow *window, float scalex, float scaley); // GLFW3 Window Content Scale Callback, runs when window changes scale
static void WindowPosCallback(GLFWwindow *window, int x, int y);                        // GLFW3 WindowPos Callback, runs when window is moved
static void WindowIconifyCallback(GLFWwindow *window, int iconified);                   // GLFW3 WindowIconify Callback, runs when window is minimized/restored
static void WindowMaximizeCallback(GLFWwindow *window, int maximized);                  // GLFW3 Window Maximize Callback, runs when window is maximized
static void WindowFocusCallback(GLFWwindow *window, int focused);                       // GLFW3 WindowFocus Callback, runs when window get/lose focus
static void WindowDropCallback(GLFWwindow *window, int count, const char **paths);      // GLFW3 Window Drop Callback, runs when drop files into window

// Input callbacks events
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods); // GLFW3 Keyboard Callback, runs on key pressed
static void CharCallback(GLFWwindow *window, unsigned int codepoint);                   // GLFW3 Char Callback, runs on key pressed (get codepoint value)
static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);  // GLFW3 Mouse Button Callback, runs on mouse button pressed
static void MouseCursorPosCallback(GLFWwindow *window, double x, double y);             // GLFW3 Cursor Position Callback, runs on mouse move
static void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);    // GLFW3 Scrolling Callback, runs on mouse wheel
static void CursorEnterCallback(GLFWwindow *window, int enter);                         // GLFW3 Cursor Enter Callback, cursor enters client area
static void JoystickCallback(int jid, int event);                                       // GLFW3 Joystick Connected/Disconnected Callback

// Memory allocator wrappers [used by glfwInitAllocator()]
static void *AllocateWrapper(size_t size, void *user);                                  // GLFW3 GLFWallocatefun, wrapps around RL_CALLOC macro
static void *ReallocateWrapper(void *block, size_t size, void *user);                   // GLFW3 GLFWreallocatefun, wrapps around RL_REALLOC macro
static void DeallocateWrapper(void *block, void *user);                                 // GLFW3 GLFWdeallocatefun, wraps around RL_FREE macro

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
// NOTE: Functions declaration is provided by raylib.h

//----------------------------------------------------------------------------------
// Module Functions Definition: Window and Graphics Device
//----------------------------------------------------------------------------------

// Check if application should close
// NOTE: By default, if KEY_ESCAPE pressed or window close icon clicked
bool WindowShouldClose(void)
{
    if (CORE.Window.ready) return CORE.Window.shouldClose;
    else return true;
}

// Toggle fullscreen mode
void ToggleFullscreen(void)
{
    if (!FLAG_IS_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE))
    {
        // Store previous screen data (in case exiting fullscreen)
        CORE.Window.previousPosition = CORE.Window.position;
        CORE.Window.previousScreen = CORE.Window.screen;

        // Use current monitor the window is on to get fullscreen required size
        int monitorCount = 0;
        int monitorIndex = GetCurrentMonitor();
        GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);
        GLFWmonitor *monitor = (monitorIndex < monitorCount)? monitors[monitorIndex] : NULL;

        if (monitor != NULL)
        {
            // Get current monitor video mode
            const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitorIndex]);
            CORE.Window.display.width = mode->width;
            CORE.Window.display.height = mode->height;

            CORE.Window.position = (Point){ 0, 0 };
            CORE.Window.screen = CORE.Window.display;

            // Set fullscreen flag to be processed on FramebufferSizeCallback() accordingly
            FLAG_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE);

#if defined(_GLFW_X11) || defined(_GLFW_WAYLAND)
            // NOTE: X11 requires undecorating the window before switching to
            // fullscreen to avoid issues with framebuffer scaling
            glfwSetWindowAttrib(platform.handle, GLFW_DECORATED, GLFW_FALSE);
            FLAG_SET(CORE.Window.flags, FLAG_WINDOW_UNDECORATED);
#endif
            // WARNING: This function launches FramebufferSizeCallback()
            glfwSetWindowMonitor(platform.handle, monitor, 0, 0, CORE.Window.screen.width, CORE.Window.screen.height, GLFW_DONT_CARE);
        }
        else TRACELOG(LOG_WARNING, "GLFW: Failed to get monitor");
    }
    else
    {
        // Restore previous window position and size
        CORE.Window.position = CORE.Window.previousPosition;
        CORE.Window.screen = CORE.Window.previousScreen;

        // Set fullscreen flag to be processed on FramebufferSizeCallback() accordingly
        // and considered by GetWindowScaleDPI()
        FLAG_CLEAR(CORE.Window.flags, FLAG_FULLSCREEN_MODE);

#if !defined(__APPLE__)
        // Make sure to restore render size considering HighDPI scaling
        if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIGHDPI))
        {
            Vector2 scaleDpi = GetWindowScaleDPI();
            CORE.Window.screen.width = (unsigned int)(CORE.Window.screen.width * scaleDpi.x);
            CORE.Window.screen.height = (unsigned int)(CORE.Window.screen.height * scaleDpi.y);
        }
#endif

        // WARNING: This function launches FramebufferSizeCallback()
        glfwSetWindowMonitor(platform.handle, NULL, CORE.Window.position.x, CORE.Window.position.y,
            CORE.Window.screen.width, CORE.Window.screen.height, GLFW_DONT_CARE);

#if defined(_GLFW_X11) || defined(_GLFW_WAYLAND)
        // NOTE: X11 requires restoring the decorated window after switching from
        // fullscreen to avoid issues with framebuffer scaling
        glfwSetWindowAttrib(platform.handle, GLFW_DECORATED, GLFW_TRUE);
        FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_UNDECORATED);
#endif
    }

    // Try to enable GPU V-Sync, so frames are limited to screen refresh rate (60Hz -> 60 FPS)
    // NOTE: V-Sync can be enabled by graphic driver configuration
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_VSYNC_HINT)) glfwSwapInterval(1);
}

// Toggle borderless windowed mode
void ToggleBorderlessWindowed(void)
{
    // Leave fullscreen before attempting to set borderless windowed mode
    // NOTE: Fullscreen already saves the previous position so it does not need to be set again later
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE)) ToggleFullscreen();

    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);
    const int monitor = GetCurrentMonitor();

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);

        if (mode != NULL)
        {
            if (!FLAG_IS_SET(CORE.Window.flags, FLAG_BORDERLESS_WINDOWED_MODE))
            {
                // Store screen position and size
                // NOTE: If it was on fullscreen, screen position was already stored, so skip setting it here
                CORE.Window.previousPosition = CORE.Window.position;
                CORE.Window.previousScreen = CORE.Window.screen;

                // Set undecorated flag
                glfwSetWindowAttrib(platform.handle, GLFW_DECORATED, GLFW_FALSE);
                FLAG_SET(CORE.Window.flags, FLAG_WINDOW_UNDECORATED);

                // Get monitor position and size
                glfwGetMonitorPos(monitors[monitor], &CORE.Window.position.x, &CORE.Window.position.y);
                CORE.Window.screen.width = mode->width;
                CORE.Window.screen.height = mode->height;

                // Set screen position and size
                glfwSetWindowMonitor(platform.handle, monitors[monitor], CORE.Window.position.x, CORE.Window.position.y,
                    CORE.Window.screen.width, CORE.Window.screen.height, mode->refreshRate);

                // Refocus window
                glfwFocusWindow(platform.handle);

                FLAG_SET(CORE.Window.flags, FLAG_BORDERLESS_WINDOWED_MODE);
            }
            else
            {
                // Restore previous screen values
                CORE.Window.position = CORE.Window.previousPosition;
                CORE.Window.screen = CORE.Window.previousScreen;

                // Remove undecorated flag
                glfwSetWindowAttrib(platform.handle, GLFW_DECORATED, GLFW_TRUE);
                FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_UNDECORATED);

            #if !defined(__APPLE__)
                // Make sure to restore size considering HighDPI scaling
                if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIGHDPI))
                {
                    Vector2 scaleDpi = GetWindowScaleDPI();
                    CORE.Window.screen.width = (unsigned int)(CORE.Window.screen.width * scaleDpi.x);
                    CORE.Window.screen.height = (unsigned int)(CORE.Window.screen.height * scaleDpi.y);
                }
            #endif

                // Return to previous screen size and position
                glfwSetWindowMonitor(platform.handle, NULL, CORE.Window.position.x, CORE.Window.position.y,
                    CORE.Window.screen.width, CORE.Window.screen.height, mode->refreshRate);

                // Refocus window
                glfwFocusWindow(platform.handle);

                FLAG_CLEAR(CORE.Window.flags, FLAG_BORDERLESS_WINDOWED_MODE);
            }
        }
        else TRACELOG(LOG_WARNING, "GLFW: Failed to find video mode for selected monitor");
    }
    else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
}

// Set window state: maximized, if resizable
void MaximizeWindow(void)
{
    if (glfwGetWindowAttrib(platform.handle, GLFW_RESIZABLE) == GLFW_TRUE)
    {
        glfwMaximizeWindow(platform.handle);
        FLAG_SET(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED);
    }
}

// Set window state: minimized
void MinimizeWindow(void)
{
    // NOTE: Following function launches callback that sets appropriate flag!
    glfwIconifyWindow(platform.handle);
}

// Restore window from being minimized/maximized
void RestoreWindow(void)
{
    if (glfwGetWindowAttrib(platform.handle, GLFW_RESIZABLE) == GLFW_TRUE)
    {
        // Restores the specified window if it was previously iconified (minimized) or maximized
        glfwRestoreWindow(platform.handle);
        FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_MINIMIZED);
        FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED);
    }
}

// Set window configuration state using flags
void SetWindowState(unsigned int flags)
{
    if (!CORE.Window.ready) TRACELOG(LOG_WARNING, "WINDOW: SetWindowState does nothing before window initialization, Use \"SetConfigFlags\" instead");

    // Check previous state and requested state to apply required changes
    // NOTE: In most cases the functions already change the flags internally

    // State change: FLAG_VSYNC_HINT
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_VSYNC_HINT) != FLAG_IS_SET(flags, FLAG_VSYNC_HINT)) && FLAG_IS_SET(flags, FLAG_VSYNC_HINT))
    {
        glfwSwapInterval(1);
        FLAG_SET(CORE.Window.flags, FLAG_VSYNC_HINT);
    }

    // State change: FLAG_BORDERLESS_WINDOWED_MODE
    // NOTE: This must be handled before FLAG_FULLSCREEN_MODE because ToggleBorderlessWindowed() needs to get some fullscreen values if fullscreen is running
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_BORDERLESS_WINDOWED_MODE) != FLAG_IS_SET(flags, FLAG_BORDERLESS_WINDOWED_MODE)) && FLAG_IS_SET(flags, FLAG_BORDERLESS_WINDOWED_MODE))
    {
        ToggleBorderlessWindowed();     // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_FULLSCREEN_MODE
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE) != FLAG_IS_SET(flags, FLAG_FULLSCREEN_MODE)) && FLAG_IS_SET(flags, FLAG_FULLSCREEN_MODE))
    {
        ToggleFullscreen();     // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_RESIZABLE
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_RESIZABLE) != FLAG_IS_SET(flags, FLAG_WINDOW_RESIZABLE)) && FLAG_IS_SET(flags, FLAG_WINDOW_RESIZABLE))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_RESIZABLE, GLFW_TRUE);
        FLAG_SET(CORE.Window.flags, FLAG_WINDOW_RESIZABLE);
    }

    // State change: FLAG_WINDOW_UNDECORATED
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_UNDECORATED) != FLAG_IS_SET(flags, FLAG_WINDOW_UNDECORATED)) && FLAG_IS_SET(flags, FLAG_WINDOW_UNDECORATED))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_DECORATED, GLFW_FALSE);
        FLAG_SET(CORE.Window.flags, FLAG_WINDOW_UNDECORATED);
    }

    // State change: FLAG_WINDOW_HIDDEN
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIDDEN) != FLAG_IS_SET(flags, FLAG_WINDOW_HIDDEN)) && FLAG_IS_SET(flags, FLAG_WINDOW_HIDDEN))
    {
        glfwHideWindow(platform.handle);
        FLAG_SET(CORE.Window.flags, FLAG_WINDOW_HIDDEN);
    }

    // State change: FLAG_WINDOW_MINIMIZED
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MINIMIZED) != FLAG_IS_SET(flags, FLAG_WINDOW_MINIMIZED)) && FLAG_IS_SET(flags, FLAG_WINDOW_MINIMIZED))
    {
        //GLFW_ICONIFIED
        MinimizeWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_MAXIMIZED
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED) != FLAG_IS_SET(flags, FLAG_WINDOW_MAXIMIZED)) && FLAG_IS_SET(flags, FLAG_WINDOW_MAXIMIZED))
    {
        //GLFW_MAXIMIZED
        MaximizeWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_UNFOCUSED
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_UNFOCUSED) != FLAG_IS_SET(flags, FLAG_WINDOW_UNFOCUSED)) && FLAG_IS_SET(flags, FLAG_WINDOW_UNFOCUSED))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
        FLAG_SET(CORE.Window.flags, FLAG_WINDOW_UNFOCUSED);
    }

    // State change: FLAG_WINDOW_TOPMOST
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_TOPMOST) != FLAG_IS_SET(flags, FLAG_WINDOW_TOPMOST)) && FLAG_IS_SET(flags, FLAG_WINDOW_TOPMOST))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_FLOATING, GLFW_TRUE);
        FLAG_SET(CORE.Window.flags, FLAG_WINDOW_TOPMOST);
    }

    // State change: FLAG_WINDOW_ALWAYS_RUN
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_ALWAYS_RUN) != FLAG_IS_SET(flags, FLAG_WINDOW_ALWAYS_RUN)) && FLAG_IS_SET(flags, FLAG_WINDOW_ALWAYS_RUN))
    {
        FLAG_SET(CORE.Window.flags, FLAG_WINDOW_ALWAYS_RUN);
    }

    // The following states can not be changed after window creation

    // State change: FLAG_WINDOW_TRANSPARENT
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_TRANSPARENT) != FLAG_IS_SET(flags, FLAG_WINDOW_TRANSPARENT)) && FLAG_IS_SET(flags, FLAG_WINDOW_TRANSPARENT))
    {
        TRACELOG(LOG_WARNING, "WINDOW: Framebuffer transparency can only be configured before window initialization");
    }

    // State change: FLAG_WINDOW_HIGHDPI
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIGHDPI) != FLAG_IS_SET(flags, FLAG_WINDOW_HIGHDPI)) && FLAG_IS_SET(flags, FLAG_WINDOW_HIGHDPI))
    {
        TRACELOG(LOG_WARNING, "WINDOW: High DPI can only be configured before window initialization");
    }

    // State change: FLAG_WINDOW_MOUSE_PASSTHROUGH
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MOUSE_PASSTHROUGH) != FLAG_IS_SET(flags, FLAG_WINDOW_MOUSE_PASSTHROUGH)) && FLAG_IS_SET(flags, FLAG_WINDOW_MOUSE_PASSTHROUGH))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
        FLAG_SET(CORE.Window.flags, FLAG_WINDOW_MOUSE_PASSTHROUGH);
    }

    // State change: FLAG_MSAA_4X_HINT
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_MSAA_4X_HINT) != FLAG_IS_SET(flags, FLAG_MSAA_4X_HINT)) && FLAG_IS_SET(flags, FLAG_MSAA_4X_HINT))
    {
        TRACELOG(LOG_WARNING, "WINDOW: MSAA can only be configured before window initialization");
    }

    // State change: FLAG_INTERLACED_HINT
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_INTERLACED_HINT) != FLAG_IS_SET(flags, FLAG_INTERLACED_HINT)) && FLAG_IS_SET(flags, FLAG_INTERLACED_HINT))
    {
        TRACELOG(LOG_WARNING, "WINDOW: Interlaced mode can only be configured before window initialization");
    }
}

// Clear window configuration state flags
void ClearWindowState(unsigned int flags)
{
    // Check previous state and requested state to apply required changes
    // NOTE: In most cases the functions already change the flags internally

    // State change: FLAG_VSYNC_HINT
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_VSYNC_HINT)) && (FLAG_IS_SET(flags, FLAG_VSYNC_HINT)))
    {
        glfwSwapInterval(0);
        FLAG_CLEAR(CORE.Window.flags, FLAG_VSYNC_HINT);
    }

    // State change: FLAG_BORDERLESS_WINDOWED_MODE
    // NOTE: This must be handled before FLAG_FULLSCREEN_MODE because ToggleBorderlessWindowed() needs to get some fullscreen values if fullscreen is running
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_BORDERLESS_WINDOWED_MODE)) && (FLAG_IS_SET(flags, FLAG_BORDERLESS_WINDOWED_MODE)))
    {
        ToggleBorderlessWindowed(); // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_FULLSCREEN_MODE
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE)) && (FLAG_IS_SET(flags, FLAG_FULLSCREEN_MODE)))
    {
        ToggleFullscreen(); // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_RESIZABLE
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_RESIZABLE)) && (FLAG_IS_SET(flags, FLAG_WINDOW_RESIZABLE)))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_RESIZABLE, GLFW_FALSE);
        FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_RESIZABLE);
    }

    // State change: FLAG_WINDOW_HIDDEN
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIDDEN)) && (FLAG_IS_SET(flags, FLAG_WINDOW_HIDDEN)))
    {
        glfwShowWindow(platform.handle);
        FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_HIDDEN);
    }

    // State change: FLAG_WINDOW_MINIMIZED
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MINIMIZED)) && (FLAG_IS_SET(flags, FLAG_WINDOW_MINIMIZED)))
    {
        RestoreWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_MAXIMIZED
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED)) && (FLAG_IS_SET(flags, FLAG_WINDOW_MAXIMIZED)))
    {
        RestoreWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_UNDECORATED
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_UNDECORATED)) && (FLAG_IS_SET(flags, FLAG_WINDOW_UNDECORATED)))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_DECORATED, GLFW_TRUE);
        FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_UNDECORATED);
    }

    // State change: FLAG_WINDOW_UNFOCUSED
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_UNFOCUSED)) && (FLAG_IS_SET(flags, FLAG_WINDOW_UNFOCUSED)))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
        FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_UNFOCUSED);
    }

    // State change: FLAG_WINDOW_TOPMOST
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_TOPMOST)) && (FLAG_IS_SET(flags, FLAG_WINDOW_TOPMOST)))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_FLOATING, GLFW_FALSE);
        FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_TOPMOST);
    }

    // State change: FLAG_WINDOW_ALWAYS_RUN
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_ALWAYS_RUN)) && (FLAG_IS_SET(flags, FLAG_WINDOW_ALWAYS_RUN)))
    {
        FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_ALWAYS_RUN);
    }

    // The following states can not be changed after window creation

    // State change: FLAG_WINDOW_TRANSPARENT
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_TRANSPARENT)) && (FLAG_IS_SET(flags, FLAG_WINDOW_TRANSPARENT)))
    {
        TRACELOG(LOG_WARNING, "WINDOW: Framebuffer transparency can only be configured before window initialization");
    }

    // State change: FLAG_WINDOW_HIGHDPI
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIGHDPI)) && (FLAG_IS_SET(flags, FLAG_WINDOW_HIGHDPI)))
    {
        TRACELOG(LOG_WARNING, "WINDOW: High DPI can only be configured before window initialization");
    }

    // State change: FLAG_WINDOW_MOUSE_PASSTHROUGH
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MOUSE_PASSTHROUGH)) && (FLAG_IS_SET(flags, FLAG_WINDOW_MOUSE_PASSTHROUGH)))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_MOUSE_PASSTHROUGH, GLFW_FALSE);
        FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_MOUSE_PASSTHROUGH);
    }

    // State change: FLAG_MSAA_4X_HINT
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_MSAA_4X_HINT)) && (FLAG_IS_SET(flags, FLAG_MSAA_4X_HINT)))
    {
        TRACELOG(LOG_WARNING, "WINDOW: MSAA can only be configured before window initialization");
    }

    // State change: FLAG_INTERLACED_HINT
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_INTERLACED_HINT)) && (FLAG_IS_SET(flags, FLAG_INTERLACED_HINT)))
    {
        TRACELOG(LOG_WARNING, "RPI: Interlaced mode can only be configured before window initialization");
    }
}

// Set icon for window
// NOTE 1: Image must be in RGBA format, 8bit per channel
// NOTE 2: Image is scaled by the OS for all required sizes
void SetWindowIcon(Image image)
{
    if (image.data == NULL)
    {
        // Revert to the default window icon, pass in an empty image array
        glfwSetWindowIcon(platform.handle, 0, NULL);
    }
    else
    {
        if (image.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
        {
            GLFWimage icon[1] = { 0 };

            icon[0].width = image.width;
            icon[0].height = image.height;
            icon[0].pixels = (unsigned char *)image.data;

            // NOTE 1: Only one image icon supported
            // NOTE 2: The specified image data is copied before this function returns
            glfwSetWindowIcon(platform.handle, 1, icon);
        }
        else TRACELOG(LOG_WARNING, "GLFW: Window icon image must be in R8G8B8A8 pixel format");
    }
}

// Set icon for window, multiple images
// NOTE 1: Images must be in RGBA format, 8bit per channel
// NOTE 2: The multiple images are used depending on provided sizes
// Standard Windows icon sizes: 256, 128, 96, 64, 48, 32, 24, 16
void SetWindowIcons(Image *images, int count)
{
    if ((images == NULL) || (count <= 0))
    {
        // Revert to the default window icon, pass in an empty image array
        glfwSetWindowIcon(platform.handle, 0, NULL);
    }
    else
    {
        int valid = 0;
        GLFWimage *icons = (GLFWimage *)RL_CALLOC(count, sizeof(GLFWimage));

        for (int i = 0; i < count; i++)
        {
            if (images[i].format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
            {
                icons[valid].width = images[i].width;
                icons[valid].height = images[i].height;
                icons[valid].pixels = (unsigned char *)images[i].data;

                valid++;
            }
            else TRACELOG(LOG_WARNING, "GLFW: Window icon image must be in R8G8B8A8 pixel format");
        }
        // NOTE: Images data is copied internally before this function returns
        glfwSetWindowIcon(platform.handle, valid, icons);

        RL_FREE(icons);
    }
}

// Set title for window
void SetWindowTitle(const char *title)
{
    CORE.Window.title = title;
    glfwSetWindowTitle(platform.handle, title);
}

// Set window position on screen (windowed mode)
void SetWindowPosition(int x, int y)
{
    // Update CORE.Window.position as well
    CORE.Window.position.x = x;
    CORE.Window.position.y = y;
    glfwSetWindowPos(platform.handle, x, y);
}

// Set monitor for the current window
void SetWindowMonitor(int monitor)
{
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        if (FLAG_IS_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE))
        {
            TRACELOG(LOG_INFO, "GLFW: Selected fullscreen monitor: [%i] %s", monitor, glfwGetMonitorName(monitors[monitor]));

            const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);
            glfwSetWindowMonitor(platform.handle, monitors[monitor], 0, 0, mode->width, mode->height, mode->refreshRate);
        }
        else
        {
            TRACELOG(LOG_INFO, "GLFW: Selected monitor: [%i] %s", monitor, glfwGetMonitorName(monitors[monitor]));

            // Here the render width has to be used again in case high dpi flag is enabled
            const int screenWidth = CORE.Window.render.width;
            const int screenHeight = CORE.Window.render.height;
            int monitorWorkareaX = 0;
            int monitorWorkareaY = 0;
            int monitorWorkareaWidth = 0;
            int monitorWorkareaHeight = 0;
            glfwGetMonitorWorkarea(monitors[monitor], &monitorWorkareaX, &monitorWorkareaY, &monitorWorkareaWidth, &monitorWorkareaHeight);

            // If the screen size is larger than the monitor workarea, anchor it on the top left corner, otherwise, center it
            if ((screenWidth >= monitorWorkareaWidth) || (screenHeight >= monitorWorkareaHeight)) glfwSetWindowPos(platform.handle, monitorWorkareaX, monitorWorkareaY);
            else
            {
                const int x = monitorWorkareaX + (monitorWorkareaWidth/2) - (screenWidth/2);
                const int y = monitorWorkareaY + (monitorWorkareaHeight/2) - (screenHeight/2);
                glfwSetWindowPos(platform.handle, x, y);
            }
        }
    }
    else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
}

// Set window minimum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMinSize(int width, int height)
{
    CORE.Window.screenMin.width = width;
    CORE.Window.screenMin.height = height;

    int minWidth  = (CORE.Window.screenMin.width  == 0)? GLFW_DONT_CARE : (int)CORE.Window.screenMin.width;
    int minHeight = (CORE.Window.screenMin.height == 0)? GLFW_DONT_CARE : (int)CORE.Window.screenMin.height;
    int maxWidth  = (CORE.Window.screenMax.width  == 0)? GLFW_DONT_CARE : (int)CORE.Window.screenMax.width;
    int maxHeight = (CORE.Window.screenMax.height == 0)? GLFW_DONT_CARE : (int)CORE.Window.screenMax.height;

    glfwSetWindowSizeLimits(platform.handle, minWidth, minHeight, maxWidth, maxHeight);
}

// Set window maximum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMaxSize(int width, int height)
{
    CORE.Window.screenMax.width = width;
    CORE.Window.screenMax.height = height;

    int minWidth  = (CORE.Window.screenMin.width  == 0)? GLFW_DONT_CARE : (int)CORE.Window.screenMin.width;
    int minHeight = (CORE.Window.screenMin.height == 0)? GLFW_DONT_CARE : (int)CORE.Window.screenMin.height;
    int maxWidth  = (CORE.Window.screenMax.width  == 0)? GLFW_DONT_CARE : (int)CORE.Window.screenMax.width;
    int maxHeight = (CORE.Window.screenMax.height == 0)? GLFW_DONT_CARE : (int)CORE.Window.screenMax.height;

    glfwSetWindowSizeLimits(platform.handle, minWidth, minHeight, maxWidth, maxHeight);
}

// Set window dimensions
void SetWindowSize(int width, int height)
{
    CORE.Window.screen.width = width;
    CORE.Window.screen.height = height;

    glfwSetWindowSize(platform.handle, width, height);
}

// Set window opacity, value opacity is between 0.0 and 1.0
void SetWindowOpacity(float opacity)
{
    if (opacity >= 1.0f) opacity = 1.0f;
    else if (opacity <= 0.0f) opacity = 0.0f;
    glfwSetWindowOpacity(platform.handle, opacity);
}

// Set window focused
void SetWindowFocused(void)
{
    glfwFocusWindow(platform.handle);
}

#if defined(__linux__) && defined(_GLFW_X11)
// Local storage for the window handle returned by glfwGetX11Window
// This is needed as X11 handles are integers and may not fit inside a pointer depending on platform
// Storing the handle locally and returning a pointer in GetWindowHandle allows the code to work regardless of pointer width
static XID X11WindowHandle;
#endif
// Get native window handle
void *GetWindowHandle(void)
{
#if defined(_WIN32)
    // NOTE: Returned handle is: void *HWND (windows.h)
    return glfwGetWin32Window(platform.handle);
#endif
#if defined(__linux__)
    #if defined(_GLFW_WAYLAND)
        #if defined(_GLFW_X11)
            int platformID = glfwGetPlatform();
            if (platformID == GLFW_PLATFORM_WAYLAND)
            {
                return glfwGetWaylandWindow(platform.handle);
            }
            else
            {
                X11WindowHandle = glfwGetX11Window(platform.handle);
                return &X11WindowHandle;
            }
        #else
            return glfwGetWaylandWindow(platform.handle);
        #endif
    #elif defined(_GLFW_X11)
        // Store the window handle localy and return a pointer to the variable instead
        // Reasoning detailed in the declaration of X11WindowHandle
        X11WindowHandle = glfwGetX11Window(platform.handle);
        return &X11WindowHandle;
    #endif
#endif
#if defined(__APPLE__)
    // NOTE: Returned handle is: (objc_object *)
    return (void *)glfwGetCocoaWindow(platform.handle);
#endif

    return NULL;
}

// Get number of monitors
int GetMonitorCount(void)
{
    int monitorCount = 0;

    glfwGetMonitors(&monitorCount);

    return monitorCount;
}

// Get current monitor where window is placed
int GetCurrentMonitor(void)
{
    int index = 0;
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);
    GLFWmonitor *monitor = NULL;

    if (monitorCount >= 1)
    {
        if (IsWindowFullscreen())
        {
            // Get the handle of the monitor that the specified window is in full screen on
            monitor = glfwGetWindowMonitor(platform.handle);

            for (int i = 0; i < monitorCount; i++)
            {
                if (monitors[i] == monitor)
                {
                    index = i;
                    break;
                }
            }
        }
        else
        {
            // In case the window is between two monitors, below logic is used
            // to try to detect the "current monitor" for that window, note that
            // this is probably an overengineered solution for a very side case
            // trying to match SDL behaviour

            int closestDist = 0x7FFFFFFF;

            // Window center position
            int wcx = 0;
            int wcy = 0;

            glfwGetWindowPos(platform.handle, &wcx, &wcy);
            wcx += (int)CORE.Window.screen.width/2;
            wcy += (int)CORE.Window.screen.height/2;

            for (int i = 0; i < monitorCount; i++)
            {
                // Monitor top-left position
                int mx = 0;
                int my = 0;

                monitor = monitors[i];
                glfwGetMonitorPos(monitor, &mx, &my);
                const GLFWvidmode *mode = glfwGetVideoMode(monitor);

                if (mode)
                {
                    const int right = mx + mode->width - 1;
                    const int bottom = my + mode->height - 1;

                    if ((wcx >= mx) &&
                        (wcx <= right) &&
                        (wcy >= my) &&
                        (wcy <= bottom))
                    {
                        index = i;
                        break;
                    }

                    int xclosest = wcx;
                    if (wcx < mx) xclosest = mx;
                    else if (wcx > right) xclosest = right;

                    int yclosest = wcy;
                    if (wcy < my) yclosest = my;
                    else if (wcy > bottom) yclosest = bottom;

                    int dx = wcx - xclosest;
                    int dy = wcy - yclosest;
                    int dist = (dx*dx) + (dy*dy);
                    if (dist < closestDist)
                    {
                        index = i;
                        closestDist = dist;
                    }
                }
                else TRACELOG(LOG_WARNING, "GLFW: Failed to find video mode for selected monitor");
            }
        }
    }

    return index;
}

// Get selected monitor position
Vector2 GetMonitorPosition(int monitor)
{
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        int x = 0;
        int y = 0;
        glfwGetMonitorPos(monitors[monitor], &x, &y);

        return (Vector2){ (float)x, (float)y };
    }
    else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
    return (Vector2){ 0, 0 };
}

// Get selected monitor width (currently used by monitor)
int GetMonitorWidth(int monitor)
{
    int width = 0;
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);

        if (mode) width = mode->width;
        else TRACELOG(LOG_WARNING, "GLFW: Failed to find video mode for selected monitor");
    }
    else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");

    return width;
}

// Get selected monitor height (currently used by monitor)
int GetMonitorHeight(int monitor)
{
    int height = 0;
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);

        if (mode) height = mode->height;
        else TRACELOG(LOG_WARNING, "GLFW: Failed to find video mode for selected monitor");
    }
    else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");

    return height;
}

// Get selected monitor physical width in millimetres
int GetMonitorPhysicalWidth(int monitor)
{
    int width = 0;
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount)) glfwGetMonitorPhysicalSize(monitors[monitor], &width, NULL);
    else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");

    return width;
}

// Get selected monitor physical height in millimetres
int GetMonitorPhysicalHeight(int monitor)
{
    int height = 0;
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount)) glfwGetMonitorPhysicalSize(monitors[monitor], NULL, &height);
    else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");

    return height;
}

// Get selected monitor refresh rate
int GetMonitorRefreshRate(int monitor)
{
    int refresh = 0;
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);

        if (mode) refresh = mode->refreshRate;
        else TRACELOG(LOG_WARNING, "GLFW: Failed to find video mode for selected monitor");

    }
    else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");

    return refresh;
}

// Get the human-readable, UTF-8 encoded name of the selected monitor
const char *GetMonitorName(int monitor)
{
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        return glfwGetMonitorName(monitors[monitor]);
    }
    else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
    return "";
}

// Get window position XY on monitor
Vector2 GetWindowPosition(void)
{
    return (Vector2){ (float)CORE.Window.position.x, (float)CORE.Window.position.y };
}

// Get window scale DPI factor for current monitor
Vector2 GetWindowScaleDPI(void)
{
    Vector2 scale = { 1.0f, 1.0f };
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIGHDPI) && !FLAG_IS_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE))
        glfwGetWindowContentScale(platform.handle, &scale.x, &scale.y);
    return scale;
}

// Set clipboard text content
void SetClipboardText(const char *text)
{
    glfwSetClipboardString(platform.handle, text);
}

// Get clipboard text content
// NOTE: returned string is allocated and freed by GLFW
const char *GetClipboardText(void)
{
    return glfwGetClipboardString(platform.handle);
}

// Get clipboard image
Image GetClipboardImage(void)
{
    Image image = { 0 };

#if defined(SUPPORT_CLIPBOARD_IMAGE)
#if defined(_WIN32)
    unsigned long long int dataSize = 0;
    void *fileData = NULL;
    int width = 0;
    int height = 0;

    fileData  = (void *)Win32GetClipboardImageData(&width, &height, &dataSize);

    if (fileData == NULL) TRACELOG(LOG_WARNING, "Clipboard image: Couldn't get clipboard data.");
    else image = LoadImageFromMemory(".bmp", (const unsigned char *)fileData, (int)dataSize);
#else
    TRACELOG(LOG_WARNING, "GetClipboardImage() not implemented on target platform");
#endif
#endif // SUPPORT_CLIPBOARD_IMAGE

    return image;
}

// Show mouse cursor
void ShowCursor(void)
{
    glfwSetInputMode(platform.handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    CORE.Input.Mouse.cursorHidden = false;
}

// Hides mouse cursor
void HideCursor(void)
{
    glfwSetInputMode(platform.handle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    CORE.Input.Mouse.cursorHidden = true;
}

// Enables cursor (unlock cursor)
void EnableCursor(void)
{
    glfwSetInputMode(platform.handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Set cursor position in the middle
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);

    if (glfwRawMouseMotionSupported()) glfwSetInputMode(platform.handle, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);

    CORE.Input.Mouse.cursorHidden = false;
    CORE.Input.Mouse.cursorLocked = false;
}

// Disables cursor (lock cursor)
void DisableCursor(void)
{
    // Reset mouse position within the window area before disabling cursor
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);

    glfwSetInputMode(platform.handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glfwRawMouseMotionSupported()) glfwSetInputMode(platform.handle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    CORE.Input.Mouse.cursorHidden = true;
    CORE.Input.Mouse.cursorLocked = true;
}

// Swap back buffer with front buffer (screen drawing)
void SwapScreenBuffer(void)
{
    glfwSwapBuffers(platform.handle);
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Misc
//----------------------------------------------------------------------------------

// Get elapsed time measure in seconds since InitTimer()
double GetTime(void)
{
    double time = glfwGetTime();   // Elapsed time since glfwInit()
    return time;
}

// Open URL with default system browser (if available)
// NOTE: This function is only safe to use if you control the URL given
// A user could craft a malicious string performing another action
// Only call this function yourself not with user input or make sure to check the string yourself
// REF: https://github.com/raysan5/raylib/issues/686
void OpenURL(const char *url)
{
    // Security check to (partially) avoid malicious code
    if (strchr(url, '\'') != NULL) TRACELOG(LOG_WARNING, "SYSTEM: Provided URL could be potentially malicious, avoid [\'] character");
    else
    {
        char *cmd = (char *)RL_CALLOC(strlen(url) + 32, sizeof(char));
#if defined(_WIN32)
        sprintf(cmd, "explorer \"%s\"", url);
#endif
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
        sprintf(cmd, "xdg-open '%s'", url); // Alternatives: firefox, x-www-browser
#endif
#if defined(__APPLE__)
        sprintf(cmd, "open '%s'", url);
#endif
        int result = system(cmd);
        if (result == -1) TRACELOG(LOG_WARNING, "OpenURL() child process could not be created");
        RL_FREE(cmd);
    }
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Inputs
//----------------------------------------------------------------------------------

// Set internal gamepad mappings
int SetGamepadMappings(const char *mappings)
{
    return glfwUpdateGamepadMappings(mappings);
}

// Set gamepad vibration
void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor, float duration)
{
    TRACELOG(LOG_WARNING, "SetGamepadVibration() not available on target platform");
}

// Set mouse position XY
void SetMousePosition(int x, int y)
{
    CORE.Input.Mouse.currentPosition = (Vector2){ (float)x, (float)y };
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;

    // NOTE: emscripten not implemented
    glfwSetCursorPos(platform.handle, CORE.Input.Mouse.currentPosition.x, CORE.Input.Mouse.currentPosition.y);
}

// Set mouse cursor
void SetMouseCursor(int cursor)
{
    CORE.Input.Mouse.cursor = cursor;
    if (cursor == MOUSE_CURSOR_DEFAULT) glfwSetCursor(platform.handle, NULL);
    else
    {
        // NOTE: Mapping internal GLFW enum values to MouseCursor enum values
        glfwSetCursor(platform.handle, glfwCreateStandardCursor(0x00036000 + cursor));
    }
}

// Get physical key name
const char *GetKeyName(int key)
{
    return glfwGetKeyName(key, glfwGetKeyScancode(key));
}

// Register all input events
void PollInputEvents(void)
{
#if defined(SUPPORT_GESTURES_SYSTEM)
    // NOTE: Gestures update must be called every frame to reset gestures correctly
    // because ProcessGestureEvent() is just called on an event, not every frame
    UpdateGestures();
#endif

    // Reset keys/chars pressed registered
    CORE.Input.Keyboard.keyPressedQueueCount = 0;
    CORE.Input.Keyboard.charPressedQueueCount = 0;

    // Reset last gamepad button/axis registered state
    CORE.Input.Gamepad.lastButtonPressed = 0;       // GAMEPAD_BUTTON_UNKNOWN
    //CORE.Input.Gamepad.axisCount = 0;

    // Keyboard/Mouse input polling (automatically managed by GLFW3 through callback)

    // Register previous keys states
    for (int i = 0; i < MAX_KEYBOARD_KEYS; i++)
    {
        CORE.Input.Keyboard.previousKeyState[i] = CORE.Input.Keyboard.currentKeyState[i];
        CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }

    // Register previous mouse states
    for (int i = 0; i < MAX_MOUSE_BUTTONS; i++) CORE.Input.Mouse.previousButtonState[i] = CORE.Input.Mouse.currentButtonState[i];

    // Register previous mouse wheel state
    CORE.Input.Mouse.previousWheelMove = CORE.Input.Mouse.currentWheelMove;
    CORE.Input.Mouse.currentWheelMove = (Vector2){ 0.0f, 0.0f };

    // Register previous mouse position
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;

    // Register previous touch states
    for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.previousTouchState[i] = CORE.Input.Touch.currentTouchState[i];

    // Reset touch positions
    //for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.position[i] = (Vector2){ 0, 0 };

    // Map touch position to mouse position for convenience
    // WARNING: If the target desktop device supports touch screen, this behaviour should be reviewed!
    // TODO: GLFW does not support multi-touch input yet
    // REF: https://www.codeproject.com/Articles/668404/Programming-for-Multi-Touch
    // REF: https://docs.microsoft.com/en-us/windows/win32/wintouch/getting-started-with-multi-touch-messages
    CORE.Input.Touch.position[0] = CORE.Input.Mouse.currentPosition;

    // Check if gamepads are ready
    // NOTE: Doing it here in case of disconnection
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        if (glfwJoystickPresent(i)) CORE.Input.Gamepad.ready[i] = true;
        else CORE.Input.Gamepad.ready[i] = false;
    }

    // Register gamepads buttons events
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        if (CORE.Input.Gamepad.ready[i])     // Check if gamepad is available
        {
            // Register previous gamepad states
            for (int k = 0; k < MAX_GAMEPAD_BUTTONS; k++) CORE.Input.Gamepad.previousButtonState[i][k] = CORE.Input.Gamepad.currentButtonState[i][k];

            // Get current gamepad state
            // NOTE: There is no callback available, getting it manually
            GLFWgamepadstate state = { 0 };
            int result = glfwGetGamepadState(i, &state); // This remaps all gamepads so they have their buttons mapped like an xbox controller
            if (result == GLFW_FALSE) // No joystick is connected, no gamepad mapping or an error occurred
            {
                // Setting axes to expected resting value instead of GLFW 0.0f default when gamepad is not connected
                state.axes[GAMEPAD_AXIS_LEFT_TRIGGER] = -1.0f;
                state.axes[GAMEPAD_AXIS_RIGHT_TRIGGER] = -1.0f;
            }

            const unsigned char *buttons = state.buttons;

            for (int k = 0; (buttons != NULL) && (k < MAX_GAMEPAD_BUTTONS); k++)
            {
                int button = -1;        // GamepadButton enum values assigned

                switch (k)
                {
                    case GLFW_GAMEPAD_BUTTON_Y: button = GAMEPAD_BUTTON_RIGHT_FACE_UP; break;
                    case GLFW_GAMEPAD_BUTTON_B: button = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT; break;
                    case GLFW_GAMEPAD_BUTTON_A: button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN; break;
                    case GLFW_GAMEPAD_BUTTON_X: button = GAMEPAD_BUTTON_RIGHT_FACE_LEFT; break;

                    case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER: button = GAMEPAD_BUTTON_LEFT_TRIGGER_1; break;
                    case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER: button = GAMEPAD_BUTTON_RIGHT_TRIGGER_1; break;

                    case GLFW_GAMEPAD_BUTTON_BACK: button = GAMEPAD_BUTTON_MIDDLE_LEFT; break;
                    case GLFW_GAMEPAD_BUTTON_GUIDE: button = GAMEPAD_BUTTON_MIDDLE; break;
                    case GLFW_GAMEPAD_BUTTON_START: button = GAMEPAD_BUTTON_MIDDLE_RIGHT; break;

                    case GLFW_GAMEPAD_BUTTON_DPAD_UP: button = GAMEPAD_BUTTON_LEFT_FACE_UP; break;
                    case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT: button = GAMEPAD_BUTTON_LEFT_FACE_RIGHT; break;
                    case GLFW_GAMEPAD_BUTTON_DPAD_DOWN: button = GAMEPAD_BUTTON_LEFT_FACE_DOWN; break;
                    case GLFW_GAMEPAD_BUTTON_DPAD_LEFT: button = GAMEPAD_BUTTON_LEFT_FACE_LEFT; break;

                    case GLFW_GAMEPAD_BUTTON_LEFT_THUMB: button = GAMEPAD_BUTTON_LEFT_THUMB; break;
                    case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB: button = GAMEPAD_BUTTON_RIGHT_THUMB; break;
                    default: break;
                }

                if (button != -1)   // Check for valid button
                {
                    if (buttons[k] == GLFW_PRESS)
                    {
                        CORE.Input.Gamepad.currentButtonState[i][button] = 1;
                        CORE.Input.Gamepad.lastButtonPressed = button;
                    }
                    else CORE.Input.Gamepad.currentButtonState[i][button] = 0;
                }
            }

            // Get current state of axes
            const float *axes = state.axes;

            for (int k = 0; (axes != NULL) && (k < GLFW_GAMEPAD_AXIS_LAST + 1); k++)
            {
                CORE.Input.Gamepad.axisState[i][k] = axes[k];
            }

            // Register buttons for 2nd triggers (because GLFW doesn't count these as buttons but rather as axes)
            if (CORE.Input.Gamepad.axisState[i][GAMEPAD_AXIS_LEFT_TRIGGER] > 0.1f)
            {
                CORE.Input.Gamepad.currentButtonState[i][GAMEPAD_BUTTON_LEFT_TRIGGER_2] = 1;
                CORE.Input.Gamepad.lastButtonPressed = GAMEPAD_BUTTON_LEFT_TRIGGER_2;
            }
            else CORE.Input.Gamepad.currentButtonState[i][GAMEPAD_BUTTON_LEFT_TRIGGER_2] = 0;
            if (CORE.Input.Gamepad.axisState[i][GAMEPAD_AXIS_RIGHT_TRIGGER] > 0.1f)
            {
                CORE.Input.Gamepad.currentButtonState[i][GAMEPAD_BUTTON_RIGHT_TRIGGER_2] = 1;
                CORE.Input.Gamepad.lastButtonPressed = GAMEPAD_BUTTON_RIGHT_TRIGGER_2;
            }
            else CORE.Input.Gamepad.currentButtonState[i][GAMEPAD_BUTTON_RIGHT_TRIGGER_2] = 0;

            CORE.Input.Gamepad.axisCount[i] = GLFW_GAMEPAD_AXIS_LAST + 1;
        }
    }

    CORE.Window.resizedLastFrame = false;

    if ((CORE.Window.eventWaiting) ||
        (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MINIMIZED) && !FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_ALWAYS_RUN)))
    {
        glfwWaitEvents();     // Wait for in input events before continue (drawing is paused)
        CORE.Time.previous = GetTime();
    }
    else glfwPollEvents();      // Poll input events: keyboard/mouse/window events (callbacks) -> Update keys state

    CORE.Window.shouldClose = glfwWindowShouldClose(platform.handle);

    // Reset close status for next frame
    glfwSetWindowShouldClose(platform.handle, GLFW_FALSE);
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------
// Function wrappers around RL_*ALLOC macros, used by glfwInitAllocator() inside of InitPlatform()
// GLFWallocator expects function pointers with specific signatures to be provided
// REF: https://www.glfw.org/docs/latest/intro_guide.html#init_allocator
static void *AllocateWrapper(size_t size, void *user)
{
    (void)user;
    return RL_CALLOC(size, 1);
}
static void *ReallocateWrapper(void *block, size_t size, void *user)
{
    (void)user;
    return RL_REALLOC(block, size);
}
static void DeallocateWrapper(void *block, void *user)
{
    (void)user;
    RL_FREE(block);
}

// Initialize platform: graphics, inputs and more
int InitPlatform(void)
{
    glfwSetErrorCallback(ErrorCallback);

    const GLFWallocator allocator = {
        .allocate = AllocateWrapper,
        .reallocate = ReallocateWrapper,
        .deallocate = DeallocateWrapper,
        .user = NULL, // RL_*ALLOC macros are not capable of handling user-provided data
    };

    glfwInitAllocator(&allocator);

#if defined(__APPLE__)
    glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif
    // Initialize GLFW internal global state
    int result = glfwInit();
    if (result == GLFW_FALSE) { TRACELOG(LOG_WARNING, "GLFW: Failed to initialize GLFW"); return -1; }

    // Initialize graphic device: display/window and graphic context
    //----------------------------------------------------------------------------
    glfwDefaultWindowHints();                       // Set default windows hints
    //glfwWindowHint(GLFW_RED_BITS, 8);             // Framebuffer red color component bits
    //glfwWindowHint(GLFW_GREEN_BITS, 8);           // Framebuffer green color component bits
    //glfwWindowHint(GLFW_BLUE_BITS, 8);            // Framebuffer blue color component bits
    //glfwWindowHint(GLFW_ALPHA_BITS, 8);           // Framebuffer alpha color component bits
    //glfwWindowHint(GLFW_DEPTH_BITS, 24);          // Depthbuffer bits
    //glfwWindowHint(GLFW_REFRESH_RATE, 0);         // Refresh rate for fullscreen window
    //glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API); // OpenGL API to use. Alternative: GLFW_OPENGL_ES_API
    //glfwWindowHint(GLFW_AUX_BUFFERS, 0);          // Number of auxiliar buffers

    // Disable GlFW auto iconify behaviour
    // Auto Iconify automatically minimizes (iconifies) the window if the window loses focus
    // additionally auto iconify restores the hardware resolution of the monitor if the window that loses focus is a fullscreen window
    glfwWindowHint(GLFW_AUTO_ICONIFY, 0);

    // Window flags requested before initialization to be applied after initialization
    unsigned int requestedWindowFlags = CORE.Window.flags;

    // Check window creation flags
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIDDEN)) glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Visible window
    else glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);     // Window initially hidden

    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_UNDECORATED)) glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Border and buttons on Window
    else glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);   // Decorated window

    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_RESIZABLE)) glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // Resizable window
    else glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);  // Avoid window being resizable

    // Disable FLAG_WINDOW_MINIMIZED, not supported on initialization
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MINIMIZED)) FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_MINIMIZED);

    // Disable FLAG_WINDOW_MAXIMIZED, not supported on initialization
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED)) FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED);

    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_UNFOCUSED)) glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
    else glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_TOPMOST)) glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    else glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);

    // NOTE: Some GLFW flags are not supported on HTML5
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_TRANSPARENT)) glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);     // Transparent framebuffer
    else glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);  // Opaque framebuffer

    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIGHDPI))
    {
#if defined(__APPLE__)
        glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_FALSE);
#endif
        // Resize window content area based on the monitor content scale
        // NOTE: This hint only has an effect on platforms where screen coordinates and
        // pixels always map 1:1 such as Windows and X11
        // On platforms like macOS the resolution of the framebuffer is changed independently of the window size
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
#if defined(__APPLE__)
        glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_TRUE);
#endif
    }
    else
    {
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
#if defined(__APPLE__)
        glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_FALSE);
#endif
    }

    // Mouse passthrough
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MOUSE_PASSTHROUGH)) glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
    else glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_FALSE);

    if (FLAG_IS_SET(CORE.Window.flags, FLAG_MSAA_4X_HINT))
    {
        // NOTE: MSAA is only enabled for main framebuffer, not user-created FBOs
        TRACELOG(LOG_INFO, "DISPLAY: Trying to enable MSAA x4");
        glfwWindowHint(GLFW_SAMPLES, 4);   // Tries to enable multisampling x4 (MSAA), default is 0
    }

    // NOTE: When asking for an OpenGL context version, most drivers provide the highest supported version
    // with backward compatibility to older OpenGL versions
    // For example, if using OpenGL 1.1, driver can provide a 4.3 backwards compatible context

    // Check selection OpenGL version
    if (rlGetVersion() == RL_OPENGL_21)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);          // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);          // Choose OpenGL minor version (just hint)
    }
    else if (rlGetVersion() == RL_OPENGL_33)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);          // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);          // Choose OpenGL minor version (just hint)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Profiles Hint: Only 3.3 and above!
                                                                       // Values: GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_ANY_PROFILE, GLFW_OPENGL_COMPAT_PROFILE
#if defined(__APPLE__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);  // OSX Requires forward compatibility
#else
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE); // Forward Compatibility Hint: Only 3.3 and above!
#endif
        //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); // Request OpenGL DEBUG context
    }
    else if (rlGetVersion() == RL_OPENGL_43)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);          // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);          // Choose OpenGL minor version (just hint)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE);
#if defined(RLGL_ENABLE_OPENGL_DEBUG_CONTEXT)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);   // Enable OpenGL Debug Context
#endif
    }
    else if (rlGetVersion() == RL_OPENGL_ES_20)                 // Request OpenGL ES 2.0 context
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    }
    else if (rlGetVersion() == RL_OPENGL_ES_30)                 // Request OpenGL ES 3.0 context
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    }

    // NOTE: GLFW 3.4+ defers initialization of the Joystick subsystem on the first call to any Joystick related functions
    // Forcing this initialization here avoids doing it on PollInputEvents() called by EndDrawing() after first frame has been just drawn
    // The initialization will still happen and possible delays still occur, but before the window is shown, which is a nicer experience
    // REF: https://github.com/raysan5/raylib/issues/1554
    glfwSetJoystickCallback(NULL);

    if ((CORE.Window.screen.width == 0) || (CORE.Window.screen.height == 0)) FLAG_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE);

    // Init window in fullscreen mode if requested
    // NOTE: Keeping original screen size for toggle
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE))
    {
        // NOTE: Fullscreen applications default to the primary monitor
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        if (!monitor)
        {
            TRACELOG(LOG_WARNING, "GLFW: Failed to get primary monitor");
            return -1;
        }

        // Set dimensions from monitor
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);

        // Default display resolution to that of the current mode
        CORE.Window.display.width = mode->width;
        CORE.Window.display.height = mode->height;

        // Check if user requested some screen size
        if ((CORE.Window.screen.width == 0) || (CORE.Window.screen.height == 0))
        {
            // Set some default screen size in case user decides to exit fullscreen mode
            CORE.Window.previousScreen.width = 800;
            CORE.Window.previousScreen.height = 450;
            CORE.Window.previousPosition.x = CORE.Window.display.width/2 - 800/2;
            CORE.Window.previousPosition.y = CORE.Window.display.height/2 - 450/2;

            // Set screen width/height to the display width/height
            if (CORE.Window.screen.width == 0) CORE.Window.screen.width = CORE.Window.display.width;
            if (CORE.Window.screen.height == 0) CORE.Window.screen.height = CORE.Window.display.height;
        }
        else
        {
            CORE.Window.previousScreen = CORE.Window.screen;
            CORE.Window.screen = CORE.Window.display;
        }

        platform.handle = glfwCreateWindow(CORE.Window.screen.width, CORE.Window.screen.height, (CORE.Window.title != 0)? CORE.Window.title : " ", monitor, NULL);
        if (!platform.handle)
        {
            glfwTerminate();
            TRACELOG(LOG_WARNING, "GLFW: Failed to initialize Window");
            return -1;
        }
    }
    else
    {
        // Default to at least one pixel in size, as creation with a zero dimension is not allowed
        if (CORE.Window.screen.width == 0) CORE.Window.screen.width = 1;
        if (CORE.Window.screen.height == 0) CORE.Window.screen.height = 1;

        platform.handle = glfwCreateWindow(CORE.Window.screen.width, CORE.Window.screen.height, (CORE.Window.title != 0)? CORE.Window.title : " ", NULL, NULL);
        if (!platform.handle)
        {
            glfwTerminate();
            TRACELOG(LOG_WARNING, "GLFW: Failed to initialize Window");
            return -1;
        }

        // After the window was created, determine the monitor that the window manager assigned
        // Derive display sizes and, if possible, window size in case it was zero at beginning

        int monitorCount = 0;
        int monitorIndex = GetCurrentMonitor();
        GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

        if (monitorIndex < monitorCount)
        {
            GLFWmonitor *monitor = monitors[monitorIndex];
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);

            // Default display resolution to that of the current mode
            CORE.Window.display.width = mode->width;
            CORE.Window.display.height = mode->height;

            // Set screen width/height to the display width/height if they are 0
            if (CORE.Window.screen.width == 0) CORE.Window.screen.width = CORE.Window.display.width;
            if (CORE.Window.screen.height == 0) CORE.Window.screen.height = CORE.Window.display.height;

            glfwSetWindowSize(platform.handle, CORE.Window.screen.width, CORE.Window.screen.height);
        }
        else
        {
            // The monitor for the window-manager-created window can not be determined, so it can not be centered
            glfwTerminate();
            TRACELOG(LOG_WARNING, "GLFW: Failed to determine Monitor to center Window");
            return -1;
        }

        // NOTE: Not considering scale factor now, considered below
        CORE.Window.render.width = CORE.Window.screen.width;
        CORE.Window.render.height = CORE.Window.screen.height;
    }

    glfwMakeContextCurrent(platform.handle);
    result = glfwGetError(NULL);
    if ((result != GLFW_NO_WINDOW_CONTEXT) && (result != GLFW_PLATFORM_ERROR)) CORE.Window.ready = true; // Checking context activation

    if (CORE.Window.ready)
    {
        // Setup additional windows configs and register required window size info

        glfwSwapInterval(0); // No V-Sync by default

        // Try to enable GPU V-Sync, so frames are limited to screen refresh rate (60Hz -> 60 FPS)
        // NOTE: V-Sync can be enabled by graphic driver configuration, it doesn't need
        // to be activated on web platforms since VSync is enforced there
        if (FLAG_IS_SET(CORE.Window.flags, FLAG_VSYNC_HINT))
        {
            // WARNING: It seems to hit a critical render path in Intel HD Graphics
            glfwSwapInterval(1);
            TRACELOG(LOG_INFO, "DISPLAY: Trying to enable VSYNC");
        }

        int fbWidth = CORE.Window.screen.width;
        int fbHeight = CORE.Window.screen.height;

        if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIGHDPI))
        {
            // NOTE: On APPLE platforms system should manage window/input scaling and also framebuffer scaling
            // Framebuffer scaling is activated with: glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_TRUE);

            // Get current framebuffer size, on high-dpi it could be bigger than screen size
            glfwGetFramebufferSize(platform.handle, &fbWidth, &fbHeight);

            // Screen scaling matrix is required in case desired screen area is different from display area
            CORE.Window.screenScale = MatrixScale((float)fbWidth/CORE.Window.screen.width, (float)fbHeight/CORE.Window.screen.height, 1.0f);
#if !defined(__APPLE__)
            // Mouse input scaling for the new screen size
            SetMouseScale((float)CORE.Window.screen.width/fbWidth, (float)CORE.Window.screen.height/fbHeight);
#endif
        }

        CORE.Window.render.width = fbWidth;
        CORE.Window.render.height = fbHeight;
        CORE.Window.currentFbo.width = fbWidth;
        CORE.Window.currentFbo.height = fbHeight;

        TRACELOG(LOG_INFO, "DISPLAY: Device initialized successfully");
        TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
        TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
        TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
        TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);

        // Try to center window on screen but avoiding window-bar outside of screen
        int monitorCount = 0;
        int monitorIndex = GetCurrentMonitor();
        GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);
        GLFWmonitor *monitor = monitors[monitorIndex];

        int monitorX = 0;
        int monitorY = 0;
        int monitorWidth = 0;
        int monitorHeight = 0;
        glfwGetMonitorWorkarea(monitor, &monitorX, &monitorY, &monitorWidth, &monitorHeight);

        // TODO: Here CORE.Window.render.width/height should be used instead of
        // CORE.Window.screen.width/height to center the window correctly when the high dpi flag is enabled
        CORE.Window.position.x = monitorX + (monitorWidth - (int)CORE.Window.screen.width)/2;
        CORE.Window.position.y = monitorY + (monitorHeight - (int)CORE.Window.screen.height)/2;
        //if (CORE.Window.position.x < monitorX) CORE.Window.position.x = monitorX;
        //if (CORE.Window.position.y < monitorY) CORE.Window.position.y = monitorY;

        SetWindowPosition(CORE.Window.position.x, CORE.Window.position.y);

        if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MINIMIZED)) MinimizeWindow();
    }
    else
    {
        TRACELOG(LOG_FATAL, "PLATFORM: Failed to initialize graphics device");
        return -1;
    }

    // Apply window flags requested previous to initialization
    SetWindowState(requestedWindowFlags);

    // Load OpenGL extensions
    // NOTE: GL procedures address loader is required to load extensions
    rlLoadExtensions(glfwGetProcAddress);
    //----------------------------------------------------------------------------

    // Initialize input events callbacks
    //----------------------------------------------------------------------------
    // Set window callback events
    glfwSetWindowSizeCallback(platform.handle, WindowSizeCallback); // NOTE: Resizing is not enabled by default
    glfwSetFramebufferSizeCallback(platform.handle, FramebufferSizeCallback);
    glfwSetWindowPosCallback(platform.handle, WindowPosCallback);
    glfwSetWindowMaximizeCallback(platform.handle, WindowMaximizeCallback);
    glfwSetWindowIconifyCallback(platform.handle, WindowIconifyCallback);
    glfwSetWindowFocusCallback(platform.handle, WindowFocusCallback);
    glfwSetDropCallback(platform.handle, WindowDropCallback);
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIGHDPI)) glfwSetWindowContentScaleCallback(platform.handle, WindowContentScaleCallback);

    // Set input callback events
    glfwSetKeyCallback(platform.handle, KeyCallback);
    glfwSetCharCallback(platform.handle, CharCallback);
    glfwSetMouseButtonCallback(platform.handle, MouseButtonCallback);
    glfwSetCursorPosCallback(platform.handle, MouseCursorPosCallback); // Track mouse position changes
    glfwSetScrollCallback(platform.handle, MouseScrollCallback);
    glfwSetCursorEnterCallback(platform.handle, CursorEnterCallback);
    glfwSetJoystickCallback(JoystickCallback);
    glfwSetInputMode(platform.handle, GLFW_LOCK_KEY_MODS, GLFW_TRUE); // Enable lock keys modifiers (CAPS, NUM)

    // Retrieve gamepad names
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        // WARNING: If glfwGetJoystickName() is longer than MAX_GAMEPAD_NAME_LENGTH,
        // only copying up to (MAX_GAMEPAD_NAME_LENGTH - 1)
        if (glfwJoystickPresent(i))
        {
          CORE.Input.Gamepad.ready[i] = true;
          CORE.Input.Gamepad.axisCount[i] = GLFW_GAMEPAD_AXIS_LAST + 1;
          strncpy(CORE.Input.Gamepad.name[i], glfwGetJoystickName(i), MAX_GAMEPAD_NAME_LENGTH - 1);
        }
    }
    //----------------------------------------------------------------------------

    // Initialize timming system
    //----------------------------------------------------------------------------
    InitTimer();
    //----------------------------------------------------------------------------

    // Initialize storage system
    //----------------------------------------------------------------------------
    CORE.Storage.basePath = GetWorkingDirectory();
    //----------------------------------------------------------------------------

#if defined(__NetBSD__)
    // Workaround for NetBSD
    char *glfwPlatform = "X11 (NetBSD)";
#else
    char *glfwPlatform = "";
    switch (glfwGetPlatform())
    {
        case GLFW_PLATFORM_WIN32: glfwPlatform = "Win32"; break;
        case GLFW_PLATFORM_COCOA: glfwPlatform = "Cocoa"; break;
        case GLFW_PLATFORM_WAYLAND: glfwPlatform = "Wayland"; break;
        case GLFW_PLATFORM_X11: glfwPlatform = "X11"; break;
        case GLFW_PLATFORM_NULL: glfwPlatform = "Null"; break;
        default: break;
    }
#endif

    TRACELOG(LOG_INFO, "PLATFORM: DESKTOP (GLFW - %s): Initialized successfully", glfwPlatform);

    return 0;
}

// Close platform
void ClosePlatform(void)
{
    glfwDestroyWindow(platform.handle);
    glfwTerminate();

#if defined(_WIN32) && defined(SUPPORT_WINMM_HIGHRES_TIMER) && !defined(SUPPORT_BUSY_WAIT_LOOP)
    timeEndPeriod(1);           // Restore time period
#endif
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
// NOTE: Those functions are only required for current platform
//----------------------------------------------------------------------------------

// GLFW3: Error callback, runs on GLFW3 error
static void ErrorCallback(int error, const char *description)
{
    TRACELOG(LOG_WARNING, "GLFW: Error: %i Description: %s", error, description);
}

// GLFW3: Window size change callback, runs when window is resized
// NOTE: Window resizing not enabled by default, use SetConfigFlags()
static void WindowSizeCallback(GLFWwindow *window, int width, int height)
{
    // Nothing to do for now on window resize...
    //TRACELOG(LOG_INFO, "GLFW3: Window size callback called [%i,%i]", width, height);
}

// GLFW3: Framebuffer size change callback, runs when framebuffer is resized
// WARNING: If FLAG_WINDOW_HIGHDPI is set, WindowContentScaleCallback() is called before this function
static void FramebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    //TRACELOG(LOG_INFO, "GLFW3: Window framebuffer size callback called [%i,%i]", width, height);

    // WARNING: On window minimization, callback is called with 0 values,
    // but internal screen values should not be changed, it breaks things
    if ((width == 0) || (height == 0)) return;

    // Reset viewport and projection matrix for new size
    // NOTE: Stores current render size: CORE.Window.render
    SetupViewport(width, height);

    // Set render size
    CORE.Window.currentFbo.width = width;
    CORE.Window.currentFbo.height = height;
    CORE.Window.resizedLastFrame = true;

    if (FLAG_IS_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE))
    {
        // On fullscreen mode, strategy is ignoring high-dpi and
        // use the all available display size

        // Set screen size to render size (physical pixel size)
        CORE.Window.screen.width = width;
        CORE.Window.screen.height = height;
        CORE.Window.screenScale = MatrixScale(1.0f, 1.0f, 1.0f);
        SetMouseScale(1.0f, 1.0f);
    }
    else // Window mode (including borderless window)
    {
        // Check if render size was actually scaled for high-dpi
        if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIGHDPI))
        {
            // Set screen size to logical pixel size, considering content scaling
            Vector2 scaleDpi = GetWindowScaleDPI();
            CORE.Window.screen.width = (int)((float)width/scaleDpi.x);
            CORE.Window.screen.height = (int)((float)height/scaleDpi.y);
            CORE.Window.screenScale = MatrixScale(scaleDpi.x, scaleDpi.y, 1.0f);
#if !defined(__APPLE__)
            // Mouse input scaling for the new screen size
            SetMouseScale(1.0f/scaleDpi.x, 1.0f/scaleDpi.y);
#endif
        }
        else
        {
            // Set screen size to render size (physical pixel size)
            CORE.Window.screen.width = width;
            CORE.Window.screen.height = height;
        }
    }

    // WARNING: If using a render texture, it is not scaled to new size
}

// GLFW3: Window content scale callback, runs on monitor content scale change detected
// WARNING: If FLAG_WINDOW_HIGHDPI is not set, this function is not called
static void WindowContentScaleCallback(GLFWwindow *window, float scalex, float scaley)
{
    //TRACELOG(LOG_INFO, "GLFW3: Window content scale changed, scale: [%.2f,%.2f]", scalex, scaley);

    float fbWidth = (float)CORE.Window.screen.width*scalex;
    float fbHeight = (float)CORE.Window.screen.height*scaley;

    // NOTE: On APPLE platforms system should manage window/input scaling and also framebuffer scaling
    // Framebuffer scaling is activated with: glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_TRUE);
    CORE.Window.screenScale = MatrixScale(scalex, scaley, 1.0f);

#if !defined(__APPLE__)
    // Mouse input scaling for the new screen size
    SetMouseScale(1.0f/scalex, 1.0f/scaley);
#endif

    CORE.Window.render.width = (int)fbWidth;
    CORE.Window.render.height = (int)fbHeight;
    CORE.Window.currentFbo = CORE.Window.render;
}

// GLFW3: Window position callback, runs when window position changes
static void WindowPosCallback(GLFWwindow *window, int x, int y)
{
    // Set current window position
    CORE.Window.position.x = x;
    CORE.Window.position.y = y;
}

// GLFW3: Window iconify callback, runs when window is minimized/restored
static void WindowIconifyCallback(GLFWwindow *window, int iconified)
{
    if (iconified) FLAG_SET(CORE.Window.flags, FLAG_WINDOW_MINIMIZED);   // The window was iconified
    else FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_MINIMIZED);           // The window was restored
}

// GLFW3: Window maximize callback, runs when window is maximized/restored
static void WindowMaximizeCallback(GLFWwindow *window, int maximized)
{
    if (maximized) FLAG_SET(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED);  // The window was maximized
    else FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED);          // The window was restored
}

// GLFW3: Window focus callback, runs when window get/lose focus
static void WindowFocusCallback(GLFWwindow *window, int focused)
{
    if (focused) FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_UNFOCUSED);    // The window was focused
    else FLAG_SET(CORE.Window.flags, FLAG_WINDOW_UNFOCUSED);          // The window lost focus
}

// GLFW3: Window drop callback, runs when files are dropped into window
static void WindowDropCallback(GLFWwindow *window, int count, const char **paths)
{
    if (count > 0)
    {
        // In case previous dropped filepaths have not been freed, free them
        if (CORE.Window.dropFileCount > 0)
        {
            for (unsigned int i = 0; i < CORE.Window.dropFileCount; i++) RL_FREE(CORE.Window.dropFilepaths[i]);

            RL_FREE(CORE.Window.dropFilepaths);

            CORE.Window.dropFileCount = 0;
            CORE.Window.dropFilepaths = NULL;
        }

        // WARNING: Paths are freed by GLFW when the callback returns, keeping an internal copy
        CORE.Window.dropFileCount = count;
        CORE.Window.dropFilepaths = (char **)RL_CALLOC(CORE.Window.dropFileCount, sizeof(char *));

        for (unsigned int i = 0; i < CORE.Window.dropFileCount; i++)
        {
            CORE.Window.dropFilepaths[i] = (char *)RL_CALLOC(MAX_FILEPATH_LENGTH, sizeof(char));
            strncpy(CORE.Window.dropFilepaths[i], paths[i], MAX_FILEPATH_LENGTH - 1);
        }
    }
}

// GLFW3: Keyboard callback, runs on key pressed
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key < 0) return;    // Security check, macOS fn key generates -1

    // WARNING: GLFW could return GLFW_REPEAT, it needs to be considered as 1
    // to work properly with our implementation (IsKeyDown/IsKeyUp checks)
    if (action == GLFW_RELEASE) CORE.Input.Keyboard.currentKeyState[key] = 0;
    else if (action == GLFW_PRESS) CORE.Input.Keyboard.currentKeyState[key] = 1;
    else if (action == GLFW_REPEAT) CORE.Input.Keyboard.keyRepeatInFrame[key] = 1;

    // WARNING: Check if CAPS/NUM key modifiers are enabled and force down state for those keys
    if (((key == KEY_CAPS_LOCK) && (FLAG_IS_SET(mods, GLFW_MOD_CAPS_LOCK))) ||
        ((key == KEY_NUM_LOCK) && (FLAG_IS_SET(mods, GLFW_MOD_NUM_LOCK)))) CORE.Input.Keyboard.currentKeyState[key] = 1;

    // Check if there is space available in the key queue
    if ((CORE.Input.Keyboard.keyPressedQueueCount < MAX_KEY_PRESSED_QUEUE) && (action == GLFW_PRESS))
    {
        // Add character to the queue
        CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = key;
        CORE.Input.Keyboard.keyPressedQueueCount++;
    }

    // Check the exit key to set close window
    if ((key == CORE.Input.Keyboard.exitKey) && (action == GLFW_PRESS)) glfwSetWindowShouldClose(platform.handle, GLFW_TRUE);
}

// GLFW3: Char callback, runs on key pressed to get unicode codepoint value
static void CharCallback(GLFWwindow *window, unsigned int codepoint)
{
    // NOTE: Registers any key down considering OS keyboard layout but
    // does not detect action events, those should be managed by user...
    // REF: https://github.com/glfw/glfw/issues/668#issuecomment-166794907
    // REF: https://www.glfw.org/docs/latest/input_guide.html#input_char

    // Check if there is space available in the queue
    if (CORE.Input.Keyboard.charPressedQueueCount < MAX_CHAR_PRESSED_QUEUE)
    {
        // Add character to the queue
        CORE.Input.Keyboard.charPressedQueue[CORE.Input.Keyboard.charPressedQueueCount] = codepoint;
        CORE.Input.Keyboard.charPressedQueueCount++;
    }
}

// GLFW3: Mouse button callback, runs on mouse button pressed
static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    // WARNING: GLFW could only return GLFW_PRESS (1) or GLFW_RELEASE (0) for now,
    // but future releases may add more actions (i.e. GLFW_REPEAT)
    CORE.Input.Mouse.currentButtonState[button] = action;
    CORE.Input.Touch.currentTouchState[button] = action;

#if defined(SUPPORT_GESTURES_SYSTEM) && defined(SUPPORT_MOUSE_GESTURES)
    // Process mouse events as touches to be able to use mouse-gestures
    GestureEvent gestureEvent = { 0 };

    // Register touch actions
    if ((CORE.Input.Mouse.currentButtonState[button] == 1) && (CORE.Input.Mouse.previousButtonState[button] == 0)) gestureEvent.touchAction = TOUCH_ACTION_DOWN;
    else if ((CORE.Input.Mouse.currentButtonState[button] == 0) && (CORE.Input.Mouse.previousButtonState[button] == 1)) gestureEvent.touchAction = TOUCH_ACTION_UP;

    // NOTE: TOUCH_ACTION_MOVE event is registered in MouseCursorPosCallback()

    // Assign a pointer ID
    gestureEvent.pointId[0] = 0;

    // Register touch points count
    gestureEvent.pointCount = 1;

    // Register touch points position, only one point registered
    gestureEvent.position[0] = GetMousePosition();

    // Normalize gestureEvent.position[0] for CORE.Window.screen.width and CORE.Window.screen.height
    gestureEvent.position[0].x /= (float)GetScreenWidth();
    gestureEvent.position[0].y /= (float)GetScreenHeight();

    // Gesture data is sent to gestures-system for processing
    ProcessGestureEvent(gestureEvent);
#endif
}

// GLFW3: Cursor position callback, runs on mouse movement
static void MouseCursorPosCallback(GLFWwindow *window, double x, double y)
{
    CORE.Input.Mouse.currentPosition.x = (float)x;
    CORE.Input.Mouse.currentPosition.y = (float)y;
    CORE.Input.Touch.position[0] = CORE.Input.Mouse.currentPosition;

#if defined(SUPPORT_GESTURES_SYSTEM) && defined(SUPPORT_MOUSE_GESTURES)
    // Process mouse events as touches to be able to use mouse-gestures
    GestureEvent gestureEvent = { 0 };

    gestureEvent.touchAction = TOUCH_ACTION_MOVE;

    // Assign a pointer ID
    gestureEvent.pointId[0] = 0;

    // Register touch points count
    gestureEvent.pointCount = 1;

    // Register touch points position, only one point registered
    gestureEvent.position[0] = CORE.Input.Touch.position[0];

    // Normalize gestureEvent.position[0] for CORE.Window.screen.width and CORE.Window.screen.height
    gestureEvent.position[0].x /= (float)GetScreenWidth();
    gestureEvent.position[0].y /= (float)GetScreenHeight();

    // Gesture data is sent to gestures-system for processing
    ProcessGestureEvent(gestureEvent);
#endif
}

// GLFW3: Mouse wheel scroll callback, runs on mouse wheel changes
static void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    CORE.Input.Mouse.currentWheelMove = (Vector2){ (float)xoffset, (float)yoffset };
}

// GLFW3: Cursor ennter callback, when cursor enters the window
static void CursorEnterCallback(GLFWwindow *window, int enter)
{
    if (enter) CORE.Input.Mouse.cursorOnScreen = true;
    else CORE.Input.Mouse.cursorOnScreen = false;
}

// GLFW3: Joystick connected/disconnected callback
static void JoystickCallback(int jid, int event)
{
    if (event == GLFW_CONNECTED)
    {
        // WARNING: If glfwGetJoystickName() is longer than MAX_GAMEPAD_NAME_LENGTH,
        // only copy up to (MAX_GAMEPAD_NAME_LENGTH -1) to destination string
        memset(CORE.Input.Gamepad.name[jid], 0, MAX_GAMEPAD_NAME_LENGTH);
        strncpy(CORE.Input.Gamepad.name[jid], glfwGetJoystickName(jid), MAX_GAMEPAD_NAME_LENGTH - 1);
    }
    else if (event == GLFW_DISCONNECTED)
    {
        memset(CORE.Input.Gamepad.name[jid], 0, MAX_GAMEPAD_NAME_LENGTH);
    }
}

#ifdef _WIN32
#   define WIN32_CLIPBOARD_IMPLEMENTATION
#   include "../external/win32_clipboard.h"
#endif
// EOF
