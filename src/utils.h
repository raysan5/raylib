/**********************************************************************************************
*
*   raylib.utils - Some common utility functions
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2019 Ramon Santamaria (@raysan5)
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
void InitAssetManager(AAssetManager *manager);  // Initialize asset manager from android app
FILE *android_fopen(const char *fileName, const char *mode);    // Replacement for fopen()
#endif

#if defined(PLATFORM_UWP)

// UWP Messages System

typedef enum
{
	None = 0,

	//Send
	ShowMouse,
	HideMouse,
	LockMouse,
	UnlockMouse,
	SetMouseLocation, //Vector0 (pos)

	//Recieve (Into C)
	RegisterKey, //Int0 (key), Char0 (status)
	RegisterClick, //Int0 (button), Char0 (status)
	ScrollWheelUpdate, //Int0 (delta)
	UpdateMouseLocation, //Vector0 (pos)
	MarkGamepadActive, //Int0 (gamepad), Bool0 (active or not)
	MarkGamepadButton, //Int0 (gamepad), Int1 (button), Char0 (status)
	MarkGamepadAxis, //Int0 (gamepad), int1 (axis), Float0 (value)
} UWPMessageType;

typedef struct UWPMessage {
	//The message type
	UWPMessageType Type;

	//Vector parameters
	Vector2 Vector0;

	//Int parameters
	int Int0;
	int Int1;

	//Char parameters
	char Char0;

	//Float parameters
	float Float0;

	//Bool parameters
	bool Bool0;

	//More parameters can be added and fed to functions
} UWPMessage;

//Allocate UWP Message
RLAPI UWPMessage* CreateUWPMessage(void);

//Free UWP Message
RLAPI void DeleteUWPMessage(UWPMessage* msg);

//Get messages into C++
RLAPI bool UWPHasMessages(void);
RLAPI UWPMessage* UWPGetMessage(void);
RLAPI void UWPSendMessage(UWPMessage* msg);

//For C to call
#ifndef _cplusplus //Hide from C++ code
void SendMessageToUWP(UWPMessage* msg);
bool HasMessageFromUWP(void);
UWPMessage* GetMessageFromUWP(void);
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif // UTILS_H