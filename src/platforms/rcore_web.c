/**********************************************************************************************
*
*   rcore_web - Functions to manage window, graphics device and inputs
*
*   PLATFORM: WEB
*       - HTML5 (WebAssembly)
*
*   LIMITATIONS:
*       - Limitation 01
*       - Limitation 02
*
*   POSSIBLE IMPROVEMENTS:
*       - Replace glfw3 dependency by direct browser API calls (same as library_glfw3.js)
*
*   ADDITIONAL NOTES:
*       - TRACELOG() function is located in raylib [utils] module
*
*   CONFIGURATION:
*       #define RCORE_PLATFORM_CUSTOM_FLAG
*           Custom flag for rcore on target platform -not used-
*
*   DEPENDENCIES:
*       - emscripten: Allow interaction between browser API and C
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
#include "GLFW/glfw3.h"         // GLFW3: Windows, OpenGL context and Input management

#include <emscripten/emscripten.h>      // Emscripten functionality for C
#include <emscripten/html5.h>           // Emscripten HTML5 library

#include <sys/time.h>   // Required for: timespec, nanosleep(), select() - POSIX

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// TODO: HACK: Added flag if not provided by GLFW when using external library
// Latest GLFW release (GLFW 3.3.8) does not implement this flag, it was added for 3.4.0-dev
#if !defined(GLFW_MOUSE_PASSTHROUGH)
    #define GLFW_MOUSE_PASSTHROUGH      0x0002000D
#endif

#if (_POSIX_C_SOURCE < 199309L)
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 199309L     // Required for: CLOCK_MONOTONIC if compiled with c99 without gnu ext.
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    GLFWwindow *handle;                 // GLFW window handle (graphic device)
    bool ourFullscreen;                 // Internal var to filter our handling of fullscreen vs the user handling of fullscreen
} PlatformData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern CoreData CORE;                   // Global CORE state context

static PlatformData platform = { 0 };   // Platform specific data

//----------------------------------------------------------------------------------
// Local Variables Definition
//----------------------------------------------------------------------------------
static const char cursorLUT[11][12] = {
    "default",     // 0  MOUSE_CURSOR_DEFAULT
    "default",     // 1  MOUSE_CURSOR_ARROW
    "text",        // 2  MOUSE_CURSOR_IBEAM
    "crosshair",   // 3  MOUSE_CURSOR_CROSSHAIR
    "pointer",     // 4  MOUSE_CURSOR_POINTING_HAND
    "ew-resize",   // 5  MOUSE_CURSOR_RESIZE_EW
    "ns-resize",   // 6  MOUSE_CURSOR_RESIZE_NS
    "nwse-resize", // 7  MOUSE_CURSOR_RESIZE_NWSE
    "nesw-resize", // 8  MOUSE_CURSOR_RESIZE_NESW
    "move",        // 9  MOUSE_CURSOR_RESIZE_ALL
    "not-allowed"  // 10 MOUSE_CURSOR_NOT_ALLOWED
};

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
int InitPlatform(void);          // Initialize platform (graphics, inputs and more)
void ClosePlatform(void);        // Close platform

// Error callback event
static void ErrorCallback(int error, const char *description);                      // GLFW3 Error Callback, runs on GLFW3 error

// Window callbacks events
static void WindowSizeCallback(GLFWwindow *window, int width, int height);              // GLFW3 WindowSize Callback, runs when window is resized
static void WindowIconifyCallback(GLFWwindow *window, int iconified);                   // GLFW3 WindowIconify Callback, runs when window is minimized/restored
//static void WindowMaximizeCallback(GLFWwindow *window, int maximized);                // GLFW3 Window Maximize Callback, runs when window is maximized
static void WindowFocusCallback(GLFWwindow *window, int focused);                       // GLFW3 WindowFocus Callback, runs when window get/lose focus
static void WindowDropCallback(GLFWwindow *window, int count, const char **paths);      // GLFW3 Window Drop Callback, runs when drop files into window
static void WindowContentScaleCallback(GLFWwindow *window, float scalex, float scaley); // GLFW3 Window Content Scale Callback, runs when window changes scale

// Input callbacks events
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods); // GLFW3 Keyboard Callback, runs on key pressed
static void CharCallback(GLFWwindow *window, unsigned int key);                           // GLFW3 Char Key Callback, runs on key pressed (get char value)
static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);    // GLFW3 Mouse Button Callback, runs on mouse button pressed
static void MouseCursorPosCallback(GLFWwindow *window, double x, double y);               // GLFW3 Cursor Position Callback, runs on mouse move
static void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);      // GLFW3 Srolling Callback, runs on mouse wheel
static void CursorEnterCallback(GLFWwindow *window, int enter);                           // GLFW3 Cursor Enter Callback, cursor enters client area

// Emscripten window callback events
static EM_BOOL EmscriptenFullscreenChangeCallback(int eventType, const EmscriptenFullscreenChangeEvent *event, void *userData);
static EM_BOOL EmscriptenWindowResizedCallback(int eventType, const EmscriptenUiEvent *event, void *userData);
static EM_BOOL EmscriptenResizeCallback(int eventType, const EmscriptenUiEvent *event, void *userData);

// Emscripten input callback events
static EM_BOOL EmscriptenMouseCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
static EM_BOOL EmscriptenPointerlockCallback(int eventType, const EmscriptenPointerlockChangeEvent *pointerlockChangeEvent, void *userData);
static EM_BOOL EmscriptenTouchCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
static EM_BOOL EmscriptenGamepadCallback(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData);

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
// NOTE: Functions declaration is provided by raylib.h

//----------------------------------------------------------------------------------
// Module Functions Definition: Window and Graphics Device
//----------------------------------------------------------------------------------

// Check if application should close
bool WindowShouldClose(void)
{
    // Emterpreter-Async required to run sync code
    // https://github.com/emscripten-core/emscripten/wiki/Emterpreter#emterpreter-async-run-synchronous-code
    // By default, this function is never called on a web-ready raylib example because we encapsulate
    // frame code in a UpdateDrawFrame() function, to allow browser manage execution asynchronously
    // but now emscripten allows sync code to be executed in an interpreted way, using emterpreter!
    emscripten_sleep(16);
    return false;
}

// Toggle fullscreen mode
void ToggleFullscreen(void)
{
    platform.ourFullscreen = true;
    bool enterFullscreen = false;

    const bool wasFullscreen = EM_ASM_INT( { if (document.fullscreenElement) return 1; }, 0);
    if (wasFullscreen)
    {
        if (CORE.Window.flags & FLAG_FULLSCREEN_MODE) enterFullscreen = false;
        else if (CORE.Window.flags & FLAG_BORDERLESS_WINDOWED_MODE) enterFullscreen = true;
        else
        {
            const int canvasWidth = EM_ASM_INT( { return document.getElementById('canvas').width; }, 0);
            const int canvasStyleWidth = EM_ASM_INT( { return parseInt(document.getElementById('canvas').style.width); }, 0);
            if (canvasStyleWidth > canvasWidth) enterFullscreen = false;
            else enterFullscreen = true;
        }

        EM_ASM(document.exitFullscreen(););

        CORE.Window.fullscreen = false;
        CORE.Window.flags &= ~FLAG_FULLSCREEN_MODE;
        CORE.Window.flags &= ~FLAG_BORDERLESS_WINDOWED_MODE;
    }
    else enterFullscreen = true;

    if (enterFullscreen)
    {
        // NOTE: The setTimeouts handle the browser mode change delay
        EM_ASM(
            setTimeout(function()
            {
                Module.requestFullscreen(false, false);
            }, 100);
        );
        CORE.Window.fullscreen = true;
        CORE.Window.flags |= FLAG_FULLSCREEN_MODE;
    }

    // NOTE: Old notes below:
    /*
        EM_ASM
        (
            // This strategy works well while using raylib minimal web shell for emscripten,
            // it re-scales the canvas to fullscreen using monitor resolution, for tools this
            // is a good strategy but maybe games prefer to keep current canvas resolution and
            // display it in fullscreen, adjusting monitor resolution if possible
            if (document.fullscreenElement) document.exitFullscreen();
            else Module.requestFullscreen(true, true); //false, true);
        );
    */
    // EM_ASM(Module.requestFullscreen(false, false););
    /*
        if (!CORE.Window.fullscreen)
        {
            // Option 1: Request fullscreen for the canvas element
            // This option does not seem to work at all:
            // emscripten_request_pointerlock() and emscripten_request_fullscreen() are affected by web security,
            // the user must click once on the canvas to hide the pointer or transition to full screen
            //emscripten_request_fullscreen("#canvas", false);

            // Option 2: Request fullscreen for the canvas element with strategy
            // This option does not seem to work at all
            // Ref: https://github.com/emscripten-core/emscripten/issues/5124
            // EmscriptenFullscreenStrategy strategy = {
                // .scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH, //EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT,
                // .canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF,
                // .filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT,
                // .canvasResizedCallback = EmscriptenWindowResizedCallback,
                // .canvasResizedCallbackUserData = NULL
            // };
            //emscripten_request_fullscreen_strategy("#canvas", EM_FALSE, &strategy);

            // Option 3: Request fullscreen for the canvas element with strategy
            // It works as expected but only inside the browser (client area)
            EmscriptenFullscreenStrategy strategy = {
                .scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT,
                .canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF,
                .filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT,
                .canvasResizedCallback = EmscriptenWindowResizedCallback,
                .canvasResizedCallbackUserData = NULL
            };
            emscripten_enter_soft_fullscreen("#canvas", &strategy);

            int width, height;
            emscripten_get_canvas_element_size("#canvas", &width, &height);
            TRACELOG(LOG_WARNING, "Emscripten: Enter fullscreen: Canvas size: %i x %i", width, height);

            CORE.Window.fullscreen = true;          // Toggle fullscreen flag
            CORE.Window.flags |= FLAG_FULLSCREEN_MODE;
        }
        else
        {
            //emscripten_exit_fullscreen();
            //emscripten_exit_soft_fullscreen();

            int width, height;
            emscripten_get_canvas_element_size("#canvas", &width, &height);
            TRACELOG(LOG_WARNING, "Emscripten: Exit fullscreen: Canvas size: %i x %i", width, height);

            CORE.Window.fullscreen = false;          // Toggle fullscreen flag
            CORE.Window.flags &= ~FLAG_FULLSCREEN_MODE;
        }
    */
}

