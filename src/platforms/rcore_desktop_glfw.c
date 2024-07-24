/**********************************************************************************************
*
*   rcore_desktop - Functions to manage window, graphics device and inputs
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
*   ADDITIONAL NOTES:
*       - TRACELOG() function is located in raylib [utils] module
*
*   CONFIGURATION:
*       #define RCORE_PLATFORM_CUSTOM_FLAG
*           Custom flag for rcore on target platform -not used-
*
*   DEPENDENCIES:
*       - rglfw: Manage graphic device, OpenGL context and inputs (Windows, Linux, OSX, FreeBSD...)
*       - gestures: Gestures system for touch-ready devices (or simulated from mouse inputs)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2024 Ramon Santamaria (@raysan5) and contributors
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
    typedef void *PVOID;
    typedef PVOID HANDLE;
    typedef HANDLE HWND;
    #define GLFW_EXPOSE_NATIVE_WIN32
    #define GLFW_NATIVE_INCLUDE_NONE // To avoid some symbols re-definition in windows.h
    #include "GLFW/glfw3native.h"

    #if defined(SUPPORT_WINMM_HIGHRES_TIMER) && !defined(SUPPORT_BUSY_WAIT_LOOP)
        // NOTE: Those functions require linking with winmm library
        unsigned int __stdcall timeBeginPeriod(unsigned int uPeriod);
        unsigned int __stdcall timeEndPeriod(unsigned int uPeriod);
    #endif
#endif
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
    #include <sys/time.h>               // Required for: timespec, nanosleep(), select() - POSIX

    //#define GLFW_EXPOSE_NATIVE_X11      // WARNING: Exposing Xlib.h > X.h results in dup symbols for Font type
    //#define GLFW_EXPOSE_NATIVE_WAYLAND
    #include "GLFW/glfw3native.h"       // Required for: glfwGetX11Window()
#endif
#if defined(__APPLE__)
    #include <unistd.h>                 // Required for: usleep()

    //#define GLFW_EXPOSE_NATIVE_COCOA    // WARNING: Fails due to type redefinition
    void *glfwGetCocoaWindow(GLFWwindow* handle);
    #include "GLFW/glfw3native.h"       // Required for: glfwGetCocoaWindow()
#endif

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
static void ErrorCallback(int error, const char *description);                             // GLFW3 Error Callback, runs on GLFW3 error

// Window callbacks events
static void WindowSizeCallback(GLFWwindow *window, int width, int height);                 // GLFW3 WindowSize Callback, runs when window is resized
static void WindowIconifyCallback(GLFWwindow *window, int iconified);                      // GLFW3 WindowIconify Callback, runs when window is minimized/restored
static void WindowMaximizeCallback(GLFWwindow* window, int maximized);                     // GLFW3 Window Maximize Callback, runs when window is maximized
static void WindowFocusCallback(GLFWwindow *window, int focused);                          // GLFW3 WindowFocus Callback, runs when window get/lose focus
static void WindowDropCallback(GLFWwindow *window, int count, const char **paths);         // GLFW3 Window Drop Callback, runs when drop files into window
static void WindowContentScaleCallback(GLFWwindow *window, float scalex, float scaley);    // GLFW3 Window Content Scale Callback, runs when window changes scale

// Input callbacks events
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);  // GLFW3 Keyboard Callback, runs on key pressed
static void CharCallback(GLFWwindow *window, unsigned int codepoint);                      // GLFW3 Char Callback, runs on key pressed (get codepoint value)
static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);     // GLFW3 Mouse Button Callback, runs on mouse button pressed
static void MouseCursorPosCallback(GLFWwindow *window, double x, double y);                // GLFW3 Cursor Position Callback, runs on mouse move
static void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);       // GLFW3 Scrolling Callback, runs on mouse wheel
static void CursorEnterCallback(GLFWwindow *window, int enter);                            // GLFW3 Cursor Enter Callback, cursor enters client area
static void JoystickCallback(int jid, int event);                                           // GLFW3 Joystick Connected/Disconnected Callback

// Customized and internal tool functions
static void _SetupFramebuffer(int frameBufferWidth, int frameBufferHeight, bool inHardwareFullscreenMode); // Better implementation of SetupFramebuffer()

static bool _ActivateHardwareFullscreenMode(int monitorIndex, int desiredWidth, int desiredHeight, int desiredRefreshRate); 
static void _DeactivateHardwareFullscreenMode();

static void _SetupPlatformMouseScaleAndOffset(); // Update mouse scale and offset according to current viewport

static void _SetPlatformMouseOffset(int offsetX, int offsetY);
static void _SetPlatformMouseScale(float scaleX, float scaleY);

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
    if (IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE))
    {
        // We can't go straighforward from one fullscreen mode to an other
        // there needs to be at least one loop between them
        // so we just toggle the currently active fullscreen mode and leave.
        ToggleBorderlessWindowed();
        return;
    }

    if (!CORE.Window.fullscreen)
    {
        _ActivateHardwareFullscreenMode( GetCurrentMonitor(), CORE.Window.screen.width, CORE.Window.screen.height, GLFW_DONT_CARE );
    }
    else
    {
        _DeactivateHardwareFullscreenMode();
    }

    // Refocus window
    glfwFocusWindow(platform.handle);

    // Try to enable GPU V-Sync, so frames are limited to screen refresh rate (60Hz -> 60 FPS)
    // NOTE: V-Sync can be enabled by graphic driver configuration
    if (CORE.Window.flags & FLAG_VSYNC_HINT) glfwSwapInterval(1);
}

// Toggle borderless windowed mode
void ToggleBorderlessWindowed(void)
{
    if (CORE.Window.fullscreen)
    {
        // We can't go straighforward from one fullscreen mode to an other
        // there needs to be at least one loop between them
        // so we just toggle the currently active fullscreen mode and leave.
        ToggleFullscreen();
        return;
    }

    const int monitor = GetCurrentMonitor();
    int monitorCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);

        if (mode)
        {
            TRACELOG(LOG_INFO, "DISPLAY: ToggleBorderlessWindowed before");
            TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
            TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
            TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
            TRACELOG(LOG_INFO, "    > Screen scale: %f x %f", CORE.Window.screenScale.m0, CORE.Window.screenScale.m5);
            TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);
            
            if (!IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE))
            {
                // Store screen position and size :
                glfwGetWindowPos(platform.handle, &CORE.Window.previousPosition.x, &CORE.Window.previousPosition.y);

                // GLFW 3.4 may trigger an error if platform like Wayland don't support setting or getting the position of the window
                glfwGetError(NULL); // So we must clear this error

                //!\ We need to save the frameBuffer size intead of the "screen size"

                int prevWidth, prevHeight;
                glfwGetFramebufferSize(platform.handle, &prevWidth, &prevHeight);

                CORE.Window.previousScreen.width = prevWidth;
                CORE.Window.previousScreen.height = prevHeight;

                // Ask fullscreen window :
                glfwSetWindowMonitor(platform.handle, monitors[monitor], 0, 0, mode->width, mode->height, mode->refreshRate);

                _SetupFramebuffer( mode->width , mode->height, false );

                // Refocus window
                glfwFocusWindow(platform.handle);

                CORE.Window.flags |= FLAG_BORDERLESS_WINDOWED_MODE;
            }
            else
            {
                // Return previous screen size and position
                int prevPosX = CORE.Window.previousPosition.x;
                int prevPosY = CORE.Window.previousPosition.y;
                int prevWidth = CORE.Window.previousScreen.width;
                int prevHeight = CORE.Window.previousScreen.height;

                glfwSetWindowMonitor(platform.handle, NULL, prevPosX, prevPosY, prevWidth, prevHeight, GLFW_DONT_CARE);

                _SetupFramebuffer(prevWidth , prevHeight, false);

                // Refocus window
                glfwFocusWindow(platform.handle);

                CORE.Window.flags &= ~FLAG_BORDERLESS_WINDOWED_MODE;
            }
            TRACELOG(LOG_INFO, "DISPLAY: ToggleBorderlessWindowed after");
            TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
            TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
            TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
            TRACELOG(LOG_INFO, "    > Screen scale: %f x %f", CORE.Window.screenScale.m0, CORE.Window.screenScale.m5);
            TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);
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
        CORE.Window.flags |= FLAG_WINDOW_MAXIMIZED;
    }
}

// Set window state: minimized
void MinimizeWindow(void)
{
    // NOTE: Following function launches callback that sets appropriate flag!
    glfwIconifyWindow(platform.handle);
}

// Set window state: not minimized/maximized
void RestoreWindow(void)
{
    if (glfwGetWindowAttrib(platform.handle, GLFW_RESIZABLE) == GLFW_TRUE)
    {
        // Restores the specified window if it was previously iconified (minimized) or maximized
        glfwRestoreWindow(platform.handle);
        CORE.Window.flags &= ~FLAG_WINDOW_MINIMIZED;
        CORE.Window.flags &= ~FLAG_WINDOW_MAXIMIZED;
    }
}

// Set window configuration state using flags
void SetWindowState(unsigned int flags)
{
    // Check previous state and requested state to apply required changes
    // NOTE: In most cases the functions already change the flags internally

    // State change: FLAG_VSYNC_HINT
    if (((CORE.Window.flags & FLAG_VSYNC_HINT) != (flags & FLAG_VSYNC_HINT)) && ((flags & FLAG_VSYNC_HINT) > 0))
    {
        glfwSwapInterval(1);
        CORE.Window.flags |= FLAG_VSYNC_HINT;
    }

    // State change: FLAG_BORDERLESS_WINDOWED_MODE
    // NOTE: This must be handled before FLAG_FULLSCREEN_MODE because ToggleBorderlessWindowed() needs to get some fullscreen values if fullscreen is running
    if (((CORE.Window.flags & FLAG_BORDERLESS_WINDOWED_MODE) != (flags & FLAG_BORDERLESS_WINDOWED_MODE)) && ((flags & FLAG_BORDERLESS_WINDOWED_MODE) > 0))
    {
        ToggleBorderlessWindowed();     // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_FULLSCREEN_MODE
    if ((CORE.Window.flags & FLAG_FULLSCREEN_MODE) != (flags & FLAG_FULLSCREEN_MODE))
    {
        ToggleFullscreen();     // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_RESIZABLE
    if (((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) != (flags & FLAG_WINDOW_RESIZABLE)) && ((flags & FLAG_WINDOW_RESIZABLE) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_RESIZABLE, GLFW_TRUE);
        CORE.Window.flags |= FLAG_WINDOW_RESIZABLE;
    }

    // State change: FLAG_WINDOW_UNDECORATED
    if (((CORE.Window.flags & FLAG_WINDOW_UNDECORATED) != (flags & FLAG_WINDOW_UNDECORATED)) && (flags & FLAG_WINDOW_UNDECORATED))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_DECORATED, GLFW_FALSE);
        CORE.Window.flags |= FLAG_WINDOW_UNDECORATED;
    }

    // State change: FLAG_WINDOW_HIDDEN
    if (((CORE.Window.flags & FLAG_WINDOW_HIDDEN) != (flags & FLAG_WINDOW_HIDDEN)) && ((flags & FLAG_WINDOW_HIDDEN) > 0))
    {
        glfwHideWindow(platform.handle);
        CORE.Window.flags |= FLAG_WINDOW_HIDDEN;
    }

    // State change: FLAG_WINDOW_MINIMIZED
    if (((CORE.Window.flags & FLAG_WINDOW_MINIMIZED) != (flags & FLAG_WINDOW_MINIMIZED)) && ((flags & FLAG_WINDOW_MINIMIZED) > 0))
    {
        //GLFW_ICONIFIED
        MinimizeWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_MAXIMIZED
    if (((CORE.Window.flags & FLAG_WINDOW_MAXIMIZED) != (flags & FLAG_WINDOW_MAXIMIZED)) && ((flags & FLAG_WINDOW_MAXIMIZED) > 0))
    {
        //GLFW_MAXIMIZED
        MaximizeWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_UNFOCUSED
    if (((CORE.Window.flags & FLAG_WINDOW_UNFOCUSED) != (flags & FLAG_WINDOW_UNFOCUSED)) && ((flags & FLAG_WINDOW_UNFOCUSED) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
        CORE.Window.flags |= FLAG_WINDOW_UNFOCUSED;
    }

    // State change: FLAG_WINDOW_TOPMOST
    if (((CORE.Window.flags & FLAG_WINDOW_TOPMOST) != (flags & FLAG_WINDOW_TOPMOST)) && ((flags & FLAG_WINDOW_TOPMOST) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_FLOATING, GLFW_TRUE);
        CORE.Window.flags |= FLAG_WINDOW_TOPMOST;
    }

    // State change: FLAG_WINDOW_ALWAYS_RUN
    if (((CORE.Window.flags & FLAG_WINDOW_ALWAYS_RUN) != (flags & FLAG_WINDOW_ALWAYS_RUN)) && ((flags & FLAG_WINDOW_ALWAYS_RUN) > 0))
    {
        CORE.Window.flags |= FLAG_WINDOW_ALWAYS_RUN;
    }

    // The following states can not be changed after window creation

    // State change: FLAG_WINDOW_TRANSPARENT
    if (((CORE.Window.flags & FLAG_WINDOW_TRANSPARENT) != (flags & FLAG_WINDOW_TRANSPARENT)) && ((flags & FLAG_WINDOW_TRANSPARENT) > 0))
    {
        TRACELOG(LOG_WARNING, "WINDOW: Framebuffer transparency can only be configured before window initialization");
    }

    // State change: FLAG_WINDOW_HIGHDPI
    if (((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) != (flags & FLAG_WINDOW_HIGHDPI)) && ((flags & FLAG_WINDOW_HIGHDPI) > 0))
    {
        TRACELOG(LOG_WARNING, "WINDOW: High DPI can only be configured before window initialization");
    }

    // State change: FLAG_WINDOW_MOUSE_PASSTHROUGH
    if (((CORE.Window.flags & FLAG_WINDOW_MOUSE_PASSTHROUGH) != (flags & FLAG_WINDOW_MOUSE_PASSTHROUGH)) && ((flags & FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
        CORE.Window.flags |= FLAG_WINDOW_MOUSE_PASSTHROUGH;
    }

    // State change: FLAG_MSAA_4X_HINT
    if (((CORE.Window.flags & FLAG_MSAA_4X_HINT) != (flags & FLAG_MSAA_4X_HINT)) && ((flags & FLAG_MSAA_4X_HINT) > 0))
    {
        TRACELOG(LOG_WARNING, "WINDOW: MSAA can only be configured before window initialization");
    }

    // State change: FLAG_INTERLACED_HINT
    if (((CORE.Window.flags & FLAG_INTERLACED_HINT) != (flags & FLAG_INTERLACED_HINT)) && ((flags & FLAG_INTERLACED_HINT) > 0))
    {
        TRACELOG(LOG_WARNING, "RPI: Interlaced mode can only be configured before window initialization");
    }

    // State change: FLAG_RESCALE_CONTENT
    if (((CORE.Window.flags & FLAG_RESCALE_CONTENT) != (flags & FLAG_RESCALE_CONTENT)) && ((flags & FLAG_RESCALE_CONTENT) > 0))
    {
        CORE.Window.flags |= FLAG_RESCALE_CONTENT;
        int fbWidth;
        int fbHeight;
        glfwGetFramebufferSize(platform.handle, &fbWidth, &fbHeight);
        WindowSizeCallback(platform.handle, fbWidth, fbHeight);
    }

    // State change: FLAG_TEXT_LINEAR_FILTER
    if (((CORE.Window.flags & FLAG_TEXT_LINEAR_FILTER) != (flags & FLAG_TEXT_LINEAR_FILTER)) && ((flags & FLAG_TEXT_LINEAR_FILTER) > 0))
    {
        CORE.Window.flags |= FLAG_TEXT_LINEAR_FILTER;
        rlTextureParameters(GetFontDefault().texture.id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_LINEAR);
        rlTextureParameters(GetFontDefault().texture.id, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_LINEAR);
    }
}

// Clear window configuration state flags
void ClearWindowState(unsigned int flags)
{
    // Check previous state and requested state to apply required changes
    // NOTE: In most cases the functions already change the flags internally

    // State change: FLAG_VSYNC_HINT
    if (((CORE.Window.flags & FLAG_VSYNC_HINT) > 0) && ((flags & FLAG_VSYNC_HINT) > 0))
    {
        glfwSwapInterval(0);
        CORE.Window.flags &= ~FLAG_VSYNC_HINT;
    }

    // State change: FLAG_BORDERLESS_WINDOWED_MODE
    // NOTE: This must be handled before FLAG_FULLSCREEN_MODE because ToggleBorderlessWindowed() needs to get some fullscreen values if fullscreen is running
    if (((CORE.Window.flags & FLAG_BORDERLESS_WINDOWED_MODE) > 0) && ((flags & FLAG_BORDERLESS_WINDOWED_MODE) > 0))
    {
        ToggleBorderlessWindowed();     // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_FULLSCREEN_MODE
    if (((CORE.Window.flags & FLAG_FULLSCREEN_MODE) > 0) && ((flags & FLAG_FULLSCREEN_MODE) > 0))
    {
        ToggleFullscreen();     // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_RESIZABLE
    if (((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) > 0) && ((flags & FLAG_WINDOW_RESIZABLE) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_RESIZABLE, GLFW_FALSE);
        CORE.Window.flags &= ~FLAG_WINDOW_RESIZABLE;
    }

    // State change: FLAG_WINDOW_HIDDEN
    if (((CORE.Window.flags & FLAG_WINDOW_HIDDEN) > 0) && ((flags & FLAG_WINDOW_HIDDEN) > 0))
    {
        glfwShowWindow(platform.handle);
        CORE.Window.flags &= ~FLAG_WINDOW_HIDDEN;
    }

    // State change: FLAG_WINDOW_MINIMIZED
    if (((CORE.Window.flags & FLAG_WINDOW_MINIMIZED) > 0) && ((flags & FLAG_WINDOW_MINIMIZED) > 0))
    {
        RestoreWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_MAXIMIZED
    if (((CORE.Window.flags & FLAG_WINDOW_MAXIMIZED) > 0) && ((flags & FLAG_WINDOW_MAXIMIZED) > 0))
    {
        RestoreWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_UNDECORATED
    if (((CORE.Window.flags & FLAG_WINDOW_UNDECORATED) > 0) && ((flags & FLAG_WINDOW_UNDECORATED) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_DECORATED, GLFW_TRUE);
        CORE.Window.flags &= ~FLAG_WINDOW_UNDECORATED;
    }

    // State change: FLAG_WINDOW_UNFOCUSED
    if (((CORE.Window.flags & FLAG_WINDOW_UNFOCUSED) > 0) && ((flags & FLAG_WINDOW_UNFOCUSED) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
        CORE.Window.flags &= ~FLAG_WINDOW_UNFOCUSED;
    }

    // State change: FLAG_WINDOW_TOPMOST
    if (((CORE.Window.flags & FLAG_WINDOW_TOPMOST) > 0) && ((flags & FLAG_WINDOW_TOPMOST) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_FLOATING, GLFW_FALSE);
        CORE.Window.flags &= ~FLAG_WINDOW_TOPMOST;
    }

    // State change: FLAG_WINDOW_ALWAYS_RUN
    if (((CORE.Window.flags & FLAG_WINDOW_ALWAYS_RUN) > 0) && ((flags & FLAG_WINDOW_ALWAYS_RUN) > 0))
    {
        CORE.Window.flags &= ~FLAG_WINDOW_ALWAYS_RUN;
    }

    // The following states can not be changed after window creation

    // State change: FLAG_WINDOW_TRANSPARENT
    if (((CORE.Window.flags & FLAG_WINDOW_TRANSPARENT) > 0) && ((flags & FLAG_WINDOW_TRANSPARENT) > 0))
    {
        TRACELOG(LOG_WARNING, "WINDOW: Framebuffer transparency can only be configured before window initialization");
    }

    // State change: FLAG_WINDOW_HIGHDPI
    if (((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) > 0) && ((flags & FLAG_WINDOW_HIGHDPI) > 0))
    {
        TRACELOG(LOG_WARNING, "WINDOW: High DPI can only be configured before window initialization");
    }

    // State change: FLAG_WINDOW_MOUSE_PASSTHROUGH
    if (((CORE.Window.flags & FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0) && ((flags & FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_MOUSE_PASSTHROUGH, GLFW_FALSE);
        CORE.Window.flags &= ~FLAG_WINDOW_MOUSE_PASSTHROUGH;
    }

    // State change: FLAG_MSAA_4X_HINT
    if (((CORE.Window.flags & FLAG_MSAA_4X_HINT) > 0) && ((flags & FLAG_MSAA_4X_HINT) > 0))
    {
        TRACELOG(LOG_WARNING, "WINDOW: MSAA can only be configured before window initialization");
    }

    // State change: FLAG_INTERLACED_HINT
    if (((CORE.Window.flags & FLAG_INTERLACED_HINT) > 0) && ((flags & FLAG_INTERLACED_HINT) > 0))
    {
        TRACELOG(LOG_WARNING, "RPI: Interlaced mode can only be configured before window initialization");
    }

    // State change: FLAG_RESCALE_CONTENT
    if (((CORE.Window.flags & FLAG_RESCALE_CONTENT) > 0) && ((flags & FLAG_RESCALE_CONTENT) > 0))
    {
        CORE.Window.flags &= ~FLAG_RESCALE_CONTENT;
        int fbWidth;
        int fbHeight;
        glfwGetFramebufferSize(platform.handle, &fbWidth, &fbHeight);
        WindowSizeCallback(platform.handle, fbWidth, fbHeight);
    }

    // State change: FLAG_TEXT_LINEAR_FILTER
    if (((CORE.Window.flags & FLAG_TEXT_LINEAR_FILTER) > 0) && ((flags & FLAG_TEXT_LINEAR_FILTER) > 0))
    {
        CORE.Window.flags &= ~FLAG_TEXT_LINEAR_FILTER;
        rlTextureParameters(GetFontDefault().texture.id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_NEAREST);
        rlTextureParameters(GetFontDefault().texture.id, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_NEAREST);
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

            // NOTE 1: We only support one image icon
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
        GLFWimage *icons = RL_CALLOC(count, sizeof(GLFWimage));

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
    glfwSetWindowPos(platform.handle, x, y);

    // GLFW 3.4 may trigger an error if platform like Wayland don't support setting or getting the position of the window
    glfwGetError(NULL); // So we must clear this error
}

// Set monitor for the current window
void SetWindowMonitor(int monitor)
{
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        bool changingMonitor = (CORE.Window.lastAssociatedMonitorIndex != monitor);

        CORE.Window.lastAssociatedMonitorIndex = monitor;

        if ((CORE.Window.fullscreen) || IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE))
        {
            TRACELOG(LOG_INFO, "GLFW: Selected fullscreen monitor: [%i] %s", monitor, glfwGetMonitorName(monitors[monitor]));

            const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);
            glfwSetWindowMonitor(platform.handle, monitors[monitor], 0, 0, mode->width, mode->height, mode->refreshRate);
        }
        else
        if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND)
        {
            // On current implementation of Wayland and GLFW, 
            // this is the only way i found to move the window 
            // from one monitor to another :

            if (changingMonitor && !IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE))
            {
                //!\ TODO FIXME This workaround might causes some unexpected issues and could be improved

                ToggleBorderlessWindowed(); // The fullscreen window is automatically moved to the monitor
                ToggleBorderlessWindowed(); // and is restored as windowed window on the same monitor.
            }
        }
        else
        {
            TRACELOG(LOG_INFO, "GLFW: Selected monitor: [%i] %s", monitor, glfwGetMonitorName(monitors[monitor]));

            const int screenWidth = CORE.Window.screen.width;
            const int screenHeight = CORE.Window.screen.height;
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

            // GLFW 3.4 may trigger an error if platform like Wayland don't support setting or getting the position of the window
            glfwGetError(NULL); // So we must clear this error

            // NOTE : As of current version of Wayland and GLFW, it is not possible to set the position of a windowed 
            //        window by code, so this fonction will not move it from a display to another. Instead, it will 
            //        associate a monitor index to the window for fullscreen mode.

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

// Get native window handle
void *GetWindowHandle(void)
{
#if defined(_WIN32)
    // NOTE: Returned handle is: void *HWND (windows.h)
    return glfwGetWin32Window(platform.handle);
#endif
#if defined(__linux__)
    // NOTE: Returned handle is: unsigned long Window (X.h)
    // typedef unsigned long XID;
    // typedef XID Window;
    //unsigned long id = (unsigned long)glfwGetX11Window(platform.handle);
    //return NULL;    // TODO: Find a way to return value... cast to void *?
    return (void *)platform.handle;
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

// Get number of monitors
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
        if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND)
        {
            // The algorithm bellow, which relies on the window position, won't work with current version of Wayland
            // and GLFW, because Wayland does not allow to know the window position.
            // NOTE : this is not a bug in Wayland or GLFW, it is a "feature design" of current version of Wayland.

            // So intead of computing on which monitor is mostly displayed the window, we return the index of the monitor
            // to which the user wanted to associate the window to using `SetWindowMonitor()` :

            index = CORE.Window.lastAssociatedMonitorIndex;

            if ((index < 0) || (index >= monitorCount))
            {
                index = monitorCount - 1;

                // A monitor was probably unplugged.
                // TODO : add a TRACELOG ?

                CORE.Window.lastAssociatedMonitorIndex = index;
            }
        }
        else
        {
            // In case the window is between two monitors, we use below logic
            // to try to detect the "current monitor" for that window, note that
            // this is probably an overengineered solution for a very side case
            // trying to match SDL behaviour

            // NOTE : this algorithm won't work with Wayland because it does not allow to know the window position.

            int closestDist = 0x7FFFFFFF;

            // Window center position
            int wcx = 0;
            int wcy = 0;

            glfwGetWindowPos(platform.handle, &wcx, &wcy);
            wcx += (int)CORE.Window.screen.width/2;
            wcy += (int)CORE.Window.screen.height/2;

            // GLFW 3.4 may trigger an error if platform like Wayland don't support setting or getting the position of the window
            glfwGetError(NULL); // So we must clear this error

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
        int x, y;
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
        const GLFWvidmode *vidmode = glfwGetVideoMode(monitors[monitor]);
        refresh = vidmode->refreshRate;
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
    int x = 0;
    int y = 0;

    glfwGetWindowPos(platform.handle, &x, &y);

    // GLFW 3.4 may trigger an error if platform like Wayland don't support setting or getting the position of the window
    glfwGetError(NULL); // So we must clear this error

    return (Vector2){ (float)x, (float)y };
}

// Get window scale DPI factor for current monitor
Vector2 GetWindowScaleDPI(void)
{
    Vector2 scale = {1.0, 1.0};

    // On platforms like Wayland, the window and its frameBuffer are automatically rescaled
    // according to the current DPI scale of the display on which the window is visible, 
    // `glfwGetWindowContentScale()` will always return `{1.0, 1.0}`, because there is no need
    // for the application to apply any DPI rescaling by itself.
    // So, to keep consistency between all supported platforms, if `FLAG_WINDOW_HIGHDPI` is enabled
    // we ignore the value returned by `glfwGetWindowContentScale()` and return `{1.0, 1.0}` instead.
    // Same if we are in an hardware/exclusive fullscreen mode.

    if (!IsWindowState(FLAG_WINDOW_HIGHDPI) && !CORE.Window.fullscreen) 
    {
        // We only need to know the DPI scale of the display if we have to rescale our UI and texts manually by code.
        // TODO @SoloByte __APPLE__ test mission

        glfwGetWindowContentScale(platform.handle, &scale.x, &scale.y);
    }

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
}

// Disables cursor (lock cursor)
void DisableCursor(void)
{
    glfwSetInputMode(platform.handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set cursor position in the middle
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);

    if (glfwRawMouseMotionSupported()) glfwSetInputMode(platform.handle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    CORE.Input.Mouse.cursorHidden = true;
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
// NOTE: This function is only safe to use if you control the URL given.
// A user could craft a malicious string performing another action.
// Only call this function yourself not with user input or make sure to check the string yourself.
// Ref: https://github.com/raysan5/raylib/issues/686
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
void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor)
{
    TRACELOG(LOG_WARNING, "GamepadSetVibration() not available on target platform");
}

// Set the system mouse position XY in `CORE.Window.screen` coordinates 
// NOTE : user's `SetMouseOffset()` and `SetMouseScale()` are ignored.
void SetMousePosition(int x, int y)
{
    // If the requested mouse coordinates are outside the screen, we clip them :

    x = (x < 0) ? 0 : ((x < CORE.Window.screen.width) ? x : (CORE.Window.screen.width - 1));
    y = (y < 0) ? 0 : ((y < CORE.Window.screen.height) ? y : (CORE.Window.screen.height - 1));

    // We rescale and offset the requested coordinates according to the platform's setup,
    // so it also works when `FLAG_WINDOW_HIGHDPI` or `FLAG_RESCALE_CONTENT` are enabled.
    // User's mouse offset and rescaling must be ignored, because they could put the mouse cursor outside the window.

    float mouseX = (float)x/CORE.Input.Mouse.platformScale.x - CORE.Input.Mouse.platformOffset.x;
    float mouseY = (float)y/CORE.Input.Mouse.platformScale.y - CORE.Input.Mouse.platformOffset.y;

    // Update the metrics :

    CORE.Input.Mouse.currentPosition = (Vector2){ mouseX, mouseY };
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;

    // And we're done :

    glfwSetCursorPos(platform.handle, mouseX, mouseY);
}

// Set mouse cursor
void SetMouseCursor(int cursor)
{
    CORE.Input.Mouse.cursor = cursor;
    if (cursor == MOUSE_CURSOR_DEFAULT) glfwSetCursor(platform.handle, NULL);
    else
    {
        // NOTE: We are relating internal GLFW enum values to our MouseCursor enum values
        glfwSetCursor(platform.handle, glfwCreateStandardCursor(0x00036000 + cursor));
    }
}

// Get physical key name.
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
    // TODO: GLFW does not support multi-touch input just yet
    // https://www.codeproject.com/Articles/668404/Programming-for-Multi-Touch
    // https://docs.microsoft.com/en-us/windows/win32/wintouch/getting-started-with-multi-touch-messages
    CORE.Input.Touch.position[0] = CORE.Input.Mouse.currentPosition;

    // Check if gamepads are ready
    // NOTE: We do it here in case of disconnection
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
            // NOTE: There is no callback available, so we get it manually
            GLFWgamepadstate state = { 0 };
            glfwGetGamepadState(i, &state); // This remapps all gamepads so they have their buttons mapped like an xbox controller

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

            // Get current axis state
            const float *axes = state.axes;

            for (int k = 0; (axes != NULL) && (k < GLFW_GAMEPAD_AXIS_LAST + 1); k++)
            {
                CORE.Input.Gamepad.axisState[i][k] = axes[k];
            }

            // Register buttons for 2nd triggers (because GLFW doesn't count these as buttons but rather axis)
            CORE.Input.Gamepad.currentButtonState[i][GAMEPAD_BUTTON_LEFT_TRIGGER_2] = (char)(CORE.Input.Gamepad.axisState[i][GAMEPAD_AXIS_LEFT_TRIGGER] > 0.1f);
            CORE.Input.Gamepad.currentButtonState[i][GAMEPAD_BUTTON_RIGHT_TRIGGER_2] = (char)(CORE.Input.Gamepad.axisState[i][GAMEPAD_AXIS_RIGHT_TRIGGER] > 0.1f);

            CORE.Input.Gamepad.axisCount[i] = GLFW_GAMEPAD_AXIS_LAST + 1;
        }
    }

    CORE.Window.resizedLastFrame = false;

    if (CORE.Window.eventWaiting) glfwWaitEvents();     // Wait for in input events before continue (drawing is paused)
    else glfwPollEvents();      // Poll input events: keyboard/mouse/window events (callbacks) -> Update keys state

    // While window minimized, stop loop execution
    while (IsWindowState(FLAG_WINDOW_MINIMIZED) && !IsWindowState(FLAG_WINDOW_ALWAYS_RUN)) glfwWaitEvents();

    CORE.Window.shouldClose = glfwWindowShouldClose(platform.handle);

    // Reset close status for next frame
    glfwSetWindowShouldClose(platform.handle, GLFW_FALSE);
}


//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------


// Initialize platform: graphics, inputs and more
int InitPlatform(void)
{
    glfwSetErrorCallback(ErrorCallback);
/*
    // TODO: Setup GLFW custom allocators to match raylib ones
    const GLFWallocator allocator = {
        .allocate = MemAlloc,
        .deallocate = MemFree,
        .reallocate = MemRealloc,
        .user = NULL
    };

    glfwInitAllocator(&allocator);
*/

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

    // Check window creation flags
    if ((CORE.Window.flags & FLAG_FULLSCREEN_MODE) > 0) CORE.Window.fullscreen = true;

    if ((CORE.Window.flags & FLAG_WINDOW_HIDDEN) > 0) glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Visible window
    else glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);     // Window initially hidden

    if ((CORE.Window.flags & FLAG_WINDOW_UNDECORATED) > 0) glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Border and buttons on Window
    else glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);   // Decorated window

    if ((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) > 0) glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // Resizable window
    else glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);  // Avoid window being resizable

    // Disable FLAG_WINDOW_MINIMIZED, not supported on initialization
    if ((CORE.Window.flags & FLAG_WINDOW_MINIMIZED) > 0) CORE.Window.flags &= ~FLAG_WINDOW_MINIMIZED;

    // Disable FLAG_WINDOW_MAXIMIZED, not supported on initialization
    if ((CORE.Window.flags & FLAG_WINDOW_MAXIMIZED) > 0) CORE.Window.flags &= ~FLAG_WINDOW_MAXIMIZED;

    if ((CORE.Window.flags & FLAG_WINDOW_UNFOCUSED) > 0) glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
    else glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

    if ((CORE.Window.flags & FLAG_WINDOW_TOPMOST) > 0) glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    else glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);

    // NOTE: Some GLFW flags are not supported on HTML5
    if ((CORE.Window.flags & FLAG_WINDOW_TRANSPARENT) > 0) glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);     // Transparent framebuffer
    else glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);  // Opaque framebuffer


    if ((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) > 0)
    {
        // When the DPI of the monitor on which the window is displayed is changed,
        // the hint bellow ask GLFW to resize the window accordingly to the DPI :

        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE); // Only has effect on Windows and X11

        // The hints bellow ask GLFW to resize the frameBuffer accordingly to the DPI.
        // It only has effet MacOS and Wayland.

        glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_FALSE); // Wayland needs it to be false
        // TODO @SoloByte __APPLE__ test mission

        if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND)
        {
            // On Wayland, `FLAG_WINDOW_HIGHDPI` must have no effect because the window and its framebuffer are 
            // automatically rescaled by the OS. So, instead of rewriting the pipeline to add a Wayland special case 
            // everywhere this flag is involved, we just disable it from here so it has no more effect afterward.
            CORE.Window.flags &= ~FLAG_WINDOW_HIGHDPI;
        }
    }
    else 
    {
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE); 
        glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_FALSE); // Wayland needs it to be false
        // TODO @SoloByte __APPLE__ test mission 
    }

    // Mouse passthrough
    if ((CORE.Window.flags & FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0)
    {
        glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
    }
    else 
    {
        glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_FALSE);
    }

    if (CORE.Window.flags & FLAG_MSAA_4X_HINT)
    {
        // NOTE: MSAA is only enabled for main framebuffer, not user-created FBOs
        TRACELOG(LOG_INFO, "DISPLAY: Trying to enable MSAA x4");
        glfwWindowHint(GLFW_SAMPLES, 4);   // Tries to enable multisampling x4 (MSAA), default is 0
    }

    // NOTE: When asking for an OpenGL context version, most drivers provide the highest supported version
    // with backward compatibility to older OpenGL versions.
    // For example, if using OpenGL 1.1, driver can provide a 4.3 backwards compatible context.

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

    // NOTE: GLFW 3.4+ defers initialization of the Joystick subsystem on the first call to any Joystick related functions.
    // Forcing this initialization here avoids doing it on PollInputEvents() called by EndDrawing() after first frame has been just drawn.
    // The initialization will still happen and possible delays still occur, but before the window is shown, which is a nicer experience.
    // REF: https://github.com/raysan5/raylib/issues/1554
    glfwSetJoystickCallback(NULL);


    // Window creation :
    //-----------------------------------

    // For the sake of code readability, we're going to establish a boolean vocabulary :

    bool requestWindowHDPI          = (CORE.Window.flags & FLAG_WINDOW_HIGHDPI) > 0;
    
    bool requestVsync               = (CORE.Window.flags & FLAG_VSYNC_HINT) > 0;

    bool invalidWindowSizeRequested = (CORE.Window.screen.width <= 0) || (CORE.Window.screen.height <= 0);

    // In the original source code `InitWindow(0,0)` was interpreted as a request 
    // for "windowed fullscreen mode" which is also triggered by `FLAG_BORDERLESS_WINDOWED_MODE`.
    // We replicate this behavior to keep backward compatibility.
    // So if the requested window size is invalid, it is considered a "Borderless windowed" fullscreen mode.

    bool requestBorderlessWindowed = invalidWindowSizeRequested || (CORE.Window.flags & FLAG_BORDERLESS_WINDOWED_MODE);
    bool requestHardwareFullscreen = CORE.Window.fullscreen ;

    bool requestWindowedWindow     = !requestBorderlessWindowed && !requestHardwareFullscreen;

    // We can't have both fullscreen modes requested at the same time so we have to make a choice :

    if (requestBorderlessWindowed && requestHardwareFullscreen) 
    {
        TRACELOG(LOG_WARNING, "DISPLAY: both fullscreen modes were requested. Ignoring `FLAG_FULLSCREEN_MODE`.");

        requestHardwareFullscreen = false ; // update the boolean vocabulary 

        // And let's not forget to update the real flags :
        CORE.Window.flags &= ~FLAG_FULLSCREEN_MODE;
        CORE.Window.fullscreen = false ;
    }

    // By default, when a fullscreen window loses focus, GLFW iconifies it and restores the desktop monitor resolution.
    // This default behavior can be emulated on user's side with a simple code : `if ( ! IsWindowFocused() ) MinimizeWindow();`
    // So we disable this GLFW default behavior and let the user decides by themself the behavior of their program,
    // because they may want their fullscreen program to remain visible while interacting with other application on 
    // a secondary monitor.

    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);

    // Even if a fullscreen mode is requested, for now we will just open a windowed window with a minimal size. 
    // Next, this initial window will be resized and positionned according to what it should be restored to if the user toggles from fullscreen.
    // Once the window fully ready, we will toggle the requested fullscreen mode.
    // NOTE : this means the hardware fullscreen will occurs on the same display than the window will be displayed on.
    // TODO : make it possible to request a hardware fullscreen resolution bigger than current monitor display resolution.

    int frameBufferWidth  = 100 ;
    int frameBufferHeight = 100 ;
    
    // Now let's create the window :

    const char *windowTitle = (CORE.Window.title != 0) ? CORE.Window.title : " ";

    platform.handle = glfwCreateWindow(frameBufferWidth, frameBufferHeight, windowTitle, NULL, NULL);

    if (platform.handle == NULL)
    {
        TRACELOG(LOG_WARNING, "GLFW: Failed to initialize Window");
        glfwTerminate();
        return -1;
    }

    // Now that the window is created, we can determine on which monitor it was assigned to
    // and derive other missing metrics from the configuration of the display :
    // NOTE : we can't use glfwGetWindowMonitor() directly because it only works if the window is 
    // already in hardware fullscreen mode, otherwise it returns NULL. So we use `GetCurrentMonitor()`.

    if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND)
    {
        // On current version of Wayland and GLFW it is not possible to know on which monitor the window
        // is mostly visible onto. On some distro, the window will just open on the monitor with the mouse cursor.
        // Unfortunately, the mouse coordinates returned by GLFW are always relative to the top left corner of the
        // window frameBuffer. It is thus impossible to guess on which monitor the window was opened.
        // So we pretend it was on the main monitor, because it is the most common scenario.
        // It is thus possible to associate an other monitor to the window afterward using `SetWindowMonitor()`.
        // However, this association will only have an effect when toggling the window to hardware fullscreen mode.

        CORE.Window.lastAssociatedMonitorIndex = 0;
    }
    else
    {
        CORE.Window.lastAssociatedMonitorIndex = GetCurrentMonitor();
    }

    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if (CORE.Window.lastAssociatedMonitorIndex >= monitorCount)
    {
        TRACELOG(LOG_WARNING, "GLFW: Failed to determine Monitor to center Window");
        glfwTerminate();
        return -1;
    }

    GLFWmonitor *monitor = monitors[CORE.Window.lastAssociatedMonitorIndex];

    // We got the monitor, now let's get its current video mode :

    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    if (mode == NULL)
    {
        TRACELOG(LOG_WARNING, "GLFW: Failed to get Monitor video mode");
        glfwTerminate();
        return -1;
    }

    // Now we can update all the missing metrics :

    CORE.Window.display.width = mode->width;
    CORE.Window.display.height = mode->height;

    if (invalidWindowSizeRequested) 
    {
        // For backward compatibility purpose, an invalid window size is interpreted as a requestBorderlessWindowed.
        // The `FLAG_BORDERLESS_WINDOWED_MODE` flag will be set when `ToggleBorderlessWindowed()` will be called later
        // once we're done setting up the window.
        // For now, we must give an arbitrary valid size to this window.
        // This is the size the window will have if restored from fullscreen mode.

        CORE.Window.screen.width = mode->width*3/4;
        CORE.Window.screen.height = mode->height*3/4;
    }

    // If the user did not requestWindowHDPI, the size of the frameBuffer is the same as the requested screen size :

    frameBufferWidth = CORE.Window.screen.width;
    frameBufferHeight = CORE.Window.screen.height;

    CORE.Window.currentFbo.width = frameBufferWidth;
    CORE.Window.currentFbo.height = frameBufferHeight;

    // If the user requestWindowHDPI scaling, we must update the size of the initial frameBuffer.

    if (requestWindowHDPI)
    {

        float dpiScaleX, dpiScaleY;
        glfwGetWindowContentScale(platform.handle, &dpiScaleX, &dpiScaleY);

        // TODO : take `__APPLE__` special case into account. @SoloByte mission (don't forget to give a look at `_SetupFramebuffer()` just in case the special case is already managed there)
        // TODO : take Wayland special case into account.  (don't forget to give a look at `_SetupFramebuffer()` just in case the special case is already managed there)
        // TODO : take `FLAG_RESCALE_CONTENT` into account ? (don't forget to give a look at `_SetupFramebuffer()` just in case the special case is

        frameBufferWidth  = (int)roundf((float)frameBufferWidth*dpiScaleX);
        frameBufferHeight = (int)roundf((float)frameBufferHeight*dpiScaleY);
    }

    // Now that we have the correct frameBuffer size, we can resize the window :

    glfwSetWindowSize(platform.handle, frameBufferWidth, frameBufferHeight);

    // We are also ready to recompute all the involved metrics using known `CORE.Window.screen` and frameBuffer sizes :

    _SetupFramebuffer(frameBufferWidth, frameBufferHeight, false); //!\ We're still in windowed mode for now, so the 3rd argument is `false`

    // Even if the user requested a fullscreen mode, we're still in windowed mode for now.
    // We will toggle to the requested fullscreen mode once we will be done setting
    // the window as it should be restored if the user decide to leave the fullscreen
    // mode afterward.

    // So let's try to center it on the desktop :

    int monitorX = 0;
    int monitorY = 0;
    int monitorWidth = 0;
    int monitorHeight = 0;
    glfwGetMonitorWorkarea(monitor, &monitorX, &monitorY, &monitorWidth, &monitorHeight);

    int posX = monitorX + (monitorWidth  - frameBufferWidth)/2;
    int posY = monitorY + (monitorHeight - frameBufferHeight)/2;

    // If the frameBuffer was larger than the desktop area, we offset its position :
    if (posX < monitorX) posX = monitorX;
    if (posY < monitorY) posY = monitorY;

    glfwSetWindowPos(platform.handle, posX, posY);  //!\ NOTE : This function sets the position of the upper-left corner 
                                                    //          of the content area (aka the "frameBuffer") and not the 
                                                    //          upper-left corner of the window's decoration. 
                                                    //          This means that the desktop manager or GLFW might not be
                                                    //          able to fullfil this request and might change the position.

    // GLFW 3.4 may trigger an error if platform like Wayland don't support setting or getting the position of the window
    glfwGetError(NULL); // So we must clear this error

    // As our windowed window is almost ready, we can associate the OpenGL context to it :

    glfwMakeContextCurrent(platform.handle); // NOTE : from here, we must `glfwDestroyWindow()` before any `glfwTerminate()`

    result = glfwGetError(NULL);

    if (result != GLFW_NO_ERROR)
    {
        TRACELOG(LOG_FATAL, "PLATFORM: Failed to initialize graphics device");
        glfwDestroyWindow(platform.handle);
        glfwTerminate();
        return -1;
    }

    // From here, we should have a working windowed window with a working OpenGL context :

    CORE.Window.ready = true;

    // Activate V-Sync if requested :
    //--------------------------------------------------------------------------

    glfwSwapInterval(0);        // No V-Sync by default

    // Try to enable GPU V-Sync, so frames are limited to screen refresh rate (60Hz -> 60 FPS)
    // NOTE: V-Sync can be enabled by graphic driver configuration, it doesn't need
    // to be activated on web platforms since VSync is enforced there.
    if (requestVsync)
    {
        // WARNING: It seems to hit a critical render path in Intel HD Graphics
        glfwSwapInterval(1);
        TRACELOG(LOG_INFO, "DISPLAY: Trying to enable VSYNC");
    }


    // Complete platform intialization :
    //----------------------------------------------------------------------------

    // Load OpenGL extensions
    // NOTE: GL procedures address loader is required to load extensions
    rlLoadExtensions(glfwGetProcAddress);
    //----------------------------------------------------------------------------

    // Initialize input events callbacks
    //----------------------------------------------------------------------------
    // Set window callback events
    
    //!\ `glfwSetFramebufferSizeCallback()` offers better cross-platform compatibility than `glfwSetWindowSizeCallback()`
    glfwSetFramebufferSizeCallback(platform.handle, WindowSizeCallback); 
    
    glfwSetWindowMaximizeCallback(platform.handle, WindowMaximizeCallback);
    glfwSetWindowIconifyCallback(platform.handle, WindowIconifyCallback);
    glfwSetWindowFocusCallback(platform.handle, WindowFocusCallback);
    glfwSetDropCallback(platform.handle, WindowDropCallback);

