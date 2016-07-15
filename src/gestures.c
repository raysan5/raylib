/**********************************************************************************************
*
*   raylib Gestures System - Gestures Processing based on input gesture events (touch/mouse)
*
*   Initial design by Marc Palau
*   Redesigned by Albert Martos and Ian Eito
*   Reviewed by Ramon Santamaria (@raysan5)
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
    #include "raylib.h"         // Required for: Vector2, Gestures
#endif

#include <math.h>               // Required for: atan2(), sqrt()
#include <stdint.h>             // Required for: uint64_t

#if defined(_WIN32)
    // Functions required to query time on Windows
    int __stdcall QueryPerformanceCounter(unsigned long long int *lpPerformanceCount);
    int __stdcall QueryPerformanceFrequency(unsigned long long int *lpFrequency);
#elif defined(__linux)
    #include <sys/time.h>       // Required for: timespec
    #include <time.h>           // Required for: clock_gettime()
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define FORCE_TO_SWIPE          0.0005f     // Measured in normalized screen units/time
#define MINIMUM_DRAG            0.015f      // Measured in normalized screen units (0.0f to 1.0f)
#define MINIMUM_PINCH           0.005f      // Measured in normalized screen units (0.0f to 1.0f)
#define TAP_TIMEOUT             300         // Time in milliseconds
#define PINCH_TIMEOUT           300         // Time in milliseconds
#define DOUBLETAP_RANGE         0.03f       // Measured in normalized screen units (0.0f to 1.0f)

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------

// Touch gesture variables
static Vector2 touchDownPosition = { 0.0f, 0.0f };
static Vector2 touchDownPosition2 = { 0.0f, 0.0f };
static Vector2 touchDownDragPosition = { 0.0f, 0.0f };
static Vector2 touchUpPosition = { 0.0f, 0.0f };
static Vector2 moveDownPosition = { 0.0f, 0.0f };
static Vector2 moveDownPosition2 = { 0.0f, 0.0f };
static int numTap = 0;

static int pointCount = 0;
static int firstTouchId = -1;

static double eventTime = 0.0;
static double swipeTime = 0.0;

// Hold gesture variables
static int numHold = 0;
static float timeHold = 0.0f;

// Drag gesture variables
static Vector2 dragVector = { 0.0f , 0.0f };    // DRAG vector (between initial and current position)
static float dragAngle = 0.0f;                  // DRAG angle (relative to x-axis)
static float dragDistance = 0.0f;               // DRAG distance (from initial touch point to final) (normalized [0..1])
static float dragIntensity = 0.0f;              // DRAG intensity, how far why did the DRAG (pixels per frame)
static bool startMoving = false;                // SWIPE used to define when start measuring swipeTime

// Pinch gesture variables
static Vector2 pinchVector = { 0.0f , 0.0f };   // PINCH vector (between first and second touch points)
static float pinchAngle = 0.0f;                 // PINCH angle (relative to x-axis)
static float pinchDistance = 0.0f;              // PINCH displacement distance (normalized [0..1])

// Detected gestures
static int previousGesture = GESTURE_NONE;
static int currentGesture = GESTURE_NONE;

// Enabled gestures flags, all gestures enabled by default 
static unsigned int enabledGestures = 0b0000001111111111;   

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static float Vector2Angle(Vector2 initialPosition, Vector2 finalPosition);
static float Vector2Distance(Vector2 v1, Vector2 v2);
static double GetCurrentTime(void);

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Enable only desired getures to be detected
void SetGesturesEnabled(unsigned int gestureFlags)
{
    enabledGestures = gestureFlags;
}

// Check if a gesture have been detected
bool IsGestureDetected(int gesture)
{
    if ((enabledGestures & currentGesture) == gesture) return true;
    else return false;
}

// Process gesture event and translate it into gestures
void ProcessGestureEvent(GestureEvent event)
{
    // Reset required variables
    previousGesture = currentGesture;
    
    pointCount = event.pointCount;      // Required on UpdateGestures()
    
    if (pointCount < 2)
    {
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
            touchDownDragPosition = event.position[0];
            
            touchUpPosition = touchDownPosition;
            eventTime = GetCurrentTime();
            
            firstTouchId = event.pointerId[0];
            
            dragVector = (Vector2){ 0.0f, 0.0f };
        }
        else if (event.touchAction == TOUCH_UP)
        {
            if (currentGesture == GESTURE_DRAG) touchUpPosition = event.position[0];

            // NOTE: dragIntensity dependend on the resolution of the screen
            dragDistance = Vector2Distance(touchDownPosition, touchUpPosition);
            dragIntensity = dragDistance/(float)((GetCurrentTime() - swipeTime));
            
            startMoving = false;
            
            // Detect GESTURE_SWIPE
            if ((dragIntensity > FORCE_TO_SWIPE) && firstTouchId == event.pointerId[0])
            {
                // NOTE: Angle should be inverted in Y
                dragAngle = 360.0f - Vector2Angle(touchDownPosition, touchUpPosition);
                
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
            
            touchDownDragPosition = (Vector2){ 0.0f, 0.0f };
            pointCount = 0;
        }
        else if (event.touchAction == TOUCH_MOVE)
        {
            if (currentGesture == GESTURE_DRAG) eventTime = GetCurrentTime();
            
            if (!startMoving)
            {
                swipeTime = GetCurrentTime();
                startMoving = true;
            }
            
            moveDownPosition = event.position[0];
            
            if (currentGesture == GESTURE_HOLD)
            {
                if (numHold == 1) touchDownPosition = event.position[0];
                
                numHold = 2;
                
                // Detect GESTURE_DRAG
                if (Vector2Distance(touchDownPosition, moveDownPosition) >= MINIMUM_DRAG)
                {
                    eventTime = GetCurrentTime();
                    currentGesture = GESTURE_DRAG;
                }
            }
        
            dragVector.x = moveDownPosition.x - touchDownDragPosition.x;
            dragVector.y = moveDownPosition.y - touchDownDragPosition.y;
        }
    }
    else    // Two touch points
    {
        if (event.touchAction == TOUCH_DOWN)
        {
            touchDownPosition = event.position[0];
            touchDownPosition2 = event.position[1];
            
            //pinchDistance = Vector2Distance(touchDownPosition, touchDownPosition2);
            
            pinchVector.x = touchDownPosition2.x - touchDownPosition.x;
            pinchVector.y = touchDownPosition2.y - touchDownPosition.y;
            
            currentGesture = GESTURE_HOLD;
            timeHold = GetCurrentTime();
        }
        else if (event.touchAction == TOUCH_MOVE)
        {
            pinchDistance = Vector2Distance(moveDownPosition, moveDownPosition2);
            
            touchDownPosition = moveDownPosition;
            touchDownPosition2 = moveDownPosition2;
            
            moveDownPosition = event.position[0];
            moveDownPosition2 = event.position[1];
            
            pinchVector.x = moveDownPosition2.x - moveDownPosition.x;
            pinchVector.y = moveDownPosition2.y - moveDownPosition.y;
                        
            if ((Vector2Distance(touchDownPosition, moveDownPosition) >= MINIMUM_PINCH) || (Vector2Distance(touchDownPosition2, moveDownPosition2) >= MINIMUM_PINCH))
            {
                if ((Vector2Distance(moveDownPosition, moveDownPosition2) - pinchDistance) < 0) currentGesture = GESTURE_PINCH_IN;
                else currentGesture = GESTURE_PINCH_OUT;
            }
            else
            {
                currentGesture = GESTURE_HOLD;
                timeHold = GetCurrentTime();
            }
            
            // NOTE: Angle should be inverted in Y
            pinchAngle = 360.0f - Vector2Angle(moveDownPosition, moveDownPosition2);
        }
        else if (event.touchAction == TOUCH_UP)
        {
            pinchDistance = 0.0f;
            pinchAngle = 0.0f;
            pinchVector = (Vector2){ 0.0f, 0.0f };
            pointCount = 0;
            
            currentGesture = GESTURE_NONE;
        }
    }
}

// Update gestures detected (must be called every frame)
void UpdateGestures(void)
{
    // NOTE: Gestures are processed through system callbacks on touch events

    // Detect GESTURE_HOLD
    if (((currentGesture == GESTURE_TAP) || (currentGesture == GESTURE_DOUBLETAP)) && (pointCount < 2))
    {
        currentGesture = GESTURE_HOLD;
        timeHold = GetCurrentTime();
    }
    
    if (((GetCurrentTime() - eventTime) > TAP_TIMEOUT) && (currentGesture == GESTURE_DRAG) && (pointCount < 2))
    {
        currentGesture = GESTURE_HOLD;
        timeHold = GetCurrentTime();
        numHold = 1;
    }
   
    // Detect GESTURE_NONE
    if ((currentGesture == GESTURE_SWIPE_RIGHT) || (currentGesture == GESTURE_SWIPE_UP) || (currentGesture == GESTURE_SWIPE_LEFT) || (currentGesture == GESTURE_SWIPE_DOWN))
    {
        currentGesture = GESTURE_NONE;
    }
}

// Get number of touch points
int GetTouchPointsCount(void)
{
    // NOTE: point count is calculated when ProcessGestureEvent(GestureEvent event) is called
    
    return pointCount;
}

// Get latest detected gesture
int GetGestureDetected(void)
{
    // Get current gesture only if enabled
    return (enabledGestures & currentGesture);
}

// Hold time measured in ms
float GetGestureHoldDuration(void)
{
    // NOTE: time is calculated on current gesture HOLD
    
    float time = 0.0f;
    
    if (currentGesture == GESTURE_HOLD) time = (float)GetCurrentTime() - timeHold;
    
    return time;
}

// Get drag vector (between initial touch point to current)
Vector2 GetGestureDragVector(void)
{
    // NOTE: drag vector is calculated on one touch points TOUCH_MOVE
    
    return dragVector;
}

// Get drag angle
// NOTE: Angle in degrees, horizontal-right is 0, counterclock-wise
float GetGestureDragAngle(void)
{
    // NOTE: drag angle is calculated on one touch points TOUCH_UP
    
    return dragAngle;
}

// Get distance between two pinch points
Vector2 GetGesturePinchVector(void)
{
    // NOTE: The position values used for pinchDistance are not modified like the position values of [core.c]-->GetTouchPosition(int index)
    // NOTE: pinch distance is calculated on two touch points TOUCH_MOVE
    
    return pinchVector;
}

// Get angle beween two pinch points
// NOTE: Angle in degrees, horizontal-right is 0, counterclock-wise
float GetGesturePinchAngle(void)
{
    // NOTE: pinch angle is calculated on two touch points TOUCH_MOVE
    
    return pinchAngle;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Returns angle from two-points vector with X-axis
static float Vector2Angle(Vector2 initialPosition, Vector2 finalPosition)
{
    float angle;

    angle = atan2(finalPosition.y - initialPosition.y, finalPosition.x - initialPosition.x);
    angle *= RAD2DEG;
    
    if (angle < 0) angle += 360.0f;

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
static double GetCurrentTime(void)
{
    double time = 0;
    
#if defined(_WIN32)
    unsigned long long int clockFrequency, currentTime;
    
    QueryPerformanceFrequency(&clockFrequency);
    QueryPerformanceCounter(&currentTime);
    
    time = (double)currentTime/clockFrequency*1000.0f;  // Time in miliseconds
#endif

#if defined(__linux)
    // NOTE: Only for Linux-based systems
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    uint64_t nowTime = (uint64_t)now.tv_sec*1000000000LLU + (uint64_t)now.tv_nsec;     // Time in nanoseconds
    
    time = ((double)nowTime/1000000.0);     // Time in miliseconds
#endif

    return time;
}
