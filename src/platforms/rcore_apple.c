/**********************************************************************************************
*
*   rcore_apple - Functions to manage the graphics device and inputs on iOS and tvOS
*
*   This software is provided under the zlib/libpng license. See LICENSE for details.
*
**********************************************************************************************/

#include "rcore_apple.h"

static AppleFrame appleFrame = { 0 };
static bool appleNeedsFrame = false;

static Size AppleViewportSize(void)
{
    return (Size){ CORE.Window.render.width - CORE.Window.renderOffset.x,
                   CORE.Window.render.height - CORE.Window.renderOffset.y };
}

static void AppleUpdateViewport(void)
{
    float scale = fminf((float)appleFrame.width/CORE.Window.screen.width,
                        (float)appleFrame.height/CORE.Window.screen.height);
    int width = (int)fmaxf(1, roundf(CORE.Window.screen.width*scale));
    int height = (int)fmaxf(1, roundf(CORE.Window.screen.height*scale));
    CORE.Window.renderOffset = (Point){ appleFrame.width - width, appleFrame.height - height };
    CORE.Window.render = (Size){ appleFrame.width, appleFrame.height };
    CORE.Window.currentFbo = AppleViewportSize();
}

static void AppleUpdateTouch(AppleTouchEvent touch)
{
    int count = CORE.Input.Touch.pointCount;
    int index = 0;
    while ((index < count) && (CORE.Input.Touch.pointId[index] != touch.id)) index++;
    if (index == count)
    {
        if ((touch.action != APPLE_TOUCH_DOWN) || (count >= MAX_TOUCH_POINTS)) return;
        CORE.Input.Touch.pointId[index] = touch.id;
        CORE.Input.Touch.currentTouchState[index] = 1;
        count++;
        CORE.Input.Touch.pointCount = count;
    }

    Size viewport = AppleViewportSize();
    Vector2 position = {
        (touch.x*appleFrame.width/appleFrame.pointWidth - CORE.Window.renderOffset.x/2)*CORE.Window.screen.width/viewport.width,
        (touch.y*appleFrame.height/appleFrame.pointHeight - (CORE.Window.renderOffset.y + 1)/2)*CORE.Window.screen.height/viewport.height
    };
    CORE.Input.Touch.position[index] = position;
    if (index == 0) CORE.Input.Mouse.currentPosition = position;

#if SUPPORT_GESTURES_SYSTEM
    GestureEvent event = { .touchAction = touch.action, .pointCount = count };
    for (int i = 0; i < count; i++)
    {
        event.pointId[i] = CORE.Input.Touch.pointId[i];
        event.position[i] = (Vector2){ CORE.Input.Touch.position[i].x/CORE.Window.screen.width,
                                       CORE.Input.Touch.position[i].y/CORE.Window.screen.height };
    }
    ProcessGestureEvent(event);
#endif

    if ((touch.action == APPLE_TOUCH_UP) || (touch.action == APPLE_TOUCH_CANCEL))
    {
        for (int i = index; i + 1 < count; i++)
        {
            CORE.Input.Touch.position[i] = CORE.Input.Touch.position[i + 1];
            CORE.Input.Touch.pointId[i] = CORE.Input.Touch.pointId[i + 1];
        }
        CORE.Input.Touch.pointId[count - 1] = -1;
        CORE.Input.Touch.currentTouchState[count - 1] = 0;
        CORE.Input.Touch.pointCount--;
    }

    CORE.Input.Mouse.currentButtonState[MOUSE_BUTTON_LEFT] = CORE.Input.Touch.pointCount > 0;
    if (CORE.Input.Touch.pointCount > 0) CORE.Input.Mouse.currentPosition = CORE.Input.Touch.position[0];
}

