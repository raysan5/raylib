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
typedef enum {
    TYPE_MOTIONLESS,
    TYPE_DRAG,
    TYPE_DUAL_INPUT
} GestureType;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
// Tap gesture variables
static Vector2 initialTapPosition = { 0, 0 };

// Double Tap gesture variables
static bool doubleTapping = false;
static bool untap = false;              // Check if recently done a tap

// Drag gesture variables
static Vector2 initialDragPosition = { 0, 0 };
static Vector2 endDragPosition = { 0, 0 };
static Vector2 lastDragPosition = { 0, 0 };
static Vector2 dragVector = { 0, 0 };

// Albert&Ian
static Vector2 touchDownPosition = { 0, 0 };
static Vector2 touchDownPosition2 = { 0, 0 };
static Vector2 touchUpPosition = { 0, 0 };
static Vector2 moveDownPosition = { 0, 0 };
static Vector2 moveDownPosition2 = { 0, 0 };

static int numTap = 0;
static int numHold = 0;
static int numPinch = 0;
static int pointCount = 0;
static int touchId = -1;

static double eventTime = 0;

static float magnitude = 0;             // Distance traveled dragging
static float angle = 0;                 // Angle direction of the drag
static float intensity = 0;             // How fast we did the drag (pixels per frame)
static int draggingTimeCounter = 0;     // Time that have passed while dragging

// Pinch gesture variables
static Vector2 firstInitialPinchPosition = { 0, 0 };
static Vector2 secondInitialPinchPosition = { 0, 0 };
static Vector2 firstEndPinchPosition = { 0, 0 };
static Vector2 secondEndPinchPosition = { 0, 0 };
static float pinchDelta = 0;            // Pinch delta displacement

// Detected gestures
static int previousGesture = GESTURE_NONE;
static int currentGesture = GESTURE_NONE;