#if defined(_WIN32)
    // On Windows 11, in multi-monitor setups, each display might have 
    // its own DPI scaling. When moving ANY window (including Calc.exe) 
    // from one monitor to another, it appears that Windows 11 resizes 
    // this window according to the DPI scaling of the destination display.
    // This also affects when toggle from fullscreen to windowed mode
    // multiple time.
    // This behavior interferes with the current pipeline, and could
    // triggers some tiny changes in screen and render sizes that could 
    // surprise the user or the developper.
    // Unfortunately, I could not find any standard solution to disable 
    // this behavior. So, for now, it will be a "can't fix".
    
    // TODO FIXME : find which version of Windows are affected ?
    // TODO FIXME : find a way to disable this behavior using a GLFW ?
    // TODO FIXME : find a way to disable this behavior using a Win32 API call ?

    // We enable the callback just to help the programmer notice the TRACELOG
    // triggered by the change of scale, which may lead them to this instructive
    // comment.
    // NOTE : WindowContentScaleCallback() is not expected to do anything
    // beside triggering a TRACELOG.

    glfwSetWindowContentScaleCallback(platform.handle, WindowContentScaleCallback);

#else
    if ((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) > 0)
    {
        glfwSetWindowContentScaleCallback(platform.handle, WindowContentScaleCallback);
    }
