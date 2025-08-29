/**********************************************************************************************
*
*   rcore_desktop_rgfw - Functions to manage window, graphics device and inputs
*
*   PLATFORM: RGFW
*       - Windows (Win32, Win64)
*       - Linux (X11/Wayland desktop mode)
*       - MacOS (Cocoa)
*       - HTML5 (Emscripten)
*       - Others (untested)
*
*   LIMITATIONS:
*       - TODO
*
*   POSSIBLE IMPROVEMENTS:
*       - TODO
*
*   ADDITIONAL NOTES:
*       - TRACELOG() function is located in raylib [utils] module
*
*   CONFIGURATION:
*       #define RCORE_PLATFORM_RGFW
*           Custom flag for rcore on target platform RGFW
*
*   DEPENDENCIES:
*       - RGFW.h (main library): Windowing and inputs management
*       - gestures: Gestures system for touch-ready devices (or simulated from mouse inputs)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2025 Ramon Santamaria (@raysan5), Colleague Riley and contributors
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

#ifndef RAYLIB_H /* this should never actually happen, it's only here for IDEs */
#include "raylib.h"
#include "../rcore.c"
#endif

#if defined(PLATFORM_WEB_RGFW)
#define RGFW_NO_GL_HEADER
#endif

#if defined(GRAPHICS_API_OPENGL_ES2) && !defined(PLATFORM_WEB_RGFW)
    #define RGFW_OPENGL_ES2
#endif

void ShowCursor(void);
void CloseWindow(void);

#if defined(__linux__)
    #define _INPUT_EVENT_CODES_H
#endif

#if defined(__unix__) || defined(__linux__)
    #define _XTYPEDEF_FONT
#endif

#define RGFW_IMPLEMENTATION

#if defined(_WIN32) || defined(_WIN64)
    #define WIN32_LEAN_AND_MEAN
    #define Rectangle rectangle_win32
    #define CloseWindow CloseWindow_win32
    #define ShowCursor __imp_ShowCursor
    #define _APISETSTRING_
    
    #undef MAX_PATH

    __declspec(dllimport) int __stdcall MultiByteToWideChar(unsigned int CodePage, unsigned long dwFlags, const char *lpMultiByteStr, int cbMultiByte, wchar_t *lpWideCharStr, int cchWideChar);
#endif

#if defined(__APPLE__)
    #define Point NSPOINT
    #define Size NSSIZE
#endif

#define RGFW_ALLOC RL_MALLOC
#define RGFW_FREE RL_FREE
#define RGFW_CALLOC RL_CALLOC

#include "../external/RGFW.h"

#if defined(_WIN32) || defined(_WIN64)
    #undef DrawText
    #undef ShowCursor
    #undef CloseWindow
    #undef Rectangle

    #undef MAX_PATH
    #define MAX_PATH 1025
#endif

#if defined(__APPLE__)
    #undef Point
    #undef Size
#endif

#include <stdbool.h>
#include <string.h>     // Required for: strcmp()

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    RGFW_window *window;                // Native display device (physical screen connection)
    RGFW_monitor mon;
} PlatformData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern CoreData CORE;                   // Global CORE state context

static PlatformData platform = { 0 };   // Platform specific

static bool RGFW_disableCursor = false;

static const unsigned short keyMappingRGFW[] = {
    [RGFW_keyNULL] = KEY_NULL,
    [RGFW_return] = KEY_ENTER,
    [RGFW_return] = KEY_ENTER,
    [RGFW_apostrophe] = KEY_APOSTROPHE,
    [RGFW_comma] = KEY_COMMA,
    [RGFW_minus] = KEY_MINUS,
    [RGFW_period] = KEY_PERIOD,
    [RGFW_slash] = KEY_SLASH,
    [RGFW_escape] = KEY_ESCAPE,
    [RGFW_F1] = KEY_F1,
    [RGFW_F2] = KEY_F2,
    [RGFW_F3] = KEY_F3,
    [RGFW_F4] = KEY_F4,
    [RGFW_F5] = KEY_F5,
    [RGFW_F6] = KEY_F6,
    [RGFW_F7] = KEY_F7,
    [RGFW_F8] = KEY_F8,
    [RGFW_F9] = KEY_F9,
    [RGFW_F10] = KEY_F10,
    [RGFW_F11] = KEY_F11,
    [RGFW_F12] = KEY_F12,
    [RGFW_backtick] = KEY_GRAVE,
    [RGFW_0] = KEY_ZERO,
    [RGFW_1] = KEY_ONE,
    [RGFW_2] = KEY_TWO,
    [RGFW_3] = KEY_THREE,
    [RGFW_4] = KEY_FOUR,
    [RGFW_5] = KEY_FIVE,
    [RGFW_6] = KEY_SIX,
    [RGFW_7] = KEY_SEVEN,
    [RGFW_8] = KEY_EIGHT,
    [RGFW_9] = KEY_NINE,
    [RGFW_equals] = KEY_EQUAL,
    [RGFW_backSpace] = KEY_BACKSPACE,
    [RGFW_tab] = KEY_TAB,
    [RGFW_capsLock] = KEY_CAPS_LOCK,
    [RGFW_shiftL] = KEY_LEFT_SHIFT,
    [RGFW_controlL] = KEY_LEFT_CONTROL,
    [RGFW_altL] = KEY_LEFT_ALT,
    [RGFW_superL] = KEY_LEFT_SUPER,
    #ifndef RGFW_MACOS
    [RGFW_shiftR] = KEY_RIGHT_SHIFT,
    [RGFW_controlR] = KEY_RIGHT_CONTROL,
    [RGFW_altR] = KEY_RIGHT_ALT,
    [RGFW_superR] = KEY_RIGHT_SUPER,
    #endif
    [RGFW_space] = KEY_SPACE,

    [RGFW_a] = KEY_A,
    [RGFW_b] = KEY_B,
    [RGFW_c] = KEY_C,
    [RGFW_d] = KEY_D,
    [RGFW_e] = KEY_E,
    [RGFW_f] = KEY_F,
    [RGFW_g] = KEY_G,
    [RGFW_h] = KEY_H,
    [RGFW_i] = KEY_I,
    [RGFW_j] = KEY_J,
    [RGFW_k] = KEY_K,
    [RGFW_l] = KEY_L,
    [RGFW_m] = KEY_M,
    [RGFW_n] = KEY_N,
    [RGFW_o] = KEY_O,
    [RGFW_p] = KEY_P,
    [RGFW_q] = KEY_Q,
    [RGFW_r] = KEY_R,
    [RGFW_s] = KEY_S,
    [RGFW_t] = KEY_T,
    [RGFW_u] = KEY_U,
    [RGFW_v] = KEY_V,
    [RGFW_w] = KEY_W,
    [RGFW_x] = KEY_X,
    [RGFW_y] = KEY_Y,
    [RGFW_z] = KEY_Z,
    [RGFW_bracket] = KEY_LEFT_BRACKET,
    [RGFW_backSlash] = KEY_BACKSLASH,
    [RGFW_closeBracket] = KEY_RIGHT_BRACKET,
    [RGFW_semicolon] = KEY_SEMICOLON,
    [RGFW_insert] = KEY_INSERT,
    [RGFW_home] = KEY_HOME,
    [RGFW_pageUp] = KEY_PAGE_UP,
    [RGFW_delete] = KEY_DELETE,
    [RGFW_end] = KEY_END,
    [RGFW_pageDown] = KEY_PAGE_DOWN,
    [RGFW_right] = KEY_RIGHT,
    [RGFW_left] = KEY_LEFT,
    [RGFW_down] = KEY_DOWN,
    [RGFW_up] = KEY_UP,
    [RGFW_numLock] = KEY_NUM_LOCK,
    [RGFW_KP_Slash] = KEY_KP_DIVIDE,
    [RGFW_multiply] = KEY_KP_MULTIPLY,
    [RGFW_KP_Minus] = KEY_KP_SUBTRACT,
    [RGFW_KP_Return] = KEY_KP_ENTER,
    [RGFW_KP_1] = KEY_KP_1,
    [RGFW_KP_2] = KEY_KP_2,
    [RGFW_KP_3] = KEY_KP_3,
    [RGFW_KP_4] = KEY_KP_4,
    [RGFW_KP_5] = KEY_KP_5,
    [RGFW_KP_6] = KEY_KP_6,
    [RGFW_KP_7] = KEY_KP_7,
    [RGFW_KP_8] = KEY_KP_8,
    [RGFW_KP_9] = KEY_KP_9,
    [RGFW_KP_0] = KEY_KP_0,
    [RGFW_KP_Period] = KEY_KP_DECIMAL,
    [RGFW_scrollLock] = KEY_SCROLL_LOCK,
};

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
int InitPlatform(void);          // Initialize platform (graphics, inputs and more)
bool InitGraphicsDevice(void);   // Initialize graphics device

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
    if (CORE.Window.shouldClose == false)
        CORE.Window.shouldClose = RGFW_window_shouldClose(platform.window);
    if (CORE.Window.ready) return CORE.Window.shouldClose;
    else return true;
}

