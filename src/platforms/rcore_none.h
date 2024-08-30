/**********************************************************************************************
*
*   rcore_none - Functions to manage window, graphics device and inputs
*
*   PLATFORM: NONE
*
*   LIMITATIONS:
*       - Limitation 01
*       - Limitation 02
*
*   POSSIBLE IMPROVEMENTS:
*       -   For OverrideInternalFunction, we should maybe consider hashing the input string 
*           so that we can use a switch case? But to be fair this also isn't a function you should be running on loop.
*       -   We could introduce a config flag to disable the if/else check if a function is set, letting the user
*           improve the performance at the cost of segfaults if a function pointer isn't set.
*
*   ADDITIONAL NOTES:
*       - You still need a platform that has OpenGL, or a port of it, to use this.
*       - There is no responsibility for the maintainers of Raylib to keep a stable API here.
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

#ifndef RCORE_NONE_H
#define RCORE_NONE_H

#include <stddef.h>

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
} PlatformData;


//----------------------------------------------------------------------------------
// Macros Declaration
//----------------------------------------------------------------------------------
// NOTE(IOI_XD):
// Yes, macros suck. However, there is a right way to use them, I believe I have taken said precautions, and I believe it is worth it here because
// it would make updating this file much easier.
// That said, these are functions that will not have to be modified past when this is merged, except to rename/add them.

// Macro that lets us put two vararg sections in one macro
#define LIST(...) __VA_ARGS__

// The code for warning the user that a function hasn't been overriden. On PLATFORM_NONE, this isn't emitted.
#ifndef PLATFORM_OFFSCREEN
#define FUNCTION_MISSING_WARNING(fn_name) TRACELOG(LOG_WARNING, #fn_name " was called but not overriden by the user");
#else
#define FUNCTION_MISSING_WARNING(fn_name)
#endif

// Generate a callback
#define GEN_CALLBACK_HEAD(fn_ret, ret_value, fn_name, named_args, args) \
    typedef fn_ret (*fn_name##Callback)(LIST named_args); \
    static fn_name##Callback * internal##fn_name##Callback = NULL; \
    fn_ret fn_name(LIST named_args);

// Generate a callback that returns a void.
#define GEN_CALLBACK_HEAD_VOID(fn_name, named_args, args) \
    typedef void (*fn_name##Callback)(LIST named_args); \
    static fn_name##Callback * internal##fn_name##Callback = NULL; \
    void fn_name(LIST named_args);

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
// NOTE: Functions declaration is provided by raylib.h

// We start with any functions that unambigiously must be overriden.
GEN_CALLBACK_HEAD_VOID(ClosePlatform, (void), ())
GEN_CALLBACK_HEAD_VOID(ToggleFullscreen, (void), ())
GEN_CALLBACK_HEAD_VOID(ToggleBorderlessWindowed, (void), ())
GEN_CALLBACK_HEAD_VOID(MaximizeWindow, (void), ())
GEN_CALLBACK_HEAD_VOID(MinimizeWindow, (void), ())
GEN_CALLBACK_HEAD_VOID(RestoreWindow, (void), ())
GEN_CALLBACK_HEAD_VOID(SetWindowIcon, (Image image), (image))
GEN_CALLBACK_HEAD_VOID(SetWindowIcons, (Image *images, int count), (images, count))
GEN_CALLBACK_HEAD_VOID(SetWindowTitle, (const char *title), (title))
GEN_CALLBACK_HEAD_VOID(SetWindowPosition, (int x, int y), (x,y))
GEN_CALLBACK_HEAD_VOID(SetWindowMonitor, (int monitor), (monitor))
GEN_CALLBACK_HEAD_VOID(SetWindowOpacity, (float opacity), (opacity))
GEN_CALLBACK_HEAD_VOID(SetWindowFocused, (void), ())
GEN_CALLBACK_HEAD_VOID(EnableCursor, (void), ())
GEN_CALLBACK_HEAD_VOID(DisableCursor, (void), ())
GEN_CALLBACK_HEAD_VOID(SwapScreenBuffer, (void), ())
GEN_CALLBACK_HEAD_VOID(SetGamepadVibration, (int gamepad, float leftMotor, float rightMotor), (gamepad,leftMotor,rightMotor))
GEN_CALLBACK_HEAD_VOID(SetMousePosition, (int x, int y), (x,y))
GEN_CALLBACK_HEAD_VOID(SetMouseCursor, (int cursor), (cursor))
GEN_CALLBACK_HEAD_VOID(PollInputEvents, (void), ())
GEN_CALLBACK_HEAD_VOID(SetClipboardText, (const char *text), (text))
GEN_CALLBACK_HEAD(int, 0, SetGamepadMappings, (const char *mappings), (mappings))
GEN_CALLBACK_HEAD(void *, NULL, GetWindowHandle, (void), ())
GEN_CALLBACK_HEAD(Vector2, Vector2Zero(), GetMonitorPosition, (int monitor), (monitor))
GEN_CALLBACK_HEAD(int, 0, GetMonitorWidth, (int monitor), (monitor))
GEN_CALLBACK_HEAD(int, 0, GetMonitorHeight, (int monitor), (monitor))
GEN_CALLBACK_HEAD(int, 0, GetMonitorPhysicalWidth, (int monitor), (monitor))
GEN_CALLBACK_HEAD(int, 0, GetMonitorPhysicalHeight, (int monitor), (monitor))
GEN_CALLBACK_HEAD(int, 0, GetMonitorRefreshRate, (int monitor), (monitor))
GEN_CALLBACK_HEAD(const char*, "", GetMonitorName, (int monitor), (monitor))
GEN_CALLBACK_HEAD(Vector2, Vector2Zero(), GetWindowPosition, (void), ())
GEN_CALLBACK_HEAD(Vector2, Vector2Zero(), GetWindowScaleDPI, (void), ())
GEN_CALLBACK_HEAD(const char *, "", GetClipboardText, (void), ())
GEN_CALLBACK_HEAD(const char *, "", GetKeyName, (int key), (key))
GEN_CALLBACK_HEAD(double, 0.0, GetTime, (void), ())
GEN_CALLBACK_HEAD_VOID(OpenURL, (const char *url), (url))

// We then move on to functions that can be overriden, but on the offscreen platform, should have things set by default.
GEN_CALLBACK_HEAD(int, 0, InitPlatform, (void), ())
GEN_CALLBACK_HEAD(bool, true, WindowShouldClose, (void), ())
GEN_CALLBACK_HEAD(int, 0, GetMonitorCount, (void), ())
GEN_CALLBACK_HEAD(int, 0, GetCurrentMonitor, (void), ())
GEN_CALLBACK_HEAD_VOID(ShowCursor, (void), ())
GEN_CALLBACK_HEAD_VOID(HideCursor, (void), ())
GEN_CALLBACK_HEAD_VOID(SetWindowState, (unsigned int flags), (flags))
GEN_CALLBACK_HEAD_VOID(ClearWindowState, (unsigned int flags), (flags))
GEN_CALLBACK_HEAD_VOID(SetWindowMinSize, (int width, int height), (width,height))
GEN_CALLBACK_HEAD_VOID(SetWindowMaxSize, (int width, int height), (width,height))
GEN_CALLBACK_HEAD_VOID(SetWindowSize, (int width, int height), (width,height))

// Generate any of the fields for the OverridableFunctionPointer union.
#define GEN_FIELD(fn_name) fn_name##Callback fn_name;

// The avaliable functions you can pass to OverrideInternalFunction. 
// The fields are only accessible when compiling with PLATFORM_NONE or PLATFORM_OFFSCREEN 
union OverridableFunctionPointer {
    GEN_FIELD(InitPlatform)
    GEN_FIELD(WindowShouldClose)
    GEN_FIELD(GetMonitorCount)
    GEN_FIELD(GetCurrentMonitor)
    GEN_FIELD(ShowCursor)
    GEN_FIELD(HideCursor)
    GEN_FIELD(SetWindowState)
    GEN_FIELD(ClearWindowState)
    GEN_FIELD(SetWindowMinSize)
    GEN_FIELD(SetWindowMaxSize)
    GEN_FIELD(SetWindowSize)
    GEN_FIELD(ClosePlatform)
    GEN_FIELD(ToggleFullscreen)
    GEN_FIELD(ToggleBorderlessWindowed)
    GEN_FIELD(MaximizeWindow)
    GEN_FIELD(MinimizeWindow)
    GEN_FIELD(RestoreWindow)
    GEN_FIELD(SetWindowIcon)
    GEN_FIELD(SetWindowIcons)
    GEN_FIELD(SetWindowTitle)
    GEN_FIELD(SetWindowPosition)
    GEN_FIELD(SetWindowMonitor)
    GEN_FIELD(SetWindowOpacity)
    GEN_FIELD(SetWindowFocused)
    GEN_FIELD(EnableCursor)
    GEN_FIELD(DisableCursor)
    GEN_FIELD(SwapScreenBuffer)
    GEN_FIELD(SetGamepadVibration)
    GEN_FIELD(SetMousePosition)
    GEN_FIELD(SetMouseCursor)
    GEN_FIELD(PollInputEvents)
    GEN_FIELD(SetClipboardText)
    GEN_FIELD(SetGamepadMappings)
    GEN_FIELD(GetWindowHandle)
    GEN_FIELD(GetMonitorPosition)
    GEN_FIELD(GetMonitorWidth)
    GEN_FIELD(GetMonitorHeight)
    GEN_FIELD(GetMonitorPhysicalWidth)
    GEN_FIELD(GetMonitorPhysicalHeight)
    GEN_FIELD(GetMonitorRefreshRate)
    GEN_FIELD(GetMonitorName)
    GEN_FIELD(GetWindowPosition)
    GEN_FIELD(GetWindowScaleDPI)
    GEN_FIELD(GetClipboardText)
    GEN_FIELD(GetKeyName)
    GEN_FIELD(GetTime)
};

void OverrideInternalFunction(const char * funcName, union OverridableFunctionPointer * func);
//CoreData * GetCore(); // Get and set variables of Raylib's core struct

// EOF
#endif