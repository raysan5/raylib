/*******************************************************************************************
*
*   raylib Camera System - Camera Modes Setup and Control Functions
*
*   Copyright (c) 2015 Marc Palau and Ramon Santamaria
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

#ifndef CAMERA_H
#define CAMERA_H

#include "raylib.h"

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Camera modes
typedef enum { CAMERA_CUSTOM = 0, CAMERA_FREE, CAMERA_ORBITAL, CAMERA_FIRST_PERSON, CAMERA_THIRD_PERSON } CameraMode;


#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void SetCameraMode(int mode);                               // Select camera mode (multiple camera modes available)
Camera UpdateCamera(Vector3 *position);                     // Update camera with position

void SetCameraControls(int front, int left, int back, int right, int up, int down);
void SetCameraMouseSensitivity(float sensitivity);
void SetCameraResetPosition(Vector3 resetPosition);
void SetCameraResetControl(int resetKey);
void SetCameraPawnControl(int pawnControlKey);
void SetCameraFnControl(int fnControlKey);
void SetCameraSmoothZoomControl(int smoothZoomControlKey);
void SetCameraOrbitalTarget(Vector3 target);

#ifdef __cplusplus
}
#endif

#endif // CAMERA_H