// Toggle fullscreen mode
void ToggleFullscreen(void)
{
    if (!CORE.Window.fullscreen)
    {
        // Store previous window position (in case we exit fullscreen)
        CORE.Window.previousPosition = CORE.Window.position;
        CORE.Window.previousScreen = CORE.Window.screen;

        platform.mon = RGFW_window_getMonitor(platform.window);
        CORE.Window.fullscreen = true;
        CORE.Window.flags |= FLAG_FULLSCREEN_MODE;

        RGFW_monitor_scaleToWindow(platform.mon, platform.window);
        RGFW_window_setFullscreen(platform.window, 1);
    }
    else
    {
        CORE.Window.fullscreen = false;
        CORE.Window.flags &= ~FLAG_FULLSCREEN_MODE;

        if (platform.mon.mode.area.w)
        {
            RGFW_monitor monitor = RGFW_window_getMonitor(platform.window);
            RGFW_monitor_requestMode(monitor, platform.mon.mode, RGFW_monitorScale);

            platform.mon.mode.area.w = 0;
        }

        // we update the window position right away
        CORE.Window.position = CORE.Window.previousPosition;
        RGFW_window_setFullscreen(platform.window, 0);
        RGFW_window_move(platform.window, RGFW_POINT(CORE.Window.position.x, CORE.Window.position.y));
        RGFW_window_resize(platform.window, RGFW_AREA(CORE.Window.previousScreen.width, CORE.Window.previousScreen.height));
    }

    // Try to enable GPU V-Sync, so frames are limited to screen refresh rate (60Hz -> 60 FPS)
    // NOTE: V-Sync can be enabled by graphic driver configuration
    if (CORE.Window.flags & FLAG_VSYNC_HINT) RGFW_window_swapInterval(platform.window, 1);
}

// Toggle borderless windowed mode
void ToggleBorderlessWindowed(void)
{
    if (CORE.Window.fullscreen)
    {
        CORE.Window.previousPosition = CORE.Window.position;
        CORE.Window.previousScreen = CORE.Window.screen;

        RGFW_window_setBorder(platform.window, 0);

        RGFW_monitor mon = RGFW_window_getMonitor(platform.window);
        RGFW_window_resize(platform.window, mon.mode.area);
    }
    else
    {
        RGFW_window_setBorder(platform.window, 1);

        CORE.Window.position = CORE.Window.previousPosition;
        RGFW_window_resize(platform.window, RGFW_AREA(CORE.Window.previousScreen.width, CORE.Window.previousScreen.height));
    }

    CORE.Window.fullscreen = !CORE.Window.fullscreen;
}

// Set window state: maximized, if resizable
void MaximizeWindow(void)
{
    RGFW_window_maximize(platform.window);
}

// Set window state: minimized
void MinimizeWindow(void)
{
    RGFW_window_minimize(platform.window);
}

// Restore window from being minimized/maximized
void RestoreWindow(void)
{
    if (!(CORE.Window.flags & FLAG_WINDOW_UNFOCUSED)) RGFW_window_focus(platform.window);

    RGFW_window_restore(platform.window);
}

