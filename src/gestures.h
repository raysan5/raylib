/**********************************************************************************************
*
*   raylib.gestures - Gestures system, gestures processing based on input events (touch/mouse)
*
*   NOTE: Memory footprint of this library is aproximately 128 bytes (global variables)
*
*   CONFIGURATION:
*
*   #define GESTURES_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*   #define GESTURES_STANDALONE
*       If defined, the library can be used as standalone to process gesture events with
*       no external dependencies.
*
*   CONTRIBUTORS:
*       Marc Palau:         Initial implementation (2014)
*       Albert Martos:      Complete redesign and testing (2015)
*       Ian Eito:           Complete redesign and testing (2015)
*       Ramon Santamaria:   Supervision, review, update and maintenance
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2021 Ramon Santamaria (@raysan5)
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

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Types and Structures Definition
// NOTE: Below types are required for GESTURES_STANDALONE usage
//----------------------------------------------------------------------------------
#if defined(GESTURES_STANDALONE)
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
        GESTURE_NONE        = 0,
        GESTURE_TAP         = 1,
        GESTURE_DOUBLETAP   = 2,
        GESTURE_HOLD        = 4,
        GESTURE_DRAG        = 8,
        GESTURE_SWIPE_RIGHT = 16,
        GESTURE_SWIPE_LEFT  = 32,
        GESTURE_SWIPE_UP    = 64,
        GESTURE_SWIPE_DOWN  = 128,
        GESTURE_PINCH_IN    = 256,
        GESTURE_PINCH_OUT   = 512
    } Gestures;
#endif

typedef enum { TOUCH_UP, TOUCH_DOWN, TOUCH_MOVE } TouchAction;

// Gesture event
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
void ProcessGestureEvent(GestureEvent event);           // Process gesture event and translate it into gestures
void UpdateGestures(void);                              // Update gestures detected (must be called every frame)

#if defined(GESTURES_STANDALONE)
void SetGesturesEnabled(unsigned int flags);     // Enable a set of gestures using flags
bool IsGestureDetected(int gesture);                    // Check if a gesture have been detected
int GetGestureDetected(void);                           // Get latest detected gesture
int GetTouchPointsCount(void);                          // Get touch points count
float GetGestureHoldDuration(void);                     // Get gesture hold time in milliseconds
Vector2 GetGestureDragVector(void);                     // Get gesture drag vector
float GetGestureDragAngle(void);                        // Get gesture drag angle
Vector2 GetGesturePinchVector(void);                    // Get gesture pinch delta
float GetGesturePinchAngle(void);                       // Get gesture pinch angle
#endif

#ifdef __cplusplus
}
#endif

#endif // GESTURES_H

/***********************************************************************************
*
*   GESTURES IMPLEMENTATION
*
************************************************************************************/

#if defined(GESTURES_IMPLEMENTATION)

#if defined(_WIN32)
    // Functions required to query time on Windows
    int __stdcall QueryPerformanceCounter(unsigned long long int *lpPerformanceCount);
    int __stdcall QueryPerformanceFrequency(unsigned long long int *lpFrequency);
#elif defined(__linux__)
    #if _POSIX_C_SOURCE < 199309L
        #undef _POSIX_C_SOURCE
        #define _POSIX_C_SOURCE 199309L // Required for CLOCK_MONOTONIC if compiled with c99 without gnu ext.
    #endif
    #include <sys/time.h>               // Required for: timespec
    #include <time.h>                   // Required for: clock_gettime()

    #include <math.h>                   // Required for: sqrtf(), atan2f()
