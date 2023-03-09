/*******************************************************************************************
*
*   rcamera - Basic camera system with support for multiple camera modes
*
*   CONFIGURATION:
*
*   #define CAMERA_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*   #define CAMERA_STANDALONE
*       If defined, the library can be used as standalone as a camera system but some
*       functions must be redefined to manage inputs accordingly.
*
*   CONTRIBUTORS:
*       Ramon Santamaria:   Supervision, review, update and maintenance
*       Christoph Wagner:   Complete redesign, using raymath (2022)
*       Marc Palau:         Initial implementation (2014)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2022-2023 Christoph Wagner (@Crydsch) & Ramon Santamaria (@raysan5)
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

#ifndef RCAMERA_H
#define RCAMERA_H

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...
#if defined(CAMERA_STANDALONE)
#define CAMERA_CULL_DISTANCE_NEAR      0.01
#define CAMERA_CULL_DISTANCE_FAR    1000.0
#else
#define CAMERA_CULL_DISTANCE_NEAR   RL_CULL_DISTANCE_NEAR
#define CAMERA_CULL_DISTANCE_FAR    RL_CULL_DISTANCE_FAR
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
// NOTE: Below types are required for CAMERA_STANDALONE usage
//----------------------------------------------------------------------------------
// TODO review
#if defined(CAMERA_STANDALONE)
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
    typedef struct Camera3D {
        Vector3 position;       // Camera position
        Vector3 target;         // Camera target it looks-at
        Vector3 up;             // Camera up vector (rotation over its axis)
        float fovy;             // Camera field-of-view apperture in Y (degrees) in perspective, used as near plane width in orthographic
        int projection;         // Camera type, defines projection type: CAMERA_PERSPECTIVE or CAMERA_ORTHOGRAPHIC
    } Camera3D;

    typedef Camera3D Camera;    // Camera type fallback, defaults to Camera3D

    // Camera system modes
    typedef enum {
        CAMERA_CUSTOM = 0,
        CAMERA_FREE,
        CAMERA_ORBITAL,
        CAMERA_FIRST_PERSON,
        CAMERA_THIRD_PERSON
    } CameraMode;

    // Camera projection modes
    typedef enum {
        CAMERA_PERSPECTIVE = 0,
        CAMERA_ORTHOGRAPHIC
    } CameraProjection;
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

Vector3 GetCameraForward(Camera *camera);
Vector3 GetCameraUp(Camera *camera);
Vector3 GetCameraRight(Camera *camera);

void CameraMoveForward(Camera *camera, float distance, bool moveInWorldPlane);
void CameraMoveUp(Camera *camera, float distance);
void CameraMoveRight(Camera *camera, float distance, bool moveInWorldPlane);
void CameraMoveToTarget(Camera *camera, float delta);

void CameraYaw(Camera *camera, float angle, bool rotateAroundTarget);
void CameraPitch(Camera *camera, float angle, bool lockView, bool rotateAroundTarget, bool rotateUp);
void CameraRoll(Camera *camera, float angle);

Matrix GetCameraViewMatrix(Camera *camera);
Matrix GetCameraProjectionMatrix(Camera* camera, float aspect);

#if defined(__cplusplus)
}
#endif

#endif // CAMERA_H


/***********************************************************************************
*
*   CAMERA IMPLEMENTATION
*
************************************************************************************/

#if defined(CAMERA_IMPLEMENTATION)

#include "raymath.h"        // Required for vector maths:
                            // Vector3Add()
                            // Vector3Subtract()
                            // Vector3Scale()
                            // Vector3Normalize()
                            // Vector3Distance()
                            // Vector3CrossProduct()
                            // Vector3RotateByAxisAngle()
                            // Vector3Angle()
                            // Vector3Negate()
                            // MatrixLookAt()
                            // MatrixPerspective()
                            // MatrixOrtho()
                            // MatrixIdentity()

// raylib required functionality:
                            // GetMouseDelta()
                            // GetMouseWheelMove()
                            // IsKeyDown()
                            // IsKeyPressed()
                            // GetFrameTime()

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define CAMERA_MOVE_SPEED                               0.09f
#define CAMERA_ROTATION_SPEED                           0.03f

// Camera mouse movement sensitivity
#define CAMERA_MOUSE_MOVE_SENSITIVITY                   0.003f    // TODO: it should be independant of framerate
#define CAMERA_MOUSE_SCROLL_SENSITIVITY                 1.5f

#define CAMERA_ORBITAL_SPEED                            0.5f       // Radians per second

#define CAMERA_FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER  8.0f
#define CAMERA_FIRST_PERSON_STEP_DIVIDER                30.0f
#define CAMERA_FIRST_PERSON_WAVING_DIVIDER              200.0f

// PLAYER (used by camera)
#define PLAYER_MOVEMENT_SENSITIVITY                     20.0f

