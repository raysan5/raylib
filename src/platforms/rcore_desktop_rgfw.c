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
*       - TBD
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
*   Copyright (c) 2013-2026 Ramon Santamaria (@raysan5), Colleague Riley and contributors
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

#if defined(_WIN32) || defined(_WIN64)
    #define BI_ALPHABITFIELDS 4
    #define LoadImage LoadImageA

    // Temporarily rename conflicting symbols
    #define CloseWindow CloseWindowWin32
    #define Rectangle RectangleWin32
    #define ShowCursor ShowCursorWin32

    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

    // Restore for raylib/RGFW
    #undef CloseWindow
    #undef Rectangle
    #undef ShowCursor
    #undef LoadImage

    #include "../external/fix_win32_compatibility.h"
#endif

#if defined(PLATFORM_WEB_RGFW)
    #define RGFW_NO_GL_HEADER
#endif

#if defined(GRAPHICS_API_OPENGL_ES2) && !defined(PLATFORM_WEB_RGFW)
    #define RGFW_OPENGL_ES2
#endif

void ShowCursor(void);
void CloseWindow(void);
double get_time_seconds(void);

#if defined(__unix__) || defined(__linux__)
    #define _XTYPEDEF_FONT
#endif

#define RGFW_OPENGL
#define RGFW_IMPLEMENTATION

#if defined(_WIN32) || defined(_WIN64)
    #define WIN32_LEAN_AND_MEAN
    #define Rectangle rectangle_win32
    #define CloseWindow CloseWindow_win32
    #define ShowCursor __imp_ShowCursor
    #define _APISETSTRING_

#if defined(__cplusplus)
extern "C" {
#endif
    __declspec(dllimport) int __stdcall MultiByteToWideChar(unsigned int CodePage, unsigned long dwFlags, const char *lpMultiByteStr, int cbMultiByte, wchar_t *lpWideCharStr, int cchWideChar);
#if defined(__cplusplus)
}
#endif

#endif

#if defined(__APPLE__)
    #define Point NSPOINT
    #define Size NSSIZE

    #ifdef GetColor
        #undef GetColor
    #endif
    #define GetColor GetColor_osx
    #ifdef EventType
        #undef EventType
    #endif
    #define EventType EventType_osx
#endif

#if defined(__APPLE__)
    // older macs (unsupported?) are missing these
    #include <IOKit/hid/IOHIDUsageTables.h>
    #ifndef kHIDUsage_Button_5
        #define kHIDUsage_Button_5 0x05
    #endif
    #ifndef kHIDUsage_Button_6
        #define kHIDUsage_Button_6 0x06
    #endif
    #ifndef kHIDUsage_Button_7
        #define kHIDUsage_Button_7 0x07
    #endif
    #ifndef kHIDUsage_Button_8
        #define kHIDUsage_Button_8 0x08
    #endif
    #ifndef kHIDUsage_Button_9
        #define kHIDUsage_Button_9 0x09
    #endif
    #ifndef kHIDUsage_Button_10
        #define kHIDUsage_Button_10 0x0A
    #endif
#endif

// minigamepad used for gamepad support
#define MG_MAX_GAMEPADS MAX_GAMEPADS // copy raylibs define into minigamepad
#define MG_IMPLEMENTATION
#include "../external/RGFW/deps/minigamepad.h"

#define RGFW_ALLOC RL_MALLOC
#define RGFW_FREE RL_FREE
#define RGFW_CALLOC RL_CALLOC
#define RGFW_INT_DEFINED 1 // to avoid issues with minigamepad+RGFW definitions

#include "../external/RGFW/RGFW.h"

#if defined(_WIN32) || defined(_WIN64)
    #undef DrawText
    #undef ShowCursor
    #undef CloseWindow
    #undef Rectangle

    #ifdef MAX_PATH
        #undef MAX_PATH
    #endif
    #define MAX_PATH 1025
#endif

#if defined(__APPLE__)
    #undef Point
    #undef Size
#endif

#include <stdbool.h>

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    double startTime;
    RGFW_window *window;                // Native display device (physical screen connection)
    RGFW_monitor *monitor;
    mg_gamepads minigamepad;
} PlatformData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern CoreData CORE;                   // Global CORE state context

static PlatformData platform = { 0 };   // Platform specific

#if defined(__linux__) // prevent collision of raylibs KEY_<X> and linux/input.h KEY_<X>
    #undef KEY_NULL
    // Alphanumeric keys
    #undef KEY_APOSTROPHE
    #undef KEY_COMMA
    #undef KEY_MINUS
    #undef KEY_PERIOD
    #undef KEY_SLASH
    #undef KEY_ZERO
    #undef KEY_ONE
    #undef KEY_TWO
    #undef KEY_THREE
    #undef KEY_FOUR
    #undef KEY_FIVE
    #undef KEY_SIX
    #undef KEY_SEVEN
    #undef KEY_EIGHT
    #undef KEY_NINE
    #undef KEY_SEMICOLON
    #undef KEY_EQUAL
    #undef KEY_A
    #undef KEY_B
    #undef KEY_C
    #undef KEY_D
    #undef KEY_E
    #undef KEY_F
    #undef KEY_G
    #undef KEY_H
    #undef KEY_I
    #undef KEY_J
    #undef KEY_K
    #undef KEY_L
    #undef KEY_M
    #undef KEY_N
    #undef KEY_O
    #undef KEY_P
    #undef KEY_Q
    #undef KEY_R
    #undef KEY_S
    #undef KEY_T
    #undef KEY_U
    #undef KEY_V
    #undef KEY_W
    #undef KEY_X
    #undef KEY_Y
    #undef KEY_Z
    #undef KEY_LEFT_BRACKET
    #undef KEY_BACKSLASH
    #undef KEY_RIGHT_BRACKET
    #undef KEY_GRAVE
    // Function keys
    #undef KEY_SPACE
    #undef KEY_ESCAPE
    #undef KEY_ENTER
    #undef KEY_TAB
    #undef KEY_BACKSPACE
    #undef KEY_INSERT
    #undef KEY_DELETE
    #undef KEY_RIGHT
    #undef KEY_LEFT
    #undef KEY_DOWN
    #undef KEY_UP
    #undef KEY_PAGE_UP
    #undef KEY_PAGE_DOWN
    #undef KEY_HOME
    #undef KEY_END
    #undef KEY_CAPS_LOCK
    #undef KEY_SCROLL_LOCK
    #undef KEY_NUM_LOCK
    #undef KEY_PRINT_SCREEN
    #undef KEY_PAUSE
    #undef KEY_F1
    #undef KEY_F2
    #undef KEY_F3
    #undef KEY_F4
    #undef KEY_F5
    #undef KEY_F6
    #undef KEY_F7
    #undef KEY_F8
    #undef KEY_F9
    #undef KEY_F10
    #undef KEY_F11
    #undef KEY_F12
    #undef KEY_LEFT_SHIFT
    #undef KEY_LEFT_CONTROL
    #undef KEY_LEFT_ALT
    #undef KEY_LEFT_SUPER
    #undef KEY_RIGHT_SHIFT
    #undef KEY_RIGHT_CONTROL
    #undef KEY_RIGHT_ALT
    #undef KEY_RIGHT_SUPER
    #undef KEY_KB_MENU
    // Keypad keys
    #undef KEY_KP_0
    #undef KEY_KP_1
    #undef KEY_KP_2
    #undef KEY_KP_3
    #undef KEY_KP_4
    #undef KEY_KP_5
    #undef KEY_KP_6
    #undef KEY_KP_7
    #undef KEY_KP_8
    #undef KEY_KP_9
    #undef KEY_KP_DECIMAL
    #undef KEY_KP_DIVIDE
    #undef KEY_KP_MULTIPLY
    #undef KEY_KP_SUBTRACT
    #undef KEY_KP_ADD
    #undef KEY_KP_ENTER
    #undef KEY_KP_EQUAL
