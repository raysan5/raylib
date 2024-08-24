/**********************************************************************************************
*
*   rcore_desktop_rgfw template - Functions to manage window, graphics device and inputs
*
*   PLATFORM: RGFW
*       - Windows (Win32, Win64)
*       - Linux (X11/Wayland desktop mode)
*       - MacOS (Cocoa)
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
*   Copyright (c) 2013-2024 Ramon Santamaria (@raysan5), Colleague Riley and contributors
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

#if defined(GRAPHICS_API_OPENGL_ES2)
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

#if defined(__WIN32) || defined(__WIN64)
    #define WIN32_LEAN_AND_MEAN
    #define Rectangle rectangle_win32
    #define CloseWindow CloseWindow_win32
    #define ShowCursor __imp_ShowCursor
    #define _APISETSTRING_
#endif

#if defined(__APPLE__)
    #define Point NSPOINT
    #define Size NSSIZE
#endif

__declspec(dllimport) int __stdcall MultiByteToWideChar(unsigned int CodePage, unsigned long dwFlags, const char *lpMultiByteStr, int cbMultiByte, wchar_t *lpWideCharStr, int cchWideChar);

#include "../external/RGFW.h"

#if defined(__WIN32) || defined(__WIN64)
    #undef DrawText
    #undef ShowCursor
    #undef CloseWindow
    #undef Rectangle
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
} PlatformData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern CoreData CORE;                   // Global CORE state context

static PlatformData platform = { NULL }; // Platform specific

static bool RGFW_disableCursor = false;

static const unsigned short keyMappingRGFW[] = {
    [RGFW_KEY_NULL] = KEY_NULL,
    [RGFW_Quote] = KEY_APOSTROPHE,
    [RGFW_Comma] = KEY_COMMA,
    [RGFW_Minus] = KEY_MINUS,
    [RGFW_Period] = KEY_PERIOD,
    [RGFW_Slash] = KEY_SLASH,
    [RGFW_Escape] = KEY_ESCAPE,
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
    [RGFW_Backtick] = KEY_GRAVE,
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
    [RGFW_Equals] = KEY_EQUAL,
    [RGFW_BackSpace] = KEY_BACKSPACE,
    [RGFW_Tab] = KEY_TAB,
    [RGFW_CapsLock] = KEY_CAPS_LOCK,
    [RGFW_ShiftL] = KEY_LEFT_SHIFT,
    [RGFW_ControlL] = KEY_LEFT_CONTROL,
    [RGFW_AltL] = KEY_LEFT_ALT,
    [RGFW_SuperL] = KEY_LEFT_SUPER,
    #ifndef RGFW_MACOS
    [RGFW_ShiftR] = KEY_RIGHT_SHIFT,

    [RGFW_AltR] = KEY_RIGHT_ALT,
    #endif
    [RGFW_Space] = KEY_SPACE,

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
    [RGFW_Bracket] = KEY_LEFT_BRACKET,
    [RGFW_BackSlash] = KEY_BACKSLASH,
    [RGFW_CloseBracket] = KEY_RIGHT_BRACKET,
    [RGFW_Semicolon] = KEY_SEMICOLON,
    [RGFW_Insert] = KEY_INSERT,
    [RGFW_Home] = KEY_HOME,
    [RGFW_PageUp] = KEY_PAGE_UP,
    [RGFW_Delete] = KEY_DELETE,
    [RGFW_End] = KEY_END,
    [RGFW_PageDown] = KEY_PAGE_DOWN,
    [RGFW_Right] = KEY_RIGHT,
    [RGFW_Left] = KEY_LEFT,
    [RGFW_Down] = KEY_DOWN,
    [RGFW_Up] = KEY_UP,
    [RGFW_Numlock] = KEY_NUM_LOCK,
    [RGFW_KP_Slash] = KEY_KP_DIVIDE,
    [RGFW_Multiply] = KEY_KP_MULTIPLY,
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
    [RGFW_KP_Period] = KEY_KP_DECIMAL
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
    RGFW_window_maximize(platform.window);
    ToggleBorderlessWindowed();
}

// Toggle borderless windowed mode
void ToggleBorderlessWindowed(void)
{
    if (platform.window != NULL)
    {
        RGFW_window_setBorder(platform.window, CORE.Window.flags & FLAG_WINDOW_UNDECORATED);
    }
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

// Set window state: not minimized/maximized
void RestoreWindow(void)
{
    RGFW_window_restore(platform.window);
}

// Set window configuration state using flags
void SetWindowState(unsigned int flags)
{
    CORE.Window.flags |= flags;

    if (flags & FLAG_VSYNC_HINT)
    {
        RGFW_window_swapInterval(platform.window, 1);
    }
    if (flags & FLAG_FULLSCREEN_MODE)
    {
        RGFW_window_maximize(platform.window);
        ToggleBorderlessWindowed();
    }
    if (flags & FLAG_WINDOW_RESIZABLE)
    {
        printf("%i %i\n", platform.window->r.w, platform.window->r.h);
        RGFW_window_setMaxSize(platform.window, RGFW_AREA(platform.window->r.w, platform.window->r.h));
        RGFW_window_setMinSize(platform.window, RGFW_AREA(platform.window->r.w, platform.window->r.h));
    }
    if (flags & FLAG_WINDOW_UNDECORATED)
    {
        ToggleBorderlessWindowed();
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
        TRACELOG(LOG_WARNING, "SetWindowState() - FLAG_WINDOW_UNFOCUSED is not supported on PLATFORM_DESKTOP_RGFW");
    }
    if (flags & FLAG_WINDOW_TOPMOST)
    {
        TRACELOG(LOG_WARNING, "SetWindowState() - FLAG_WINDOW_TOPMOST is not supported on PLATFORM_DESKTOP_RGFW");
    }
    if (flags & FLAG_WINDOW_ALWAYS_RUN)
    {
        TRACELOG(LOG_WARNING, "SetWindowState() - FLAG_WINDOW_ALWAYS_RUN is not supported on PLATFORM_DESKTOP_RGFW");
    }
    if (flags & FLAG_WINDOW_TRANSPARENT)
    {
        TRACELOG(LOG_WARNING, "SetWindowState() - FLAG_WINDOW_TRANSPARENT post window creation post window creation is not supported on PLATFORM_DESKTOP_RGFW");
    }
    if (flags & FLAG_WINDOW_HIGHDPI)
    {
        TRACELOG(LOG_WARNING, "SetWindowState() - FLAG_WINDOW_HIGHDPI is not supported on PLATFORM_DESKTOP_RGFW");
    }
    if (flags & FLAG_WINDOW_MOUSE_PASSTHROUGH)
    {
        RGFW_window_setMousePassthrough(platform.window, flags & FLAG_WINDOW_MOUSE_PASSTHROUGH);
    }
    if (flags & FLAG_BORDERLESS_WINDOWED_MODE)
    {
        ToggleBorderlessWindowed();
    }
    if (flags & FLAG_MSAA_4X_HINT)
    {
        RGFW_setGLSamples(4);
    }
    if (flags & FLAG_INTERLACED_HINT)
    {
        TRACELOG(LOG_WARNING, "SetWindowState() - FLAG_INTERLACED_HINT is not supported on PLATFORM_DESKTOP_RGFW");
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
        ToggleBorderlessWindowed();
        RGFW_window_restore(platform.window);
        CORE.Window.fullscreen = false;
    }
    if (flags & FLAG_WINDOW_RESIZABLE)
    {
        RGFW_window_setMaxSize(platform.window, RGFW_AREA(0, 0));
        RGFW_window_setMinSize(platform.window, RGFW_AREA(0, 0));
    }
    if (flags & FLAG_WINDOW_UNDECORATED)
    {
        ToggleBorderlessWindowed();
    }
    if (flags & FLAG_WINDOW_HIDDEN)
    {
        RGFW_window_show(platform.window);
    }
    if (flags & FLAG_WINDOW_MINIMIZED)
    {
        RGFW_window_restore(platform.window);
    }
    if (flags & FLAG_WINDOW_MAXIMIZED)
    {
        RGFW_window_restore(platform.window);
    }
    if (flags & FLAG_WINDOW_UNFOCUSED)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState() - FLAG_WINDOW_UNFOCUSED is not supported on PLATFORM_DESKTOP_RGFW");
    }
    if (flags & FLAG_WINDOW_TOPMOST)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState() - FLAG_WINDOW_TOPMOST is not supported on PLATFORM_DESKTOP_RGFW");
    }
    if (flags & FLAG_WINDOW_ALWAYS_RUN)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState() - FLAG_WINDOW_ALWAYS_RUN is not supported on PLATFORM_DESKTOP_RGFW");
    }
    if (flags & FLAG_WINDOW_TRANSPARENT)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState() - FLAG_WINDOW_TRANSPARENT is not supported on PLATFORM_DESKTOP_RGFW");
    }
    if (flags & FLAG_WINDOW_HIGHDPI)
    {
        // NOTE: There also doesn't seem to be a feature to disable high DPI once enabled
        TRACELOG(LOG_WARNING, "ClearWindowState() - FLAG_WINDOW_HIGHDPI is not supported on PLATFORM_DESKTOP_RGFW");
    }
    if (flags & FLAG_WINDOW_MOUSE_PASSTHROUGH)
    {
        RGFW_window_setMousePassthrough(platform.window, flags & FLAG_WINDOW_MOUSE_PASSTHROUGH);
        TRACELOG(LOG_WARNING, "ClearWindowState() - FLAG_WINDOW_MOUSE_PASSTHROUGH is not supported on PLATFORM_DESKTOP_RGFW");
    }
    if (flags & FLAG_BORDERLESS_WINDOWED_MODE)
    {
        ToggleFullscreen();
    }
    if (flags & FLAG_MSAA_4X_HINT)
    {
        RGFW_setGLSamples(0);
    }
    if (flags & FLAG_INTERLACED_HINT)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState() - FLAG_INTERLACED_HINT is not supported on PLATFORM_DESKTOP_RGFW");
    }
}

// Set icon for window
void SetWindowIcon(Image image)
{
    i32 channels = 4;

    switch (image.format)
    {
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
        case PIXELFORMAT_UNCOMPRESSED_R16:           // 16 bpp (1 channel - half float)
        case PIXELFORMAT_UNCOMPRESSED_R32:           // 32 bpp (1 channel - float)
        {
            channels = 1;
        } break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:    // 8*2 bpp (2 channels)
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5:        // 16 bpp
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8:        // 24 bpp
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:      // 16 bpp (1 bit alpha)
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:      // 16 bpp (4 bit alpha)
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:      // 32 bpp
        {
            channels = 2;
        } break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32:     // 32*3 bpp (3 channels - float)
        case PIXELFORMAT_UNCOMPRESSED_R16G16B16:     // 16*3 bpp (3 channels - half float)
        case PIXELFORMAT_COMPRESSED_DXT1_RGB:        // 4 bpp (no alpha)
        case PIXELFORMAT_COMPRESSED_ETC1_RGB:        // 4 bpp
        case PIXELFORMAT_COMPRESSED_ETC2_RGB:        // 4 bpp
        case PIXELFORMAT_COMPRESSED_PVRT_RGB:        // 4 bpp
        {
            channels = 3;
        } break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:  // 32*4 bpp (4 channels - float)
        case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:  // 16*4 bpp (4 channels - half float)
        case PIXELFORMAT_COMPRESSED_DXT1_RGBA:       // 4 bpp (1 bit alpha)
        case PIXELFORMAT_COMPRESSED_DXT3_RGBA:       // 8 bpp
        case PIXELFORMAT_COMPRESSED_DXT5_RGBA:       // 8 bpp
        case PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA:   // 8 bpp
        case PIXELFORMAT_COMPRESSED_PVRT_RGBA:       // 4 bpp
        case PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA:   // 8 bpp
        case PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA:   // 2 bpp
        {
            channels = 4;
        } break;
        default: break;
    }

    RGFW_window_setIcon(platform.window, image.data, RGFW_AREA(image.width, image.height), channels);
}

// Set icon for window
void SetWindowIcons(Image *images, int count)
{
    TRACELOG(LOG_WARNING, "SetWindowIcons() not available on target platform");
}

// Set title for window
void SetWindowTitle(const char *title)
{
    RGFW_window_setName(platform.window, (char*)title);
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
    RGFW_window_moveToMonitor(platform.window, RGFW_getMonitors()[monitor]);
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
    RGFW_window_resize(platform.window, RGFW_AREA(width, height));
}

// Set window opacity, value opacity is between 0.0 and 1.0
void SetWindowOpacity(float opacity)
{
    TRACELOG(LOG_WARNING, "SetWindowOpacity() not available on target platform");
}

// Set window focused
void SetWindowFocused(void)
{
    RGFW_window_show(platform.window);
}

// Get native window handle
void *GetWindowHandle(void)
{
#ifdef RGFW_WEBASM
    return (void*)platform.window->src.ctx;
#else
    return (void*)platform.window->src.window;
#endif
}

// Get number of monitors
int GetMonitorCount(void)
{
    #define MAX_MONITORS_SUPPORTED 6

    int count = MAX_MONITORS_SUPPORTED;
    RGFW_monitor *mons = RGFW_getMonitors();

    for (int i = 0; i < 6; i++)
    {
        if (!mons[i].rect.x && !mons[i].rect.y && !mons[i].rect.w && mons[i].rect.h)
        {
            count = i;
            break;
        }
    }

    return count;
}

// Get number of monitors
int GetCurrentMonitor(void)
{
    RGFW_monitor *mons = RGFW_getMonitors();
    RGFW_monitor mon = RGFW_window_getMonitor(platform.window);

    for (int i = 0; i < 6; i++)
    {
        if ((mons[i].rect.x ==  mon.rect.x) && (mons[i].rect.y ==  mon.rect.y)) return i;
    }

    return 0;
}

// Get selected monitor position
Vector2 GetMonitorPosition(int monitor)
{
    RGFW_monitor *mons = RGFW_getMonitors();

    return (Vector2){mons[monitor].rect.x, mons[monitor].rect.y};
}

// Get selected monitor width (currently used by monitor)
int GetMonitorWidth(int monitor)
{
    RGFW_monitor *mons = RGFW_getMonitors();

    return mons[monitor].rect.w;
}

// Get selected monitor height (currently used by monitor)
int GetMonitorHeight(int monitor)
{
    RGFW_monitor *mons = RGFW_getMonitors();

    return mons[monitor].rect.h;
}

// Get selected monitor physical width in millimetres
int GetMonitorPhysicalWidth(int monitor)
{
    RGFW_monitor* mons = RGFW_getMonitors();

    return mons[monitor].physW;
}

// Get selected monitor physical height in millimetres
int GetMonitorPhysicalHeight(int monitor)
{
    RGFW_monitor *mons = RGFW_getMonitors();

    return mons[monitor].physH;
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
    RGFW_monitor *mons = RGFW_getMonitors();

    return mons[monitor].name;
}

// Get window position XY on monitor
Vector2 GetWindowPosition(void)
{
    return (Vector2){ platform.window->r.x, platform.window->r.y };
}

// Get window scale DPI factor for current monitor
Vector2 GetWindowScaleDPI(void)
{
    RGFW_monitor monitor = RGFW_window_getMonitor(platform.window);

    return (Vector2){monitor.scaleX, monitor.scaleX};
}

// Set clipboard text content
void SetClipboardText(const char *text)
{
    RGFW_writeClipboard(text, strlen(text));
}

// Get clipboard text content
// NOTE: returned string is allocated and freed by GLFW
const char *GetClipboardText(void)
{
    return RGFW_readClipboard(NULL);
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

    RGFW_window_mouseHold(platform.window, RGFW_AREA(CORE.Window.screen.width / 2, CORE.Window.screen.height / 2));

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
    double time = 0.0;
    unsigned long long int nanoSeconds = RGFW_getTimeNS();
    time = (double)(nanoSeconds - CORE.Time.base)*1e-9;  // Elapsed time since InitTimer()

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
    TRACELOG(LOG_WARNING, "SetGamepadMappings() not implemented on target platform");
    return 0;
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
    TRACELOG(LOG_WARNING, "GetKeyName() not implemented on target platform");
    return "";
}

static KeyboardKey ConvertScancodeToKey(u32 keycode);

// TODO: Review function to avoid duplicate with RSGL
char RSGL_keystrToChar(const char *str)
{
    if (str[1] == 0) return str[0];

    static const char *map[] = {
        "asciitilde", "`",
        "grave", "~",
        "exclam", "!",
        "at", "@",
        "numbersign", "#",
        "dollar", "$",
        "percent", "%%",
        "asciicircum", "^",
        "ampersand", "&",
        "asterisk", "*",
        "parenleft", "(",
        "parenright", ")",
        "underscore", "_",
        "minus", "-",
        "plus", "+",
        "equal", "=",
        "braceleft", "{",
        "bracketleft", "[",
        "bracketright", "]",
        "braceright", "}",
        "colon", ":",
        "semicolon", ";",
        "quotedbl", "\"",
        "apostrophe", "'",
        "bar", "|",
        "backslash", "\'",
        "less", "<",
        "comma", ",",
        "greater", ">",
        "period", ".",
        "question", "?",
        "slash", "/",
        "space", " ",
        "Return", "\n",
        "Enter", "\n",
        "enter", "\n",
    };

    for (unsigned char i = 0; i < (sizeof(map)/sizeof(char *)); i += 2)
    {
        if (strcmp(map[i], str) == 0) return *map[i + 1];
    }

    return '\0';
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


    #define RGFW_HOLD_MOUSE			(1L<<2)
    #if defined(RGFW_X11) //|| defined(RGFW_MACOS)
    if (platform.window->_winArgs & RGFW_HOLD_MOUSE)
    {
        CORE.Input.Mouse.previousPosition = (Vector2){ 0.0f, 0.0f };
        CORE.Input.Mouse.currentPosition = (Vector2){ 0.0f, 0.0f };
    }
    else
    {
        CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
    }
#endif

    while (RGFW_window_checkEvent(platform.window))
    {

        if ((platform.window->event.type >= RGFW_jsButtonPressed) && (platform.window->event.type <= RGFW_jsAxisMove))
        {
            if (!CORE.Input.Gamepad.ready[platform.window->event.joystick])
            {
                CORE.Input.Gamepad.ready[platform.window->event.joystick] = true;
                CORE.Input.Gamepad.axisCount[platform.window->event.joystick] = platform.window->event.axisesCount;
                CORE.Input.Gamepad.name[platform.window->event.joystick][0] = '\0';
                CORE.Input.Gamepad.axisState[platform.window->event.joystick][GAMEPAD_AXIS_LEFT_TRIGGER] = -1.0f;
                CORE.Input.Gamepad.axisState[platform.window->event.joystick][GAMEPAD_AXIS_RIGHT_TRIGGER] = -1.0f;
            }
        }

        RGFW_Event *event = &platform.window->event;

        // All input events can be processed after polling
        switch (event->type)
        {
            case RGFW_quit: CORE.Window.shouldClose = true; break;
            case RGFW_dnd:      // Dropped file
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
                CORE.Window.screen.width = platform.window->r.w;
                CORE.Window.screen.height =  platform.window->r.h;
                CORE.Window.currentFbo.width = platform.window->r.w;;
                CORE.Window.currentFbo.height = platform.window->r.h;
                CORE.Window.resizedLastFrame = true;
            } break;
            case RGFW_windowMoved:
            {
                CORE.Window.position.x = platform.window->r.x;
                CORE.Window.position.y = platform.window->r.x;
            } break;

            // Keyboard events
            case RGFW_keyPressed:
            {
                KeyboardKey key = ConvertScancodeToKey(event->keyCode);

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
                    CORE.Input.Keyboard.charPressedQueue[CORE.Input.Keyboard.charPressedQueueCount] = RSGL_keystrToChar(event->keyName);
                    CORE.Input.Keyboard.charPressedQueueCount++;
                }
            } break;
            case RGFW_keyReleased:
            {
                KeyboardKey key = ConvertScancodeToKey(event->keyCode);
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
                if (platform.window->_winArgs & RGFW_HOLD_MOUSE)
                {
                    CORE.Input.Mouse.previousPosition = (Vector2){ 0.0f, 0.0f };

                    if (event->point.x)
                        CORE.Input.Mouse.previousPosition.x = CORE.Input.Mouse.currentPosition.x;
                    if (event->point.y)
                        CORE.Input.Mouse.previousPosition.y = CORE.Input.Mouse.currentPosition.y;

                    CORE.Input.Mouse.currentPosition.x = (float)event->point.x;
                    CORE.Input.Mouse.currentPosition.y = (float)event->point.y;
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
            case RGFW_jsButtonPressed:
            {
                int button = -1;

                switch (event->button)
                {
                    case RGFW_JS_Y: button = GAMEPAD_BUTTON_RIGHT_FACE_UP; break;
                    case RGFW_JS_B: button = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT; break;
                    case RGFW_JS_A: button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN; break;
                    case RGFW_JS_X: button = GAMEPAD_BUTTON_RIGHT_FACE_LEFT; break;

                    case RGFW_JS_L1: button = GAMEPAD_BUTTON_LEFT_TRIGGER_1; break;
                    case RGFW_JS_R1: button = GAMEPAD_BUTTON_RIGHT_TRIGGER_1; break;

                    case RGFW_JS_L2: button = GAMEPAD_BUTTON_LEFT_TRIGGER_2; break;
                    case RGFW_JS_R2: button = GAMEPAD_BUTTON_RIGHT_TRIGGER_2; break;

                    case RGFW_JS_SELECT: button = GAMEPAD_BUTTON_MIDDLE_LEFT; break;
                    case RGFW_JS_HOME: button = GAMEPAD_BUTTON_MIDDLE; break;
                    case RGFW_JS_START: button = GAMEPAD_BUTTON_MIDDLE_RIGHT; break;

                    case RGFW_JS_UP: button = GAMEPAD_BUTTON_LEFT_FACE_UP; break;
                    case RGFW_JS_RIGHT: button = GAMEPAD_BUTTON_LEFT_FACE_RIGHT; break;
                    case RGFW_JS_DOWN: button = GAMEPAD_BUTTON_LEFT_FACE_DOWN; break;
                    case RGFW_JS_LEFT: button = GAMEPAD_BUTTON_LEFT_FACE_LEFT; break;

                    default: break;
                }

                if (button >= 0)
                {
                    CORE.Input.Gamepad.currentButtonState[event->joystick][button] = 1;
                    CORE.Input.Gamepad.lastButtonPressed = button;
                }
            } break;
            case RGFW_jsButtonReleased:
            {
                int button = -1;
                switch (event->button)
                {
                    case RGFW_JS_Y: button = GAMEPAD_BUTTON_RIGHT_FACE_UP; break;
                    case RGFW_JS_B: button = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT; break;
                    case RGFW_JS_A: button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN; break;
                    case RGFW_JS_X: button = GAMEPAD_BUTTON_RIGHT_FACE_LEFT; break;

                    case RGFW_JS_L1: button = GAMEPAD_BUTTON_LEFT_TRIGGER_1; break;
                    case RGFW_JS_R1: button = GAMEPAD_BUTTON_RIGHT_TRIGGER_1; break;

                    case RGFW_JS_L2: button = GAMEPAD_BUTTON_LEFT_TRIGGER_2; break;
                    case RGFW_JS_R2: button = GAMEPAD_BUTTON_RIGHT_TRIGGER_2; break;

                    case RGFW_JS_SELECT: button = GAMEPAD_BUTTON_MIDDLE_LEFT; break;
                    case RGFW_JS_HOME: button = GAMEPAD_BUTTON_MIDDLE; break;
                    case RGFW_JS_START: button = GAMEPAD_BUTTON_MIDDLE_RIGHT; break;

                    case RGFW_JS_UP: button = GAMEPAD_BUTTON_LEFT_FACE_UP; break;
                    case RGFW_JS_RIGHT: button = GAMEPAD_BUTTON_LEFT_FACE_RIGHT; break;
                    case RGFW_JS_DOWN: button = GAMEPAD_BUTTON_LEFT_FACE_DOWN; break;
                    case RGFW_JS_LEFT: button = GAMEPAD_BUTTON_LEFT_FACE_LEFT; break;
                    default: break;
                }

                if (button >= 0)
                {
                    CORE.Input.Gamepad.currentButtonState[event->joystick][button] = 0;
                    if (CORE.Input.Gamepad.lastButtonPressed == button) CORE.Input.Gamepad.lastButtonPressed = 0;
                }
            } break;
            case RGFW_jsAxisMove:
            {
                int axis = -1;
                for (int i = 0; i < event->axisesCount; i++)
                {
                    switch(i)
                    {
                        case 0:
                        {
                            if (abs(event->axis[i].x) > abs(event->axis[i].y))
                            {
                                axis = GAMEPAD_AXIS_LEFT_X;
                                break;
                            }

                            axis = GAMEPAD_AXIS_LEFT_Y;
                        } break;
                        case 1:
                        {
                            if (abs(event->axis[i].x) > abs(event->axis[i].y))
                            {
                                axis = GAMEPAD_AXIS_RIGHT_X;
                                break;
                            }

                            axis = GAMEPAD_AXIS_RIGHT_Y;
                        } break;
                        case 2: axis = GAMEPAD_AXIS_LEFT_TRIGGER; break;
                        case 3: axis = GAMEPAD_AXIS_RIGHT_TRIGGER; break;
                        default: break;
                    }

                    #ifdef __linux__
                    float value = (event->axis[i].x + event->axis[i].y)/(float)32767;
                    #else
                    float value = (event->axis[i].x + -event->axis[i].y)/(float)32767;
                    #endif
                    CORE.Input.Gamepad.axisState[event->joystick][axis] = value;

                    // Register button state for triggers in addition to their axes
                    if ((axis == GAMEPAD_AXIS_LEFT_TRIGGER) || (axis == GAMEPAD_AXIS_RIGHT_TRIGGER))
                    {
                        int button = (axis == GAMEPAD_AXIS_LEFT_TRIGGER)? GAMEPAD_BUTTON_LEFT_TRIGGER_2 : GAMEPAD_BUTTON_RIGHT_TRIGGER_2;
                        int pressed = (value > 0.1f);
                        CORE.Input.Gamepad.currentButtonState[event->joystick][button] = pressed;
                        
                        if (pressed) CORE.Input.Gamepad.lastButtonPressed = button;
                        else if (CORE.Input.Gamepad.lastButtonPressed == button) CORE.Input.Gamepad.lastButtonPressed = 0;
                    }
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
    unsigned int flags = RGFW_CENTER | RGFW_ALLOW_DND;

    // Check window creation flags
    if ((CORE.Window.flags & FLAG_FULLSCREEN_MODE) > 0)
    {
        CORE.Window.fullscreen = true;
        flags |= RGFW_FULLSCREEN;
    }

    if ((CORE.Window.flags & FLAG_WINDOW_UNDECORATED) > 0) flags |= RGFW_NO_BORDER;
    if ((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) == 0) flags |= RGFW_NO_RESIZE;

    if ((CORE.Window.flags & FLAG_WINDOW_TRANSPARENT) > 0) flags |= RGFW_TRANSPARENT_WINDOW;

    if ((CORE.Window.flags & FLAG_FULLSCREEN_MODE) > 0) flags |= RGFW_FULLSCREEN;

    // NOTE: Some OpenGL context attributes must be set before window creation

    // Check selection OpenGL version
    if (rlGetVersion() == RL_OPENGL_21)
    {
        RGFW_setGLVersion(RGFW_GL_CORE, 2, 1);
    }
    else if (rlGetVersion() == RL_OPENGL_33)
    {
        RGFW_setGLVersion(RGFW_GL_CORE, 3, 3);
    }
    else if (rlGetVersion() == RL_OPENGL_43)
    {
        RGFW_setGLVersion(RGFW_GL_CORE, 4, 1);
    }

    if (CORE.Window.flags & FLAG_MSAA_4X_HINT)
    {
        RGFW_setGLSamples(4);
    }

    platform.window = RGFW_createWindow(CORE.Window.title, RGFW_RECT(0, 0, CORE.Window.screen.width, CORE.Window.screen.height), flags);

    RGFW_area screenSize = RGFW_getScreenSize();
    CORE.Window.display.width = screenSize.w;
    CORE.Window.display.height = screenSize.h;
    /* 
        I think this is needed by Raylib now ? 
        If so, rcore_destkop_sdl should be updated too
    */
    SetupFramebuffer(CORE.Window.display.width, CORE.Window.display.height);


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
    rlLoadExtensions((void*)RGFW_getProcAddress);
    //----------------------------------------------------------------------------

    // TODO: Initialize input events system
    // It could imply keyboard, mouse, gamepad, touch...
    // Depending on the platform libraries/SDK it could use a callback mechanism
    // For system events and inputs evens polling on a per-frame basis, use PollInputEvents()
    //----------------------------------------------------------------------------
    // ...
    //----------------------------------------------------------------------------

    // Initialize timing system
    //----------------------------------------------------------------------------
    InitTimer();
    //----------------------------------------------------------------------------

    // Initialize storage system
    //----------------------------------------------------------------------------
    CORE.Storage.basePath = GetWorkingDirectory();
    //----------------------------------------------------------------------------

#ifdef RGFW_X11
    for (int i = 0; (i < 4) && (i < MAX_GAMEPADS); i++)
    {
        RGFW_registerJoystick(platform.window, i);
    }
#endif

    TRACELOG(LOG_INFO, "PLATFORM: CUSTOM: Initialized successfully");

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