#endif
#if defined(__APPLE__)                  // macOS also defines __MACH__
    #include <mach/clock.h>             // Required for: clock_get_time()
    #include <mach/mach.h>              // Required for: mach_timespec_t
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define FORCE_TO_SWIPE      0.0005f     // Swipe force, measured in normalized screen units/time
#define MINIMUM_DRAG        0.015f      // Drag minimum force, measured in normalized screen units (0.0f to 1.0f)
#define MINIMUM_PINCH       0.005f      // Pinch minimum force, measured in normalized screen units (0.0f to 1.0f)
#define TAP_TIMEOUT         300         // Tap minimum time, measured in milliseconds
#define PINCH_TIMEOUT       300         // Pinch minimum time, measured in milliseconds
#define DOUBLETAP_RANGE     0.03f       // DoubleTap range, measured in normalized screen units (0.0f to 1.0f)

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Gestures module state context [136 bytes]
typedef struct {
    unsigned int current;               // Current detected gesture
    unsigned int enabledFlags;          // Enabled gestures flags
    struct {
        int firstId;                    // Touch id for first touch point
        int pointCount;                 // Touch points counter
        double eventTime;               // Time stamp when an event happened
        Vector2 upPosition;             // Touch up position
        Vector2 downPositionA;          // First touch down position
        Vector2 downPositionB;          // Second touch down position
        Vector2 downDragPosition;       // Touch drag position
        Vector2 moveDownPositionA;      // First touch down position on move
        Vector2 moveDownPositionB;      // Second touch down position on move
        int tapCounter;                 // TAP counter (one tap implies TOUCH_DOWN and TOUCH_UP actions)
    } Touch;
    struct {
        bool resetRequired;             // HOLD reset to get first touch point again
        double timeDuration;            // HOLD duration in milliseconds
    } Hold;
    struct {
        Vector2 vector;                 // DRAG vector (between initial and current position)
        float angle;                    // DRAG angle (relative to x-axis)
        float distance;                 // DRAG distance (from initial touch point to final) (normalized [0..1])
        float intensity;                // DRAG intensity, how far why did the DRAG (pixels per frame)
    } Drag;
    struct {
        bool start;                     // SWIPE used to define when start measuring GESTURES.Swipe.timeDuration
        double timeDuration;            // SWIPE time to calculate drag intensity
    } Swipe;
    struct {
        Vector2 vector;                 // PINCH vector (between first and second touch points)
        float angle;                    // PINCH angle (relative to x-axis)
        float distance;                 // PINCH displacement distance (normalized [0..1])
    } Pinch;
} GesturesData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static GesturesData GESTURES = {
    .Touch.firstId = -1,
    .current = GESTURE_NONE,            // No current gesture detected
    .enabledFlags = 0b0000001111111111  // All gestures supported by default
};

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
#if defined(GESTURES_STANDALONE)
// Some required math functions provided by raymath.h
static float Vector2Angle(Vector2 initialPosition, Vector2 finalPosition);
static float Vector2Distance(Vector2 v1, Vector2 v2);
#endif
static double GetCurrentTime(void);

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Enable only desired getures to be detected
void SetGesturesEnabled(unsigned int flags)
{
    GESTURES.enabledFlags = flags;
}

// Check if a gesture have been detected
bool IsGestureDetected(int gesture)
{
    if ((GESTURES.enabledFlags & GESTURES.current) == gesture) return true;
    else return false;
}

