/*******************************************************************************************
*
*   rcamera - Basic camera system for multiple camera modes
*
*   NOTE: Memory footprint of this library is aproximately 52 bytes (global variables)
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

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Types and Structures Definition
// NOTE: Below types are required for CAMERA_STANDALONE usage
//----------------------------------------------------------------------------------
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

#include <math.h>               // Required for: sinf(), cosf(), sqrtf()

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#ifndef PI
    #define PI 3.14159265358979323846
#endif
#ifndef DEG2RAD
    #define DEG2RAD (PI/180.0f)
#endif
#ifndef RAD2DEG
    #define RAD2DEG (180.0f/PI)
#endif

// Camera mouse movement sensitivity
#define CAMERA_MOUSE_MOVE_SENSITIVITY                   0.003f
#define CAMERA_MOUSE_SCROLL_SENSITIVITY                 1.5f

// FREE_CAMERA
#define CAMERA_FREE_MOUSE_SENSITIVITY                   0.01f
#define CAMERA_FREE_DISTANCE_MIN_CLAMP                  0.3f
#define CAMERA_FREE_DISTANCE_MAX_CLAMP                  120.0f
#define CAMERA_FREE_MIN_CLAMP                           85.0f
#define CAMERA_FREE_MAX_CLAMP                          -85.0f
#define CAMERA_FREE_SMOOTH_ZOOM_SENSITIVITY             0.05f
#define CAMERA_FREE_PANNING_DIVIDER                     5.1f

// ORBITAL_CAMERA
#define CAMERA_ORBITAL_SPEED                            0.01f       // Radians per frame

// FIRST_PERSON
//#define CAMERA_FIRST_PERSON_MOUSE_SENSITIVITY           0.003f
#define CAMERA_FIRST_PERSON_FOCUS_DISTANCE              25.0f
#define CAMERA_FIRST_PERSON_MIN_CLAMP                   89.0f
#define CAMERA_FIRST_PERSON_MAX_CLAMP                  -89.0f

#define CAMERA_FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER  8.0f
#define CAMERA_FIRST_PERSON_STEP_DIVIDER                30.0f
#define CAMERA_FIRST_PERSON_WAVING_DIVIDER              200.0f

// THIRD_PERSON
//#define CAMERA_THIRD_PERSON_MOUSE_SENSITIVITY           0.003f
#define CAMERA_THIRD_PERSON_DISTANCE_CLAMP              1.2f
#define CAMERA_THIRD_PERSON_MIN_CLAMP                   5.0f
#define CAMERA_THIRD_PERSON_MAX_CLAMP                  -85.0f
#define CAMERA_THIRD_PERSON_OFFSET                      (Vector3){ 0.4f, 0.0f, 0.0f }

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

// Camera global state context data [56 bytes]
typedef struct {
    unsigned int mode;              // Current camera mode
    float targetDistance;           // Camera distance from position to target
    float playerEyesPosition;       // Player eyes position from ground (in meters)
    Vector2 angle;                  // Camera angle in plane XZ
    Vector2 previousMousePosition;  // Previous mouse position

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
    .previousMousePosition = { 0 },
    .moveControl = { 'W', 'S', 'D', 'A', 'E', 'Q' },
    .smoothZoomControl = 341,       // raylib: KEY_LEFT_CONTROL
    .altControl = 342,              // raylib: KEY_LEFT_ALT
    .panControl = 2                 // raylib: MOUSE_BUTTON_MIDDLE
};

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
#if defined(CAMERA_STANDALONE)
// NOTE: Camera controls depend on some raylib input functions
static void EnableCursor() {}       // Unlock cursor
static void DisableCursor() {}      // Lock cursor

static int IsKeyDown(int key) { return 0; }

static int IsMouseButtonDown(int button) { return 0;}
static float GetMouseWheelMove() { return 0.0f; }
static Vector2 GetMousePosition() { return (Vector2){ 0.0f, 0.0f }; }
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Select camera mode (multiple camera modes available)
void SetCameraMode(Camera camera, int mode)
{
    Vector3 v1 = camera.position;
    Vector3 v2 = camera.target;

    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;
    float dz = v2.z - v1.z;

    CAMERA.targetDistance = sqrtf(dx*dx + dy*dy + dz*dz);   // Distance to target

    // Camera angle calculation
    CAMERA.angle.x = atan2f(dx, dz);                        // Camera angle in plane XZ (0 aligned with Z, move positive CCW)
    CAMERA.angle.y = atan2f(dy, sqrtf(dx*dx + dz*dz));      // Camera angle in plane XY (0 aligned with X, move positive CW)

    CAMERA.playerEyesPosition = camera.position.y;          // Init player eyes position to camera Y position

    CAMERA.previousMousePosition = GetMousePosition();      // Init mouse position

    // Lock cursor for first person and third person cameras
    if ((mode == CAMERA_FIRST_PERSON) || (mode == CAMERA_THIRD_PERSON)) DisableCursor();
    else EnableCursor();

    CAMERA.mode = mode;
}

// Update camera depending on selected mode
// NOTE: Camera controls depend on some raylib functions:
//       System: EnableCursor(), DisableCursor()
//       Mouse: IsMouseButtonDown(), GetMousePosition(), GetMouseWheelMove()
//       Keys:  IsKeyDown()
void UpdateCamera(Camera *camera)
{
    static int swingCounter = 0;    // Used for 1st person swinging movement

    // TODO: Compute CAMERA.targetDistance and CAMERA.angle here (?)

    // Mouse movement detection
    Vector2 mousePositionDelta = { 0.0f, 0.0f };
    Vector2 mousePosition = GetMousePosition();
    float mouseWheelMove = GetMouseWheelMove();

    // Keys input detection
    // TODO: Input detection is raylib-dependant, it could be moved outside the module
    bool keyPan = IsMouseButtonDown(CAMERA.panControl);
    bool keyAlt = IsKeyDown(CAMERA.altControl);
    bool szoomKey = IsKeyDown(CAMERA.smoothZoomControl);
    bool direction[6] = { IsKeyDown(CAMERA.moveControl[MOVE_FRONT]),
                          IsKeyDown(CAMERA.moveControl[MOVE_BACK]),
                          IsKeyDown(CAMERA.moveControl[MOVE_RIGHT]),
                          IsKeyDown(CAMERA.moveControl[MOVE_LEFT]),
                          IsKeyDown(CAMERA.moveControl[MOVE_UP]),
                          IsKeyDown(CAMERA.moveControl[MOVE_DOWN]) };

    if (CAMERA.mode != CAMERA_CUSTOM)
    {
        mousePositionDelta.x = mousePosition.x - CAMERA.previousMousePosition.x;
        mousePositionDelta.y = mousePosition.y - CAMERA.previousMousePosition.y;

        CAMERA.previousMousePosition = mousePosition;
    }

    // Support for multiple automatic camera modes
    // NOTE: In case of CAMERA_CUSTOM nothing happens here, user must update it manually
    switch (CAMERA.mode)
    {
        case CAMERA_FREE:           // Camera free controls, using standard 3d-content-creation scheme
        {
            // Camera zoom
            if ((CAMERA.targetDistance < CAMERA_FREE_DISTANCE_MAX_CLAMP) && (mouseWheelMove < 0))
            {
                CAMERA.targetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);
                if (CAMERA.targetDistance > CAMERA_FREE_DISTANCE_MAX_CLAMP) CAMERA.targetDistance = CAMERA_FREE_DISTANCE_MAX_CLAMP;
            }

            // Camera looking down
            else if ((camera->position.y > camera->target.y) && (CAMERA.targetDistance == CAMERA_FREE_DISTANCE_MAX_CLAMP) && (mouseWheelMove < 0))
            {
                camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
                camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
                camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
            }
            else if ((camera->position.y > camera->target.y) && (camera->target.y >= 0))
            {
                camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
                camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
                camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;

                // if (camera->target.y < 0) camera->target.y = -0.001;
            }
            else if ((camera->position.y > camera->target.y) && (camera->target.y < 0) && (mouseWheelMove > 0))
            {
                CAMERA.targetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);
                if (CAMERA.targetDistance < CAMERA_FREE_DISTANCE_MIN_CLAMP) CAMERA.targetDistance = CAMERA_FREE_DISTANCE_MIN_CLAMP;
            }
            // Camera looking up
            else if ((camera->position.y < camera->target.y) && (CAMERA.targetDistance == CAMERA_FREE_DISTANCE_MAX_CLAMP) && (mouseWheelMove < 0))
            {
                camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
                camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
                camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
            }
            else if ((camera->position.y < camera->target.y) && (camera->target.y <= 0))
            {
                camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
                camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;
                camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/CAMERA.targetDistance;

                // if (camera->target.y > 0) camera->target.y = 0.001;
            }
            else if ((camera->position.y < camera->target.y) && (camera->target.y > 0) && (mouseWheelMove > 0))
            {
                CAMERA.targetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);
                if (CAMERA.targetDistance < CAMERA_FREE_DISTANCE_MIN_CLAMP) CAMERA.targetDistance = CAMERA_FREE_DISTANCE_MIN_CLAMP;
            }

            // Input keys checks
            if (keyPan)
            {
                if (keyAlt)     // Alternative key behaviour
                {
                    if (szoomKey)
                    {
                        // Camera smooth zoom
                        CAMERA.targetDistance += (mousePositionDelta.y*CAMERA_FREE_SMOOTH_ZOOM_SENSITIVITY);
                    }
                    else
                    {
                        // Camera rotation
                        CAMERA.angle.x += mousePositionDelta.x*-CAMERA_FREE_MOUSE_SENSITIVITY;
                        CAMERA.angle.y += mousePositionDelta.y*-CAMERA_FREE_MOUSE_SENSITIVITY;

                        // Angle clamp
                        if (CAMERA.angle.y > CAMERA_FREE_MIN_CLAMP*DEG2RAD) CAMERA.angle.y = CAMERA_FREE_MIN_CLAMP*DEG2RAD;
                        else if (CAMERA.angle.y < CAMERA_FREE_MAX_CLAMP*DEG2RAD) CAMERA.angle.y = CAMERA_FREE_MAX_CLAMP*DEG2RAD;
                    }
                }
                else
                {
                    // Camera panning
                    camera->target.x += ((mousePositionDelta.x*CAMERA_FREE_MOUSE_SENSITIVITY)*cosf(CAMERA.angle.x) + (mousePositionDelta.y*-CAMERA_FREE_MOUSE_SENSITIVITY)*sinf(CAMERA.angle.x)*sinf(CAMERA.angle.y))*(CAMERA.targetDistance/CAMERA_FREE_PANNING_DIVIDER);
                    camera->target.y += ((mousePositionDelta.y*CAMERA_FREE_MOUSE_SENSITIVITY)*cosf(CAMERA.angle.y))*(CAMERA.targetDistance/CAMERA_FREE_PANNING_DIVIDER);
                    camera->target.z += ((mousePositionDelta.x*-CAMERA_FREE_MOUSE_SENSITIVITY)*sinf(CAMERA.angle.x) + (mousePositionDelta.y*-CAMERA_FREE_MOUSE_SENSITIVITY)*cosf(CAMERA.angle.x)*sinf(CAMERA.angle.y))*(CAMERA.targetDistance/CAMERA_FREE_PANNING_DIVIDER);
                }
            }

            // Update camera position with changes
            camera->position.x = -sinf(CAMERA.angle.x)*CAMERA.targetDistance*cosf(CAMERA.angle.y) + camera->target.x;
            camera->position.y = -sinf(CAMERA.angle.y)*CAMERA.targetDistance + camera->target.y;
            camera->position.z = -cosf(CAMERA.angle.x)*CAMERA.targetDistance*cosf(CAMERA.angle.y) + camera->target.z;

        } break;
        case CAMERA_ORBITAL:        // Camera just orbits around target, only zoom allowed
        {
            CAMERA.angle.x += CAMERA_ORBITAL_SPEED;      // Camera orbit angle
            CAMERA.targetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);   // Camera zoom

            // Camera distance clamp
            if (CAMERA.targetDistance < CAMERA_THIRD_PERSON_DISTANCE_CLAMP) CAMERA.targetDistance = CAMERA_THIRD_PERSON_DISTANCE_CLAMP;

            // Update camera position with changes
            camera->position.x = sinf(CAMERA.angle.x)*CAMERA.targetDistance*cosf(CAMERA.angle.y) + camera->target.x;
            camera->position.y = ((CAMERA.angle.y <= 0.0f)? 1 : -1)*sinf(CAMERA.angle.y)*CAMERA.targetDistance*sinf(CAMERA.angle.y) + camera->target.y;
            camera->position.z = cosf(CAMERA.angle.x)*CAMERA.targetDistance*cosf(CAMERA.angle.y) + camera->target.z;

        } break;
        case CAMERA_FIRST_PERSON:   // Camera moves as in a first-person game, controls are configurable
        {
            camera->position.x += (sinf(CAMERA.angle.x)*direction[MOVE_BACK] -
                                   sinf(CAMERA.angle.x)*direction[MOVE_FRONT] -
                                   cosf(CAMERA.angle.x)*direction[MOVE_LEFT] +
                                   cosf(CAMERA.angle.x)*direction[MOVE_RIGHT])/PLAYER_MOVEMENT_SENSITIVITY;

            camera->position.y += (sinf(CAMERA.angle.y)*direction[MOVE_FRONT] -
                                   sinf(CAMERA.angle.y)*direction[MOVE_BACK] +
                                   1.0f*direction[MOVE_UP] - 1.0f*direction[MOVE_DOWN])/PLAYER_MOVEMENT_SENSITIVITY;

            camera->position.z += (cosf(CAMERA.angle.x)*direction[MOVE_BACK] -
                                   cosf(CAMERA.angle.x)*direction[MOVE_FRONT] +
                                   sinf(CAMERA.angle.x)*direction[MOVE_LEFT] -
                                   sinf(CAMERA.angle.x)*direction[MOVE_RIGHT])/PLAYER_MOVEMENT_SENSITIVITY;

            // Camera orientation calculation
            CAMERA.angle.x += (mousePositionDelta.x*-CAMERA_MOUSE_MOVE_SENSITIVITY);
            CAMERA.angle.y += (mousePositionDelta.y*-CAMERA_MOUSE_MOVE_SENSITIVITY);

            // Angle clamp
            if (CAMERA.angle.y > CAMERA_FIRST_PERSON_MIN_CLAMP*DEG2RAD) CAMERA.angle.y = CAMERA_FIRST_PERSON_MIN_CLAMP*DEG2RAD;
            else if (CAMERA.angle.y < CAMERA_FIRST_PERSON_MAX_CLAMP*DEG2RAD) CAMERA.angle.y = CAMERA_FIRST_PERSON_MAX_CLAMP*DEG2RAD;

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
            float cosy = cosf(-(PI*2 - CAMERA.angle.x));
            float siny = sinf(-(PI*2 - CAMERA.angle.x));
            float cosx = cosf(-(PI*2 - CAMERA.angle.y));
            float sinx = sinf(-(PI*2 - CAMERA.angle.y));

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

            // If movement detected (some key pressed), increase swinging
            for (int i = 0; i < 6; i++) if (direction[i]) { swingCounter++; break; }

            // Camera position update
            // NOTE: On CAMERA_FIRST_PERSON player Y-movement is limited to player 'eyes position'
            camera->position.y = CAMERA.playerEyesPosition - sinf(swingCounter/CAMERA_FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER)/CAMERA_FIRST_PERSON_STEP_DIVIDER;

            camera->up.x = sinf(swingCounter/(CAMERA_FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER*2))/CAMERA_FIRST_PERSON_WAVING_DIVIDER;
            camera->up.z = -sinf(swingCounter/(CAMERA_FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER*2))/CAMERA_FIRST_PERSON_WAVING_DIVIDER;

        } break;
        case CAMERA_THIRD_PERSON:   // Camera moves as in a third-person game, following target at a distance, controls are configurable
        {
            camera->position.x += (sinf(CAMERA.angle.x)*direction[MOVE_BACK] -
                                   sinf(CAMERA.angle.x)*direction[MOVE_FRONT] -
                                   cosf(CAMERA.angle.x)*direction[MOVE_LEFT] +
                                   cosf(CAMERA.angle.x)*direction[MOVE_RIGHT])/PLAYER_MOVEMENT_SENSITIVITY;

            camera->position.y += (sinf(CAMERA.angle.y)*direction[MOVE_FRONT] -
                                   sinf(CAMERA.angle.y)*direction[MOVE_BACK] +
                                   1.0f*direction[MOVE_UP] - 1.0f*direction[MOVE_DOWN])/PLAYER_MOVEMENT_SENSITIVITY;

            camera->position.z += (cosf(CAMERA.angle.x)*direction[MOVE_BACK] -
                                   cosf(CAMERA.angle.x)*direction[MOVE_FRONT] +
                                   sinf(CAMERA.angle.x)*direction[MOVE_LEFT] -
                                   sinf(CAMERA.angle.x)*direction[MOVE_RIGHT])/PLAYER_MOVEMENT_SENSITIVITY;

            // Camera orientation calculation
            CAMERA.angle.x += (mousePositionDelta.x*-CAMERA_MOUSE_MOVE_SENSITIVITY);
            CAMERA.angle.y += (mousePositionDelta.y*-CAMERA_MOUSE_MOVE_SENSITIVITY);

            // Angle clamp
            if (CAMERA.angle.y > CAMERA_THIRD_PERSON_MIN_CLAMP*DEG2RAD) CAMERA.angle.y = CAMERA_THIRD_PERSON_MIN_CLAMP*DEG2RAD;
            else if (CAMERA.angle.y < CAMERA_THIRD_PERSON_MAX_CLAMP*DEG2RAD) CAMERA.angle.y = CAMERA_THIRD_PERSON_MAX_CLAMP*DEG2RAD;

            // Camera zoom
            CAMERA.targetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);

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

#endif // CAMERA_IMPLEMENTATION