#endif

    // Set input callback events
    glfwSetKeyCallback(platform.handle, KeyCallback);
    glfwSetCharCallback(platform.handle, CharCallback);
    glfwSetMouseButtonCallback(platform.handle, MouseButtonCallback);
    glfwSetCursorPosCallback(platform.handle, MouseCursorPosCallback);   // Track mouse position changes
    glfwSetScrollCallback(platform.handle, MouseScrollCallback);
    glfwSetCursorEnterCallback(platform.handle, CursorEnterCallback);
    glfwSetJoystickCallback(JoystickCallback);

    glfwSetInputMode(platform.handle, GLFW_LOCK_KEY_MODS, GLFW_TRUE);    // Enable lock keys modifiers (CAPS, NUM)

    // Retrieve gamepad names
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        if (glfwJoystickPresent(i)) strcpy(CORE.Input.Gamepad.name[i], glfwGetJoystickName(i));
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
    char *glfwPlatform = "X11";
#else
    char *glfwPlatform = "";
    switch (glfwGetPlatform())
    {
        case GLFW_PLATFORM_WIN32:   glfwPlatform = "Win32";   break;
        case GLFW_PLATFORM_COCOA:   glfwPlatform = "Cocoa";   break;
        case GLFW_PLATFORM_WAYLAND: glfwPlatform = "Wayland"; break;
        case GLFW_PLATFORM_X11:     glfwPlatform = "X11";     break;
        case GLFW_PLATFORM_NULL:    glfwPlatform = "Null";    break;
    }
