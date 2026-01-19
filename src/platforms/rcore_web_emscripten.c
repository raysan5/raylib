/**********************************************************************************************
*
*   rcore_web_emscripten - Functions to manage window, graphics device and inputs
*
*   PLATFORM: WEB - EMSCRIPTEN
*       - HTML5 (WebAssembly)
*
*   LIMITATIONS:
*       - TBD
*
*   POSSIBLE IMPROVEMENTS:
*       - TBD
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
*   Copyright (c) 2025-2026 Ramon Santamaria (@raysan5) and contributors
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

#include <emscripten/emscripten.h>      // Emscripten functionality for C
#include <emscripten/html5.h>           // Emscripten HTML5 library

#include <sys/time.h>   // Required for: timespec, nanosleep(), select() - POSIX

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#if (_POSIX_C_SOURCE < 199309L)
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 199309L     // Required for: CLOCK_MONOTONIC if compiled with c99 without gnu ext.
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    char canvasId[64];                  // Current canvas id
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE glContext; // OpenGL context
    unsigned int *pixels;               // Pointer to pixel data buffer (RGBA 32bit format)
} PlatformData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern CoreData CORE;                   // Global CORE state context

static PlatformData platform = { 0 };   // Platform specific data

//----------------------------------------------------------------------------------
// Global Variables Definition
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

// Emscripten window callback events
static EM_BOOL EmscriptenResizeCallback(int eventType, const EmscriptenUiEvent *event, void *userData);
static EM_BOOL EmscriptenFocusCallback(int eventType, const EmscriptenFocusEvent *focusEvent, void *userData);
static EM_BOOL EmscriptenVisibilityChangeCallback(int eventType, const EmscriptenVisibilityChangeEvent *visibilityChangeEvent, void *userData);
static EM_BOOL EmscriptenFullscreenChangeCallback(int eventType, const EmscriptenFullscreenChangeEvent *event, void *userData);
// TODO: Implement GLFW3 alternative for drop callback, runs when drop files into browser/canvas
//static void WindowDropCallback(GLFWwindow *window, int count, const char **paths);

// Emscripten input callback events
static EM_BOOL EmscriptenKeyboardCallback(int eventType, const EmscriptenKeyboardEvent *keyboardEvent, void *userData);
static EM_BOOL EmscriptenMouseCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
static EM_BOOL EmscriptenMouseMoveCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
static EM_BOOL EmscriptenMouseWheelCallback(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData);
static EM_BOOL EmscriptenPointerlockCallback(int eventType, const EmscriptenPointerlockChangeEvent *pointerlockChangeEvent, void *userData);
static EM_BOOL EmscriptenTouchCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
static EM_BOOL EmscriptenGamepadCallback(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData);

// JS: Set the canvas id provided by the module configuration
EM_JS(void, SetCanvasIdJs, (char *out, int outSize), {
    var canvasId = "#" + Module.canvas.id;
    stringToUTF8(canvasId, out, outSize);
});

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
// NOTE: Functions declaration is provided by raylib.h

//----------------------------------------------------------------------------------
// Module Functions Definition: Window and Graphics Device
//----------------------------------------------------------------------------------

// Check if application should close
// This will always return false on a web-build as web builds have no control over this functionality
// Sleep is handled in EndDrawing() for synchronous code
bool WindowShouldClose(void)
{
    // Emscripten Asyncify is required to run synchronous code in asynchronous JS
    // REF: https://emscripten.org/docs/porting/asyncify.html

    // WindowShouldClose() is not called on a web-ready raylib application if using emscripten_set_main_loop()
    // and encapsulating one frame execution on a UpdateDrawFrame() function,
    // allowing the browser to manage execution asynchronously

    // NOTE: Optionally, time can be managed, giving control back-to-browser as required,
    // but it seems below line could generate stuttering on some browsers
    emscripten_sleep(12);

    return false;
}

// Toggle fullscreen mode
void ToggleFullscreen(void)
{
    bool enterFullscreen = false;

    const bool wasFullscreen = EM_ASM_INT( { if (document.fullscreenElement) return 1; }, 0);
    if (wasFullscreen)
    {
        if (FLAG_IS_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE)) enterFullscreen = false;
        else if (FLAG_IS_SET(CORE.Window.flags, FLAG_BORDERLESS_WINDOWED_MODE)) enterFullscreen = true;
        else
        {
            const int canvasWidth = EM_ASM_INT( { return Module.canvas.width; }, 0);
            const int canvasStyleWidth = EM_ASM_INT( { return parseInt(Module.canvas.style.width); }, 0);
            if (canvasStyleWidth > canvasWidth) enterFullscreen = false;
            else enterFullscreen = true;
        }

        EM_ASM(document.exitFullscreen(););

        FLAG_CLEAR(CORE.Window.flags, FLAG_FULLSCREEN_MODE);
        FLAG_CLEAR(CORE.Window.flags, FLAG_BORDERLESS_WINDOWED_MODE);
    }
    else enterFullscreen = true;

    if (enterFullscreen)
    {
        // NOTE: The setTimeouts handle the browser mode change delay
        EM_ASM
        (
            setTimeout(function()
            {
                Module.requestFullscreen(false, false);
            }, 100);
        );

        FLAG_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE);
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
        if (!FLAG_IS_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE))
        {
            // Option 1: Request fullscreen for the canvas element
            // This option does not seem to work at all:
            // emscripten_request_pointerlock() and emscripten_request_fullscreen() are affected by web security,
            // the user must click once on the canvas to hide the pointer or transition to full screen
            //emscripten_request_fullscreen("#canvas", false);

            // Option 2: Request fullscreen for the canvas element with strategy
            // This option does not seem to work at all
            // REF: https://github.com/emscripten-core/emscripten/issues/5124
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

            int width = 0;
            int height = 0;
            emscripten_get_canvas_element_size("#canvas", &width, &height);
            TRACELOG(LOG_WARNING, "Emscripten: Enter fullscreen: Canvas size: %i x %i", width, height);

            FLAG_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE);
        }
        else
        {
            //emscripten_exit_fullscreen();
            //emscripten_exit_soft_fullscreen();

            int width, height;
            emscripten_get_canvas_element_size("#canvas", &width, &height);
            TRACELOG(LOG_WARNING, "Emscripten: Exit fullscreen: Canvas size: %i x %i", width, height);

            FLAG_CLEAR(CORE.Window.flags, FLAG_FULLSCREEN_MODE);
        }
    */
}

