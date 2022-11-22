//========================================================================
// GLFW 3.4 Cocoa - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2006-2017 Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/hid/IOHIDKeys.h>

#define GLFW_COCOA_JOYSTICK_STATE         _GLFWjoystickNS ns;
#define GLFW_COCOA_LIBRARY_JOYSTICK_STATE

#define GLFW_BUILD_COCOA_MAPPINGS

// Cocoa-specific per-joystick data
//
typedef struct _GLFWjoystickNS
{
    IOHIDDeviceRef      device;
    CFMutableArrayRef   axes;
    CFMutableArrayRef   buttons;
    CFMutableArrayRef   hats;
} _GLFWjoystickNS;

GLFWbool _glfwInitJoysticksCocoa(void);
void _glfwTerminateJoysticksCocoa(void);
GLFWbool _glfwPollJoystickCocoa(_GLFWjoystick* js, int mode);
const char* _glfwGetMappingNameCocoa(void);
void _glfwUpdateGamepadGUIDCocoa(char* guid);