// Enabled gestures flags, all gestures enabled by default 
static unsigned int enabledGestures = 0b0000001111111111;   

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void InitPinchGesture(Vector2 posA, Vector2 posB);
static float CalculateAngle(Vector2 initialPosition, Vector2 actualPosition, float magnitude);
static float VectorDistance(Vector2 v1, Vector2 v2);
static float VectorDotProduct(Vector2 v1, Vector2 v2);
static double GetCurrentTime();
static float Vector2Distance();

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Process gesture event and translate it into gestures
void ProcessGestureEvent(GestureEvent event)
{
    // Resets
    dragVector = (Vector2){ 0, 0 };
    pinchDelta = 0;
    
    previousGesture = currentGesture;
    
    pointCount = event.pointCount;
    
    // Albert&Ian
    if (pointCount < 2)
    {      
        touchId = event.pointerId[0];
        if (event.touchAction == TOUCH_DOWN)
        {
            numTap++;    // Tap counter

            // Detect GESTURE_DOUBLE_TAP
            if ((currentGesture == GESTURE_NONE) && (numTap >= 2) && ((GetCurrentTime() - eventTime) < TAP_TIMEOUT) && (GetMagnitude(touchDownPosition, event.position[0]) < DOUBLETAP_RANGE))
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
            if (currentGesture = GESTURE_DRAG) 
            {
                touchUpPosition = event.position[0];
            }
            
            // Calculate for swipe
            magnitude = GetMagnitude(touchDownPosition, touchUpPosition);
            intensity = magnitude / (float)draggingTimeCounter;
            
            // Detect GESTURE_SWIPE
            if ((intensity > FORCE_TO_SWIPE) && (touchId == 0))
            {
                angle = CalculateAngle(touchDownPosition, touchUpPosition, magnitude);
                if ((angle < 30) || (angle > 330)) currentGesture = GESTURE_SWIPE_RIGHT;        // Right
                else if ((angle > 30) && (angle < 120)) currentGesture = GESTURE_SWIPE_UP;      // Up
                else if ((angle > 120) && (angle < 210)) currentGesture = GESTURE_SWIPE_LEFT;   // Left
                else if ((angle > 210) && (angle < 300)) currentGesture = GESTURE_SWIPE_DOWN;   // Down
                else currentGesture = GESTURE_NONE;
            }
            else
            {
                magnitude = 0;
                angle = 0;
                intensity = 0;
                
                currentGesture = GESTURE_NONE;
            }
            
            draggingTimeCounter = 0;
        }
        else if (event.touchAction == TOUCH_MOVE)
        {
            if (GetMagnitude(moveDownPosition, event.position[0]) > 5) eventTime = GetCurrentTime();
            moveDownPosition = event.position[0];
            
            if (currentGesture == GESTURE_HOLD) 
            {
                if (numHold == 1) touchDownPosition = event.position[0];
                
                numHold = 2;

                magnitude = GetMagnitude(touchDownPosition, moveDownPosition);
                
                // Detect GESTURE_DRAG
                if (magnitude >= FORCE_TO_DRAG) currentGesture = GESTURE_DRAG;
            }
            
            draggingTimeCounter++;
        }
    }
    else
    {
        // two fingers
        
        if (event.touchAction == TOUCH_DOWN)
        {
            touchDownPosition = event.position[0];
            touchDownPosition2 = event.position[1];
            
            currentGesture = GESTURE_HOLD;
        }
        else if (event.touchAction == TOUCH_MOVE)
        {
            magnitude = GetMagnitude(moveDownPosition, moveDownPosition2);
            
            touchDownPosition = moveDownPosition;
            touchDownPosition2 = moveDownPosition2;
            
            moveDownPosition = event.position[0];
            moveDownPosition2 = event.position[1];
            
            if ( (GetMagnitude(touchDownPosition, moveDownPosition) > FORCE_TO_PINCH) || (GetMagnitude(touchDownPosition2, moveDownPosition2) > FORCE_TO_PINCH))
            {
                if ((GetMagnitude(moveDownPosition, moveDownPosition2) - magnitude) < 0) currentGesture = GESTURE_PINCH_IN;
                else currentGesture = GESTURE_PINCH_OUT;
            }
            else 
            {
               currentGesture = GESTURE_HOLD; 
            }
        }
        else if (event.touchAction == TOUCH_UP)
        {
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

// Calculate distance between two vectors
float Vector2Distance(Vector2 v1, Vector3 v2)
{
    float result;

    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;

    result = sqrt(dx*dx + dy*dy);

    return result;
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

// Get drag intensity (pixels per frame)
float GetGestureDragIntensity(void)
{
    return intensity;
}

// Get drag angle
// NOTE: Angle in degrees, horizontal-right is 0, counterclock-wise
float GetGestureDragAngle(void)
{
    return angle;
}

// Get drag vector (between initial and final position)
Vector2 GetGestureDragVector(void)
{
    return dragVector;
}

// Hold time measured in frames
int GetGestureHoldDuration(void)
{
    return 0;
}

// Get magnitude between two pinch points
float GetGesturePinchDelta(void)
{
    return pinchDelta;
}

// Get angle beween two pinch points
// NOTE: Angle in degrees, horizontal-right is 0, counterclock-wise
float GetGesturePinchAngle(void)
{
    return 0;
}

// Update gestures detected (must be called every frame)
void UpdateGestures(void)
{
    // NOTE: Gestures are processed through system callbacks on touch events
    
    // When screen is touched, in first frame GESTURE_TAP is called but in next frame touch event callback is not called (if touch position doesn't change),
    // so we need to store previous frame gesture type manually in this update function to switch to HOLD if current gesture is
    // GESTURE_TAP two frames in a row. Due to current gesture is set to HOLD, current gesture doesn't need to be reset to NONE every frame.
    // It will be reset when UP is called.
    if(currentGesture == GESTURE_TAP) previousGesture = currentGesture;
    
    if(previousGesture == GESTURE_TAP && currentGesture == GESTURE_TAP) currentGesture = GESTURE_HOLD;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

static float CalculateAngle(Vector2 initialPosition, Vector2 finalPosition, float magnitude)
{
    float angle;
    
    // Calculate arcsinus of the movement
    angle = asin((finalPosition.y - initialPosition.y)/magnitude);
    angle *= RAD2DEG;
    
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

static void InitPinchGesture(Vector2 posA, Vector2 posB)
{
    initialDragPosition = (Vector2){ 0, 0 };
    endDragPosition = (Vector2){ 0, 0 };
    lastDragPosition = (Vector2){ 0, 0 };

    // Initialize positions
    firstInitialPinchPosition = posA;
    secondInitialPinchPosition = posB;
    
    firstEndPinchPosition = firstInitialPinchPosition;
    secondEndPinchPosition = secondInitialPinchPosition;
    
    // Resets
    magnitude = 0;
    angle = 0;
    intensity = 0;

    gestureType = TYPE_DUAL_INPUT;
}

static float VectorDistance(Vector2 v1, Vector2 v2)
{
    float result;

    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;

    result = sqrt(dx*dx + dy*dy);

    return result;
}

static float VectorDotProduct(Vector2 v1, Vector2 v2)
{
    float result;

    float v1Module = sqrt(v1.x*v1.x + v1.y*v1.y);
    float v2Module = sqrt(v2.x*v2.x + v2.y*v2.y);
    
    Vector2 v1Normalized = { v1.x / v1Module, v1.y / v1Module };
    Vector2 v2Normalized = { v2.x / v2Module, v2.y / v2Module };
    
    result = v1Normalized.x*v2Normalized.x + v1Normalized.y*v2Normalized.y;

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
