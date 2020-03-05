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
// UWP Messages System
typedef enum {
    UWP_MSG_NONE = 0,

    // Send
    UWP_MSG_SHOW_MOUSE,
    UWP_MSG_HIDE_MOUSE,
    UWP_MSG_LOCK_MOUSE,
    UWP_MSG_UNLOCK_MOUSE,
    UWP_MSG_SET_MOUSE_LOCATION,     // paramVector0 (pos)

    // Receive (Into C)
    UWP_MSG_REGISTER_KEY,           // paramInt0 (key), paramChar0 (status)
    UWP_MSG_REGISTER_CLICK,         // paramInt0 (button), paramChar0 (status)
    UWP_MSG_SCROLL_WHEEL_UPDATE,    // paramInt0 (delta)
    UWP_MSG_UPDATE_MOUSE_LOCATION,  // paramVector0 (pos)
    UWP_MSG_SET_GAMEPAD_ACTIVE,     // paramInt0 (gamepad), paramBool0 (active or not)
    UWP_MSG_SET_GAMEPAD_BUTTON,     // paramInt0 (gamepad), paramInt1 (button), paramChar0 (status)
    UWP_MSG_SET_GAMEPAD_AXIS,       // paramInt0 (gamepad), int1 (axis), paramFloat0 (value)
    UWP_MSG_SET_DISPLAY_DIMS,       // paramVector0 (display dimensions)
    UWP_MSG_HANDLE_RESIZE,          // paramVector0 (new dimensions) - Onresized event
    UWP_MSG_SET_GAME_TIME,          // paramInt0
} UWPMessageType;

typedef struct UWPMessage {
    UWPMessageType type;            // Message type

    Vector2 paramVector0;           // Vector parameters
    int paramInt0;                  // Int parameter
    int paramInt1;                  // Int parameter
    char paramChar0;                // Char parameters
    float paramFloat0;              // Float parameters
    double paramDouble0;            // Double parameters
    bool paramBool0;                // Bool parameters

    // More parameters can be added and fed to functions
} UWPMessage;

// Allocate UWP Message
RLAPI UWPMessage* CreateUWPMessage(void);

// Free UWP Message
RLAPI void DeleteUWPMessage(UWPMessage* msg);

// Get messages into C++
RLAPI bool UWPHasMessages(void);
RLAPI UWPMessage* UWPGetMessage(void);
RLAPI void UWPSendMessage(UWPMessage* msg);

// For C to call
#ifndef __cplusplus // Hide from C++ code
void SendMessageToUWP(UWPMessage* msg);
bool HasMessageFromUWP(void);
UWPMessage* GetMessageFromUWP(void);
#endif

#endif      //defined(PLATFORM_UWP)

#ifdef __cplusplus
}
#endif

#endif // UTILS_H