#if defined(PLATFORM_NX)
// FREE_CAMERA
#define CAMERA_FREE_MOUSE_SENSITIVITY                   0.01f
#define CAMERA_FREE_DISTANCE_MIN_CLAMP                  0.3f
#define CAMERA_FREE_DISTANCE_MAX_CLAMP                  120.0f
#define CAMERA_FREE_MIN_CLAMP                           85.0f
#define CAMERA_FREE_MAX_CLAMP                          -85.0f
#define CAMERA_FREE_PANNING_DIVIDER                     5.1f
#define CAMERA_FREE_SMOOTH_ZOOM_SENSITIVITY            -0.5f

// THIRD_PERSON
//#define CAMERA_THIRD_PERSON_MOUSE_SENSITIVITY           0.003f
#define CAMERA_THIRD_PERSON_DISTANCE_CLAMP              1.2f
#define CAMERA_THIRD_PERSON_MIN_CLAMP                   5.0f
#define CAMERA_THIRD_PERSON_MAX_CLAMP                  -85.0f
#define CAMERA_THIRD_PERSON_OFFSET                      (Vector3){ 0.4f, 0.0f, 0.0f }

// FIRST_PERSON
//#define CAMERA_FIRST_PERSON_MOUSE_SENSITIVITY           0.003f
#define CAMERA_FIRST_PERSON_FOCUS_DISTANCE              25.0f
#define CAMERA_FIRST_PERSON_MIN_CLAMP                   89.0f
#define CAMERA_FIRST_PERSON_MAX_CLAMP                  -89.0f

// When walking, y-position of the player moves up-down at step frequency (swinging) but
// also the body slightly tilts left-right on every step, when all the body weight is left over one foot (tilting)
#define CAMERA_FIRST_PERSON_STEP_FREQUENCY               1.8f       // Step frequency when walking (steps per second)
#define CAMERA_FIRST_PERSON_SWINGING_DELTA               0.03f      // Maximum up-down swinging distance when walking
#define CAMERA_FIRST_PERSON_TILTING_DELTA                0.005f     // Maximum left-right tilting distance when walking

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Camera move modes (first person and third person cameras)
typedef enum {
    MOVE_FRONT = 0,
    MOVE_BACK,
    MOVE_RIGHT,
    MOVE_LEFT,
    MOVE_UP,
    MOVE_DOWN
} CameraMove;

// Camera global state context data [56 bytes]
typedef struct {
    unsigned int mode;              // Current camera mode
    float targetDistance;           // Camera distance from position to target
    float playerEyesPosition;       // Player eyes position from ground (in meters)
    Vector2 angle;                  // Camera angle in plane XZ

    // Camera movement control keys
    int moveControl[6];             // Move controls (CAMERA_FIRST_PERSON)
    int smoothZoomControl;          // Smooth zoom control key
    int altControl;                 // Alternative control key
    int panControl;                 // Pan view control key
} CameraData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static CameraData CAMERA = {        // Global CAMERA state context
    .mode = 0,
    .targetDistance = 0,
    .playerEyesPosition = 1.85f,
    .angle = { 0 },
    .moveControl = { 'W', 'S', 'D', 'A', 'E', 'Q' },
#if defined(PLATFORM_NX)
    .smoothZoomControl = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,    // Npad button A
#else
    .smoothZoomControl = 341,       // raylib: KEY_LEFT_CONTROL
#endif
    .altControl = 342,              // raylib: KEY_LEFT_ALT
    .panControl = 2                 // raylib: MOUSE_BUTTON_MIDDLE
};
#endif

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
// Returns the cameras forward vector (normalized)
Vector3 GetCameraForward(Camera *camera)
{
    return Vector3Normalize(Vector3Subtract(camera->target, camera->position));
}

// Returns the cameras up vector (normalized)
// Note: The up vector might not be perpendicular to the forward vector
Vector3 GetCameraUp(Camera *camera)
{
    return Vector3Normalize(camera->up);
}

// Returns the cameras right vector (normalized)
Vector3 GetCameraRight(Camera *camera)
{
    Vector3 forward = GetCameraForward(camera);
    Vector3 up = GetCameraUp(camera);

    return Vector3CrossProduct(forward, up);
}

// Moves the camera in its forward direction
void CameraMoveForward(Camera *camera, float distance, bool moveInWorldPlane)
{
    Vector3 forward = GetCameraForward(camera);

    if (moveInWorldPlane)
    {
        // Project vector onto world plane
        forward.y = 0;
        forward = Vector3Normalize(forward);
    }

    // Scale by distance
    forward = Vector3Scale(forward, distance);

    // Move position and target
    camera->position = Vector3Add(camera->position, forward);
    camera->target = Vector3Add(camera->target, forward);
}

// Moves the camera in its up direction
void CameraMoveUp(Camera *camera, float distance)
{
    Vector3 up = GetCameraUp(camera);

    // Scale by distance
    up = Vector3Scale(up, distance);

    // Move position and target
    camera->position = Vector3Add(camera->position, up);
    camera->target = Vector3Add(camera->target, up);
}