void PollInputEvents(void)
{
#if SUPPORT_GESTURES_SYSTEM
    UpdateGestures();
#endif
    CORE.Input.Keyboard.keyPressedQueueCount = 0;
    CORE.Input.Keyboard.charPressedQueueCount = 0;
    CORE.Input.Gamepad.lastButtonPressed = GAMEPAD_BUTTON_UNKNOWN;
    memcpy(CORE.Input.Keyboard.previousKeyState, CORE.Input.Keyboard.currentKeyState, sizeof(CORE.Input.Keyboard.currentKeyState));
    memcpy(CORE.Input.Mouse.previousButtonState, CORE.Input.Mouse.currentButtonState, sizeof(CORE.Input.Mouse.currentButtonState));
    memcpy(CORE.Input.Touch.previousTouchState, CORE.Input.Touch.currentTouchState, sizeof(CORE.Input.Touch.currentTouchState));
    memcpy(CORE.Input.Gamepad.previousButtonState, CORE.Input.Gamepad.currentButtonState, sizeof(CORE.Input.Gamepad.currentButtonState));
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;

    if (appleFrame.resetInput)
    {
        memset(CORE.Input.Touch.currentTouchState, 0, sizeof(CORE.Input.Touch.currentTouchState));
        for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.pointId[i] = -1;
        CORE.Input.Touch.pointCount = 0;
        CORE.Input.Mouse.currentButtonState[MOUSE_BUTTON_LEFT] = 0;
#if SUPPORT_GESTURES_SYSTEM
        ProcessGestureEvent((GestureEvent){ .touchAction = TOUCH_ACTION_CANCEL });
#endif
        appleFrame.resetInput = false;
    }

    AppleTouchEvent touch = { 0 };
    while (AppleNextTouch(&touch)) AppleUpdateTouch(touch);

    const int remoteKeys[APPLE_REMOTE_KEYS] = { KEY_UP, KEY_RIGHT, KEY_DOWN, KEY_LEFT, KEY_ENTER, KEY_SPACE };
    for (int i = 0; i < APPLE_REMOTE_KEYS; i++)
    {
        int key = remoteKeys[i];
        CORE.Input.Keyboard.currentKeyState[key] = appleFrame.remoteKeys[i];
        if (CORE.Input.Keyboard.currentKeyState[key] && !CORE.Input.Keyboard.previousKeyState[key])
        {
            if (CORE.Input.Keyboard.keyPressedQueueCount < MAX_KEY_PRESSED_QUEUE)
                CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount++] = key;
            if (key == CORE.Input.Keyboard.exitKey) CORE.Window.shouldClose = true;
        }
    }

    for (int i = 0; (i < APPLE_GAMEPADS) && (i < MAX_GAMEPADS); i++)
    {
        const AppleGamepad *gamepad = &appleFrame.gamepads[i];
        CORE.Input.Gamepad.ready[i] = gamepad->connected;
        CORE.Input.Gamepad.axisCount[i] = gamepad->connected? ((APPLE_GAMEPAD_AXES < MAX_GAMEPAD_AXES)? APPLE_GAMEPAD_AXES : MAX_GAMEPAD_AXES) : 0;
        snprintf(CORE.Input.Gamepad.name[i], MAX_GAMEPAD_NAME_LENGTH, "%s", gamepad->name);
        for (int axis = 0; axis < CORE.Input.Gamepad.axisCount[i]; axis++) CORE.Input.Gamepad.axisState[i][axis] = gamepad->axes[axis];
        for (int button = 1; (button < APPLE_GAMEPAD_BUTTONS) && (button < MAX_GAMEPAD_BUTTONS); button++)
        {
            CORE.Input.Gamepad.currentButtonState[i][button] = gamepad->buttons[button];
            if (gamepad->buttons[button] && !CORE.Input.Gamepad.previousButtonState[i][button]) CORE.Input.Gamepad.lastButtonPressed = button;
        }
    }
}

bool WindowShouldClose(void)
{
    if (!CORE.Window.ready || CORE.Window.shouldClose) return true;
    if (appleNeedsFrame)
    {
        AppleFrame next = { 0 };
        if (!AppleWaitFrame(appleFrame.timestamp + CORE.Time.target, &next))
        {
            CORE.Window.shouldClose = true;
            return true;
        }

        CORE.Window.resizedLastFrame = (next.width != appleFrame.width) || (next.height != appleFrame.height) ||
                                       (next.pointWidth != appleFrame.pointWidth) || (next.pointHeight != appleFrame.pointHeight);
        CORE.Time.frame = next.interval;
        appleFrame = next;
        CORE.Window.display = (Size){ next.displayWidth, next.displayHeight };
        rlAppleDrawableFramebuffer = next.framebuffer;
        if (CORE.Window.resizedLastFrame)
        {
            AppleUpdateViewport();
            SetupViewport(next.width, next.height);
            rlSetFramebufferWidth(next.width);
            rlSetFramebufferHeight(next.height);
        }
        appleNeedsFrame = false;
#if !SUPPORT_CUSTOM_FRAME_CONTROL
        PollInputEvents();
#endif
    }

    return CORE.Window.shouldClose;
}

void SwapScreenBuffer(void)
{
    ApplePresentFrame();
    appleNeedsFrame = true;
}

double GetTime(void)
{
    return (double)(clock_gettime_nsec_np(CLOCK_MONOTONIC) - CORE.Time.base)*1e-9;
}