#endif

static const unsigned short RGFW_keyConvertTable[] = {
    [RGFW_keyNULL] = KEY_NULL,
    [RGFW_apostrophe] = KEY_APOSTROPHE,
    [RGFW_comma] = KEY_COMMA,
    [RGFW_minus] = KEY_MINUS,
    [RGFW_period] = KEY_PERIOD,
    [RGFW_slash] = KEY_SLASH,
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
    [RGFW_semicolon] = KEY_SEMICOLON,
    [RGFW_equals] = KEY_EQUAL,
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
    [RGFW_backtick] = KEY_GRAVE,
    [RGFW_space] = KEY_SPACE,
    [RGFW_escape] = KEY_ESCAPE,
    [RGFW_return] = KEY_ENTER,
    [RGFW_tab] = KEY_TAB,
    [RGFW_backSpace] = KEY_BACKSPACE,
    [RGFW_insert] = KEY_INSERT,
    [RGFW_delete] = KEY_DELETE,
    [RGFW_right] = KEY_RIGHT,
    [RGFW_left] = KEY_LEFT,
    [RGFW_down] = KEY_DOWN,
    [RGFW_up] = KEY_UP,
    [RGFW_pageUp] = KEY_PAGE_UP,
    [RGFW_pageDown] = KEY_PAGE_DOWN,
    [RGFW_home] = KEY_HOME,
    [RGFW_end] = KEY_END,
    [RGFW_capsLock] = KEY_CAPS_LOCK,
    [RGFW_scrollLock] = KEY_SCROLL_LOCK,
    [RGFW_numLock] = KEY_NUM_LOCK,
    [RGFW_printScreen] = KEY_PRINT_SCREEN,
    [RGFW_pause] = KEY_PAUSE,
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
    [RGFW_shiftL] = KEY_LEFT_SHIFT,
    [RGFW_controlL] = KEY_LEFT_CONTROL,
    [RGFW_altL] = KEY_LEFT_ALT,
    [RGFW_superL] = KEY_LEFT_SUPER,
    // #ifndef RGFW_MACOS
    [RGFW_shiftR] = KEY_RIGHT_SHIFT,
    [RGFW_controlR] = KEY_RIGHT_CONTROL,
    [RGFW_altR] = KEY_RIGHT_ALT,
    [RGFW_superR] = KEY_RIGHT_SUPER,
    // #endif
    [RGFW_menu] = KEY_KB_MENU,
    [RGFW_kp0] = KEY_KP_0,
    [RGFW_kp1] = KEY_KP_1,
    [RGFW_kp2] = KEY_KP_2,
    [RGFW_kp3] = KEY_KP_3,
    [RGFW_kp4] = KEY_KP_4,
    [RGFW_kp5] = KEY_KP_5,
    [RGFW_kp6] = KEY_KP_6,
    [RGFW_kp7] = KEY_KP_7,
    [RGFW_kp8] = KEY_KP_8,
    [RGFW_kp9] = KEY_KP_9,
    [RGFW_kpPeriod] = KEY_KP_DECIMAL,
    [RGFW_kpSlash] = KEY_KP_DIVIDE,
    [RGFW_kpMultiply] = KEY_KP_MULTIPLY,
    [RGFW_kpMinus] = KEY_KP_SUBTRACT,
    [RGFW_kpPlus] = KEY_KP_ADD,
    [RGFW_kpReturn] = KEY_KP_ENTER,
    [RGFW_kpEqual] = KEY_KP_EQUAL,
};

static int mg_buttonConvertTable[] = {
    [MG_BUTTON_NORTH] = GAMEPAD_BUTTON_RIGHT_FACE_UP,
    [MG_BUTTON_EAST] = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,
    [MG_BUTTON_SOUTH] = GAMEPAD_BUTTON_RIGHT_FACE_DOWN,
    [MG_BUTTON_WEST] = GAMEPAD_BUTTON_RIGHT_FACE_LEFT,
    [MG_BUTTON_LEFT_SHOULDER] = GAMEPAD_BUTTON_LEFT_TRIGGER_1,
    [MG_BUTTON_RIGHT_SHOULDER] = GAMEPAD_BUTTON_RIGHT_TRIGGER_1,
    [MG_BUTTON_LEFT_TRIGGER] = GAMEPAD_BUTTON_LEFT_TRIGGER_2,
    [MG_BUTTON_RIGHT_TRIGGER] = GAMEPAD_BUTTON_RIGHT_TRIGGER_2,
    [MG_BUTTON_BACK] = GAMEPAD_BUTTON_MIDDLE_LEFT,
    [MG_BUTTON_GUIDE] = GAMEPAD_BUTTON_MIDDLE,
    [MG_BUTTON_START] = GAMEPAD_BUTTON_MIDDLE_RIGHT,
    [MG_BUTTON_DPAD_UP] = GAMEPAD_BUTTON_LEFT_FACE_UP,
    [MG_BUTTON_DPAD_RIGHT] = GAMEPAD_BUTTON_LEFT_FACE_RIGHT,
    [MG_BUTTON_DPAD_DOWN] = GAMEPAD_BUTTON_LEFT_FACE_DOWN,
    [MG_BUTTON_DPAD_LEFT] = GAMEPAD_BUTTON_LEFT_FACE_LEFT,
    [MG_BUTTON_LEFT_STICK] = GAMEPAD_BUTTON_LEFT_THUMB,
    [MG_BUTTON_RIGHT_STICK] = GAMEPAD_BUTTON_RIGHT_THUMB,
};

