/**********************************************************************************************
*
*   rcore_ios - Functions to manage window, graphics device and inputs
*
*   PLATFORM: IOS
*       - iOS (arm64)
*       - iOS Simulator
*
*   LIMITATIONS:
*       - No keyboard input support
*       - No gamepad input support
*
*   DEPENDENCIES:
*       - UIKit/QuartzCore/OpenGLES (provided by iOS SDK)
*       - raylib_ios_main.m provides the native window and GL bridge
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2026 Ramon Santamaria (@raysan5) and contributors
*
**********************************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(__APPLE__)
    #include <unistd.h>
#endif

// UIKit/OpenGL bridge functions implemented in [rcore_ios_main.m]
RLAPI bool ios_initialize_window(int requestedWidth, int requestedHeight, int *screenWidth, int *screenHeight, int *renderWidth, int *renderHeight, float *scaleX, float *scaleY);
RLAPI void ios_close_platform(void);
RLAPI void ios_make_current_context(void);
RLAPI void ios_present_frame(void);
RLAPI void *ios_get_window_handle(void);
RLAPI void ios_get_window_metrics(int *screenWidth, int *screenHeight, int *renderWidth, int *renderHeight, float *scaleX, float *scaleY);
RLAPI void *ios_get_proc_address(const char *name);
RLAPI void ios_open_url(const char *url);
RLAPI void ios_set_target_fps(float fps);

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    bool initialized;
    struct {
        // TODO: Use CORE.Input.Touch data... why is it required to duplicate this data?
        int32_t pointCount;                     // Number of active touch points
        int32_t pointId[MAX_TOUCH_POINTS];      // Touch point ids
        Vector2 position[MAX_TOUCH_POINTS];     // Touch point positions
        int32_t hoverPoints[MAX_TOUCH_POINTS];  // Hover point slots
    } TouchRaw;
} PlatformData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern CoreData CORE;                       // Global CORE state context

static PlatformData platform = { 0 };       // Platform specific data

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
int InitPlatform(void);                     // Initialize platform (graphics, inputs and more)
bool InitGraphicsDevice(void);              // Initialize graphics device

static int FindTouchSlot(intptr_t touchId, bool allowAllocate);
static void UpdateMouseFromTouches(void);

//----------------------------------------------------------------------------------
// Module Functions Definition: Window and Graphics Device
//----------------------------------------------------------------------------------

// Check if application should close
bool WindowShouldClose(void)
{
    if (CORE.Window.ready) return CORE.Window.shouldClose;
    else return true;
}

// Toggle fullscreen mode
void ToggleFullscreen(void)
{
    TRACELOG(LOG_WARNING, "ToggleFullscreen() not available on target platform");
}

// Toggle borderless windowed mode
void ToggleBorderlessWindowed(void)
{
    TRACELOG(LOG_WARNING, "ToggleBorderlessWindowed() not available on target platform");
}

// Set window state: maximized, if resizable
void MaximizeWindow(void)
{
    TRACELOG(LOG_WARNING, "MaximizeWindow() not available on target platform");
}

// Set window state: minimized
void MinimizeWindow(void)
{
    TRACELOG(LOG_WARNING, "MinimizeWindow() not available on target platform");
}

// Restore window from being minimized/maximized
void RestoreWindow(void)
{
    TRACELOG(LOG_WARNING, "RestoreWindow() not available on target platform");
}

// Set window configuration state using flags
void SetWindowState(unsigned int flags)
{
    TRACELOG(LOG_WARNING, "SetWindowState() not available on target platform");
}

// Clear window configuration state flags
void ClearWindowState(unsigned int flags)
{
    TRACELOG(LOG_WARNING, "ClearWindowState() not available on target platform");
}

// Set icon for window
void SetWindowIcon(Image image)
{
    TRACELOG(LOG_WARNING, "SetWindowIcon() not available on target platform");
}

// Set icons for window
void SetWindowIcons(Image *images, int count)
{
    TRACELOG(LOG_WARNING, "SetWindowIcons() not available on target platform");
}

// Set title for window
void SetWindowTitle(const char *title)
{
    CORE.Window.title = title;
}

// Set window position on screen (windowed mode)
void SetWindowPosition(int x, int y)
{
    TRACELOG(LOG_WARNING, "SetWindowPosition() not available on target platform");
}

// Set monitor for the current window
void SetWindowMonitor(int monitor)
{
    TRACELOG(LOG_WARNING, "SetWindowMonitor() not available on target platform");
}

// Set window minimum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMinSize(int width, int height)
{
    CORE.Window.screenMin.width = width;
    CORE.Window.screenMin.height = height;
}

// Set window maximum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMaxSize(int width, int height)
{
    CORE.Window.screenMax.width = width;
    CORE.Window.screenMax.height = height;
}

// Set window dimensions
void SetWindowSize(int width, int height)
{
    TRACELOG(LOG_WARNING, "SetWindowSize() not available on target platform");
}

// Set window opacity [0.0f..1.0f]
void SetWindowOpacity(float opacity)
{
    TRACELOG(LOG_WARNING, "SetWindowOpacity() not available on target platform");
}

// Set window focused
void SetWindowFocused(void)
{
    TRACELOG(LOG_WARNING, "SetWindowFocused() not available on target platform");
}

// Get native window handle
void *GetWindowHandle(void)
{
    return ios_get_window_handle();
}

// Get number of monitors
int GetMonitorCount(void) { return 1; }

// Get current monitor where window is placed
int GetCurrentMonitor(void) { return 0; }

// Get selected monitor position
Vector2 GetMonitorPosition(int monitor) { return (Vector2){ 0, 0 }; }

// Get selected monitor width (currently used by monitor)
int GetMonitorWidth(int monitor) { return CORE.Window.screen.width; }

// Get selected monitor height (currently used by monitor)
int GetMonitorHeight(int monitor) { return CORE.Window.screen.height; }

// Get selected monitor physical width in millimetres
int GetMonitorPhysicalWidth(int monitor) { return 0; }

// Get selected monitor physical height in millimetres
int GetMonitorPhysicalHeight(int monitor) { return 0; }

// Get selected monitor refresh rate
int GetMonitorRefreshRate(int monitor) { return 0; }

// Get the human-readable, UTF-8 encoded name of the selected monitor
const char *GetMonitorName(int monitor) { return ""; }

// Get window position XY on monitor
Vector2 GetWindowPosition(void) { return (Vector2){ 0, 0 }; }

// Get window scale DPI factor for current monitor
Vector2 GetWindowScaleDPI(void)
{
    int screenWidth = 0, screenHeight = 0;
    int renderWidth = 0, renderHeight = 0;
    float scaleX = 1.0f, scaleY = 1.0f;

    ios_get_window_metrics(&screenWidth, &screenHeight, &renderWidth, &renderHeight, &scaleX, &scaleY);
    
    (void)screenWidth; (void)screenHeight;
    (void)renderWidth; (void)renderHeight;

    return (Vector2){ scaleX, scaleY };
}

// Set clipboard text content
void SetClipboardText(const char *text)
{
    TRACELOG(LOG_WARNING, "SetClipboardText() not implemented on target platform");
}

// Get clipboard text content
const char *GetClipboardText(void)
{
    TRACELOG(LOG_WARNING, "GetClipboardText() not implemented on target platform");
    return NULL;
}

// Get clipboard image
Image GetClipboardImage(void)
{
    Image image = { 0 };
    TRACELOG(LOG_WARNING, "GetClipboardImage() not implemented on target platform");
    return image;
}

// Show mouse cursor
void ShowCursor(void) { CORE.Input.Mouse.cursorHidden = false; }

// Hide mouse cursor
void HideCursor(void) { CORE.Input.Mouse.cursorHidden = true; }

// Enable cursor (unlock cursor)
void EnableCursor(void)
{
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);
    CORE.Input.Mouse.cursorHidden = false;
}

// Disable cursor (lock cursor)
void DisableCursor(void)
{
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);
    CORE.Input.Mouse.cursorHidden = true;
}

// Swap back buffer with front buffer (screen drawing)
void SwapScreenBuffer(void)
{
    if (CORE.Time.target > 0) ios_set_target_fps((float)(1.0/CORE.Time.target));
    
    ios_present_frame();
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Misc
//----------------------------------------------------------------------------------

// Get elapsed time measure in seconds since InitTimer()
double GetTime(void)
{
    struct timespec ts = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &ts);
    unsigned long long int nanoSeconds = (unsigned long long int)ts.tv_sec*1000000000LLU + (unsigned long long int)ts.tv_nsec;

    return (double)(nanoSeconds - CORE.Time.base)*1e-9;  // Elapsed time since InitTimer()
}

// Open URL with default system browser (if available)
// NOTE: This function is only safe to use if you control the URL given.
// A user could craft a malicious string performing another action.
// Only call this function yourself not with user input or make sure to check
// the string yourself. Ref: https://github.com/raysan5/raylib/issues/686
void OpenURL(const char *url)
{
    // Security check to (partially) avoid malicious code
    // TODO: Security concerns, this cod should b reviewed (not only for iOS)
    if (strchr(url, '\'') != NULL) TRACELOG(LOG_WARNING, "SYSTEM: Provided URL could be potentially malicious, avoid [\'] character");
    else ios_open_url(url);
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Inputs
//----------------------------------------------------------------------------------

// Set internal gamepad mappings
int SetGamepadMappings(const char *mappings)
{
    TRACELOG(LOG_WARNING, "SetGamepadMappings() not implemented on target platform");
    return 0;
}

// Set gamepad vibration
void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor, float duration)
{
    TRACELOG(LOG_WARNING, "SetGamepadVibration() not implemented on target platform");
}

// Set mouse position XY
void SetMousePosition(int x, int y)
{
    CORE.Input.Mouse.currentPosition = (Vector2){ (float)x, (float)y };
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
}

// Set mouse cursor
void SetMouseCursor(int cursor)
{
    TRACELOG(LOG_WARNING, "SetMouseCursor() not implemented on target platform");
}

// Get physical key name
const char *GetKeyName(int key)
{
    TRACELOG(LOG_WARNING, "GetKeyName() not implemented on target platform");
    return "";
}

// Register all input events
void PollInputEvents(void)
{
#if defined(SUPPORT_GESTURES_SYSTEM)
    // NOTE: Gestures update must be called every frame to reset gestures correctly
    // because ProcessGestureEvent() is called on an event, not every frame
    UpdateGestures();
#endif

    // Reset keys/chars pressed registered
    CORE.Input.Keyboard.keyPressedQueueCount = 0;
    CORE.Input.Keyboard.charPressedQueueCount = 0;

    // Reset key repeats
    for (int i = 0; i < MAX_KEYBOARD_KEYS; i++) CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;

    // Reset last gamepad button/axis registered state
    CORE.Input.Gamepad.lastButtonPressed = 0; // GAMEPAD_BUTTON_UNKNOWN

    // Register previous touch states
    for (int i = 0; i < MAX_TOUCH_POINTS; i++)
        CORE.Input.Touch.previousTouchState[i] = CORE.Input.Touch.currentTouchState[i];

    // Register previous keys states
    for (int i = 0; i < MAX_KEYBOARD_KEYS; i++)
    {
        CORE.Input.Keyboard.previousKeyState[i] = CORE.Input.Keyboard.currentKeyState[i];
        CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }

    UpdateMouseFromTouches();
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

// Initialize platform: graphics, inputs and more
int InitPlatform(void)
{
    FLAG_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE);

    if (FLAG_IS_SET(CORE.Window.flags, FLAG_MSAA_4X_HINT)) TRACELOG(LOG_INFO, "DISPLAY: Trying to enable MSAA x4");

    int screenWidth = 0, screenHeight = 0;
    int renderWidth = 0, renderHeight = 0;
    float scaleX = 1.0f, scaleY = 1.0f;

    if (!ios_initialize_window(CORE.Window.screen.width, CORE.Window.screen.height,
        &screenWidth, &screenHeight, &renderWidth, &renderHeight, &scaleX, &scaleY))
    {
        TRACELOG(LOG_FATAL, "PLATFORM: Failed to initialize iOS window and graphics context");
        return -1;
    }

    ios_make_current_context();

    CORE.Window.screen.width = screenWidth;
    CORE.Window.screen.height = screenHeight;
    CORE.Window.display.width = (int)((float)screenWidth*scaleX);
    CORE.Window.display.height = (int)((float)screenHeight*scaleY);
    CORE.Window.render.width = renderWidth;
    CORE.Window.render.height = renderHeight;
    CORE.Window.currentFbo.width = CORE.Window.render.width;
    CORE.Window.currentFbo.height = CORE.Window.render.height;
    CORE.Window.screenScale = MatrixScale(scaleX, scaleY, 1.0f);

    rlLoadExtensions(ios_get_proc_address);

    for (int i = 0; i < MAX_TOUCH_POINTS; i++) platform.TouchRaw.hoverPoints[i] = -1;

    platform.initialized = true;
    CORE.Window.ready = true;

    InitTimer();
    CORE.Storage.basePath = GetWorkingDirectory();

    TRACELOG(LOG_INFO, "PLATFORM: IOS: Initialized successfully");
    TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
    TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);

    return 0;
}

// Close platform
void ClosePlatform(void)
{
    ios_close_platform();

    CORE = (CoreData){ 0 };
    platform = (PlatformData){ 0 };
    platform.TouchRaw = (TouchRaw){ 0 };
}

// Find or allocate a touch slot for the given touchId
static int FindTouchSlot(intptr_t touchId, bool allowAllocate)
{
    for (int i = 0; i < platform.TouchRaw.pointCount; i++)
    {
        if (platform.TouchRaw.pointId[i] == (int32_t)touchId) return i;
    }

    if (!allowAllocate) return -1;
    if (platform.TouchRaw.pointCount < MAX_TOUCH_POINTS) return platform.TouchRaw.pointCount;

    return -1;
}

// Sync mouse position/state from primary touch point
static void UpdateMouseFromTouches(void)
{
    if (platform.TouchRaw.pointCount > 0)
    {
        CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
        CORE.Input.Mouse.currentPosition = platform.TouchRaw.position[0];
        CORE.Input.Mouse.currentWheelMove = (Vector2){ 0.0f, 0.0f };
    }
}

//----------------------------------------------------------------------------------
// iOS Bridge Callbacks (called from rcore_ios_main.m)
//----------------------------------------------------------------------------------

void ios_handle_touch_began(intptr_t touchId, float x, float y)
{
    int index = FindTouchSlot(touchId, true);
    if (index < 0) return;

    platform.TouchRaw.pointId[index] = (int32_t)touchId;
    platform.TouchRaw.position[index] = (Vector2){ x, y };

    CORE.Input.Touch.pointId[index] = (int)touchId;
    CORE.Input.Touch.position[index] = (Vector2){ x, y };
    CORE.Input.Touch.currentTouchState[index] = 1;
    CORE.Input.Touch.previousTouchState[index] = 0;

    CORE.Input.Mouse.currentButtonState[MOUSE_BUTTON_LEFT] = 1;
    CORE.Input.Mouse.previousButtonState[MOUSE_BUTTON_LEFT] = 0;

    if (platform.TouchRaw.pointCount < MAX_TOUCH_POINTS) platform.TouchRaw.pointCount++;

    CORE.Input.Touch.pointCount = platform.TouchRaw.pointCount;

    UpdateMouseFromTouches();
}

void ios_handle_touch_moved(intptr_t touchId, float x, float y)
{
    int index = FindTouchSlot(touchId, false);
    if (index < 0) return;

    platform.TouchRaw.position[index]                  = (Vector2){ x, y };
    CORE.Input.Touch.pointId[index]           = (int)touchId;
    CORE.Input.Touch.position[index]          = (Vector2){ x, y };
    CORE.Input.Touch.currentTouchState[index] = 1;
    CORE.Input.Mouse.currentButtonState[MOUSE_BUTTON_LEFT] = 1;
    CORE.Input.Touch.pointCount = platform.TouchRaw.pointCount;

    UpdateMouseFromTouches();
}

void ios_handle_touch_ended(intptr_t touchId, float x, float y)
{
    int index = FindTouchSlot(touchId, false);
    if (index < 0) return;

    platform.TouchRaw.position[index] = (Vector2){ x, y };
    CORE.Input.Touch.position[index] = (Vector2){ x, y };
    CORE.Input.Touch.currentTouchState[index] = 0;
    CORE.Input.Touch.previousTouchState[index] = 1;
    CORE.Input.Touch.pointId[index] = 0;

    // Compact the touch arrays — shift remaining points down
    for (int i = index; i < platform.TouchRaw.pointCount - 1; i++)
    {
        platform.TouchRaw.pointId[i] = platform.TouchRaw.pointId[i + 1];
        platform.TouchRaw.position[i] = platform.TouchRaw.position[i + 1];

        CORE.Input.Touch.pointId[i] = CORE.Input.Touch.pointId[i + 1];
        CORE.Input.Touch.position[i] = CORE.Input.Touch.position[i + 1];
        CORE.Input.Touch.currentTouchState[i] = CORE.Input.Touch.currentTouchState[i + 1];
        CORE.Input.Touch.previousTouchState[i] = CORE.Input.Touch.previousTouchState[i + 1];
    }

    if (platform.TouchRaw.pointCount > 0) platform.TouchRaw.pointCount--;
    CORE.Input.Touch.pointCount = platform.TouchRaw.pointCount;

    CORE.Input.Mouse.currentButtonState[MOUSE_BUTTON_LEFT] = (platform.TouchRaw.pointCount > 0)? 1 : 0;
    CORE.Input.Mouse.previousButtonState[MOUSE_BUTTON_LEFT] = 1;

    UpdateMouseFromTouches();
}

void ios_handle_touch_cancelled(intptr_t touchId, float x, float y)
{
    ios_handle_touch_ended(touchId, x, y);
}

void ios_request_close(void)
{
    CORE.Window.shouldClose = true;
}

void ios_set_window_focused(bool focused)
{
    if (focused) FLAG_CLEAR(CORE.Window.flags, FLAG_WINDOW_UNFOCUSED);
    else FLAG_SET(CORE.Window.flags, FLAG_WINDOW_UNFOCUSED);
}