#endif

    //
    // Activate fullscreen mode if it was requested
    //----------------------------------------------------------------------------

    // Now that we have our callbacks ready, we can safely rely on them to apply the appropriate 

    if (requestWindowedWindow)
    {
        // We need to call the resize callback at least once
        WindowSizeCallback(platform.handle, frameBufferWidth, frameBufferHeight);
    }
    else
    if (requestBorderlessWindowed)
    {
        // Before being able to call ToggleBorderlessWindowed()
        // we must unset this flag it was explicitely set
        // otherwise ToggleBorderlessWindowed() will beleive it is
        // already in windowed fullscreen mode. 
        CORE.Window.flags &= ~FLAG_BORDERLESS_WINDOWED_MODE;
        ToggleBorderlessWindowed();
    }
    else
    if (requestHardwareFullscreen)
    {
        // We don't use `ToggleFullscreen()` directly because `CORE.Window.fullscreen` is already set to `true`,
        // and `ToggleFullscreen()` would think it is already in fullscreen mode.

        bool _result = _ActivateHardwareFullscreenMode(CORE.Window.lastAssociatedMonitorIndex, CORE.Window.screen.width, CORE.Window.screen.height, GLFW_DONT_CARE);
        if ( _result == false )
        {
            TRACELOG(LOG_WARNING,"DISPLAY: failed to activate fullscreen mode.");
            // We don't close the window, nor terminate GLFW, nor interrupt the paltform intialization because it is just a warning.
            // The user should still be able to use and access the window in windowed mode.
            // TODO : see with @raysan5 how to formalize the returned error codes.

            CORE.Window.fullscreen = false;
            CORE.Window.flags &= ~FLAG_FULLSCREEN_MODE;
        }
    }

    glfwFocusWindow(platform.handle);
   
    // Not sure why a user would want to minimize a window just after creation
    // but for the sake of backward compatibility, we leave this option available here :

    if ((CORE.Window.flags & FLAG_WINDOW_MINIMIZED) > 0) MinimizeWindow();

    // We're done !

    TRACELOG(LOG_INFO, "GLFW platform: %s", glfwPlatform);
    TRACELOG(LOG_INFO, "PLATFORM: DESKTOP (GLFW): Initialized successfully");

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

