/*******************************************************************************************
*
*   raylib.camera - Camera system with multiple modes support
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
*   Copyright (c) 2015-2019 Ramon Santamaria (@raysan5)
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
    } CameraType;
#endif

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
#if defined(CAMERA_STANDALONE)
void SetCameraMode(Camera camera, int mode);                // Set camera mode (multiple camera modes available)
void UpdateCamera(Camera *camera);                          // Update camera position for selected mode

void SetCameraPanControl(int panKey);                       // Set camera pan key to combine with mouse movement (free camera)
void SetCameraAltControl(int altKey);                       // Set camera alt key to combine with mouse movement (free camera)
void SetCameraSmoothZoomControl(int szoomKey);              // Set camera smooth zoom key to combine with mouse (free camera)
void SetCameraMoveControls(int frontKey, int backKey, 
                           int rightKey, int leftKey, 
                           int upKey, int downKey);         // Set camera move controls (1st person and 3rd person cameras)
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

#include <math.h>               // Required for: sqrt(), sinf(), cosf()

#ifndef PI
    #define PI 3.14159265358979323846
#endif
#ifndef DEG2RAD
    #define DEG2RAD (PI/180.0f)
#endif
#ifndef RAD2DEG
    #define RAD2DEG (180.0f/PI)
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
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
#define CAMERA_FIRST_PERSON_MIN_CLAMP                   85.0f
#define CAMERA_FIRST_PERSON_MAX_CLAMP                  -85.0f

#define CAMERA_FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER  5.0f
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

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static Vector2 cameraAngle = { 0.0f, 0.0f };          // Camera angle in plane XZ
static float cameraTargetDistance = 0.0f;             // Camera distance from position to target 
static float playerEyesPosition = 1.85f;              // Default player eyes position from ground (in meters) 

static int cameraMoveControl[6]  = { 'W', 'S', 'D', 'A', 'E', 'Q' };
static int cameraPanControlKey = 2;                   // raylib: MOUSE_MIDDLE_BUTTON
static int cameraAltControlKey = 342;                 // raylib: KEY_LEFT_ALT
static int cameraSmoothZoomControlKey = 341;          // raylib: KEY_LEFT_CONTROL

static int cameraMode = CAMERA_CUSTOM;                // Current camera mode

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
#if defined(CAMERA_STANDALONE)
// NOTE: Camera controls depend on some raylib input functions
// TODO: Set your own input functions (used in UpdateCamera())
static void EnableCursor() {}       // Unlock cursor
static void DisableCursor() {}      // Lock cursor

static int IsKeyDown(int key) { return 0; }

static int IsMouseButtonDown(int button) { return 0;}
static int GetMouseWheelMove() { return 0; }
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
    
    cameraTargetDistance = sqrtf(dx*dx + dy*dy + dz*dz);
    
    Vector2 distance = { 0.0f, 0.0f };
    distance.x = sqrtf(dx*dx + dz*dz);
    distance.y = sqrtf(dx*dx + dy*dy);
    
    // Camera angle calculation
    cameraAngle.x = asinf( (float)fabs(dx)/distance.x);  // Camera angle in plane XZ (0 aligned with Z, move positive CCW)
    cameraAngle.y = -asinf( (float)fabs(dy)/distance.y); // Camera angle in plane XY (0 aligned with X, move positive CW)
    
    // NOTE: Just testing what cameraAngle means
    //cameraAngle.x = 0.0f*DEG2RAD;       // Camera angle in plane XZ (0 aligned with Z, move positive CCW)
    //cameraAngle.y = -60.0f*DEG2RAD;     // Camera angle in plane XY (0 aligned with X, move positive CW)
    
    playerEyesPosition = camera.position.y;

    // Lock cursor for first person and third person cameras
    if ((mode == CAMERA_FIRST_PERSON) || (mode == CAMERA_THIRD_PERSON)) DisableCursor();
    else EnableCursor();

    cameraMode = mode;
}

// Update camera depending on selected mode
// NOTE: Camera controls depend on some raylib functions:
//       System: EnableCursor(), DisableCursor()
//       Mouse: IsMouseButtonDown(), GetMousePosition(), GetMouseWheelMove()
//       Keys:  IsKeyDown()
// TODO: Port to quaternion-based camera
void UpdateCamera(Camera *camera)
{
    static int swingCounter = 0;    // Used for 1st person swinging movement
    static Vector2 previousMousePosition = { 0.0f, 0.0f };

    // TODO: Compute cameraTargetDistance and cameraAngle here
    
    // Mouse movement detection
    Vector2 mousePositionDelta = { 0.0f, 0.0f };
    Vector2 mousePosition = GetMousePosition();
    int mouseWheelMove = GetMouseWheelMove();
    
    // Keys input detection
    bool panKey = IsMouseButtonDown(cameraPanControlKey);
    bool altKey = IsKeyDown(cameraAltControlKey);
    bool szoomKey = IsKeyDown(cameraSmoothZoomControlKey);
    
    bool direction[6] = { IsKeyDown(cameraMoveControl[MOVE_FRONT]),
                          IsKeyDown(cameraMoveControl[MOVE_BACK]),
                          IsKeyDown(cameraMoveControl[MOVE_RIGHT]),
                          IsKeyDown(cameraMoveControl[MOVE_LEFT]),
                          IsKeyDown(cameraMoveControl[MOVE_UP]),
                          IsKeyDown(cameraMoveControl[MOVE_DOWN]) };
                          
    // TODO: Consider touch inputs for camera

    if (cameraMode != CAMERA_CUSTOM)
    {
        mousePositionDelta.x = mousePosition.x - previousMousePosition.x;
        mousePositionDelta.y = mousePosition.y - previousMousePosition.y;

        previousMousePosition = mousePosition;
    }

    // Support for multiple automatic camera modes
    switch (cameraMode)
    {
        case CAMERA_FREE:
        {
            // Camera zoom
            if ((cameraTargetDistance < CAMERA_FREE_DISTANCE_MAX_CLAMP) && (mouseWheelMove < 0))
            {
                cameraTargetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);

                if (cameraTargetDistance > CAMERA_FREE_DISTANCE_MAX_CLAMP) cameraTargetDistance = CAMERA_FREE_DISTANCE_MAX_CLAMP;
            }
            // Camera looking down
			// TODO: Review, weird comparisson of cameraTargetDistance == 120.0f?
            else if ((camera->position.y > camera->target.y) && (cameraTargetDistance == CAMERA_FREE_DISTANCE_MAX_CLAMP) && (mouseWheelMove < 0))
            {
                camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/cameraTargetDistance;
                camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/cameraTargetDistance;
                camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/cameraTargetDistance;
            }
            else if ((camera->position.y > camera->target.y) && (camera->target.y >= 0))
            {
                camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/cameraTargetDistance;
                camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/cameraTargetDistance;
                camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/cameraTargetDistance;

                // if (camera->target.y < 0) camera->target.y = -0.001;
            }
            else if ((camera->position.y > camera->target.y) && (camera->target.y < 0) && (mouseWheelMove > 0))
            {
                cameraTargetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);
                if (cameraTargetDistance < CAMERA_FREE_DISTANCE_MIN_CLAMP) cameraTargetDistance = CAMERA_FREE_DISTANCE_MIN_CLAMP;
            }
            // Camera looking up
			// TODO: Review, weird comparisson of cameraTargetDistance == 120.0f?
            else if ((camera->position.y < camera->target.y) && (cameraTargetDistance == CAMERA_FREE_DISTANCE_MAX_CLAMP) && (mouseWheelMove < 0))
            {
                camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/cameraTargetDistance;
                camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/cameraTargetDistance;
                camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/cameraTargetDistance;
            }
            else if ((camera->position.y < camera->target.y) && (camera->target.y <= 0))
            {
                camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/cameraTargetDistance;
                camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/cameraTargetDistance;
                camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/cameraTargetDistance;

                // if (camera->target.y > 0) camera->target.y = 0.001;
            }
            else if ((camera->position.y < camera->target.y) && (camera->target.y > 0) && (mouseWheelMove > 0))
            {
                cameraTargetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);
                if (cameraTargetDistance < CAMERA_FREE_DISTANCE_MIN_CLAMP) cameraTargetDistance = CAMERA_FREE_DISTANCE_MIN_CLAMP;
            }

            // Input keys checks
            if (panKey)
            {
                if (altKey)     // Alternative key behaviour
                {
                    if (szoomKey)
                    {
                        // Camera smooth zoom
                        cameraTargetDistance += (mousePositionDelta.y*CAMERA_FREE_SMOOTH_ZOOM_SENSITIVITY);
                    }
                    else
                    {
                        // Camera rotation
                        cameraAngle.x += mousePositionDelta.x*-CAMERA_FREE_MOUSE_SENSITIVITY;
                        cameraAngle.y += mousePositionDelta.y*-CAMERA_FREE_MOUSE_SENSITIVITY;

                        // Angle clamp
                        if (cameraAngle.y > CAMERA_FREE_MIN_CLAMP*DEG2RAD) cameraAngle.y = CAMERA_FREE_MIN_CLAMP*DEG2RAD;
                        else if (cameraAngle.y < CAMERA_FREE_MAX_CLAMP*DEG2RAD) cameraAngle.y = CAMERA_FREE_MAX_CLAMP*DEG2RAD;
                    }
                }
                else
                {
                    // Camera panning
                    camera->target.x += ((mousePositionDelta.x*-CAMERA_FREE_MOUSE_SENSITIVITY)*cosf(cameraAngle.x) + (mousePositionDelta.y*CAMERA_FREE_MOUSE_SENSITIVITY)*sinf(cameraAngle.x)*sinf(cameraAngle.y))*(cameraTargetDistance/CAMERA_FREE_PANNING_DIVIDER);
                    camera->target.y += ((mousePositionDelta.y*CAMERA_FREE_MOUSE_SENSITIVITY)*cosf(cameraAngle.y))*(cameraTargetDistance/CAMERA_FREE_PANNING_DIVIDER);
                    camera->target.z += ((mousePositionDelta.x*CAMERA_FREE_MOUSE_SENSITIVITY)*sinf(cameraAngle.x) + (mousePositionDelta.y*CAMERA_FREE_MOUSE_SENSITIVITY)*cosf(cameraAngle.x)*sinf(cameraAngle.y))*(cameraTargetDistance/CAMERA_FREE_PANNING_DIVIDER);
                }
            }
            
            // Update camera position with changes
            camera->position.x = sinf(cameraAngle.x)*cameraTargetDistance*cosf(cameraAngle.y) + camera->target.x;
            camera->position.y = ((cameraAngle.y <= 0.0f)? 1 : -1)*sinf(cameraAngle.y)*cameraTargetDistance*sinf(cameraAngle.y) + camera->target.y;
            camera->position.z = cosf(cameraAngle.x)*cameraTargetDistance*cosf(cameraAngle.y) + camera->target.z;

        } break;
        case CAMERA_ORBITAL:
        {
            cameraAngle.x += CAMERA_ORBITAL_SPEED;      // Camera orbit angle
            cameraTargetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);   // Camera zoom
            
            // Camera distance clamp
            if (cameraTargetDistance < CAMERA_THIRD_PERSON_DISTANCE_CLAMP) cameraTargetDistance = CAMERA_THIRD_PERSON_DISTANCE_CLAMP;
            
            // Update camera position with changes
            camera->position.x = sinf(cameraAngle.x)*cameraTargetDistance*cosf(cameraAngle.y) + camera->target.x;
            camera->position.y = ((cameraAngle.y <= 0.0f)? 1 : -1)*sinf(cameraAngle.y)*cameraTargetDistance*sinf(cameraAngle.y) + camera->target.y;
            camera->position.z = cosf(cameraAngle.x)*cameraTargetDistance*cosf(cameraAngle.y) + camera->target.z;
   
        } break;
        case CAMERA_FIRST_PERSON:
        case CAMERA_THIRD_PERSON:
        {
            camera->position.x += (sinf(cameraAngle.x)*direction[MOVE_BACK] -
                                   sinf(cameraAngle.x)*direction[MOVE_FRONT] -
                                   cosf(cameraAngle.x)*direction[MOVE_LEFT] +
                                   cosf(cameraAngle.x)*direction[MOVE_RIGHT])/PLAYER_MOVEMENT_SENSITIVITY;
                                   
            camera->position.y += (sinf(cameraAngle.y)*direction[MOVE_FRONT] -
                                   sinf(cameraAngle.y)*direction[MOVE_BACK] +
                                   1.0f*direction[MOVE_UP] - 1.0f*direction[MOVE_DOWN])/PLAYER_MOVEMENT_SENSITIVITY;
                                   
            camera->position.z += (cosf(cameraAngle.x)*direction[MOVE_BACK] -
                                   cosf(cameraAngle.x)*direction[MOVE_FRONT] +
                                   sinf(cameraAngle.x)*direction[MOVE_LEFT] -
                                   sinf(cameraAngle.x)*direction[MOVE_RIGHT])/PLAYER_MOVEMENT_SENSITIVITY;

            bool isMoving = false;  // Required for swinging

            for (int i = 0; i < 6; i++) if (direction[i]) { isMoving = true; break; }
            
            // Camera orientation calculation
            cameraAngle.x += (mousePositionDelta.x*-CAMERA_MOUSE_MOVE_SENSITIVITY);
            cameraAngle.y += (mousePositionDelta.y*-CAMERA_MOUSE_MOVE_SENSITIVITY);
            
            // Angle clamp
            if (cameraAngle.y > CAMERA_FIRST_PERSON_MIN_CLAMP*DEG2RAD) cameraAngle.y = CAMERA_FIRST_PERSON_MIN_CLAMP*DEG2RAD;
            else if (cameraAngle.y < CAMERA_FIRST_PERSON_MAX_CLAMP*DEG2RAD) cameraAngle.y = CAMERA_FIRST_PERSON_MAX_CLAMP*DEG2RAD;

            // Camera is always looking at player
            camera->target.x = camera->position.x - sinf(cameraAngle.x)*CAMERA_FIRST_PERSON_FOCUS_DISTANCE;
            camera->target.y = camera->position.y + sinf(cameraAngle.y)*CAMERA_FIRST_PERSON_FOCUS_DISTANCE;
            camera->target.z = camera->position.z - cosf(cameraAngle.x)*CAMERA_FIRST_PERSON_FOCUS_DISTANCE;
            
            if (cameraMode == CAMERA_FIRST_PERSON)
            {
                if (isMoving) swingCounter++;

                // Camera position update
                // NOTE: On CAMERA_FIRST_PERSON player Y-movement is limited to player 'eyes position'
                camera->position.y = playerEyesPosition - sinf(swingCounter/CAMERA_FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER)/CAMERA_FIRST_PERSON_STEP_DIVIDER;

                camera->up.x = sinf(swingCounter/(CAMERA_FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER*2))/CAMERA_FIRST_PERSON_WAVING_DIVIDER;
                camera->up.z = -sinf(swingCounter/(CAMERA_FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER*2))/CAMERA_FIRST_PERSON_WAVING_DIVIDER;
            }
            else if (cameraMode == CAMERA_THIRD_PERSON)
            {
                // Camera zoom
                cameraTargetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);
                if (cameraTargetDistance < CAMERA_THIRD_PERSON_DISTANCE_CLAMP) cameraTargetDistance = CAMERA_THIRD_PERSON_DISTANCE_CLAMP;
            }
            
        } break;
        default: break;
    }
}

// Set camera pan key to combine with mouse movement (free camera)
void SetCameraPanControl(int panKey) { cameraPanControlKey = panKey; }

// Set camera alt key to combine with mouse movement (free camera)
void SetCameraAltControl(int altKey) { cameraAltControlKey = altKey; }

// Set camera smooth zoom key to combine with mouse (free camera)
void SetCameraSmoothZoomControl(int szoomKey) { cameraSmoothZoomControlKey = szoomKey; }

// Set camera move controls (1st person and 3rd person cameras)
void SetCameraMoveControls(int frontKey, int backKey, int rightKey, int leftKey, int upKey, int downKey)
{
    cameraMoveControl[MOVE_FRONT] = frontKey;
    cameraMoveControl[MOVE_BACK] = backKey;
    cameraMoveControl[MOVE_RIGHT] = rightKey;
    cameraMoveControl[MOVE_LEFT] = leftKey;
    cameraMoveControl[MOVE_UP] = upKey;
    cameraMoveControl[MOVE_DOWN] = downKey;
}

#endif // CAMERA_IMPLEMENTATION