static int mg_axisConvertTable[] = {
    [MG_AXIS_LEFT_X] = GAMEPAD_AXIS_LEFT_X,
    [MG_AXIS_LEFT_Y] = GAMEPAD_AXIS_LEFT_Y,
    [MG_AXIS_RIGHT_X] = GAMEPAD_AXIS_RIGHT_X,
    [MG_AXIS_RIGHT_Y] = GAMEPAD_AXIS_RIGHT_Y,
    [MG_AXIS_LEFT_TRIGGER] = GAMEPAD_AXIS_LEFT_TRIGGER,
    [MG_AXIS_RIGHT_TRIGGER] = GAMEPAD_AXIS_RIGHT_TRIGGER,

    /* unsupported in raylib */
    [MG_AXIS_HAT_DPAD_LEFT_RIGHT] = -1,
    [MG_AXIS_HAT_DPAD_LEFT] = -1,
    [MG_AXIS_HAT_DPAD_RIGHT] = -1,
    [MG_AXIS_HAT_DPAD_UP_DOWN] = -1,
    [MG_AXIS_HAT_DPAD_UP] = -1,
    [MG_AXIS_HAT_DPAD_DOWN] = -1,
};

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
int InitPlatform(void);          // Initialize platform (graphics, inputs and more)
bool InitGraphicsDevice(void);   // Initialize graphics device

static KeyboardKey ConvertScancodeToKey(u32 keycode);

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
    if (!FLAG_IS_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE))
    {
        FLAG_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE);
        // Store previous window position (in case we exit fullscreen)
        Vector2 currentPosition = GetWindowPosition();
        CORE.Window.previousPosition.x = currentPosition.x;
        CORE.Window.previousPosition.y = currentPosition.y;
        CORE.Window.previousScreen = CORE.Window.screen;

        RGFW_monitor* currentMonitor = RGFW_window_getMonitor(platform.window);
        RGFW_monitor_scaleToWindow(currentMonitor, platform.window);
        RGFW_window_setFullscreen(platform.window, 1);
    }
    else
    {
        FLAG_CLEAR(CORE.Window.flags, FLAG_FULLSCREEN_MODE);

        // we update the window position right away
        CORE.Window.position = CORE.Window.previousPosition;
        RGFW_window_setFullscreen(platform.window, 0);
        RGFW_window_move(platform.window, CORE.Window.position.x, CORE.Window.position.y);
        RGFW_window_resize(platform.window, CORE.Window.previousScreen.width, CORE.Window.previousScreen.height);
    }

    // Try to enable GPU V-Sync, so frames are limited to screen refresh rate (60Hz -> 60 FPS)
    // NOTE: V-Sync can be enabled by graphic driver configuration
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_VSYNC_HINT)) RGFW_window_swapInterval_OpenGL(platform.window, 1);
}

// Toggle borderless windowed mode
void ToggleBorderlessWindowed(void)
{
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE))
    {
        ToggleFullscreen();

        // it seems like returning here is a more desireable outcome
        return;
    }

    if (!FLAG_IS_SET(CORE.Window.flags, FLAG_BORDERLESS_WINDOWED_MODE))
    {
        FLAG_SET(CORE.Window.flags, FLAG_BORDERLESS_WINDOWED_MODE);

        Vector2 currentPosition = GetWindowPosition();
        CORE.Window.previousPosition.x = (int)currentPosition.x;
        CORE.Window.previousPosition.y = (int)currentPosition.y;
        CORE.Window.previousScreen = CORE.Window.screen;

        RGFW_monitor *currentMonitor = RGFW_window_getMonitor(platform.window);
        RGFW_window_setBorder(platform.window, 0);
        RGFW_window_move(platform.window, 0, 0);
        RGFW_window_resize(platform.window, currentMonitor->mode.w, currentMonitor->mode.h);
    }
    else
    {
        FLAG_CLEAR(CORE.Window.flags, FLAG_BORDERLESS_WINDOWED_MODE);
        RGFW_window_setBorder(platform.window, 1);
        
        CORE.Window.position = CORE.Window.previousPosition;

        RGFW_window_resize(platform.window, CORE.Window.previousScreen.width, CORE.Window.previousScreen.height);
        RGFW_window_move(platform.window, CORE.Window.position.x, CORE.Window.position.y);
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

// Restore window from being minimized/maximized
void RestoreWindow(void)
{
    if (!FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_UNFOCUSED)) RGFW_window_focus(platform.window);

    RGFW_window_restore(platform.window);
}

// Set window configuration state using flags
void SetWindowState(unsigned int flags)
{
    if (!CORE.Window.ready) TRACELOG(LOG_WARNING, "WINDOW: SetWindowState does nothing before window initialization, Use \"SetConfigFlags\" instead");

    FLAG_SET(CORE.Window.flags, flags);

    if (FLAG_IS_SET(flags, FLAG_VSYNC_HINT))
    {
        RGFW_window_swapInterval_OpenGL(platform.window, 1);
    }
    if (FLAG_IS_SET(flags, FLAG_FULLSCREEN_MODE))
    {
        ToggleFullscreen();
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_RESIZABLE))
    {
        RGFW_window_setMaxSize(platform.window, 0, 0);
        RGFW_window_setMinSize(platform.window, 0, 0);
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_UNDECORATED))
    {
        RGFW_window_setBorder(platform.window, 0);
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_HIDDEN))
    {
        RGFW_window_hide(platform.window);
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_MINIMIZED))
    {
        RGFW_window_minimize(platform.window);
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_MAXIMIZED))
    {
        RGFW_window_maximize(platform.window);
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_UNFOCUSED))
    {
        FLAG_SET(CORE.Window.flags, FLAG_WINDOW_UNFOCUSED);
        FLAG_CLEAR(platform.window->internal.flags, RGFW_windowFocusOnShow);
        RGFW_window_setFlags(platform.window, platform.window->internal.flags);
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_TOPMOST))
    {
        RGFW_window_setFloating(platform.window, RGFW_TRUE);
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_ALWAYS_RUN))
    {
        FLAG_SET(CORE.Window.flags, FLAG_WINDOW_ALWAYS_RUN);
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_TRANSPARENT))
    {
        TRACELOG(LOG_WARNING, "WINDOW: Framebuffer transparency can only be configured before window initialization");
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_HIGHDPI))
    {
        TRACELOG(LOG_WARNING, "WINDOW: High DPI can only be configured before window initialization");
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_MOUSE_PASSTHROUGH))
    {
        RGFW_window_setMousePassthrough(platform.window, 1);
    }
    if (FLAG_IS_SET(flags, FLAG_BORDERLESS_WINDOWED_MODE))
    {
        ToggleBorderlessWindowed();
    }
    if (FLAG_IS_SET(flags, FLAG_MSAA_4X_HINT))
    {
        RGFW_glHints* hints = RGFW_getGlobalHints_OpenGL();
        hints->samples = 4;
        RGFW_setGlobalHints_OpenGL(hints);
    }
    if (FLAG_IS_SET(flags, FLAG_INTERLACED_HINT))
    {
        TRACELOG(LOG_WARNING, "RPI: Interlaced mode can only be configured before window initialization");
    }
}

