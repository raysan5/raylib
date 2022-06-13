/*******************************************************************************************
*
*   rcamera - Basic camera system for multiple camera modes
*
*   NOTE: Memory footprint of this library is aproximately ??? TODO bytes (global variables)
*
*   CONFIGURATION: TODO
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
*       Christoph Wagner:   Quaternion-based redesign (2022)
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
// NOTE: Raylib (and OpenGL) uses a right-handed coordinate system
//       Just invert CAMERA_WORLD_FRONT for a left-handed coordinate system
#define CAMERA_WORLD_FRONT (Vector3) { 0.0f, 0.0f, -1.0f }
#define CAMERA_WORLD_UP (Vector3) { 0.0f, 1.0f, 0.0f }
#define CAMERA_WORLD_RIGHT (Vector3) { 1.0f, 0.0f, 0.0f }

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

// TODO review

#ifdef __cplusplus
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

#ifdef __cplusplus
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
// TODO review all defines

#define CAMERA_MOVE_SPEED                   0.03f

// Camera mouse movement sensitivity
#define CAMERA_MOUSE_MOVE_SENSITIVITY                   0.003f
#define CAMERA_MOUSE_SCROLL_SENSITIVITY                 1.5f

#define CAMERA_ORBITAL_SPEED                            0.01f       // Radians per frame


#define CAMERA_FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER  8.0f
#define CAMERA_FIRST_PERSON_STEP_DIVIDER                30.0f
#define CAMERA_FIRST_PERSON_WAVING_DIVIDER              200.0f

// PLAYER (used by camera)
#define PLAYER_MOVEMENT_SENSITIVITY                     20.0f

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


//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------


//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// TODO declare all functions in raylib.h and standalone "header"
void CameraPitch(Camera3D* camera, float angle);
void CameraYaw(Camera3D* camera, float angle);
void CameraRoll(Camera3D* camera, float angle);


// Initializes a camera with default values (Can be used to reset a camera)
void InitializeCamera(Camera3D* camera, float aspect) {
    camera->mode = CAMERA_FIRST_PERSON;
    camera->target_position = (Vector3){ 0.0f, 2.0f, 0.0f };
    camera->target_distance = 0.0f;
    camera->orientation = QuaternionIdentity();
    camera->fovy = 60.0f;
    camera->aspect = aspect;
    camera->projection = CAMERA_PERSPECTIVE;
}

// Select camera mode (multiple camera modes available)
void SetCameraMode(Camera3D* camera, int mode)
{
    camera->mode = mode;

    if (mode == CAMERA_FREE)
    {
        camera->target_distance = 0.0f;
        return;
    }
    else if (mode == CAMERA_FIRST_PERSON)
    {
        camera->target_distance = 0.0f;
    }
    else if (mode == CAMERA_THIRD_PERSON)
    {
        camera->target_distance = 4.0f;
    }
    else if (mode == CAMERA_ORBITAL)
    {
        camera->target_distance = 20.0f;
    }

    // Reset roll and fix overrotation (i.e. somersaults)

    Vector3 front = GetCameraFront(camera);
    // Reduce numerical errors in the following trig functions
    front = Vector3Normalize(front);

    float yaw = atan2f(front.x, CAMERA_WORLD_FRONT.z * front.z);
    float pitch = -asinf(front.y);

    camera->orientation = QuaternionIdentity();
    CameraPitch(camera, pitch);
    CameraYaw(camera, yaw);
}

// Returns the cameras current front vector
Vector3 GetCameraFront(Camera3D* camera) {
    return Vector3RotateByQuaternion(CAMERA_WORLD_FRONT, QuaternionInvert(camera->orientation));
}

// Returns the cameras current up vector
Vector3 GetCameraUp(Camera3D* camera) {
    return Vector3RotateByQuaternion(CAMERA_WORLD_UP, QuaternionInvert(camera->orientation));
}

// Returns the cameras current right vector
Vector3 GetCameraRight(Camera3D* camera) {
    return Vector3RotateByQuaternion(CAMERA_WORLD_RIGHT, QuaternionInvert(camera->orientation));
}

// Returns the cameras current eye position
// Note: This is equivalent to target_position if target_distance == 0
//       (i.e. in modes CAMERA_FREE and CAMERA_FIRST_PERSON)
Vector3 GetCameraEyePosition(Camera3D* camera) {
    Vector3 back = Vector3Negate(GetCameraFront(camera));

    Vector3 offset = Vector3Scale(back, camera->target_distance);

    return Vector3Add(camera->target_position, offset);
}

// Moves the camera in its current front vector direction
void CameraMoveForward(Camera3D *camera, float distance) {
    Vector3 front = GetCameraFront(camera);

    if (camera->mode == CAMERA_FIRST_PERSON ||
        camera->mode == CAMERA_THIRD_PERSON)
    {
        // Project vector onto world plane
        front.y = 0;
        front = Vector3Normalize(front);
    }

    camera->target_position = Vector3Add(camera->target_position, Vector3Scale(front, distance));
}

// Moves the camera in its current up vector direction
void CameraMoveUp(Camera3D *camera, float distance) {
    Vector3 up = { 0 }; 
    
    if (camera->mode == CAMERA_FREE ||
        camera->mode == CAMERA_ORBITAL)
    {
        up = GetCameraUp(camera);
    }
    else if (camera->mode == CAMERA_FIRST_PERSON ||
             camera->mode == CAMERA_THIRD_PERSON)
    {
        up = CAMERA_WORLD_UP;
    }

    camera->target_position = Vector3Add(camera->target_position, Vector3Scale(up, distance));
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

    camera->target_position = Vector3Add(camera->target_position, Vector3Scale(right, distance));
}

// Rotates the camera around its current up vector
// Yaw is "looking left and right"
// Note: angle must be provided in radians
void CameraYaw(Camera3D *camera, float angle) {
    Quaternion rotationUp = QuaternionFromAxisAngle(CAMERA_WORLD_UP, angle);

    if (camera->mode == CAMERA_FIRST_PERSON||
        camera->mode == CAMERA_THIRD_PERSON ||
        camera->mode == CAMERA_ORBITAL)
    {
        // NOTE: The multiplication order is important, not to induce roll from pitch+yaw
        camera->orientation = QuaternionMultiply(camera->orientation, rotationUp);
    }
    else if (camera->mode == CAMERA_FREE)
    {
        camera->orientation = QuaternionMultiply(rotationUp, camera->orientation);
    }
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
        Vector3 front = GetCameraFront(camera);

        // clamp angle upwards
        float max_angle = Vector3Angle(CAMERA_WORLD_UP, front);
        max_angle -= 0.00001f; // avoid numerical errors
        angle = -1.0f * MIN(-angle, max_angle);

        // clamp angle downwards
        max_angle = Vector3Angle(Vector3Negate(CAMERA_WORLD_UP), front);
        //max_angle = PI - max_angle; // TODO
        max_angle -= 0.00001f; // avoid numerical errors
        angle = MIN(angle, max_angle);
    }

    Quaternion rotationRight = QuaternionFromAxisAngle(CAMERA_WORLD_RIGHT, angle);
    camera->orientation = QuaternionMultiply(rotationRight, camera->orientation);
}

// Rotates the camera around its current front vector
// Roll is "turning your head sideways to the left or right"
// Note: angle must be provided in radians
void CameraRoll(Camera3D *camera, float angle) {
    Quaternion rotationFront = QuaternionFromAxisAngle(CAMERA_WORLD_FRONT, angle);
    camera->orientation = QuaternionMultiply(rotationFront, camera->orientation);
}

// Returns the current camera view matrix
Matrix GetCameraViewMatrix(Camera3D* camera) {
    // NOTE: The orientation quaternion de-normalizes over time
    //       So we re-normalize it once per frame when the view matrix is requested
    camera->orientation = QuaternionNormalize(camera->orientation);
    
    Vector3 eye = GetCameraEyePosition(camera);
    
    Matrix translation = MatrixTranslate(-eye.x, -eye.y, -eye.z);
    Matrix rotation = QuaternionToMatrix(camera->orientation);

    return MatrixMultiply(translation, rotation);
}

// Returns the current camera projection matrix
Matrix GetCameraProjectionMatrix(Camera3D* camera) {

    if (camera->projection == CAMERA_PERSPECTIVE)
    {
        return MatrixPerspective(camera->fovy * DEG2RAD, camera->aspect, CAMERA_CULL_DISTANCE_NEAR, CAMERA_CULL_DISTANCE_FAR);
    }
    else if (camera->projection == CAMERA_ORTHOGRAPHIC)
    {
        double top = camera->fovy / 2.0;
        double right = top * camera->aspect;
        return MatrixOrtho(-right, right, -top, top, CAMERA_CULL_DISTANCE_NEAR, CAMERA_CULL_DISTANCE_FAR);
    }
    
    return MatrixIdentity();
}

static bool on_init = true;
void UpdateCamera(Camera3D* camera) {

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
    if (IsKeyDown(KEY_DOWN)) CameraPitch(camera, CAMERA_MOVE_SPEED);
    if (IsKeyDown(KEY_UP)) CameraPitch(camera, -CAMERA_MOVE_SPEED);
    if (IsKeyDown(KEY_RIGHT)) CameraYaw(camera, CAMERA_MOVE_SPEED);
    if (IsKeyDown(KEY_LEFT)) CameraYaw(camera, -CAMERA_MOVE_SPEED);
    if (IsKeyDown(KEY_Q)) CameraRoll(camera, CAMERA_MOVE_SPEED);
    if (IsKeyDown(KEY_E)) CameraRoll(camera, -CAMERA_MOVE_SPEED);

    CameraYaw(camera, mousePositionDelta.x * CAMERA_MOUSE_MOVE_SENSITIVITY);
    CameraPitch(camera, mousePositionDelta.y * CAMERA_MOUSE_MOVE_SENSITIVITY);

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