// Toggle borderless windowed mode
void ToggleBorderlessWindowed(void)
{
    platform.ourFullscreen = true;
    bool enterBorderless = false;

    const bool wasFullscreen = EM_ASM_INT( { if (document.fullscreenElement) return 1; }, 0);
    if (wasFullscreen)
    {
        if (CORE.Window.flags & FLAG_BORDERLESS_WINDOWED_MODE) enterBorderless = false;
        else if (CORE.Window.flags & FLAG_FULLSCREEN_MODE) enterBorderless = true;
        else
        {
            const int canvasWidth = EM_ASM_INT( { return document.getElementById('canvas').width; }, 0);
            const int screenWidth = EM_ASM_INT( { return screen.width; }, 0);
            if (screenWidth == canvasWidth) enterBorderless = false;
            else enterBorderless = true;
        }

        EM_ASM(document.exitFullscreen(););

        CORE.Window.fullscreen = false;
        CORE.Window.flags &= ~FLAG_FULLSCREEN_MODE;
        CORE.Window.flags &= ~FLAG_BORDERLESS_WINDOWED_MODE;
    }
    else enterBorderless = true;

    if (enterBorderless)
    {
        // NOTE: 1. The setTimeouts handle the browser mode change delay
        //       2. The style unset handles the possibility of a width="value%" like on the default shell.html file
        EM_ASM(
            setTimeout(function()
            {
                Module.requestFullscreen(false, true);
                setTimeout(function()
                {
                    canvas.style.width="unset";
                }, 100);
            }, 100);
        );
        CORE.Window.flags |= FLAG_BORDERLESS_WINDOWED_MODE;
    }
}

// Set window state: maximized, if resizable
void MaximizeWindow(void)
{
    TRACELOG(LOG_WARNING, "MaximizeWindow() not available on target platform");
}

// Set window state: minimized
void MinimizeWindow(void)
{
    TRACELOG(LOG_WARNING, "MinimizeWindow() not available on target platform");
}

// Set window state: not minimized/maximized
void RestoreWindow(void)
{
    TRACELOG(LOG_WARNING, "RestoreWindow() not available on target platform");
}