// Clear window configuration state flags
void ClearWindowState(unsigned int flags)
{
    FLAG_CLEAR(CORE.Window.flags, flags);

    if (FLAG_IS_SET(flags, FLAG_VSYNC_HINT))
    {
        RGFW_window_swapInterval_OpenGL(platform.window, 0);
    }
    if (FLAG_IS_SET(flags, FLAG_FULLSCREEN_MODE))
    {
        ToggleFullscreen();
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_RESIZABLE))
    {
        RGFW_window_setMaxSize(platform.window, platform.window->w, platform.window->h);
        RGFW_window_setMinSize(platform.window, platform.window->w, platform.window->h);
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_UNDECORATED))
    {
        RGFW_window_setBorder(platform.window, 1);
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_HIDDEN))
    {
        if (!FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_UNFOCUSED)) RGFW_window_focus(platform.window);

        RGFW_window_show(platform.window);
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_MINIMIZED))
    {
        if (!FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_UNFOCUSED)) RGFW_window_focus(platform.window);

        RGFW_window_restore(platform.window);
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_MAXIMIZED))
    {
        if (!FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_UNFOCUSED)) RGFW_window_focus(platform.window);

        RGFW_window_restore(platform.window);
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_UNFOCUSED))
    {
        RGFW_window_setFlags(platform.window, platform.window->internal.flags | RGFW_windowFocusOnShow);
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_TOPMOST))
    {
        RGFW_window_setFloating(platform.window, RGFW_FALSE);
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_TRANSPARENT))
    {
        TRACELOG(LOG_WARNING, "WINDOW: Framebuffer transparency can only be configured before window initialization");
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_HIGHDPI))
    {
        TRACELOG(LOG_WARNING, "WINDOW: High DPI can only be configured before window initialization");
    }
    if (FLAG_IS_SET(flags, FLAG_WINDOW_MOUSE_PASSTHROUGH))
    {
        RGFW_window_setMousePassthrough(platform.window, 0);
    }
    if (FLAG_IS_SET(flags, FLAG_BORDERLESS_WINDOWED_MODE))
    {
        ToggleBorderlessWindowed();
    }
    if (FLAG_IS_SET(flags, FLAG_MSAA_4X_HINT))
    {
        RGFW_glHints* hints = RGFW_getGlobalHints_OpenGL();
        hints->samples = 0;
        RGFW_setGlobalHints_OpenGL(hints);
    }
    if (FLAG_IS_SET(flags, FLAG_INTERLACED_HINT))
    {
        TRACELOG(LOG_WARNING, "RPI: Interlaced mode can only be configured before window initialization");
    }
}