// Set window configuration state using flags
void SetWindowState(unsigned int flags)
{
    if (!CORE.Window.ready) TRACELOG(LOG_WARNING, "WINDOW: SetWindowState does nothing before window initialization, Use \"SetConfigFlags\" instead");

    CORE.Window.flags |= flags;

    if (flags & FLAG_VSYNC_HINT)
    {
        RGFW_window_swapInterval(platform.window, 1);
    }
    if (flags & FLAG_FULLSCREEN_MODE)
    {
        if (!CORE.Window.fullscreen) ToggleFullscreen();
    }
    if (flags & FLAG_WINDOW_RESIZABLE)
    {
        RGFW_window_setMaxSize(platform.window, RGFW_AREA(0, 0));
        RGFW_window_setMinSize(platform.window, RGFW_AREA(0, 0));
    }
    if (flags & FLAG_WINDOW_UNDECORATED)
    {
        RGFW_window_setBorder(platform.window, 0);
    }
    if (flags & FLAG_WINDOW_HIDDEN)
    {
        RGFW_window_hide(platform.window);
    }
    if (flags & FLAG_WINDOW_MINIMIZED)
    {
        RGFW_window_minimize(platform.window);
    }
    if (flags & FLAG_WINDOW_MAXIMIZED)
    {
        RGFW_window_maximize(platform.window);
    }
    if (flags & FLAG_WINDOW_UNFOCUSED)
    {
        CORE.Window.flags |= FLAG_WINDOW_UNFOCUSED;
        platform.window->_flags &= ~RGFW_windowFocusOnShow;
        RGFW_window_setFlags(platform.window, platform.window->_flags);
    }
    if (flags & FLAG_WINDOW_TOPMOST)
    {
        RGFW_window_setFloating(platform.window, RGFW_TRUE);
    }
    if (flags & FLAG_WINDOW_ALWAYS_RUN)
    {
        CORE.Window.flags |= FLAG_WINDOW_ALWAYS_RUN;
    }
    if (flags & FLAG_WINDOW_TRANSPARENT)
    {
        TRACELOG(LOG_WARNING, "WINDOW: Framebuffer transparency can only be configured before window initialization");
    }
    if (flags & FLAG_WINDOW_HIGHDPI)
    {
        TRACELOG(LOG_WARNING, "WINDOW: High DPI can only be configured before window initialization");
    }
    if (flags & FLAG_WINDOW_MOUSE_PASSTHROUGH)
    {
        RGFW_window_setMousePassthrough(platform.window, 1);
    }
    if (flags & FLAG_BORDERLESS_WINDOWED_MODE)
    {
        ToggleBorderlessWindowed();
    }
    if (flags & FLAG_MSAA_4X_HINT)
    {
        RGFW_setGLHint(RGFW_glSamples, 4);
    }
    if (flags & FLAG_INTERLACED_HINT)
    {
        TRACELOG(LOG_WARNING, "RPI: Interlaced mode can only be configured before window initialization");
    }
}

// Clear window configuration state flags
void ClearWindowState(unsigned int flags)
{
    CORE.Window.flags &= ~flags;

    if (flags & FLAG_VSYNC_HINT)
    {
        RGFW_window_swapInterval(platform.window, 0);
    }
    if (flags & FLAG_FULLSCREEN_MODE)
    {
        if (CORE.Window.fullscreen) ToggleFullscreen();
    }
    if (flags & FLAG_WINDOW_RESIZABLE)
    {
        RGFW_window_setMaxSize(platform.window, RGFW_AREA(platform.window->r.w, platform.window->r.h));
        RGFW_window_setMinSize(platform.window, RGFW_AREA(platform.window->r.w, platform.window->r.h));
    }
    if (flags & FLAG_WINDOW_UNDECORATED)
    {
        RGFW_window_setBorder(platform.window, 1);
    }
    if (flags & FLAG_WINDOW_HIDDEN)
    {
        if (!(CORE.Window.flags & FLAG_WINDOW_UNFOCUSED)) RGFW_window_focus(platform.window);

        RGFW_window_show(platform.window);
    }
    if (flags & FLAG_WINDOW_MINIMIZED)
    {
        if (!(CORE.Window.flags & FLAG_WINDOW_UNFOCUSED)) RGFW_window_focus(platform.window);

        RGFW_window_restore(platform.window);
    }
    if (flags & FLAG_WINDOW_MAXIMIZED)
    {
        if (!(CORE.Window.flags & FLAG_WINDOW_UNFOCUSED)) RGFW_window_focus(platform.window);

        RGFW_window_restore(platform.window);
    }
    if (flags & FLAG_WINDOW_UNFOCUSED)
    {
        RGFW_window_setFlags(platform.window, platform.window->_flags | RGFW_windowFocusOnShow);
        CORE.Window.flags &= ~FLAG_WINDOW_UNFOCUSED;
    }
    if (flags & FLAG_WINDOW_TOPMOST)
    {
        RGFW_window_setFloating(platform.window, RGFW_FALSE);
    }
    if (flags & FLAG_WINDOW_ALWAYS_RUN)
    {
        CORE.Window.flags &= ~FLAG_WINDOW_ALWAYS_RUN;
    }
    if (flags & FLAG_WINDOW_TRANSPARENT)
    {
        TRACELOG(LOG_WARNING, "WINDOW: Framebuffer transparency can only be configured before window initialization");
    }
    if (flags & FLAG_WINDOW_HIGHDPI)
    {
        TRACELOG(LOG_WARNING, "WINDOW: High DPI can only be configured before window initialization");
    }
    if (flags & FLAG_WINDOW_MOUSE_PASSTHROUGH)
    {
        RGFW_window_setMousePassthrough(platform.window, 0);
    }
    if (flags & FLAG_BORDERLESS_WINDOWED_MODE)
    {
        if (CORE.Window.fullscreen) ToggleBorderlessWindowed();
    }
    if (flags & FLAG_MSAA_4X_HINT)
    {
        RGFW_setGLHint(RGFW_glSamples, 0);
    }
    if (flags & FLAG_INTERLACED_HINT)
    {
        TRACELOG(LOG_WARNING, "RPI: Interlaced mode can only be configured before window initialization");
    }
}

int RGFW_formatToChannels(int format)
{
    switch (format)
    {
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
        case PIXELFORMAT_UNCOMPRESSED_R16:           // 16 bpp (1 channel - half float)
        case PIXELFORMAT_UNCOMPRESSED_R32:           // 32 bpp (1 channel - float)
            return 1;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:    // 8*2 bpp (2 channels)
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5:        // 16 bpp
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8:        // 24 bpp
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:      // 16 bpp (1 bit alpha)
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:      // 16 bpp (4 bit alpha)
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:      // 32 bpp
            return 2;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32:     // 32*3 bpp (3 channels - float)
        case PIXELFORMAT_UNCOMPRESSED_R16G16B16:     // 16*3 bpp (3 channels - half float)
        case PIXELFORMAT_COMPRESSED_DXT1_RGB:        // 4 bpp (no alpha)
        case PIXELFORMAT_COMPRESSED_ETC1_RGB:        // 4 bpp
        case PIXELFORMAT_COMPRESSED_ETC2_RGB:        // 4 bpp
        case PIXELFORMAT_COMPRESSED_PVRT_RGB:        // 4 bpp
            return 3;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:  // 32*4 bpp (4 channels - float)
        case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:  // 16*4 bpp (4 channels - half float)
        case PIXELFORMAT_COMPRESSED_DXT1_RGBA:       // 4 bpp (1 bit alpha)
        case PIXELFORMAT_COMPRESSED_DXT3_RGBA:       // 8 bpp
        case PIXELFORMAT_COMPRESSED_DXT5_RGBA:       // 8 bpp
        case PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA:   // 8 bpp
        case PIXELFORMAT_COMPRESSED_PVRT_RGBA:       // 4 bpp
        case PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA:   // 8 bpp
        case PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA:   // 2 bpp
            return 4;
        default: return 4;
    }
}

