/**********************************************************************************************
*
*   raylib.camera
*
*   Camera Modes Setup and Control Functions
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

#include "raylib.h"
#include <math.h>

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// CAMERA_GENERIC
#define CAMERA_SCROLL_SENSITIVITY               1.5

// FREE_CAMERA
#define FREE_CAMERA_MOUSE_SENSITIVITY           0.01
#define FREE_CAMERA_DISTANCE_MIN_CLAMP          0.3
#define FREE_CAMERA_DISTANCE_MAX_CLAMP          12
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
#define FIRST_PERSON_MAX_CLAMP                  -85

#define FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER 5.0
#define FIRST_PERSON_STEP_DIVIDER               30.0
#define FIRST_PERSON_WAVING_DIVIDER             200.0

#define FIRST_PERSON_HEIGHT_RELATIVE_EYES_POSITION  0.85

// THIRD_PERSON
//#define THIRD_PERSON_MOUSE_SENSITIVITY          0.003
#define THIRD_PERSON_DISTANCE_CLAMP             1.2
#define THIRD_PERSON_MIN_CLAMP                  5
#define THIRD_PERSON_MAX_CLAMP                  -85
#define THIRD_PERSON_OFFSET                     (Vector3){ 0.4, 0, 0 }

// PLAYER (used by camera)
#define PLAYER_WIDTH                0.4
#define PLAYER_HEIGHT               0.9
#define PLAYER_DEPTH                0.4
#define PLAYER_MOVEMENT_DIVIDER     20.0

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static Camera internalCamera = {{2,0,2},{0,0,0},{0,1,0}};
static Vector2 cameraAngle = { 0, 0 };
static float cameraTargetDistance = 5;
static Vector3 resetingPosition = { 0, 0, 0 };
static int resetingKey = 'Z';
static Vector2 cameraMousePosition = { 0, 0 };
static Vector2 cameraMouseVariation = { 0, 0 };
static float mouseSensitivity = 0.003;
static int cameraMovementController[6]  = { 'W', 'A', 'S', 'D', 'E', 'Q' };
static int cameraMovementCounter = 0;
static bool cameraUseGravity = true;
static int pawnControllingKey = MOUSE_MIDDLE_BUTTON;
static int fnControllingKey = KEY_LEFT_ALT;
static int smoothZoomControllingKey = KEY_LEFT_CONTROL;

static int cameraMode = CAMERA_CUSTOM;

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void ProcessCamera(Camera *camera, Vector3 *playerPosition);
/*
static void SetCameraControls(int front, int left, int back, right, up, down);
static void SetMouseSensitivity(int sensitivity);
static void SetResetPosition(Vector3 resetPosition);
static void SetResetControl(int resetKey);
static void SetPawnControl(int pawnControlKey);
static void SetFnControl(int fnControlKey);
static void SetSmoothZoomControl(int smoothZoomControlKey);
*/

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Select camera mode (multiple camera modes available)
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
        internalCamera.target = (Vector3){ 0, 0, 0};
        ProcessCamera(&internalCamera, &internalCamera.position);
    }
    else if ((cameraMode == CAMERA_CUSTOM) && (mode == CAMERA_ORBITAL))
    {
        cameraTargetDistance = 10;
        cameraAngle.x = 225 * DEG2RAD;
        cameraAngle.y = -40 * DEG2RAD;
        internalCamera.target = (Vector3){ 3, 0, 3};
        ProcessCamera(&internalCamera, &internalCamera.position);
    }

    cameraMode = mode;
}

