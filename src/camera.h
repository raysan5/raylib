/*******************************************************************************************
*
*   raylib Camera System - Camera Modes Setup and Control Functions
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
*   NOTE: Memory footprint of this library is aproximately 52 bytes (global variables)
*
*   Initial design by Marc Palau (2014)
*   Reviewed by Ramon Santamaria (2015-2016)
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
    // Camera modes
    typedef enum { 
        CAMERA_CUSTOM = 0, 
        CAMERA_FREE, 
        CAMERA_ORBITAL, 
        CAMERA_FIRST_PERSON, 
        CAMERA_THIRD_PERSON 
    } CameraMode;

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
        float fovy;
    } Camera;
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
void SetCameraSmoothZoomControl(int szKey);                 // Set camera smooth zoom key to combine with mouse (free camera)
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

#include <math.h>               // Required for: sqrt(), sin(), cos()

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
typedef enum { MOVE_FRONT = 0, MOVE_BACK, MOVE_RIGHT, MOVE_LEFT, MOVE_UP, MOVE_DOWN } CameraMove;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static Vector2 cameraAngle = { 0.0f, 0.0f };          // TODO: Remove! Compute it in UpdateCamera()
static float cameraTargetDistance = 0.0f;             // TODO: Remove! Compute it in UpdateCamera()
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
static Vector2 GetMousePosition() { return (Vector2){ 0.0f, 0.0f }; }
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
void SetCameraMode(Camera camera, int mode)
{
    // TODO: cameraTargetDistance and cameraAngle should be 
    // calculated using camera parameters on UpdateCamera()

    Vector3 v1 = camera.position;
    Vector3 v2 = camera.target;
    
    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;
    float dz = v2.z - v1.z;
    
    cameraTargetDistance = sqrt(dx*dx + dy*dy + dz*dz);
    
    Vector2 distance = { 0.0f, 0.0f };
    distance.x = sqrt(dx*dx + dz*dz);
    distance.y = sqrt(dx*dx + dy*dy);
    
    // Camera angle calculation
    cameraAngle.x = asin(fabs(dx)/distance.x);  // Camera angle in plane XZ (0 aligned with Z, move positive CCW)
    cameraAngle.y = -asin(fabs(dy)/distance.y); // Camera angle in plane XY (0 aligned with X, move positive CW)
    
    // NOTE: Just testing what cameraAngle means
    //cameraAngle.x = 0.0f*DEG2RAD;      // Camera angle in plane XZ (0 aligned with Z, move positive CCW)
    //cameraAngle.y = -60.0f*DEG2RAD;     // Camera angle in plane XY (0 aligned with X, move positive CW)
    
    playerEyesPosition = camera.position.y;

    cameraMode = mode;
}

// Update camera depending on selected mode
// NOTE: Camera controls depend on some raylib functions:
//       Mouse: GetMousePosition(), SetMousePosition(), IsMouseButtonDown(), GetMouseWheelMove()
//       System: GetScreenWidth(), GetScreenHeight(), ShowCursor(), HideCursor()
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
        // Get screen size
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        
        if ((cameraMode == CAMERA_FIRST_PERSON) || 
            (cameraMode == CAMERA_THIRD_PERSON))
        {
            HideCursor();

            if (mousePosition.x < screenHeight/3) SetMousePosition((Vector2){ screenWidth - screenHeight/3, mousePosition.y });
            else if (mousePosition.y < screenHeight/3) SetMousePosition((Vector2){ mousePosition.x, screenHeight - screenHeight/3 });
            else if (mousePosition.x > (screenWidth - screenHeight/3)) SetMousePosition((Vector2){ screenHeight/3, mousePosition.y });
            else if (mousePosition.y > (screenHeight - screenHeight/3)) SetMousePosition((Vector2){ mousePosition.x, screenHeight/3 });
            else
            {
                mousePositionDelta.x = mousePosition.x - previousMousePosition.x;
                mousePositionDelta.y = mousePosition.y - previousMousePosition.y;
            }
        }
        else    // CAMERA_FREE, CAMERA_ORBITAL
        {
            ShowCursor();

            mousePositionDelta.x = mousePosition.x - previousMousePosition.x;
            mousePositionDelta.y = mousePosition.y - previousMousePosition.y;
        }

        // NOTE: We GetMousePosition() again because it can be modified by a previous SetMousePosition() call
        // If using directly mousePosition variable we have problems on CAMERA_FIRST_PERSON and CAMERA_THIRD_PERSON
        previousMousePosition = GetMousePosition();
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
                    camera->target.x += ((mousePositionDelta.x*-CAMERA_FREE_MOUSE_SENSITIVITY)*cos(cameraAngle.x) + (mousePositionDelta.y*CAMERA_FREE_MOUSE_SENSITIVITY)*sin(cameraAngle.x)*sin(cameraAngle.y))*(cameraTargetDistance/CAMERA_FREE_PANNING_DIVIDER);
                    camera->target.y += ((mousePositionDelta.y*CAMERA_FREE_MOUSE_SENSITIVITY)*cos(cameraAngle.y))*(cameraTargetDistance/CAMERA_FREE_PANNING_DIVIDER);
                    camera->target.z += ((mousePositionDelta.x*CAMERA_FREE_MOUSE_SENSITIVITY)*sin(cameraAngle.x) + (mousePositionDelta.y*CAMERA_FREE_MOUSE_SENSITIVITY)*cos(cameraAngle.x)*sin(cameraAngle.y))*(cameraTargetDistance/CAMERA_FREE_PANNING_DIVIDER);
                }
            }

        } break;
        case CAMERA_ORBITAL:
        {
            cameraAngle.x += CAMERA_ORBITAL_SPEED;      // Camera orbit angle
            cameraTargetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);   // Camera zoom
            
            // Camera distance clamp
            if (cameraTargetDistance < CAMERA_THIRD_PERSON_DISTANCE_CLAMP) cameraTargetDistance = CAMERA_THIRD_PERSON_DISTANCE_CLAMP;
            
        } break;
        case CAMERA_FIRST_PERSON:
        case CAMERA_THIRD_PERSON:
        {
            camera->position.x += (sin(cameraAngle.x)*direction[MOVE_BACK] -
                                   sin(cameraAngle.x)*direction[MOVE_FRONT] -
                                   cos(cameraAngle.x)*direction[MOVE_LEFT] +
                                   cos(cameraAngle.x)*direction[MOVE_RIGHT])/PLAYER_MOVEMENT_SENSITIVITY;
                                   
            camera->position.y += (sin(cameraAngle.y)*direction[MOVE_FRONT] -
                                   sin(cameraAngle.y)*direction[MOVE_BACK] +
                                   1.0f*direction[MOVE_UP] - 1.0f*direction[MOVE_DOWN])/PLAYER_MOVEMENT_SENSITIVITY;
                                   
            camera->position.z += (cos(cameraAngle.x)*direction[MOVE_BACK] -
                                   cos(cameraAngle.x)*direction[MOVE_FRONT] +
                                   sin(cameraAngle.x)*direction[MOVE_LEFT] -
                                   sin(cameraAngle.x)*direction[MOVE_RIGHT])/PLAYER_MOVEMENT_SENSITIVITY;

            bool isMoving = false;  // Required for swinging

            for (int i = 0; i < 6; i++) if (direction[i]) { isMoving = true; break; }
            
            // Camera orientation calculation
            cameraAngle.x += (mousePositionDelta.x*-CAMERA_MOUSE_MOVE_SENSITIVITY);
            cameraAngle.y += (mousePositionDelta.y*-CAMERA_MOUSE_MOVE_SENSITIVITY);
            
            if (cameraMode == CAMERA_THIRD_PERSON)
            {
                // Angle clamp
                if (cameraAngle.y > CAMERA_THIRD_PERSON_MIN_CLAMP*DEG2RAD) cameraAngle.y = CAMERA_THIRD_PERSON_MIN_CLAMP*DEG2RAD;
                else if (cameraAngle.y < CAMERA_THIRD_PERSON_MAX_CLAMP*DEG2RAD) cameraAngle.y = CAMERA_THIRD_PERSON_MAX_CLAMP*DEG2RAD;

                // Camera zoom
                cameraTargetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);

                // Camera distance clamp
                if (cameraTargetDistance < CAMERA_THIRD_PERSON_DISTANCE_CLAMP) cameraTargetDistance = CAMERA_THIRD_PERSON_DISTANCE_CLAMP;

                // Camera is always looking at player
                camera->target.x = camera->position.x + CAMERA_THIRD_PERSON_OFFSET.x*cos(cameraAngle.x) + CAMERA_THIRD_PERSON_OFFSET.z*sin(cameraAngle.x);
                camera->target.y = camera->position.y + CAMERA_THIRD_PERSON_OFFSET.y;
                camera->target.z = camera->position.z + CAMERA_THIRD_PERSON_OFFSET.z*sin(cameraAngle.x) - CAMERA_THIRD_PERSON_OFFSET.x*sin(cameraAngle.x);
            }
            else    // CAMERA_FIRST_PERSON
            {
                // Angle clamp
                if (cameraAngle.y > CAMERA_FIRST_PERSON_MIN_CLAMP*DEG2RAD) cameraAngle.y = CAMERA_FIRST_PERSON_MIN_CLAMP*DEG2RAD;
                else if (cameraAngle.y < CAMERA_FIRST_PERSON_MAX_CLAMP*DEG2RAD) cameraAngle.y = CAMERA_FIRST_PERSON_MAX_CLAMP*DEG2RAD;

                // Camera is always looking at player
                camera->target.x = camera->position.x - sin(cameraAngle.x)*CAMERA_FIRST_PERSON_FOCUS_DISTANCE;
                camera->target.y = camera->position.y + sin(cameraAngle.y)*CAMERA_FIRST_PERSON_FOCUS_DISTANCE;
                camera->target.z = camera->position.z - cos(cameraAngle.x)*CAMERA_FIRST_PERSON_FOCUS_DISTANCE;
                
                if (isMoving) swingCounter++;

                // Camera position update
                // NOTE: On CAMERA_FIRST_PERSON player Y-movement is limited to player 'eyes position'
                camera->position.y = playerEyesPosition - sin(swingCounter/CAMERA_FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER)/CAMERA_FIRST_PERSON_STEP_DIVIDER;

                camera->up.x = sin(swingCounter/(CAMERA_FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER*2))/CAMERA_FIRST_PERSON_WAVING_DIVIDER;
                camera->up.z = -sin(swingCounter/(CAMERA_FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER*2))/CAMERA_FIRST_PERSON_WAVING_DIVIDER;
            }
        } break;
        default: break;
    }
    
    // Update camera position with changes
    if ((cameraMode == CAMERA_FREE) ||
        (cameraMode == CAMERA_ORBITAL) ||
        (cameraMode == CAMERA_THIRD_PERSON))
    {
        // TODO: It seems camera->position is not correctly updated or some rounding issue makes the camera move straight to camera->target...
        camera->position.x = sin(cameraAngle.x)*cameraTargetDistance*cos(cameraAngle.y) + camera->target.x;
        if (cameraAngle.y <= 0.0f) camera->position.y = sin(cameraAngle.y)*cameraTargetDistance*sin(cameraAngle.y) + camera->target.y;
        else camera->position.y = -sin(cameraAngle.y)*cameraTargetDistance*sin(cameraAngle.y) + camera->target.y;
        camera->position.z = cos(cameraAngle.x)*cameraTargetDistance*cos(cameraAngle.y) + camera->target.z;
    }
}

// Set camera pan key to combine with mouse movement (free camera)
void SetCameraPanControl(int panKey) { cameraPanControlKey = panKey; }

// Set camera alt key to combine with mouse movement (free camera)
void SetCameraAltControl(int altKey) { cameraAltControlKey = altKey; }

// Set camera smooth zoom key to combine with mouse (free camera)
void SetCameraSmoothZoomControl(int szKey) { cameraSmoothZoomControlKey = szKey; }

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