// Set icon for window
void SetWindowIcon(Image image)
{
    RGFW_window_setIcon(platform.window, image.data, RGFW_AREA(image.width, image.height), RGFW_formatToChannels(image.format));
}

// Set icon for window
void SetWindowIcons(Image *images, int count)
{
    if ((images == NULL) || (count <= 0))
    {
        RGFW_window_setIcon(platform.window, NULL, RGFW_AREA(0, 0), 0);
    }
    else
    {
        Image *bigIcon = NULL;
        Image *smallIcon = NULL;

        for (int i = 0; i < count; i++)
        {
            if ((bigIcon == NULL) || ((images[i].width > bigIcon->width) && (images[i].height > bigIcon->height))) bigIcon = &images[i];
            if ((smallIcon == NULL) || ((images[i].width < smallIcon->width) && (images[i].height > smallIcon->height))) smallIcon = &images[i];
        }

        if (smallIcon != NULL) RGFW_window_setIconEx(platform.window, smallIcon->data, RGFW_AREA(smallIcon->width, smallIcon->height), RGFW_formatToChannels(smallIcon->format), RGFW_iconWindow);
        if (bigIcon != NULL) RGFW_window_setIconEx(platform.window, bigIcon->data, RGFW_AREA(bigIcon->width, bigIcon->height), RGFW_formatToChannels(bigIcon->format), RGFW_iconTaskbar);
    }
}

// Set title for window
void SetWindowTitle(const char *title)
{
    RGFW_window_setName(platform.window, (char *)title);
    CORE.Window.title = title;
}

// Set window position on screen (windowed mode)
void SetWindowPosition(int x, int y)
{
    RGFW_window_move(platform.window, RGFW_POINT(x, y));
}

// Set monitor for the current window
void SetWindowMonitor(int monitor)
{
    RGFW_window_moveToMonitor(platform.window, RGFW_getMonitors(NULL)[monitor]);
}

// Set window minimum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMinSize(int width, int height)
{
    RGFW_window_setMinSize(platform.window, RGFW_AREA(width, height));
    CORE.Window.screenMin.width = width;
    CORE.Window.screenMin.height = height;
}

// Set window maximum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMaxSize(int width, int height)
{
    RGFW_window_setMaxSize(platform.window, RGFW_AREA(width, height));
    CORE.Window.screenMax.width = width;
    CORE.Window.screenMax.height = height;
}

// Set window dimensions
void SetWindowSize(int width, int height)
{
    CORE.Window.screen.width = width;
    CORE.Window.screen.height = height;

    RGFW_window_resize(platform.window, RGFW_AREA(width, height));
}

// Set window opacity, value opacity is between 0.0 and 1.0
void SetWindowOpacity(float opacity)
{
    RGFW_window_setOpacity(platform.window, opacity);
}

// Set window focused
void SetWindowFocused(void)
{
    RGFW_window_focus(platform.window);
}

// Get native window handle
void *GetWindowHandle(void)
{
    if (platform.window == NULL) return NULL;
#ifdef RGFW_WASM
    return (void *)platform.window->src.ctx;
#else
    return (void *)platform.window->src.window;
#endif
}

// Get number of monitors
int GetMonitorCount(void)
{
    #define MAX_MONITORS_SUPPORTED 6

    int count = MAX_MONITORS_SUPPORTED;
    RGFW_monitor *mons = RGFW_getMonitors(NULL);

    for (int i = 0; i < 6; i++)
    {
        if (!mons[i].x && !mons[i].y && !mons[i].mode.area.w && mons[i].mode.area.h)
        {
            count = i;
            break;
        }
    }

    return count;
}

// Get current monitor where window is placed
int GetCurrentMonitor(void)
{
    RGFW_monitor *mons = RGFW_getMonitors(NULL);
    RGFW_monitor mon = { 0 };

    if (platform.window) mon = RGFW_window_getMonitor(platform.window);
    else mon = RGFW_getPrimaryMonitor();

    for (int i = 0; i < 6; i++)
    {
        if ((mons[i].x ==  mon.x) && (mons[i].y ==  mon.y)) return i;
    }

    return 0;
}

// Get selected monitor position
Vector2 GetMonitorPosition(int monitor)
{
    RGFW_monitor *mons = RGFW_getMonitors(NULL);

    return (Vector2){ (float)mons[monitor].x, (float)mons[monitor].y };
}

// Get selected monitor width (currently used by monitor)
int GetMonitorWidth(int monitor)
{
    RGFW_monitor *mons = RGFW_getMonitors(NULL);

    return mons[monitor].mode.area.w;
}

// Get selected monitor height (currently used by monitor)
int GetMonitorHeight(int monitor)
{
    RGFW_monitor *mons = RGFW_getMonitors(NULL);

    return mons[monitor].mode.area.h;
}

// Get selected monitor physical width in millimetres
int GetMonitorPhysicalWidth(int monitor)
{
    RGFW_monitor *mons = RGFW_getMonitors(NULL);

    return mons[monitor].physW;
}

// Get selected monitor physical height in millimetres
int GetMonitorPhysicalHeight(int monitor)
{
    RGFW_monitor *mons = RGFW_getMonitors(NULL);

    return (int)mons[monitor].physH;
}

// Get selected monitor refresh rate
int GetMonitorRefreshRate(int monitor)
{
    RGFW_monitor *mons = RGFW_getMonitors(NULL);

    return (int)mons[monitor].mode.refreshRate;
}

// Get the human-readable, UTF-8 encoded name of the selected monitor
const char *GetMonitorName(int monitor)
{
    RGFW_monitor *mons = RGFW_getMonitors(NULL);

    return mons[monitor].name;
}

// Get window position XY on monitor
Vector2 GetWindowPosition(void)
{
    if (platform.window == NULL) return (Vector2){ 0.0f, 0.0f };
    return (Vector2){ (float)platform.window->r.x, (float)platform.window->r.y };
}

// Get window scale DPI factor for current monitor
Vector2 GetWindowScaleDPI(void)
{
    RGFW_monitor monitor = { 0 };

    if (platform.window) monitor = RGFW_window_getMonitor(platform.window);
    else monitor = RGFW_getPrimaryMonitor();

    return (Vector2){ monitor.scaleX, monitor.scaleX };
}

// Set clipboard text content
void SetClipboardText(const char *text)
{
    RGFW_writeClipboard(text, strlen(text));
}