int InitPlatform(void)
{
    if (!AppleWaitFrame(0.0, &appleFrame))
    {
        TRACELOG(LOG_WARNING, "APPLE: Could not start an OpenGL ES frame. Launch the game with RaylibRunAppleApp()");
        return -1;
    }

    if (CORE.Window.screen.width < 1) CORE.Window.screen.width = (int)appleFrame.pointWidth;
    if (CORE.Window.screen.height < 1) CORE.Window.screen.height = (int)appleFrame.pointHeight;
    CORE.Window.display = (Size){ appleFrame.displayWidth, appleFrame.displayHeight };
    CORE.Window.flags = FLAG_FULLSCREEN_MODE;
    AppleUpdateViewport();
    rlAppleDrawableFramebuffer = appleFrame.framebuffer;
    rlLoadExtensions(NULL);
    CORE.Time.base = clock_gettime_nsec_np(CLOCK_MONOTONIC);
    InitTimer();
    CORE.Storage.basePath = GetWorkingDirectory();
    CORE.Window.ready = true;
    appleNeedsFrame = true;
    for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.pointId[i] = -1;
    return 0;
}

void ClosePlatform(void)
{
    rlAppleDrawableFramebuffer = 0;
    appleNeedsFrame = false;
}

// UIKit and the system manage the native window
void ToggleFullscreen(void) { }
void ToggleBorderlessWindowed(void) { }
void MaximizeWindow(void) { }
void MinimizeWindow(void) { }
void RestoreWindow(void) { }
void SetWindowState(unsigned int flags) { (void)flags; }
void ClearWindowState(unsigned int flags) { (void)flags; }
void SetWindowIcon(Image image) { (void)image; }
void SetWindowIcons(Image *images, int count) { (void)images; (void)count; }
void SetWindowTitle(const char *title) { (void)title; }
void SetWindowPosition(int x, int y) { (void)x; (void)y; }
void SetWindowMonitor(int monitor) { (void)monitor; }
void SetWindowMinSize(int width, int height) { (void)width; (void)height; }
void SetWindowMaxSize(int width, int height) { (void)width; (void)height; }
void SetWindowOpacity(float opacity) { (void)opacity; }
void SetWindowFocused(void) { }
void *GetWindowHandle(void) { return appleFrame.window; }

void SetWindowSize(int width, int height)
{
    if (!CORE.Window.ready || (width < 1) || (height < 1)) return;
    CORE.Window.screen = (Size){ width, height };
    AppleUpdateViewport();
    SetupViewport(appleFrame.width, appleFrame.height);
    CORE.Window.resizedLastFrame = true;
}

int GetMonitorCount(void) { return CORE.Window.ready? 1 : 0; }
int GetCurrentMonitor(void) { return 0; }
Vector2 GetMonitorPosition(int monitor) { (void)monitor; return (Vector2){ 0 }; }
int GetMonitorWidth(int monitor) { return (monitor == 0)? appleFrame.displayWidth : 0; }
int GetMonitorHeight(int monitor) { return (monitor == 0)? appleFrame.displayHeight : 0; }
int GetMonitorPhysicalWidth(int monitor) { (void)monitor; return 0; }
int GetMonitorPhysicalHeight(int monitor) { (void)monitor; return 0; }
int GetMonitorRefreshRate(int monitor) { return (monitor == 0)? appleFrame.refreshRate : 0; }
const char *GetMonitorName(int monitor) { return (monitor == 0)? "Apple display" : ""; }
Vector2 GetWindowPosition(void) { return (Vector2){ 0 }; }

Vector2 GetWindowScaleDPI(void)
{
    if (!CORE.Window.ready) return (Vector2){ 1.0f, 1.0f };
    Size viewport = AppleViewportSize();
    return (Vector2){ (float)viewport.width/CORE.Window.screen.width, (float)viewport.height/CORE.Window.screen.height };
}

void SetClipboardText(const char *text) { (void)text; }
const char *GetClipboardText(void) { return NULL; }
Image GetClipboardImage(void) { return (Image){ 0 }; }
void ShowCursor(void) { CORE.Input.Mouse.cursorHidden = false; }
void HideCursor(void) { CORE.Input.Mouse.cursorHidden = true; }
void EnableCursor(void) { ShowCursor(); }
void DisableCursor(void) { HideCursor(); }
void SetMouseCursor(int cursor) { (void)cursor; }

void SetMousePosition(int x, int y)
{
    CORE.Input.Mouse.currentPosition = (Vector2){ x, y };
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
}

const char *GetKeyName(int key)
{
    static char name[2] = { 0 };
    name[0] = ((key >= 32) && (key <= 126))? (char)key : '\0';
    return name;
}

int SetGamepadMappings(const char *mappings) { (void)mappings; return 0; }
void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor, float duration)
{
    (void)gamepad;
    (void)leftMotor;
    (void)rightMotor;
    (void)duration;
}

void OpenURL(const char *url)
{
    if ((url != NULL) && ((strncmp(url, "https://", 8) == 0) || (strncmp(url, "http://", 7) == 0))) AppleOpenURL(url);
}
