/**********************************************************************************************
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

//#define CAMERA_STANDALONE     // NOTE: To use the camera module as standalone lib, just uncomment this line
                                // NOTE: ProcessCamera() should be reviewed to adapt inputs to other systems

#if defined(CAMERA_STANDALONE)
    #include "camera.h"
#else
    #include "raylib.h"
#endif

#include <math.h>

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// CAMERA_GENERIC
#define CAMERA_SCROLL_SENSITIVITY               1.5

// FREE_CAMERA
#define FREE_CAMERA_MOUSE_SENSITIVITY           0.01
#define FREE_CAMERA_DISTANCE_MIN_CLAMP          0.3
#define FREE_CAMERA_DISTANCE_MAX_CLAMP          120
#define FREE_CAMERA_MIN_CLAMP                   85
#define FREE_CAMERA_MAX_CLAMP                  -85
#define FREE_CAMERA_SMOOTH_ZOOM_SENSITIVITY     0.05
#define FREE_CAMERA_PANNING_DIVIDER             5.1

// ORBITAL_CAMERA
#define ORBITAL_CAMERA_SPEED                    0.01

// FIRST_PERSON
//#define FIRST_PERSON_MOUSE_SENSITIVITY          0.003
#define FIRST_PERSON_FOCUS_DISTANCE             25
#define FIRST_PERSON_MIN_CLAMP                  85
#define FIRST_PERSON_MAX_CLAMP                 -85

#define FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER 5.0
#define FIRST_PERSON_STEP_DIVIDER               30.0
#define FIRST_PERSON_WAVING_DIVIDER             200.0

#define FIRST_PERSON_HEIGHT_RELATIVE_EYES_POSITION  0.85

// THIRD_PERSON
//#define THIRD_PERSON_MOUSE_SENSITIVITY          0.003
#define THIRD_PERSON_DISTANCE_CLAMP             1.2
#define THIRD_PERSON_MIN_CLAMP                  5
#define THIRD_PERSON_MAX_CLAMP                 -85
#define THIRD_PERSON_OFFSET                     (Vector3){ 0.4, 0, 0 }

// PLAYER (used by camera)
#define PLAYER_WIDTH                0.4
#define PLAYER_HEIGHT               0.9
#define PLAYER_DEPTH                0.4
#define PLAYER_MOVEMENT_DIVIDER     20.0

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Camera move modes (first person and third person cameras)
typedef enum { MOVE_FRONT = 0, MOVE_LEFT, MOVE_BACK, MOVE_RIGHT, MOVE_UP, MOVE_DOWN } CameraMove;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static Camera internalCamera = {{2, 0, 2}, {0, 0, 0}, {0, 1, 0}};
static Vector2 cameraAngle = { 0, 0 };
static float cameraTargetDistance = 5.0f;
static Vector2 cameraMousePosition = { 0, 0 };
static Vector2 cameraMouseVariation = { 0, 0 };
static float mouseSensitivity = 0.003f;
static int cameraMoveControl[6]  = { 'W', 'A', 'S', 'D', 'E', 'Q' };
static int cameraMoveCounter = 0;
static int cameraUseGravity = 1;
static int panControlKey = 2;                   // raylib: MOUSE_MIDDLE_BUTTON
static int altControlKey = 342;                 // raylib: KEY_LEFT_ALT
static int smoothZoomControlKey = 341;          // raylib: KEY_LEFT_CONTROL

static int cameraMode = CAMERA_CUSTOM;

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void ProcessCamera(Camera *camera, Vector3 *playerPosition);

#if defined(CAMERA_STANDALONE)
// NOTE: Camera controls depend on some raylib input functions
// TODO: Set your own input functions (used in ProcessCamera())
static Vector2 GetMousePosition() { return (Vector2){ 0, 0}; }
static void SetMousePosition(Vector2 pos) {} 
static int IsMouseButtonDown(int button) { return 0;}
static int GetMouseWheelMove() { return 0; }
static int GetScreenWidth() { return 1280; }
static int GetScreenHeight() { return 720; }
static void ShowCursor() {}
static void HideCursor() {}
static int IsKeyDown(int key) { return 0; }
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Select camera mode (multiple camera modes available)
// TODO: Review hardcoded values when changing modes...
void SetCameraMode(int mode)
{
    if ((cameraMode == CAMERA_FIRST_PERSON) && (mode == CAMERA_FREE))
    {
        cameraMode = CAMERA_THIRD_PERSON;
        cameraTargetDistance = 5;
        cameraAngle.y = -40 * DEG2RAD;
        ProcessCamera(&internalCamera, &internalCamera.position);
    }
    else if ((cameraMode == CAMERA_FIRST_PERSON) && (mode == CAMERA_ORBITAL))
    {
        cameraMode = CAMERA_THIRD_PERSON;
        cameraTargetDistance = 5;
        cameraAngle.y = -40 * DEG2RAD;
        ProcessCamera(&internalCamera, &internalCamera.position);
    }
    else if ((cameraMode == CAMERA_CUSTOM) && (mode == CAMERA_FREE))
    {
        cameraTargetDistance = 10;
        cameraAngle.x = 45 * DEG2RAD;
        cameraAngle.y = -40 * DEG2RAD;
        internalCamera.target = (Vector3){ 0, 0, 0 };
        ProcessCamera(&internalCamera, &internalCamera.position);
        
        ShowCursor();
    }
    else if ((cameraMode == CAMERA_CUSTOM) && (mode == CAMERA_ORBITAL))
    {
        cameraTargetDistance = 10;
        cameraAngle.x = 225 * DEG2RAD;
        cameraAngle.y = -40 * DEG2RAD;
        internalCamera.target = (Vector3){ 0, 0, 0};
        ProcessCamera(&internalCamera, &internalCamera.position);
    }

    cameraMode = mode;
}

// Update camera (player position is ignored)
void UpdateCamera(Camera *camera)
{
    Vector3 position = { 0, 0, 0 };
    
    // Process internal camera and player position (if required)
    if (cameraMode != CAMERA_CUSTOM) ProcessCamera(&internalCamera, &position);

    *camera = internalCamera;
}

// Update camera and player position (1st person and 3rd person cameras)
void UpdateCameraPlayer(Camera *camera, Vector3 *position)
{
    // Process internal camera and player position (if required)
    if (cameraMode != CAMERA_CUSTOM) ProcessCamera(&internalCamera, position);

    *camera = internalCamera;
}

// Set internal camera position
void SetCameraPosition(Vector3 position)
{
    internalCamera.position = position;
    
    Vector3 v1 = internalCamera.position;
    Vector3 v2 = internalCamera.target;
    
    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;
    float dz = v2.z - v1.z;
    
    cameraTargetDistance = sqrt(dx*dx + dy*dy + dz*dz);
}

// Set internal camera target
void SetCameraTarget(Vector3 target)
{
    internalCamera.target = target;
    
    Vector3 v1 = internalCamera.position;
    Vector3 v2 = internalCamera.target;
    
    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;
    float dz = v2.z - v1.z;
    
    cameraTargetDistance = sqrt(dx*dx + dy*dy + dz*dz);
}

// Set camera pan key to combine with mouse movement (free camera)
void SetCameraPanControl(int panKey)
{
    panControlKey = panKey;
}

// Set camera alt key to combine with mouse movement (free camera)
void SetCameraAltControl(int altKey)
{
    altControlKey = altKey;
}

// Set camera smooth zoom key to combine with mouse (free camera)
void SetCameraSmoothZoomControl(int szKey)
{
    smoothZoomControlKey = szKey;
}

// Set camera move controls (1st person and 3rd person cameras)
void SetCameraMoveControls(int frontKey, int backKey, int leftKey, int rightKey, int upKey, int downKey)
{
    cameraMoveControl[MOVE_FRONT] = frontKey;
    cameraMoveControl[MOVE_LEFT] = leftKey;
    cameraMoveControl[MOVE_BACK] = backKey;
    cameraMoveControl[MOVE_RIGHT] = rightKey;
    cameraMoveControl[MOVE_UP] = upKey;
    cameraMoveControl[MOVE_DOWN] = downKey;
}

// Set camera mouse sensitivity (1st person and 3rd person cameras)
void SetCameraMouseSensitivity(float sensitivity)
{
    mouseSensitivity = (sensitivity/10000.0);
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Process desired camera mode and controls
// NOTE: Camera controls depend on some raylib functions:
//       Mouse: GetMousePosition(), SetMousePosition(), IsMouseButtonDown(), GetMouseWheelMove()
//       System: GetScreenWidth(), GetScreenHeight(), ShowCursor(), HideCursor()
//       Keys:  IsKeyDown()
static void ProcessCamera(Camera *camera, Vector3 *playerPosition)
{
    // Mouse movement detection
    Vector2 mousePosition = GetMousePosition();
    int mouseWheelMove = GetMouseWheelMove();
    int panKey = IsMouseButtonDown(panControlKey);    // bool value
    
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    if ((cameraMode != CAMERA_FREE) && (cameraMode != CAMERA_ORBITAL))
    {
        HideCursor();

        if (mousePosition.x < screenHeight/3) SetMousePosition((Vector2){ screenWidth - screenHeight/3, mousePosition.y});
        else if (mousePosition.y < screenHeight/3) SetMousePosition((Vector2){ mousePosition.x, screenHeight - screenHeight/3});
        else if (mousePosition.x > screenWidth - screenHeight/3) SetMousePosition((Vector2) { screenHeight/3, mousePosition.y});
        else if (mousePosition.y > screenHeight - screenHeight/3) SetMousePosition((Vector2){ mousePosition.x, screenHeight/3});
        else
        {
            cameraMouseVariation.x = mousePosition.x - cameraMousePosition.x;
            cameraMouseVariation.y = mousePosition.y - cameraMousePosition.y;
        }
    }
    else
    {
        ShowCursor();

        cameraMouseVariation.x = mousePosition.x - cameraMousePosition.x;
        cameraMouseVariation.y = mousePosition.y - cameraMousePosition.y;
    }

	// NOTE: We GetMousePosition() again because it can be modified by a previous SetMousePosition() call
	// If using directly mousePosition variable we have problems on CAMERA_FIRST_PERSON and CAMERA_THIRD_PERSON
    cameraMousePosition = GetMousePosition();

    // Support for multiple automatic camera modes
    switch (cameraMode)
    {
        case CAMERA_FREE:
        {
            // Camera zoom
            if ((cameraTargetDistance < FREE_CAMERA_DISTANCE_MAX_CLAMP) && (mouseWheelMove < 0))
            {
                cameraTargetDistance -= (mouseWheelMove*CAMERA_SCROLL_SENSITIVITY);

                if (cameraTargetDistance > FREE_CAMERA_DISTANCE_MAX_CLAMP) cameraTargetDistance = FREE_CAMERA_DISTANCE_MAX_CLAMP;
            }
            // Camera looking down
            else if ((camera->position.y > camera->target.y) && (cameraTargetDistance == FREE_CAMERA_DISTANCE_MAX_CLAMP) && (mouseWheelMove < 0))
            {
                camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_SCROLL_SENSITIVITY/cameraTargetDistance;
                camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_SCROLL_SENSITIVITY/cameraTargetDistance;
                camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_SCROLL_SENSITIVITY/cameraTargetDistance;
            }
            else if ((camera->position.y > camera->target.y) && (camera->target.y >= 0))
            {
                camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_SCROLL_SENSITIVITY/cameraTargetDistance;
                camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_SCROLL_SENSITIVITY/cameraTargetDistance;
                camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_SCROLL_SENSITIVITY/cameraTargetDistance;

                if (camera->target.y < 0) camera->target.y = -0.001;
            }
            else if ((camera->position.y > camera->target.y) && (camera->target.y < 0) && (mouseWheelMove > 0))
            {
                cameraTargetDistance -= (mouseWheelMove*CAMERA_SCROLL_SENSITIVITY);
                if (cameraTargetDistance < FREE_CAMERA_DISTANCE_MIN_CLAMP) cameraTargetDistance = FREE_CAMERA_DISTANCE_MIN_CLAMP;
            }
            // Camera looking up
            else if ((camera->position.y < camera->target.y) && (cameraTargetDistance == FREE_CAMERA_DISTANCE_MAX_CLAMP) && (mouseWheelMove < 0))
            {
                camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_SCROLL_SENSITIVITY/cameraTargetDistance;
                camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_SCROLL_SENSITIVITY/cameraTargetDistance;
                camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_SCROLL_SENSITIVITY/cameraTargetDistance;
            }
            else if ((camera->position.y < camera->target.y) && (camera->target.y <= 0))
            {
                camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_SCROLL_SENSITIVITY/cameraTargetDistance;
                camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_SCROLL_SENSITIVITY/cameraTargetDistance;
                camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_SCROLL_SENSITIVITY/cameraTargetDistance;

                if (camera->target.y > 0) camera->target.y = 0.001;
            }
            else if ((camera->position.y < camera->target.y) && (camera->target.y > 0) && (mouseWheelMove > 0))
            {
                cameraTargetDistance -= (mouseWheelMove*CAMERA_SCROLL_SENSITIVITY);
                if (cameraTargetDistance < FREE_CAMERA_DISTANCE_MIN_CLAMP) cameraTargetDistance = FREE_CAMERA_DISTANCE_MIN_CLAMP;
            }

            // Inputs
            if (IsKeyDown(altControlKey))
            {
                if (IsKeyDown(smoothZoomControlKey))
                {
                    // Camera smooth zoom
                    if (panKey) cameraTargetDistance += (cameraMouseVariation.y*FREE_CAMERA_SMOOTH_ZOOM_SENSITIVITY);
                }
                // Camera orientation calculation
                else if (panKey)
                {
                    // Camera orientation calculation
                    // Get the mouse sensitivity
                    cameraAngle.x += cameraMouseVariation.x*-FREE_CAMERA_MOUSE_SENSITIVITY;
                    cameraAngle.y += cameraMouseVariation.y*-FREE_CAMERA_MOUSE_SENSITIVITY;

                    // Angle clamp
                    if (cameraAngle.y > FREE_CAMERA_MIN_CLAMP*DEG2RAD) cameraAngle.y = FREE_CAMERA_MIN_CLAMP*DEG2RAD;
                    else if (cameraAngle.y < FREE_CAMERA_MAX_CLAMP*DEG2RAD) cameraAngle.y = FREE_CAMERA_MAX_CLAMP*DEG2RAD;
                }
            }
            // Paning
            else if (panKey)
            {
                camera->target.x += ((cameraMouseVariation.x*-FREE_CAMERA_MOUSE_SENSITIVITY)*cos(cameraAngle.x) + (cameraMouseVariation.y*FREE_CAMERA_MOUSE_SENSITIVITY)*sin(cameraAngle.x)*sin(cameraAngle.y))*(cameraTargetDistance/FREE_CAMERA_PANNING_DIVIDER);
                camera->target.y += ((cameraMouseVariation.y*FREE_CAMERA_MOUSE_SENSITIVITY)*cos(cameraAngle.y))*(cameraTargetDistance/FREE_CAMERA_PANNING_DIVIDER);
                camera->target.z += ((cameraMouseVariation.x*FREE_CAMERA_MOUSE_SENSITIVITY)*sin(cameraAngle.x) + (cameraMouseVariation.y*FREE_CAMERA_MOUSE_SENSITIVITY)*cos(cameraAngle.x)*sin(cameraAngle.y))*(cameraTargetDistance/FREE_CAMERA_PANNING_DIVIDER);
            }

            // Focus to center
            // TODO: Move this function out of the module?
            if (IsKeyDown('Z')) camera->target = (Vector3){ 0, 0, 0 };

            // Camera position update
            camera->position.x = sin(cameraAngle.x)*cameraTargetDistance*cos(cameraAngle.y) + camera->target.x;

            if (cameraAngle.y <= 0) camera->position.y = sin(cameraAngle.y)*cameraTargetDistance*sin(cameraAngle.y) + camera->target.y;
            else camera->position.y = -sin(cameraAngle.y)*cameraTargetDistance*sin(cameraAngle.y) + camera->target.y;

            camera->position.z = cos(cameraAngle.x)*cameraTargetDistance*cos(cameraAngle.y) + camera->target.z;

        } break;
        case CAMERA_ORBITAL:
        {
            cameraAngle.x += ORBITAL_CAMERA_SPEED;

            // Camera zoom
            cameraTargetDistance -= (mouseWheelMove*CAMERA_SCROLL_SENSITIVITY);
            
            // Camera distance clamp
            if (cameraTargetDistance < THIRD_PERSON_DISTANCE_CLAMP) cameraTargetDistance = THIRD_PERSON_DISTANCE_CLAMP;

            // Focus to center
            if (IsKeyDown('Z')) camera->target = (Vector3){ 0, 0, 0 };

            // Camera position update
            camera->position.x = sin(cameraAngle.x)*cameraTargetDistance*cos(cameraAngle.y) + camera->target.x;

            if (cameraAngle.y <= 0) camera->position.y = sin(cameraAngle.y)*cameraTargetDistance*sin(cameraAngle.y) + camera->target.y;
            else camera->position.y = -sin(cameraAngle.y)*cameraTargetDistance*sin(cameraAngle.y) + camera->target.y;

            camera->position.z = cos(cameraAngle.x)*cameraTargetDistance*cos(cameraAngle.y) + camera->target.z;

        } break;
        case CAMERA_FIRST_PERSON:
        case CAMERA_THIRD_PERSON:
        {
            int isMoving = 0;

            // Keyboard inputs
            if (IsKeyDown(cameraMoveControl[MOVE_FRONT]))
            {
                playerPosition->x -= sin(cameraAngle.x)/PLAYER_MOVEMENT_DIVIDER;
                playerPosition->z -= cos(cameraAngle.x)/PLAYER_MOVEMENT_DIVIDER;
                
                if (!cameraUseGravity) camera->position.y += sin(cameraAngle.y)/PLAYER_MOVEMENT_DIVIDER;

                isMoving = 1;
            }
            else if (IsKeyDown(cameraMoveControl[MOVE_BACK]))
            {
                playerPosition->x += sin(cameraAngle.x)/PLAYER_MOVEMENT_DIVIDER;
                playerPosition->z += cos(cameraAngle.x)/PLAYER_MOVEMENT_DIVIDER;
                
                if (!cameraUseGravity) camera->position.y -= sin(cameraAngle.y)/PLAYER_MOVEMENT_DIVIDER;

                isMoving = 1;
            }

            if (IsKeyDown(cameraMoveControl[MOVE_LEFT]))
            {
                playerPosition->x -= cos(cameraAngle.x)/PLAYER_MOVEMENT_DIVIDER;
                playerPosition->z += sin(cameraAngle.x)/PLAYER_MOVEMENT_DIVIDER;

                isMoving = 1;
            }
            else if (IsKeyDown(cameraMoveControl[MOVE_RIGHT]))
            {
                playerPosition->x += cos(cameraAngle.x)/PLAYER_MOVEMENT_DIVIDER;
                playerPosition->z -= sin(cameraAngle.x)/PLAYER_MOVEMENT_DIVIDER;

                isMoving = 1;
            }

            if (IsKeyDown(cameraMoveControl[MOVE_UP]))
            {
                if (!cameraUseGravity) playerPosition->y += 1/PLAYER_MOVEMENT_DIVIDER;
            }
            else if (IsKeyDown(cameraMoveControl[MOVE_DOWN]))
            {
                if (!cameraUseGravity) playerPosition->y -= 1/PLAYER_MOVEMENT_DIVIDER;
            }

            if (cameraMode == CAMERA_THIRD_PERSON)
            {
                // Camera orientation calculation
                cameraAngle.x += cameraMouseVariation.x*-mouseSensitivity;
                cameraAngle.y += cameraMouseVariation.y*-mouseSensitivity;

                // Angle clamp
                if (cameraAngle.y > THIRD_PERSON_MIN_CLAMP*DEG2RAD) cameraAngle.y = THIRD_PERSON_MIN_CLAMP*DEG2RAD;
                else if (cameraAngle.y < THIRD_PERSON_MAX_CLAMP*DEG2RAD) cameraAngle.y = THIRD_PERSON_MAX_CLAMP*DEG2RAD;

                // Camera zoom
                cameraTargetDistance -= (mouseWheelMove*CAMERA_SCROLL_SENSITIVITY);

                // Camera distance clamp
                if (cameraTargetDistance < THIRD_PERSON_DISTANCE_CLAMP) cameraTargetDistance = THIRD_PERSON_DISTANCE_CLAMP;

                // Camera is always looking at player
                camera->target.x = playerPosition->x + THIRD_PERSON_OFFSET.x*cos(cameraAngle.x) + THIRD_PERSON_OFFSET.z*sin(cameraAngle.x);
                camera->target.y = playerPosition->y + PLAYER_HEIGHT*FIRST_PERSON_HEIGHT_RELATIVE_EYES_POSITION + THIRD_PERSON_OFFSET.y;
                camera->target.z = playerPosition->z + THIRD_PERSON_OFFSET.z*sin(cameraAngle.x) - THIRD_PERSON_OFFSET.x*sin(cameraAngle.x);

                // Camera position update
                camera->position.x = sin(cameraAngle.x)*cameraTargetDistance*cos(cameraAngle.y) + camera->target.x;

                if (cameraAngle.y <= 0) camera->position.y = sin(cameraAngle.y)*cameraTargetDistance*sin(cameraAngle.y) + camera->target.y;
                else camera->position.y = -sin(cameraAngle.y)*cameraTargetDistance*sin(cameraAngle.y) + camera->target.y;

                camera->position.z = cos(cameraAngle.x)*cameraTargetDistance*cos(cameraAngle.y) + camera->target.z;
            }
            else    // CAMERA_FIRST_PERSON
            {
                if (isMoving) cameraMoveCounter++;

                // Camera orientation calculation
                cameraAngle.x += (cameraMouseVariation.x * -mouseSensitivity);
                cameraAngle.y += (cameraMouseVariation.y * -mouseSensitivity);

                // Angle clamp
                if (cameraAngle.y > FIRST_PERSON_MIN_CLAMP*DEG2RAD) cameraAngle.y = FIRST_PERSON_MIN_CLAMP*DEG2RAD;
                else if (cameraAngle.y < FIRST_PERSON_MAX_CLAMP*DEG2RAD) cameraAngle.y = FIRST_PERSON_MAX_CLAMP*DEG2RAD;

                // Camera is always looking at player
                camera->target.x = camera->position.x - sin(cameraAngle.x)*FIRST_PERSON_FOCUS_DISTANCE;
                camera->target.y = camera->position.y + sin(cameraAngle.y)*FIRST_PERSON_FOCUS_DISTANCE;
                camera->target.z = camera->position.z - cos(cameraAngle.x)*FIRST_PERSON_FOCUS_DISTANCE;

                camera->position.x = playerPosition->x;
                camera->position.y = (playerPosition->y + PLAYER_HEIGHT*FIRST_PERSON_HEIGHT_RELATIVE_EYES_POSITION) - sin(cameraMoveCounter/FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER)/FIRST_PERSON_STEP_DIVIDER;
                camera->position.z = playerPosition->z;

                camera->up.x = sin(cameraMoveCounter/(FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER*2))/FIRST_PERSON_WAVING_DIVIDER;
                camera->up.z = -sin(cameraMoveCounter/(FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER*2))/FIRST_PERSON_WAVING_DIVIDER;
            }
        } break;
        default: break;
    }
}