// Get clipboard text content
// NOTE: returned string is allocated and freed by RGFW
const char *GetClipboardText(void)
{
    return RGFW_readClipboard(NULL);
}

#if defined(SUPPORT_CLIPBOARD_IMAGE)
#if defined(_WIN32)
    #define WIN32_CLIPBOARD_IMPLEMENTATION
    #define WINUSER_ALREADY_INCLUDED
    #define WINBASE_ALREADY_INCLUDED
    #define WINGDI_ALREADY_INCLUDED
    #include "../external/win32_clipboard.h"
#endif
#endif

// Get clipboard image
Image GetClipboardImage(void)
{
    Image image = { 0 };
    unsigned long long int dataSize = 0;
    void *fileData = NULL;

#if defined(SUPPORT_CLIPBOARD_IMAGE)
#if defined(_WIN32)
    int width = 0;
    int height = 0;
    fileData  = (void *)Win32GetClipboardImageData(&width, &height, &dataSize);

    if (fileData == NULL) TRACELOG(LOG_WARNING, "Clipboard image: Couldn't get clipboard data");
    else image = LoadImageFromMemory(".bmp", fileData, dataSize);
#else
    TRACELOG(LOG_WARNING, "Clipboard image: PLATFORM_DESKTOP_RGFW doesn't implement GetClipboardImage() for this OS");
#endif
#endif // SUPPORT_CLIPBOARD_IMAGE

    return image;
}

// Show mouse cursor
void ShowCursor(void)
{
    RGFW_window_showMouse(platform.window, true);
    CORE.Input.Mouse.cursorHidden = false;
}

// Hides mouse cursor
void HideCursor(void)
{
    RGFW_window_showMouse(platform.window, false);
    CORE.Input.Mouse.cursorHidden = true;
}

// Enables cursor (unlock cursor)
void EnableCursor(void)
{
    RGFW_disableCursor = false;
    RGFW_window_mouseUnhold(platform.window);

    // Set cursor position in the middle
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);
    RGFW_window_showMouse(platform.window, true);
    CORE.Input.Mouse.cursorHidden = false;
}

// Disables cursor (lock cursor)
void DisableCursor(void)
{
    RGFW_disableCursor = true;
    RGFW_window_mouseHold(platform.window, RGFW_AREA(0, 0));
    HideCursor();
}