// Set icon for window
void SetWindowIcon(Image image)
{
    if (image.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
    {
        TRACELOG(LOG_WARNING, "RGFW: Window icon image must be in R8G8B8A8 pixel format");
        return;
    }
    RGFW_window_setIcon(platform.window, (u8 *)image.data, image.width, image.height, 4);
}

// Set icon for window
void SetWindowIcons(Image *images, int count)
{
    if ((images == NULL) || (count <= 0))
    {
        RGFW_window_setIcon(platform.window, NULL, 0, 0, 0);
    }
    else
    {
        Image *bigIcon = NULL;
        Image *smallIcon = NULL;

        for (int i = 0; i < count; i++)
        {
            if (images[i].format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
            {
                TRACELOG(LOG_WARNING, "RGFW: Window icon image must be in R8G8B8A8 pixel format");
                continue;
            }
            if ((bigIcon == NULL) || ((images[i].width > bigIcon->width) && (images[i].height > bigIcon->height))) bigIcon = &images[i];
            if ((smallIcon == NULL) || ((images[i].width < smallIcon->width) && (images[i].height > smallIcon->height))) smallIcon = &images[i];
        }

        if (smallIcon != NULL) RGFW_window_setIconEx(platform.window, (u8 *)smallIcon->data, smallIcon->width, smallIcon->height, 4, RGFW_iconWindow);
        if (bigIcon != NULL) RGFW_window_setIconEx(platform.window, (u8 *)bigIcon->data, bigIcon->width, bigIcon->height, 4, RGFW_iconTaskbar);
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
    RGFW_window_move(platform.window, x, y);
}

// Set monitor for the current window
void SetWindowMonitor(int monitor)
{
    RGFW_window_moveToMonitor(platform.window, RGFW_getMonitors(NULL)[monitor]);
}

// Set window minimum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMinSize(int width, int height)
{
    RGFW_window_setMinSize(platform.window, width, height);
    CORE.Window.screenMin.width = width;
    CORE.Window.screenMin.height = height;
}

// Set window maximum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMaxSize(int width, int height)
{
    RGFW_window_setMaxSize(platform.window, width, height);
    CORE.Window.screenMax.width = width;
    CORE.Window.screenMax.height = height;
}

// Set window dimensions
void SetWindowSize(int width, int height)
{
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIGHDPI))
    {
        CORE.Window.screen.width = width;
        CORE.Window.screen.height = height;

        Vector2 scaleDpi = GetWindowScaleDPI();

        #if defined(__APPLE__)
            RGFW_monitor* currentMonitor = RGFW_window_getMonitor(platform.window);
            CORE.Window.screenScale = MatrixScale(currentMonitor->pixelRatio, currentMonitor->pixelRatio, 1.0f);

            CORE.Window.render.width = CORE.Window.screen.width * currentMonitor->pixelRatio;
            CORE.Window.render.height = CORE.Window.screen.height * currentMonitor->pixelRatio;
            CORE.Window.currentFbo.width = CORE.Window.render.width;
            CORE.Window.currentFbo.height = CORE.Window.render.height;
        #else
            SetMouseScale(1.0f/scaleDpi.x, 1.0f/scaleDpi.y);
            CORE.Window.screenScale = MatrixScale(scaleDpi.x, scaleDpi.y, 1.0f);
        #endif

        CORE.Window.currentFbo.width = CORE.Window.render.width;
        CORE.Window.currentFbo.height = CORE.Window.render.height;
    }
    else
    {
        CORE.Window.screen.width = width;
        CORE.Window.screen.height = height;
    }
    
    RGFW_window_resize(platform.window, CORE.Window.screen.width, CORE.Window.screen.height);
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

#if defined(RGFW_WASM)
    return (void *)&platform.window->src.ctx;
#elif defined(RGFW_WAYLAND)
    return (void *)platform.window->src.surface;
#else
    return (void *)platform.window->src.window;
#endif
}

// Get number of monitors
int GetMonitorCount(void)
{
    size_t count = 0;
    RGFW_getMonitors(&count);

    return count;
}

// Get current monitor where window is placed
int GetCurrentMonitor(void)
{
    RGFW_monitor **mons = RGFW_getMonitors(NULL);
    RGFW_monitor *mon = NULL;

    if (platform.window) mon = RGFW_window_getMonitor(platform.window);
    else mon = RGFW_getPrimaryMonitor();

    for (int i = 0; i < 6; i++)
    {
        if ((mons[i]->x ==  mon->x) && (mons[i]->y ==  mon->y)) return i;
    }

    return 0;
}

// Get selected monitor position
Vector2 GetMonitorPosition(int monitor)
{
    RGFW_monitor **mons = RGFW_getMonitors(NULL);

    return (Vector2){ (float)mons[monitor]->x, (float)mons[monitor]->y };
}

// Get selected monitor width (currently used by monitor)
int GetMonitorWidth(int monitor)
{
    RGFW_monitor **mons = RGFW_getMonitors(NULL);

    return mons[monitor]->mode.w;
}

// Get selected monitor height (currently used by monitor)
int GetMonitorHeight(int monitor)
{
    RGFW_monitor **mons = RGFW_getMonitors(NULL);

    return mons[monitor]->mode.h;
}

// Get selected monitor physical width in millimetres
int GetMonitorPhysicalWidth(int monitor)
{
    RGFW_monitor **mons = RGFW_getMonitors(NULL);

    return mons[monitor]->physW;
}

// Get selected monitor physical height in millimetres
int GetMonitorPhysicalHeight(int monitor)
{
    RGFW_monitor **mons = RGFW_getMonitors(NULL);

    return (int)mons[monitor]->physH;
}

// Get selected monitor refresh rate
int GetMonitorRefreshRate(int monitor)
{
    RGFW_monitor **mons = RGFW_getMonitors(NULL);

    return (int)mons[monitor]->mode.refreshRate;
}

// Get the human-readable, UTF-8 encoded name of the selected monitor
const char *GetMonitorName(int monitor)
{
    RGFW_monitor **mons = RGFW_getMonitors(NULL);

    return mons[monitor]->name;
}

// Get window position XY on monitor
Vector2 GetWindowPosition(void)
{
    if (platform.window == NULL)
    {
        return (Vector2){ 0.0f, 0.0f };
    }

    if (RGFW_window_getPosition(platform.window, &platform.window->x, &platform.window->y)) {
        return (Vector2){ (float)platform.window->x, (float)platform.window->y };
    }

    return (Vector2){ 0.0f, 0.0f };
}

// Get window scale DPI factor for current monitor
Vector2 GetWindowScaleDPI(void)
{
    RGFW_monitor *monitor = NULL;

    if (platform.window) monitor = RGFW_window_getMonitor(platform.window);
    else monitor = RGFW_getPrimaryMonitor();

    #if defined(__APPLE__)
        // apple does < 1.0f scaling, example: 0.66f, 0.5f
        // we want to convert this to be consistent
        return (Vector2){ 1.0f / monitor->scaleX, 1.0f / monitor->scaleX };
    #else
        // linux and windows do >= 1.0f scaling, example: 1.0f, 1.25f, 2.0f
        return (Vector2){ monitor->scaleX, monitor->scaleX };
    #endif
}

// Not part of raylib. Mac has a different pixel ratio for retina displays
// and we want to be able to handle it
float GetMonitorPixelRatio(void)
{
    RGFW_monitor *monitor = NULL;

    if (platform.window) monitor = RGFW_window_getMonitor(platform.window);
    else monitor = RGFW_getPrimaryMonitor();

    return monitor->pixelRatio;
}

// Set clipboard text content
void SetClipboardText(const char *text)
{
    // add 1 for null terminator
    RGFW_writeClipboard(text, strlen(text)+1);
}

// Get clipboard text content
// NOTE: returned string is allocated and freed by RGFW
const char *GetClipboardText(void)
{
    return RGFW_readClipboard(NULL);
}

#if SUPPORT_CLIPBOARD_IMAGE
#if defined(_WIN32)
    #define WIN32_CLIPBOARD_IMPLEMENTATION
    #define WINUSER_ALREADY_INCLUDED
    #define WINBASE_ALREADY_INCLUDED
    #define WINGDI_ALREADY_INCLUDED
    #include "../external/win32_clipboard.h"
#elif defined(__linux__)
    #include <X11/Xlib.h>
    #include <X11/Xatom.h>
#endif
#endif

// Get clipboard image
Image GetClipboardImage(void)
{
    Image image = { 0 };
    
#if SUPPORT_CLIPBOARD_IMAGE && SUPPORT_MODULE_RTEXTURES
#if defined(_WIN32)

    unsigned long long int dataSize = 0; // moved into _WIN32 scope until other platforms gain support
    void *fileData = NULL; // moved into _WIN32 scope until other platforms gain support

    int width = 0;
    int height = 0;
    fileData  = (void *)Win32GetClipboardImageData(&width, &height, &dataSize);

    if (fileData == NULL) TRACELOG(LOG_WARNING, "Clipboard image: Couldn't get clipboard data");
    else image = LoadImageFromMemory(".bmp", (const unsigned char *)fileData, dataSize);

#elif defined(__linux__)

    // Implementation based on https://github.com/ColleagueRiley/Clipboard-Copy-Paste/blob/main/x11.c
    Display* dpy = XOpenDisplay(NULL);
    if (!dpy) return image;

    Window root = DefaultRootWindow(dpy);
    Window win = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);

    Atom clipboard = XInternAtom(dpy, "CLIPBOARD", False);
    Atom targetType = XInternAtom(dpy, "image/png", False); // Ask for PNG
    Atom property = XInternAtom(dpy, "RAYLIB_CLIPBOARD_MANAGER", False);

    // Request the data: "Convert whatever is in CLIPBOARD to image/png and put it in RAYLIB_CLIPBOARD_MANAGER"
    XConvertSelection(dpy, clipboard, targetType, property, win, CurrentTime);

    // Wait for the SelectionNotify event
    XEvent ev;
    XNextEvent(dpy, &ev);

    Atom actualType;
    int actualFormat;
    unsigned long nitems, bytesAfter;
    unsigned char* data = NULL;

    // Read the data from our ghost window's property
    XGetWindowProperty(dpy, win, property, 0, ~0L, False, AnyPropertyType,
                        &actualType, &actualFormat, &nitems, &bytesAfter, &data);

    if (data != NULL) {
        image = LoadImageFromMemory(".png", data, (int)nitems);
        XFree(data);
    }

    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
#else
    TRACELOG(LOG_WARNING, "Clipboard image: PLATFORM_DESKTOP_RGFW doesn't implement GetClipboardImage() for this OS");
#endif // defined(_WIN32)
#else
    TRACELOG(LOG_WARNING, "Clipboard image: SUPPORT_CLIPBOARD_IMAGE requires SUPPORT_MODULE_RTEXTURES to work properly");
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
    RGFW_window_captureMouse(platform.window, false);

    // Set cursor position in the middle
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);
    ShowCursor();

    CORE.Input.Mouse.cursorLocked = true;
}

// Disables cursor (lock cursor)
void DisableCursor(void)
{
    RGFW_window_captureMouse(platform.window, true);
    HideCursor();

    CORE.Input.Mouse.cursorLocked = true;
}

// Swap back buffer with front buffer (screen drawing)
void SwapScreenBuffer(void)
{
    RGFW_window_swapBuffers_OpenGL(platform.window);
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Misc
//----------------------------------------------------------------------------------

// Get elapsed time measure in seconds since InitTimer()
double GetTime(void)
{
    return get_time_seconds() - platform.startTime;
}

// Open URL with default system browser (if available)
// NOTE: This function is only safe to use if you control the URL given
// A user could craft a malicious string performing another action
void OpenURL(const char *url)
{
    // Security check to (partially) avoid malicious code on target platform
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
    return mg_update_gamepad_mappings(&platform.minigamepad, mappings);
}

// Set gamepad vibration
void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor, float duration)
{
    TRACELOG(LOG_WARNING, "SetGamepadVibration() unsupported on target platform");
}

