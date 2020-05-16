/**********************************************************************************************
*
*   raylib.uwp_events - Functions for bootstrapping UWP functionality within raylib's core.
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2020-2020 Reece Mackie (@Rover656)
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

#ifndef UWP_EVENTS_H
#define UWP_EVENTS_H

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(PLATFORM_UWP)

// Determine if UWP functions are set and ready for raylib's use.
bool UWPIsConfigured();

// Call this to set the UWP data path you wish for saving and loading.
void UWPSetDataPath(const char* path);

// Function for getting program time.
typedef double(*UWPQueryTimeFunc)();
UWPQueryTimeFunc UWPGetQueryTimeFunc(void);
void UWPSetQueryTimeFunc(UWPQueryTimeFunc func);

// Function for sleeping the current thread
typedef void (*UWPSleepFunc)(double sleepUntil);
UWPSleepFunc UWPGetSleepFunc(void);
void UWPSetSleepFunc(UWPSleepFunc func);

// Function for querying the display size
typedef void(*UWPDisplaySizeFunc)(int* width, int* height);
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
void UWPKeyDownEvent(int key, bool down, bool controlKey);

// Call this on the CoreWindow::CharacterRecieved event
void UWPKeyCharEvent(int key);

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

// Call when the touch point moves
void UWPGestureMove(int pointer, float x, float y);

// Call when there is a touch down or up
void UWPGestureTouch(int pointer, float x, float y, bool touch);

// Set the core window pointer so that we can pass it to EGL.
void* UWPGetCoreWindowPtr();
void UWPSetCoreWindowPtr(void* ptr);

#if defined(__cplusplus)
}
#endif

#endif // PLATFORM_UWP

#endif // UWP_EVENTS_H
