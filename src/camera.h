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

#ifndef PI
    #define PI 3.14159265358979323846
#endif

#define DEG2RAD (PI/180.0f)
#define RAD2DEG (180.0f/PI)

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Types and Structures Definition
// NOTE: Below types are required for CAMERA_STANDALONE usage
//----------------------------------------------------------------------------------
// Camera modes
typedef enum { CAMERA_CUSTOM = 0, CAMERA_FREE, CAMERA_ORBITAL, CAMERA_FIRST_PERSON, CAMERA_THIRD_PERSON } CameraMode;

// Vector2 type
typedef struct Vector2 {
    float x;
    float y;
} Vector2;

// Vector3 type
typedef struct Vector3 {
    float x;
    float y;
    float z;
} Vector3;

// Camera type, defines a camera position/orientation in 3d space
typedef struct Camera {
    Vector3 position;
    Vector3 target;
    Vector3 up;
} Camera;

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
void SetCameraMode(int mode);                               // Set camera mode (multiple camera modes available)
void UpdateCamera(Camera *camera);                          // Update camera (player position is ignored)
void UpdateCameraPlayer(Camera *camera, Vector3 *position); // Update camera and player position (1st person and 3rd person cameras)

void SetCameraPosition(Vector3 position);                   // Set internal camera position
void SetCameraTarget(Vector3 target);                       // Set internal camera target

void SetCameraPanControl(int panKey);                       // Set camera pan key to combine with mouse movement (free camera)
void SetCameraAltControl(int altKey);                       // Set camera alt key to combine with mouse movement (free camera)
void SetCameraSmoothZoomControl(int szKey);                 // Set camera smooth zoom key to combine with mouse (free camera)

void SetCameraMoveControls(int frontKey, int backKey, 
                           int leftKey, int rightKey, 
                           int upKey, int downKey);         // Set camera move controls (1st person and 3rd person cameras)
void SetCameraMouseSensitivity(float sensitivity);          // Set camera mouse sensitivity (1st person and 3rd person cameras)

#ifdef __cplusplus
}
#endif

#endif // CAMERA_H