// Update camera with position
Camera UpdateCamera(Vector3 *position)
{
    // Calculate camera
    if (cameraMode != CAMERA_CUSTOM) ProcessCamera(&internalCamera, position);

    return internalCamera;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Process desired camera mode and controls
static void ProcessCamera(Camera *camera, Vector3 *playerPosition)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB) || defined(PLATFORM_RPI)
    // Mouse movement detection
    if (cameraMode != CAMERA_FREE)
    {
        HideCursor();
        
        if (GetMousePosition().x < GetScreenHeight() / 3) SetMousePosition((Vector2){ GetScreenWidth() - GetScreenHeight() / 3, GetMousePosition().y});
        else if (GetMousePosition().y < GetScreenHeight() / 3) SetMousePosition((Vector2){ GetMousePosition().x, GetScreenHeight() - GetScreenHeight() / 3});
        else if (GetMousePosition().x > GetScreenWidth() - GetScreenHeight() / 3) SetMousePosition((Vector2) { GetScreenHeight() / 3, GetMousePosition().y});
        else if (GetMousePosition().y > GetScreenHeight() - GetScreenHeight() / 3) SetMousePosition((Vector2){ GetMousePosition().x, GetScreenHeight() / 3});
        else
        {
            cameraMouseVariation.x = GetMousePosition().x - cameraMousePosition.x;
            cameraMouseVariation.y = GetMousePosition().y - cameraMousePosition.y;
        }
    }
    else
    {
        ShowCursor();
        
        cameraMouseVariation.x = GetMousePosition().x - cameraMousePosition.x;
        cameraMouseVariation.y = GetMousePosition().y - cameraMousePosition.y;
    }

    cameraMousePosition = GetMousePosition();

    // Support for multiple automatic camera modes
    switch (cameraMode)
    {
        case CAMERA_FREE:
        {
            // Camera zoom
            if ((cameraTargetDistance < FREE_CAMERA_DISTANCE_MAX_CLAMP) && (GetMouseWheelMove() < 0))
            {
                cameraTargetDistance -= (GetMouseWheelMove() * CAMERA_SCROLL_SENSITIVITY);

                if (cameraTargetDistance > FREE_CAMERA_DISTANCE_MAX_CLAMP) cameraTargetDistance = FREE_CAMERA_DISTANCE_MAX_CLAMP;
            }
            // Camera looking down
            else if ((camera->position.y > camera->target.y) && (cameraTargetDistance == FREE_CAMERA_DISTANCE_MAX_CLAMP) && (GetMouseWheelMove() < 0))
            {
                camera->target.x += GetMouseWheelMove() * (camera->target.x - camera->position.x) * CAMERA_SCROLL_SENSITIVITY / cameraTargetDistance;
                camera->target.y += GetMouseWheelMove() * (camera->target.y - camera->position.y) * CAMERA_SCROLL_SENSITIVITY / cameraTargetDistance;
                camera->target.z += GetMouseWheelMove() * (camera->target.z - camera->position.z) * CAMERA_SCROLL_SENSITIVITY / cameraTargetDistance;
            }
            else if ((camera->position.y > camera->target.y) && (camera->target.y >= 0))
            {
                camera->target.x += GetMouseWheelMove() * (camera->target.x - camera->position.x) * CAMERA_SCROLL_SENSITIVITY / cameraTargetDistance;
                camera->target.y += GetMouseWheelMove() * (camera->target.y - camera->position.y) * CAMERA_SCROLL_SENSITIVITY / cameraTargetDistance;
                camera->target.z += GetMouseWheelMove() * (camera->target.z - camera->position.z) * CAMERA_SCROLL_SENSITIVITY / cameraTargetDistance;

                if (camera->target.y < 0) camera->target.y = -0.001;
            }
            else if ((camera->position.y > camera->target.y) && (camera->target.y < 0) && (GetMouseWheelMove() > 0))
            {
                cameraTargetDistance -= (GetMouseWheelMove() * CAMERA_SCROLL_SENSITIVITY);
                if (cameraTargetDistance < FREE_CAMERA_DISTANCE_MIN_CLAMP) cameraTargetDistance = FREE_CAMERA_DISTANCE_MIN_CLAMP;
            }
            // Camera looking up
            else if ((camera->position.y < camera->target.y) && (cameraTargetDistance == FREE_CAMERA_DISTANCE_MAX_CLAMP) && (GetMouseWheelMove() < 0))
            {
                camera->target.x += GetMouseWheelMove() * (camera->target.x - camera->position.x) * CAMERA_SCROLL_SENSITIVITY / cameraTargetDistance;
                camera->target.y += GetMouseWheelMove() * (camera->target.y - camera->position.y) * CAMERA_SCROLL_SENSITIVITY / cameraTargetDistance;
                camera->target.z += GetMouseWheelMove() * (camera->target.z - camera->position.z) * CAMERA_SCROLL_SENSITIVITY / cameraTargetDistance;
            }
            else if ((camera->position.y < camera->target.y) && (camera->target.y <= 0))
            {
                camera->target.x += GetMouseWheelMove() * (camera->target.x - camera->position.x) * CAMERA_SCROLL_SENSITIVITY / cameraTargetDistance;
                camera->target.y += GetMouseWheelMove() * (camera->target.y - camera->position.y) * CAMERA_SCROLL_SENSITIVITY / cameraTargetDistance;
                camera->target.z += GetMouseWheelMove() * (camera->target.z - camera->position.z) * CAMERA_SCROLL_SENSITIVITY / cameraTargetDistance;

                if (camera->target.y > 0) camera->target.y = 0.001;
            }
            else if ((camera->position.y < camera->target.y) && (camera->target.y > 0) && (GetMouseWheelMove() > 0))
            {
                cameraTargetDistance -= (GetMouseWheelMove() * CAMERA_SCROLL_SENSITIVITY);
                if (cameraTargetDistance < FREE_CAMERA_DISTANCE_MIN_CLAMP) cameraTargetDistance = FREE_CAMERA_DISTANCE_MIN_CLAMP;
            }

            // Inputs
            if (IsKeyDown(fnControllingKey))
            {
                if (IsKeyDown(smoothZoomControllingKey))
                {
                    // Camera smooth zoom
                    if (IsMouseButtonDown(pawnControllingKey)) cameraTargetDistance += (cameraMouseVariation.y * FREE_CAMERA_SMOOTH_ZOOM_SENSITIVITY);
                }
                // Camera orientation calculation
                else if (IsMouseButtonDown(pawnControllingKey))
                {
                    // Camera orientation calculation
                    // Get the mouse sensitivity
                    cameraAngle.x += cameraMouseVariation.x * -FREE_CAMERA_MOUSE_SENSITIVITY;
                    cameraAngle.y += cameraMouseVariation.y * -FREE_CAMERA_MOUSE_SENSITIVITY;

                    // Angle clamp
                    if (cameraAngle.y > FREE_CAMERA_MIN_CLAMP * DEG2RAD) cameraAngle.y = FREE_CAMERA_MIN_CLAMP * DEG2RAD;
                    else if (cameraAngle.y < FREE_CAMERA_MAX_CLAMP * DEG2RAD) cameraAngle.y = FREE_CAMERA_MAX_CLAMP * DEG2RAD;
                }
            }
            // Paning
            else if (IsMouseButtonDown(pawnControllingKey))
            {
                camera->target.x += ((cameraMouseVariation.x * -FREE_CAMERA_MOUSE_SENSITIVITY) * cos(cameraAngle.x) + (cameraMouseVariation.y * FREE_CAMERA_MOUSE_SENSITIVITY) * sin(cameraAngle.x) * sin(cameraAngle.y)) * (cameraTargetDistance / FREE_CAMERA_PANNING_DIVIDER);
                camera->target.y += ((cameraMouseVariation.y * FREE_CAMERA_MOUSE_SENSITIVITY) * cos(cameraAngle.y)) * (cameraTargetDistance / FREE_CAMERA_PANNING_DIVIDER);
                camera->target.z += ((cameraMouseVariation.x * FREE_CAMERA_MOUSE_SENSITIVITY) * sin(cameraAngle.x) + (cameraMouseVariation.y * FREE_CAMERA_MOUSE_SENSITIVITY) * cos(cameraAngle.x) * sin(cameraAngle.y)) * (cameraTargetDistance / FREE_CAMERA_PANNING_DIVIDER);
            }

            // Focus to center
            if (IsKeyDown(resetingKey)) camera->target = resetingPosition;

            // Camera position update
            camera->position.x = sin(cameraAngle.x) * cameraTargetDistance * cos(cameraAngle.y) + camera->target.x;

            if (cameraAngle.y <= 0) camera->position.y = sin(cameraAngle.y) * cameraTargetDistance * sin(cameraAngle.y) + camera->target.y;
            else camera->position.y = -sin(cameraAngle.y) * cameraTargetDistance * sin(cameraAngle.y) + camera->target.y;

            camera->position.z = cos(cameraAngle.x) * cameraTargetDistance * cos(cameraAngle.y) + camera->target.z;

        } break;
        case CAMERA_ORBITAL:
        {
            cameraAngle.x += ORBITAL_CAMERA_SPEED;

            // Camera zoom
            cameraTargetDistance -= (GetMouseWheelMove() * CAMERA_SCROLL_SENSITIVITY);
            // Camera distance clamp
            if (cameraTargetDistance < THIRD_PERSON_DISTANCE_CLAMP) cameraTargetDistance = THIRD_PERSON_DISTANCE_CLAMP;

            // Focus to center
            if (IsKeyDown('Z')) camera->target = (Vector3) { 0, 0, 0 };

            // Camera position update
            camera->position.x = sin(cameraAngle.x) * cameraTargetDistance * cos(cameraAngle.y) + camera->target.x;

            if (cameraAngle.y <= 0) camera->position.y = sin(cameraAngle.y) * cameraTargetDistance * sin(cameraAngle.y) + camera->target.y;
            else camera->position.y = -sin(cameraAngle.y) * cameraTargetDistance * sin(cameraAngle.y) + camera->target.y;

            camera->position.z = cos(cameraAngle.x) * cameraTargetDistance * cos(cameraAngle.y) + camera->target.z;

        } break;
        case CAMERA_FIRST_PERSON:
        case CAMERA_THIRD_PERSON:
        {
            bool isMoving = false;

            // Keyboard inputs
            if (IsKeyDown(cameraMovementController[0]))
            {
                playerPosition->x -= sin(cameraAngle.x) / PLAYER_MOVEMENT_DIVIDER;
                playerPosition->z -= cos(cameraAngle.x) / PLAYER_MOVEMENT_DIVIDER;
                if (!cameraUseGravity) camera->position.y += sin(cameraAngle.y) / PLAYER_MOVEMENT_DIVIDER;

                isMoving = true;
            }
            else if (IsKeyDown(cameraMovementController[2]))
            {
                playerPosition->x += sin(cameraAngle.x) / PLAYER_MOVEMENT_DIVIDER;
                playerPosition->z += cos(cameraAngle.x) / PLAYER_MOVEMENT_DIVIDER;
                if (!cameraUseGravity) camera->position.y -= sin(cameraAngle.y) / PLAYER_MOVEMENT_DIVIDER;

                isMoving = true;
            }

            if (IsKeyDown(cameraMovementController[1]))
            {
                playerPosition->x -= cos(cameraAngle.x) / PLAYER_MOVEMENT_DIVIDER;
                playerPosition->z += sin(cameraAngle.x) / PLAYER_MOVEMENT_DIVIDER;

                isMoving = true;
            }
            else if (IsKeyDown(cameraMovementController[3]))
            {
                playerPosition->x += cos(cameraAngle.x) / PLAYER_MOVEMENT_DIVIDER;
                playerPosition->z -= sin(cameraAngle.x) / PLAYER_MOVEMENT_DIVIDER;

                isMoving = true;
            }

            if (IsKeyDown(cameraMovementController[4]))
            {
                if (!cameraUseGravity) playerPosition->y += 1 / PLAYER_MOVEMENT_DIVIDER;
            }
            else if (IsKeyDown(cameraMovementController[5]))
            {
                if (!cameraUseGravity) playerPosition->y -= 1 / PLAYER_MOVEMENT_DIVIDER;
            }

            if (cameraMode == CAMERA_THIRD_PERSON)
            {
                // Camera orientation calculation
                // Get the mouse sensitivity
                cameraAngle.x += cameraMouseVariation.x * -mouseSensitivity;
                cameraAngle.y += cameraMouseVariation.y * -mouseSensitivity;

                // Angle clamp
                if (cameraAngle.y > THIRD_PERSON_MIN_CLAMP * DEG2RAD) cameraAngle.y = THIRD_PERSON_MIN_CLAMP * DEG2RAD;
                else if (cameraAngle.y < THIRD_PERSON_MAX_CLAMP * DEG2RAD) cameraAngle.y = THIRD_PERSON_MAX_CLAMP * DEG2RAD;

                // Camera zoom
                cameraTargetDistance -= (GetMouseWheelMove() * CAMERA_SCROLL_SENSITIVITY);

                // Camera distance clamp
                if (cameraTargetDistance < THIRD_PERSON_DISTANCE_CLAMP) cameraTargetDistance = THIRD_PERSON_DISTANCE_CLAMP;

                // Camera is always looking at player
                camera->target.x = playerPosition->x + THIRD_PERSON_OFFSET.x * cos(cameraAngle.x) + THIRD_PERSON_OFFSET.z * sin(cameraAngle.x);
                camera->target.y = playerPosition->y + PLAYER_HEIGHT * FIRST_PERSON_HEIGHT_RELATIVE_EYES_POSITION + THIRD_PERSON_OFFSET.y;
                camera->target.z = playerPosition->z + THIRD_PERSON_OFFSET.z * sin(cameraAngle.x) - THIRD_PERSON_OFFSET.x * sin(cameraAngle.x);

                // Camera position update
                camera->position.x = sin(cameraAngle.x) * cameraTargetDistance * cos(cameraAngle.y) + camera->target.x;

                if (cameraAngle.y <= 0) camera->position.y = sin(cameraAngle.y) * cameraTargetDistance * sin(cameraAngle.y) + camera->target.y;
                else camera->position.y = -sin(cameraAngle.y) * cameraTargetDistance * sin(cameraAngle.y) + camera->target.y;

                camera->position.z = cos(cameraAngle.x) * cameraTargetDistance * cos(cameraAngle.y) + camera->target.z;
            }
            else
            {
                if (isMoving) cameraMovementCounter++;

                // Camera orientation calculation
                // Get the mouse sensitivity
                cameraAngle.x += cameraMouseVariation.x * -mouseSensitivity;
                cameraAngle.y += cameraMouseVariation.y * -mouseSensitivity;

                // Angle clamp
                if (cameraAngle.y > FIRST_PERSON_MIN_CLAMP * DEG2RAD) cameraAngle.y = FIRST_PERSON_MIN_CLAMP * DEG2RAD;
                else if (cameraAngle.y < FIRST_PERSON_MAX_CLAMP * DEG2RAD) cameraAngle.y = FIRST_PERSON_MAX_CLAMP * DEG2RAD;

                // Camera is always looking at player
                camera->target.x = camera->position.x - sin(cameraAngle.x) * FIRST_PERSON_FOCUS_DISTANCE;
                camera->target.y = camera->position.y + sin(cameraAngle.y) * FIRST_PERSON_FOCUS_DISTANCE;
                camera->target.z = camera->position.z - cos(cameraAngle.x) * FIRST_PERSON_FOCUS_DISTANCE;

                camera->position.x = playerPosition->x;
                camera->position.y = (playerPosition->y + PLAYER_HEIGHT * FIRST_PERSON_HEIGHT_RELATIVE_EYES_POSITION) - sin(cameraMovementCounter / FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER) / FIRST_PERSON_STEP_DIVIDER;
                camera->position.z = playerPosition->z;

                camera->up.x = sin(cameraMovementCounter / (FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER * 2)) / FIRST_PERSON_WAVING_DIVIDER;
                camera->up.z = -sin(cameraMovementCounter / (FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER * 2)) / FIRST_PERSON_WAVING_DIVIDER;
            }
        } break;
        default: break;
    }
#endif
}

void SetCameraControls(int frontKey, int leftKey, int backKey, int rightKey, int upKey, int downKey)
{
    cameraMovementController[0] = frontKey;
    cameraMovementController[1] = leftKey;
    cameraMovementController[2] = backKey;
    cameraMovementController[3] = rightKey;
    cameraMovementController[4] = upKey;
    cameraMovementController[5] = downKey;
}

void SetMouseSensitivity(float sensitivity)
{
    mouseSensitivity = (sensitivity / 10000.0);
}
 
void SetResetPosition(Vector3 resetPosition)
{
    resetingPosition = resetPosition;
}

void SetResetControl(int resetKey)
{
    resetingKey = resetKey;
}

void SetPawnControl(int pawnControlKey)
{
    pawnControllingKey = pawnControlKey;
}

void SetFnControl(int fnControlKey)
{
    fnControllingKey = fnControlKey;
}

void SetSmoothZoomControl(int smoothZoomControlKey)
{
    smoothZoomControllingKey = smoothZoomControlKey;
}


























