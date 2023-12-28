/**********************************************************************************************
*
*   rcore_desktop_sdl - Functions to manage window, graphics device and inputs
*
*   PLATFORM: DESKTOP: SDL
*       - Windows (Win32, Win64)
*       - Linux (X11/Wayland desktop mode)
*       - Others (not tested)
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
*       - SDL 2 (main library): Windowing and inputs management
*       - gestures: Gestures system for touch-ready devices (or simulated from mouse inputs)
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

#include "SDL.h"                // SDL base library (window/rendered, input, timing... functionality)

#if defined(GRAPHICS_API_OPENGL_ES2)
    // It seems it does not need to be included to work
    //#include "SDL_opengles2.h"
#else
    #include "SDL_opengl.h"     // SDL OpenGL functionality (if required, instead of internal renderer)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    SDL_Window *window;
    SDL_GLContext glContext;

    SDL_Joystick *gamepad;
    SDL_Cursor *cursor;
    bool cursorRelative;
} PlatformData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern CoreData CORE;                   // Global CORE state context

static PlatformData platform = { 0 };   // Platform specific data

//----------------------------------------------------------------------------------
// Local Variables Definition
//----------------------------------------------------------------------------------
#define SCANCODE_MAPPED_NUM 232
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
    KEY_KP_DECIMAL,     // SDL_SCANCODE_KP_PERIOD
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0,
    KEY_LEFT_CONTROL,   //SDL_SCANCODE_LCTRL
    KEY_LEFT_SHIFT,     //SDL_SCANCODE_LSHIFT
    KEY_LEFT_ALT,       //SDL_SCANCODE_LALT
    KEY_LEFT_SUPER,     //SDL_SCANCODE_LGUI
    KEY_RIGHT_CONTROL,  //SDL_SCANCODE_RCTRL
    KEY_RIGHT_SHIFT,    //SDL_SCANCODE_RSHIFT
    KEY_RIGHT_ALT,      //SDL_SCANCODE_RALT
    KEY_RIGHT_SUPER     //SDL_SCANCODE_RGUI
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
int InitPlatform(void);                                      // Initialize platform (graphics, inputs and more)
void ClosePlatform(void);                                    // Close platform

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
    const int monitor = SDL_GetWindowDisplayIndex(platform.window);
    const int monitorCount = SDL_GetNumVideoDisplays();
    if ((monitor >= 0) && (monitor < monitorCount))
    {
        if ((CORE.Window.flags & FLAG_FULLSCREEN_MODE) > 0)
        {
            SDL_SetWindowFullscreen(platform.window, 0);
            CORE.Window.flags &= ~FLAG_FULLSCREEN_MODE;
            CORE.Window.fullscreen = false;
        }
        else
        {
            SDL_SetWindowFullscreen(platform.window, SDL_WINDOW_FULLSCREEN);
            CORE.Window.flags |= FLAG_FULLSCREEN_MODE;
            CORE.Window.fullscreen = true;
        }
    }
    else TRACELOG(LOG_WARNING, "SDL: Failed to find selected monitor");
}

// Toggle borderless windowed mode
void ToggleBorderlessWindowed(void)
{
    const int monitor = SDL_GetWindowDisplayIndex(platform.window);
    const int monitorCount = SDL_GetNumVideoDisplays();
    if ((monitor >= 0) && (monitor < monitorCount))
    {
        if ((CORE.Window.flags & FLAG_BORDERLESS_WINDOWED_MODE) > 0)
        {
            SDL_SetWindowFullscreen(platform.window, 0);
            CORE.Window.flags &= ~FLAG_BORDERLESS_WINDOWED_MODE;
        }
        else
        {
            SDL_SetWindowFullscreen(platform.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
            CORE.Window.flags |= FLAG_BORDERLESS_WINDOWED_MODE;
        }
    }
    else TRACELOG(LOG_WARNING, "SDL: Failed to find selected monitor");
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
    CORE.Window.flags |= flags;

    if (flags & FLAG_VSYNC_HINT)
    {
        SDL_GL_SetSwapInterval(1);
    }
    if (flags & FLAG_FULLSCREEN_MODE)
    {
        const int monitor = SDL_GetWindowDisplayIndex(platform.window);
        const int monitorCount = SDL_GetNumVideoDisplays();
        if ((monitor >= 0) && (monitor < monitorCount))
        {
            SDL_SetWindowFullscreen(platform.window, SDL_WINDOW_FULLSCREEN);
            CORE.Window.fullscreen = true;
        }
        else TRACELOG(LOG_WARNING, "SDL: Failed to find selected monitor");
    }
    if (flags & FLAG_WINDOW_RESIZABLE)
    {
        SDL_SetWindowResizable(platform.window, SDL_TRUE);
    }
    if (flags & FLAG_WINDOW_UNDECORATED)
    {
        SDL_SetWindowBordered(platform.window, SDL_FALSE);
    }
    if (flags & FLAG_WINDOW_HIDDEN)
    {
        SDL_HideWindow(platform.window);
    }
    if (flags & FLAG_WINDOW_MINIMIZED)
    {
        SDL_MinimizeWindow(platform.window);
    }
    if (flags & FLAG_WINDOW_MAXIMIZED)
    {
        SDL_MaximizeWindow(platform.window);
    }
    if (flags & FLAG_WINDOW_UNFOCUSED)
    {
        // NOTE: To be able to implement this part it seems that we should
        // do it ourselves, via `Windows.h`, `X11/Xlib.h` or even `Cocoa.h`
        TRACELOG(LOG_WARNING, "SetWindowState() - FLAG_WINDOW_UNFOCUSED is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & FLAG_WINDOW_TOPMOST)
    {
        SDL_SetWindowAlwaysOnTop(platform.window, SDL_FALSE);
    }
    if (flags & FLAG_WINDOW_ALWAYS_RUN)
    {
        TRACELOG(LOG_WARNING, "SetWindowState() - FLAG_WINDOW_ALWAYS_RUN is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & FLAG_WINDOW_TRANSPARENT)
    {
        TRACELOG(LOG_WARNING, "SetWindowState() - FLAG_WINDOW_TRANSPARENT is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & FLAG_WINDOW_HIGHDPI)
    {
        // NOTE: Such a function does not seem to exist
        TRACELOG(LOG_WARNING, "SetWindowState() - FLAG_WINDOW_HIGHDPI is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & FLAG_WINDOW_MOUSE_PASSTHROUGH)
    {
        //SDL_SetWindowGrab(platform.window, SDL_FALSE);
        TRACELOG(LOG_WARNING, "SetWindowState() - FLAG_WINDOW_MOUSE_PASSTHROUGH is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & FLAG_BORDERLESS_WINDOWED_MODE)
    {
        const int monitor = SDL_GetWindowDisplayIndex(platform.window);
        const int monitorCount = SDL_GetNumVideoDisplays();
        if ((monitor >= 0) && (monitor < monitorCount))
        {
            SDL_SetWindowFullscreen(platform.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
        else TRACELOG(LOG_WARNING, "SDL: Failed to find selected monitor");
    }
    if (flags & FLAG_MSAA_4X_HINT)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1); // Enable multisampling buffers
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // Enable multisampling
    }
    if (flags & FLAG_INTERLACED_HINT)
    {
        TRACELOG(LOG_WARNING, "SetWindowState() - FLAG_INTERLACED_HINT is not supported on PLATFORM_DESKTOP_SDL");
    }
}

// Clear window configuration state flags
void ClearWindowState(unsigned int flags)
{
    CORE.Window.flags &= ~flags;

    if (flags & FLAG_VSYNC_HINT)
    {
        SDL_GL_SetSwapInterval(0);
    }
    if (flags & FLAG_FULLSCREEN_MODE)
    {
        SDL_SetWindowFullscreen(platform.window, 0);
        CORE.Window.fullscreen = false;
    }
    if (flags & FLAG_WINDOW_RESIZABLE)
    {
        SDL_SetWindowResizable(platform.window, SDL_FALSE);
    }
    if (flags & FLAG_WINDOW_UNDECORATED)
    {
        SDL_SetWindowBordered(platform.window, SDL_TRUE);
    }
    if (flags & FLAG_WINDOW_HIDDEN)
    {
        SDL_ShowWindow(platform.window);
    }
    if (flags & FLAG_WINDOW_MINIMIZED)
    {
        SDL_RestoreWindow(platform.window);
    }
    if (flags & FLAG_WINDOW_MAXIMIZED)
    {
        SDL_RestoreWindow(platform.window);
    }
    if (flags & FLAG_WINDOW_UNFOCUSED)
    {
        //SDL_RaiseWindow(platform.window);
        TRACELOG(LOG_WARNING, "ClearWindowState() - FLAG_WINDOW_UNFOCUSED is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & FLAG_WINDOW_TOPMOST)
    {
        SDL_SetWindowAlwaysOnTop(platform.window, SDL_FALSE);
    }
    if (flags & FLAG_WINDOW_ALWAYS_RUN)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState() - FLAG_WINDOW_ALWAYS_RUN is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & FLAG_WINDOW_TRANSPARENT)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState() - FLAG_WINDOW_TRANSPARENT is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & FLAG_WINDOW_HIGHDPI)
    {
        // NOTE: There also doesn't seem to be a feature to disable high DPI once enabled
        TRACELOG(LOG_WARNING, "ClearWindowState() - FLAG_WINDOW_HIGHDPI is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & FLAG_WINDOW_MOUSE_PASSTHROUGH)
    {
        //SDL_SetWindowGrab(platform.window, SDL_TRUE);
        TRACELOG(LOG_WARNING, "ClearWindowState() - FLAG_WINDOW_MOUSE_PASSTHROUGH is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & FLAG_BORDERLESS_WINDOWED_MODE)
    {
        SDL_SetWindowFullscreen(platform.window, 0);
    }
    if (flags & FLAG_MSAA_4X_HINT)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0); // Disable multisampling buffers
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0); // Disable multisampling
    }
    if (flags & FLAG_INTERLACED_HINT)
    {
        TRACELOG(LOG_WARNING, "ClearWindowState() - FLAG_INTERLACED_HINT is not supported on PLATFORM_DESKTOP_SDL");
    }
}

// Set icon for window
void SetWindowIcon(Image image)
{
    SDL_Surface* iconSurface = NULL;

    Uint32 rmask, gmask, bmask, amask;
    int depth = 0;  // Depth in bits
    int pitch = 0;  // Pixel spacing (pitch) in bytes

    switch (image.format)
    {
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
            rmask = 0xFF, gmask = 0;
            bmask = 0, amask = 0;
            depth = 8, pitch = image.width;
            break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
            rmask = 0xFF, gmask = 0xFF00;
            bmask = 0, amask = 0;
            depth = 16, pitch = image.width * 2;
            break;
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
            rmask = 0xF800, gmask = 0x07E0;
            bmask = 0x001F, amask = 0;
            depth = 16, pitch = image.width * 2;
            break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8: // Uses BGR for 24-bit
            rmask = 0x0000FF, gmask = 0x00FF00;
            bmask = 0xFF0000, amask = 0;
            depth = 24, pitch = image.width * 3;
            break;
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
            rmask = 0xF800, gmask = 0x07C0;
            bmask = 0x003E, amask = 0x0001;
            depth = 16, pitch = image.width * 2;
            break;
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
            rmask = 0xF000, gmask = 0x0F00;
            bmask = 0x00F0, amask = 0x000F;
            depth = 16, pitch = image.width * 2;
            break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
            rmask = 0xFF000000, gmask = 0x00FF0000;
            bmask = 0x0000FF00, amask = 0x000000FF;
            depth = 32, pitch = image.width * 4;
            break;
        case PIXELFORMAT_UNCOMPRESSED_R32:
            rmask = 0xFFFFFFFF, gmask = 0;
            bmask = 0, amask = 0;
            depth = 32, pitch = image.width * 4;
            break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32:
            rmask = 0xFFFFFFFF, gmask = 0xFFFFFFFF;
            bmask = 0xFFFFFFFF, amask = 0;
            depth = 96, pitch = image.width * 12;
            break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
            rmask = 0xFFFFFFFF, gmask = 0xFFFFFFFF;
            bmask = 0xFFFFFFFF, amask = 0xFFFFFFFF;
            depth = 128, pitch = image.width * 16;
            break;
        case PIXELFORMAT_UNCOMPRESSED_R16:
            rmask = 0xFFFF, gmask = 0;
            bmask = 0, amask = 0;
            depth = 16, pitch = image.width * 2;
            break;
        case PIXELFORMAT_UNCOMPRESSED_R16G16B16:
            rmask = 0xFFFF, gmask = 0xFFFF;
            bmask = 0xFFFF, amask = 0;
            depth = 48, pitch = image.width * 6;
            break;
        case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
            rmask = 0xFFFF, gmask = 0xFFFF;
            bmask = 0xFFFF, amask = 0xFFFF;
            depth = 64, pitch = image.width * 8;
            break;
        default:
            // Compressed formats are not supported
            return;
    }

    iconSurface = SDL_CreateRGBSurfaceFrom(
        image.data, image.width, image.height, depth, pitch,
        rmask, gmask, bmask, amask
    );

    if (iconSurface)
    {
        SDL_SetWindowIcon(platform.window, iconSurface);
        SDL_FreeSurface(iconSurface);
    }
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
    const int monitorCount = SDL_GetNumVideoDisplays();
    if ((monitor >= 0) && (monitor < monitorCount))
    {
        // NOTE:
        // 1. SDL started supporting moving exclusive fullscreen windows between displays on SDL3,
        //    see commit https://github.com/libsdl-org/SDL/commit/3f5ef7dd422057edbcf3e736107e34be4b75d9ba
        // 2. A workaround for SDL2 is leaving fullscreen, moving the window, then entering full screen again.
        const bool wasFullscreen = ((CORE.Window.flags & FLAG_FULLSCREEN_MODE) > 0) ? true : false;

        const int screenWidth = CORE.Window.screen.width;
        const int screenHeight = CORE.Window.screen.height;
        SDL_Rect usableBounds;
        if (SDL_GetDisplayUsableBounds(monitor, &usableBounds) == 0)
        {
            if (wasFullscreen == 1) ToggleFullscreen(); // Leave fullscreen.

            // If the screen size is larger than the monitor usable area, anchor it on the top left corner, otherwise, center it
            if ((screenWidth >= usableBounds.w) || (screenHeight >= usableBounds.h))
            {
                // NOTE:
                // 1. There's a known issue where if the window larger than the target display bounds,
                //    when moving the windows to that display, the window could be clipped back
                //    ending up positioned partly outside the target display.
                // 2. The workaround for that is, previously to moving the window,
                //    setting the window size to the target display size, so they match.
                // 3. It wasn't done here because we can't assume changing the window size automatically
                //    is acceptable behavior by the user.
                SDL_SetWindowPosition(platform.window, usableBounds.x, usableBounds.y);
                CORE.Window.position.x = usableBounds.x;
                CORE.Window.position.y = usableBounds.y;
            }
            else
            {
                const int x = usableBounds.x + (usableBounds.w/2) - (screenWidth/2);
                const int y = usableBounds.y + (usableBounds.h/2) - (screenHeight/2);
                SDL_SetWindowPosition(platform.window, x, y);
                CORE.Window.position.x = x;
                CORE.Window.position.y = y;
            }

            if (wasFullscreen == 1) ToggleFullscreen(); // Re-enter fullscreen
        }
        else TRACELOG(LOG_WARNING, "SDL: Failed to get selected display usable bounds");
    }
    else TRACELOG(LOG_WARNING, "SDL: Failed to find selected monitor");
}

// Set window minimum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMinSize(int width, int height)
{
    SDL_SetWindowMinimumSize(platform.window, width, height);

    CORE.Window.screenMin.width = width;
    CORE.Window.screenMin.height = height;
}

// Set window maximum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMaxSize(int width, int height)
{
    SDL_SetWindowMaximumSize(platform.window, width, height);

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
    int currentMonitor = 0;

    currentMonitor = SDL_GetWindowDisplayIndex(platform.window);

    return currentMonitor;
}

// Get selected monitor position
Vector2 GetMonitorPosition(int monitor)
{
    const int monitorCount = SDL_GetNumVideoDisplays();
    if ((monitor >= 0) && (monitor < monitorCount))
    {
        SDL_Rect displayBounds;
        if (SDL_GetDisplayUsableBounds(monitor, &displayBounds) == 0)
        {
            return (Vector2){ (float)displayBounds.x, (float)displayBounds.y };
        }
        else TRACELOG(LOG_WARNING, "SDL: Failed to get selected display usable bounds");
    }
    else TRACELOG(LOG_WARNING, "SDL: Failed to find selected monitor");
    return (Vector2){ 0.0f, 0.0f };
}

// Get selected monitor width (currently used by monitor)
int GetMonitorWidth(int monitor)
{
    int width = 0;

    const int monitorCount = SDL_GetNumVideoDisplays();
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

    const int monitorCount = SDL_GetNumVideoDisplays();
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

    const int monitorCount = SDL_GetNumVideoDisplays();
    if ((monitor >= 0) && (monitor < monitorCount))
    {
        float ddpi = 0.0f;
        SDL_GetDisplayDPI(monitor, &ddpi, NULL, NULL);
        SDL_DisplayMode mode;
        SDL_GetCurrentDisplayMode(monitor, &mode);
        // Calculate size on inches, then convert to millimeter
        if (ddpi > 0.0f) width = (mode.w/ddpi)*25.4f;
    }
    else TRACELOG(LOG_WARNING, "SDL: Failed to find selected monitor");

    return width;
}

// Get selected monitor physical height in millimetres
int GetMonitorPhysicalHeight(int monitor)
{
    int height = 0;

    const int monitorCount = SDL_GetNumVideoDisplays();
    if ((monitor >= 0) && (monitor < monitorCount))
    {
        float ddpi = 0.0f;
        SDL_GetDisplayDPI(monitor, &ddpi, NULL, NULL);
        SDL_DisplayMode mode;
        SDL_GetCurrentDisplayMode(monitor, &mode);
        // Calculate size on inches, then convert to millimeter
        if (ddpi > 0.0f) height = (mode.h/ddpi)*25.4f;
    }
    else TRACELOG(LOG_WARNING, "SDL: Failed to find selected monitor");

    return height;
}

// Get selected monitor refresh rate
int GetMonitorRefreshRate(int monitor)
{
    int refresh = 0;

    const int monitorCount = SDL_GetNumVideoDisplays();
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
    const int monitorCount = SDL_GetNumVideoDisplays();

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
    Vector2 scale = { 1.0f, 1.0f };

    // NOTE: SDL_GetWindowDisplayScale was only added on SDL3
    //       see https://wiki.libsdl.org/SDL3/SDL_GetWindowDisplayScale
    // TODO: Implement the window scale factor calculation manually.
    TRACELOG(LOG_WARNING, "GetWindowScaleDPI() not implemented on target platform");

    return scale;
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

    platform.cursorRelative = false;
    CORE.Input.Mouse.cursorHidden = false;
}

// Disables cursor (lock cursor)
void DisableCursor(void)
{
    SDL_SetRelativeMouseMode(SDL_TRUE);

    platform.cursorRelative = true;
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
// NOTE: This function is only safe to use if you control the URL given.
// A user could craft a malicious string performing another action.
// Only call this function yourself not with user input or make sure to check the string yourself.
// Ref: https://github.com/raysan5/raylib/issues/686
void OpenURL(const char *url)
{
    // Security check to (partially) avoid malicious code
    if (strchr(url, '\'') != NULL) TRACELOG(LOG_WARNING, "SYSTEM: Provided URL could be potentially malicious, avoid [\'] character");
    else SDL_OpenURL(url);
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Inputs
//----------------------------------------------------------------------------------

// Set internal gamepad mappings
int SetGamepadMappings(const char *mappings)
{
    return SDL_GameControllerAddMapping(mappings);
}

// Set mouse position XY
void SetMousePosition(int x, int y)
{
    SDL_WarpMouseInWindow(platform.window, x, y);

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

    // Reset mouse wheel
    CORE.Input.Mouse.currentWheelMove.x = 0;
    CORE.Input.Mouse.currentWheelMove.y = 0;

    // Register previous mouse position
    if (platform.cursorRelative) CORE.Input.Mouse.currentPosition = (Vector2){ 0.0f, 0.0f };
    else CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;

    // Reset last gamepad button/axis registered state
    CORE.Input.Gamepad.lastButtonPressed = GAMEPAD_BUTTON_UNKNOWN;
    for (int i = 0; i < MAX_GAMEPADS; i++) CORE.Input.Gamepad.axisCount[i] = 0;

    // Register previous touch states
    for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.previousTouchState[i] = CORE.Input.Touch.currentTouchState[i];

    // Reset touch positions
    // TODO: It resets on target platform the mouse position and not filled again until a move-event,
    // so, if mouse is not moved it returns a (0, 0) position... this behaviour should be reviewed!
    //for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.position[i] = (Vector2){ 0, 0 };

    // Map touch position to mouse position for convenience
    // WARNING: If the target desktop device supports touch screen, this behavious should be reviewed!
    // https://www.codeproject.com/Articles/668404/Programming-for-Multi-Touch
    // https://docs.microsoft.com/en-us/windows/win32/wintouch/getting-started-with-multi-touch-messages
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
    /*
    // WARNING: Indexes into this array are obtained by using SDL_Scancode values, not SDL_Keycode values
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    for (int i = 0; i < 256; ++i)
    {
        CORE.Input.Keyboard.currentKeyState[i] = keys[i];
        //if (keys[i]) TRACELOG(LOG_WARNING, "Pressed key: %i", i);
    }
    */

    CORE.Window.resizedLastFrame = false;

    SDL_Event event = { 0 };
    while (SDL_PollEvent(&event) != 0)
    {
        // All input events can be processed after polling
        switch (event.type)
        {
            case SDL_QUIT: CORE.Window.shouldClose = true; break;

            case SDL_DROPFILE:      // Dropped file
            {
                if (CORE.Window.dropFileCount == 0)
                {
                    // When a new file is dropped, we reserve a fixed number of slots for all possible dropped files
                    // at the moment we limit the number of drops at once to 1024 files but this behaviour should probably be reviewed
                    // TODO: Pointers should probably be reallocated for any new file added...
                    CORE.Window.dropFilepaths = (char **)RL_CALLOC(1024, sizeof(char *));

                    CORE.Window.dropFilepaths[CORE.Window.dropFileCount] = (char *)RL_CALLOC(MAX_FILEPATH_LENGTH, sizeof(char));
                    strcpy(CORE.Window.dropFilepaths[CORE.Window.dropFileCount], event.drop.file);
                    SDL_free(event.drop.file);

                    CORE.Window.dropFileCount++;
                }
                else if (CORE.Window.dropFileCount < 1024)
                {
                    CORE.Window.dropFilepaths[CORE.Window.dropFileCount] = (char *)RL_CALLOC(MAX_FILEPATH_LENGTH, sizeof(char));
                    strcpy(CORE.Window.dropFilepaths[CORE.Window.dropFileCount], event.drop.file);
                    SDL_free(event.drop.file);

                    CORE.Window.dropFileCount++;
                }
                else TRACELOG(LOG_WARNING, "FILE: Maximum drag and drop files at once is limited to 1024 files!");

            } break;

            // Window events are also polled (Minimized, maximized, close...)
            case SDL_WINDOWEVENT:
            {
                switch (event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        const int width = event.window.data1;
                        const int height = event.window.data2;
                        SetupViewport(width, height);
                        CORE.Window.screen.width = width;
                        CORE.Window.screen.height = height;
                        CORE.Window.currentFbo.width = width;
                        CORE.Window.currentFbo.height = height;
                        CORE.Window.resizedLastFrame = true;
                    } break;
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

                if (event.key.repeat) CORE.Input.Keyboard.keyRepeatInFrame[key] = 1;

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

            case SDL_TEXTINPUT:
            {
                // NOTE: event.text.text data comes an UTF-8 text sequence but we register codepoints (int)

                int codepointSize = 0;

                // Check if there is space available in the key queue
                if (CORE.Input.Keyboard.keyPressedQueueCount < MAX_KEY_PRESSED_QUEUE)
                {
                    // Add character (key) to the queue
                    CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = GetCodepointNext(event.text.text, &codepointSize);
                    CORE.Input.Keyboard.keyPressedQueueCount++;
                }

                // Check if there is space available in the queue
                if (CORE.Input.Keyboard.charPressedQueueCount < MAX_CHAR_PRESSED_QUEUE)
                {
                    // Add character (codepoint) to the queue
                    CORE.Input.Keyboard.charPressedQueue[CORE.Input.Keyboard.charPressedQueueCount] = GetCodepointNext(event.text.text, &codepointSize);
                    CORE.Input.Keyboard.charPressedQueueCount++;
                }
            } break;

            // Check mouse events
            case SDL_MOUSEBUTTONDOWN:
            {
                // NOTE: SDL2 mouse button order is LEFT, MIDDLE, RIGHT, but raylib uses LEFT, RIGHT, MIDDLE like GLFW
                //       The following conditions align SDL with raylib.h MouseButton enum order
                int btn = event.button.button - 1;
                if (btn == 2) btn = 1;
                else if (btn == 1) btn = 2;

                CORE.Input.Mouse.currentButtonState[btn] = 1;
                CORE.Input.Touch.currentTouchState[btn] = 1;

                touchAction = 1;
            } break;
            case SDL_MOUSEBUTTONUP:
            {
                // NOTE: SDL2 mouse button order is LEFT, MIDDLE, RIGHT, but raylib uses LEFT, RIGHT, MIDDLE like GLFW
                //       The following conditions align SDL with raylib.h MouseButton enum order
                int btn = event.button.button - 1;
                if (btn == 2) btn = 1;
                else if (btn == 1) btn = 2;

                CORE.Input.Mouse.currentButtonState[btn] = 0;
                CORE.Input.Touch.currentTouchState[btn] = 0;

                touchAction = 0;
            } break;
            case SDL_MOUSEWHEEL:
            {
                CORE.Input.Mouse.currentWheelMove.x = (float)event.wheel.x;
                CORE.Input.Mouse.currentWheelMove.y = (float)event.wheel.y;
            } break;
            case SDL_MOUSEMOTION:
            {
                if (platform.cursorRelative)
                {
                    CORE.Input.Mouse.currentPosition.x = (float)event.motion.xrel;
                    CORE.Input.Mouse.currentPosition.y = (float)event.motion.yrel;
                    CORE.Input.Mouse.previousPosition = (Vector2){ 0.0f, 0.0f };
                }
                else
                {
                    CORE.Input.Mouse.currentPosition.x = (float)event.motion.x;
                    CORE.Input.Mouse.currentPosition.y = (float)event.motion.y;
                }

                CORE.Input.Touch.position[0] = CORE.Input.Mouse.currentPosition;
                touchAction = 2;
            } break;

            // Check touch events
            // NOTE: These cases need to be reviewed on a real touch screen
            case SDL_FINGERDOWN:
            {
                const int touchId = (int)event.tfinger.fingerId;
                CORE.Input.Touch.currentTouchState[touchId] = 1;
                CORE.Input.Touch.position[touchId].x = event.tfinger.x * CORE.Window.screen.width;
                CORE.Input.Touch.position[touchId].y = event.tfinger.y * CORE.Window.screen.height;

                touchAction = 1;
                realTouch = true;
            } break;
            case SDL_FINGERUP:
            {
                const int touchId = (int)event.tfinger.fingerId;
                CORE.Input.Touch.currentTouchState[touchId] = 0;
                CORE.Input.Touch.position[touchId].x = event.tfinger.x * CORE.Window.screen.width;
                CORE.Input.Touch.position[touchId].y = event.tfinger.y * CORE.Window.screen.height;

                touchAction = 0;
                realTouch = true;
            } break;
            case SDL_FINGERMOTION:
            {
                const int touchId = (int)event.tfinger.fingerId;
                CORE.Input.Touch.position[touchId].x = event.tfinger.x * CORE.Window.screen.width;
                CORE.Input.Touch.position[touchId].y = event.tfinger.y * CORE.Window.screen.height;

                touchAction = 2;
                realTouch = true;
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
    // Initialize SDL internal global state
    int result = SDL_Init(SDL_INIT_EVERYTHING);
    if (result < 0) { TRACELOG(LOG_WARNING, "SDL: Failed to initialize SDL"); return -1; }

    // Initialize graphic device: display/window and graphic context
    //----------------------------------------------------------------------------
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

    // Check selection OpenGL version
    if (rlGetVersion() == RL_OPENGL_21)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    }
    else if (rlGetVersion() == RL_OPENGL_33)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#if defined(__APPLE__)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);  // OSX Requires forward compatibility
#else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
    }
    else if (rlGetVersion() == RL_OPENGL_43)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#if defined(RLGL_ENABLE_OPENGL_DEBUG_CONTEXT)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);   // Enable OpenGL Debug Context
#endif
    }
    else if (rlGetVersion() == RL_OPENGL_ES_20)                 // Request OpenGL ES 2.0 context
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    }
    else if (rlGetVersion() == RL_OPENGL_ES_30)                 // Request OpenGL ES 3.0 context
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    }

    if (CORE.Window.flags & FLAG_VSYNC_HINT)
    {
        SDL_GL_SetSwapInterval(1);
    }

    if (CORE.Window.flags & FLAG_MSAA_4X_HINT)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    }

    // Init window
    platform.window = SDL_CreateWindow(CORE.Window.title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, CORE.Window.screen.width, CORE.Window.screen.height, flags);

    // Init OpenGL context
    platform.glContext = SDL_GL_CreateContext(platform.window);

    // Check window and glContext have been initialized successfully
    if ((platform.window != NULL) && (platform.glContext != NULL))
    {
        CORE.Window.ready = true;

        SDL_DisplayMode displayMode = { 0 };
        SDL_GetCurrentDisplayMode(GetCurrentMonitor(), &displayMode);

        CORE.Window.display.width = displayMode.w;
        CORE.Window.display.height = displayMode.h;

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

    // Load OpenGL extensions
    // NOTE: GL procedures address loader is required to load extensions
    rlLoadExtensions(SDL_GL_GetProcAddress);
    //----------------------------------------------------------------------------

    // Initialize input events system
    //----------------------------------------------------------------------------
    if (SDL_NumJoysticks() >= 1)
    {
        platform.gamepad = SDL_JoystickOpen(0);
        //if (platform.gamepadgamepad == NULL) TRACELOG(LOG_WARNING, "PLATFORM: Unable to open game controller [ERROR: %s]", SDL_GetError());
    }

    // Disable mouse events being interpreted as touch events
    // NOTE: This is wanted because there are SDL_FINGER* events available which provide unique data
    //       Due to the way PollInputEvents() and rgestures.h are currently implemented, setting this won't break SUPPORT_MOUSE_GESTURES
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");

    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    //----------------------------------------------------------------------------

    // Initialize timing system
    //----------------------------------------------------------------------------
    // NOTE: No need to call InitTimer(), let SDL manage it internally
    CORE.Time.previous = GetTime();     // Get time as double

    #if defined(_WIN32) && defined(SUPPORT_WINMM_HIGHRES_TIMER) && !defined(SUPPORT_BUSY_WAIT_LOOP)
    SDL_SetHint(SDL_HINT_TIMER_RESOLUTION, "1");     // SDL equivalent of timeBeginPeriod() and timeEndPeriod()
    #endif
    //----------------------------------------------------------------------------

    // Initialize storage system
    //----------------------------------------------------------------------------
    CORE.Storage.basePath = GetWorkingDirectory();  // Define base path for storage
    //----------------------------------------------------------------------------

    TRACELOG(LOG_INFO, "PLATFORM: DESKTOP (SDL): Initialized successfully");

    return 0;
}

// Close platform
void ClosePlatform(void)
{
    SDL_FreeCursor(platform.cursor); // Free cursor
    SDL_GL_DeleteContext(platform.glContext); // Deinitialize OpenGL context
    SDL_DestroyWindow(platform.window);
    SDL_Quit(); // Deinitialize SDL internal global state
}

// Scancode to keycode mapping
static KeyboardKey ConvertScancodeToKey(SDL_Scancode sdlScancode)
{
    if (sdlScancode >= 0 && sdlScancode < SCANCODE_MAPPED_NUM)
    {
        return ScancodeToKey[sdlScancode];
    }
    return KEY_NULL; // No equivalent key in Raylib
}
// EOF