// Moves the camera target in its current right direction
void CameraMoveRight(Camera *camera, float distance, bool moveInWorldPlane)
{
    Vector3 right = GetCameraRight(camera);

    if (moveInWorldPlane)
    {
        // Project vector onto world plane
        right.y = 0;
        right = Vector3Normalize(right);
    }

    // Scale by distance
    right = Vector3Scale(right, distance);

    // Move position and target
    camera->position = Vector3Add(camera->position, right);
    camera->target = Vector3Add(camera->target, right);
}

// Moves the camera position closer/farther to/from the camera target
void CameraMoveToTarget(Camera *camera, float delta)
{
    float distance = Vector3Distance(camera->position, camera->target);

    // Apply delta
    distance += delta;

    // Distance must be greater than 0
    if (distance < 0) distance = 0.001f;

    // Set new distance by moving the position along the forward vector
    Vector3 forward = GetCameraForward(camera);
    camera->position = Vector3Add(camera->target, Vector3Scale(forward, -distance));
}

// Rotates the camera around its up vector
// Yaw is "looking left and right"
// If rotateAroundTarget is false, the camera rotates around its position
// Note: angle must be provided in radians
void CameraYaw(Camera *camera, float angle, bool rotateAroundTarget)
{
    // Rotation axis
    Vector3 up = GetCameraUp(camera);

    // View vector
    Vector3 target_position = Vector3Subtract(camera->target, camera->position);

    // Rotate view vector around up axis
    target_position = Vector3RotateByAxisAngle(target_position, up, angle);

    if (rotateAroundTarget)
    {
        // Move position relative to target
        camera->position = Vector3Subtract(camera->target, target_position);
    }
    else // rotate around camera.position
    {
        // Move target relative to position
        camera->target = Vector3Add(camera->position, target_position);
    }
}