// Toggle borderless windowed mode
void ToggleBorderlessWindowed(void)
{
    bool enterBorderless = false;

    const bool wasFullscreen = EM_ASM_INT( { if (document.fullscreenElement) return 1; }, 0);
    if (wasFullscreen)
    {
        if (FLAG_IS_SET(CORE.Window.flags, FLAG_BORDERLESS_WINDOWED_MODE)) enterBorderless = false;
        else if (FLAG_IS_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE)) enterBorderless = true;
        else
        {
            const int canvasWidth = EM_ASM_INT( { return Module.canvas.width; }, 0);
            const int screenWidth = EM_ASM_INT( { return screen.width; }, 0);
            if (screenWidth == canvasWidth) enterBorderless = false;
            else enterBorderless = true;
        }

        EM_ASM(document.exitFullscreen(););

        FLAG_CLEAR(CORE.Window.flags, FLAG_FULLSCREEN_MODE);
        FLAG_CLEAR(CORE.Window.flags, FLAG_BORDERLESS_WINDOWED_MODE);
    }
    else enterBorderless = true;

    if (enterBorderless)
    {
        // 1. The setTimeouts handle the browser mode change delay
        // 2. The style unset handles the possibility of a width="value%" like on the default shell.html file
        EM_ASM
        (
            setTimeout(function()
            {
                Module.requestFullscreen(false, true);
                setTimeout(function()
                {
                    canvas.style.width="unset";
                }, 100);
            }, 100);
        );
        FLAG_SET(CORE.Window.flags, FLAG_BORDERLESS_WINDOWED_MODE);
    }
}

// Set window state: maximized, if resizable
void MaximizeWindow(void)
{
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_RESIZABLE) && !FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED))
    {
        const int tabWidth = EM_ASM_INT( return window.innerWidth; );
        const int tabHeight = EM_ASM_INT( return window.innerHeight; );

        FLAG_SET(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED);
    }
}

// Set window state: minimized
void MinimizeWindow(void)
{
    TRACELOG(LOG_WARNING, "MinimizeWindow() not available on target platform");
}

