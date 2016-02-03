/**********************************************************************************************
*
*   raylib Gestures System - Gestures Processing based on input gesture events (touch/mouse)
*
*   Reviewed by Ramon Santamaria
*   Redesigned by Albert Martos and Ian Eito
*   Initial design by Marc Palau
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

//#define GESTURES_STANDALONE     // NOTE: To use the gestures module as standalone lib, just uncomment this line

#if defined(GESTURES_STANDALONE)
    #include "gestures.h"
#else
    #include "raylib.h"         // Required for typedef(s): Vector2, Gestures
#endif

#include <stdlib.h>             // malloc(), free()
#include <stdio.h>              // printf(), fprintf()
#include <math.h>               // Used for ...
#include <stdint.h>             // Defines int32_t, int64_t

#if defined(_WIN32)
    // Functions required to query time on Windows
    int __stdcall QueryPerformanceCounter(unsigned long long int *lpPerformanceCount);
    int __stdcall QueryPerformanceFrequency(unsigned long long int *lpFrequency);
#elif defined(__linux)
    #include <time.h>           // Used for clock functions
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define FORCE_TO_SWIPE          20
#define FORCE_TO_DRAG           20
#define FORCE_TO_PINCH          5
#define TAP_TIMEOUT             300     // Time in milliseconds
#define PINCH_TIMEOUT           300     // Time in milliseconds
#define DOUBLETAP_RANGE         30
//#define MAX_TOUCH_POINTS        4

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------

// Drag gesture variables
static Vector2 dragVector = { 0.0f , 0.0f };

// Touch gesture variables
static Vector2 touchDownPosition = { 0, 0 };
static Vector2 touchDownPosition2 = { 0, 0 };
static Vector2 touchUpPosition = { 0, 0 };
static Vector2 moveDownPosition = { 0, 0 };
static Vector2 moveDownPosition2 = { 0, 0 };

static int numTap = 0;
static int numHold = 0;
static int pointCount = 0;
static int touchId = -1;

static double eventTime = 0.0;

static float dragDistance = 0.0f;       // DRAG distance (from initial touch point to current)
static float dragAngle = 0;             // DRAG angle direction
static float dragIntensity = 0;         // DRAG intensity, how far why did the DRAG (pixels per frame)
static int draggingTimeCounter = 0;     // DRAG time         // RAY: WTF!!! Counting... frames???

// Pinch gesture variables
static float pinchDistance = 0.0f;      // Pinch displacement distance      // RAY: Not used! o__O

// Detected gestures
static int previousGesture = GESTURE_NONE;
static int currentGesture = GESTURE_NONE;

// Enabled gestures flags, all gestures enabled by default 
static unsigned int enabledGestures = 0b0000001111111111;   

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static float CalculateAngle(Vector2 initialPosition, Vector2 actualPosition, float magnitude);
static float Vector2Distance(Vector2 v1, Vector2 v2);
static double GetCurrentTime();

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Process gesture event and translate it into gestures
void ProcessGestureEvent(GestureEvent event)
{
    // Reset required variables
    dragVector = (Vector2){ 0.0f, 0.0f };   // RAY: Not used???
    pinchDistance = 0.0f;
    
    previousGesture = currentGesture;
    
    pointCount = event.pointCount;      // Required on UpdateGestures()
    
    if (pointCount < 2)
    {      
        touchId = event.pointerId[0];
        
        if (event.touchAction == TOUCH_DOWN)
        {
            numTap++;    // Tap counter

            // Detect GESTURE_DOUBLE_TAP
            if ((currentGesture == GESTURE_NONE) && (numTap >= 2) && ((GetCurrentTime() - eventTime) < TAP_TIMEOUT) && (Vector2Distance(touchDownPosition, event.position[0]) < DOUBLETAP_RANGE))
            {
                currentGesture = GESTURE_DOUBLETAP;
                numTap = 0;
            }
            else    // Detect GESTURE_TAP
            {
                numTap = 1;
                currentGesture = GESTURE_TAP;
            }
            
            touchDownPosition = event.position[0];
            
            touchUpPosition = touchDownPosition;
            eventTime = GetCurrentTime();
        }
        else if (event.touchAction == TOUCH_UP)
        {
            if (currentGesture == GESTURE_DRAG) touchUpPosition = event.position[0];

            // Calculate for swipe
            dragDistance = Vector2Distance(touchDownPosition, touchUpPosition);
            dragIntensity = dragDistance/(float)draggingTimeCounter;       // RAY: WTF!!! Counting frames???
            
            // Detect GESTURE_SWIPE
            if ((dragIntensity > FORCE_TO_SWIPE) && (touchId == 0))     // RAY: why check (touchId == 0)???
            {
                dragAngle = CalculateAngle(touchDownPosition, touchUpPosition, dragDistance);
                
                if ((dragAngle < 30) || (dragAngle > 330)) currentGesture = GESTURE_SWIPE_RIGHT;        // Right
                else if ((dragAngle > 30) && (dragAngle < 120)) currentGesture = GESTURE_SWIPE_UP;      // Up
                else if ((dragAngle > 120) && (dragAngle < 210)) currentGesture = GESTURE_SWIPE_LEFT;   // Left
                else if ((dragAngle > 210) && (dragAngle < 300)) currentGesture = GESTURE_SWIPE_DOWN;   // Down
                else currentGesture = GESTURE_NONE;
            }
            else
            {
                dragDistance = 0.0f;
                dragIntensity = 0.0f;
                dragAngle = 0.0f;
                
                currentGesture = GESTURE_NONE;
            }
            
            draggingTimeCounter = 0;
        }
        else if (event.touchAction == TOUCH_MOVE)
        {
            if (Vector2Distance(moveDownPosition, event.position[0]) > 5) eventTime = GetCurrentTime();
            
            moveDownPosition = event.position[0];
            
            if (currentGesture == GESTURE_HOLD) 
            {
                if (numHold == 1) touchDownPosition = event.position[0];
                
                numHold = 2;

                dragDistance = Vector2Distance(touchDownPosition, moveDownPosition);
                
                // Detect GESTURE_DRAG
                if (dragDistance >= FORCE_TO_DRAG) currentGesture = GESTURE_DRAG;
            }
            
            draggingTimeCounter++;      // RAY: What do you count??? Move event actions?
        }
    }
    else    // Two touch points
    {
        if (event.touchAction == TOUCH_DOWN)
        {
            touchDownPosition = event.position[0];
            touchDownPosition2 = event.position[1];
            
            currentGesture = GESTURE_HOLD;
        }
        else if (event.touchAction == TOUCH_MOVE)
        {
            pinchDistance = Vector2Distance(moveDownPosition, moveDownPosition2);
            
            touchDownPosition = moveDownPosition;
            touchDownPosition2 = moveDownPosition2;
            
            moveDownPosition = event.position[0];
            moveDownPosition2 = event.position[1];
            
            if ((Vector2Distance(touchDownPosition, moveDownPosition) > FORCE_TO_PINCH) || (Vector2Distance(touchDownPosition2, moveDownPosition2) > FORCE_TO_PINCH))
            {
                if ((Vector2Distance(moveDownPosition, moveDownPosition2) - pinchDistance) < 0) currentGesture = GESTURE_PINCH_IN;
                else currentGesture = GESTURE_PINCH_OUT;
            }
            else 
            {
               currentGesture = GESTURE_HOLD; 
            }
        }
        else if (event.touchAction == TOUCH_UP)
        {
            pinchDistance = 0.0f;
            
            currentGesture = GESTURE_NONE;
        }
    }
}

// Update gestures detected (must be called every frame)
void UpdateGestures(void)
{
    // NOTE: Gestures are processed through system callbacks on touch events

    // Detect GESTURE_HOLD
    if (((currentGesture == GESTURE_TAP) || (currentGesture == GESTURE_DOUBLETAP)) && pointCount < 2) currentGesture = GESTURE_HOLD;
    
    if ((GetCurrentTime() - eventTime) > TAP_TIMEOUT && (currentGesture == GESTURE_DRAG) && pointCount < 2)
    {
        currentGesture = GESTURE_HOLD;
        numHold = 1;
    }
   
    // Detect GESTURE_NONE
    if ((currentGesture == GESTURE_SWIPE_RIGHT) || (currentGesture == GESTURE_SWIPE_UP) || (currentGesture == GESTURE_SWIPE_LEFT) || (currentGesture == GESTURE_SWIPE_DOWN))
    {
        currentGesture = GESTURE_NONE;
    }
}

// Check if a gesture have been detected
bool IsGestureDetected(void)
{
    if ((enabledGestures & currentGesture) != GESTURE_NONE) return true;
    else return false;
}

// Check gesture type
int GetGestureType(void)
{
    // Get current gesture only if enabled
    return (enabledGestures & currentGesture);
}

void SetGesturesEnabled(unsigned int gestureFlags)
{
    enabledGestures = gestureFlags;
}

// Get drag dragIntensity (pixels per frame)
float GetGestureDragdragIntensity(void)
{
    return dragIntensity;
}

// Get drag angle
// NOTE: Angle in degrees, horizontal-right is 0, counterclock-wise
float GetGestureDragAngle(void)
{
    return dragAngle;
}

// Get drag vector (between initial and final position)
Vector2 GetGestureDragVector(void)
{
    // TODO: Calculate DRAG vector
    
    return dragVector;
}

// Hold time measured in ms
int GetGestureHoldDuration(void)
{
    // TODO: Return last hold time in ms
    
    return 0;
}

// Get distance between two pinch points
float GetGesturePinchDelta(void)
{
    return pinchDistance;
}

// Get angle beween two pinch points
// NOTE: Angle in degrees, horizontal-right is 0, counterclock-wise
float GetGesturePinchAngle(void)
{
    // TODO: Calculate pinch angle
    
    return 0;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// RAY: Do we really need magnitude??? why???
// TODO: Remove magnitude dependency...
static float CalculateAngle(Vector2 initialPosition, Vector2 finalPosition, float magnitude)
{
    float angle;
    
    // Calculate arcsinus of the movement   // RAY: o__O
    angle = asin((finalPosition.y - initialPosition.y)/magnitude);
    angle *= RAD2DEG;

    // RAY: review this (better) solution
    //angle = atan2(p1.y - p2.y, p1.x - p2.x);
    //angle *= RAD2DEG;
    
    // http://stackoverflow.com/questions/21483999/using-atan2-to-find-angle-between-two-vectors
    
    // TODO: Remove sector dependency (self-note: check moving eyes exercise)
    
    // Calculate angle depending on the sector
    if ((finalPosition.x - initialPosition.x) >= 0)
    {
        // Sector 4
        if ((finalPosition.y - initialPosition.y) >= 0)
        {
            angle *= -1;
            angle += 360;
        }
        // Sector 1
        else angle *= -1;
    }
    else
    {
        // Sector 3
        if ((finalPosition.y - initialPosition.y) >= 0) angle += 180;
        // Sector 2
        else
        {
            angle *= -1;
            angle = 180 - angle;
        }
    }
    
    return angle;
}

// Calculate distance between two Vector2
static float Vector2Distance(Vector2 v1, Vector2 v2)
{
    float result;

    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;

    result = sqrt(dx*dx + dy*dy);

    return result;
}

// Time measure returned are milliseconds
static double GetCurrentTime()
{
    double time = 0;
    
#if defined(_WIN32)
    unsigned long long int clockFrequency, currentTime;
    
    QueryPerformanceFrequency(&clockFrequency);
    QueryPerformanceCounter(&currentTime);
    
    time = (double)currentTime/clockFrequency*1000.0f;  // time in miliseconds
#endif

#if defined(__linux)
    // NOTE: Only for Linux-based systems
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    uint64_t nowTime = (uint64_t)now.tv_sec*1000000000LLU + (uint64_t)now.tv_nsec;     // Time provided in nanoseconds
    
    time = ((double)nowTime/1000000.0);     // time in miliseconds
#endif

    return time;
}