#if defined(PLATFORM_NX)
// Update camera depending on selected mode
// NOTE: Camera controls depend on some raylib functions:
//       System: EnableCursor(), DisableCursor()
//       Mouse: IsMouseButtonDown(), GetMousePosition(), GetMouseWheelMove()
//       Keys:  IsKeyDown()
void UpdateCameraGamepad(Camera *camera, int gamepad)
{
    static float swingCounter[MAX_GAMEPADS] = { 0.0f, 0.0f, 0.0f, 0.0f };    // Used for 1st person swinging movement

    // TODO: Compute CAMERA.targetDistance and CAMERA.angle here (?)

    // Pan with Left Thumbstick
    Vector2 leftStickDelta = {
        -GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X), 
        GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y)
    };
    // Rotate with Right Thumbstick
    Vector2 rightStickDelta = {
        -GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_X), 
        GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_Y)
    };
    float zoomFactor = GetMouseWheelMove();
    bool szoomKey = IsGamepadButtonDown(gamepad, CAMERA.smoothZoomControl);   // Npad A

    bool direction[4] = { 
        GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y) > 0,
        GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y) < 0,
        GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X) > 0,
        GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X) < 0
    };

    // Support for multiple automatic camera modes
    // NOTE: In case of CAMERA_CUSTOM nothing happens here, user must update it manually
    switch (CAMERA.mode)
    {
        case CAMERA_FREE:           // Camera free controls, using standard 3d-content-creation scheme
        {
            // Camera zoom
            if ((CAMERA.targetDistance < CAMERA_FREE_DISTANCE_MAX_CLAMP) && (zoomFactor < 0))
            {
                CAMERA.targetDistance -= (zoomFactor*CAMERA_MOUSE_SCROLL_SENSITIVITY);
                if (CAMERA.targetDistance > CAMERA_FREE_DISTANCE_MAX_CLAMP) CAMERA.targetDistance = CAMERA_FREE_DISTANCE_MAX_CLAMP;
            }

            // Camera looking down
            else if ((camera->position.y > camera->target.y) && (CAMERA.targetDistance == CAMERA_FREE_DISTANCE_MAX_CLAMP) && (zoomFactor < 0))
            {
                camera->target.x += zoomFactor*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
                camera->target.y += zoomFactor*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
                camera->target.z += zoomFactor*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
            }
            else if ((camera->position.y > camera->target.y) && (camera->target.y >= 0))
            {
                camera->target.x += zoomFactor*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
                camera->target.y += zoomFactor*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
                camera->target.z += zoomFactor*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;

                // if (camera->target.y < 0) camera->target.y = -0.001;
            }
            else if ((camera->position.y > camera->target.y) && (camera->target.y < 0) && (zoomFactor > 0))
            {
                CAMERA.targetDistance -= (zoomFactor*CAMERA_MOUSE_SCROLL_SENSITIVITY);
                if (CAMERA.targetDistance < CAMERA_FREE_DISTANCE_MIN_CLAMP) CAMERA.targetDistance = CAMERA_FREE_DISTANCE_MIN_CLAMP;
            }
            // Camera looking up
            else if ((camera->position.y < camera->target.y) && (CAMERA.targetDistance == CAMERA_FREE_DISTANCE_MAX_CLAMP) && (zoomFactor < 0))
            {
                camera->target.x += zoomFactor*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
                camera->target.y += zoomFactor*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
                camera->target.z += zoomFactor*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
            }
            else if ((camera->position.y < camera->target.y) && (camera->target.y <= 0))
            {
                camera->target.x += zoomFactor*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
                camera->target.y += zoomFactor*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
                camera->target.z += zoomFactor*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;

                // if (camera->target.y > 0) camera->target.y = 0.001;
            }
            else if ((camera->position.y < camera->target.y) && (camera->target.y > 0) && (zoomFactor > 0))
            {
                CAMERA.targetDistance -= (zoomFactor*CAMERA_MOUSE_SCROLL_SENSITIVITY);
                if (CAMERA.targetDistance < CAMERA_FREE_DISTANCE_MIN_CLAMP) CAMERA.targetDistance = CAMERA_FREE_DISTANCE_MIN_CLAMP;
            }


            // Camera rotation
            CAMERA.angle.x += rightStickDelta.x*CAMERA_FREE_MOUSE_SENSITIVITY;
            CAMERA.angle.y += rightStickDelta.y*CAMERA_FREE_MOUSE_SENSITIVITY;

            // Angle clamp
            if (CAMERA.angle.y > CAMERA_FREE_MIN_CLAMP*DEG2RAD) CAMERA.angle.y = CAMERA_FREE_MIN_CLAMP*DEG2RAD;
            else if (CAMERA.angle.y < CAMERA_FREE_MAX_CLAMP*DEG2RAD) CAMERA.angle.y = CAMERA_FREE_MAX_CLAMP*DEG2RAD;
            if (szoomKey)
            {
                // Camera smooth zoom
                CAMERA.targetDistance += (leftStickDelta.y*CAMERA_FREE_SMOOTH_ZOOM_SENSITIVITY);
            } else {
                // Camera panning
                camera->target.x += ((leftStickDelta.x*CAMERA_FREE_MOUSE_SENSITIVITY)*cosf(CAMERA.angle.x) + (leftStickDelta.y*-CAMERA_FREE_MOUSE_SENSITIVITY)*sinf(CAMERA.angle.x)*sinf(CAMERA.angle.y))*(CAMERA.targetDistance/CAMERA_FREE_PANNING_DIVIDER);
                camera->target.y += ((leftStickDelta.y*CAMERA_FREE_MOUSE_SENSITIVITY)*cosf(CAMERA.angle.y))*(CAMERA.targetDistance/CAMERA_FREE_PANNING_DIVIDER);
                camera->target.z += ((leftStickDelta.x*-CAMERA_FREE_MOUSE_SENSITIVITY)*sinf(CAMERA.angle.x) + (leftStickDelta.y*-CAMERA_FREE_MOUSE_SENSITIVITY)*cosf(CAMERA.angle.x)*sinf(CAMERA.angle.y))*(CAMERA.targetDistance/CAMERA_FREE_PANNING_DIVIDER);
            }

            // Update camera position with changes
            camera->position.x = -sinf(CAMERA.angle.x)*CAMERA.targetDistance*cosf(CAMERA.angle.y) + camera->target.x;
            camera->position.y = -sinf(CAMERA.angle.y)*CAMERA.targetDistance + camera->target.y;
            camera->position.z = -cosf(CAMERA.angle.x)*CAMERA.targetDistance*cosf(CAMERA.angle.y) + camera->target.z;

            // L/R Bumpers to zoom out/in
            SetCameraPrevZoomFactor((Vector2) { 0.0f, zoomFactor });
            if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_TRIGGER_1))
            {
                SetCameraZoomFactor((Vector2){ 0.0f, 1.0f });
            } else if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_TRIGGER_1))
            {
                SetCameraZoomFactor((Vector2){ 0.0f, -1.0f });
            } else {
                SetCameraZoomFactor((Vector2){ 0.0f, 0.0f });
            }

        } break;
        case CAMERA_ORBITAL:        // Camera just orbits around target, only zoom allowed
        {
            CAMERA.angle.x += CAMERA_ORBITAL_SPEED*GetFrameTime();      // Camera orbit angle
            CAMERA.targetDistance -= (zoomFactor*CAMERA_MOUSE_SCROLL_SENSITIVITY);   // Camera zoom

            // Camera distance clamp
            if (CAMERA.targetDistance < CAMERA_THIRD_PERSON_DISTANCE_CLAMP) CAMERA.targetDistance = CAMERA_THIRD_PERSON_DISTANCE_CLAMP;

            // Update camera position with changes
            camera->position.x = sinf(CAMERA.angle.x)*CAMERA.targetDistance*cosf(CAMERA.angle.y) + camera->target.x;
            camera->position.y = ((CAMERA.angle.y <= 0.0f)? 1 : -1)*sinf(CAMERA.angle.y)*CAMERA.targetDistance*sinf(CAMERA.angle.y) + camera->target.y;
            camera->position.z = cosf(CAMERA.angle.x)*CAMERA.targetDistance*cosf(CAMERA.angle.y) + camera->target.z;

            // L/R Bumpers to zoom out/in
            SetCameraPrevZoomFactor((Vector2) { 0.0f, zoomFactor });
            if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_TRIGGER_1))
            {
                SetCameraZoomFactor((Vector2){ 0.0f, 1.0f });
            } else if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_TRIGGER_1))
            {
                SetCameraZoomFactor((Vector2){ 0.0f, -1.0f });
            } else {
                SetCameraZoomFactor((Vector2){ 0.0f, 0.0f });
            }

        } break;
        case CAMERA_FIRST_PERSON:   // Camera moves as in a first-person game, controls are configurable
        {
            camera->position.x += (sinf(camera->angle.x)*direction[MOVE_BACK] -
                                   sinf(camera->angle.x)*direction[MOVE_FRONT] -
                                   cosf(camera->angle.x)*direction[MOVE_LEFT] +
                                   cosf(camera->angle.x)*direction[MOVE_RIGHT])*PLAYER_MOVEMENT_SENSITIVITY*GetFrameTime();

            camera->position.y += (sinf(camera->angle.y)*direction[MOVE_FRONT] -
                                   sinf(CAMERA.angle.y)*direction[MOVE_BACK]) *PLAYER_MOVEMENT_SENSITIVITY*GetFrameTime();
                                //    1.0f*direction[MOVE_UP] - 1.0f*direction[MOVE_DOWN])*PLAYER_MOVEMENT_SENSITIVITY*GetFrameTime();

            camera->position.z += (cosf(camera->angle.x)*direction[MOVE_BACK] -
                                   cosf(camera->angle.x)*direction[MOVE_FRONT] +
                                   sinf(camera->angle.x)*direction[MOVE_LEFT] -
                                   sinf(camera->angle.x)*direction[MOVE_RIGHT])*PLAYER_MOVEMENT_SENSITIVITY*GetFrameTime();

            // Camera orientation calculation
            camera->angle.x += rightStickDelta.x*CAMERA_MOUSE_MOVE_SENSITIVITY*GetFrameTime();
            camera->angle.y += GetGamepadYAxisInverted(gamepad)*rightStickDelta.y*CAMERA_MOUSE_MOVE_SENSITIVITY*GetFrameTime();

            // Angle clamp
            if (camera->angle.y > CAMERA_FIRST_PERSON_MIN_CLAMP*DEG2RAD) camera->angle.y = CAMERA_FIRST_PERSON_MIN_CLAMP*DEG2RAD;
            else if (camera->angle.y < CAMERA_FIRST_PERSON_MAX_CLAMP*DEG2RAD) camera->angle.y = CAMERA_FIRST_PERSON_MAX_CLAMP*DEG2RAD;

            // Calculate translation matrix
            Matrix matTranslation = { 1.0f, 0.0f, 0.0f, 0.0f,
                                      0.0f, 1.0f, 0.0f, 0.0f,
                                      0.0f, 0.0f, 1.0f, (CAMERA.targetDistance/CAMERA_FREE_PANNING_DIVIDER),
                                      0.0f, 0.0f, 0.0f, 1.0f };

            // Calculate rotation matrix
            Matrix matRotation = { 1.0f, 0.0f, 0.0f, 0.0f,
                                   0.0f, 1.0f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f, 0.0f,
                                   0.0f, 0.0f, 0.0f, 1.0f };

            float cosz = cosf(0.0f);
            float sinz = sinf(0.0f);
            float cosy = cosf(-(PI*2 - camera->angle.x));
            float siny = sinf(-(PI*2 - camera->angle.x));
            float cosx = cosf(-(PI*2 - camera->angle.y));
            float sinx = sinf(-(PI*2 - camera->angle.y));

            matRotation.m0 = cosz*cosy;
            matRotation.m4 = (cosz*siny*sinx) - (sinz*cosx);
            matRotation.m8 = (cosz*siny*cosx) + (sinz*sinx);
            matRotation.m1 = sinz*cosy;
            matRotation.m5 = (sinz*siny*sinx) + (cosz*cosx);
            matRotation.m9 = (sinz*siny*cosx) - (cosz*sinx);
            matRotation.m2 = -siny;
            matRotation.m6 = cosy*sinx;
            matRotation.m10= cosy*cosx;

            // Multiply translation and rotation matrices
            Matrix matTransform = { 0 };
            matTransform.m0 = matTranslation.m0*matRotation.m0 + matTranslation.m1*matRotation.m4 + matTranslation.m2*matRotation.m8 + matTranslation.m3*matRotation.m12;
            matTransform.m1 = matTranslation.m0*matRotation.m1 + matTranslation.m1*matRotation.m5 + matTranslation.m2*matRotation.m9 + matTranslation.m3*matRotation.m13;
            matTransform.m2 = matTranslation.m0*matRotation.m2 + matTranslation.m1*matRotation.m6 + matTranslation.m2*matRotation.m10 + matTranslation.m3*matRotation.m14;
            matTransform.m3 = matTranslation.m0*matRotation.m3 + matTranslation.m1*matRotation.m7 + matTranslation.m2*matRotation.m11 + matTranslation.m3*matRotation.m15;
            matTransform.m4 = matTranslation.m4*matRotation.m0 + matTranslation.m5*matRotation.m4 + matTranslation.m6*matRotation.m8 + matTranslation.m7*matRotation.m12;
            matTransform.m5 = matTranslation.m4*matRotation.m1 + matTranslation.m5*matRotation.m5 + matTranslation.m6*matRotation.m9 + matTranslation.m7*matRotation.m13;
            matTransform.m6 = matTranslation.m4*matRotation.m2 + matTranslation.m5*matRotation.m6 + matTranslation.m6*matRotation.m10 + matTranslation.m7*matRotation.m14;
            matTransform.m7 = matTranslation.m4*matRotation.m3 + matTranslation.m5*matRotation.m7 + matTranslation.m6*matRotation.m11 + matTranslation.m7*matRotation.m15;
            matTransform.m8 = matTranslation.m8*matRotation.m0 + matTranslation.m9*matRotation.m4 + matTranslation.m10*matRotation.m8 + matTranslation.m11*matRotation.m12;
            matTransform.m9 = matTranslation.m8*matRotation.m1 + matTranslation.m9*matRotation.m5 + matTranslation.m10*matRotation.m9 + matTranslation.m11*matRotation.m13;
            matTransform.m10 = matTranslation.m8*matRotation.m2 + matTranslation.m9*matRotation.m6 + matTranslation.m10*matRotation.m10 + matTranslation.m11*matRotation.m14;
            matTransform.m11 = matTranslation.m8*matRotation.m3 + matTranslation.m9*matRotation.m7 + matTranslation.m10*matRotation.m11 + matTranslation.m11*matRotation.m15;
            matTransform.m12 = matTranslation.m12*matRotation.m0 + matTranslation.m13*matRotation.m4 + matTranslation.m14*matRotation.m8 + matTranslation.m15*matRotation.m12;
            matTransform.m13 = matTranslation.m12*matRotation.m1 + matTranslation.m13*matRotation.m5 + matTranslation.m14*matRotation.m9 + matTranslation.m15*matRotation.m13;
            matTransform.m14 = matTranslation.m12*matRotation.m2 + matTranslation.m13*matRotation.m6 + matTranslation.m14*matRotation.m10 + matTranslation.m15*matRotation.m14;
            matTransform.m15 = matTranslation.m12*matRotation.m3 + matTranslation.m13*matRotation.m7 + matTranslation.m14*matRotation.m11 + matTranslation.m15*matRotation.m15;

            camera->target.x = camera->position.x - matTransform.m12;
            camera->target.y = camera->position.y - matTransform.m13;
            camera->target.z = camera->position.z - matTransform.m14;

            // Camera position update
            // NOTE: On CAMERA_FIRST_PERSON player Y-movement is limited to player 'eyes position'
            camera->position.y = CAMERA.playerEyesPosition;

            // Camera swinging (y-movement), only when walking (some key pressed)
            for (int i = 0; i < MAX_GAMEPADS; i++) {if (direction[i]){ swingCounter[gamepad] += GetFrameTime(); break; }}
            camera->position.y -= sinf(2*PI*CAMERA_FIRST_PERSON_STEP_FREQUENCY*swingCounter[gamepad])*CAMERA_FIRST_PERSON_SWINGING_DELTA;

            // Camera waiving (xz-movement), only when walking (some key pressed)
            camera->up.x = sinf(2*PI*CAMERA_FIRST_PERSON_STEP_FREQUENCY*swingCounter[gamepad])*CAMERA_FIRST_PERSON_TILTING_DELTA;
            camera->up.z = -sinf(2*PI*CAMERA_FIRST_PERSON_STEP_FREQUENCY*swingCounter[gamepad])*CAMERA_FIRST_PERSON_TILTING_DELTA;

        } break;
        case CAMERA_THIRD_PERSON:   // Camera moves as in a third-person game, following target at a distance, controls are configurable
        {
            camera->position.x += (sinf(CAMERA.angle.x)*direction[MOVE_BACK] -
                                   sinf(CAMERA.angle.x)*direction[MOVE_FRONT] -
                                   cosf(CAMERA.angle.x)*direction[MOVE_LEFT] +
                                   cosf(CAMERA.angle.x)*direction[MOVE_RIGHT])*PLAYER_MOVEMENT_SENSITIVITY*GetFrameTime();

            camera->position.y += (sinf(CAMERA.angle.y)*direction[MOVE_FRONT] -
                                   sinf(CAMERA.angle.y)*direction[MOVE_BACK] +
                                   1.0f*direction[MOVE_UP] - 1.0f*direction[MOVE_DOWN])*PLAYER_MOVEMENT_SENSITIVITY*GetFrameTime();

            camera->position.z += (cosf(CAMERA.angle.x)*direction[MOVE_BACK] -
                                   cosf(CAMERA.angle.x)*direction[MOVE_FRONT] +
                                   sinf(CAMERA.angle.x)*direction[MOVE_LEFT] -
                                   sinf(CAMERA.angle.x)*direction[MOVE_RIGHT])*PLAYER_MOVEMENT_SENSITIVITY*GetFrameTime();

            // Camera orientation calculation
            CAMERA.angle.x += (leftStickDelta.x*-CAMERA_MOUSE_MOVE_SENSITIVITY);
            CAMERA.angle.y += (leftStickDelta.y*-CAMERA_MOUSE_MOVE_SENSITIVITY);

            // Angle clamp
            if (CAMERA.angle.y > CAMERA_THIRD_PERSON_MIN_CLAMP*DEG2RAD) CAMERA.angle.y = CAMERA_THIRD_PERSON_MIN_CLAMP*DEG2RAD;
            else if (CAMERA.angle.y < CAMERA_THIRD_PERSON_MAX_CLAMP*DEG2RAD) CAMERA.angle.y = CAMERA_THIRD_PERSON_MAX_CLAMP*DEG2RAD;

            // Camera zoom
            CAMERA.targetDistance -= (zoomFactor*CAMERA_MOUSE_SCROLL_SENSITIVITY);

            // Camera distance clamp
            if (CAMERA.targetDistance < CAMERA_THIRD_PERSON_DISTANCE_CLAMP) CAMERA.targetDistance = CAMERA_THIRD_PERSON_DISTANCE_CLAMP;

            camera->position.x = sinf(CAMERA.angle.x)*CAMERA.targetDistance*cosf(CAMERA.angle.y) + camera->target.x;

            if (CAMERA.angle.y <= 0.0f) camera->position.y = sinf(CAMERA.angle.y)*CAMERA.targetDistance*sinf(CAMERA.angle.y) + camera->target.y;
            else camera->position.y = -sinf(CAMERA.angle.y)*CAMERA.targetDistance*sinf(CAMERA.angle.y) + camera->target.y;

            camera->position.z = cosf(CAMERA.angle.x)*CAMERA.targetDistance*cosf(CAMERA.angle.y) + camera->target.z;

        } break;
        case CAMERA_CUSTOM: break;
        default: break;
    }
}
#endif