// Restore window from being minimized/maximized
void RestoreWindow(void)
{
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_RESIZABLE) && FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED))
    {
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
    if (FLAG_IS_SET(flags, FLAG_VSYNC_HINT))
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_VSYNC_HINT) not available on target platform");
    }

    // State change: FLAG_BORDERLESS_WINDOWED_MODE
    if (FLAG_IS_SET(flags, FLAG_BORDERLESS_WINDOWED_MODE))
    {
        // NOTE: Window state flag updated inside ToggleBorderlessWindowed() function
        const bool wasFullscreen = EM_ASM_INT( { if (document.fullscreenElement) return 1; }, 0);
        if (wasFullscreen)
        {
            const int canvasWidth = EM_ASM_INT( { return Module.canvas.width; }, 0);
            const int canvasStyleWidth = EM_ASM_INT( { return parseInt(Module.canvas.style.width); }, 0);
            if ((FLAG_IS_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE)) || canvasStyleWidth > canvasWidth) ToggleBorderlessWindowed();
        }
        else ToggleBorderlessWindowed();
    }

    // State change: FLAG_FULLSCREEN_MODE
    if (FLAG_IS_SET(flags, FLAG_FULLSCREEN_MODE))
    {
        // NOTE: Window state flag updated inside ToggleFullscreen() function
        const bool wasFullscreen = EM_ASM_INT( { if (document.fullscreenElement) return 1; }, 0);
        if (wasFullscreen)
        {
            const int canvasWidth = EM_ASM_INT( { return Module.canvas.width; }, 0);
            const int screenWidth = EM_ASM_INT( { return screen.width; }, 0);
            if (FLAG_IS_SET(CORE.Window.flags, FLAG_BORDERLESS_WINDOWED_MODE) || (screenWidth == canvasWidth)) ToggleFullscreen();
        }
        else ToggleFullscreen();
    }

    // State change: FLAG_WINDOW_RESIZABLE
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_RESIZABLE) != FLAG_IS_SET(flags, FLAG_WINDOW_RESIZABLE)) && FLAG_IS_SET(flags, FLAG_WINDOW_RESIZABLE))
    {
        FLAG_SET(CORE.Window.flags, FLAG_WINDOW_RESIZABLE);
    }

    // State change: FLAG_WINDOW_UNDECORATED
    if (FLAG_IS_SET(flags, FLAG_WINDOW_UNDECORATED))
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_UNDECORATED) not available on target platform");
    }

    // State change: FLAG_WINDOW_HIDDEN
    if (FLAG_IS_SET(flags, FLAG_WINDOW_HIDDEN))
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_HIDDEN) not available on target platform");
    }

    // State change: FLAG_WINDOW_MINIMIZED
    if (FLAG_IS_SET(flags, FLAG_WINDOW_MINIMIZED))
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_MINIMIZED) not available on target platform");
    }

    // State change: FLAG_WINDOW_MAXIMIZED
    if ((FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED) != FLAG_IS_SET(flags, FLAG_WINDOW_MAXIMIZED)) && FLAG_IS_SET(flags, FLAG_WINDOW_MAXIMIZED))
    {
        if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_RESIZABLE))
        {
            const int tabWidth = EM_ASM_INT( return window.innerWidth; );
            const int tabHeight = EM_ASM_INT( return window.innerHeight; );

            FLAG_SET(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED);
        }
    }

    // State change: FLAG_WINDOW_UNFOCUSED
    if (FLAG_IS_SET(flags, FLAG_WINDOW_UNFOCUSED))
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_UNFOCUSED) not available on target platform");
    }

    // State change: FLAG_WINDOW_TOPMOST
    if (FLAG_IS_SET(flags, FLAG_WINDOW_TOPMOST))
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_TOPMOST) not available on target platform");
    }

    // State change: FLAG_WINDOW_ALWAYS_RUN
    if (FLAG_IS_SET(flags, FLAG_WINDOW_ALWAYS_RUN))
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_ALWAYS_RUN) not available on target platform");
    }

    // The following states can not be changed after window creation
    // NOTE: Review for PLATFORM_WEB

    // State change: FLAG_WINDOW_TRANSPARENT
    if (FLAG_IS_SET(flags, FLAG_WINDOW_TRANSPARENT))
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_TRANSPARENT) not available on target platform");
    }

    // State change: FLAG_WINDOW_HIGHDPI
    if (FLAG_IS_SET(flags, FLAG_WINDOW_HIGHDPI))
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_HIGHDPI) not available on target platform");
    }

    // State change: FLAG_WINDOW_MOUSE_PASSTHROUGH
    if (FLAG_IS_SET(flags, FLAG_WINDOW_MOUSE_PASSTHROUGH))
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_WINDOW_MOUSE_PASSTHROUGH) not available on target platform");
    }

    // State change: FLAG_MSAA_4X_HINT
    if (FLAG_IS_SET(flags, FLAG_MSAA_4X_HINT))
    {
        TRACELOG(LOG_WARNING, "SetWindowState(FLAG_MSAA_4X_HINT) not available on target platform");
    }

    // State change: FLAG_INTERLACED_HINT
    if (FLAG_IS_SET(flags, FLAG_INTERLACED_HINT))
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
    if (FLAG_IS_SET(flags, FLAG_VSYNC_HINT))
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_VSYNC_HINT) not available on target platform");
    }

    // State change: FLAG_BORDERLESS_WINDOWED_MODE
    if (FLAG_IS_SET(flags, FLAG_BORDERLESS_WINDOWED_MODE))
    {
        const bool wasFullscreen = EM_ASM_INT( { if (document.fullscreenElement) return 1; }, 0);
        if (wasFullscreen)
        {
            const int canvasWidth = EM_ASM_INT( { return Module.canvas.width; }, 0);
            const int screenWidth = EM_ASM_INT( { return screen.width; }, 0);
            if (FLAG_IS_SET(CORE.Window.flags, FLAG_BORDERLESS_WINDOWED_MODE) || (screenWidth == canvasWidth)) EM_ASM(document.exitFullscreen(););
        }

        FLAG_CLEAR(CORE.Window.flags, FLAG_BORDERLESS_WINDOWED_MODE);
    }

    // State change: FLAG_FULLSCREEN_MODE
    if (FLAG_IS_SET(flags, FLAG_FULLSCREEN_MODE))
    {
        const bool wasFullscreen = EM_ASM_INT( { if (document.fullscreenElement) return 1; }, 0);
        if (wasFullscreen)
        {
            const int canvasWidth = EM_ASM_INT( { return Module.canvas.width; }, 0);
            const int canvasStyleWidth = EM_ASM_INT( { return parseInt(Module.canvas.style.width); }, 0);
            if (FLAG_IS_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE) || (canvasStyleWidth > canvasWidth)) EM_ASM(document.exitFullscreen(););
        }

        FLAG_CLEAR(CORE.Window.flags, FLAG_FULLSCREEN_MODE);
    }

    // State change: FLAG_WINDOW_RESIZABLE
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_RESIZABLE) && FLAG_IS_SET(flags, FLAG_WINDOW_RESIZABLE))
    {
        FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_RESIZABLE);
    }

    // State change: FLAG_WINDOW_HIDDEN
    if (FLAG_IS_SET(flags, FLAG_WINDOW_HIDDEN))
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_HIDDEN) not available on target platform");
    }

    // State change: FLAG_WINDOW_MINIMIZED
    if (FLAG_IS_SET(flags, FLAG_WINDOW_MINIMIZED))
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_MINIMIZED) not available on target platform");
    }

    // State change: FLAG_WINDOW_MAXIMIZED
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED) && FLAG_IS_SET(flags, FLAG_WINDOW_MAXIMIZED))
    {
        if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_RESIZABLE))
        {
            FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED);
        }
    }

    // State change: FLAG_WINDOW_UNDECORATED
    if (FLAG_IS_SET(flags, FLAG_WINDOW_UNDECORATED))
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_UNDECORATED) not available on target platform");
    }

    // State change: FLAG_WINDOW_UNFOCUSED
    if (FLAG_IS_SET(flags, FLAG_WINDOW_UNFOCUSED))
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_UNFOCUSED) not available on target platform");
    }

    // State change: FLAG_WINDOW_TOPMOST
    if (FLAG_IS_SET(flags, FLAG_WINDOW_TOPMOST))
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_TOPMOST) not available on target platform");
    }

    // State change: FLAG_WINDOW_ALWAYS_RUN
    if (FLAG_IS_SET(flags, FLAG_WINDOW_ALWAYS_RUN))
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_ALWAYS_RUN) not available on target platform");
    }

    // The following states can not be changed after window creation
    // NOTE: Review for PLATFORM_WEB

    // State change: FLAG_WINDOW_TRANSPARENT
    if (FLAG_IS_SET(flags, FLAG_WINDOW_TRANSPARENT))
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_TRANSPARENT) not available on target platform");
    }

    // State change: FLAG_WINDOW_HIGHDPI
    if (FLAG_IS_SET(flags, FLAG_WINDOW_HIGHDPI))
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_HIGHDPI) not available on target platform");
    }

    // State change: FLAG_WINDOW_MOUSE_PASSTHROUGH
    if (FLAG_IS_SET(flags, FLAG_WINDOW_MOUSE_PASSTHROUGH))
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_WINDOW_MOUSE_PASSTHROUGH) not available on target platform");
    }

    // State change: FLAG_MSAA_4X_HINT
    if (FLAG_IS_SET(flags, FLAG_MSAA_4X_HINT))
    {
        TRACELOG(LOG_WARNING, "ClearWindowState(FLAG_MSAA_4X_HINT) not available on target platform");
    }

    // State change: FLAG_INTERLACED_HINT
    if (FLAG_IS_SET(flags, FLAG_INTERLACED_HINT))
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
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_RESIZABLE) != 0) EmscriptenResizeCallback(EMSCRIPTEN_EVENT_RESIZE, NULL, NULL);
}

// Set window maximum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMaxSize(int width, int height)
{
    CORE.Window.screenMax.width = width;
    CORE.Window.screenMax.height = height;

    // Trigger the resize event once to update the window maximum width and height
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_RESIZABLE) != 0) EmscriptenResizeCallback(EMSCRIPTEN_EVENT_RESIZE, NULL, NULL);
}