// Process gesture event and translate it into gestures
void ProcessGestureEvent(GestureEvent event)
{
    // Reset required variables
    GESTURES.Touch.pointCount = event.pointCount;      // Required on UpdateGestures()

    if (GESTURES.Touch.pointCount < 2)
    {
        if (event.touchAction == TOUCH_DOWN)
        {
            GESTURES.Touch.tapCounter++;    // Tap counter

            // Detect GESTURE_DOUBLE_TAP
            if ((GESTURES.current == GESTURE_NONE) && (GESTURES.Touch.tapCounter >= 2) && ((GetCurrentTime() - GESTURES.Touch.eventTime) < TAP_TIMEOUT) && (Vector2Distance(GESTURES.Touch.downPositionA, event.position[0]) < DOUBLETAP_RANGE))
            {
                GESTURES.current = GESTURE_DOUBLETAP;
                GESTURES.Touch.tapCounter = 0;
            }
            else    // Detect GESTURE_TAP
            {
                GESTURES.Touch.tapCounter = 1;
                GESTURES.current = GESTURE_TAP;
            }

            GESTURES.Touch.downPositionA = event.position[0];
            GESTURES.Touch.downDragPosition = event.position[0];

            GESTURES.Touch.upPosition = GESTURES.Touch.downPositionA;
            GESTURES.Touch.eventTime = GetCurrentTime();

            GESTURES.Touch.firstId = event.pointerId[0];

            GESTURES.Drag.vector = (Vector2){ 0.0f, 0.0f };
        }
        else if (event.touchAction == TOUCH_UP)
        {
            if (GESTURES.current == GESTURE_DRAG) GESTURES.Touch.upPosition = event.position[0];

            // NOTE: GESTURES.Drag.intensity dependend on the resolution of the screen
            GESTURES.Drag.distance = Vector2Distance(GESTURES.Touch.downPositionA, GESTURES.Touch.upPosition);
            GESTURES.Drag.intensity = GESTURES.Drag.distance/(float)((GetCurrentTime() - GESTURES.Swipe.timeDuration));

            GESTURES.Swipe.start = false;

            // Detect GESTURE_SWIPE
            if ((GESTURES.Drag.intensity > FORCE_TO_SWIPE) && (GESTURES.Touch.firstId == event.pointerId[0]))
            {
                // NOTE: Angle should be inverted in Y
                GESTURES.Drag.angle = 360.0f - Vector2Angle(GESTURES.Touch.downPositionA, GESTURES.Touch.upPosition);

                if ((GESTURES.Drag.angle < 30) || (GESTURES.Drag.angle > 330)) GESTURES.current = GESTURE_SWIPE_RIGHT;        // Right
                else if ((GESTURES.Drag.angle > 30) && (GESTURES.Drag.angle < 120)) GESTURES.current = GESTURE_SWIPE_UP;      // Up
                else if ((GESTURES.Drag.angle > 120) && (GESTURES.Drag.angle < 210)) GESTURES.current = GESTURE_SWIPE_LEFT;   // Left
                else if ((GESTURES.Drag.angle > 210) && (GESTURES.Drag.angle < 300)) GESTURES.current = GESTURE_SWIPE_DOWN;   // Down
                else GESTURES.current = GESTURE_NONE;
            }
            else
            {
                GESTURES.Drag.distance = 0.0f;
                GESTURES.Drag.intensity = 0.0f;
                GESTURES.Drag.angle = 0.0f;

                GESTURES.current = GESTURE_NONE;
            }

            GESTURES.Touch.downDragPosition = (Vector2){ 0.0f, 0.0f };
            GESTURES.Touch.pointCount = 0;
        }
        else if (event.touchAction == TOUCH_MOVE)
        {
            if (GESTURES.current == GESTURE_DRAG) GESTURES.Touch.eventTime = GetCurrentTime();

            if (!GESTURES.Swipe.start)
            {
                GESTURES.Swipe.timeDuration = GetCurrentTime();
                GESTURES.Swipe.start = true;
            }

            GESTURES.Touch.moveDownPositionA = event.position[0];

            if (GESTURES.current == GESTURE_HOLD)
            {
                if (GESTURES.Hold.resetRequired) GESTURES.Touch.downPositionA = event.position[0];

                GESTURES.Hold.resetRequired = false;

                // Detect GESTURE_DRAG
                if (Vector2Distance(GESTURES.Touch.downPositionA, GESTURES.Touch.moveDownPositionA) >= MINIMUM_DRAG)
                {
                    GESTURES.Touch.eventTime = GetCurrentTime();
                    GESTURES.current = GESTURE_DRAG;
                }
            }

            GESTURES.Drag.vector.x = GESTURES.Touch.moveDownPositionA.x - GESTURES.Touch.downDragPosition.x;
            GESTURES.Drag.vector.y = GESTURES.Touch.moveDownPositionA.y - GESTURES.Touch.downDragPosition.y;
        }
    }
    else    // Two touch points
    {
        if (event.touchAction == TOUCH_DOWN)
        {
            GESTURES.Touch.downPositionA = event.position[0];
            GESTURES.Touch.downPositionB = event.position[1];

            //GESTURES.Pinch.distance = Vector2Distance(GESTURES.Touch.downPositionA, GESTURES.Touch.downPositionB);

            GESTURES.Pinch.vector.x = GESTURES.Touch.downPositionB.x - GESTURES.Touch.downPositionA.x;
            GESTURES.Pinch.vector.y = GESTURES.Touch.downPositionB.y - GESTURES.Touch.downPositionA.y;

            GESTURES.current = GESTURE_HOLD;
            GESTURES.Hold.timeDuration = GetCurrentTime();
        }
        else if (event.touchAction == TOUCH_MOVE)
        {
            GESTURES.Pinch.distance = Vector2Distance(GESTURES.Touch.moveDownPositionA, GESTURES.Touch.moveDownPositionB);

            GESTURES.Touch.downPositionA = GESTURES.Touch.moveDownPositionA;
            GESTURES.Touch.downPositionB = GESTURES.Touch.moveDownPositionB;

            GESTURES.Touch.moveDownPositionA = event.position[0];
            GESTURES.Touch.moveDownPositionB = event.position[1];

            GESTURES.Pinch.vector.x = GESTURES.Touch.moveDownPositionB.x - GESTURES.Touch.moveDownPositionA.x;
            GESTURES.Pinch.vector.y = GESTURES.Touch.moveDownPositionB.y - GESTURES.Touch.moveDownPositionA.y;

            if ((Vector2Distance(GESTURES.Touch.downPositionA, GESTURES.Touch.moveDownPositionA) >= MINIMUM_PINCH) || (Vector2Distance(GESTURES.Touch.downPositionB, GESTURES.Touch.moveDownPositionB) >= MINIMUM_PINCH))
            {
                if ((Vector2Distance(GESTURES.Touch.moveDownPositionA, GESTURES.Touch.moveDownPositionB) - GESTURES.Pinch.distance) < 0) GESTURES.current = GESTURE_PINCH_IN;
                else GESTURES.current = GESTURE_PINCH_OUT;
            }
            else
            {
                GESTURES.current = GESTURE_HOLD;
                GESTURES.Hold.timeDuration = GetCurrentTime();
            }

            // NOTE: Angle should be inverted in Y
            GESTURES.Pinch.angle = 360.0f - Vector2Angle(GESTURES.Touch.moveDownPositionA, GESTURES.Touch.moveDownPositionB);
        }
        else if (event.touchAction == TOUCH_UP)
        {
            GESTURES.Pinch.distance = 0.0f;
            GESTURES.Pinch.angle = 0.0f;
            GESTURES.Pinch.vector = (Vector2){ 0.0f, 0.0f };
            GESTURES.Touch.pointCount = 0;

            GESTURES.current = GESTURE_NONE;
        }
    }
}