// GLFW3 Error Callback, runs on GLFW3 error
static void ErrorCallback(int error, const char *description)
{
    TRACELOG(LOG_WARNING, "GLFW: Error: %i Description: %s", error, description);
}

// GLFW3 WindowSize Callback
// Called each time the window is resized, toggled to/from fullscreen mode,
// or when the DPI scale of the monitor is updated if `FLAG_WINDOW_HIGHDPI` is set.
// NOTE: Window resizing and DPI scaling are not enabled by default.
static void WindowSizeCallback(GLFWwindow *window, int frameBufferWidth, int frameBufferHeight)
{
    CORE.Window.currentFbo.width = frameBufferWidth;
    CORE.Window.currentFbo.height = frameBufferHeight;

    CORE.Window.resizedLastFrame = true; // Update the indicator returned by `IsWindowResized()`

    _SetupFramebuffer(frameBufferWidth, frameBufferHeight, CORE.Window.fullscreen);

    SetupViewport(CORE.Window.render.width, CORE.Window.render.height);

    // We have to rescale and offset the mouse coordinate system 
    // so that it always matches the offset and scale of the "screen" surface

    _SetupPlatformMouseScaleAndOffset();

    // NOTE: Postprocessing texture is not scaled to new size
}

static void _SetPlatformMouseOffset(int offsetX, int offsetY)
{
    CORE.Input.Mouse.platformOffset = (Vector2){ (float)offsetX, (float)offsetY };
}