// Set window dimensions
void SetWindowSize(int width, int height)
{
    // When resizing the canvas, several elements must be considered:
    // - CSS canvas size: Web layout size, logical pixels
    // - Canvas contained framebuffer resolution
    // * Browser monitor, device pixel ratio (HighDPI)

    double canvasCssWidth = 0.0;
    double canvasCssHeight = 0.0;
    emscripten_get_element_css_size(platform.canvasId, &canvasCssWidth, &canvasCssHeight);

    // NOTE: emscripten_get_canvas_element_size() returns canvas framebuffer size, not CSS canvas size

    // Get device pixel ratio
    // TODO: Should DPI be considered at this point?
    double dpr = emscripten_get_device_pixel_ratio();

    // Set canvas framebuffer size
    emscripten_set_canvas_element_size(platform.canvasId, width*dpr, height*dpr);

    // Set canvas CSS size
    // TODO: Consider canvas CSS style if already scaled 100%
    EM_ASM({ Module.canvas.style.width = $0; }, width*dpr);
    EM_ASM({ Module.canvas.style.height = $0; }, height*dpr);

    SetupViewport(width*dpr, height*dpr); // Reset viewport and projection matrix for new size
}

// Set window opacity, value opacity is between 0.0 and 1.0
void SetWindowOpacity(float opacity)
{
    if (opacity >= 1.0f) opacity = 1.0f;
    else if (opacity <= 0.0f) opacity = 0.0f;

    EM_ASM({ Module.canvas.style.opacity = $0; }, opacity);
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

// Get current monitor where window is placed
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
    // Get the width of the user's entire screen in CSS logical pixels,
    // no physical pixels, it would require multiplying by device pixel ratio
    // NOTE: Returned value is limited to the current monitor where the browser window is located
    int width = 0;
    width = EM_ASM_INT( { return window.screen.width; }, 0);
    return width;
}

// Get selected monitor height (currently used by monitor)
int GetMonitorHeight(int monitor)
{
    // Get the height of the user's entire screen in CSS logical pixels,
    // no physical pixels, it would require multiplying by device pixel ratio
    // NOTE: Returned value is limited to the current monitor where the browser window is located
    int height = 0;
    height = EM_ASM_INT( { return window.screen.height; }, 0);
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
    // Browser window position, top-left corner relative to the physical screen origin, expressed in CSS logical pixels
    // NOTE: Returned position is relative to the current monitor where the browser window is located
    Vector2 position = { 0, 0 };
    position.x = (float)EM_ASM_INT( { return window.screenX; }, 0);
    position.y = (float)EM_ASM_INT( { return window.screenY; }, 0);
    return position;
}

// Get current monitor device pixel ratio
Vector2 GetWindowScaleDPI(void)
{
    // Get device pixel ratio
    // NOTE: Returned scale is relative to the current monitor where the browser window is located
    Vector2 scale = { 1.0f, 1.0f };
    scale.x = (float)EM_ASM_DOUBLE( { return window.devicePixelRatio; } );
    scale.y = scale.x;
    return scale;
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

// Get clipboard image
Image GetClipboardImage(void)
{
    Image image = { 0 };

    // NOTE: In theory, the new navigator.clipboard.read() can be used to return arbitrary data from clipboard (2024)
    // REF: https://developer.mozilla.org/en-US/docs/Web/API/Clipboard/read
    TRACELOG(LOG_WARNING, "GetClipboardImage() not implemented on target platform");

    return image;
}

// Show mouse cursor
void ShowCursor(void)
{
    if (CORE.Input.Mouse.cursorHidden)
    {
        EM_ASM( { Module.canvas.style.cursor = UTF8ToString($0); }, cursorLUT[CORE.Input.Mouse.cursor]);

        CORE.Input.Mouse.cursorHidden = false;
    }
}

// Hides mouse cursor
void HideCursor(void)
{
    if (!CORE.Input.Mouse.cursorHidden)
    {
        EM_ASM(Module.canvas.style.cursor = 'none';);

        CORE.Input.Mouse.cursorHidden = true;
    }
}

// Enables cursor (unlock cursor)
void EnableCursor(void)
{
    emscripten_exit_pointerlock();

    // Set cursor position in the middle
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);

    // NOTE: CORE.Input.Mouse.cursorLocked handled by EmscriptenPointerlockCallback()
}

// Disables cursor (lock cursor)
void DisableCursor(void)
{
    emscripten_request_pointerlock(platform.canvasId, 1);

    // Set cursor position in the middle
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);

    // NOTE: CORE.Input.Mouse.cursorLocked handled by EmscriptenPointerlockCallback()
}

// Swap back buffer with front buffer (screen drawing)
void SwapScreenBuffer(void)
{
#if defined(GRAPHICS_API_OPENGL_11_SOFTWARE)
    // Update framebuffer
    rlCopyFramebuffer(0, 0, CORE.Window.render.width, CORE.Window.render.height, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, platform.pixels);

    // Copy framebuffer data into canvas
    EM_ASM({
        const width = $0;
        const height = $1;
        const ptr = $2;

        // Get canvas and 2d context created
        const canvas = Module.canvas;
        //const canvas = Module['canvas'];
        const ctx = canvas.getContext('2d');

        if (!Module.__img || (Module.__img.width !== width) || (Module.__img.height !== height)) {
            Module.__img = ctx.createImageData(width, height);
        }

        const src = HEAPU8.subarray(ptr, ptr + width*height*4); // RGBA (4 bytes)
        Module.__img.data.set(src);
        ctx.putImageData(Module.__img, 0, 0);

    }, CORE.Window.screen.width, CORE.Window.screen.height, platform.pixels);
#endif
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Misc
//----------------------------------------------------------------------------------

// Get elapsed time measure in seconds since InitTimer()
double GetTime(void)
{
    double time = 0.0;
    /*
    struct timespec ts = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &ts);
    unsigned long long int nanoSeconds = (unsigned long long int)ts.tv_sec*1000000000LLU + (unsigned long long int)ts.tv_nsec;
    time = (double)(nanoSeconds - CORE.Time.base)*1e-9;  // Elapsed time since InitTimer()
    */
    time = emscripten_get_now()*1000.0;

    return time;
}

// Open URL with default system browser (if available)
// NOTE: This function is only safe to use if you control the URL given
// A user could craft a malicious string performing another action
// Only call this function yourself not with user input or make sure to check the string yourself
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
void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor, float duration)
{
    if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad] && (duration > 0.0f))
    {
        if (leftMotor < 0.0f) leftMotor = 0.0f;
        if (leftMotor > 1.0f) leftMotor = 1.0f;
        if (rightMotor < 0.0f) rightMotor = 0.0f;
        if (rightMotor > 1.0f) rightMotor = 1.0f;
        if (duration > MAX_GAMEPAD_VIBRATION_TIME) duration = MAX_GAMEPAD_VIBRATION_TIME;
        duration *= 1000.0f; // Convert duration to ms

        // NOTE: [2024.10.21] Current browser support:
        // - vibrationActuator API: Chrome, Edge, Opera, Safari, Android Chrome, Android Webview
        // - hapticActuators API: Firefox
        EM_ASM({
            try { navigator.getGamepads()[$0].vibrationActuator.playEffect('dual-rumble', { startDelay: 0, duration: $3, weakMagnitude: $1, strongMagnitude: $2 }); }
            catch (e)
            {
                try { navigator.getGamepads()[$0].hapticActuators[0].pulse($2, $3); }
                catch (e) { }
            }
        }, gamepad, leftMotor, rightMotor, duration);
    }
}

