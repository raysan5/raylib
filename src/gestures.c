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
    //#define WIN32_LEAN_AND_MEAN
    //#include <Windows.h>        // ISSUE: Rectangle redeclared, CloseWindow/ShowCursor conflicting types
int __stdcall QueryPerformanceCounter(unsigned long long int *lpPerformanceCount);
int __stdcall QueryPerformanceFrequency(unsigned long long int *lpFrequency);
#elif defined(__linux)
    #include <time.h>           // Used for clock functions
#endif

#if defined(PLATFORM_ANDROID)
    #include <jni.h>                        // Java native interface
    #include <android/sensor.h>             // Android sensors functions
    #include <android/window.h>             // Defines AWINDOW_FLAG_FULLSCREEN and others
#endif

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
    #include <emscripten/html5.h>
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define FORCE_TO_SWIPE          20
#define TAP_TIMEOUT             300
#define MAX_TOUCH_POINTS        4

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum {
    TYPE_MOTIONLESS,
    TYPE_DRAG,
    TYPE_DUAL_INPUT
} GestureType;

typedef enum {
    UP,
    DOWN,
    MOVE
} ActionType;

typedef struct {
    ActionType action;
    int pointCount;
    int pointerId[MAX_TOUCH_POINTS];
    Vector2 position[MAX_TOUCH_POINTS];
} GestureEvent;


//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static GestureType gestureType = TYPE_MOTIONLESS;
static double eventTime = 0;
//static int32_t touchId;               // Not used...

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

static unsigned int enabledGestures = 0;       // TODO: Currently not in use...

static Vector2 touchPosition;

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void ProcessMotionEvent(GestureEvent event);

static void InitPinchGesture(Vector2 posA, Vector2 posB);
static float CalculateAngle(Vector2 initialPosition, Vector2 actualPosition, float magnitude);
static float VectorDistance(Vector2 v1, Vector2 v2);
static float VectorDotProduct(Vector2 v1, Vector2 v2);
static double GetCurrentTime();

#if defined(PLATFORM_WEB)
static EM_BOOL EmscriptenInputCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
#endif

#if defined(PLATFORM_ANDROID)
static int32_t AndroidInputCallback(struct android_app *app, AInputEvent *event);
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Returns touch position X
int GetTouchX(void)
{
    return (int)touchPosition.x;
}

// Returns touch position Y
int GetTouchY(void)
{
    return (int)touchPosition.y;
}

// Returns touch position XY
// TODO: touch position should be scaled depending on display size and render size
Vector2 GetTouchPosition(void)
{
    Vector2 position = touchPosition;
/*
    if ((screenWidth > displayWidth) || (screenHeight > displayHeight))
    {
        // TODO: Seems to work ok but... review!
        position.x = position.x*((float)screenWidth / (float)(displayWidth - renderOffsetX)) - renderOffsetX/2;
        position.y = position.y*((float)screenHeight / (float)(displayHeight - renderOffsetY)) - renderOffsetY/2;
    }
    else
    {
        position.x = position.x*((float)renderWidth / (float)displayWidth) - renderOffsetX/2;
        position.y = position.y*((float)renderHeight / (float)displayHeight) - renderOffsetY/2;
    }
*/
    return position;
}

// Check if a gesture have been detected
bool IsGestureDetected(void)
{
/*
    if (currentGesture == GESTURE_DRAG)                 TraceLog(INFO, "DRAG");
    else if (currentGesture == GESTURE_TAP)             TraceLog(INFO, "TAP");
    else if (currentGesture == GESTURE_DOUBLETAP)       TraceLog(INFO, "DOUBLE");
    else if (currentGesture == GESTURE_HOLD)            TraceLog(INFO, "HOLD");
    else if (currentGesture == GESTURE_SWIPE_RIGHT)     TraceLog(INFO, "RIGHT");
    else if (currentGesture == GESTURE_SWIPE_UP)        TraceLog(INFO, "UP");
    else if (currentGesture == GESTURE_SWIPE_LEFT)      TraceLog(INFO, "LEFT");
    else if (currentGesture == GESTURE_SWIPE_DOWN)      TraceLog(INFO, "DOWN");
    else if (currentGesture == GESTURE_PINCH_IN)        TraceLog(INFO, "PINCH IN");
    else if (currentGesture == GESTURE_PINCH_OUT)       TraceLog(INFO, "PINCH OUT");
*/

    if (currentGesture != GESTURE_NONE) return true;
    else return false;
}