static void _SetPlatformMouseScale(float scaleX, float scaleY)
{
    CORE.Input.Mouse.platformScale = (Vector2){ scaleX, scaleY };
}

static void _SetupPlatformMouseScaleAndOffset()
{
    // Depending the current viewport and setup, we have to rescale and offset the mouse coordinate system
    // returned by Raylib's mouse API functions.

    // TODO : test __APPLE__ special case, @SoloByte mission
 
#if defined(__APPLE__)
    _SetPlatformMouseScale(1.0, 1.0); //!\ We don't use the user's side SetMouseScale()
#else
    if (IsWindowState(FLAG_RESCALE_CONTENT) || (glfwGetPlatform() != GLFW_PLATFORM_WAYLAND))
    {
        float mouseScaleX = (float)CORE.Window.screen.width/(float)CORE.Window.render.width;
        float mouseScaleY = (float)CORE.Window.screen.height/(float)CORE.Window.render.height;
        
        _SetPlatformMouseScale(mouseScaleX, mouseScaleY); //!\ We don't use the user's side SetMouseScale()
    }
    else
    {
        _SetPlatformMouseScale(1.0, 1.0); //!\ We don't use the user's side SetMouseScale()
    }
#endif

    float mouseOffsetX = -0.5f*CORE.Window.renderOffset.x;
    float mouseOffsetY = -0.5f*CORE.Window.renderOffset.y;

    _SetPlatformMouseOffset(mouseOffsetX, mouseOffsetY); //!\ We don't use the user's side SetMouseOffset()
}