// Update gestures detected (must be called every frame)
void UpdateGestures(void)
{
    // NOTE: Gestures are processed through system callbacks on touch events

    // Detect GESTURE_HOLD
    if (((GESTURES.current == GESTURE_TAP) || (GESTURES.current == GESTURE_DOUBLETAP)) && (GESTURES.Touch.pointCount < 2))
    {
        GESTURES.current = GESTURE_HOLD;
        GESTURES.Hold.timeDuration = GetCurrentTime();
    }

    if (((GetCurrentTime() - GESTURES.Touch.eventTime) > TAP_TIMEOUT) && (GESTURES.current == GESTURE_DRAG) && (GESTURES.Touch.pointCount < 2))
    {
        GESTURES.current = GESTURE_HOLD;
        GESTURES.Hold.timeDuration = GetCurrentTime();
        GESTURES.Hold.resetRequired = true;
    }

    // Detect GESTURE_NONE
    if ((GESTURES.current == GESTURE_SWIPE_RIGHT) || (GESTURES.current == GESTURE_SWIPE_UP) || (GESTURES.current == GESTURE_SWIPE_LEFT) || (GESTURES.current == GESTURE_SWIPE_DOWN))
    {
        GESTURES.current = GESTURE_NONE;
    }
}

// Get number of touch points
int GetTouchPointsCount(void)
{
    // NOTE: point count is calculated when ProcessGestureEvent(GestureEvent event) is called

    return GESTURES.Touch.pointCount;
}

