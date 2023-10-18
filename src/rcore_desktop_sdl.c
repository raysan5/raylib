/**********************************************************************************************
*
*   rcore_desktop_sdl - Functions to manage window, graphics device and inputs
*
*   PLATFORM: DESKTOP: SDL
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
*       - SDL 2 (main library)
*       - Dependency 02
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2023 Ramon Santamaria (@raysan5) and contributors
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

#include "rcore.h"

#include "SDL.h"            // SDL base library (window/rendered, input, timming... functionality)
#include "SDL_opengl.h"     // SDL OpenGL functionality (if required, instead of internal renderer)

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    SDL_Window *window;
    SDL_GLContext glContext;

    SDL_Joystick *gamepad;
    SDL_Cursor *cursor;
} PlatformData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern CoreData CORE;                   // Global CORE state context

static PlatformData platform = { 0 };   // Platform specific data

//----------------------------------------------------------------------------------
// Local Variables Definition
//----------------------------------------------------------------------------------
#define SCANCODE_MAPPED_NUM 100
static const KeyboardKey ScancodeToKey[SCANCODE_MAPPED_NUM] = {
    KEY_NULL,           // SDL_SCANCODE_UNKNOWN
    0,
    0,
    0,
    KEY_A,              // SDL_SCANCODE_A
    KEY_B,              // SDL_SCANCODE_B
    KEY_C,              // SDL_SCANCODE_C
    KEY_D,              // SDL_SCANCODE_D
    KEY_E,              // SDL_SCANCODE_E
    KEY_F,              // SDL_SCANCODE_F
    KEY_G,              // SDL_SCANCODE_G
    KEY_H,              // SDL_SCANCODE_H
    KEY_I,              // SDL_SCANCODE_I
    KEY_J,              // SDL_SCANCODE_J
    KEY_K,              // SDL_SCANCODE_K
    KEY_L,              // SDL_SCANCODE_L
    KEY_M,              // SDL_SCANCODE_M
    KEY_N,              // SDL_SCANCODE_N
    KEY_O,              // SDL_SCANCODE_O
    KEY_P,              // SDL_SCANCODE_P
    KEY_Q,              // SDL_SCANCODE_Q
    KEY_R,              // SDL_SCANCODE_R
    KEY_S,              // SDL_SCANCODE_S
    KEY_T,              // SDL_SCANCODE_T
    KEY_U,              // SDL_SCANCODE_U
    KEY_V,              // SDL_SCANCODE_V
    KEY_W,              // SDL_SCANCODE_W
    KEY_X,              // SDL_SCANCODE_X
    KEY_Y,              // SDL_SCANCODE_Y
    KEY_Z,              // SDL_SCANCODE_Z
    KEY_ONE,            // SDL_SCANCODE_1
    KEY_TWO,            // SDL_SCANCODE_2
    KEY_THREE,          // SDL_SCANCODE_3
    KEY_FOUR,           // SDL_SCANCODE_4
    KEY_FIVE,           // SDL_SCANCODE_5
    KEY_SIX,            // SDL_SCANCODE_6
    KEY_SEVEN,          // SDL_SCANCODE_7
    KEY_EIGHT,          // SDL_SCANCODE_8
    KEY_NINE,           // SDL_SCANCODE_9
    KEY_ZERO,           // SDL_SCANCODE_0
    KEY_ENTER,          // SDL_SCANCODE_RETURN
    KEY_ESCAPE,         // SDL_SCANCODE_ESCAPE
    KEY_BACKSPACE,      // SDL_SCANCODE_BACKSPACE
    KEY_TAB,            // SDL_SCANCODE_TAB
    KEY_SPACE,          // SDL_SCANCODE_SPACE
    KEY_MINUS,          // SDL_SCANCODE_MINUS
    KEY_EQUAL,          // SDL_SCANCODE_EQUALS
    KEY_LEFT_BRACKET,   // SDL_SCANCODE_LEFTBRACKET
    KEY_RIGHT_BRACKET,  // SDL_SCANCODE_RIGHTBRACKET
    KEY_BACKSLASH,      // SDL_SCANCODE_BACKSLASH
    0,                  // SDL_SCANCODE_NONUSHASH
    KEY_SEMICOLON,      // SDL_SCANCODE_SEMICOLON
    KEY_APOSTROPHE,     // SDL_SCANCODE_APOSTROPHE
    KEY_GRAVE,          // SDL_SCANCODE_GRAVE
    KEY_COMMA,          // SDL_SCANCODE_COMMA
    KEY_PERIOD,         // SDL_SCANCODE_PERIOD
    KEY_SLASH,          // SDL_SCANCODE_SLASH
    KEY_CAPS_LOCK,      // SDL_SCANCODE_CAPSLOCK
    KEY_F1,             // SDL_SCANCODE_F1
    KEY_F2,             // SDL_SCANCODE_F2
    KEY_F3,             // SDL_SCANCODE_F3
    KEY_F4,             // SDL_SCANCODE_F4
    KEY_F5,             // SDL_SCANCODE_F5
    KEY_F6,             // SDL_SCANCODE_F6
    KEY_F7,             // SDL_SCANCODE_F7
    KEY_F8,             // SDL_SCANCODE_F8
    KEY_F9,             // SDL_SCANCODE_F9
    KEY_F10,            // SDL_SCANCODE_F10
    KEY_F11,            // SDL_SCANCODE_F11
    KEY_F12,            // SDL_SCANCODE_F12
    KEY_PRINT_SCREEN,   // SDL_SCANCODE_PRINTSCREEN
    KEY_SCROLL_LOCK,    // SDL_SCANCODE_SCROLLLOCK
    KEY_PAUSE,          // SDL_SCANCODE_PAUSE
    KEY_INSERT,         // SDL_SCANCODE_INSERT
    KEY_HOME,           // SDL_SCANCODE_HOME
    KEY_PAGE_UP,        // SDL_SCANCODE_PAGEUP
    KEY_DELETE,         // SDL_SCANCODE_DELETE
    KEY_END,            // SDL_SCANCODE_END
    KEY_PAGE_DOWN,      // SDL_SCANCODE_PAGEDOWN
    KEY_RIGHT,          // SDL_SCANCODE_RIGHT
    KEY_LEFT,           // SDL_SCANCODE_LEFT
    KEY_DOWN,           // SDL_SCANCODE_DOWN
    KEY_UP,             // SDL_SCANCODE_UP
    KEY_NUM_LOCK,       // SDL_SCANCODE_NUMLOCKCLEAR
    KEY_KP_DIVIDE,      // SDL_SCANCODE_KP_DIVIDE
    KEY_KP_MULTIPLY,    // SDL_SCANCODE_KP_MULTIPLY
    KEY_KP_SUBTRACT,    // SDL_SCANCODE_KP_MINUS
    KEY_KP_ADD,         // SDL_SCANCODE_KP_PLUS
    KEY_KP_ENTER,       // SDL_SCANCODE_KP_ENTER
    KEY_KP_1,           // SDL_SCANCODE_KP_1
    KEY_KP_2,           // SDL_SCANCODE_KP_2
    KEY_KP_3,           // SDL_SCANCODE_KP_3
    KEY_KP_4,           // SDL_SCANCODE_KP_4
    KEY_KP_5,           // SDL_SCANCODE_KP_5
    KEY_KP_6,           // SDL_SCANCODE_KP_6
    KEY_KP_7,           // SDL_SCANCODE_KP_7
    KEY_KP_8,           // SDL_SCANCODE_KP_8
    KEY_KP_9,           // SDL_SCANCODE_KP_9
    KEY_KP_0,           // SDL_SCANCODE_KP_0
    KEY_KP_DECIMAL      // SDL_SCANCODE_KP_PERIOD
};

static const int CursorsLUT[] = {
    SDL_SYSTEM_CURSOR_ARROW,       // 0  MOUSE_CURSOR_DEFAULT
    SDL_SYSTEM_CURSOR_ARROW,       // 1  MOUSE_CURSOR_ARROW
    SDL_SYSTEM_CURSOR_IBEAM,       // 2  MOUSE_CURSOR_IBEAM
    SDL_SYSTEM_CURSOR_CROSSHAIR,   // 3  MOUSE_CURSOR_CROSSHAIR
    SDL_SYSTEM_CURSOR_HAND,        // 4  MOUSE_CURSOR_POINTING_HAND
    SDL_SYSTEM_CURSOR_SIZEWE,      // 5  MOUSE_CURSOR_RESIZE_EW
    SDL_SYSTEM_CURSOR_SIZENS,      // 6  MOUSE_CURSOR_RESIZE_NS
    SDL_SYSTEM_CURSOR_SIZENWSE,    // 7  MOUSE_CURSOR_RESIZE_NWSE
    SDL_SYSTEM_CURSOR_SIZENESW,    // 8  MOUSE_CURSOR_RESIZE_NESW
    SDL_SYSTEM_CURSOR_SIZEALL,     // 9  MOUSE_CURSOR_RESIZE_ALL
    SDL_SYSTEM_CURSOR_NO           // 10 MOUSE_CURSOR_NOT_ALLOWED
    //SDL_SYSTEM_CURSOR_WAIT,      // No equivalent implemented on MouseCursor enum on raylib.h
    //SDL_SYSTEM_CURSOR_WAITARROW, // No equivalent implemented on MouseCursor enum on raylib.h
};

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
static int InitPlatform(void);                                      // Initialize platform (graphics, inputs and more)
static void ClosePlatform(void);                                    // Close platform
static KeyboardKey ConvertScancodeToKey(SDL_Scancode sdlScancode);  // Help convert SDL scancodes to raylib key

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
    if (CORE.Window.ready) return CORE.Window.shouldClose;
    else return true;
}

// Toggle fullscreen mode
void ToggleFullscreen(void)
{
    //SDL_SetWindowFullscreen
}

// Toggle borderless windowed mode
void ToggleBorderlessWindowed(void)
{
    //SDL_SetWindowFullscreen
}

// Set window state: maximized, if resizable
void MaximizeWindow(void)
{
    SDL_MaximizeWindow(platform.window);
    CORE.Window.flags |= FLAG_WINDOW_MAXIMIZED;
}

// Set window state: minimized
void MinimizeWindow(void)
{
    SDL_MinimizeWindow(platform.window);
    CORE.Window.flags |= FLAG_WINDOW_MINIMIZED;
}

// Set window state: not minimized/maximized
void RestoreWindow(void)
{
    SDL_ShowWindow(platform.window);
}

// Set window configuration state using flags
void SetWindowState(unsigned int flags)
{
    //SDL_HideWindow(platform.window);
}

// Clear window configuration state flags
void ClearWindowState(unsigned int flags)
{
    TRACELOG(LOG_WARNING, "ClearWindowState() not available on target platform");
}

// Set icon for window
void SetWindowIcon(Image image)
{
    TRACELOG(LOG_WARNING, "SetWindowIcon() not available on target platform");
}

// Set icon for window
void SetWindowIcons(Image *images, int count)
{
    TRACELOG(LOG_WARNING, "SetWindowIcons() not available on target platform");
}

// Set title for window
void SetWindowTitle(const char *title)
{
    SDL_SetWindowTitle(platform.window, title);

    CORE.Window.title = title;
}

// Set window position on screen (windowed mode)
void SetWindowPosition(int x, int y)
{
    SDL_SetWindowPosition(platform.window, x, y);

    CORE.Window.position.x = x;
    CORE.Window.position.y = y;
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
}

// Set window maximum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMaxSize(int width, int height)
{
    CORE.Window.screenMax.width = width;
    CORE.Window.screenMax.height = height;
}

// Set window dimensions
void SetWindowSize(int width, int height)
{
    SDL_SetWindowSize(platform.window, width, height);

    CORE.Window.screen.width = width;
    CORE.Window.screen.height = height;
}

// Set window opacity, value opacity is between 0.0 and 1.0
void SetWindowOpacity(float opacity)
{
    if (opacity >= 1.0f) opacity = 1.0f;
    else if (opacity <= 0.0f) opacity = 0.0f;

    SDL_SetWindowOpacity(platform.window, opacity);
}

// Set window focused
void SetWindowFocused(void)
{
    SDL_RaiseWindow(platform.window);
}

// Get native window handle
void *GetWindowHandle(void)
{
    return (void *)platform.window;
}

// Get number of monitors
int GetMonitorCount(void)
{
    int monitorCount = 0;

    monitorCount = SDL_GetNumVideoDisplays();

    return monitorCount;
}

// Get number of monitors
int GetCurrentMonitor(void)
{
    return SDL_GetWindowDisplayIndex(platform.window);
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
    int width = 0;

    int monitorCount = 0;
    monitorCount = SDL_GetNumVideoDisplays();

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        SDL_DisplayMode mode;
        SDL_GetCurrentDisplayMode(monitor, &mode);
        width = mode.w;
    }
    else TRACELOG(LOG_WARNING, "SDL: Failed to find selected monitor");

    return width;
}

// Get selected monitor height (currently used by monitor)
int GetMonitorHeight(int monitor)
{
    int height = 0;

    int monitorCount = 0;
    monitorCount = SDL_GetNumVideoDisplays();

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        SDL_DisplayMode mode;
        SDL_GetCurrentDisplayMode(monitor, &mode);
        height = mode.h;
    }
    else TRACELOG(LOG_WARNING, "SDL: Failed to find selected monitor");

    return height;
}

// Get selected monitor physical width in millimetres
int GetMonitorPhysicalWidth(int monitor)
{
    int width = 0;

    int monitorCount = 0;
    monitorCount = SDL_GetNumVideoDisplays();

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        float vdpi = 0.0f;
        SDL_GetDisplayDPI(monitor, NULL, NULL, &vdpi);
        SDL_DisplayMode mode;
        SDL_GetCurrentDisplayMode(monitor, &mode);
        // Calculate size on inches, then convert to millimeter
        if (vdpi > 0.0f) width = (mode.w/vdpi)*25.4f;
    }
    else TRACELOG(LOG_WARNING, "SDL: Failed to find selected monitor");

    return width;
}

// Get selected monitor physical height in millimetres
int GetMonitorPhysicalHeight(int monitor)
{
    int height = 0;

    int monitorCount = 0;
    monitorCount = SDL_GetNumVideoDisplays();

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        float vdpi = 0.0f;
        SDL_GetDisplayDPI(monitor, NULL, NULL, &vdpi);
        SDL_DisplayMode mode;
        SDL_GetCurrentDisplayMode(monitor, &mode);
        // Calculate size on inches, then convert to millimeter
        if (vdpi > 0.0f) height = (mode.h/vdpi)*25.4f;
    }
    else TRACELOG(LOG_WARNING, "SDL: Failed to find selected monitor");

    return height;
}

// Get selected monitor refresh rate
int GetMonitorRefreshRate(int monitor)
{
    int refresh = 0;

    int monitorCount = 0;
    monitorCount = SDL_GetNumVideoDisplays();

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        SDL_DisplayMode mode;
        SDL_GetCurrentDisplayMode(monitor, &mode);
        refresh = mode.refresh_rate;
    }
    else TRACELOG(LOG_WARNING, "SDL: Failed to find selected monitor");

    return refresh;
}

// Get the human-readable, UTF-8 encoded name of the selected monitor
const char *GetMonitorName(int monitor)
{
    int monitorCount = 0;
    monitorCount = SDL_GetNumVideoDisplays();

    if ((monitor >= 0) && (monitor < monitorCount)) return SDL_GetDisplayName(monitor);
    else TRACELOG(LOG_WARNING, "SDL: Failed to find selected monitor");

    return "";
}

// Get window position XY on monitor
Vector2 GetWindowPosition(void)
{
    int x = 0;
    int y = 0;

    SDL_GetWindowPosition(platform.window, &x, &y);

    return (Vector2){ (float)x, (float)y };
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
    SDL_SetClipboardText(text);
}

// Get clipboard text content
// NOTE: returned string must be freed with SDL_free()
const char *GetClipboardText(void)
{
    return SDL_GetClipboardText();
}

// Show mouse cursor
void ShowCursor(void)
{
    SDL_ShowCursor(SDL_ENABLE);

    CORE.Input.Mouse.cursorHidden = false;
}

// Hides mouse cursor
void HideCursor(void)
{
    SDL_ShowCursor(SDL_DISABLE);

    CORE.Input.Mouse.cursorHidden = true;
}

// Enables cursor (unlock cursor)
void EnableCursor(void)
{
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_ShowCursor(SDL_ENABLE);

    CORE.Input.Mouse.cursorHidden = false;
}

// Disables cursor (lock cursor)
void DisableCursor(void)
{
    SDL_SetRelativeMouseMode(SDL_TRUE);

    CORE.Input.Mouse.cursorHidden = true;
}

// Swap back buffer with front buffer (screen drawing)
void SwapScreenBuffer(void)
{
    SDL_GL_SwapWindow(platform.window);
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Misc
//----------------------------------------------------------------------------------

// Get elapsed time measure in seconds
double GetTime(void)
{
    unsigned int ms = SDL_GetTicks();    // Elapsed time in milliseconds since SDL_Init()
    double time = (double)ms/1000;
    return time;
}

// Open URL with default system browser (if available)
void OpenURL(const char *url)
{
    SDL_OpenURL(url);
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Inputs
//----------------------------------------------------------------------------------

// Set internal gamepad mappings
int SetGamepadMappings(const char *mappings)
{
    SDL_GameControllerAddMapping(mappings);
}

// Set mouse position XY
void SetMousePosition(int x, int y)
{
    CORE.Input.Mouse.currentPosition = (Vector2){ (float)x, (float)y };
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
}

// Set mouse cursor
void SetMouseCursor(int cursor)
{
    platform.cursor = SDL_CreateSystemCursor(CursorsLUT[cursor]);
    SDL_SetCursor(platform.cursor);

    CORE.Input.Mouse.cursor = cursor;
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

    // Reset key repeats
    for (int i = 0; i < MAX_KEYBOARD_KEYS; i++) CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;

    // Reset mouse wheel
    CORE.Input.Mouse.currentWheelMove.x = 0;
    CORE.Input.Mouse.currentWheelMove.y = 0;

    // Register previous mouse position
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;

    // Reset last gamepad button/axis registered state
    CORE.Input.Gamepad.lastButtonPressed = GAMEPAD_BUTTON_UNKNOWN;
    for (int i = 0; i < MAX_GAMEPADS; i++) CORE.Input.Gamepad.axisCount[i] = 0;

    // Register previous touch states
    for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.previousTouchState[i] = CORE.Input.Touch.currentTouchState[i];

    // Reset touch positions
    // TODO: It resets on target platform the mouse position and not filled again until a move-event,
    // so, if mouse is not moved it returns a (0, 0) position... this behaviour should be reviewed!
    //for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.position[i] = (Vector2){ 0, 0 };

    // Register previous keys states
    // NOTE: Android supports up to 260 keys
    for (int i = 0; i < 260; i++)
    {
        CORE.Input.Keyboard.previousKeyState[i] = CORE.Input.Keyboard.currentKeyState[i];
        CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }

    // Register previous mouse states
    for (int i = 0; i < MAX_MOUSE_BUTTONS; i++) CORE.Input.Mouse.previousButtonState[i] = CORE.Input.Mouse.currentButtonState[i];

    // Poll input events for current plaform
    //-----------------------------------------------------------------------------
    /*
    // WARNING: Indexes into this array are obtained by using SDL_Scancode values, not SDL_Keycode values
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    for (int i = 0; i < 256; ++i)
    {
        CORE.Input.Keyboard.currentKeyState[i] = keys[i];
        //if (keys[i]) TRACELOG(LOG_WARNING, "Pressed key: %i", i);
    }
    */

    SDL_Event event = { 0 };
    while (SDL_PollEvent(&event) != 0)
    {
        // All input events can be processed after polling
        switch (event.type)
        {
            case SDL_QUIT: CORE.Window.shouldClose = true; break;

            // Window events are also polled (Minimized, maximized, close...)
            case SDL_WINDOWEVENT:
            {
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_LEAVE:
                case SDL_WINDOWEVENT_HIDDEN:
                case SDL_WINDOWEVENT_MINIMIZED:
                case SDL_WINDOWEVENT_FOCUS_LOST:
                case SDL_WINDOWEVENT_ENTER:
                case SDL_WINDOWEVENT_SHOWN:
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                case SDL_WINDOWEVENT_MAXIMIZED:
                case SDL_WINDOWEVENT_RESTORED:
                default: break;
                }
            } break;

            // Keyboard events
            case SDL_KEYDOWN:
            {
                KeyboardKey key = ConvertScancodeToKey(event.key.keysym.scancode);
                if (key != KEY_NULL) CORE.Input.Keyboard.currentKeyState[key] = 1;

                // TODO: Put exitKey verification outside the switch?
                if (CORE.Input.Keyboard.currentKeyState[CORE.Input.Keyboard.exitKey])
                {
                    CORE.Window.shouldClose = true;
                }
            } break;

            case SDL_KEYUP:
            {
                KeyboardKey key = ConvertScancodeToKey(event.key.keysym.scancode);
                if (key != KEY_NULL) CORE.Input.Keyboard.currentKeyState[key] = 0;
            } break;

            // Check mouse events
            case SDL_MOUSEBUTTONDOWN:
            {
                CORE.Input.Mouse.currentButtonState[event.button.button - 1] = 1;
            } break;
            case SDL_MOUSEBUTTONUP:
            {
                CORE.Input.Mouse.currentButtonState[event.button.button - 1] = 0;
            } break;
            case SDL_MOUSEWHEEL:
            {
                CORE.Input.Mouse.currentWheelMove.x = (float)event.wheel.x;
                CORE.Input.Mouse.currentWheelMove.y = (float)event.wheel.y;
            } break;
            case SDL_MOUSEMOTION:
            {
                CORE.Input.Mouse.currentPosition.x = (float)event.motion.x;
                CORE.Input.Mouse.currentPosition.y = (float)event.motion.y;
            } break;

            // Check gamepad events
            case SDL_JOYAXISMOTION:
            {
                // Motion on gamepad 0
                if (event.jaxis.which == 0)
                {
                    // X axis motion
                    if (event.jaxis.axis == 0)
                    {
                        //...
                    }
                    // Y axis motion
                    else if (event.jaxis.axis == 1)
                    {
                        //...
                    }
                }
            } break;
            default: break;
        }
    }
    //-----------------------------------------------------------------------------
}