// Set mouse position XY
void SetMousePosition(int x, int y)
{
    // WARNING: Not supported by browser for security reasons
}

// Set mouse cursor
void SetMouseCursor(int cursor)
{
    if (CORE.Input.Mouse.cursor != cursor)
    {
        if (!CORE.Input.Mouse.cursorLocked) EM_ASM( { Module.canvas.style.cursor = UTF8ToString($0); }, cursorLUT[cursor]);
        CORE.Input.Mouse.cursor = cursor;
    }
}

// Get physical key name
const char *GetKeyName(int key)
{
    // TODO: Browser can definitely provide a key name e->key
    TRACELOG(LOG_WARNING, "GetKeyName() not implemented on target platform");
    return "";
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

    // Get number of gamepads connected
    int numGamepads = 0;
    if (emscripten_sample_gamepad_data() == EMSCRIPTEN_RESULT_SUCCESS) numGamepads = emscripten_get_num_gamepads();

    for (int i = 0; (i < numGamepads) && (i < MAX_GAMEPADS); i++)
    {
        // Register previous gamepad button states
        for (int k = 0; k < MAX_GAMEPAD_BUTTONS; k++) CORE.Input.Gamepad.previousButtonState[i][k] = CORE.Input.Gamepad.currentButtonState[i][k];

        EmscriptenGamepadEvent gamepadState = { 0 };
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

                if (button + 1 != 0)   // Check for valid button
                {
                    if (gamepadState.digitalButton[j] == 1)
                    {
                        CORE.Input.Gamepad.currentButtonState[i][button] = 1;
                        CORE.Input.Gamepad.lastButtonPressed = button;
                    }
                    else CORE.Input.Gamepad.currentButtonState[i][button] = 0;
                }

                //TRACELOG(LOG_DEBUG, "INPUT: Gamepad %d, button %d: Digital: %d, Analog: %g", gamepadState.index, j, gamepadState.digitalButton[j], gamepadState.analogButton[j]);
            }

            // Register axis data for every connected gamepad
            for (int j = 0; (j < gamepadState.numAxes) && (j < MAX_GAMEPAD_AXES); j++)
            {
                CORE.Input.Gamepad.axisState[i][j] = gamepadState.axis[j];
            }

            CORE.Input.Gamepad.axisCount[i] = gamepadState.numAxes;
        }
    }

    CORE.Window.resizedLastFrame = false;
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