// Set window configuration state using flags
void SetWindowState(unsigned int flags)
{
    // Check previous state and requested state to apply required changes
    // NOTE: In most cases the functions already change the flags internally

    // State change: FLAG_VSYNC_HINT
    if ((flags & FLAG_VSYNC_HINT) > 0)
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_VSYNC_HINT) not available on target platform");
    }

    // State change: FLAG_BORDERLESS_WINDOWED_MODE
    if ((flags & FLAG_BORDERLESS_WINDOWED_MODE) > 0)
    {
        // NOTE: Window state flag updated inside ToggleBorderlessWindowed() function
        const bool wasFullscreen = EM_ASM_INT( { if (document.fullscreenElement) return 1; }, 0);
        if (wasFullscreen)
        {
            const int canvasWidth = EM_ASM_INT( { return document.getElementById('canvas').width; }, 0);
            const int canvasStyleWidth = EM_ASM_INT( { return parseInt(document.getElementById('canvas').style.width); }, 0);
            if ((CORE.Window.flags & FLAG_FULLSCREEN_MODE) || canvasStyleWidth > canvasWidth) ToggleBorderlessWindowed();
        }
        else ToggleBorderlessWindowed();
    }

    // State change: FLAG_FULLSCREEN_MODE
    if ((flags & FLAG_FULLSCREEN_MODE) > 0)
    {
        // NOTE: Window state flag updated inside ToggleFullscreen() function
        const bool wasFullscreen = EM_ASM_INT( { if (document.fullscreenElement) return 1; }, 0);
        if (wasFullscreen)
        {
            const int canvasWidth = EM_ASM_INT( { return document.getElementById('canvas').width; }, 0);
            const int screenWidth = EM_ASM_INT( { return screen.width; }, 0);
            if ((CORE.Window.flags & FLAG_BORDERLESS_WINDOWED_MODE) || screenWidth == canvasWidth ) ToggleFullscreen();
        }
        else ToggleFullscreen();
    }

    // State change: FLAG_WINDOW_RESIZABLE
    if (((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) != (flags & FLAG_WINDOW_RESIZABLE)) && ((flags & FLAG_WINDOW_RESIZABLE) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_RESIZABLE, GLFW_TRUE);
        CORE.Window.flags |= FLAG_WINDOW_RESIZABLE;
    }

    // State change: FLAG_WINDOW_UNDECORATED
    if ((flags & FLAG_WINDOW_UNDECORATED) > 0)
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_UNDECORATED) not available on target platform");
    }

    // State change: FLAG_WINDOW_HIDDEN
    if ((flags & FLAG_WINDOW_HIDDEN) > 0)
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_HIDDEN) not available on target platform");
    }

    // State change: FLAG_WINDOW_MINIMIZED
    if ((flags & FLAG_WINDOW_MINIMIZED) > 0)
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_MINIMIZED) not available on target platform");
    }

    // State change: FLAG_WINDOW_MAXIMIZED
    if ((flags & FLAG_WINDOW_MAXIMIZED) > 0)
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_MAXIMIZED) not available on target platform");
    }

    // State change: FLAG_WINDOW_UNFOCUSED
    if ((flags & FLAG_WINDOW_UNFOCUSED) > 0)
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_UNFOCUSED) not available on target platform");
    }

    // State change: FLAG_WINDOW_TOPMOST
    if ((flags & FLAG_WINDOW_TOPMOST) > 0)
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_TOPMOST) not available on target platform");
    }

    // State change: FLAG_WINDOW_ALWAYS_RUN
    if ((flags & FLAG_WINDOW_ALWAYS_RUN) > 0)
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_ALWAYS_RUN) not available on target platform");
    }

    // The following states can not be changed after window creation
    // NOTE: Review for PLATFORM_WEB

    // State change: FLAG_WINDOW_TRANSPARENT
    if ((flags & FLAG_WINDOW_TRANSPARENT) > 0)
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_TRANSPARENT) not available on target platform");
    }

    // State change: FLAG_WINDOW_HIGHDPI
    if ((flags & FLAG_WINDOW_HIGHDPI) > 0)
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_HIGHDPI) not available on target platform");
    }

    // State change: FLAG_WINDOW_MOUSE_PASSTHROUGH
    if ((flags & FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0)
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_MOUSE_PASSTHROUGH) not available on target platform");
    }

    // State change: FLAG_MSAA_4X_HINT
    if ((flags & FLAG_MSAA_4X_HINT) > 0)
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_MSAA_4X_HINT) not available on target platform");
    }

    // State change: FLAG_INTERLACED_HINT
    if ((flags & FLAG_INTERLACED_HINT) > 0)
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_INTERLACED_HINT) not available on target platform");
    }
}

// Clear window configuration state flags
void ClearWindowState(unsigned int flags)
{
    // Check previous state and requested state to apply required changes
    // NOTE: In most cases the functions already change the flags internally

    // State change: FLAG_VSYNC_HINT
    if ((flags & FLAG_VSYNC_HINT) > 0)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_VSYNC_HINT) not available on target platform");
    }

    // State change: FLAG_BORDERLESS_WINDOWED_MODE
    if ((flags & FLAG_BORDERLESS_WINDOWED_MODE) > 0)
    {
        const bool wasFullscreen = EM_ASM_INT( { if (document.fullscreenElement) return 1; }, 0);
        if (wasFullscreen)
        {
            const int canvasWidth = EM_ASM_INT( { return document.getElementById('canvas').width; }, 0);
            const int screenWidth = EM_ASM_INT( { return screen.width; }, 0);
            if ((CORE.Window.flags & FLAG_BORDERLESS_WINDOWED_MODE) || (screenWidth == canvasWidth)) EM_ASM(document.exitFullscreen(););
        }

        CORE.Window.flags &= ~FLAG_BORDERLESS_WINDOWED_MODE;
    }

    // State change: FLAG_FULLSCREEN_MODE
    if ((flags & FLAG_FULLSCREEN_MODE) > 0)
    {
        const bool wasFullscreen = EM_ASM_INT( { if (document.fullscreenElement) return 1; }, 0);
        if (wasFullscreen)
        {
            const int canvasWidth = EM_ASM_INT( { return document.getElementById('canvas').width; }, 0);
            const int canvasStyleWidth = EM_ASM_INT( { return parseInt(document.getElementById('canvas').style.width); }, 0);
            if ((CORE.Window.flags & FLAG_FULLSCREEN_MODE) || (canvasStyleWidth > canvasWidth)) EM_ASM(document.exitFullscreen(););
        }

        CORE.Window.fullscreen = false;
        CORE.Window.flags &= ~FLAG_FULLSCREEN_MODE;
    }

    // State change: FLAG_WINDOW_RESIZABLE
    if (((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) > 0) && ((flags & FLAG_WINDOW_RESIZABLE) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_RESIZABLE, GLFW_FALSE);
        CORE.Window.flags &= ~FLAG_WINDOW_RESIZABLE;
    }

    // State change: FLAG_WINDOW_HIDDEN
    if ((flags & FLAG_WINDOW_HIDDEN) > 0)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_HIDDEN) not available on target platform");
    }

    // State change: FLAG_WINDOW_MINIMIZED
    if ((flags & FLAG_WINDOW_MINIMIZED) > 0)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_MINIMIZED) not available on target platform");
    }

    // State change: FLAG_WINDOW_MAXIMIZED
    if ((flags & FLAG_WINDOW_MAXIMIZED) > 0)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_MAXIMIZED) not available on target platform");
    }

    // State change: FLAG_WINDOW_UNDECORATED
    if ((flags & FLAG_WINDOW_UNDECORATED) > 0)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_UNDECORATED) not available on target platform");
    }

    // State change: FLAG_WINDOW_UNFOCUSED
    if ((flags & FLAG_WINDOW_UNFOCUSED) > 0)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_UNFOCUSED) not available on target platform");
    }

    // State change: FLAG_WINDOW_TOPMOST
    if ((flags & FLAG_WINDOW_TOPMOST) > 0)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_TOPMOST) not available on target platform");
    }

    // State change: FLAG_WINDOW_ALWAYS_RUN
    if ((flags & FLAG_WINDOW_ALWAYS_RUN) > 0)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_ALWAYS_RUN) not available on target platform");
    }

    // The following states can not be changed after window creation
    // NOTE: Review for PLATFORM_WEB

    // State change: FLAG_WINDOW_TRANSPARENT
    if ((flags & FLAG_WINDOW_TRANSPARENT) > 0)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_TRANSPARENT) not available on target platform");
    }

    // State change: FLAG_WINDOW_HIGHDPI
    if ((flags & FLAG_WINDOW_HIGHDPI) > 0)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_HIGHDPI) not available on target platform");
    }

    // State change: FLAG_WINDOW_MOUSE_PASSTHROUGH
    if ((flags & FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_MOUSE_PASSTHROUGH) not available on target platform");
    }

    // State change: FLAG_MSAA_4X_HINT
    if ((flags & FLAG_MSAA_4X_HINT) > 0)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_MSAA_4X_HINT) not available on target platform");
    }

    // State change: FLAG_INTERLACED_HINT
    if ((flags & FLAG_INTERLACED_HINT) > 0)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_INTERLACED_HINT) not available on target platform");
    }
}

// Set icon for window
void SetWindowIcon(Image image)
{
    TRACELOG(LOG_WARNING, "SetWindowIcon() not available on target platform");
}

// Set icon for window, multiple images
void SetWindowIcons(Image *images, int count)
{
    TRACELOG(LOG_WARNING, "SetWindowIcons() not available on target platform");
}