// Check gesture type
int GetGestureType(void)
{
    return currentGesture;
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

#if defined(PLATFORM_WEB)
// Init gestures system (web)
void InitGesturesSystem(void)
{
    // Init gestures system web (emscripten)
    
    // NOTE: Some code examples
    //emscripten_set_touchstart_callback(0, NULL, 1, Emscripten_HandleTouch);
    //emscripten_set_touchend_callback("#canvas", data, 0, Emscripten_HandleTouch);
    
    emscripten_set_touchstart_callback("#canvas", NULL, 1, EmscriptenInputCallback);
    emscripten_set_touchend_callback("#canvas", NULL, 1, EmscriptenInputCallback);
    emscripten_set_touchmove_callback("#canvas", NULL, 1, EmscriptenInputCallback);
    emscripten_set_touchcancel_callback("#canvas", NULL, 1, EmscriptenInputCallback);
}
#elif defined(PLATFORM_ANDROID)
// Init gestures system (android)
void InitGesturesSystem(struct android_app *app)
{
    app->onInputEvent = AndroidInputCallback;
    
    // TODO: Receive frameBuffer data: displayWidth/displayHeight, renderWidth/renderHeight, screenWidth/screenHeight
}
#endif

// Update gestures detected (must be called every frame)
void UpdateGestures(void)
{
    // NOTE: Gestures are processed through system callbacks on touch events
    
    if ((previousGesture == GESTURE_TAP) && (currentGesture == GESTURE_TAP)) currentGesture = GESTURE_HOLD;
    else if (currentGesture != GESTURE_HOLD) currentGesture = GESTURE_NONE;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
static void ProcessMotionEvent(GestureEvent event)
{
    // Resets
    dragVector = (Vector2){ 0, 0 };
    pinchDelta = 0;
    
    previousGesture = currentGesture;
    
    switch (gestureType)
    {
        case TYPE_MOTIONLESS: // Detect TAP, DOUBLE_TAP and HOLD events
        {
            if (event.action == DOWN)
            {
                if (event.pointCount > 1) InitPinchGesture(event.position[0], event.position[1]);
                else
                {
                    // Set the press position
                    initialTapPosition = event.position[0];
                    
                    // If too much time have passed, we reset the double tap
                    if (GetCurrentTime() - eventTime > TAP_TIMEOUT) untap = false;
                    
                    // If we are in time, we detect the double tap
                    if (untap) doubleTapping = true;
                    
                    // Update our event time
                    eventTime = GetCurrentTime();
                    
                    // Set hold
                    if (doubleTapping) currentGesture = GESTURE_DOUBLETAP;
                    else currentGesture = GESTURE_TAP;
                }
            }
            else if (event.action == UP)
            {
			    currentGesture = GESTURE_NONE;

                // Detect that we are tapping instead of holding
                if (GetCurrentTime() - eventTime < TAP_TIMEOUT)
                {
                    if (doubleTapping) untap = false;
                    else untap = true;
                }
                
                // Tap finished
                doubleTapping = false;

                // Update our event time
                eventTime = GetCurrentTime();
            }
            // Begin dragging
            else if (event.action == MOVE)
            {
                if (event.pointCount > 1) InitPinchGesture(event.position[0], event.position[1]);
                else
                {
                    // Set the drag starting position
                    initialDragPosition = initialTapPosition;
                    endDragPosition = initialDragPosition;
                    
                    // Initialize drag
                    draggingTimeCounter = 0;
                    gestureType = TYPE_DRAG;
                    currentGesture = GESTURE_NONE;
                }
            }
        } break;
        case TYPE_DRAG: // Detect DRAG and SWIPE events 
        {
            // end of the drag
            if (event.action == UP)
            {
                // Return Swipe if we have enough sensitivity
                if (intensity > FORCE_TO_SWIPE)
                {
                    if (angle < 30 || angle > 330) currentGesture = GESTURE_SWIPE_RIGHT; // Right
                    else if (angle > 60 && angle < 120) currentGesture = GESTURE_SWIPE_UP; // Up
                    else if (angle > 150 && angle < 210) currentGesture = GESTURE_SWIPE_LEFT; // Left
                    else if (angle > 240 && angle < 300) currentGesture = GESTURE_SWIPE_DOWN; // Down
                }
                
                magnitude = 0;
                angle = 0;
                intensity = 0;
                
                gestureType = TYPE_MOTIONLESS;
            }
            // Update while we are dragging
            else if (event.action == MOVE)
            {
                if (event.pointCount > 1) InitPinchGesture(event.position[0], event.position[1]);
                else
                {
                    lastDragPosition = endDragPosition;
                    
                    endDragPosition = touchPosition;
                    
                    //endDragPosition.x = AMotionEvent_getX(event, 0);
                    //endDragPosition.y = AMotionEvent_getY(event, 0);
                    
                    // Calculate attributes
                    dragVector = (Vector2){ endDragPosition.x - lastDragPosition.x, endDragPosition.y - lastDragPosition.y };
                    magnitude = sqrt(pow(endDragPosition.x - initialDragPosition.x, 2) + pow(endDragPosition.y - initialDragPosition.y, 2));
                    angle = CalculateAngle(initialDragPosition, endDragPosition, magnitude);
                    intensity = magnitude / (float)draggingTimeCounter;
                    
                    currentGesture = GESTURE_DRAG;
                    draggingTimeCounter++;
                }
            }
        } break;
        case TYPE_DUAL_INPUT:
        {
            if (event.action == UP)
            {
                if (event.pointCount == 1)
                {
                    // Set the drag starting position
                    initialTapPosition = event.position[0];
                }
                gestureType = TYPE_MOTIONLESS;
            }
            else if (event.action == MOVE)
            {
                // Adapt the ending position of the inputs
                firstEndPinchPosition = event.position[0];
                secondEndPinchPosition = event.position[1];
                
                // If there is no more than two inputs
                if (event.pointCount == 2)
                {
                    // Calculate distances
                    float initialDistance = VectorDistance(firstInitialPinchPosition, secondInitialPinchPosition);
                    float endDistance = VectorDistance(firstEndPinchPosition, secondEndPinchPosition);

                    // Calculate Vectors
                    Vector2 firstTouchVector = { firstEndPinchPosition.x - firstInitialPinchPosition.x, firstEndPinchPosition.y - firstInitialPinchPosition.y };
                    Vector2 secondTouchVector = { secondEndPinchPosition.x - secondInitialPinchPosition.x, secondEndPinchPosition.y - secondInitialPinchPosition.y };

                    // Detect the pinch gesture
                    if (VectorDotProduct(firstTouchVector, secondTouchVector) < -0.5) pinchDelta = initialDistance - endDistance;
                    else pinchDelta = 0;
                    
                    // Pinch gesture resolution
                    if (pinchDelta != 0)
                    {
                        if (pinchDelta > 0) currentGesture = GESTURE_PINCH_IN;
                        else currentGesture = GESTURE_PINCH_OUT;
                    }
                }
                else
                {
                    // Set the drag starting position
                    initialTapPosition = event.position[0];
                    
                    gestureType = TYPE_MOTIONLESS;
                }
                
                // Readapt the initial position of the inputs
                firstInitialPinchPosition = firstEndPinchPosition;
                secondInitialPinchPosition = secondEndPinchPosition;
            }
        } break;
    }
    //--------------------------------------------------------------------
}

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

static double GetCurrentTime()
{
    double time = 0;
#if defined(_WIN32)
/*
    // NOTE: Requires Windows.h
	FILETIME tm;
	GetSystemTimePreciseAsFileTime(&tm);
	ULONGLONG nowTime = ((ULONGLONG)tm.dwHighDateTime << 32) | (ULONGLONG)tm.dwLowDateTime;   // Time provided in 100-nanosecond intervals
    
	time = ((double)nowTime/10000000.0);    // time in seconds
*/
/*
    double pcFreq = 0.0;
    __int64 counterStart = 0;   // In C99 defined a standard 64-bit integer type named int64_t and unsigned version uint64_t in stdint.h.
    
    //int64_t or uint64_t is type defined as long long or unsigned long long in C99's stdint.h.
    
    //LARGE_INTEGER li; // Represents a 64-bit signed integer value
    //li.QuadPart       // A signed 64-bit integer
    
    unsigned long long int li;      // __int64, same as long long
    if(!QueryPerformanceFrequency(&li)) return 0;

    pcFreq = (double)(li)/1000.0;

    QueryPerformanceCounter(&li);
    counterStart = li;

    unsigned long long int tm;
    QueryPerformanceCounter(&tm);
    time = (double)(tm - counterStart)/pcFreq;
*/    
    unsigned long long int tm, tf;
    time = (double)(QueryPerformanceCounter(&tm)/QueryPerformanceFrequency(&tf));   // time in seconds
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

#if defined(PLATFORM_ANDROID)
// Android: Get input events
static int32_t AndroidInputCallback(struct android_app *app, AInputEvent *event)
{
    int type = AInputEvent_getType(event);

    if (type == AINPUT_EVENT_TYPE_MOTION)
    {
        touchPosition.x = AMotionEvent_getX(event, 0);
        touchPosition.y = AMotionEvent_getY(event, 0);
    }
    else if (type == AINPUT_EVENT_TYPE_KEY)
    {
        //int32_t key = AKeyEvent_getKeyCode(event);
        //int32_t AKeyEvent_getMetaState(event);
        
        //int32_t code = AKeyEvent_getKeyCode((const AInputEvent *)event);
        
        // If we are in active mode, we eat the back button and move into pause mode.  
        // If we are already in pause mode, we allow the back button to be handled by the OS, which means we'll be shut down.
        /*
        if ((code == AKEYCODE_BACK) && mActiveMode)
        {
            setActiveMode(false);
            return 1;
        }
        */
    }
    
    int32_t action = AMotionEvent_getAction(event);
    unsigned int flags = action & AMOTION_EVENT_ACTION_MASK;
    
    GestureEvent gestureEvent;
    
    // Action
    if (flags == AMOTION_EVENT_ACTION_DOWN) gestureEvent.action = DOWN;
    else if (flags == AMOTION_EVENT_ACTION_UP) gestureEvent.action = UP;
    else if (flags == AMOTION_EVENT_ACTION_MOVE) gestureEvent.action = MOVE;
    
    // Points
    gestureEvent.pointCount = AMotionEvent_getPointerCount(event);
    
    // Position
    gestureEvent.position[0] = (Vector2){ AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0) };
    gestureEvent.position[1] = (Vector2){ AMotionEvent_getX(event, 1), AMotionEvent_getY(event, 1) };
    
    ProcessMotionEvent(gestureEvent);

    return 0;   // return 1;
}
#endif

#if defined(PLATFORM_WEB)
// Web: Get input events
static EM_BOOL EmscriptenInputCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData)
{
    /*
    for (int i = 0; i < touchEvent->numTouches; i++)
    {
        long x, y, id;

        if (!touchEvent->touches[i].isChanged) continue;

        id = touchEvent->touches[i].identifier;
        x = touchEvent->touches[i].canvasX;
        y = touchEvent->touches[i].canvasY;
    }
    
    printf("%s, numTouches: %d %s%s%s%s\n", emscripten_event_type_to_string(eventType), event->numTouches,
           event->ctrlKey ? " CTRL" : "", event->shiftKey ? " SHIFT" : "", event->altKey ? " ALT" : "", event->metaKey ? " META" : "");

    for(int i = 0; i < event->numTouches; ++i)
    {
        const EmscriptenTouchPoint *t = &event->touches[i];
        
        printf("  %ld: screen: (%ld,%ld), client: (%ld,%ld), page: (%ld,%ld), isChanged: %d, onTarget: %d, canvas: (%ld, %ld)\n",
          t->identifier, t->screenX, t->screenY, t->clientX, t->clientY, t->pageX, t->pageY, t->isChanged, t->onTarget, t->canvasX, t->canvasY);
    }
    */
    GestureEvent gestureEvent;
    
    // Action
    if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART) gestureEvent.action = DOWN;
    else if (eventType == EMSCRIPTEN_EVENT_TOUCHEND) gestureEvent.action = UP;
    else if (eventType == EMSCRIPTEN_EVENT_TOUCHMOVE) gestureEvent.action = MOVE;
    
    // Points
    gestureEvent.pointCount = touchEvent->numTouches;
    
    // Position
    gestureEvent.position[0] = (Vector2){ touchEvent->touches[0].canvasX, touchEvent->touches[0].canvasY };
    gestureEvent.position[1] = (Vector2){ touchEvent->touches[1].canvasX, touchEvent->touches[1].canvasY };
    
    touchPosition = gestureEvent.position[0];
    
    ProcessMotionEvent(gestureEvent);

    return 1;
}
#endif