// Initialize platform: graphics, inputs and more
int InitPlatform(void)
{
    SetCanvasIdJs(platform.canvasId, 64); // Get the current canvas id

    // Initialize graphic device: display/window and graphic context
    //----------------------------------------------------------------------------
    emscripten_set_canvas_element_size(platform.canvasId, CORE.Window.screen.width, CORE.Window.screen.height);
    EmscriptenWebGLContextAttributes attribs = { 0 };
    emscripten_webgl_init_context_attributes(&attribs);
    attribs.alpha = EM_TRUE;
    attribs.depth = EM_TRUE;
    attribs.stencil = EM_FALSE;
    attribs.antialias = EM_FALSE;

    // Check window creation flags
    // Disable FLAG_WINDOW_MINIMIZED, not supported
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MINIMIZED)) FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_MINIMIZED);

    // Disable FLAG_WINDOW_MAXIMIZED, not supported
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED)) FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED);

    // Disable FLAG_WINDOW_TOPMOST, not supported
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_TOPMOST)) FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_TOPMOST);

    // NOTE: Some other flags are not supported on HTML5

    // TODO: Scale content area based on the monitor content scale where window is placed on

    // Request MSAA (usually x4 on WebGL 1.0)
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_MSAA_4X_HINT)) attribs.antialias = EM_TRUE;

    // Check selection OpenGL version
    if (rlGetVersion() == RL_OPENGL_11_SOFTWARE)
    {
        // Avoid creating a WebGL canvas, create 2d canvas for software rendering
        emscripten_set_canvas_element_size(platform.canvasId, CORE.Window.screen.width, CORE.Window.screen.height);
        EM_ASM({
            const canvas = document.getElementById(platform.canvasId);
            Module.canvas = canvas;
        });

        // Load memory framebuffer with desired screen size
        platform.pixels = (unsigned int *)RL_CALLOC(CORE.Window.screen.width*CORE.Window.screen.height, sizeof(unsigned int));
    }
    else if (rlGetVersion() == RL_OPENGL_ES_20) // Request OpenGL ES 2.0 context --> WebGL 1.0
    {
        attribs.majorVersion = 1; // WebGL 1.0 requested
        attribs.minorVersion = 0;

        // Create WebGL context
        platform.glContext = emscripten_webgl_create_context(platform.canvasId, &attribs);
        if (platform.glContext == 0) return 0;

        emscripten_webgl_make_context_current(platform.glContext);
    }
    else if (rlGetVersion() == RL_OPENGL_ES_30) // Request OpenGL ES 3.0 context --> WebGL 2.0
    {
        attribs.majorVersion = 2; // WebGL 2.0 requested
        attribs.minorVersion = 0;

        // Create WebGL context
        platform.glContext = emscripten_webgl_create_context(platform.canvasId, &attribs);
        if (platform.glContext == 0) return 0;

        emscripten_webgl_make_context_current(platform.glContext);
    }

    // NOTE: Getting video modes is not implemented in emscripten GLFW3 version
    CORE.Window.display.width = CORE.Window.screen.width;
    CORE.Window.display.height = CORE.Window.screen.height;
    CORE.Window.render.width = CORE.Window.screen.width;
    CORE.Window.render.height = CORE.Window.screen.height;

    // Set default window title
    emscripten_set_window_title((CORE.Window.title != 0)? CORE.Window.title : " ");

    // Check context activation
    if ((platform.glContext != 0) || (platform.pixels != NULL))
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

    // Load OpenGL extensions
    // NOTE: GL procedures address loader is required to load extensions
    if (platform.glContext != 0) rlLoadExtensions(emscripten_webgl_get_proc_address);
    //----------------------------------------------------------------------------

    // Initialize events callbacks
    //----------------------------------------------------------------------------
    // Setup window/canvas events callbacks
    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 1, EmscriptenFullscreenChangeCallback);
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 1, EmscriptenResizeCallback);
    emscripten_set_blur_callback(platform.canvasId, NULL, 1, EmscriptenFocusCallback);
    emscripten_set_focus_callback(platform.canvasId, NULL, 1, EmscriptenFocusCallback);
    emscripten_set_visibilitychange_callback(NULL, 1, EmscriptenVisibilityChangeCallback);

    // Setup input events
    emscripten_set_keypress_callback(platform.canvasId, NULL, 1, EmscriptenKeyboardCallback);
    emscripten_set_keydown_callback(platform.canvasId, NULL, 1, EmscriptenKeyboardCallback);
    emscripten_set_keyup_callback(platform.canvasId, NULL, 1, EmscriptenKeyboardCallback);

    emscripten_set_click_callback(platform.canvasId, NULL, 1, EmscriptenMouseCallback);
    //emscripten_set_dblclick_callback(platform.canvasId, NULL, 1, EmscriptenMouseCallback);
    emscripten_set_mousedown_callback(platform.canvasId, NULL, 1, EmscriptenMouseCallback);
    emscripten_set_mouseup_callback(platform.canvasId, NULL, 1, EmscriptenMouseCallback);
    emscripten_set_mousemove_callback(platform.canvasId, NULL, 1, EmscriptenMouseCallback);
    emscripten_set_mousemove_callback(platform.canvasId, NULL, 1, EmscriptenMouseMoveCallback);
    emscripten_set_wheel_callback(platform.canvasId, NULL, 1, EmscriptenMouseWheelCallback);
    emscripten_set_pointerlockchange_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 1, EmscriptenPointerlockCallback);

    emscripten_set_touchstart_callback(platform.canvasId, NULL, 1, EmscriptenTouchCallback);
    emscripten_set_touchend_callback(platform.canvasId, NULL, 1, EmscriptenTouchCallback);
    emscripten_set_touchmove_callback(platform.canvasId, NULL, 1, EmscriptenTouchCallback);
    emscripten_set_touchcancel_callback(platform.canvasId, NULL, 1, EmscriptenTouchCallback);

    emscripten_set_gamepadconnected_callback(NULL, 1, EmscriptenGamepadCallback);
    emscripten_set_gamepaddisconnected_callback(NULL, 1, EmscriptenGamepadCallback);

    // Trigger resize callback to force initial size
    EmscriptenResizeCallback(EMSCRIPTEN_EVENT_RESIZE, NULL, NULL);
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
// NOTE: Platform closing is managed by browser, so,
// this function is actually not required, but still
// implementing some logic behaviour
void ClosePlatform(void)
{
    if (platform.pixels != NULL) RL_FREE(platform.pixels);
    if (platform.glContext != 0) emscripten_webgl_destroy_context(platform.glContext);
}

// Emscripten callback functions, called on specific browser events
//-------------------------------------------------------------------------------------------------------
// Emscripten: Called on resize event
static EM_BOOL EmscriptenResizeCallback(int eventType, const EmscriptenUiEvent *event, void *userData)
{
    // Don't resize non-resizeable windows
    if (!FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_RESIZABLE)) return 1;
/*
    // Set current screen size
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIGHDPI))
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
*/
    // This event is called whenever the window changes sizes,
    // so the size of the canvas object is explicitly retrieved below
    int width = EM_ASM_INT( return window.innerWidth; );
    int height = EM_ASM_INT( return window.innerHeight; );

    if (width < (int)CORE.Window.screenMin.width) width = CORE.Window.screenMin.width;
    else if ((width > (int)CORE.Window.screenMax.width) && (CORE.Window.screenMax.width > 0)) width = CORE.Window.screenMax.width;

    if (height < (int)CORE.Window.screenMin.height) height = CORE.Window.screenMin.height;
    else if ((height > (int)CORE.Window.screenMax.height) && (CORE.Window.screenMax.height > 0)) height = CORE.Window.screenMax.height;

    emscripten_set_canvas_element_size(platform.canvasId, width, height);

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

// Emscripten: Called on windows focus change events
static EM_BOOL EmscriptenFocusCallback(int eventType, const EmscriptenFocusEvent *focusEvent, void *userData)
{
    EM_BOOL consumed = 1;

    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_BLUR: FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_UNFOCUSED); break; // The canvas lost focus
        case EMSCRIPTEN_EVENT_FOCUS: FLAG_SET(CORE.Window.flags, FLAG_WINDOW_UNFOCUSED); break;
        default: consumed = 0; break;
    }

    return consumed;
}

// Emscripten: Called on visibility change events
static EM_BOOL EmscriptenVisibilityChangeCallback(int eventType, const EmscriptenVisibilityChangeEvent *visibilityChangeEvent, void *userData)
{
    if (visibilityChangeEvent->hidden) FLAG_SET(CORE.Window.flags, FLAG_WINDOW_HIDDEN); // The window was hidden
    else FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_HIDDEN); // The window was restored

    return 1; // The event was consumed by the callback handler
}