// Set title for window
void SetWindowTitle(const char *title)
{
    CORE.Window.title = title;
    emscripten_set_window_title(title);
}

// Set window position on screen (windowed mode)
void SetWindowPosition(int x, int y)
{
    TRACELOG(LOG_WARNING, "SetWindowPosition() not available on target platform");
}

// Set monitor for the current window
void SetWindowMonitor(int monitor)
{
    TRACELOG(LOG_WARNING, "SetWindowMonitor() not available on target platform");
}

// Set window minimum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMinSize(int width, int height)
{
    CORE.Window.screenMin.width = width;
    CORE.Window.screenMin.height = height;

    // Trigger the resize event once to update the window minimum width and height
    if ((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) != 0) EmscriptenResizeCallback(EMSCRIPTEN_EVENT_RESIZE, NULL, NULL);
}

// Set window maximum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMaxSize(int width, int height)
{
    CORE.Window.screenMax.width = width;
    CORE.Window.screenMax.height = height;

    // Trigger the resize event once to update the window maximum width and height
    if ((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) != 0) EmscriptenResizeCallback(EMSCRIPTEN_EVENT_RESIZE, NULL, NULL);
}

// Set window dimensions
void SetWindowSize(int width, int height)
{
    glfwSetWindowSize(platform.handle, width, height);
}

// Set window opacity, value opacity is between 0.0 and 1.0
void SetWindowOpacity(float opacity)
{
    TRACELOG(LOG_WARNING, "SetWindowOpacity() not available on target platform");
}

// Set window focused
void SetWindowFocused(void)
{
    TRACELOG(LOG_WARNING, "SetWindowFocused() not available on target platform");
}

// Get native window handle
void *GetWindowHandle(void)
{
    TRACELOG(LOG_WARNING, "GetWindowHandle() not implemented on target platform");
    return NULL;
}

// Get number of monitors
int GetMonitorCount(void)
{
    TRACELOG(LOG_WARNING, "GetMonitorCount() not implemented on target platform");
    return 1;
}

// Get number of monitors
int GetCurrentMonitor(void)
{
    TRACELOG(LOG_WARNING, "GetCurrentMonitor() not implemented on target platform");
    return 0;
}

// Get selected monitor position
Vector2 GetMonitorPosition(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorPosition() not implemented on target platform");
    return (Vector2){ 0, 0 };
}

// Get selected monitor width (currently used by monitor)
int GetMonitorWidth(int monitor)
{
    // NOTE: Returned value is limited to the current monitor where the browser window is located
    int width = 0;
    width = EM_ASM_INT( { return screen.width; }, 0);
    return width;
}

// Get selected monitor height (currently used by monitor)
int GetMonitorHeight(int monitor)
{
    // NOTE: Returned value is limited to the current monitor where the browser window is located
    int height = 0;
    height = EM_ASM_INT( { return screen.height; }, 0);
    return height;
}

// Get selected monitor physical width in millimetres
int GetMonitorPhysicalWidth(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorPhysicalWidth() not implemented on target platform");
    return 0;
}

// Get selected monitor physical height in millimetres
int GetMonitorPhysicalHeight(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorPhysicalHeight() not implemented on target platform");
    return 0;
}

// Get selected monitor refresh rate
int GetMonitorRefreshRate(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorRefreshRate() not implemented on target platform");
    return 0;
}

// Get the human-readable, UTF-8 encoded name of the selected monitor
const char *GetMonitorName(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorName() not implemented on target platform");
    return "";
}

// Get window position XY on monitor
Vector2 GetWindowPosition(void)
{
    // NOTE: Returned position is relative to the current monitor where the browser window is located
    Vector2 position = { 0, 0 };
    position.x = (float)EM_ASM_INT( { return window.screenX; }, 0);
    position.y = (float)EM_ASM_INT( { return window.screenY; }, 0);
    return position;
}

// Get window scale DPI factor for current monitor
Vector2 GetWindowScaleDPI(void)
{
    TRACELOG(LOG_WARNING, "GetWindowScaleDPI() not implemented on target platform");
    return (Vector2){ 1.0f, 1.0f };
}

// Set clipboard text content
void SetClipboardText(const char *text)
{
    // Security check to (partially) avoid malicious code
    if (strchr(text, '\'') != NULL) TRACELOG(LOG_WARNING, "SYSTEM: Provided Clipboard could be potentially malicious, avoid [\'] character");
    else EM_ASM({ navigator.clipboard.writeText(UTF8ToString($0)); }, text);
}

// Get clipboard text content
// NOTE: returned string is allocated and freed by GLFW
const char *GetClipboardText(void)
{
/*
    // Accessing clipboard data from browser is tricky due to security reasons
    // The method to use is navigator.clipboard.readText() but this is an asynchronous method
    // that will return at some moment after the function is called with the required data
    emscripten_run_script_string("navigator.clipboard.readText() \
        .then(text => { document.getElementById('clipboard').innerText = text; console.log('Pasted content: ', text); }) \
        .catch(err => { console.error('Failed to read clipboard contents: ', err); });"
    );

    // The main issue is getting that data, one approach could be using ASYNCIFY and wait
    // for the data but it requires adding Asyncify emscripten library on compilation

    // Another approach could be just copy the data in a HTML text field and try to retrieve it
    // later on if available... and clean it for future accesses
*/
    return NULL;
}

// Show mouse cursor
void ShowCursor(void)
{
    if (CORE.Input.Mouse.cursorHidden)
    {
        EM_ASM( { document.getElementById("canvas").style.cursor = UTF8ToString($0); }, cursorLUT[CORE.Input.Mouse.cursor]);

        CORE.Input.Mouse.cursorHidden = false;
    }
}

// Hides mouse cursor
void HideCursor(void)
{
    if (!CORE.Input.Mouse.cursorHidden)
    {
        EM_ASM(document.getElementById('canvas').style.cursor = 'none';);

        CORE.Input.Mouse.cursorHidden = true;
    }
}

// Enables cursor (unlock cursor)
void EnableCursor(void)
{
    emscripten_exit_pointerlock();

    // Set cursor position in the middle
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);

    // NOTE: CORE.Input.Mouse.cursorHidden handled by EmscriptenPointerlockCallback()
}

// Disables cursor (lock cursor)
void DisableCursor(void)
{
    // TODO: figure out how not to hard code the canvas ID here.
    emscripten_request_pointerlock("#canvas", 1);

    // Set cursor position in the middle
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);

    // NOTE: CORE.Input.Mouse.cursorHidden handled by EmscriptenPointerlockCallback()
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
    // Security check to (partially) avoid malicious code on target platform
    if (strchr(url, '\'') != NULL) TRACELOG(LOG_WARNING, "SYSTEM: Provided URL could be potentially malicious, avoid [\'] character");
    else emscripten_run_script(TextFormat("window.open('%s', '_blank')", url));
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Inputs
//----------------------------------------------------------------------------------

// Set internal gamepad mappings
int SetGamepadMappings(const char *mappings)
{
    TRACELOG(LOG_INFO, "SetGamepadMappings not implemented in rcore_web.c");

    return 0;
}