// Set mouse position XY
void SetMousePosition(int x, int y)
{
    RGFW_window_moveMouse(platform.window, x, y);
    CORE.Input.Mouse.currentPosition = (Vector2){ (float)x, (float)y };
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
}

// Set mouse cursor
void SetMouseCursor(int cursor)
{
    RGFW_window_setMouseStandard(platform.window, cursor);
}

// Get physical key name
const char *GetKeyName(int key)
{
    TRACELOG(LOG_WARNING, "GetKeyName() unsupported on target platform");

    return "";
}

// Register all input events
void PollInputEvents(void)
{
#if SUPPORT_GESTURES_SYSTEM
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
    for (int i = 0; i < MAX_GAMEPADS; i++)
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
    if (RGFW_window_isCaptured(platform.window))
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
        CORE.Time.previous = GetTime();
    }

    RGFW_event rgfw_event;
    while (RGFW_window_checkEvent(platform.window, &rgfw_event))
    {
        // All input events can be processed after polling
        switch (rgfw_event.type)
        {
            case RGFW_mouseEnter: CORE.Input.Mouse.cursorOnScreen = true; break;
            case RGFW_mouseLeave: CORE.Input.Mouse.cursorOnScreen = false; break;
            case RGFW_quit:
                RGFW_window_setShouldClose(platform.window, true);
                return;
            case RGFW_dataDrop:      // Dropped file
            {
                for (int i = 0; i < rgfw_event.drop.count; i++)
                {
                    if (CORE.Window.dropFileCount == 0)
                    {
                        // When a new file is dropped, we reserve a fixed number of slots for all possible dropped files
                        // at the moment we limit the number of drops at once to 1024 files but this behaviour should probably be reviewed
                        // TODO: Pointers should probably be reallocated for any new file added...
                        CORE.Window.dropFilepaths = (char **)RL_CALLOC(1024, sizeof(char *));

                        CORE.Window.dropFilepaths[CORE.Window.dropFileCount] = (char *)RL_CALLOC(MAX_FILEPATH_LENGTH, sizeof(char));
                        strcpy(CORE.Window.dropFilepaths[CORE.Window.dropFileCount], rgfw_event.drop.files[i]);

                        CORE.Window.dropFileCount++;
                    }
                    else if (CORE.Window.dropFileCount < 1024)
                    {
                        CORE.Window.dropFilepaths[CORE.Window.dropFileCount] = (char *)RL_CALLOC(MAX_FILEPATH_LENGTH, sizeof(char));
                        strcpy(CORE.Window.dropFilepaths[CORE.Window.dropFileCount], rgfw_event.drop.files[i]);

                        CORE.Window.dropFileCount++;
                    }
                    else TRACELOG(LOG_WARNING, "FILE: Maximum drag and drop files at once is limited to 1024 files!");
                }
            } break;

            // Window events are also polled (Minimized, maximized, close...)
            case RGFW_windowResized:
            {
                #if defined(__APPLE__)
                    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIGHDPI))
                    {
                        RGFW_monitor* currentMonitor = RGFW_window_getMonitor(platform.window);
                        SetupViewport(platform.window->w * currentMonitor->pixelRatio, platform.window->h * currentMonitor->pixelRatio);
                        CORE.Window.screenScale = MatrixScale(currentMonitor->pixelRatio, currentMonitor->pixelRatio, 1.0f);

                        CORE.Window.screen.width = platform.window->w;
                        CORE.Window.screen.height = platform.window->h;
                        CORE.Window.render.width = CORE.Window.screen.width * currentMonitor->pixelRatio;
                        CORE.Window.render.height = CORE.Window.screen.height * currentMonitor->pixelRatio;
                    }
                    else
                    {
                        SetupViewport(platform.window->w, platform.window->h);
                        CORE.Window.screen.width = platform.window->w;
                        CORE.Window.screen.height = platform.window->h;
                        CORE.Window.render.width = CORE.Window.screen.width;
                        CORE.Window.render.height = CORE.Window.screen.height;
                    }

                    CORE.Window.currentFbo.width = CORE.Window.render.width;
                    CORE.Window.currentFbo.height = CORE.Window.render.height;
                #elif defined(PLATFORM_WEB_RGFW)
                    // do nothing for web
                    return;
                #else
                    SetupViewport(platform.window->w, platform.window->h);
                    // if we are doing automatic DPI scaling, then the "screen" size is divided by the window scale
                    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIGHDPI))
                    {
                        Vector2 scaleDpi = GetWindowScaleDPI();
                        CORE.Window.screen.width = (int)(platform.window->w/scaleDpi.x);
                        CORE.Window.screen.height = (int)(platform.window->h/scaleDpi.y);
                        CORE.Window.screenScale = MatrixScale(scaleDpi.x, scaleDpi.y, 1.0f);
                        // mouse scale doesnt seem needed
                        // SetMouseScale(1.0f/scaleDpi.x, 1.0f/scaleDpi.y);
                    }
                    else
                    {
                        CORE.Window.screen.width = platform.window->w;
                        CORE.Window.screen.height = platform.window->h;
                    }

                    CORE.Window.currentFbo.width = CORE.Window.screen.width;
                    CORE.Window.currentFbo.height = CORE.Window.screen.height;
                #endif
                CORE.Window.resizedLastFrame = true;
            } break;
            case RGFW_windowMaximized:
            {
                FLAG_SET(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED);  // The window was maximized
            } break;
            case RGFW_windowMinimized:
            {
                FLAG_SET(CORE.Window.flags, FLAG_WINDOW_MINIMIZED);  // The window was iconified
            } break;
            case RGFW_windowRestored:
            {
                if (RGFW_window_isMaximized(platform.window))
                    FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED);           // The window was restored
                if (RGFW_window_isMinimized(platform.window))
                    FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_MINIMIZED);           // The window was restored
            } break;
            case RGFW_windowMoved:
            {
                CORE.Window.position.x = platform.window->x;
                CORE.Window.position.y = platform.window->x;
            } break;

            // Keyboard events
            case RGFW_keyPressed:
            {
                KeyboardKey key = ConvertScancodeToKey(rgfw_event.key.value);
                if (key != KEY_NULL)
                {
                    // If key was up, add it to the key pressed queue
                    if ((CORE.Input.Keyboard.currentKeyState[key] == 0) && (CORE.Input.Keyboard.keyPressedQueueCount < MAX_KEY_PRESSED_QUEUE))
                    {
                        CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = key;
                        CORE.Input.Keyboard.keyPressedQueueCount++;
                    }

                    CORE.Input.Keyboard.currentKeyState[key] = 1;

                    if (CORE.Input.Keyboard.currentKeyState[CORE.Input.Keyboard.exitKey]) RGFW_window_setShouldClose(platform.window, true);
                }
            } break;
            case RGFW_keyReleased:
            {
                KeyboardKey key = ConvertScancodeToKey(rgfw_event.key.value);
                if (key != KEY_NULL) CORE.Input.Keyboard.currentKeyState[key] = 0;
            } break;

            case RGFW_keyChar:
            {
                // NOTE: event.text.text data comes an UTF-8 text sequence but we register codepoints (int)
                // Check if there is space available in the queue
                if (CORE.Input.Keyboard.charPressedQueueCount < MAX_CHAR_PRESSED_QUEUE)
                {
                    // Add character (codepoint) to the queue
                    CORE.Input.Keyboard.charPressedQueue[CORE.Input.Keyboard.charPressedQueueCount] = rgfw_event.keyChar.value;
                    CORE.Input.Keyboard.charPressedQueueCount++;
                }
            } break;

            // Check mouse events
            case RGFW_mouseScroll:
            {
                CORE.Input.Mouse.currentWheelMove.x += rgfw_event.scroll.x;
                CORE.Input.Mouse.currentWheelMove.y += rgfw_event.scroll.y;
            } break;
            case RGFW_mouseButtonPressed:
            {
                int btn = rgfw_event.button.value;
                if (btn == RGFW_mouseLeft) btn = 1;
                else if (btn == RGFW_mouseRight) btn = 2;
                else if (btn == RGFW_mouseMiddle) btn = 3;

                CORE.Input.Mouse.currentButtonState[btn - 1] = 1;
                CORE.Input.Touch.currentTouchState[btn - 1] = 1;

                touchAction = 1;
            } break;
            case RGFW_mouseButtonReleased:
            {
                int btn = rgfw_event.button.value;
                if (btn == RGFW_mouseLeft) btn = 1;
                else if (btn == RGFW_mouseRight) btn = 2;
                else if (btn == RGFW_mouseMiddle) btn = 3;

                CORE.Input.Mouse.currentButtonState[btn - 1] = 0;
                CORE.Input.Touch.currentTouchState[btn - 1] = 0;

                touchAction = 0;
            } break;
            case RGFW_mousePosChanged:
            {
                if (RGFW_window_isCaptured(platform.window))
                {
                    CORE.Input.Mouse.currentPosition.x += (float)rgfw_event.mouse.vecX;
                    CORE.Input.Mouse.currentPosition.y += (float)rgfw_event.mouse.vecY;
                }
                else
                {
                    CORE.Input.Mouse.currentPosition.x = (float)rgfw_event.mouse.x;
                    CORE.Input.Mouse.currentPosition.y = (float)rgfw_event.mouse.y;
                }

                CORE.Input.Touch.position[0] = CORE.Input.Mouse.currentPosition;
                touchAction = 2;
            } break;
            default: break;
        }