// Emscripten: Called on fullscreen change events
// TODO: Review fullscreen strategy
static EM_BOOL EmscriptenFullscreenChangeCallback(int eventType, const EmscriptenFullscreenChangeEvent *event, void *userData)
{
    // NOTE: Reset the fullscreen flags if the user left fullscreen manually by pressing the Escape key
    const bool wasFullscreen = EM_ASM_INT( { if (document.fullscreenElement) return 1; }, 0);
    if (!wasFullscreen)
    {
        FLAG_CLEAR(CORE.Window.flags, FLAG_FULLSCREEN_MODE);
        FLAG_CLEAR(CORE.Window.flags, FLAG_BORDERLESS_WINDOWED_MODE);
    }

    return 1; // The event was consumed by the callback handler
}

/*
// GLFW3: Called on file-drop over the window
// TODO: Implement Emscripten (or HTML5/JS) alternative
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

        // WARNING: Paths are freed by GLFW when the callback returns, an internal copy should be kept
        CORE.Window.dropFileCount = count;
        CORE.Window.dropFilepaths = (char **)RL_CALLOC(CORE.Window.dropFileCount, sizeof(char *));

        for (unsigned int i = 0; i < CORE.Window.dropFileCount; i++)
        {
            CORE.Window.dropFilepaths[i] = (char *)RL_CALLOC(MAX_FILEPATH_LENGTH, sizeof(char));
            strncpy(CORE.Window.dropFilepaths[i], paths[i], MAX_FILEPATH_LENGTH - 1);
        }
    }
}
*/

// Emscripten: Called on key events
// TODO: keyCodes should be mapped to raylib/GLFW3 Key values
static EM_BOOL EmscriptenKeyboardCallback(int eventType, const EmscriptenKeyboardEvent *keyboardEvent, void *userData)
{
    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_KEYPRESS:
        {
            if (keyboardEvent->repeat) CORE.Input.Keyboard.keyRepeatInFrame[keyboardEvent->keyCode] = 1;
        } break;
        case EMSCRIPTEN_EVENT_KEYDOWN:
        {
            CORE.Input.Keyboard.currentKeyState[keyboardEvent->keyCode] = 1;
        } break;
        case EMSCRIPTEN_EVENT_KEYUP:
        {
            CORE.Input.Keyboard.currentKeyState[keyboardEvent->keyCode] = 0;
        } break;
        default: break;
    }

    // TODO: Add char codes
    //unsigned int charCode
    // Check if there is space available in the queue for characters to be added
    /*
    if (CORE.Input.Keyboard.charPressedQueueCount < MAX_CHAR_PRESSED_QUEUE)
    {
        // Add character to the queue
        CORE.Input.Keyboard.charPressedQueue[CORE.Input.Keyboard.charPressedQueueCount] = keyboardEvent->charCode;
        CORE.Input.Keyboard.charPressedQueueCount++;
    }
    */
    /*
    // Check if there is space available in the key queue
    if ((CORE.Input.Keyboard.keyPressedQueueCount < MAX_KEY_PRESSED_QUEUE) && (eventType == EMSCRIPTEN_EVENT_KEYPRESS))
    {
        // Add character to the queue
        CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = keyboardEvent->keyCode;
        CORE.Input.Keyboard.keyPressedQueueCount++;
    }

    // Check the exit key to set close window
    //if ((keyboardEvent->keyCode == CORE.Input.Keyboard.exitKey) && (eventType == EMSCRIPTEN_EVENT_KEYPRESS)) CORE.Window.shouldClose = true;
    */

    return 1; // The event was consumed by the callback handler
}

// Emscripten: Called on mouse input events
static EM_BOOL EmscriptenMouseCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
    switch (eventType)
    {
        case EMSCRIPTEN_EVENT_MOUSEENTER: CORE.Input.Mouse.cursorOnScreen = true; break;
        case EMSCRIPTEN_EVENT_MOUSELEAVE: CORE.Input.Mouse.cursorOnScreen = false; break;
        case EMSCRIPTEN_EVENT_MOUSEDOWN:
        {
            // NOTE: Emscripten and raylib buttons indices are not aligned
            if (mouseEvent->button == 0) CORE.Input.Mouse.currentButtonState[MOUSE_BUTTON_LEFT] = 1;
            else if (mouseEvent->button == 1) CORE.Input.Mouse.currentButtonState[MOUSE_BUTTON_MIDDLE] = 1;
            else if (mouseEvent->button == 2) CORE.Input.Mouse.currentButtonState[MOUSE_BUTTON_RIGHT] = 1;

            //CORE.Input.Touch.currentTouchState[button] = action;
        } break;
        case EMSCRIPTEN_EVENT_MOUSEUP:
        {
            if (mouseEvent->button == 0) CORE.Input.Mouse.currentButtonState[MOUSE_BUTTON_LEFT] = 0;
            else if (mouseEvent->button == 1) CORE.Input.Mouse.currentButtonState[MOUSE_BUTTON_MIDDLE] = 0;
            else if (mouseEvent->button == 2) CORE.Input.Mouse.currentButtonState[MOUSE_BUTTON_RIGHT] = 0;
        } break;
        default: break;
    }

#if defined(SUPPORT_GESTURES_SYSTEM) && defined(SUPPORT_MOUSE_GESTURES)
    // Process mouse events as touches to be able to use mouse-gestures
    GestureEvent gestureEvent = { 0 };

    // Register touch actions
    if ((CORE.Input.Mouse.currentButtonState[MOUSE_BUTTON_LEFT] == 1) && (CORE.Input.Mouse.previousButtonState[MOUSE_BUTTON_LEFT] == 0)) gestureEvent.touchAction = TOUCH_ACTION_DOWN;
    else if ((CORE.Input.Mouse.currentButtonState[MOUSE_BUTTON_LEFT] == 0) && (CORE.Input.Mouse.previousButtonState[MOUSE_BUTTON_LEFT] == 1)) gestureEvent.touchAction = TOUCH_ACTION_UP;

    // NOTE: TOUCH_ACTION_MOVE event is registered in MouseMoveCallback()

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

    return 1; // The event was consumed by the callback handler
}