// Set gamepad vibration
void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor)
{
    TRACELOG(LOG_WARNING, "GamepadSetVibration() not implemented on target platform");
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
    if (CORE.Input.Mouse.cursor != cursor)
    {
        if (!CORE.Input.Mouse.cursorHidden) EM_ASM( { document.getElementById('canvas').style.cursor = UTF8ToString($0); }, cursorLUT[cursor]);

        CORE.Input.Mouse.cursor = cursor;
    }
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
    // TODO: It resets on target platform the mouse position and not filled again until a move-event,
    // so, if mouse is not moved it returns a (0, 0) position... this behaviour should be reviewed!
    //for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.position[i] = (Vector2){ 0, 0 };


    // Gamepad support using emscripten API
    // NOTE: GLFW3 joystick functionality not available in web

    // Get number of gamepads connected
    int numGamepads = 0;
    if (emscripten_sample_gamepad_data() == EMSCRIPTEN_RESULT_SUCCESS) numGamepads = emscripten_get_num_gamepads();

    for (int i = 0; (i < numGamepads) && (i < MAX_GAMEPADS); i++)
    {
        // Register previous gamepad button states
        for (int k = 0; k < MAX_GAMEPAD_BUTTONS; k++) CORE.Input.Gamepad.previousButtonState[i][k] = CORE.Input.Gamepad.currentButtonState[i][k];

        EmscriptenGamepadEvent gamepadState;

        int result = emscripten_get_gamepad_status(i, &gamepadState);

        if (result == EMSCRIPTEN_RESULT_SUCCESS)
        {
            // Register buttons data for every connected gamepad
            for (int j = 0; (j < gamepadState.numButtons) && (j < MAX_GAMEPAD_BUTTONS); j++)
            {
                GamepadButton button = -1;

                // Gamepad Buttons reference: https://www.w3.org/TR/gamepad/#gamepad-interface
                switch (j)
                {
                    case 0: button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN; break;
                    case 1: button = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT; break;
                    case 2: button = GAMEPAD_BUTTON_RIGHT_FACE_LEFT; break;
                    case 3: button = GAMEPAD_BUTTON_RIGHT_FACE_UP; break;
                    case 4: button = GAMEPAD_BUTTON_LEFT_TRIGGER_1; break;
                    case 5: button = GAMEPAD_BUTTON_RIGHT_TRIGGER_1; break;
                    case 6: button = GAMEPAD_BUTTON_LEFT_TRIGGER_2; break;
                    case 7: button = GAMEPAD_BUTTON_RIGHT_TRIGGER_2; break;
                    case 8: button = GAMEPAD_BUTTON_MIDDLE_LEFT; break;
                    case 9: button = GAMEPAD_BUTTON_MIDDLE_RIGHT; break;
                    case 10: button = GAMEPAD_BUTTON_LEFT_THUMB; break;
                    case 11: button = GAMEPAD_BUTTON_RIGHT_THUMB; break;
                    case 12: button = GAMEPAD_BUTTON_LEFT_FACE_UP; break;
                    case 13: button = GAMEPAD_BUTTON_LEFT_FACE_DOWN; break;
                    case 14: button = GAMEPAD_BUTTON_LEFT_FACE_LEFT; break;
                    case 15: button = GAMEPAD_BUTTON_LEFT_FACE_RIGHT; break;
                    default: break;
                }

                if (button != -1)   // Check for valid button
                {
                    if (gamepadState.digitalButton[j] == 1)
                    {
                        CORE.Input.Gamepad.currentButtonState[i][button] = 1;
                        CORE.Input.Gamepad.lastButtonPressed = button;
                    }
                    else CORE.Input.Gamepad.currentButtonState[i][button] = 0;
                }

                //TRACELOGD("INPUT: Gamepad %d, button %d: Digital: %d, Analog: %g", gamepadState.index, j, gamepadState.digitalButton[j], gamepadState.analogButton[j]);
            }

            // Register axis data for every connected gamepad
            for (int j = 0; (j < gamepadState.numAxes) && (j < MAX_GAMEPAD_AXIS); j++)
            {
                CORE.Input.Gamepad.axisState[i][j] = gamepadState.axis[j];
            }

            CORE.Input.Gamepad.axisCount[i] = gamepadState.numAxes;
        }
    }

    CORE.Window.resizedLastFrame = false;

    // TODO: This code does not seem to do anything??
    //if (CORE.Window.eventWaiting) glfwWaitEvents();     // Wait for in input events before continue (drawing is paused)
    //else glfwPollEvents(); // Poll input events: keyboard/mouse/window events (callbacks) --> WARNING: Where is key input reset?
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

// Initialize platform: graphics, inputs and more
int InitPlatform(void)
{
    glfwSetErrorCallback(ErrorCallback);

    // Initialize GLFW internal global state
    int result = glfwInit();
    if (result == GLFW_FALSE) { TRACELOG(LOG_WARNING, "GLFW: Failed to initialize GLFW"); return -1; }

    // Initialize graphic device: display/window and graphic context
    //----------------------------------------------------------------------------
    glfwDefaultWindowHints(); // Set default windows hints
    // glfwWindowHint(GLFW_RED_BITS, 8);             // Framebuffer red color component bits
    // glfwWindowHint(GLFW_GREEN_BITS, 8);           // Framebuffer green color component bits
    // glfwWindowHint(GLFW_BLUE_BITS, 8);            // Framebuffer blue color component bits
    // glfwWindowHint(GLFW_ALPHA_BITS, 8);           // Framebuffer alpha color component bits
    // glfwWindowHint(GLFW_DEPTH_BITS, 24);          // Depthbuffer bits
    // glfwWindowHint(GLFW_REFRESH_RATE, 0);         // Refresh rate for fullscreen window
    // glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API); // OpenGL API to use. Alternative: GLFW_OPENGL_ES_API
    // glfwWindowHint(GLFW_AUX_BUFFERS, 0);          // Number of auxiliar buffers

    // Check window creation flags
    if ((CORE.Window.flags & FLAG_FULLSCREEN_MODE) > 0) CORE.Window.fullscreen = true;

    if ((CORE.Window.flags & FLAG_WINDOW_HIDDEN) > 0) glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Visible window
    else glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE); // Window initially hidden

    if ((CORE.Window.flags & FLAG_WINDOW_UNDECORATED) > 0) glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Border and buttons on Window
    else glfwWindowHint(GLFW_DECORATED, GLFW_TRUE); // Decorated window

    if ((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) > 0) glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // Resizable window
    else glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Avoid window being resizable

    // Disable FLAG_WINDOW_MINIMIZED, not supported on initialization
    if ((CORE.Window.flags & FLAG_WINDOW_MINIMIZED) > 0) CORE.Window.flags &= ~FLAG_WINDOW_MINIMIZED;

    // Disable FLAG_WINDOW_MAXIMIZED, not supported on initialization
    if ((CORE.Window.flags & FLAG_WINDOW_MAXIMIZED) > 0) CORE.Window.flags &= ~FLAG_WINDOW_MAXIMIZED;

    if ((CORE.Window.flags & FLAG_WINDOW_UNFOCUSED) > 0) glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
    else glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

    if ((CORE.Window.flags & FLAG_WINDOW_TOPMOST) > 0) glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    else glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);

    // NOTE: Some GLFW flags are not supported on HTML5
    // e.g.: GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_MOUSE_PASSTHROUGH

    // Scale content area based on the monitor content scale where window is placed on
    // NOTE: This feature requires emscripten 3.1.51
    //if ((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) > 0) glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    //else glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);

    if (CORE.Window.flags & FLAG_MSAA_4X_HINT)
    {
        // NOTE: MSAA is only enabled for main framebuffer, not user-created FBOs
        TRACELOG(LOG_INFO, "DISPLAY: Trying to enable MSAA x4");
        glfwWindowHint(GLFW_SAMPLES, 4); // Tries to enable multisampling x4 (MSAA), default is 0
    }

    // NOTE: When asking for an OpenGL context version, most drivers provide the highest supported version
    // with backward compatibility to older OpenGL versions.
    // For example, if using OpenGL 1.1, driver can provide a 4.3 backwards compatible context.

    // Check selection OpenGL version
    if (rlGetVersion() == RL_OPENGL_21)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); // Choose OpenGL minor version (just hint)
    }
    else if (rlGetVersion() == RL_OPENGL_33)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);                 // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);                 // Choose OpenGL minor version (just hint)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Profiles Hint: Only 3.3 and above!
                                                                       // Values: GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_ANY_PROFILE, GLFW_OPENGL_COMPAT_PROFILE
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE); // Forward Compatibility Hint: Only 3.3 and above!
        // glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); // Request OpenGL DEBUG context
    }
    else if (rlGetVersion() == RL_OPENGL_43)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Choose OpenGL minor version (just hint)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE);