// Get latest detected gesture
int GetGestureDetected(void)
{
    // Get current gesture only if enabled
    return (GESTURES.enabledFlags & GESTURES.current);
}

// Hold time measured in ms
float GetGestureHoldDuration(void)
{
    // NOTE: time is calculated on current gesture HOLD

    double time = 0.0;

    if (GESTURES.current == GESTURE_HOLD) time = GetCurrentTime() - GESTURES.Hold.timeDuration;

    return (float)time;
}

// Get drag vector (between initial touch point to current)
Vector2 GetGestureDragVector(void)
{
    // NOTE: drag vector is calculated on one touch points TOUCH_MOVE

    return GESTURES.Drag.vector;
}

// Get drag angle
// NOTE: Angle in degrees, horizontal-right is 0, counterclock-wise
float GetGestureDragAngle(void)
{
    // NOTE: drag angle is calculated on one touch points TOUCH_UP

    return GESTURES.Drag.angle;
}

// Get distance between two pinch points
Vector2 GetGesturePinchVector(void)
{
    // NOTE: The position values used for GESTURES.Pinch.distance are not modified like the position values of [core.c]-->GetTouchPosition(int index)
    // NOTE: pinch distance is calculated on two touch points TOUCH_MOVE

    return GESTURES.Pinch.vector;
}

// Get angle beween two pinch points
// NOTE: Angle in degrees, horizontal-right is 0, counterclock-wise
float GetGesturePinchAngle(void)
{
    // NOTE: pinch angle is calculated on two touch points TOUCH_MOVE

    return GESTURES.Pinch.angle;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
#if defined(GESTURES_STANDALONE)
// Returns angle from two-points vector with X-axis
static float Vector2Angle(Vector2 v1, Vector2 v2)
{
    float angle = atan2f(v2.y - v1.y, v2.x - v1.x)*(180.0f/PI);

    if (angle < 0) angle += 360.0f;

    return angle;
}

// Calculate distance between two Vector2
static float Vector2Distance(Vector2 v1, Vector2 v2)
{
    float result;

    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;

    result = (float)sqrt(dx*dx + dy*dy);

    return result;
}
#endif

// Time measure returned are milliseconds
static double GetCurrentTime(void)
{
    double time = 0;

#if defined(_WIN32)
    unsigned long long int clockFrequency, currentTime;

    QueryPerformanceFrequency(&clockFrequency);     // BE CAREFUL: Costly operation!
    QueryPerformanceCounter(&currentTime);

    time = (double)currentTime/clockFrequency*1000.0f;  // Time in miliseconds
#endif

#if defined(__linux__)
    // NOTE: Only for Linux-based systems
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    unsigned long long int nowTime = (unsigned long long int)now.tv_sec*1000000000LLU + (unsigned long long int)now.tv_nsec;     // Time in nanoseconds

    time = ((double)nowTime/1000000.0);     // Time in miliseconds
#endif

#if defined(__APPLE__)
    //#define CLOCK_REALTIME  CALENDAR_CLOCK    // returns UTC time since 1970-01-01
    //#define CLOCK_MONOTONIC SYSTEM_CLOCK      // returns the time since boot time

    clock_serv_t cclock;
    mach_timespec_t now;
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);

    // NOTE: OS X does not have clock_gettime(), using clock_get_time()
    clock_get_time(cclock, &now);
    mach_port_deallocate(mach_task_self(), cclock);
    unsigned long long int nowTime = (unsigned long long int)now.tv_sec*1000000000LLU + (unsigned long long int)now.tv_nsec;     // Time in nanoseconds

    time = ((double)nowTime/1000000.0);     // Time in miliseconds
#endif

    return time;
}

#endif // GESTURES_IMPLEMENTATION
