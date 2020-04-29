/**********************************************************************************************
*
*   raylib.utils - Some common utility functions
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2020 Ramon Santamaria (@raysan5)
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

#ifndef UTILS_H
#define UTILS_H

#if defined(PLATFORM_ANDROID)
    #include <stdio.h>                      // Required for: FILE
    #include <android/asset_manager.h>      // Required for: AAssetManager
#endif

#if defined(SUPPORT_TRACELOG)
    #define TRACELOG(level, ...) TraceLog(level, __VA_ARGS__)

    #if defined(SUPPORT_TRACELOG_DEBUG)
        #define TRACELOGD(...) TraceLog(LOG_DEBUG, __VA_ARGS__)
    #else
        #define TRACELOGD(...) (void)0
    #endif
#else
    #define TRACELOG(level, ...) (void)0
    #define TRACELOGD(...) (void)0
#endif

//----------------------------------------------------------------------------------
// Some basic Defines
//----------------------------------------------------------------------------------
#if defined(PLATFORM_ANDROID)
    #define fopen(name, mode) android_fopen(name, mode)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
// Nop...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
#if defined(PLATFORM_ANDROID)
void InitAssetManager(AAssetManager *manager, const char *dataPath);   // Initialize asset manager from android app
FILE *android_fopen(const char *fileName, const char *mode);            // Replacement for fopen() -> Read-only!
#endif

#if defined(PLATFORM_UWP)
// UWP Implementation Features

// Determine if UWP functions are set and ready for raylib's use.
bool UWPIsConfigured();

// Function for getting program time.
typedef double(*UWPQueryTimeFunc)();
UWPQueryTimeFunc UWPGetQueryTimeFunc(void);
void UWPSetQueryTimeFunc(UWPQueryTimeFunc func);

// Function for sleeping the current thread
typedef void (*UWPSleepFunc)(double sleepUntil);
UWPSleepFunc UWPGetSleepFunc(void);
void UWPSetSleepFunc(UWPSleepFunc func);

// Function for querying the display size
typedef void(*UWPDisplaySizeFunc)(int *width, int* height);
UWPDisplaySizeFunc UWPGetDisplaySizeFunc(void);
void UWPSetDisplaySizeFunc(UWPDisplaySizeFunc func);

// Functions for mouse cursor control
typedef void(*UWPMouseFunc)(void);
UWPMouseFunc UWPGetMouseLockFunc();
void UWPSetMouseLockFunc(UWPMouseFunc func);
UWPMouseFunc UWPGetMouseUnlockFunc();
void UWPSetMouseUnlockFunc(UWPMouseFunc func);
UWPMouseFunc UWPGetMouseShowFunc();
void UWPSetMouseShowFunc(UWPMouseFunc func);
UWPMouseFunc UWPGetMouseHideFunc();
void UWPSetMouseHideFunc(UWPMouseFunc func);

// Function for setting mouse cursor position.
typedef void (*UWPMouseSetPosFunc)(int x, int y);
UWPMouseSetPosFunc UWPGetMouseSetPosFunc();
void UWPSetMouseSetPosFunc(UWPMouseSetPosFunc func);

// The below functions are implemented in core.c but are placed here so they can be called by user code.
// This choice is made as platform-specific code is preferred to be kept away from raylib.h

// Call this when a Key is pressed or released.
void UWPKeyDownEvent(int key, bool down);

// Call when a mouse button state changes
void UWPMouseButtonEvent(int button, bool down);

// Call when the mouse cursor moves
void UWPMousePosEvent(double x, double y);

// Call when the mouse wheel moves
void UWPMouseWheelEvent(int deltaY);

// Call when the window resizes
void UWPResizeEvent(int width, int height);

// Call when a gamepad is made active
void UWPActivateGamepadEvent(int gamepad, bool active);

// Call when a gamepad button state changes
void UWPRegisterGamepadButton(int gamepad, int button, bool down);

// Call when a gamepad axis state changes
void UWPRegisterGamepadAxis(int gamepad, int axis, float value);

// Set the core window pointer so that we can pass it to EGL.
void* UWPGetCoreWindowPtr();
void UWPSetCoreWindowPtr(void* ptr);

#endif      // defined(PLATFORM_UWP)

#ifdef __cplusplus
}
#endif

#endif // UTILS_H