#if defined(RLGL_ENABLE_OPENGL_DEBUG_CONTEXT)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); // Enable OpenGL Debug Context
#endif
    }
    else if (rlGetVersion() == RL_OPENGL_ES_20) // Request OpenGL ES 2.0 context
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
    }
    else if (rlGetVersion() == RL_OPENGL_ES_30) // Request OpenGL ES 3.0 context
    {
        // TODO: It seems WebGL 2.0 context is not set despite being requested
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
    }

    // NOTE: Getting video modes is not implemented in emscripten GLFW3 version
    CORE.Window.display.width = CORE.Window.screen.width;
    CORE.Window.display.height = CORE.Window.screen.height;

    // Init fullscreen toggle required var:
    platform.ourFullscreen = false;

    if (CORE.Window.fullscreen)
    {
        // remember center for switchinging from fullscreen to window
        if ((CORE.Window.screen.height == CORE.Window.display.height) && (CORE.Window.screen.width == CORE.Window.display.width))
        {
            // If screen width/height equal to the display, we can't calculate the window pos for toggling full-screened/windowed.
            // Toggling full-screened/windowed with pos(0, 0) can cause problems in some platforms, such as X11.
            CORE.Window.position.x = CORE.Window.display.width/4;
            CORE.Window.position.y = CORE.Window.display.height/4;
        }
        else
        {
            CORE.Window.position.x = CORE.Window.display.width/2 - CORE.Window.screen.width/2;
            CORE.Window.position.y = CORE.Window.display.height/2 - CORE.Window.screen.height/2;
        }

        if (CORE.Window.position.x < 0) CORE.Window.position.x = 0;
        if (CORE.Window.position.y < 0) CORE.Window.position.y = 0;

        // Obtain recommended CORE.Window.display.width/CORE.Window.display.height from a valid videomode for the monitor
        int count = 0;
        const GLFWvidmode *modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);

        // Get closest video mode to desired CORE.Window.screen.width/CORE.Window.screen.height
        for (int i = 0; i < count; i++)
        {
            if ((unsigned int)modes[i].width >= CORE.Window.screen.width)
            {
                if ((unsigned int)modes[i].height >= CORE.Window.screen.height)
                {
                    CORE.Window.display.width = modes[i].width;
                    CORE.Window.display.height = modes[i].height;
                    break;
                }
            }
        }

        TRACELOG(LOG_WARNING, "SYSTEM: Closest fullscreen videomode: %i x %i", CORE.Window.display.width, CORE.Window.display.height);

        // NOTE: ISSUE: Closest videomode could not match monitor aspect-ratio, for example,
        // for a desired screen size of 800x450 (16:9), closest supported videomode is 800x600 (4:3),
        // framebuffer is rendered correctly but once displayed on a 16:9 monitor, it gets stretched
        // by the sides to fit all monitor space...

        // Try to setup the most appropriate fullscreen framebuffer for the requested screenWidth/screenHeight
        // It considers device display resolution mode and setups a framebuffer with black bars if required (render size/offset)
        // Modified global variables: CORE.Window.screen.width/CORE.Window.screen.height - CORE.Window.render.width/CORE.Window.render.height - CORE.Window.renderOffset.x/CORE.Window.renderOffset.y - CORE.Window.screenScale
        // TODO: It is a quite cumbersome solution to display size vs requested size, it should be reviewed or removed...
        // HighDPI monitors are properly considered in a following similar function: SetupViewport()
        SetupFramebuffer(CORE.Window.display.width, CORE.Window.display.height);

        platform.handle = glfwCreateWindow(CORE.Window.display.width, CORE.Window.display.height, (CORE.Window.title != 0)? CORE.Window.title : " ", glfwGetPrimaryMonitor(), NULL);

        // NOTE: Full-screen change, not working properly...
        // glfwSetWindowMonitor(platform.handle, glfwGetPrimaryMonitor(), 0, 0, CORE.Window.screen.width, CORE.Window.screen.height, GLFW_DONT_CARE);
    }
    else
    {
        // No-fullscreen window creation
        platform.handle = glfwCreateWindow(CORE.Window.screen.width, CORE.Window.screen.height, (CORE.Window.title != 0)? CORE.Window.title : " ", NULL, NULL);

        if (platform.handle)
        {
            CORE.Window.render.width = CORE.Window.screen.width;
            CORE.Window.render.height = CORE.Window.screen.height;
        }
    }

    if (!platform.handle)
    {
        glfwTerminate();
        TRACELOG(LOG_WARNING, "GLFW: Failed to initialize Window");
        return -1;
    }

    // WARNING: glfwCreateWindow() title doesn't work with emscripten
    emscripten_set_window_title((CORE.Window.title != 0)? CORE.Window.title : " ");

    // Set window callback events
    glfwSetWindowSizeCallback(platform.handle, WindowSizeCallback); // NOTE: Resizing not allowed by default!
    glfwSetWindowIconifyCallback(platform.handle, WindowIconifyCallback);
    glfwSetWindowFocusCallback(platform.handle, WindowFocusCallback);
    glfwSetDropCallback(platform.handle, WindowDropCallback);

    if ((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) > 0)
    {
       glfwSetWindowContentScaleCallback(platform.handle, WindowContentScaleCallback);
    }

    // Set input callback events
    glfwSetKeyCallback(platform.handle, KeyCallback);
    glfwSetCharCallback(platform.handle, CharCallback);
    glfwSetMouseButtonCallback(platform.handle, MouseButtonCallback);
    glfwSetCursorPosCallback(platform.handle, MouseCursorPosCallback); // Track mouse position changes
    glfwSetScrollCallback(platform.handle, MouseScrollCallback);
    glfwSetCursorEnterCallback(platform.handle, CursorEnterCallback);

    glfwMakeContextCurrent(platform.handle);
    result = true; // TODO: WARNING: glfwGetError(NULL); symbol can not be found in Web

    // Check context activation
    if (result == true) //(result != GLFW_NO_WINDOW_CONTEXT) && (result != GLFW_PLATFORM_ERROR))
    {
        CORE.Window.ready = true;

        int fbWidth = CORE.Window.screen.width;
        int fbHeight = CORE.Window.screen.height;

        CORE.Window.render.width = fbWidth;
        CORE.Window.render.height = fbHeight;
        CORE.Window.currentFbo.width = fbWidth;
        CORE.Window.currentFbo.height = fbHeight;

        TRACELOG(LOG_INFO, "DISPLAY: Device initialized successfully");
        TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
        TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
        TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
        TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);
    }
    else
    {
        TRACELOG(LOG_FATAL, "PLATFORM: Failed to initialize graphics device");
        return -1;
    }

    if ((CORE.Window.flags & FLAG_WINDOW_MINIMIZED) > 0) MinimizeWindow();

    // If graphic device is no properly initialized, we end program
    if (!CORE.Window.ready) { TRACELOG(LOG_FATAL, "PLATFORM: Failed to initialize graphic device"); return -1; }

    // Load OpenGL extensions
    // NOTE: GL procedures address loader is required to load extensions
    rlLoadExtensions(glfwGetProcAddress);
    //----------------------------------------------------------------------------

    // Initialize input events callbacks
    //----------------------------------------------------------------------------
    // Setup callback functions for the DOM events
    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 1, EmscriptenFullscreenChangeCallback);

    // WARNING: Below resize code was breaking fullscreen mode for sample games and examples, it needs review
    // Check fullscreen change events(note this is done on the window since most browsers don't support this on #canvas)
    // emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 1, EmscriptenResizeCallback);
    // Check Resize event (note this is done on the window since most browsers don't support this on #canvas)
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 1, EmscriptenResizeCallback);

    // Trigger this once to get initial window sizing
    EmscriptenResizeCallback(EMSCRIPTEN_EVENT_RESIZE, NULL, NULL);

    // Support keyboard events -> Not used, GLFW.JS takes care of that
    // emscripten_set_keypress_callback("#canvas", NULL, 1, EmscriptenKeyboardCallback);
    // emscripten_set_keydown_callback("#canvas", NULL, 1, EmscriptenKeyboardCallback);

    // Support mouse events
    emscripten_set_click_callback("#canvas", NULL, 1, EmscriptenMouseCallback);
    emscripten_set_pointerlockchange_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 1, EmscriptenPointerlockCallback);

    // Support touch events
    emscripten_set_touchstart_callback("#canvas", NULL, 1, EmscriptenTouchCallback);
    emscripten_set_touchend_callback("#canvas", NULL, 1, EmscriptenTouchCallback);
    emscripten_set_touchmove_callback("#canvas", NULL, 1, EmscriptenTouchCallback);
    emscripten_set_touchcancel_callback("#canvas", NULL, 1, EmscriptenTouchCallback);

    // Support gamepad events (not provided by GLFW3 on emscripten)
    emscripten_set_gamepadconnected_callback(NULL, 1, EmscriptenGamepadCallback);
    emscripten_set_gamepaddisconnected_callback(NULL, 1, EmscriptenGamepadCallback);
    //----------------------------------------------------------------------------

    // Initialize timing system
    //----------------------------------------------------------------------------
    InitTimer();
    //----------------------------------------------------------------------------

    // Initialize storage system
    //----------------------------------------------------------------------------
    CORE.Storage.basePath = GetWorkingDirectory();
    //----------------------------------------------------------------------------

    TRACELOG(LOG_INFO, "PLATFORM: WEB: Initialized successfully");

    return 0;
}