#if SUPPORT_GESTURES_SYSTEM
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

    mg_event gamepad_event;
    while (mg_gamepads_check_event(&platform.minigamepad, &gamepad_event)) {
        int gamepadIndex = gamepad_event.gamepad->index;
        switch (gamepad_event.type) {
            case MG_EVENT_BUTTON_PRESS:
                {
                    int button = mg_buttonConvertTable[gamepad_event.button];
                    if (button >= 0)
                    {
                        CORE.Input.Gamepad.currentButtonState[gamepadIndex][button] = 1;
                        CORE.Input.Gamepad.lastButtonPressed = button;
                    }
                } break;
            case MG_EVENT_BUTTON_RELEASE:
                {
                    int button = mg_buttonConvertTable[gamepad_event.button];
                    if (button >= 0)
                    {
                        CORE.Input.Gamepad.currentButtonState[gamepadIndex][button] = 0;
                        if (CORE.Input.Gamepad.lastButtonPressed == button) CORE.Input.Gamepad.lastButtonPressed = 0;
                    }
                } break;
            case MG_EVENT_AXIS_MOVE:
                {
                    int axis = mg_axisConvertTable[gamepad_event.axis];

                    switch (axis) {
                        case GAMEPAD_AXIS_LEFT_X:
                        case GAMEPAD_AXIS_LEFT_Y:
                        case GAMEPAD_AXIS_RIGHT_X:
                        case GAMEPAD_AXIS_RIGHT_Y:
                                CORE.Input.Gamepad.axisState[gamepadIndex][axis] = platform.minigamepad.gamepads[gamepadIndex].axes[gamepad_event.axis].value;
                            break;
                        case GAMEPAD_AXIS_LEFT_TRIGGER:
                        case GAMEPAD_AXIS_RIGHT_TRIGGER:
                                CORE.Input.Gamepad.axisState[gamepadIndex][axis] = platform.minigamepad.gamepads[gamepadIndex].axes[gamepad_event.axis].value;

                                /* trigger button press when axis is all the way */
                                int button = (axis == GAMEPAD_AXIS_LEFT_TRIGGER) ? GAMEPAD_BUTTON_LEFT_TRIGGER_2 : GAMEPAD_BUTTON_RIGHT_TRIGGER_2;
                                int pressed = (platform.minigamepad.gamepads[gamepadIndex].axes[gamepad_event.axis].value >= 1.0f);

                                CORE.Input.Gamepad.currentButtonState[gamepadIndex][button] = pressed;
                                if (pressed) CORE.Input.Gamepad.lastButtonPressed = button;
                                else if (CORE.Input.Gamepad.lastButtonPressed == button) CORE.Input.Gamepad.lastButtonPressed = 0;
                            break;
                    }
                } break;
            case MG_EVENT_GAMEPAD_CONNECT:
                CORE.Input.Gamepad.ready[gamepadIndex] = true;
                CORE.Input.Gamepad.axisState[gamepadIndex][GAMEPAD_AXIS_LEFT_TRIGGER] = -1.0f;
                CORE.Input.Gamepad.axisState[gamepadIndex][GAMEPAD_AXIS_RIGHT_TRIGGER] = -1.0f;

                int axisCount = 0;
                for (int i = 0; i < MG_AXIS_COUNT; i += 1) {
                    if (platform.minigamepad.gamepads[gamepadIndex].axes[i].supported) {
                        axisCount += 1;
                    } else {
                        break;
                    }
                }
                CORE.Input.Gamepad.axisCount[gamepadIndex] = axisCount;
                strcpy(CORE.Input.Gamepad.name[gamepadIndex], platform.minigamepad.gamepads[gamepadIndex].name);
                break;
            case MG_EVENT_GAMEPAD_DISCONNECT:
                CORE.Input.Gamepad.ready[gamepadIndex] = false;
                break;
            default: break;
        }
    }
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
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE))
    {
        FLAG_SET(flags, RGFW_windowFullscreen);
    }

    if (FLAG_IS_SET(CORE.Window.flags, FLAG_BORDERLESS_WINDOWED_MODE))
    {
        FLAG_SET(flags, RGFW_windowedFullscreen);
    }

    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_UNDECORATED)) FLAG_SET(flags, RGFW_windowNoBorder);
    if (!FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_RESIZABLE)) FLAG_SET(flags, RGFW_windowNoResize);
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_TRANSPARENT)) FLAG_SET(flags, RGFW_windowTransparent);
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE)) FLAG_SET(flags, RGFW_windowFullscreen);
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIDDEN)) FLAG_SET(flags, RGFW_windowHide);
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED)) FLAG_SET(flags, RGFW_windowMaximize);

    // NOTE: Some OpenGL context attributes must be set before window creation
    // Check selection OpenGL version

    RGFW_glHints* hints = RGFW_getGlobalHints_OpenGL();
    if (rlGetVersion() == RL_OPENGL_21)
    {
        hints->major = 2;
        hints->minor = 1;
    }
    else if (rlGetVersion() == RL_OPENGL_33)
    {
        hints->major = 3;
        hints->minor = 3;
    }
    else if (rlGetVersion() == RL_OPENGL_43)
    {
        hints->major = 4;
        hints->minor = 3;
    }

    if (FLAG_IS_SET(CORE.Window.flags, FLAG_MSAA_4X_HINT)) hints->samples = 4;

    if (!FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_UNFOCUSED)) FLAG_SET(flags, RGFW_windowFocusOnShow | RGFW_windowFocus);

    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIGHDPI))
    {
        #if !defined(__APPLE__)
            CORE.Window.screen.width = CORE.Window.screen.width * GetWindowScaleDPI().x;
            CORE.Window.screen.height = CORE.Window.screen.height * GetWindowScaleDPI().y;
        #endif
    }

    RGFW_setGlobalHints_OpenGL(hints);
    platform.window = RGFW_createWindow((CORE.Window.title != 0)? CORE.Window.title : " ", 0, 0, CORE.Window.screen.width, CORE.Window.screen.height, flags | RGFW_windowOpenGL);
    platform.startTime = get_time_seconds();

