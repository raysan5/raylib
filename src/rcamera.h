/*******************************************************************************************
*
*   rcamera - Basic camera system for multiple camera modes
*
*   NOTE: Memory footprint of this library is aproximately ??? TODO bytes (global variables)
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
*       Christoph Wagner:   Redesign (2022)
*       Marc Palau:         Initial implementation (2014)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2015-2022 Ramon Santamaria (@raysan5)
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

// The only dependency // TODO review standalone mode
#include "raymath.h"

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...
#if defined(CAMERA_STANDALONE)
#define CAMERA_CULL_DISTANCE_NEAR 0.01
#define CAMERA_CULL_DISTANCE_FAR 1000.0
#else
#define CAMERA_CULL_DISTANCE_NEAR  RL_CULL_DISTANCE_NEAR
#define CAMERA_CULL_DISTANCE_FAR  RL_CULL_DISTANCE_FAR
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
        int type;               // Camera type, defines projection type: CAMERA_PERSPECTIVE or CAMERA_ORTHOGRAPHIC
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
//...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

#if defined(CAMERA_STANDALONE)
void SetCameraMode(Camera camera, int mode);                // Set camera mode (multiple camera modes available)
void UpdateCamera(Camera *camera);                          // Update camera position for selected mode

void SetCameraPanControl(int keyPan);                       // Set camera pan key to combine with mouse movement (free camera)
void SetCameraAltControl(int keyAlt);                       // Set camera alt key to combine with mouse movement (free camera)
void SetCameraSmoothZoomControl(int szoomKey);              // Set camera smooth zoom key to combine with mouse (free camera)
void SetCameraMoveControls(int keyFront, int keyBack,
                           int keyRight, int keyLeft,
                           int keyUp, int keyDown);         // Set camera move controls (1st person and 3rd person cameras)
#endif

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

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------

// Camera mouse movement sensitivity
#define CAMERA_MOUSE_MOVE_SENSITIVITY                   0.5f    // TODO: it should be independant of framerate
#define CAMERA_MOUSE_SCROLL_SENSITIVITY                 1.5f

#define CAMERA_ORBITAL_SPEED                            0.01f       // Radians per frame


// When walking, y-position of the player moves up-down at step frequency (swinging) but
// also the body slightly tilts left-right on every step, when all the body weight is left over one foot (tilting)
#define CAMERA_FIRST_PERSON_STEP_FREQUENCY               1.8f       // Step frequency when walking (steps per second)
#define CAMERA_FIRST_PERSON_SWINGING_DELTA               0.03f      // Maximum up-down swinging distance when walking
#define CAMERA_FIRST_PERSON_TILTING_DELTA                0.005f     // Maximum left-right tilting distance when walking

// PLAYER (used by camera)
#define PLAYER_MOVEMENT_SENSITIVITY                     2.0f

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Camera move modes (first person and third person cameras)
typedef enum {
    MOVE_FORWARD = 0,
    MOVE_BACK,
    MOVE_RIGHT,
    MOVE_LEFT,
    MOVE_UP,
    MOVE_DOWN
} CameraMove;

// Camera global state context data [56 bytes]
// typedef struct {
//     unsigned int mode;              // Current camera mode
//     float targetDistance;           // Camera distance from position to target
//     float playerEyesPosition;       // Player eyes position from ground (in meters)
//     Vector2 angle;                  // Camera angle in plane XZ
//     Vector2 previousMousePosition;  // Previous mouse position

//     // Camera movement control keys
//     int moveControl[6];             // Move controls (CAMERA_FIRST_PERSON)
//     int smoothZoomControl;          // Smooth zoom control key
//     int altControl;                 // Alternative control key
//     int panControl;                 // Pan view control key
// } CameraData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------

// static CameraData CAMERA = {        // Global CAMERA state context
//     .mode = 0,
//     .targetDistance = 0,
//     .playerEyesPosition = 1.85f,
//     .angle = { 0 },
//     .previousMousePosition = { 0 },
//     .moveControl = { 'W', 'S', 'D', 'A', 'E', 'Q' },
//     .smoothZoomControl = 341,       // raylib: KEY_LEFT_CONTROL
//     .altControl = 342,              // raylib: KEY_LEFT_ALT
//     .panControl = 2                 // raylib: MOUSE_BUTTON_MIDDLE
// };

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
// Rotates the given vector around the given axis
// Note: angle must be provided in radians
Vector3 Vector3RotateAxis(Vector3 v, Vector3 axis, float angle) { // TODO there is a better way without the intermediate matrix
    Matrix rot = MatrixRotate(axis, angle);
    return Vector3Transform(v, rot);
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// TODO declare all functions in raylib.h and standalone "header"
void CameraPitch(Camera3D* camera, float angle);
void CameraYaw(Camera3D* camera, float angle);
void CameraRoll(Camera3D* camera, float angle);

// Select camera mode (multiple camera modes available)
void SetCameraMode(Camera3D* camera, int mode)
{
    camera->mode = mode;

    // TODO 

    // TODO Reset roll and fix overrotation (i.e. somersaults)

}

// Returns the cameras current forward vector (normalized)
Vector3 GetCameraForward(Camera3D* camera) {
    return Vector3Normalize(Vector3Subtract(camera->target, camera->position));
}

// Returns the cameras current up vector (normalized)
Vector3 GetCameraUp(Camera3D* camera) {
    // Note: The up vector might not be perpendicular to the forward vector
    Vector3 forward = Vector3Subtract(camera->target, camera->position);
    Vector3OrthoNormalize(&forward, &camera->up);
    return camera->up;
}

// Returns the cameras current right vector (normalized)
Vector3 GetCameraRight(Camera3D* camera) {
    Vector3 forward = GetCameraForward(camera);
    Vector3 up = GetCameraUp(camera);
    return Vector3CrossProduct(forward, up);
}

// Moves the camera in its current forward vector direction
void CameraMoveForward(Camera3D *camera, float distance) {
    Vector3 forward = GetCameraForward(camera);

    if (camera->mode == CAMERA_FIRST_PERSON ||
        camera->mode == CAMERA_THIRD_PERSON)
    {
        // Project vector onto world plane
        forward.y = 0;
        forward = Vector3Normalize(forward);
    }

    forward = Vector3Scale(forward, distance);

    camera->position = Vector3Add(camera->position, forward);
    camera->target = Vector3Add(camera->target, forward);
}

// Moves the camera in its current up vector direction
void CameraMoveUp(Camera3D *camera, float distance) {
    Vector3 up = GetCameraUp(camera);
    
    up = Vector3Scale(up, distance);

    camera->position = Vector3Add(camera->position, up);
    camera->target = Vector3Add(camera->target, up);
}

// Moves the camera target in its current right vector direction
void CameraMoveRight(Camera3D *camera, float distance) {
    Vector3 right = GetCameraRight(camera);

    if (camera->mode == CAMERA_FIRST_PERSON ||
        camera->mode == CAMERA_THIRD_PERSON)
    {
        // Project vector onto world plane
        right.y = 0;
        right = Vector3Normalize(right);
    }

    right = Vector3Scale(right, distance);

    camera->position = Vector3Add(camera->position, right);
    camera->target = Vector3Add(camera->target, right);
}

// Rotates the camera around its current up vector
// Yaw is "looking left and right"
// Note: angle must be provided in radians
void CameraYaw(Camera3D *camera, float angle) {
    // Rotation axis
    Vector3 up = GetCameraUp(camera);

    Vector3 target_position = Vector3Subtract(camera->target, camera->position);

    // Rotate target
    target_position = Vector3RotateAxis(target_position, up, angle);
    camera->target = Vector3Add(camera->position, target_position);
}

// Rotates the camera around its current right vector
// Pitch is "looking up and down"
// Note: angle must be provided in radians
void CameraPitch(Camera3D *camera, float angle) {
    
    if (camera->mode == CAMERA_FIRST_PERSON ||
        camera->mode == CAMERA_THIRD_PERSON ||
        camera->mode == CAMERA_ORBITAL)
    {
        // In these camera modes we clamp the Pitch angle
        // to allow only viewing straight up or down.
        // i.e. prevent somersaults

        // TODO
    }

    // Rotation axis
    Vector3 right = GetCameraRight(camera);

    Vector3 target_position = Vector3Subtract(camera->target, camera->position);

    // Rotate target
    target_position = Vector3RotateAxis(target_position, right, angle);
    camera->target = Vector3Add(camera->position, target_position);

    // Rotate up direction
    camera->up = Vector3RotateAxis(camera->up, right, angle);
}

// Rotates the camera around its current forward vector
// Roll is "turning your head sideways to the left or right"
// Note: angle must be provided in radians
void CameraRoll(Camera3D *camera, float angle) {
    // Rotation axis
    Vector3 forward = GetCameraForward(camera);

    // Rotate up direction
    camera->up = Vector3RotateAxis(camera->up, forward, angle);
}

// Returns the current camera view matrix
Matrix GetCameraViewMatrix(Camera3D* camera) {
    // Note: MatrixLookAt normalizes up internally
    //       TODO review up needs to be perpendicular?
    return MatrixLookAt(camera->position, camera->target, camera->up);
}

// Returns the current camera projection matrix
Matrix GetCameraProjectionMatrix(Camera3D* camera, float aspect) {
    if (camera->projection == CAMERA_PERSPECTIVE)
    {
        return MatrixPerspective(camera->fovy * DEG2RAD, aspect, CAMERA_CULL_DISTANCE_NEAR, CAMERA_CULL_DISTANCE_FAR);
    }
    else if (camera->projection == CAMERA_ORTHOGRAPHIC)
    {
        double top = camera->fovy / 2.0;
        double right = top * aspect;
        return MatrixOrtho(-right, right, -top, top, CAMERA_CULL_DISTANCE_NEAR, CAMERA_CULL_DISTANCE_FAR);
    }
    
    return MatrixIdentity();
}

static bool on_init = true;
void UpdateCamera(Camera3D* camera) {

    // TODO review
    const float CAMERA_MOVE_SPEED = 0.03f;

    // TODO Input detection is raylib-dependant

    // Avoid inital mouse "jump"
    if (on_init) {
        SetMousePosition(0, 0);
        on_init = false;
    }
    Vector2 mousePositionDelta = GetMouseDelta();
    
    // Camera movement
    if (IsKeyDown(KEY_W)) CameraMoveForward(camera, CAMERA_MOVE_SPEED * 3);
    if (IsKeyDown(KEY_S)) CameraMoveForward(camera, -CAMERA_MOVE_SPEED * 3);
    if (IsKeyDown(KEY_D)) CameraMoveRight(camera, CAMERA_MOVE_SPEED * 3);
    if (IsKeyDown(KEY_A)) CameraMoveRight(camera, -CAMERA_MOVE_SPEED * 3);
    if (IsKeyDown(KEY_SPACE)) CameraMoveUp(camera, CAMERA_MOVE_SPEED * 3);
    if (IsKeyDown(KEY_LEFT_CONTROL)) CameraMoveUp(camera, -CAMERA_MOVE_SPEED * 3);

    // Camera rotation
    if (IsKeyDown(KEY_DOWN)) CameraPitch(camera, -CAMERA_MOVE_SPEED);
    if (IsKeyDown(KEY_UP)) CameraPitch(camera, CAMERA_MOVE_SPEED);
    if (IsKeyDown(KEY_RIGHT)) CameraYaw(camera, -CAMERA_MOVE_SPEED);
    if (IsKeyDown(KEY_LEFT)) CameraYaw(camera, CAMERA_MOVE_SPEED);
    if (IsKeyDown(KEY_Q)) CameraRoll(camera, -CAMERA_MOVE_SPEED);
    if (IsKeyDown(KEY_E)) CameraRoll(camera, CAMERA_MOVE_SPEED);


    CameraYaw(camera, mousePositionDelta.x * -CAMERA_MOUSE_MOVE_SENSITIVITY);
    CameraPitch(camera, mousePositionDelta.y * -CAMERA_MOUSE_MOVE_SENSITIVITY);

#if 0

    // Adjust camera target_distance
    camera->target_distance -= GetMouseWheelMove();
    if (IsKeyPressed(KEY_KP_SUBTRACT)) camera->target_distance -= 1.0f;
    if (IsKeyPressed(KEY_KP_ADD)) camera->target_distance += 1.0f;


    // Switch camera mode
    if (IsKeyPressed(KEY_ONE)) SetCameraMode(camera, CAMERA_FREE);
    if (IsKeyPressed(KEY_TWO)) SetCameraMode(camera, CAMERA_FIRST_PERSON);
    if (IsKeyPressed(KEY_THREE)) SetCameraMode(camera, CAMERA_THIRD_PERSON);
    if (IsKeyPressed(KEY_FOUR)) SetCameraMode(camera, CAMERA_ORBITAL);

    // Switch camera projection
    if (IsKeyPressed(KEY_P)) {
        if (camera->projection == CAMERA_PERSPECTIVE) {
            // Create isometric view
            InitializeCamera(camera, camera->aspect);
            camera->mode = CAMERA_THIRD_PERSON;
            camera->projection = CAMERA_ORTHOGRAPHIC;
            camera->fovy = 20.0f; // near plane width in orthografic
            camera->target_distance = 100.0f; // influences only clip distance
            CameraYaw(camera, -135 * DEG2RAD);
            CameraPitch(camera, 45 * DEG2RAD);
        }
        else if (camera->projection == CAMERA_ORTHOGRAPHIC) {
            // Reset default view
            InitializeCamera(camera, camera->aspect);
        }
    }
#endif

}

// TODO can we provide compatibility with the old rcamera usage?
#if 0

// Set camera pan key to combine with mouse movement (free camera)
void SetCameraPanControl(int keyPan) { CAMERA.panControl = keyPan; }

// Set camera alt key to combine with mouse movement (free camera)
void SetCameraAltControl(int keyAlt) { CAMERA.altControl = keyAlt; }

// Set camera smooth zoom key to combine with mouse (free camera)
void SetCameraSmoothZoomControl(int szoomKey) { CAMERA.smoothZoomControl = szoomKey; }

// Set camera move controls (1st person and 3rd person cameras)
void SetCameraMoveControls(int keyFront, int keyBack, int keyRight, int keyLeft, int keyUp, int keyDown)
{
    CAMERA.moveControl[MOVE_FRONT] = keyFront;
    CAMERA.moveControl[MOVE_BACK] = keyBack;
    CAMERA.moveControl[MOVE_RIGHT] = keyRight;
    CAMERA.moveControl[MOVE_LEFT] = keyLeft;
    CAMERA.moveControl[MOVE_UP] = keyUp;
    CAMERA.moveControl[MOVE_DOWN] = keyDown;
}
#endif


#endif // CAMERA_IMPLEMENTATION