// Close platform
void ClosePlatform(void)
{
    glfwDestroyWindow(platform.handle);
    glfwTerminate();
}

// GLFW3 Error Callback, runs on GLFW3 error
static void ErrorCallback(int error, const char *description)
{
    TRACELOG(LOG_WARNING, "GLFW: Error: %i Description: %s", error, description);
}

// GLFW3 WindowSize Callback, runs when window is resizedLastFrame
// NOTE: Window resizing not allowed by default
static void WindowSizeCallback(GLFWwindow *window, int width, int height)
{
    // Reset viewport and projection matrix for new size
    SetupViewport(width, height);

    CORE.Window.currentFbo.width = width;
    CORE.Window.currentFbo.height = height;
    CORE.Window.resizedLastFrame = true;

    if (IsWindowFullscreen()) return;

    // Set current screen size
    if ((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) > 0)
    {
        Vector2 windowScaleDPI = GetWindowScaleDPI();

        CORE.Window.screen.width = (unsigned int)(width/windowScaleDPI.x);
        CORE.Window.screen.height = (unsigned int)(height/windowScaleDPI.y);
    }
    else
    {
        CORE.Window.screen.width = width;
        CORE.Window.screen.height = height;
    }

    // NOTE: Postprocessing texture is not scaled to new size
}

static void WindowContentScaleCallback(GLFWwindow *window, float scalex, float scaley)
{
    CORE.Window.screenScale = MatrixScale(scalex, scaley, 1.0f);
}

// GLFW3 WindowIconify Callback, runs when window is minimized/restored
static void WindowIconifyCallback(GLFWwindow *window, int iconified)
{
    if (iconified) CORE.Window.flags |= FLAG_WINDOW_MINIMIZED;  // The window was iconified
    else CORE.Window.flags &= ~FLAG_WINDOW_MINIMIZED;           // The window was restored
}

/*
// GLFW3 Window Maximize Callback, runs when window is maximized
static void WindowMaximizeCallback(GLFWwindow *window, int maximized)
{
    // TODO.
}
*/

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