#ifndef PLATFORM_WEB_RGFW
    i32 screenSizeWidth;
    i32 screenSizeHeight;
    RGFW_window_getSize(platform.window, &screenSizeWidth, &screenSizeHeight);
    CORE.Window.display.width = screenSizeWidth;
    CORE.Window.display.height = screenSizeHeight;
#else
    CORE.Window.display.width = CORE.Window.screen.width;
    CORE.Window.display.height = CORE.Window.screen.height;
#endif
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_VSYNC_HINT)) RGFW_window_swapInterval_OpenGL(platform.window, 1);

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

    // NOTE: RGFW's exit key is distinct from raylib's exit key and
    // must be set to NULL to not interfere
    RGFW_window_setExitKey(platform.window, RGFW_keyNULL);
    RGFW_window_makeCurrentWindow_OpenGL(platform.window);

    //----------------------------------------------------------------------------

    // If everything work as expected, we can continue
    CORE.Window.position.x = platform.window->x;
    CORE.Window.position.y = platform.window->y;
    CORE.Window.render.width = CORE.Window.screen.width;
    CORE.Window.render.height = CORE.Window.screen.height;
    CORE.Window.currentFbo.width = CORE.Window.render.width;
    CORE.Window.currentFbo.height = CORE.Window.render.height;

    // adjust scale if using highdpi
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIGHDPI)) {
        Vector2 scaleDpi = GetWindowScaleDPI();

        #if defined(__APPLE__)
            RGFW_monitor* currentMonitor = RGFW_window_getMonitor(platform.window);
            CORE.Window.screenScale = MatrixScale(currentMonitor->pixelRatio, currentMonitor->pixelRatio, 1.0f);

            CORE.Window.render.width = CORE.Window.screen.width * currentMonitor->pixelRatio;
            CORE.Window.render.height = CORE.Window.screen.height * currentMonitor->pixelRatio;
            CORE.Window.currentFbo.width = CORE.Window.render.width;
            CORE.Window.currentFbo.height = CORE.Window.render.height;
        #else
            SetMouseScale(1.0f/scaleDpi.x, 1.0f/scaleDpi.y);
            CORE.Window.screenScale = MatrixScale(scaleDpi.x, scaleDpi.y, 1.0f);
            CORE.Window.screen.width /= scaleDpi.x;
            CORE.Window.screen.height /= scaleDpi.y;
        #endif
    }

    TRACELOG(LOG_INFO, "DISPLAY: Device initialized successfully");
    TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
    TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
    TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
    TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);

    // Load OpenGL extensions
    // NOTE: GL procedures address loader is required to load extensions
    //----------------------------------------------------------------------------
    rlLoadExtensions((void *)RGFW_getProcAddress_OpenGL);
    //----------------------------------------------------------------------------

    // Initialize timing system
    //----------------------------------------------------------------------------
    InitTimer();
    //----------------------------------------------------------------------------

    // Initialize storage system
    //----------------------------------------------------------------------------
    #if defined(__APPLE__)
        // mac defaults to the users home folder for some reason
        // this is done to help them read relative paths to the binary
        ChangeDirectory(GetApplicationDirectory());
    #endif

    CORE.Storage.basePath = GetWorkingDirectory();
    //----------------------------------------------------------------------------

#if defined(RGFW_WAYLAND)
    if (RGFW_usingWayland()) TRACELOG(LOG_INFO, "PLATFORM: DESKTOP (RGFW - Wayland): Initialized successfully");
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

    mg_gamepads_init(&platform.minigamepad);

    return 0;
}

// Close platform
void ClosePlatform(void)
{
    mg_gamepads_free(&platform.minigamepad);
    RGFW_window_close(platform.window);
}

// Keycode mapping
static KeyboardKey ConvertScancodeToKey(u32 keycode)
{
    if (keycode > sizeof(RGFW_keyConvertTable)/sizeof(unsigned short)) return KEY_NULL;

    return (KeyboardKey)RGFW_keyConvertTable[keycode];
}

// Helper functions for Time
double get_time_seconds(void)
{
    double currentTime = 0.0;

    #if defined(_WIN32)
        static LARGE_INTEGER freq = { 0 };
        static int freq_init = 0;
        LARGE_INTEGER counter;
        if (!freq_init) {
            QueryPerformanceFrequency(&freq);
            freq_init = 1;
        }
        QueryPerformanceCounter(&counter);
        currentTime = (double)counter.QuadPart / (double)freq.QuadPart;
    #elif defined(__EMSCRIPTEN__)
        currentTime = emscripten_get_now() / 1000.0;
    #elif defined(__APPLE__)
        static mach_timebase_info_data_t tb;
        static int tb_initialized = 0;

        if (!tb_initialized) {
            mach_timebase_info(&tb);
            tb_initialized = 1;
        }
        uint64_t ticks = mach_absolute_time();

        currentTime = (double)ticks * (double)tb.numer / (double)tb.denom / 1e9;
    #elif defined(__linux__)
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        currentTime = (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
    #else
        // fallback to cstd
        currentTime = (double)clock() / (double)CLOCKS_PER_SEC;
    #endif

    return currentTime;
}