// Rotates the camera around its right vector
// Pitch is "looking up and down"
// lockView prevents camera overrotation (aka "somersaults")
// If rotateAroundTarget is false, the camera rotates around its position
// rotateUp rotates the up direction as well (typically only usefull in CAMERA_FREE)
// Note: angle must be provided in radians
void CameraPitch(Camera *camera, float angle, bool lockView, bool rotateAroundTarget, bool rotateUp)
{
    // Up direction
    Vector3 up = GetCameraUp(camera);

    // View vector
    Vector3 target_position = Vector3Subtract(camera->target, camera->position);

    if (lockView)
    {
        // In these camera modes we clamp the Pitch angle
        // to allow only viewing straight up or down.

        // Clamp view up
        float max_angle_up = Vector3Angle(up, target_position);
        max_angle_up -= 0.001f; // avoid numerical errors
        if (angle > max_angle_up) angle = max_angle_up;

        // Clamp view down
        float max_angle_down = Vector3Angle(Vector3Negate(up), target_position);
        max_angle_down *= -1.0f; // downwards angle is negative
        max_angle_down += 0.001f; // avoid numerical errors
        if (angle < max_angle_down) angle = max_angle_down;
    }

    // Rotation axis
    Vector3 right = GetCameraRight(camera);

    // Rotate view vector around right axis
    target_position = Vector3RotateByAxisAngle(target_position, right, angle);

    if (rotateAroundTarget)
    {
        // Move position relative to target
        camera->position = Vector3Subtract(camera->target, target_position);
    }
    else // rotate around camera.position
    {
        // Move target relative to position
        camera->target = Vector3Add(camera->position, target_position);
    }

    if (rotateUp)
    {
        // Rotate up direction around right axis
        camera->up = Vector3RotateByAxisAngle(camera->up, right, angle);
    }
}