// Local reimplementation of SetupFramebuffer()
static void _SetupFramebuffer(int frameBufferWidth, int frameBufferHeight, bool inHardwareFullscreenMode)
{
    // To avoid breaking backward compatibility and other platforms backends
    // that rely on `rcore.c` implementations of `SetupFramebuffer()` and `SetupViewport()`
    // this GLFW backend will use this customized `_SetupFramebuffer()` version instead.

    // This version of `_SetupFramebuffer()` is meant to work with the unchanged `SetupViewport()` as of 2024-07-17.

    // Let's clarify a little to avoid further confusion ...

    // We use these definitions :
    // --`frameBuffer`             : size of the content surface of the window in which Raylib is allowed to render.
    //                               This is the frameBuffer that is cleared with `ClearBackground()`
    // - `CORE.Window.display`     : size of the display resolution of the main monitor on which the window is located.
    // - `CORE.Window.render`      : size of the viewport in which Raylib is allowed to draw. 
    //                               The `render` VP may occupy the whole frameBuffer, or just a part of it.
    // - `CORE.Window.renderOffset`: if the `render` VP is smaller than the frameBuffer, it can be centered.
    // - `CORE.Window.screen`      : the "screen" is a 2D space that is rescaled to fit inside the render viewport.
    // - `CORE.Window.screenScale` : scale of the "screen" so it fits inside the "render" viewport.

    // +---[monitor]-------------------+
    // | +----[display]--------------+ |
    // | |                           | |
    // | |  #[window]############    | |
    // | |  |+-[frameBuffer]---+|    | |
    // | |  ||  +-[render]--+  ||    | |
    // | |  ||  |           |  ||    | |
    // | |  ||  |  screen   |  ||    | |
    // | |  ||  | (rescaled)|  ||    | |
    // | |  ||  |___________|  ||    | |
    // | |  ||_________________||    | |
    // | |  +-------------------+    | |
    // | |                           | |
    // | +---------------------------+ |
    // +-------------------------------+


    // When `FLAG_RESCALE_CONTENT` is disabled (default and backward compatible mode) :
    // - the size of `render` is always the same as the size of the frameBuffer
    // - the size of `screen` is unscaled (scale 1.0), and is thus the same as `render`
    // - the size of `screen` and `render` are expressed in pixels
    // When `FLAG_WINDOW_HIGHDPI` is also enabled :
    // - the size of the frameBuffer provided as arguments, is already rescaled accordingly to the DPI scale 
    // - the size of `render` is always the same as the size of the rescaled frameBuffer
    // - the size of `screen` is also the same as `render^

    // When `FLAG_RESCALE_CONTENT` is on :
    // - the size of `render` is the size of `screen` rescaled to fit inside the frameBuffer
    // - this means that `render` has the same aspect ratio as `screen`
    // - the size of `screen` remains unchanged
    // - the size of `render` is expressed in pixels
    // - the size of `screen` is expressed in opengl 2D coordinate units
    // - once rendered into the `render` viewport, a 'screen' space unit is equal to `CORE.Window.screenScale` pixels
    // - TODO explain when FLAG_WINDOW_HIGHDPI is enabled

    // Some booleans to ease code reading :

    bool requestRescaledContent = (CORE.Window.flags & FLAG_RESCALE_CONTENT); // This is a new flag that breaks backward compatibility

    bool weAreInBackwardCompatibleMode = !requestRescaledContent;

    // If we're in backward compatible mode there is no need to rescale and offset the screen to center it into the render.
    // The size of the render is always the same as the size of the frameBuffer, and the size of screen is adapted to
    // the size of the render. This means that if the size of frameBuffer changes, the size of screen changes too.

    if (weAreInBackwardCompatibleMode)
    {
        // In backward compatible mode, render always has the same size than the frameBuffer :

        CORE.Window.render.width = frameBufferWidth;
        CORE.Window.render.height = frameBufferHeight;
        
        // Depending on which OS and desktop we are, the screen might 
        // need to be rendered accordingly to the DPI scale :

#if defined(__APPLE__)
        // On MacOS the screen should not need to be rescaled according to DPI // TODO @SoloByte mission

        CORE.Window.screen.width = frameBufferWidth;
        CORE.Window.screen.height = frameBufferHeight;
        CORE.Window.screenScale = MatrixIdentity(); 
#else
        bool requestWindowHDPI      = (CORE.Window.flags & FLAG_WINDOW_HIGHDPI);
        bool weAreOnWaylandPlatform = (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND);

        // On X11 and Windows, GLFW is asked to update the size of the window and its frameBuffer accordingly 
        // to the DPI scale of the display. The consequence is that we must rescale the screen accordingly.
        // This DPI scaling is only required if the window is in windowed mode so the UI and texts are scaled
        // proportionaly to the rest of the desktop environment. This also apply to `FLAG_BORDERLESS_WINDOWED_MODE`.
        // In hardware fullscreen mode (aka `FLAG_FULLSCREEN_MODE`), however, DPI rescaling is not required because
        // the application is running in exclusive fullscreen mode which is not meant to allow interaction with the 
        // rest of the desktop environment.

        if (!inHardwareFullscreenMode && requestWindowHDPI && !weAreOnWaylandPlatform)
        {
            Vector2 scaleDPI;
            //!\ We must not use `GetWindowScaleDPI()` because it returns data relevant to the user, not to the platform.
            glfwGetWindowContentScale(platform.handle, &scaleDPI.x, &scaleDPI.y);
            
            // If the window is in windowed fullscreen mode (aka `FLAG_BORDERLESS_WINDOWED_MODE`)
            // the size of the frameBuffer is set to the size of the display.
            // This means that GLFW could not resize the window and its frameBuffer accordingly to the DPI.
            // The consequence is that we have to recompute the size of the screen accordingly
            // to the size of the frameBuffer provided by GLFW.
            CORE.Window.screen.width = (unsigned int)roundf(frameBufferWidth/scaleDPI.x);
            CORE.Window.screen.height = (unsigned int)roundf(frameBufferHeight/scaleDPI.y);

            // And now, the screen should be scaled to fit into the render :
            CORE.Window.screenScale = MatrixScale(scaleDPI.x, scaleDPI.y, 1.0);
        }
        else // On Wayland, the screen should not need to be rescaled // TODO test Wayland
        {
            CORE.Window.screen.width = frameBufferWidth;
            CORE.Window.screen.height = frameBufferHeight;
            CORE.Window.screenScale = MatrixIdentity(); 
        }
#endif

        // And no offset is required because the render occupy the whole frameBuffer :

        CORE.Window.renderOffset.x = 0;
        CORE.Window.renderOffset.y = 0;

        // We can leave now :

        return;
    }

    // We are here because `FLAG_RESCALE_CONTENT` is enabled.
    // It means we want to rescale and center the `screen` into the frameBuffer.
    // In this mode, `FLAG_WINDOW_HIGHDPI` has no effect, because the size of screen always remains unchanged.
    // Either the size of screen is bigger than the size of frameBuffer and we need to downscale the render of screen,
    // either it is smaller and we need to upscale its render.
    // TODO : we may want to consider very unusual case scenario when DPI scaling is not "square" (ie non square pixels) ?

    if ( requestRescaledContent )
    {
        TRACELOG(LOG_WARNING, "DISPLAY: FLAG_RESCALE_CONTENT is still WIP and experimental." );
    }

    // Upscaling or downscaling uses the same algorithm. Only the TRACELOG message changes :

    bool screenIsBiggerThanDisplay = (CORE.Window.screen.width > frameBufferWidth) || (CORE.Window.screen.height > frameBufferHeight);
    bool screenIsSmallerThanDisplay = !screenIsBiggerThanDisplay;
    
    if ( screenIsBiggerThanDisplay )
    {
        TRACELOG(LOG_DEBUG, "DISPLAY: Downscaling required: Screen size (%ix%i) is bigger than frameBuffer size (%ix%i)", CORE.Window.screen.width, CORE.Window.screen.height, frameBufferWidth, frameBufferHeight);
    }
    else
    if ( screenIsSmallerThanDisplay )
    {
        TRACELOG(LOG_DEBUG, "DISPLAY: Upscaling required: Screen size (%ix%i) is smaller than frameBuffer size (%ix%i)", CORE.Window.screen.width, CORE.Window.screen.height, frameBufferWidth, frameBufferHeight);
    }

    // Aspect ratio of each surface :

    float frameBufferAspectRatio = (float)frameBufferWidth/(float)frameBufferHeight;
    float screenAspectRatio = (float)CORE.Window.screen.width/(float)CORE.Window.screen.height;

    // We need to resize render proportionaly to the aspect ratio of screen, 
    // so that render fits into frameBuffer with the same aspect ratio than screen.
    // Once we know the size of render, we will compute the scale at which screen
    // will need to be rendered to fit into render :

    float screenScaleRatio = 1.0f;

    if ( screenAspectRatio > frameBufferAspectRatio )
    {
        // Example : screen is 1600x900, and frameBuffer is 800x600
        //           render should be : 800x450
        // (we compute directly using integers to avoid float conversions and roundings)

        CORE.Window.render.width = frameBufferWidth ;
        CORE.Window.render.height = CORE.Window.screen.height*frameBufferWidth/CORE.Window.screen.width;

        CORE.Window.renderOffset.x = 0;
        CORE.Window.renderOffset.y = (frameBufferHeight - CORE.Window.render.height);

        // If the screen's aspect ratio is larger than the frameBuffer's, (like 16/9 versus 4/3)
        // this means the screen will have to be rendered rescaled by the width ratio.

        screenScaleRatio = (float)frameBufferWidth/(float)CORE.Window.screen.width;
    }
    else
    {
        // Example : screen is 800x600, and frameBuffer is 1600x900
        //           render should be : 1200x900

        CORE.Window.render.width = CORE.Window.screen.width*frameBufferHeight/CORE.Window.screen.height;
        CORE.Window.render.height = frameBufferHeight;

        CORE.Window.renderOffset.x = (frameBufferWidth - CORE.Window.render.width);
        CORE.Window.renderOffset.y = 0;

        // If the screen's aspect ratio is larger than the frameBuffer's, (like 4/3 versus 16/9)
        // this means the screen will have to be rendered rescaled by the height ratio.

        screenScaleRatio = (float)frameBufferHeight/(float)CORE.Window.screen.height;
    }

    // Let's update the scale at which the drawings will occurs on the screen surface 
    // once rendered into the "render" viewport :

    CORE.Window.screenScale = MatrixScale(screenScaleRatio, screenScaleRatio, 1.0f);

    TRACELOG(LOG_DEBUG, "DISPLAY: Rescale matrix generated, content will be rendered at (%ix%i)", CORE.Window.screen.width, CORE.Window.screen.height);
}