// Swap back buffer with front buffer (screen drawing)
void SwapScreenBuffer(void)
{
    RGFW_window_swapBuffers(platform.window);
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Misc
//----------------------------------------------------------------------------------

// Get elapsed time measure in seconds since InitTimer()
double GetTime(void)
{
    return RGFW_getTime();
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
    else
    {
        // TODO: Open URL implementation
    }
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Inputs
//----------------------------------------------------------------------------------

// Set internal gamepad mappings
int SetGamepadMappings(const char *mappings)
{
    TRACELOG(LOG_WARNING, "SetGamepadMappings() unsupported on target platform");
    return 0;
}

// Set gamepad vibration
void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor, float duration)
{
    TRACELOG(LOG_WARNING, "SetGamepadVibration() unsupported on target platform");
}

// Set mouse position XY
void SetMousePosition(int x, int y)
{
    RGFW_window_moveMouse(platform.window, RGFW_POINT(x, y));
    CORE.Input.Mouse.currentPosition = (Vector2){ (float)x, (float)y };
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
}

// Set mouse cursor
void SetMouseCursor(int cursor)
{
    RGFW_window_setMouseStandard(platform.window, cursor);
}

// Get physical key name.
const char *GetKeyName(int key)
{
    TRACELOG(LOG_WARNING, "GetKeyName() unsupported on target platform");

    return "";
}

static KeyboardKey ConvertScancodeToKey(u32 keycode);

int RGFW_gpConvTable[18] = {
    [RGFW_gamepadY] = GAMEPAD_BUTTON_RIGHT_FACE_UP,
    [RGFW_gamepadB] = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,
    [RGFW_gamepadA] = GAMEPAD_BUTTON_RIGHT_FACE_DOWN,
    [RGFW_gamepadX] = GAMEPAD_BUTTON_RIGHT_FACE_LEFT,
    [RGFW_gamepadL1] = GAMEPAD_BUTTON_LEFT_TRIGGER_1,
    [RGFW_gamepadR1] = GAMEPAD_BUTTON_RIGHT_TRIGGER_1,
    [RGFW_gamepadL2] = GAMEPAD_BUTTON_LEFT_TRIGGER_2,
    [RGFW_gamepadR2] = GAMEPAD_BUTTON_RIGHT_TRIGGER_2,
    [RGFW_gamepadSelect] = GAMEPAD_BUTTON_MIDDLE_LEFT,
    [RGFW_gamepadHome] = GAMEPAD_BUTTON_MIDDLE,
    [RGFW_gamepadStart] = GAMEPAD_BUTTON_MIDDLE_RIGHT,
    [RGFW_gamepadUp] = GAMEPAD_BUTTON_LEFT_FACE_UP,
    [RGFW_gamepadRight] = GAMEPAD_BUTTON_LEFT_FACE_RIGHT,
    [RGFW_gamepadDown] = GAMEPAD_BUTTON_LEFT_FACE_DOWN,
    [RGFW_gamepadLeft] = GAMEPAD_BUTTON_LEFT_FACE_LEFT,
    [RGFW_gamepadL3] = GAMEPAD_BUTTON_LEFT_THUMB,    
    [RGFW_gamepadR3] = GAMEPAD_BUTTON_RIGHT_THUMB,
};

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

    // Reset mouse wheel
    CORE.Input.Mouse.currentWheelMove.x = 0;
    CORE.Input.Mouse.currentWheelMove.y = 0;

    // Register previous mouse position

    // Reset last gamepad button/axis registered state
    for (int i = 0; (i < 4) && (i < MAX_GAMEPADS); i++)
    {
        // Check if gamepad is available
        if (CORE.Input.Gamepad.ready[i])
        {
            // Register previous gamepad button states
            for (int k = 0; k < MAX_GAMEPAD_BUTTONS; k++)
            {
                CORE.Input.Gamepad.previousButtonState[i][k] = CORE.Input.Gamepad.currentButtonState[i][k];
            }
        }
    }

    // Register previous touch states
    for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.previousTouchState[i] = CORE.Input.Touch.currentTouchState[i];

    // Map touch position to mouse position for convenience
    CORE.Input.Touch.position[0] = CORE.Input.Mouse.currentPosition;

    int touchAction = -1;       // 0-TOUCH_ACTION_UP, 1-TOUCH_ACTION_DOWN, 2-TOUCH_ACTION_MOVE
    bool realTouch = false;     // Flag to differentiate real touch gestures from mouse ones

    // Register previous keys states
    // NOTE: Android supports up to 260 keys
    for (int i = 0; i < MAX_KEYBOARD_KEYS; i++)
    {
        CORE.Input.Keyboard.previousKeyState[i] = CORE.Input.Keyboard.currentKeyState[i];
        CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }

    // Register previous mouse states
    for (int i = 0; i < MAX_MOUSE_BUTTONS; i++) CORE.Input.Mouse.previousButtonState[i] = CORE.Input.Mouse.currentButtonState[i];

    // Poll input events for current platform
    //-----------------------------------------------------------------------------
    CORE.Window.resizedLastFrame = false;

    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
    if (platform.window->_flags & RGFW_HOLD_MOUSE)
    {
        CORE.Input.Mouse.previousPosition = (Vector2){ 0.0f, 0.0f };
        CORE.Input.Mouse.currentPosition = (Vector2){ 0.0f, 0.0f };
    }
    else
    {
        CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
    }

    if ((CORE.Window.eventWaiting) || (IsWindowState(FLAG_WINDOW_MINIMIZED) && !IsWindowState(FLAG_WINDOW_ALWAYS_RUN)))
    {
        RGFW_window_eventWait(platform.window, -1); // Wait for input events: keyboard/mouse/window events (callbacks) -> Update keys state
        CORE.Time.previous = GetTime();
    }

    while (RGFW_window_checkEvent(platform.window))
    {
        RGFW_event *event = &platform.window->event;
        // All input events can be processed after polling

        switch (event->type)
        {
            case RGFW_mouseEnter: CORE.Input.Mouse.cursorOnScreen = true; break;
            case RGFW_mouseLeave: CORE.Input.Mouse.cursorOnScreen = false; break;
            case RGFW_quit:
                event->type = 0;
                CORE.Window.shouldClose = true;
                return;
            case RGFW_DND:      // Dropped file
            {
                for (int i = 0; i < event->droppedFilesCount; i++)
                {
                    if (CORE.Window.dropFileCount == 0)
                    {
                        // When a new file is dropped, we reserve a fixed number of slots for all possible dropped files
                        // at the moment we limit the number of drops at once to 1024 files but this behaviour should probably be reviewed
                        // TODO: Pointers should probably be reallocated for any new file added...
                        CORE.Window.dropFilepaths = (char **)RL_CALLOC(1024, sizeof(char *));

                        CORE.Window.dropFilepaths[CORE.Window.dropFileCount] = (char *)RL_CALLOC(MAX_FILEPATH_LENGTH, sizeof(char));
                        strcpy(CORE.Window.dropFilepaths[CORE.Window.dropFileCount], event->droppedFiles[i]);
                        
                        CORE.Window.dropFileCount++;
                    }
                    else if (CORE.Window.dropFileCount < 1024)
                    {
                        CORE.Window.dropFilepaths[CORE.Window.dropFileCount] = (char *)RL_CALLOC(MAX_FILEPATH_LENGTH, sizeof(char));
                        strcpy(CORE.Window.dropFilepaths[CORE.Window.dropFileCount], event->droppedFiles[i]);

                        CORE.Window.dropFileCount++;
                    }
                    else TRACELOG(LOG_WARNING, "FILE: Maximum drag and drop files at once is limited to 1024 files!");
                }
            } break;

            // Window events are also polled (Minimized, maximized, close...)
            case RGFW_windowResized:
            {
                SetupViewport(platform.window->r.w, platform.window->r.h);

                // if we are doing automatic DPI scaling, then the "screen" size is divided by the window scale
                if (IsWindowState(FLAG_WINDOW_HIGHDPI))
                {
                    CORE.Window.screen.width = (int)(platform.window->r.w/GetWindowScaleDPI().x);
                    CORE.Window.screen.height = (int)(platform.window->r.h/GetWindowScaleDPI().y);
                }
                else
                {
                    CORE.Window.screen.width = platform.window->r.w;
                    CORE.Window.screen.height = platform.window->r.h;
                }

                CORE.Window.currentFbo.width = platform.window->r.w;
                CORE.Window.currentFbo.height = platform.window->r.h;
                CORE.Window.resizedLastFrame = true;
            } break;
            case RGFW_windowMaximized:
            {
                CORE.Window.flags |= FLAG_WINDOW_MAXIMIZED;  // The window was maximized
            } break;
            case RGFW_windowMinimized:
            {
                CORE.Window.flags |= FLAG_WINDOW_MINIMIZED;  // The window was iconified
            } break;
            case RGFW_windowRestored:
            {
                if (RGFW_window_isMaximized(platform.window))
                    CORE.Window.flags &= ~FLAG_WINDOW_MAXIMIZED;           // The window was restored
                if (RGFW_window_isMinimized(platform.window))
                    CORE.Window.flags &= ~FLAG_WINDOW_MINIMIZED;           // The window was restored
            } break;
            case RGFW_windowMoved:
            {
                CORE.Window.position.x = platform.window->r.x;
                CORE.Window.position.y = platform.window->r.x;
            } break;

            // Keyboard events
            case RGFW_keyPressed:
            {
                KeyboardKey key = ConvertScancodeToKey(event->key);
                if (key != KEY_NULL)
                {
                    // If key was up, add it to the key pressed queue
                    if ((CORE.Input.Keyboard.currentKeyState[key] == 0) && (CORE.Input.Keyboard.keyPressedQueueCount < MAX_KEY_PRESSED_QUEUE))
                    {
                        CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = key;
                        CORE.Input.Keyboard.keyPressedQueueCount++;
                    }

                    CORE.Input.Keyboard.currentKeyState[key] = 1;
                }

                // TODO: Put exitKey verification outside the switch?
                if (CORE.Input.Keyboard.currentKeyState[CORE.Input.Keyboard.exitKey])
                {
                    CORE.Window.shouldClose = true;
                }

                // NOTE: event.text.text data comes an UTF-8 text sequence but we register codepoints (int)
                // Check if there is space available in the queue
                if (CORE.Input.Keyboard.charPressedQueueCount < MAX_CHAR_PRESSED_QUEUE)
                {
                    // Add character (codepoint) to the queue
                    CORE.Input.Keyboard.charPressedQueue[CORE.Input.Keyboard.charPressedQueueCount] = event->keyChar;
                    CORE.Input.Keyboard.charPressedQueueCount++;
                }
            } break;
            case RGFW_keyReleased:
            {
                KeyboardKey key = ConvertScancodeToKey(event->key);
                if (key != KEY_NULL) CORE.Input.Keyboard.currentKeyState[key] = 0;
            } break;

            // Check mouse events
            case RGFW_mouseButtonPressed:
            {
                if ((event->button == RGFW_mouseScrollUp) || (event->button == RGFW_mouseScrollDown))
                {
                    CORE.Input.Mouse.currentWheelMove.y = event->scroll;
                    break;
                }
                else CORE.Input.Mouse.currentWheelMove.y = 0;

                int btn = event->button;
                if (btn == RGFW_mouseLeft) btn = 1;
                else if (btn == RGFW_mouseRight) btn = 2;
                else if (btn == RGFW_mouseMiddle) btn = 3;

                CORE.Input.Mouse.currentButtonState[btn - 1] = 1;
                CORE.Input.Touch.currentTouchState[btn - 1] = 1;

                touchAction = 1;
            } break;
            case RGFW_mouseButtonReleased:
            {
                if ((event->button == RGFW_mouseScrollUp) || (event->button == RGFW_mouseScrollDown))
                {
                    CORE.Input.Mouse.currentWheelMove.y = event->scroll;
                    break;
                }
                else CORE.Input.Mouse.currentWheelMove.y = 0;

                int btn = event->button;
                if (btn == RGFW_mouseLeft) btn = 1;
                else if (btn == RGFW_mouseRight) btn = 2;
                else if (btn == RGFW_mouseMiddle) btn = 3;

                CORE.Input.Mouse.currentButtonState[btn - 1] = 0;
                CORE.Input.Touch.currentTouchState[btn - 1] = 0;

                touchAction = 0;
            } break;
            case RGFW_mousePosChanged:
            {
                if (platform.window->_flags & RGFW_HOLD_MOUSE)
                {
                    CORE.Input.Mouse.currentPosition.x += (float)event->vector.x;
                    CORE.Input.Mouse.currentPosition.y += (float)event->vector.y;
                }
                else
                {
                    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
                    CORE.Input.Mouse.currentPosition.x = (float)event->point.x;
                    CORE.Input.Mouse.currentPosition.y = (float)event->point.y;
                }

                CORE.Input.Touch.position[0] = CORE.Input.Mouse.currentPosition;
                touchAction = 2;
            } break;
            case RGFW_gamepadConnected:
            {
                CORE.Input.Gamepad.ready[platform.window->event.gamepad] = true;
                CORE.Input.Gamepad.axisCount[platform.window->event.gamepad] = platform.window->event.axisesCount;
                CORE.Input.Gamepad.axisState[platform.window->event.gamepad][GAMEPAD_AXIS_LEFT_TRIGGER] = -1.0f;
                CORE.Input.Gamepad.axisState[platform.window->event.gamepad][GAMEPAD_AXIS_RIGHT_TRIGGER] = -1.0f;

                strcpy(CORE.Input.Gamepad.name[platform.window->event.gamepad], RGFW_getGamepadName(platform.window, platform.window->event.gamepad));
            } break;
            case RGFW_gamepadDisconnected:
            {
                CORE.Input.Gamepad.ready[platform.window->event.gamepad] = false;
            } break;
            case RGFW_gamepadButtonPressed:
            {
                int button = RGFW_gpConvTable[event->button];

                if (button >= 0)
                {
                    CORE.Input.Gamepad.currentButtonState[event->gamepad][button] = 1;
                    CORE.Input.Gamepad.lastButtonPressed = button;
                }
            } break;
            case RGFW_gamepadButtonReleased:
            {
                int button = RGFW_gpConvTable[event->button];

                CORE.Input.Gamepad.currentButtonState[event->gamepad][button] = 0;
                if (CORE.Input.Gamepad.lastButtonPressed == button) CORE.Input.Gamepad.lastButtonPressed = 0;
            } break;
            case RGFW_gamepadAxisMove:
            {
                int axis = -1;
                float value = 0;

                switch(event->whichAxis)
                {
                    case 0:
                    {
                        CORE.Input.Gamepad.axisState[event->gamepad][GAMEPAD_AXIS_LEFT_X] = event->axis[0].x/100.0f;
                        CORE.Input.Gamepad.axisState[event->gamepad][GAMEPAD_AXIS_LEFT_Y] = event->axis[0].y/100.0f;
                    } break;
                    case 1:
                    {
                        CORE.Input.Gamepad.axisState[event->gamepad][GAMEPAD_AXIS_RIGHT_X] = event->axis[1].x/100.0f;
                        CORE.Input.Gamepad.axisState[event->gamepad][GAMEPAD_AXIS_RIGHT_Y] = event->axis[1].y/100.0f;
                    } break;
                    case 2: axis = GAMEPAD_AXIS_LEFT_TRIGGER;
                    case 3:
                    {
                        if (axis == -1) axis = GAMEPAD_AXIS_RIGHT_TRIGGER;

                        int button = (axis == GAMEPAD_AXIS_LEFT_TRIGGER)? GAMEPAD_BUTTON_LEFT_TRIGGER_2 : GAMEPAD_BUTTON_RIGHT_TRIGGER_2;
                        int pressed = (value > 0.1f);
                        CORE.Input.Gamepad.currentButtonState[event->gamepad][button] = pressed;
                        
                        if (pressed) CORE.Input.Gamepad.lastButtonPressed = button;
                        else if (CORE.Input.Gamepad.lastButtonPressed == button) CORE.Input.Gamepad.lastButtonPressed = 0;
                    }
                    default: break;
                }
            } break;
            default: break;
        }

#if defined(SUPPORT_GESTURES_SYSTEM)
        if (touchAction > -1)
        {
            // Process mouse events as touches to be able to use mouse-gestures
            GestureEvent gestureEvent = { 0 };

            // Register touch actions
            gestureEvent.touchAction = touchAction;

            // Assign a pointer ID
            gestureEvent.pointId[0] = 0;

            // Register touch points count
            gestureEvent.pointCount = 1;

            // Register touch points position, only one point registered
            if (touchAction == 2 || realTouch) gestureEvent.position[0] = CORE.Input.Touch.position[0];
            else gestureEvent.position[0] = GetMousePosition();

            // Normalize gestureEvent.position[0] for CORE.Window.screen.width and CORE.Window.screen.height
            gestureEvent.position[0].x /= (float)GetScreenWidth();
            gestureEvent.position[0].y /= (float)GetScreenHeight();

            // Gesture data is sent to gestures-system for processing
            ProcessGestureEvent(gestureEvent);

            touchAction = -1;
        }
#endif
    }
    //-----------------------------------------------------------------------------
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

// Initialize platform: graphics, inputs and more
int InitPlatform(void)
{
    // Initialize RGFW internal global state, only required systems
    unsigned int flags = RGFW_windowCenter | RGFW_windowAllowDND;

    // Check window creation flags
    if ((CORE.Window.flags & FLAG_FULLSCREEN_MODE) > 0)
    {
        CORE.Window.fullscreen = true;
        flags |= RGFW_windowFullscreen;
    }

    if ((CORE.Window.flags & FLAG_BORDERLESS_WINDOWED_MODE) > 0)
    {
        CORE.Window.fullscreen = true;
        flags |= RGFW_windowedFullscreen;
    }

    if ((CORE.Window.flags & FLAG_WINDOW_UNDECORATED) > 0) flags |= RGFW_windowNoBorder;
    if ((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) == 0) flags |= RGFW_windowNoResize;
    if ((CORE.Window.flags & FLAG_WINDOW_TRANSPARENT) > 0) flags |= RGFW_windowTransparent;
    if ((CORE.Window.flags & FLAG_FULLSCREEN_MODE) > 0) flags |= RGFW_windowFullscreen;
    if ((CORE.Window.flags & FLAG_WINDOW_HIDDEN) > 0) flags |= RGFW_windowHide;
    if ((CORE.Window.flags & FLAG_WINDOW_MAXIMIZED) > 0) flags |= RGFW_windowMaximize;

    // NOTE: Some OpenGL context attributes must be set before window creation
    // Check selection OpenGL version
    if (rlGetVersion() == RL_OPENGL_21)
    {
        RGFW_setGLHint(RGFW_glMajor, 2);
        RGFW_setGLHint(RGFW_glMinor, 1);
    }
    else if (rlGetVersion() == RL_OPENGL_33)
    {
        RGFW_setGLHint(RGFW_glMajor, 3);
        RGFW_setGLHint(RGFW_glMinor, 3);
    }
    else if (rlGetVersion() == RL_OPENGL_43)
    {
        RGFW_setGLHint(RGFW_glMajor, 4);
        RGFW_setGLHint(RGFW_glMinor, 3);
    }

    if (CORE.Window.flags & FLAG_MSAA_4X_HINT) RGFW_setGLHint(RGFW_glSamples, 4);

    if (!(CORE.Window.flags & FLAG_WINDOW_UNFOCUSED)) flags |= RGFW_windowFocusOnShow | RGFW_windowFocus;

    platform.window = RGFW_createWindow(CORE.Window.title, RGFW_RECT(0, 0, CORE.Window.screen.width, CORE.Window.screen.height), flags);
    platform.mon.mode.area.w = 0;

    if (platform.window != NULL)
    {
        // NOTE: RGFW's exit key is distinct from raylib's exit key (which can
        // be set with SetExitKey()) and defaults to Escape
        platform.window->exitKey = RGFW_keyNULL;
    }

#ifndef PLATFORM_WEB_RGFW
    RGFW_area screenSize = RGFW_getScreenSize();
    CORE.Window.display.width = screenSize.w;
    CORE.Window.display.height = screenSize.h;
#else
    CORE.Window.display.width = CORE.Window.screen.width;
    CORE.Window.display.height = CORE.Window.screen.height;
#endif
    // TODO: Is this needed by raylib now?
    // If so, rcore_desktop_sdl should be updated too
    //SetupFramebuffer(CORE.Window.display.width, CORE.Window.display.height);
    
    if (CORE.Window.flags & FLAG_VSYNC_HINT) RGFW_window_swapInterval(platform.window, 1);
    RGFW_window_makeCurrent(platform.window);

    // Check surface and context activation
    if (platform.window != NULL)
    {
        CORE.Window.ready = true;

        CORE.Window.render.width = CORE.Window.screen.width;
        CORE.Window.render.height = CORE.Window.screen.height;
        CORE.Window.currentFbo.width = CORE.Window.render.width;
        CORE.Window.currentFbo.height = CORE.Window.render.height;
    }
    else
    {
        TRACELOG(LOG_FATAL, "PLATFORM: Failed to initialize graphics device");
        return -1;
    }
    //----------------------------------------------------------------------------

    // If everything work as expected, we can continue
    CORE.Window.position.x = platform.window->r.x;
    CORE.Window.position.y = platform.window->r.y;
    CORE.Window.render.width = CORE.Window.screen.width;
    CORE.Window.render.height = CORE.Window.screen.height;
    CORE.Window.currentFbo.width = CORE.Window.render.width;
    CORE.Window.currentFbo.height = CORE.Window.render.height;

    TRACELOG(LOG_INFO, "DISPLAY: Device initialized successfully");
    TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
    TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
    TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
    TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);

    // Load OpenGL extensions
    // NOTE: GL procedures address loader is required to load extensions
    //----------------------------------------------------------------------------
    rlLoadExtensions((void *)RGFW_getProcAddress);
    //----------------------------------------------------------------------------

    // Initialize timing system
    //----------------------------------------------------------------------------
    InitTimer();
    //----------------------------------------------------------------------------

    // Initialize storage system
    //----------------------------------------------------------------------------
    CORE.Storage.basePath = GetWorkingDirectory();
    //----------------------------------------------------------------------------

#if defined(RGFW_WAYLAND)
    if (RGFW_useWaylandBool) TRACELOG(LOG_INFO, "PLATFORM: DESKTOP (RGFW - Wayland): Initialized successfully");
    else TRACELOG(LOG_INFO, "PLATFORM: DESKTOP (RGFW - X11 (fallback)): Initialized successfully");
#elif defined(RGFW_X11)
    #if defined(__APPLE__)
        TRACELOG(LOG_INFO, "PLATFORM: DESKTOP (RGFW - X11 (MacOS)): Initialized successfully");
    #else
        TRACELOG(LOG_INFO, "PLATFORM: DESKTOP (RGFW - X11): Initialized successfully");
    #endif
#elif defined (RGFW_WINDOWS)
    TRACELOG(LOG_INFO, "PLATFORM: DESKTOP (RGFW - Win32): Initialized successfully");
#elif defined(RGFW_WASM)
    TRACELOG(LOG_INFO, "PLATFORM: DESKTOP (RGFW - WASMs): Initialized successfully");
#elif defined(RGFW_MACOS)
    TRACELOG(LOG_INFO, "PLATFORM: DESKTOP (RGFW - MacOS): Initialized successfully");
#endif

    return 0;
}

// Close platform
void ClosePlatform(void)
{
    RGFW_window_close(platform.window);
}

// Keycode mapping
static KeyboardKey ConvertScancodeToKey(u32 keycode)
{
    if (keycode > sizeof(keyMappingRGFW)/sizeof(unsigned short)) return 0;

    return keyMappingRGFW[keycode];
}