// Rotates the camera around its forward vector
// Roll is "turning your head sideways to the left or right"
// Note: angle must be provided in radians
void CameraRoll(Camera *camera, float angle)
{
    // Rotation axis
    Vector3 forward = GetCameraForward(camera);

    // Rotate up direction around forward axis
    camera->up = Vector3RotateByAxisAngle(camera->up, forward, angle);
}

// Returns the camera view matrix
Matrix GetCameraViewMatrix(Camera *camera)
{
    return MatrixLookAt(camera->position, camera->target, camera->up);
}

// Returns the camera projection matrix
Matrix GetCameraProjectionMatrix(Camera *camera, float aspect)
{
    if (camera->projection == CAMERA_PERSPECTIVE)
    {
        return MatrixPerspective(camera->fovy*DEG2RAD, aspect, CAMERA_CULL_DISTANCE_NEAR, CAMERA_CULL_DISTANCE_FAR);
    }
    else if (camera->projection == CAMERA_ORTHOGRAPHIC)
    {
        double top = camera->fovy/2.0;
        double right = top*aspect;

        return MatrixOrtho(-right, right, -top, top, CAMERA_CULL_DISTANCE_NEAR, CAMERA_CULL_DISTANCE_FAR);
    }

    return MatrixIdentity();
}