static bool _ActivateHardwareFullscreenMode(int monitorIndex, int desiredWidth, int desiredHeight, int desiredRefreshRate)
{
    // TODO FIXME allow swtiching from one fullscreen mode to an other ?

    TRACELOG(LOG_INFO, "DISPLAY: Fullscreen mode before initialization");
    TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
    TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
    TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
    TRACELOG(LOG_INFO, "    > Screen scale: %f x %f", CORE.Window.screenScale.m0, CORE.Window.screenScale.m5);
    TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);

    GLFWmonitor *monitor = NULL;

    if (monitorIndex < 0)
    {
        monitor = glfwGetPrimaryMonitor();
    }
    else
    {
        int monitorCount = 0;
        GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

        // Use current monitor, so we correctly get the display the window is on
        monitor = (monitorIndex < monitorCount)? monitors[monitorIndex] : NULL;
    }

    if (monitor == NULL)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: GLFW failed to get requested monitor.");

        // Here we don't know if the user is already in fullscreen mode or if the function is called 
        // from `InitPlatform()` which would already have `CORE.Window.fullscreen` set to `true` when
        // calling this function. 
        // So we let the caller reset the `CORE.Window.fullscreen` and `CORE.Window.flags` as they should be
        // in their scenario.

        // There is nothing more to do. Just leave the window where it is.
        return false;
    }

    // Store previous window position and size (in case we exit fullscreen)
    glfwGetWindowPos(platform.handle, &CORE.Window.previousPosition.x, &CORE.Window.previousPosition.y);

    // GLFW 3.4 may trigger an error if platform like Wayland don't support setting or getting the position of the window.
    glfwGetError(NULL); // So we must clear this error

    //!\ We must remember the size of the frameBuffer, not the size of the "screen" :

    int frameBufferWidth , frameBufferHeight;
    glfwGetFramebufferSize(platform.handle, &frameBufferWidth, &frameBufferHeight);

    CORE.Window.previousScreen.width = frameBufferWidth;
    CORE.Window.previousScreen.height = frameBufferHeight;

    // Let's find a video mode that best matches our desired fullscreen mode :

    const GLFWvidmode *mode = NULL ; 

    if (desiredWidth > 0 && desiredHeight > 0)
    {
        int modesCount = 0 ;
        const GLFWvidmode *modes = glfwGetVideoModes(monitor, &modesCount);

        for (int i = 0; i < modesCount; i++)
        {
            if ((desiredRefreshRate == GLFW_DONT_CARE) || (desiredRefreshRate != modes[i].refreshRate))
            {
                if (modes[i].width >= desiredWidth)
                {
                    if (modes[i].height >= desiredHeight)
                    {
                        mode = &modes[i];
                        break;
                    }
                }
            }
        }
    }

    // If we failed to find an appropriate video mode, we default to the current 
    // display mode of the monitor associated to the window :

    if (mode == NULL)
    {
        mode = glfwGetVideoMode(monitor);
    }

    // Leave a tracelog of our final decision :

    if ((desiredWidth != mode->width) || (desiredHeight != mode->height) || (desiredRefreshRate != mode->refreshRate))
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Closest fullscreen videomode: %i x %i @ %i Hz", mode->width, mode->height, mode->refreshRate);
    }

    // Activate the fullscreen mode :

    glfwSetWindowMonitor(platform.handle, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

    const char *errorMessage;
    int _result = glfwGetError(&errorMessage);

    if (_result != GLFW_NO_ERROR)
    {
        TRACELOG(LOG_ERROR, "DISPLAY: GLFW failed to activate requested fullscreen mode.");

        // There is nothing more to do. Just leave the window where it is.
        return false;
    }

    // Now that the fullscreen mode is activated with success, we can update our metrics :

    CORE.Window.display.width = mode->width;
    CORE.Window.display.height = mode->height;

    _SetupFramebuffer(mode->width, mode->height, true); // 3rd ags is `true` because we're in an hardware fullscreen mode

    // Update fullscreen indicators :

    CORE.Window.fullscreen = true;
    CORE.Window.flags |= FLAG_FULLSCREEN_MODE;

    // And we're done !

    TRACELOG(LOG_INFO, "DISPLAY: Fullscreen mode initialized successfully");
    TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
    TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
    TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
    TRACELOG(LOG_INFO, "    > Screen scale: %f x %f", CORE.Window.screenScale.m0, CORE.Window.screenScale.m5);
    TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);

    return true;
}


static void _DeactivateHardwareFullscreenMode()
{
    TRACELOG(LOG_INFO, "DISPLAY: Fullscreen mode before deinitialization");
    TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
    TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
    TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
    TRACELOG(LOG_INFO, "    > Screen scale: %f x %f", CORE.Window.screenScale.m0, CORE.Window.screenScale.m5);
    TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);

    // Ask GLFW to restore the window and the previous monitor resolution :

    glfwSetWindowMonitor(platform.handle, NULL, CORE.Window.previousPosition.x, CORE.Window.previousPosition.y, CORE.Window.previousScreen.width, CORE.Window.previousScreen.height, GLFW_DONT_CARE);

    // As we're leaving, we update the flags :

    CORE.Window.fullscreen = false;
    CORE.Window.flags &= ~FLAG_FULLSCREEN_MODE;

    // And we must restore some metrics :

    int monitorIndex = GetCurrentMonitor();

    CORE.Window.display.width = GetMonitorWidth(monitorIndex);
    CORE.Window.display.height = GetMonitorHeight(monitorIndex);

    int prevWidth  = CORE.Window.previousScreen.width;
    int prevHeight = CORE.Window.previousScreen.height;

    _SetupFramebuffer(prevWidth, prevHeight, false); 

    // Then we're done !

    TRACELOG(LOG_INFO, "DISPLAY: Fullscreen mode deinitialized successfully");
    TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
    TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
    TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
    TRACELOG(LOG_INFO, "    > Screen scale: %f x %f", CORE.Window.screenScale.m0, CORE.Window.screenScale.m5);
    TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);
}

// GLFW3 Callback, called when the DPI of the monitor is updated
static void WindowContentScaleCallback(GLFWwindow *window, float scalex, float scaley)
{
    //!\ Nothing to do here.
    //!\ NOTE : The actual update of `CORE.Window.screenScale` will happen when `WindowSizeCallback()` is called.
    //!\ We leave this callback active for debug and informative purpose.
    
    TRACELOG(LOG_INFO, "SYSTEM: the DPI scaling of the monitor was udpated to %f x %f", scalex, scaley );
}

// GLFW3 WindowIconify Callback, runs when window is minimized/restored
static void WindowIconifyCallback(GLFWwindow *window, int iconified)
{
    if (iconified) CORE.Window.flags |= FLAG_WINDOW_MINIMIZED;  // The window was iconified
    else CORE.Window.flags &= ~FLAG_WINDOW_MINIMIZED;           // The window was restored
}

// GLFW3 WindowMaximize Callback, runs when window is maximized/restored
static void WindowMaximizeCallback(GLFWwindow *window, int maximized)
{
    if (maximized) CORE.Window.flags |= FLAG_WINDOW_MAXIMIZED;  // The window was maximized
    else CORE.Window.flags &= ~FLAG_WINDOW_MAXIMIZED;           // The window was restored
}

// GLFW3 WindowFocus Callback, runs when window get/lose focus
static void WindowFocusCallback(GLFWwindow *window, int focused)
{
    if (focused) CORE.Window.flags &= ~FLAG_WINDOW_UNFOCUSED;   // The window was focused
    else CORE.Window.flags |= FLAG_WINDOW_UNFOCUSED;            // The window lost focus
}

// GLFW3 Window Drop Callback, runs when drop files into window
static void WindowDropCallback(GLFWwindow *window, int count, const char **paths)
{
    if (count > 0)
    {
        // In case previous dropped filepaths have not been freed, we free them
        if (CORE.Window.dropFileCount > 0)
        {
            for (unsigned int i = 0; i < CORE.Window.dropFileCount; i++) RL_FREE(CORE.Window.dropFilepaths[i]);

            RL_FREE(CORE.Window.dropFilepaths);

            CORE.Window.dropFileCount = 0;
            CORE.Window.dropFilepaths = NULL;
        }

        // WARNING: Paths are freed by GLFW when the callback returns, we must keep an internal copy
        CORE.Window.dropFileCount = count;
        CORE.Window.dropFilepaths = (char **)RL_CALLOC(CORE.Window.dropFileCount, sizeof(char *));

        for (unsigned int i = 0; i < CORE.Window.dropFileCount; i++)
        {
            CORE.Window.dropFilepaths[i] = (char *)RL_CALLOC(MAX_FILEPATH_LENGTH, sizeof(char));
            strcpy(CORE.Window.dropFilepaths[i], paths[i]);
        }
    }
}

// GLFW3 Keyboard Callback, runs on key pressed
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key < 0) return;    // Security check, macOS fn key generates -1

    // WARNING: GLFW could return GLFW_REPEAT, we need to consider it as 1
    // to work properly with our implementation (IsKeyDown/IsKeyUp checks)
    if (action == GLFW_RELEASE) CORE.Input.Keyboard.currentKeyState[key] = 0;
    else if(action == GLFW_PRESS) CORE.Input.Keyboard.currentKeyState[key] = 1;
    else if(action == GLFW_REPEAT) CORE.Input.Keyboard.keyRepeatInFrame[key] = 1;

    // WARNING: Check if CAPS/NUM key modifiers are enabled and force down state for those keys
    if (((key == KEY_CAPS_LOCK) && ((mods & GLFW_MOD_CAPS_LOCK) > 0)) ||
        ((key == KEY_NUM_LOCK) && ((mods & GLFW_MOD_NUM_LOCK) > 0))) CORE.Input.Keyboard.currentKeyState[key] = 1;

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

// GLFW3 Char Callback, get unicode codepoint value
static void CharCallback(GLFWwindow *window, unsigned int codepoint)
{
    //TRACELOG(LOG_DEBUG, "Char Callback: Codepoint: %i", codepoint);

    // NOTE: Registers any key down considering OS keyboard layout but
    // does not detect action events, those should be managed by user...
    // Ref: https://github.com/glfw/glfw/issues/668#issuecomment-166794907
    // Ref: https://www.glfw.org/docs/latest/input_guide.html#input_char

    // Check if there is space available in the queue
    if (CORE.Input.Keyboard.charPressedQueueCount < MAX_CHAR_PRESSED_QUEUE)
    {
        // Add character to the queue
        CORE.Input.Keyboard.charPressedQueue[CORE.Input.Keyboard.charPressedQueueCount] = codepoint;
        CORE.Input.Keyboard.charPressedQueueCount++;
    }
}

// GLFW3 Mouse Button Callback, runs on mouse button pressed
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

// GLFW3 Cursor Position Callback, runs on mouse move
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

// GLFW3 Scrolling Callback, runs on mouse wheel
static void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    CORE.Input.Mouse.currentWheelMove = (Vector2){ (float)xoffset, (float)yoffset };
}

// GLFW3 CursorEnter Callback, when cursor enters the window
static void CursorEnterCallback(GLFWwindow *window, int enter)
{
    if (enter) CORE.Input.Mouse.cursorOnScreen = true;
    else CORE.Input.Mouse.cursorOnScreen = false;
}

// GLFW3 Joystick Connected/Disconnected Callback
static void JoystickCallback(int jid, int event)
{
    if (event == GLFW_CONNECTED)
    {
        strcpy(CORE.Input.Gamepad.name[jid], glfwGetJoystickName(jid));
    }
    else if (event == GLFW_DISCONNECTED)
    {
        memset(CORE.Input.Gamepad.name[jid], 0, 64);
    }
}

// EOF
