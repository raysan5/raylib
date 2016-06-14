/**********************************************************************************************
*
*   raylib Gestures System - Gestures Detection and Usage Functions (Android and HTML5)
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

#ifndef GESTURES_H
#define GESTURES_H

#ifndef PI
    #define PI 3.14159265358979323846
#endif

#define DEG2RAD (PI / 180.0f)
#define RAD2DEG (180.0f / PI)

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Types and Structures Definition
// NOTE: Below types are required for GESTURES_STANDALONE usage
//----------------------------------------------------------------------------------
#ifndef __cplusplus
    // Boolean type
    typedef enum { false, true } bool;
#endif

// Vector2 type
typedef struct Vector2 {
    float x;
    float y;
} Vector2;

// Gestures type
// NOTE: It could be used as flags to enable only some gestures
typedef enum {
    GESTURE_NONE        = 1,
    GESTURE_TAP         = 2,
    GESTURE_DOUBLETAP   = 4,
    GESTURE_HOLD        = 8,
    GESTURE_DRAG        = 16,
    GESTURE_SWIPE_RIGHT = 32,
    GESTURE_SWIPE_LEFT  = 64,
    GESTURE_SWIPE_UP    = 128,
    GESTURE_SWIPE_DOWN  = 256,
    GESTURE_PINCH_IN    = 512,
    GESTURE_PINCH_OUT   = 1024
} Gestures;

typedef enum { TOUCH_UP, TOUCH_DOWN, TOUCH_MOVE } TouchAction;

// Gesture events
// NOTE: MAX_TOUCH_POINTS fixed to 4
typedef struct {
    int touchAction;
    int pointCount;
    int pointerId[4];
    Vector2 position[4];
} GestureEvent;

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
void SetGesturesEnabled(unsigned int gestureFlags);     // Enable a set of gestures using flags
bool IsGestureDetected(int gesture);                    // Check if a gesture have been detected
void ProcessGestureEvent(GestureEvent event);           // Process gesture event and translate it into gestures
void UpdateGestures(void);                              // Update gestures detected (must be called every frame)

int GetTouchPointsCount(void);                          // Get touch points count
int GetGestureDetected(void);                           // Get latest detected gesture
float GetGestureHoldDuration(void);                     // Get gesture hold time in milliseconds
Vector2 GetGestureDragVector(void);                     // Get gesture drag vector
float GetGestureDragAngle(void);                        // Get gesture drag angle
Vector2 GetGesturePinchVector(void);                    // Get gesture pinch delta
float GetGesturePinchAngle(void);                       // Get gesture pinch angle


#ifdef __cplusplus
}
#endif

#endif // GESTURES_H