// GLFW3 Char Key Callback, runs on key down (gets equivalent unicode char value)
static void CharCallback(GLFWwindow *window, unsigned int key)
{
    //TRACELOG(LOG_DEBUG, "Char Callback: KEY:%i(%c)", key, key);

    // NOTE: Registers any key down considering OS keyboard layout but
    // does not detect action events, those should be managed by user...
    // Ref: https://github.com/glfw/glfw/issues/668#issuecomment-166794907
    // Ref: https://www.glfw.org/docs/latest/input_guide.html#input_char

    // Check if there is space available in the queue
    if (CORE.Input.Keyboard.charPressedQueueCount < MAX_CHAR_PRESSED_QUEUE)
    {
        // Add character to the queue
        CORE.Input.Keyboard.charPressedQueue[CORE.Input.Keyboard.charPressedQueueCount] = key;
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
    // Prevent calling ProcessGestureEvent() when Emscripten is present and there's a touch gesture, so EmscriptenTouchCallback() can handle it itself
    if (GetMouseX() != 0 || GetMouseY() != 0) ProcessGestureEvent(gestureEvent);

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

// Register fullscreen change events
static EM_BOOL EmscriptenFullscreenChangeCallback(int eventType, const EmscriptenFullscreenChangeEvent *event, void *userData)
{
    // NOTE: 1. Reset the fullscreen flags if the user left fullscreen manually by pressing the Escape key
    //       2. Which is a necessary safeguard because that case will bypass the toggles CORE.Window.flags resets
    if (platform.ourFullscreen) platform.ourFullscreen = false;
    else
    {
        const bool wasFullscreen = EM_ASM_INT( { if (document.fullscreenElement) return 1; }, 0);
        if (!wasFullscreen)
        {
            CORE.Window.fullscreen = false;
            CORE.Window.flags &= ~FLAG_FULLSCREEN_MODE;
            CORE.Window.flags &= ~FLAG_BORDERLESS_WINDOWED_MODE;
        }
    }

    return 1; // The event was consumed by the callback handler
}

// Register window resize event
static EM_BOOL EmscriptenWindowResizedCallback(int eventType, const EmscriptenUiEvent *event, void *userData)
{
    // TODO: Implement EmscriptenWindowResizedCallback()?

    return 1; // The event was consumed by the callback handler
}

EM_JS(int, GetWindowInnerWidth, (), { return window.innerWidth; });
EM_JS(int, GetWindowInnerHeight, (), { return window.innerHeight; });

// Register DOM element resize event
static EM_BOOL EmscriptenResizeCallback(int eventType, const EmscriptenUiEvent *event, void *userData)
{
    // Don't resize non-resizeable windows
    if ((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) == 0) return 1;

    // This event is called whenever the window changes sizes,
    // so the size of the canvas object is explicitly retrieved below
    int width = GetWindowInnerWidth();
    int height = GetWindowInnerHeight();

    if (width < CORE.Window.screenMin.width) width = CORE.Window.screenMin.width;
    else if (width > CORE.Window.screenMax.width && CORE.Window.screenMax.width > 0) width = CORE.Window.screenMax.width;

    if (height < CORE.Window.screenMin.height) height = CORE.Window.screenMin.height;
    else if (height > CORE.Window.screenMax.height && CORE.Window.screenMax.height > 0) height = CORE.Window.screenMax.height;

    emscripten_set_canvas_element_size("#canvas", width, height);

    SetupViewport(width, height); // Reset viewport and projection matrix for new size

    CORE.Window.currentFbo.width = width;
    CORE.Window.currentFbo.height = height;
    CORE.Window.resizedLastFrame = true;

    if (IsWindowFullscreen()) return 1;

    // Set current screen size
    CORE.Window.screen.width = width;
    CORE.Window.screen.height = height;

    // NOTE: Postprocessing texture is not scaled to new size

    return 0;
}

// Register mouse input events
static EM_BOOL EmscriptenMouseCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
    // This is only for registering mouse click events with emscripten and doesn't need to do anything

    return 1; // The event was consumed by the callback handler
}

// Register pointer lock events
static EM_BOOL EmscriptenPointerlockCallback(int eventType, const EmscriptenPointerlockChangeEvent *pointerlockChangeEvent, void *userData)
{
    CORE.Input.Mouse.cursorHidden = EM_ASM_INT( { if (document.pointerLockElement) return 1; }, 0);

    return 1; // The event was consumed by the callback handler
}

// Register connected/disconnected gamepads events
static EM_BOOL EmscriptenGamepadCallback(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData)
{
    /*
    TRACELOGD("%s: timeStamp: %g, connected: %d, index: %ld, numAxes: %d, numButtons: %d, id: \"%s\", mapping: \"%s\"",
           eventType != 0? emscripten_event_type_to_string(eventType) : "Gamepad state",
           gamepadEvent->timestamp, gamepadEvent->connected, gamepadEvent->index, gamepadEvent->numAxes, gamepadEvent->numButtons, gamepadEvent->id, gamepadEvent->mapping);

    for (int i = 0; i < gamepadEvent->numAxes; ++i) TRACELOGD("Axis %d: %g", i, gamepadEvent->axis[i]);
    for (int i = 0; i < gamepadEvent->numButtons; ++i) TRACELOGD("Button %d: Digital: %d, Analog: %g", i, gamepadEvent->digitalButton[i], gamepadEvent->analogButton[i]);
    */

    if ((gamepadEvent->connected) && (gamepadEvent->index < MAX_GAMEPADS))
    {
        CORE.Input.Gamepad.ready[gamepadEvent->index] = true;
        sprintf(CORE.Input.Gamepad.name[gamepadEvent->index], "%s", gamepadEvent->id);
    }
    else CORE.Input.Gamepad.ready[gamepadEvent->index] = false;

    return 1; // The event was consumed by the callback handler
}

// Register touch input events
static EM_BOOL EmscriptenTouchCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData)
{
    // Register touch points count
    CORE.Input.Touch.pointCount = touchEvent->numTouches;

    double canvasWidth = 0.0;
    double canvasHeight = 0.0;
    // NOTE: emscripten_get_canvas_element_size() returns canvas.width and canvas.height but
    // we are looking for actual CSS size: canvas.style.width and canvas.style.height
    // EMSCRIPTEN_RESULT res = emscripten_get_canvas_element_size("#canvas", &canvasWidth, &canvasHeight);
    emscripten_get_element_css_size("#canvas", &canvasWidth, &canvasHeight);

    for (int i = 0; (i < CORE.Input.Touch.pointCount) && (i < MAX_TOUCH_POINTS); i++)
    {
        // Register touch points id
        CORE.Input.Touch.pointId[i] = touchEvent->touches[i].identifier;

        // Register touch points position
        CORE.Input.Touch.position[i] = (Vector2){touchEvent->touches[i].targetX, touchEvent->touches[i].targetY};

        // Normalize gestureEvent.position[x] for CORE.Window.screen.width and CORE.Window.screen.height
        CORE.Input.Touch.position[i].x *= ((float)GetScreenWidth()/(float)canvasWidth);
        CORE.Input.Touch.position[i].y *= ((float)GetScreenHeight()/(float)canvasHeight);

        if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART) CORE.Input.Touch.currentTouchState[i] = 1;
        else if (eventType == EMSCRIPTEN_EVENT_TOUCHEND) CORE.Input.Touch.currentTouchState[i] = 0;
    }

    // Update mouse position if we detect a single touch.
    if (CORE.Input.Touch.pointCount == 1)
    {
        CORE.Input.Mouse.currentPosition.x = CORE.Input.Touch.position[0].x;
        CORE.Input.Mouse.currentPosition.y = CORE.Input.Touch.position[0].y;
    }

#if defined(SUPPORT_GESTURES_SYSTEM)
    GestureEvent gestureEvent = {0};

    gestureEvent.pointCount = CORE.Input.Touch.pointCount;

    // Register touch actions
    if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART) gestureEvent.touchAction = TOUCH_ACTION_DOWN;
    else if (eventType == EMSCRIPTEN_EVENT_TOUCHEND) gestureEvent.touchAction = TOUCH_ACTION_UP;
    else if (eventType == EMSCRIPTEN_EVENT_TOUCHMOVE) gestureEvent.touchAction = TOUCH_ACTION_MOVE;
    else if (eventType == EMSCRIPTEN_EVENT_TOUCHCANCEL) gestureEvent.touchAction = TOUCH_ACTION_CANCEL;

    for (int i = 0; (i < gestureEvent.pointCount) && (i < MAX_TOUCH_POINTS); i++)
    {
        gestureEvent.pointId[i] = CORE.Input.Touch.pointId[i];
        gestureEvent.position[i] = CORE.Input.Touch.position[i];

        // Normalize gestureEvent.position[i]
        gestureEvent.position[i].x /= (float)GetScreenWidth();
        gestureEvent.position[i].y /= (float)GetScreenHeight();
    }

    // Gesture data is sent to gestures system for processing
    ProcessGestureEvent(gestureEvent);

    // Reset the pointCount for web, if it was the last Touch End event
    if (eventType == EMSCRIPTEN_EVENT_TOUCHEND && CORE.Input.Touch.pointCount == 1) CORE.Input.Touch.pointCount = 0;
#endif

    return 1; // The event was consumed by the callback handler
}

// EOF