// Emscripten: Called on mouse move events
static EM_BOOL EmscriptenMouseMoveCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
    if (CORE.Input.Mouse.cursorLocked)
    {
        CORE.Input.Mouse.previousPosition.x = CORE.Input.Mouse.lockedPosition.x - mouseEvent->movementX;
        CORE.Input.Mouse.previousPosition.y = CORE.Input.Mouse.lockedPosition.y - mouseEvent->movementY;
    }
    else
    {
        // Get mouse position in canvas CSS pixels
        float mouseCssX = (float)mouseEvent->canvasX;
        float mouseCssY = (float)mouseEvent->canvasY;

        // Get canvas sizes
        double cssWidth = 0.0;
        double cssHeight = 0.0;
        emscripten_get_element_css_size(platform.canvasId, &cssWidth, &cssHeight);

        int fbWidth = 0;
        int fbHeight = 0;
        emscripten_get_canvas_element_size(platform.canvasId, &fbWidth, &fbHeight);

        // Convert CSS to framebuffer coordinates
        float scaleX = (float)fbWidth/(float)cssWidth;
        float scaleY = (float)fbHeight/(float)cssHeight;

        int mouseX = (int)(mouseCssX*scaleX);
        int mouseY = (int)(mouseCssY*scaleY);

        CORE.Input.Mouse.currentPosition.x = mouseX;//(float)mouseEvent->canvasX;
        CORE.Input.Mouse.currentPosition.y = mouseY;//(float)mouseEvent->canvasY;

        // Shorter alternative:
        //double dpr = emscripten_get_device_pixel_ratio();
        //int mouseX = (int)(e->canvasX*dpr);
        //int mouseY = (int)(e->canvasY*dpr);

        CORE.Input.Touch.position[0] = CORE.Input.Mouse.currentPosition;
    }

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

    return 1; // The event was consumed by the callback handler
}

// Emscripten: Called on mouse wheel events
static EM_BOOL EmscriptenMouseWheelCallback(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData)
{
    if (eventType == EMSCRIPTEN_EVENT_WHEEL)
    {
        CORE.Input.Mouse.currentWheelMove.x = (float)wheelEvent->deltaX;
        CORE.Input.Mouse.currentWheelMove.y = (float)wheelEvent->deltaY;
    }

    return 1; // The event was consumed by the callback handler
}

// Emscripten: Called on pointer lock events
static EM_BOOL EmscriptenPointerlockCallback(int eventType, const EmscriptenPointerlockChangeEvent *pointerlockChangeEvent, void *userData)
{
    CORE.Input.Mouse.cursorLocked = EM_ASM_INT( { if (document.pointerLockElement) return 1; }, 0);

    if (CORE.Input.Mouse.cursorLocked)
    {
        CORE.Input.Mouse.lockedPosition = CORE.Input.Mouse.currentPosition;
        CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.lockedPosition;
    }

    return 1; // The event was consumed by the callback handler
}

// Emscripten: Called on connect/disconnect gamepads events
static EM_BOOL EmscriptenGamepadCallback(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData)
{
    /*
    TRACELOG(LOG_DEBUG, "%s: timeStamp: %g, connected: %d, index: %ld, numAxes: %d, numButtons: %d, id: \"%s\", mapping: \"%s\"",
           eventType != 0? emscripten_event_type_to_string(eventType) : "Gamepad state",
           gamepadEvent->timestamp, gamepadEvent->connected, gamepadEvent->index, gamepadEvent->numAxes, gamepadEvent->numButtons, gamepadEvent->id, gamepadEvent->mapping);

    for (int i = 0; i < gamepadEvent->numAxes; i++) TRACELOG(LOG_DEBUG, "Axis %d: %g", i, gamepadEvent->axis[i]);
    for (int i = 0; i < gamepadEvent->numButtons; i++) TRACELOG(LOG_DEBUG, "Button %d: Digital: %d, Analog: %g", i, gamepadEvent->digitalButton[i], gamepadEvent->analogButton[i]);
    */

    if (gamepadEvent->connected && (gamepadEvent->index < MAX_GAMEPADS))
    {
        CORE.Input.Gamepad.ready[gamepadEvent->index] = true;
        snprintf(CORE.Input.Gamepad.name[gamepadEvent->index], MAX_GAMEPAD_NAME_LENGTH, "%s", gamepadEvent->id);
    }
    else CORE.Input.Gamepad.ready[gamepadEvent->index] = false;

    return 1; // The event was consumed by the callback handler
}

// Emscripten: Called on touch input events
static EM_BOOL EmscriptenTouchCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData)
{
    // Register touch points count
    CORE.Input.Touch.pointCount = touchEvent->numTouches;

    double canvasWidth = 0.0;
    double canvasHeight = 0.0;
    // NOTE: emscripten_get_canvas_element_size() returns canvas.width and canvas.height but
    // looking for actual CSS size: canvas.style.width and canvas.style.height
    // EMSCRIPTEN_RESULT res = emscripten_get_canvas_element_size("#canvas", &canvasWidth, &canvasHeight);
    emscripten_get_element_css_size(platform.canvasId, &canvasWidth, &canvasHeight);

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

    // Update mouse position if a single touch is detected
    if (CORE.Input.Touch.pointCount == 1)
    {
        CORE.Input.Mouse.currentPosition.x = CORE.Input.Touch.position[0].x;
        CORE.Input.Mouse.currentPosition.y = CORE.Input.Touch.position[0].y;
    }

#if defined(SUPPORT_GESTURES_SYSTEM)
    GestureEvent gestureEvent = { 0 };
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
#endif

    if (eventType == EMSCRIPTEN_EVENT_TOUCHEND)
    {
        // Identify the EMSCRIPTEN_EVENT_TOUCHEND and remove it from the list
        for (int i = 0; i < CORE.Input.Touch.pointCount; i++)
        {
            if (touchEvent->touches[i].isChanged)
            {
                // Move all touch points one position up
                for (int j = i; j < CORE.Input.Touch.pointCount - 1; j++)
                {
                    CORE.Input.Touch.pointId[j] = CORE.Input.Touch.pointId[j + 1];
                    CORE.Input.Touch.position[j] = CORE.Input.Touch.position[j + 1];
                }
                // Decrease touch points count to remove the last one
                CORE.Input.Touch.pointCount--;
                break;
            }
        }
        // Clamp pointCount to avoid negative values
        if (CORE.Input.Touch.pointCount < 0) CORE.Input.Touch.pointCount = 0;
    }

    return 1; // The event was consumed by the callback handler
}
//-------------------------------------------------------------------------------------------------------

// EOF