//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

// Initialize platform: graphics, inputs and more
static int InitPlatform(void)
{
    // Initialize SDL internal global state
    int result = SDL_Init(SDL_INIT_EVERYTHING);
    if (result < 0) { TRACELOG(LOG_WARNING, "SDL: Failed to initialize SDL"); return -1; }

    unsigned int flags = 0;
    flags |= SDL_WINDOW_SHOWN;
    flags |= SDL_WINDOW_OPENGL;
    flags |= SDL_WINDOW_INPUT_FOCUS;
    flags |= SDL_WINDOW_MOUSE_FOCUS;
    flags |= SDL_WINDOW_MOUSE_CAPTURE;  // Window has mouse captured

    // Check window creation flags
    if ((CORE.Window.flags & FLAG_FULLSCREEN_MODE) > 0)
    {
        CORE.Window.fullscreen = true;
        flags |= SDL_WINDOW_FULLSCREEN;
    }

    //if ((CORE.Window.flags & FLAG_WINDOW_HIDDEN) == 0) flags |= SDL_WINDOW_HIDDEN;
    if ((CORE.Window.flags & FLAG_WINDOW_UNDECORATED) > 0) flags |= SDL_WINDOW_BORDERLESS;
    if ((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) > 0) flags |= SDL_WINDOW_RESIZABLE;
    if ((CORE.Window.flags & FLAG_WINDOW_MINIMIZED) > 0) flags |= SDL_WINDOW_MINIMIZED;
    if ((CORE.Window.flags & FLAG_WINDOW_MAXIMIZED) > 0) flags |= SDL_WINDOW_MAXIMIZED;

    if ((CORE.Window.flags & FLAG_WINDOW_UNFOCUSED) > 0)
    {
        flags &= ~SDL_WINDOW_INPUT_FOCUS;
        flags &= ~SDL_WINDOW_MOUSE_FOCUS;
    }

    if ((CORE.Window.flags & FLAG_WINDOW_TOPMOST) > 0) flags |= SDL_WINDOW_ALWAYS_ON_TOP;
    if ((CORE.Window.flags & FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0) flags &= ~SDL_WINDOW_MOUSE_CAPTURE;

    if ((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) > 0) flags |= SDL_WINDOW_ALLOW_HIGHDPI;

    //if ((CORE.Window.flags & FLAG_WINDOW_TRANSPARENT) > 0) flags |= SDL_WINDOW_TRANSPARENT;     // Alternative: SDL_GL_ALPHA_SIZE = 8

    //if ((CORE.Window.flags & FLAG_FULLSCREEN_DESKTOP) > 0) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

    // NOTE: Some OpenGL context attributes must be set before window creation
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG | SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    if (CORE.Window.flags & FLAG_MSAA_4X_HINT)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    }

    // Init window
    platform.window = SDL_CreateWindow(CORE.Window.title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, CORE.Window.screen.width, CORE.Window.screen.height, flags);

    // Init OpenGL context
    platform.glContext = SDL_GL_CreateContext(platform.window);

    // Check window and glContext have been initialized succesfully
    if ((platform.window != NULL) && (platform.glContext != NULL))
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
    else { TRACELOG(LOG_FATAL, "PLATFORM: Failed to initialize graphic device"); return -1; }

    // Load OpenGL extensions
    // NOTE: GL procedures address loader is required to load extensions
    rlLoadExtensions(SDL_GL_GetProcAddress);


    // Init input gamepad
    if (SDL_NumJoysticks() >= 1)
    {
        SDL_Joystick *gamepad = SDL_JoystickOpen(0);
        //if (SDL_Joystick *gamepad == NULL) SDL_Log("WARNING: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
    }

    // Initialize hi-res timer
    //InitTimer();
    CORE.Time.previous = GetTime();     // Get time as double

    // Initialize base path for storage
    CORE.Storage.basePath = GetWorkingDirectory();

    return 0;
}

static void ClosePlatform(void)
{
    SDL_FreeCursor(platform.cursor); // Free cursor
    SDL_GL_DeleteContext(platform.glContext); // Deinitialize OpenGL context
    SDL_DestroyWindow(platform.window);
    SDL_Quit(); // Deinitialize SDL internal global state
}

static KeyboardKey ConvertScancodeToKey(SDL_Scancode sdlScancode)
{
    if (sdlScancode >= 0 && sdlScancode < SCANCODE_MAPPED_NUM)
    {
        return ScancodeToKey[sdlScancode];
    }
    return KEY_NULL; // No equivalent key in Raylib
}
// EOF