#ifndef CAMERA_STANDALONE
// Update camera position for selected mode
// Camera mode: CAMERA_FREE, CAMERA_FIRST_PERSON, CAMERA_THIRD_PERSON, CAMERA_ORBITAL or CUSTOM
void UpdateCamera(Camera *camera, int mode)
{
    Vector2 mousePositionDelta = GetMouseDelta();

    bool moveInWorldPlane = ((mode == CAMERA_FIRST_PERSON) || (mode == CAMERA_THIRD_PERSON));
    bool rotateAroundTarget = ((mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL));
    bool lockView = ((mode == CAMERA_FIRST_PERSON) || (mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL));
    bool rotateUp = (mode == CAMERA_FREE);

    if (mode == CAMERA_ORBITAL)
    {
        // Orbital can just orbit
        Matrix rotation = MatrixRotate(GetCameraUp(camera), CAMERA_ORBITAL_SPEED*GetFrameTime());
        Vector3 view = Vector3Subtract(camera->position, camera->target);
        view = Vector3Transform(view, rotation);
        camera->position = Vector3Add(camera->target, view);
    }
    else
    {
        // Camera rotation
        if (IsKeyDown(KEY_DOWN)) CameraPitch(camera, -CAMERA_ROTATION_SPEED, lockView, rotateAroundTarget, rotateUp);
        if (IsKeyDown(KEY_UP)) CameraPitch(camera, CAMERA_ROTATION_SPEED, lockView, rotateAroundTarget, rotateUp);
        if (IsKeyDown(KEY_RIGHT)) CameraYaw(camera, -CAMERA_ROTATION_SPEED, rotateAroundTarget);
        if (IsKeyDown(KEY_LEFT)) CameraYaw(camera, CAMERA_ROTATION_SPEED, rotateAroundTarget);
        if (IsKeyDown(KEY_Q)) CameraRoll(camera, -CAMERA_ROTATION_SPEED);
        if (IsKeyDown(KEY_E)) CameraRoll(camera, CAMERA_ROTATION_SPEED);

        CameraYaw(camera, -mousePositionDelta.x*CAMERA_MOUSE_MOVE_SENSITIVITY, rotateAroundTarget);
        CameraPitch(camera, -mousePositionDelta.y*CAMERA_MOUSE_MOVE_SENSITIVITY, lockView, rotateAroundTarget, rotateUp);

        // Camera movement
        if (IsKeyDown(KEY_W)) CameraMoveForward(camera, CAMERA_MOVE_SPEED, moveInWorldPlane);
        if (IsKeyDown(KEY_A)) CameraMoveRight(camera, -CAMERA_MOVE_SPEED, moveInWorldPlane);
        if (IsKeyDown(KEY_S)) CameraMoveForward(camera, -CAMERA_MOVE_SPEED, moveInWorldPlane);
        if (IsKeyDown(KEY_D)) CameraMoveRight(camera, CAMERA_MOVE_SPEED, moveInWorldPlane);
        //if (IsKeyDown(KEY_SPACE)) CameraMoveUp(camera, CAMERA_MOVE_SPEED);
        //if (IsKeyDown(KEY_LEFT_CONTROL)) CameraMoveUp(camera, -CAMERA_MOVE_SPEED);
    }

    if ((mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL))
    {
        // Zoom target distance
        CameraMoveToTarget(camera, -GetMouseWheelMove());
        if (IsKeyPressed(KEY_KP_SUBTRACT)) CameraMoveToTarget(camera, 2.0f);
        if (IsKeyPressed(KEY_KP_ADD)) CameraMoveToTarget(camera, -2.0f);
    }
}
#endif // !CAMERA_STANDALONE

#endif // CAMERA_IMPLEMENTATION