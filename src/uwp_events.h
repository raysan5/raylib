/**********************************************************************************************
*
*   raylib.uwp_events - Functions for bootstrapping UWP functionality
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2020-2021 Reece Mackie (@Rover656)
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
bool UWPIsConfigured();                 // Check if UWP functions are set and ready to use
void UWPSetDataPath(const char *path);  // Set the UWP data path for saving and loading

typedef double(*UWPQueryTimeFunc)();                         // Callback function to query time
UWPQueryTimeFunc UWPGetQueryTimeFunc(void);                  // Get query time function
void UWPSetQueryTimeFunc(UWPQueryTimeFunc func);             // Set query time function

typedef void (*UWPSleepFunc)(double sleepUntil);             // Callback function for sleep
UWPSleepFunc UWPGetSleepFunc(void);                          // Get sleep function
void UWPSetSleepFunc(UWPSleepFunc func);                     // Set sleep function

typedef void (*UWPDisplaySizeFunc)(int *width, int *height); // Callback function for display size change
UWPDisplaySizeFunc UWPGetDisplaySizeFunc(void);              // Get display size function
void UWPSetDisplaySizeFunc(UWPDisplaySizeFunc func);         // Set display size function

typedef void (*UWPMouseFunc)(void);                          // Callback function for mouse cursor control
UWPMouseFunc UWPGetMouseLockFunc();                          // Get mouse lock function
void UWPSetMouseLockFunc(UWPMouseFunc func);                 // Set mouse lock function
UWPMouseFunc UWPGetMouseUnlockFunc();                        // Get mouse unlock function
void UWPSetMouseUnlockFunc(UWPMouseFunc func);               // Set mouse unlock function
UWPMouseFunc UWPGetMouseShowFunc();                          // Get mouse show function
void UWPSetMouseShowFunc(UWPMouseFunc func);                 // Set mouse show function
UWPMouseFunc UWPGetMouseHideFunc();                          // Get mouse hide function
void UWPSetMouseHideFunc(UWPMouseFunc func);                 // Set mouse hide function

typedef void (*UWPMouseSetPosFunc)(int x, int y);            // Callback function to set mouse position
UWPMouseSetPosFunc UWPGetMouseSetPosFunc();                  // Get mouse set position function
void UWPSetMouseSetPosFunc(UWPMouseSetPosFunc func);         // Set mouse set position function

// NOTE: Below functions are implemented in core.c but are placed here so they can be called by user code
// This choice is made as platform-specific code is preferred to be kept away from raylib.h

void UWPKeyDownEvent(int key, bool down, bool controlKey);   // Check for key down event
void UWPKeyCharEvent(int key);                               // Check for a character event (CoreWindow::CharacterRecieved)
void UWPMouseButtonEvent(int button, bool down);             // Check for mouse button state changes event
void UWPMousePosEvent(double x, double y);                   // Check for mouse cursor move event
void UWPMouseWheelEvent(int deltaY);                         // Check for mouse wheel move event
void UWPResizeEvent(int width, int height);                  // Check for window resize event
void UWPActivateGamepadEvent(int gamepad, bool active);      // Check for gamepad activated event
void UWPRegisterGamepadButton(int gamepad, int button, bool down);  // Check for gamepad button state change event
void UWPRegisterGamepadAxis(int gamepad, int axis, float value);    // Check for gamepad axis state change event
void UWPGestureMove(int pointer, float x, float y);                 // Check for touch point move event
void UWPGestureTouch(int pointer, float x, float y, bool touch);    // Check for touch down or up event

void *UWPGetCoreWindowPtr();            // Get core window pointer
void UWPSetCoreWindowPtr(void *ptr);    // Set core window pointer, so that it can be passed to EGL

#if defined(__cplusplus)
}
#endif

#endif // PLATFORM_UWP

#endif // UWP_EVENTS_H
