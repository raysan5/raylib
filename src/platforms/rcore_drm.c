/**********************************************************************************************
*
*   rcore_drm - Functions to manage window, graphics device and inputs
*
*   PLATFORM: DRM
*       - Raspberry Pi 0-5 (DRM/KMS)
*       - Linux DRM subsystem (KMS mode)
*
*   LIMITATIONS:
*       - Most of the window/monitor functions are not implemented (not required)
*
*   POSSIBLE IMPROVEMENTS:
*       - Improvement 01
*       - Improvement 02
*
*   ADDITIONAL NOTES:
*       - TRACELOG() function is located in raylib [utils] module
*
*   CONFIGURATION:
*       #define SUPPORT_SSH_KEYBOARD_RPI (Raspberry Pi only)
*           Reconfigure standard input to receive key inputs, works with SSH connection.
*           WARNING: Reconfiguring standard input could lead to undesired effects, like breaking other
*           running processes orblocking the device if not restored properly. Use with care.
*
*   DEPENDENCIES:
*       - DRM and GLM: System libraries for display initialization and configuration
*       - gestures: Gestures system for touch-ready devices (or simulated from mouse inputs)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2024 Ramon Santamaria (@raysan5) and contributors
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

#include <fcntl.h>   // POSIX file control definitions - open(), creat(), fcntl()
#include <unistd.h>  // POSIX standard function definitions - read(), close(), STDIN_FILENO
#include <termios.h> // POSIX terminal control definitions - tcgetattr(), tcsetattr()
#include <pthread.h> // POSIX threads management (inputs reading)
#include <dirent.h>  // POSIX directory browsing

#include <sys/ioctl.h>      // Required for: ioctl() - UNIX System call for device-specific input/output operations
#include <linux/kd.h>       // Linux: KDSKBMODE, K_MEDIUMRAM constants definition
#include <linux/input.h>    // Linux: Keycodes constants definition (KEY_A, ...)
#include <linux/joystick.h> // Linux: Joystick support library

// WARNING: Both 'linux/input.h' and 'raylib.h' define KEY_F12
// To avoid conflict with the capturing code in rcore.c we undefine the macro KEY_F12,
// so the enum KEY_F12 from raylib is used
#undef KEY_F12

#include <gbm.h>            // Generic Buffer Management (native platform for EGL on DRM)
#include <xf86drm.h>        // Direct Rendering Manager user-level library interface
#include <xf86drmMode.h>    // Direct Rendering Manager mode setting (KMS) interface

#include "EGL/egl.h"        // Native platform windowing system interface
#include "EGL/eglext.h"     // EGL extensions

#ifndef EGL_OPENGL_ES3_BIT
    #define EGL_OPENGL_ES3_BIT  0x40
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define USE_LAST_TOUCH_DEVICE       // When multiple touchscreens are connected, only use the one with the highest event<N> number

#define DEFAULT_EVDEV_PATH       "/dev/input/"      // Path to the linux input events

// So actually the biggest key is KEY_CNT but we only really map the keys up to
// KEY_ALS_TOGGLE
#define KEYMAP_SIZE KEY_ALS_TOGGLE

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

typedef struct {
    // Display data
    int fd;                             // File descriptor for /dev/dri/...
    drmModeConnector *connector;        // Direct Rendering Manager (DRM) mode connector
    drmModeCrtc *crtc;                  // CRT Controller
    int modeIndex;                      // Index of the used mode of connector->modes
    struct gbm_device *gbmDevice;       // GBM device
    struct gbm_surface *gbmSurface;     // GBM surface
    struct gbm_bo *prevBO;              // Previous GBM buffer object (during frame swapping)
    uint32_t prevFB;                    // Previous GBM framebufer (during frame swapping)

    EGLDisplay device;                  // Native display device (physical screen connection)
    EGLSurface surface;                 // Surface to draw on, framebuffers (connected to context)
    EGLContext context;                 // Graphic context, mode in which drawing can be done
    EGLConfig config;                   // Graphic config

    // Keyboard data
    int defaultKeyboardMode;            // Default keyboard mode
    bool eventKeyboardMode;             // Keyboard in event mode
    int defaultFileFlags;               // Default IO file flags
    struct termios defaultSettings;     // Default keyboard settings
    int keyboardFd;                     // File descriptor for the evdev keyboard

    // Mouse data
    Vector2 eventWheelMove;             // Registers the event mouse wheel variation
    // NOTE: currentButtonState[] can't be written directly due to multithreading, app could miss the update
    char currentButtonStateEvdev[MAX_MOUSE_BUTTONS]; // Holds the new mouse state for the next polling event to grab
    bool cursorRelative;                // Relative cursor mode
    int mouseFd;                        // File descriptor for the evdev mouse/touch/gestures
    Rectangle absRange;                 // Range of values for absolute pointing devices (touchscreens)
    int touchSlot;                      // Hold the touch slot number of the currently being sent multitouch block

    // Gamepad data
    int gamepadStreamFd[MAX_GAMEPADS];  // Gamepad device file descriptor
    int gamepadAbsAxisRange[MAX_GAMEPADS][MAX_GAMEPAD_AXIS][2]; // [0] = min, [1] = range value of the axis
    int gamepadAbsAxisMap[MAX_GAMEPADS][ABS_CNT]; // Maps the axes gamepads from the evdev api to a sequential one
    int gamepadCount;                   // The number of gamepads registered
} PlatformData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern CoreData CORE;                   // Global CORE state context

static PlatformData platform = { 0 };   // Platform specific data

//----------------------------------------------------------------------------------
// Local Variables Definition
//----------------------------------------------------------------------------------

// NOTE: The complete evdev EV_KEY list can be found at /usr/include/linux/input-event-codes.h
// TODO: Complete the LUT with all unicode decimal values
// TODO: Replace this with a keymap from the X11 to get the correct regional map for the keyboard:
// Currently non US keyboards will have the wrong mapping for some keys
// NOTE: Replacing this with the keymap from X11 would probably be useless, as people use the drm
// backend to *avoid* X11
static const int evkeyToUnicodeLUT[] = {
    0, 27, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 45, 61, 8, 0, 113, 119, 101, 114,
    116, 121, 117, 105, 111, 112, 0, 0, 13, 0, 97, 115, 100, 102, 103, 104, 106, 107, 108, 59,
    39, 96, 0, 92, 122, 120, 99, 118, 98, 110, 109, 44, 46, 47, 0, 0, 0, 32
    // LUT currently incomplete, just mapped the most essential keys
};

// This is the map used to map any keycode returned from linux to a raylib code from 'raylib.h'
// NOTE: Use short here to save a little memory
static const short linuxToRaylibMap[KEYMAP_SIZE] = {
    // We don't map those with designated initialization, because we would getting
    // into loads of naming conflicts
    0,   256, 49,  50,  51,  52,  53,  54,
    55,  56,  57,  48,  45,  61,  259, 258,
    81,  87,  69,  82,  84,  89,  85,  73,
    79,  80,  91,  93,  257, 341, 65,  83,
    68,  70,  71,  72,  74,  75,  76,  59,
    39,  96,  340, 92,  90,  88,  67,  86,
    66,  78,  77,  44,  46,  47,  344, 332,
    342, 32,  280, 290, 291, 292, 293, 294,
    295, 296, 297, 298, 299, 282, 281, 327,
    328, 329, 333, 324, 325, 326, 334, 321,
    322, 323, 320, 330, 0,   85,  86,  300,
    301, 89,  90,  91,  92,  93,  94,  95,
    335, 345, 331, 283, 346, 101, 268, 265,
    266, 263, 262, 269, 264, 267, 260, 261,
    112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 347, 127,
    128, 129, 130, 131, 132, 133, 134, 135,
    136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151,
    152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167,
    168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 181, 182, 183,
    184, 185, 186, 187, 188, 189, 190, 191,
    192, 193, 194, 0,   0,   0,   0,   0,
    200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215,
    216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231,
    232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247,
    248, 0,   0,   0,   0,   0,   0,   0,

    // Gamepads are mapped according to:
    // https://www.kernel.org/doc/html/next/input/gamepad.html
    // Those mappings are standardized, but that doesn't mean people follow
    // the standards, so this is more of an approximation
    [BTN_DPAD_UP] = GAMEPAD_BUTTON_LEFT_FACE_UP,
    [BTN_DPAD_RIGHT] = GAMEPAD_BUTTON_LEFT_FACE_RIGHT,
    [BTN_DPAD_DOWN] = GAMEPAD_BUTTON_LEFT_FACE_DOWN,
    [BTN_DPAD_LEFT] = GAMEPAD_BUTTON_LEFT_FACE_LEFT,
    [BTN_Y] = GAMEPAD_BUTTON_RIGHT_FACE_UP,
    [BTN_B] = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,
    [BTN_A] = GAMEPAD_BUTTON_RIGHT_FACE_DOWN,
    [BTN_X] = GAMEPAD_BUTTON_RIGHT_FACE_LEFT,
    [BTN_TL] = GAMEPAD_BUTTON_LEFT_TRIGGER_1,
    [BTN_TL2] = GAMEPAD_BUTTON_LEFT_TRIGGER_2,
    [BTN_TR] = GAMEPAD_BUTTON_RIGHT_TRIGGER_1,
    [BTN_TR2] = GAMEPAD_BUTTON_RIGHT_TRIGGER_2,
    [BTN_SELECT] = GAMEPAD_BUTTON_MIDDLE_LEFT,
    [BTN_MODE] = GAMEPAD_BUTTON_MIDDLE,
    [BTN_START] = GAMEPAD_BUTTON_MIDDLE_RIGHT,
    [BTN_THUMBL] = GAMEPAD_BUTTON_LEFT_THUMB,
    [BTN_THUMBR] = GAMEPAD_BUTTON_RIGHT_THUMB,
};

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
int InitPlatform(void);          // Initialize platform (graphics, inputs and more)
void ClosePlatform(void);        // Close platform

#if defined(SUPPORT_SSH_KEYBOARD_RPI)
static void InitKeyboard(void);                 // Initialize raw keyboard system
static void RestoreKeyboard(void);              // Restore keyboard system
static void ProcessKeyboard(void);              // Process keyboard events
#endif

static void InitEvdevInput(void);               // Initialize evdev inputs
static void ConfigureEvdevDevice(char *device); // Identifies a input device and configures it for use if appropriate
static void PollKeyboardEvents(void);           // Process evdev keyboard events
static void PollGamepadEvents(void);            // Process evdev gamepad events
static void PollMouseEvents(void);              // Process evdev mouse events

static int FindMatchingConnectorMode(const drmModeConnector *connector, const drmModeModeInfo *mode);                               // Search matching DRM mode in connector's mode list
static int FindExactConnectorMode(const drmModeConnector *connector, uint width, uint height, uint fps, bool allowInterlaced);      // Search exactly matching DRM connector mode in connector's list
static int FindNearestConnectorMode(const drmModeConnector *connector, uint width, uint height, uint fps, bool allowInterlaced);    // Search the nearest matching DRM connector mode in connector's list

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
// NOTE: Functions declaration is provided by raylib.h

//----------------------------------------------------------------------------------
// Module Functions Definition: Window and Graphics Device
//----------------------------------------------------------------------------------

// Check if application should close
// NOTE: By default, if KEY_ESCAPE pressed
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

// Set window state: not minimized/maximized
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

// Set icon for window
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

// Set window opacity, value opacity is between 0.0 and 1.0
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
    TRACELOG(LOG_WARNING, "GetWindowHandle() not implemented on target platform");
    return NULL;
}

// Get number of monitors
int GetMonitorCount(void)
{
    TRACELOG(LOG_WARNING, "GetMonitorCount() not implemented on target platform");
    return 1;
}

// Get number of monitors
int GetCurrentMonitor(void)
{
    TRACELOG(LOG_WARNING, "GetCurrentMonitor() not implemented on target platform");
    return 0;
}

// Get selected monitor position
Vector2 GetMonitorPosition(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorPosition() not implemented on target platform");
    return (Vector2){ 0, 0 };
}

// Get selected monitor width (currently used by monitor)
int GetMonitorWidth(int monitor)
{
    int width = 0;

    if (monitor != 0)
    {
        TRACELOG(LOG_WARNING, "GetMonitorWidth() implemented for first monitor only");
    }
    else if ((platform.connector) && (platform.modeIndex >= 0))
    {
        width = platform.connector->modes[platform.modeIndex].hdisplay;
    }

    return width;
}

// Get selected monitor height (currently used by monitor)
int GetMonitorHeight(int monitor)
{
    int height = 0;

    if (monitor != 0)
    {
        TRACELOG(LOG_WARNING, "GetMonitorHeight() implemented for first monitor only");
    }
    else if ((platform.connector) && (platform.modeIndex >= 0))
    {
        height = platform.connector->modes[platform.modeIndex].vdisplay;
    }

    return height;
}

// Get selected monitor physical width in millimetres
int GetMonitorPhysicalWidth(int monitor)
{
    int physicalWidth = 0;

    if (monitor != 0)
    {
        TRACELOG(LOG_WARNING, "GetMonitorPhysicalWidth() implemented for first monitor only");
    }
    else if ((platform.connector) && (platform.modeIndex >= 0))
    {
        physicalWidth = platform.connector->mmWidth;
    }

    return physicalWidth;
}

// Get selected monitor physical height in millimetres
int GetMonitorPhysicalHeight(int monitor)
{
    int physicalHeight = 0;

    if (monitor != 0)
    {
        TRACELOG(LOG_WARNING, "GetMonitorPhysicalHeight() implemented for first monitor only");
    }
    else if ((platform.connector) && (platform.modeIndex >= 0))
    {
        physicalHeight = platform.connector->mmHeight;
    }

    return physicalHeight;
}

// Get selected monitor refresh rate
int GetMonitorRefreshRate(int monitor)
{
    int refresh = 0;

    if ((platform.connector) && (platform.modeIndex >= 0))
    {
        refresh = platform.connector->modes[platform.modeIndex].vrefresh;
    }

    return refresh;
}

// Get the human-readable, UTF-8 encoded name of the selected monitor
const char *GetMonitorName(int monitor)
{
    const char *name = "";

    if (monitor != 0)
    {
        TRACELOG(LOG_WARNING, "GetMonitorName() implemented for first monitor only");
    }
    else if ((platform.connector) && (platform.modeIndex >= 0))
    {
        name = platform.connector->modes[platform.modeIndex].name;
    }

    return name;
}

// Get window position XY on monitor
Vector2 GetWindowPosition(void)
{
    return (Vector2){ 0, 0 };
}

// Get window scale DPI factor for current monitor
Vector2 GetWindowScaleDPI(void)
{
    return (Vector2){ 1.0f, 1.0f };
}

// Set clipboard text content
void SetClipboardText(const char *text)
{
    TRACELOG(LOG_WARNING, "SetClipboardText() not implemented on target platform");
}

// Get clipboard text content
// NOTE: returned string is allocated and freed by GLFW
const char *GetClipboardText(void)
{
    TRACELOG(LOG_WARNING, "GetClipboardText() not implemented on target platform");
    return NULL;
}

// Show mouse cursor
void ShowCursor(void)
{
    CORE.Input.Mouse.cursorHidden = false;
}

// Hides mouse cursor
void HideCursor(void)
{
    CORE.Input.Mouse.cursorHidden = true;
}

// Enables cursor (unlock cursor)
void EnableCursor(void)
{
    // Set cursor position in the middle
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);

    platform.cursorRelative = false;
    CORE.Input.Mouse.cursorHidden = false;
}

// Disables cursor (lock cursor)
void DisableCursor(void)
{
    // Set cursor position in the middle
    SetMousePosition(0, 0);

    platform.cursorRelative = true;
    CORE.Input.Mouse.cursorHidden = true;
}

// Swap back buffer with front buffer (screen drawing)
void SwapScreenBuffer(void)
{
    eglSwapBuffers(platform.device, platform.surface);

    if (!platform.gbmSurface || (-1 == platform.fd) || !platform.connector || !platform.crtc) TRACELOG(LOG_ERROR, "DISPLAY: DRM initialization failed to swap");

    struct gbm_bo *bo = gbm_surface_lock_front_buffer(platform.gbmSurface);
    if (!bo) TRACELOG(LOG_ERROR, "DISPLAY: Failed GBM to lock front buffer");

    uint32_t fb = 0;
    int result = drmModeAddFB(platform.fd, platform.connector->modes[platform.modeIndex].hdisplay, platform.connector->modes[platform.modeIndex].vdisplay, 24, 32, gbm_bo_get_stride(bo), gbm_bo_get_handle(bo).u32, &fb);
    if (result != 0) TRACELOG(LOG_ERROR, "DISPLAY: drmModeAddFB() failed with result: %d", result);

    result = drmModeSetCrtc(platform.fd, platform.crtc->crtc_id, fb, 0, 0, &platform.connector->connector_id, 1, &platform.connector->modes[platform.modeIndex]);
    if (result != 0) TRACELOG(LOG_ERROR, "DISPLAY: drmModeSetCrtc() failed with result: %d", result);

    if (platform.prevFB)
    {
        result = drmModeRmFB(platform.fd, platform.prevFB);
        if (result != 0) TRACELOG(LOG_ERROR, "DISPLAY: drmModeRmFB() failed with result: %d", result);
    }

    platform.prevFB = fb;

    if (platform.prevBO) gbm_surface_release_buffer(platform.gbmSurface, platform.prevBO);

    platform.prevBO = bo;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Misc
//----------------------------------------------------------------------------------

// Get elapsed time measure in seconds since InitTimer()
double GetTime(void)
{
    double time = 0.0;
    struct timespec ts = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &ts);
    unsigned long long int nanoSeconds = (unsigned long long int)ts.tv_sec*1000000000LLU + (unsigned long long int)ts.tv_nsec;

    time = (double)(nanoSeconds - CORE.Time.base)*1e-9;  // Elapsed time since InitTimer()

    return time;
}

// Open URL with default system browser (if available)
// NOTE: This function is only safe to use if you control the URL given.
// A user could craft a malicious string performing another action.
// Only call this function yourself not with user input or make sure to check the string yourself.
// Ref: https://github.com/raysan5/raylib/issues/686
void OpenURL(const char *url)
{
    TRACELOG(LOG_WARNING, "OpenURL() not implemented on target platform");
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
    TRACELOG(LOG_WARNING, "GamepadSetVibration() not implemented on target platform");
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

// Get physical key name.
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
    // because ProcessGestureEvent() is just called on an event, not every frame
    UpdateGestures();
#endif

    // Reset keys/chars pressed registered
    CORE.Input.Keyboard.keyPressedQueueCount = 0;
    CORE.Input.Keyboard.charPressedQueueCount = 0;

    // Reset last gamepad button/axis registered state
    CORE.Input.Gamepad.lastButtonPressed = 0;       // GAMEPAD_BUTTON_UNKNOWN
    //CORE.Input.Gamepad.axisCount = 0;

    // Register previous keys states
    for (int i = 0; i < MAX_KEYBOARD_KEYS; i++)
    {
        CORE.Input.Keyboard.previousKeyState[i] = CORE.Input.Keyboard.currentKeyState[i];
        CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }

    PollKeyboardEvents();

#if defined(SUPPORT_SSH_KEYBOARD_RPI)
    // NOTE: Keyboard reading could be done using input_event(s) or just read from stdin, both methods are used here.
    // stdin reading is still used for legacy purposes, it allows keyboard input trough SSH console
    if (!platform.eventKeyboardMode) ProcessKeyboard();
#endif

    // Check exit key
    if (CORE.Input.Keyboard.currentKeyState[CORE.Input.Keyboard.exitKey] == 1) CORE.Window.shouldClose = true;

    // Register previous mouse position
    if (platform.cursorRelative) CORE.Input.Mouse.currentPosition = (Vector2){ 0.0f, 0.0f };
    else CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;

    // Register previous mouse states
    CORE.Input.Mouse.previousWheelMove = CORE.Input.Mouse.currentWheelMove;
    CORE.Input.Mouse.currentWheelMove = platform.eventWheelMove;
    platform.eventWheelMove = (Vector2){ 0.0f, 0.0f };

    for (int i = 0; i < MAX_MOUSE_BUTTONS; i++)
    {
        CORE.Input.Mouse.previousButtonState[i] = CORE.Input.Mouse.currentButtonState[i];
        CORE.Input.Mouse.currentButtonState[i] = platform.currentButtonStateEvdev[i];
        CORE.Input.Touch.currentTouchState[i] = platform.currentButtonStateEvdev[i];
    }

    // Register gamepads buttons events
    PollGamepadEvents();

    // Register previous touch states
    for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.previousTouchState[i] = CORE.Input.Touch.currentTouchState[i];

    // Reset touch positions
    //for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.position[i] = (Vector2){ 0, 0 };

    // Map touch position to mouse position for convenience
    CORE.Input.Touch.position[0] = CORE.Input.Mouse.currentPosition;

    // Handle the mouse/touch/gestures events:
    PollMouseEvents();
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

// Initialize platform: graphics, inputs and more
int InitPlatform(void)
{
    platform.fd = -1;
    platform.connector = NULL;
    platform.modeIndex = -1;
    platform.crtc = NULL;
    platform.gbmDevice = NULL;
    platform.gbmSurface = NULL;
    platform.prevBO = NULL;
    platform.prevFB = 0;

    // Initialize graphic device: display/window and graphic context
    //----------------------------------------------------------------------------
    CORE.Window.fullscreen = true;
    CORE.Window.flags |= FLAG_FULLSCREEN_MODE;

#if defined(DEFAULT_GRAPHIC_DEVICE_DRM)
    platform.fd = open(DEFAULT_GRAPHIC_DEVICE_DRM, O_RDWR);
#else
    TRACELOG(LOG_INFO, "DISPLAY: No graphic card set, trying platform-gpu-card");
    platform.fd = open("/dev/dri/by-path/platform-gpu-card",  O_RDWR); // VideoCore VI (Raspberry Pi 4)

    if ((platform.fd == -1) || (drmModeGetResources(platform.fd) == NULL))
    {
        TRACELOG(LOG_INFO, "DISPLAY: Failed to open platform-gpu-card, trying card1");
        platform.fd = open("/dev/dri/card1", O_RDWR); // Other Embedded
    }

    if ((platform.fd == -1) || (drmModeGetResources(platform.fd) == NULL))
    {
        TRACELOG(LOG_INFO, "DISPLAY: Failed to open graphic card1, trying card0");
        platform.fd = open("/dev/dri/card0", O_RDWR); // VideoCore IV (Raspberry Pi 1-3)
    }
#endif

    if (platform.fd == -1)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to open graphic card");
        return -1;
    }

    drmModeRes *res = drmModeGetResources(platform.fd);
    if (!res)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed get DRM resources");
        return -1;
    }

    TRACELOG(LOG_TRACE, "DISPLAY: Connectors found: %i", res->count_connectors);

    for (size_t i = 0; i < res->count_connectors; i++)
    {
        TRACELOG(LOG_TRACE, "DISPLAY: Connector index %i", i);

        drmModeConnector *con = drmModeGetConnector(platform.fd, res->connectors[i]);
        TRACELOG(LOG_TRACE, "DISPLAY: Connector modes detected: %i", con->count_modes);

        // In certain cases the status of the conneciton is reported as UKNOWN, but it is still connected.
        // This might be a hardware or software limitation like on Raspberry Pi Zero with composite output.
        if (((con->connection == DRM_MODE_CONNECTED) || (con->connection == DRM_MODE_UNKNOWNCONNECTION)) && (con->encoder_id))
        {
            TRACELOG(LOG_TRACE, "DISPLAY: DRM mode connected");
            platform.connector = con;
            break;
        }
        else
        {
            TRACELOG(LOG_TRACE, "DISPLAY: DRM mode NOT connected (deleting)");
            drmModeFreeConnector(con);
        }
    }

    if (!platform.connector)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: No suitable DRM connector found");
        drmModeFreeResources(res);
        return -1;
    }

    drmModeEncoder *enc = drmModeGetEncoder(platform.fd, platform.connector->encoder_id);
    if (!enc)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to get DRM mode encoder");
        drmModeFreeResources(res);
        return -1;
    }

    platform.crtc = drmModeGetCrtc(platform.fd, enc->crtc_id);
    if (!platform.crtc)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to get DRM mode crtc");
        drmModeFreeEncoder(enc);
        drmModeFreeResources(res);
        return -1;
    }

    // If InitWindow should use the current mode find it in the connector's mode list
    if ((CORE.Window.screen.width <= 0) || (CORE.Window.screen.height <= 0))
    {
        TRACELOG(LOG_TRACE, "DISPLAY: Selecting DRM connector mode for current used mode...");

        platform.modeIndex = FindMatchingConnectorMode(platform.connector, &platform.crtc->mode);

        if (platform.modeIndex < 0)
        {
            TRACELOG(LOG_WARNING, "DISPLAY: No matching DRM connector mode found");
            drmModeFreeEncoder(enc);
            drmModeFreeResources(res);
            return -1;
        }

        CORE.Window.screen.width = CORE.Window.display.width;
        CORE.Window.screen.height = CORE.Window.display.height;
    }

    const bool allowInterlaced = CORE.Window.flags & FLAG_INTERLACED_HINT;
    const int fps = (CORE.Time.target > 0)? (1.0/CORE.Time.target) : 60;

    // Try to find an exact matching mode
    platform.modeIndex = FindExactConnectorMode(platform.connector, CORE.Window.screen.width, CORE.Window.screen.height, fps, allowInterlaced);

    // If nothing found, try to find a nearly matching mode
    if (platform.modeIndex < 0) platform.modeIndex = FindNearestConnectorMode(platform.connector, CORE.Window.screen.width, CORE.Window.screen.height, fps, allowInterlaced);

    // If nothing found, try to find an exactly matching mode including interlaced
    if (platform.modeIndex < 0) platform.modeIndex = FindExactConnectorMode(platform.connector, CORE.Window.screen.width, CORE.Window.screen.height, fps, true);

    // If nothing found, try to find a nearly matching mode including interlaced
    if (platform.modeIndex < 0) platform.modeIndex = FindNearestConnectorMode(platform.connector, CORE.Window.screen.width, CORE.Window.screen.height, fps, true);

    // If nothing found, there is no suitable mode
    if (platform.modeIndex < 0)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to find a suitable DRM connector mode");
        drmModeFreeEncoder(enc);
        drmModeFreeResources(res);
        return -1;
    }

    CORE.Window.display.width = platform.connector->modes[platform.modeIndex].hdisplay;
    CORE.Window.display.height = platform.connector->modes[platform.modeIndex].vdisplay;

    TRACELOG(LOG_INFO, "DISPLAY: Selected DRM connector mode %s (%ux%u%c@%u)", platform.connector->modes[platform.modeIndex].name,
        platform.connector->modes[platform.modeIndex].hdisplay, platform.connector->modes[platform.modeIndex].vdisplay,
        (platform.connector->modes[platform.modeIndex].flags & DRM_MODE_FLAG_INTERLACE)? 'i' : 'p',
        platform.connector->modes[platform.modeIndex].vrefresh);

    // Use the width and height of the surface for render
    CORE.Window.render.width = CORE.Window.screen.width;
    CORE.Window.render.height = CORE.Window.screen.height;

    drmModeFreeEncoder(enc);
    enc = NULL;

    drmModeFreeResources(res);
    res = NULL;

    platform.gbmDevice = gbm_create_device(platform.fd);
    if (!platform.gbmDevice)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to create GBM device");
        return -1;
    }

    platform.gbmSurface = gbm_surface_create(platform.gbmDevice, platform.connector->modes[platform.modeIndex].hdisplay,
        platform.connector->modes[platform.modeIndex].vdisplay, GBM_FORMAT_ARGB8888, GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
    if (!platform.gbmSurface)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to create GBM surface");
        return -1;
    }

    EGLint samples = 0;
    EGLint sampleBuffer = 0;
    if (CORE.Window.flags & FLAG_MSAA_4X_HINT)
    {
        samples = 4;
        sampleBuffer = 1;
        TRACELOG(LOG_INFO, "DISPLAY: Trying to enable MSAA x4");
    }

    const EGLint framebufferAttribs[] =
    {
        EGL_RENDERABLE_TYPE, (rlGetVersion() == RL_OPENGL_ES_30)? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT,      // Type of context support
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,          // Don't use it on Android!
        EGL_RED_SIZE, 8,            // RED color bit depth (alternative: 5)
        EGL_GREEN_SIZE, 8,          // GREEN color bit depth (alternative: 6)
        EGL_BLUE_SIZE, 8,           // BLUE color bit depth (alternative: 5)
        EGL_ALPHA_SIZE, 8,        // ALPHA bit depth (required for transparent framebuffer)
        //EGL_TRANSPARENT_TYPE, EGL_NONE, // Request transparent framebuffer (EGL_TRANSPARENT_RGB does not work on RPI)
        EGL_DEPTH_SIZE, 16,         // Depth buffer size (Required to use Depth testing!)
        //EGL_STENCIL_SIZE, 8,      // Stencil buffer size
        EGL_SAMPLE_BUFFERS, sampleBuffer,    // Activate MSAA
        EGL_SAMPLES, samples,       // 4x Antialiasing if activated (Free on MALI GPUs)
        EGL_NONE
    };

    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    EGLint numConfigs = 0;

    // Get an EGL device connection
    platform.device = eglGetDisplay((EGLNativeDisplayType)platform.gbmDevice);
    if (platform.device == EGL_NO_DISPLAY)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
        return -1;
    }

    // Initialize the EGL device connection
    if (eglInitialize(platform.device, NULL, NULL) == EGL_FALSE)
    {
        // If all of the calls to eglInitialize returned EGL_FALSE then an error has occurred.
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
        return -1;
    }

    if (!eglChooseConfig(platform.device, NULL, NULL, 0, &numConfigs))
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to get EGL config count: 0x%x", eglGetError());
        return -1;
    }

    TRACELOG(LOG_TRACE, "DISPLAY: EGL configs available: %d", numConfigs);

    EGLConfig *configs = RL_CALLOC(numConfigs, sizeof(*configs));
    if (!configs)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to get memory for EGL configs");
        return -1;
    }

    EGLint matchingNumConfigs = 0;
    if (!eglChooseConfig(platform.device, framebufferAttribs, configs, numConfigs, &matchingNumConfigs))
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to choose EGL config: 0x%x", eglGetError());
        free(configs);
        return -1;
    }

    TRACELOG(LOG_TRACE, "DISPLAY: EGL matching configs available: %d", matchingNumConfigs);

    // find the EGL config that matches the previously setup GBM format
    int found = 0;
    for (EGLint i = 0; i < matchingNumConfigs; ++i)
    {
        EGLint id = 0;
        if (!eglGetConfigAttrib(platform.device, configs[i], EGL_NATIVE_VISUAL_ID, &id))
        {
            TRACELOG(LOG_WARNING, "DISPLAY: Failed to get EGL config attribute: 0x%x", eglGetError());
            continue;
        }

        if (GBM_FORMAT_ARGB8888 == id)
        {
            TRACELOG(LOG_TRACE, "DISPLAY: Using EGL config: %d", i);
            platform.config = configs[i];
            found = 1;
            break;
        }
    }

    RL_FREE(configs);

    if (!found)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to find a suitable EGL config");
        return -1;
    }

    // Set rendering API
    eglBindAPI(EGL_OPENGL_ES_API);

    // Create an EGL rendering context
    platform.context = eglCreateContext(platform.device, platform.config, EGL_NO_CONTEXT, contextAttribs);
    if (platform.context == EGL_NO_CONTEXT)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to create EGL context");
        return -1;
    }

    // Create an EGL window surface
    platform.surface = eglCreateWindowSurface(platform.device, platform.config, (EGLNativeWindowType)platform.gbmSurface, NULL);
    if (EGL_NO_SURFACE == platform.surface)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to create EGL window surface: 0x%04x", eglGetError());
        return -1;
    }

    // At this point we need to manage render size vs screen size
    // NOTE: This function use and modify global module variables:
    //  -> CORE.Window.screen.width/CORE.Window.screen.height
    //  -> CORE.Window.render.width/CORE.Window.render.height
    //  -> CORE.Window.screenScale
    SetupFramebuffer(CORE.Window.display.width, CORE.Window.display.height);

    // There must be at least one frame displayed before the buffers are swapped
    //eglSwapInterval(platform.device, 1);

    EGLBoolean result = eglMakeCurrent(platform.device, platform.surface, platform.surface, platform.context);

    // Check surface and context activation
    if (result != EGL_FALSE)
    {
        CORE.Window.ready = true;

        CORE.Window.render.width = CORE.Window.screen.width;
        CORE.Window.render.height = CORE.Window.screen.height;
        CORE.Window.currentFbo.width = CORE.Window.render.width;
        CORE.Window.currentFbo.height = CORE.Window.render.height;

        TRACELOG(LOG_INFO, "DISPLAY: Device initialized successfully");
        TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
        TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
        TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
        TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);
    }
    else
    {
        TRACELOG(LOG_FATAL, "PLATFORM: Failed to initialize graphics device");
        return -1;
    }

    if ((CORE.Window.flags & FLAG_WINDOW_MINIMIZED) > 0) MinimizeWindow();

    // If graphic device is no properly initialized, we end program
    if (!CORE.Window.ready) { TRACELOG(LOG_FATAL, "PLATFORM: Failed to initialize graphic device"); return -1; }
    else SetWindowPosition(GetMonitorWidth(GetCurrentMonitor())/2 - CORE.Window.screen.width/2, GetMonitorHeight(GetCurrentMonitor())/2 - CORE.Window.screen.height/2);

    // Set some default window flags
    CORE.Window.flags &= ~FLAG_WINDOW_HIDDEN;       // false
    CORE.Window.flags &= ~FLAG_WINDOW_MINIMIZED;    // false
    CORE.Window.flags |= FLAG_WINDOW_MAXIMIZED;     // true
    CORE.Window.flags &= ~FLAG_WINDOW_UNFOCUSED;    // false

    // Load OpenGL extensions
    // NOTE: GL procedures address loader is required to load extensions
    rlLoadExtensions(eglGetProcAddress);
    //----------------------------------------------------------------------------

    // Initialize timming system
    //----------------------------------------------------------------------------
    // NOTE: timming system must be initialized before the input events system
    InitTimer();
    //----------------------------------------------------------------------------

    // Initialize input events system
    //----------------------------------------------------------------------------
    InitEvdevInput();   // Evdev inputs initialization

#if defined(SUPPORT_SSH_KEYBOARD_RPI)
    InitKeyboard();     // Keyboard init (stdin)
#endif
    //----------------------------------------------------------------------------

    // Initialize storage system
    //----------------------------------------------------------------------------
    CORE.Storage.basePath = GetWorkingDirectory();
    //----------------------------------------------------------------------------

    TRACELOG(LOG_INFO, "PLATFORM: DRM: Initialized successfully");

    return 0;
}

// Close platform
void ClosePlatform(void)
{
    if (platform.prevFB)
    {
        drmModeRmFB(platform.fd, platform.prevFB);
        platform.prevFB = 0;
    }

    if (platform.prevBO)
    {
        gbm_surface_release_buffer(platform.gbmSurface, platform.prevBO);
        platform.prevBO = NULL;
    }

    if (platform.gbmSurface)
    {
        gbm_surface_destroy(platform.gbmSurface);
        platform.gbmSurface = NULL;
    }

    if (platform.gbmDevice)
    {
        gbm_device_destroy(platform.gbmDevice);
        platform.gbmDevice = NULL;
    }

    if (platform.crtc)
    {
        if (platform.connector)
        {
            drmModeSetCrtc(platform.fd, platform.crtc->crtc_id, platform.crtc->buffer_id,
                platform.crtc->x, platform.crtc->y, &platform.connector->connector_id, 1, &platform.crtc->mode);
            drmModeFreeConnector(platform.connector);
            platform.connector = NULL;
        }

        drmModeFreeCrtc(platform.crtc);
        platform.crtc = NULL;
    }

    if (platform.fd != -1)
    {
        close(platform.fd);
        platform.fd = -1;
    }

    // Close surface, context and display
    if (platform.device != EGL_NO_DISPLAY)
    {
        if (platform.surface != EGL_NO_SURFACE)
        {
            eglDestroySurface(platform.device, platform.surface);
            platform.surface = EGL_NO_SURFACE;
        }

        if (platform.context != EGL_NO_CONTEXT)
        {
            eglDestroyContext(platform.device, platform.context);
            platform.context = EGL_NO_CONTEXT;
        }

        eglTerminate(platform.device);
        platform.device = EGL_NO_DISPLAY;
    }

    CORE.Window.shouldClose = true;   // Added to force threads to exit when the close window is called

    // Close the evdev devices

    if (platform.mouseFd != -1)
    {
        close(platform.mouseFd);
        platform.mouseFd = -1;
    }

    for (int i = 0; i < platform.gamepadCount; i++)
    {
        close(platform.gamepadStreamFd[i]);
        platform.gamepadStreamFd[i] = -1;
    }

    if (platform.keyboardFd != -1)
    {
        close(platform.keyboardFd);
        platform.keyboardFd = -1;
    }
}

#if defined(SUPPORT_SSH_KEYBOARD_RPI)
// Initialize Keyboard system (using standard input)
static void InitKeyboard(void)
{
    // NOTE: We read directly from Standard Input (stdin) - STDIN_FILENO file descriptor,
    // Reading directly from stdin will give chars already key-mapped by kernel to ASCII or UNICODE

    // Save terminal keyboard settings
    tcgetattr(STDIN_FILENO, &platform.defaultSettings);

    // Reconfigure terminal with new settings
    struct termios keyboardNewSettings = { 0 };
    keyboardNewSettings = platform.defaultSettings;

    // New terminal settings for keyboard: turn off buffering (non-canonical mode), echo and key processing
    // NOTE: ISIG controls if ^C and ^Z generate break signals or not
    keyboardNewSettings.c_lflag &= ~(ICANON | ECHO | ISIG);
    //keyboardNewSettings.c_iflag &= ~(ISTRIP | INLCR | ICRNL | IGNCR | IXON | IXOFF);
    keyboardNewSettings.c_cc[VMIN] = 1;
    keyboardNewSettings.c_cc[VTIME] = 0;

    // Set new keyboard settings (change occurs immediately)
    tcsetattr(STDIN_FILENO, TCSANOW, &keyboardNewSettings);

    // Save old keyboard mode to restore it at the end
    platform.defaultFileFlags = fcntl(STDIN_FILENO, F_GETFL, 0);          // F_GETFL: Get the file access mode and the file status flags
    fcntl(STDIN_FILENO, F_SETFL, platform.defaultFileFlags | O_NONBLOCK); // F_SETFL: Set the file status flags to the value specified

    // NOTE: If ioctl() returns -1, it means the call failed for some reason (error code set in errno)
    int result = ioctl(STDIN_FILENO, KDGKBMODE, &platform.defaultKeyboardMode);

    // In case of failure, it could mean a remote keyboard is used (SSH)
    if (result < 0) TRACELOG(LOG_WARNING, "DRM: Failed to change keyboard mode, an SSH keyboard is probably used");
    else
    {
        // Reconfigure keyboard mode to get:
        //    - scancodes (K_RAW)
        //    - keycodes (K_MEDIUMRAW)
        //    - ASCII chars (K_XLATE)
        //    - UNICODE chars (K_UNICODE)
        ioctl(STDIN_FILENO, KDSKBMODE, K_XLATE);  // ASCII chars
    }

    // Register keyboard restore when program finishes
    atexit(RestoreKeyboard);
}

// Restore default keyboard input
static void RestoreKeyboard(void)
{
    // Reset to default keyboard settings
    tcsetattr(STDIN_FILENO, TCSANOW, &platform.defaultSettings);

    // Reconfigure keyboard to default mode
    fcntl(STDIN_FILENO, F_SETFL, platform.defaultFileFlags);
    ioctl(STDIN_FILENO, KDSKBMODE, platform.defaultKeyboardMode);
}

// Process keyboard inputs
static void ProcessKeyboard(void)
{
    #define MAX_KEYBUFFER_SIZE      32      // Max size in bytes to read

    // Keyboard input polling (fill keys[256] array with status)
    int bufferByteCount = 0;                        // Bytes available on the buffer
    char keysBuffer[MAX_KEYBUFFER_SIZE] = { 0 };    // Max keys to be read at a time

    // Read availables keycodes from stdin
    bufferByteCount = read(STDIN_FILENO, keysBuffer, MAX_KEYBUFFER_SIZE);     // POSIX system call

    // Reset pressed keys array (it will be filled below)
    for (int i = 0; i < MAX_KEYBOARD_KEYS; i++)
    {
        CORE.Input.Keyboard.currentKeyState[i] = 0;
        CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }

    // Fill all read bytes (looking for keys)
    for (int i = 0; i < bufferByteCount; i++)
    {
        // NOTE: If (key == 0x1b), depending on next key, it could be a special keymap code!
        // Up -> 1b 5b 41 / Left -> 1b 5b 44 / Right -> 1b 5b 43 / Down -> 1b 5b 42
        if (keysBuffer[i] == 0x1b)
        {
            // Check if ESCAPE key has been pressed to stop program
            if (bufferByteCount == 1) CORE.Input.Keyboard.currentKeyState[CORE.Input.Keyboard.exitKey] = 1;
            else
            {
                if (keysBuffer[i + 1] == 0x5b)    // Special function key
                {
                    if ((keysBuffer[i + 2] == 0x5b) || (keysBuffer[i + 2] == 0x31) || (keysBuffer[i + 2] == 0x32))
                    {
                        // Process special function keys (F1 - F12)
                        switch (keysBuffer[i + 3])
                        {
                            case 0x41: CORE.Input.Keyboard.currentKeyState[290] = 1; break;    // raylib KEY_F1
                            case 0x42: CORE.Input.Keyboard.currentKeyState[291] = 1; break;    // raylib KEY_F2
                            case 0x43: CORE.Input.Keyboard.currentKeyState[292] = 1; break;    // raylib KEY_F3
                            case 0x44: CORE.Input.Keyboard.currentKeyState[293] = 1; break;    // raylib KEY_F4
                            case 0x45: CORE.Input.Keyboard.currentKeyState[294] = 1; break;    // raylib KEY_F5
                            case 0x37: CORE.Input.Keyboard.currentKeyState[295] = 1; break;    // raylib KEY_F6
                            case 0x38: CORE.Input.Keyboard.currentKeyState[296] = 1; break;    // raylib KEY_F7
                            case 0x39: CORE.Input.Keyboard.currentKeyState[297] = 1; break;    // raylib KEY_F8
                            case 0x30: CORE.Input.Keyboard.currentKeyState[298] = 1; break;    // raylib KEY_F9
                            case 0x31: CORE.Input.Keyboard.currentKeyState[299] = 1; break;    // raylib KEY_F10
                            case 0x33: CORE.Input.Keyboard.currentKeyState[300] = 1; break;    // raylib KEY_F11
                            case 0x34: CORE.Input.Keyboard.currentKeyState[301] = 1; break;    // raylib KEY_F12
                            default: break;
                        }

                        if (keysBuffer[i + 2] == 0x5b) i += 4;
                        else if ((keysBuffer[i + 2] == 0x31) || (keysBuffer[i + 2] == 0x32)) i += 5;
                    }
                    else
                    {
                        switch (keysBuffer[i + 2])
                        {
                            case 0x41: CORE.Input.Keyboard.currentKeyState[265] = 1; break;    // raylib KEY_UP
                            case 0x42: CORE.Input.Keyboard.currentKeyState[264] = 1; break;    // raylib KEY_DOWN
                            case 0x43: CORE.Input.Keyboard.currentKeyState[262] = 1; break;    // raylib KEY_RIGHT
                            case 0x44: CORE.Input.Keyboard.currentKeyState[263] = 1; break;    // raylib KEY_LEFT
                            default: break;
                        }

                        i += 3;  // Jump to next key
                    }

                    // NOTE: Some keys are not directly keymapped (CTRL, ALT, SHIFT)
                }
            }
        }
        else if (keysBuffer[i] == 0x0a)     // raylib KEY_ENTER (don't mix with <linux/input.h> KEY_*)
        {
            CORE.Input.Keyboard.currentKeyState[257] = 1;

            CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = 257;     // Add keys pressed into queue
            CORE.Input.Keyboard.keyPressedQueueCount++;
        }
        else if (keysBuffer[i] == 0x7f)     // raylib KEY_BACKSPACE
        {
            CORE.Input.Keyboard.currentKeyState[259] = 1;

            CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = 257;     // Add keys pressed into queue
            CORE.Input.Keyboard.keyPressedQueueCount++;
        }
        else
        {
            // Translate lowercase a-z letters to A-Z
            if ((keysBuffer[i] >= 97) && (keysBuffer[i] <= 122))
            {
                CORE.Input.Keyboard.currentKeyState[(int)keysBuffer[i] - 32] = 1;
            }
            else CORE.Input.Keyboard.currentKeyState[(int)keysBuffer[i]] = 1;

            CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = keysBuffer[i];     // Add keys pressed into queue
            CORE.Input.Keyboard.keyPressedQueueCount++;
        }
    }
}
#endif  // SUPPORT_SSH_KEYBOARD_RPI

// Initialise user input from evdev(/dev/input/event<N>)
// this means mouse, keyboard or gamepad devices
static void InitEvdevInput(void)
{
    char path[MAX_FILEPATH_LENGTH] = { 0 };
    DIR *directory = NULL;
    struct dirent *entity = NULL;

    // Initialise keyboard file descriptor
    platform.keyboardFd = -1;
    platform.mouseFd = -1;

    // Reset variables
    for (int i = 0; i < MAX_TOUCH_POINTS; ++i)
    {
        CORE.Input.Touch.position[i].x = -1;
        CORE.Input.Touch.position[i].y = -1;
    }

    // Reset keyboard key state
    for (int i = 0; i < MAX_KEYBOARD_KEYS; i++)
    {
        CORE.Input.Keyboard.currentKeyState[i] = 0;
        CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }

    // Open the linux directory of "/dev/input"
    directory = opendir(DEFAULT_EVDEV_PATH);

    if (directory)
    {
        while ((entity = readdir(directory)) != NULL)
        {
            if ((strncmp("event", entity->d_name, strlen("event")) == 0) ||     // Search for devices named "event*"
                (strncmp("mouse", entity->d_name, strlen("mouse")) == 0))       // Search for devices named "mouse*"
            {
                sprintf(path, "%s%s", DEFAULT_EVDEV_PATH, entity->d_name);
                ConfigureEvdevDevice(path);                                     // Configure the device if appropriate
            }
        }

        closedir(directory);
    }
    else TRACELOG(LOG_WARNING, "INPUT: Failed to open linux event directory: %s", DEFAULT_EVDEV_PATH);
}

// Identifies a input device and configures it for use if appropriate
static void ConfigureEvdevDevice(char *device)
{
    #define BITS_PER_LONG   (8*sizeof(long))
    #define NBITS(x)        ((((x) - 1)/BITS_PER_LONG) + 1)
    #define OFF(x)          ((x)%BITS_PER_LONG)
    #define BIT(x)          (1UL<<OFF(x))
    #define LONG(x)         ((x)/BITS_PER_LONG)
    #define TEST_BIT(array, bit) ((array[LONG(bit)] >> OFF(bit)) & 1)

    unsigned long evBits[NBITS(EV_MAX)] = { 0 };
    unsigned long absBits[NBITS(ABS_MAX)] = { 0 };
    unsigned long relBits[NBITS(REL_MAX)] = { 0 };
    unsigned long keyBits[NBITS(KEY_MAX)] = { 0 };

    // Open the device
    int fd = open(device, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        TRACELOG(LOG_WARNING, "DRM: Failed to open input device: %s", device);
        return;
    }

    // At this point we have a connection to the device, but we don't yet know what the device is.
    // It could be many things, even as simple as a power button...
    //-------------------------------------------------------------------------------------------------------

    // Identify the device
    //-------------------------------------------------------------------------------------------------------
    struct {
        bool exist;
        struct input_absinfo info;
    } absinfo[ABS_CNT] = { 0 };

    // These flags aren't really a one of
    // Some devices could have properties we assosciate with keyboards as well as properties
    // we assosciate with mice
    bool isKeyboard = false;
    bool isMouse = false;
    bool isTouch = false;
    bool isGamepad = false;

    int absAxisCount = 0;

    ioctl(fd, EVIOCGBIT(0, sizeof(evBits)), evBits);
    ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyBits)), keyBits);

    if (TEST_BIT(evBits, EV_ABS))
    {
        ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absBits)), absBits);

        // If the device has an X an Y axis it's either a touch device, a special mouse or a gamepad
        bool hasAbsXY = TEST_BIT(absBits, ABS_X) && TEST_BIT(absBits, ABS_Y);

        if (hasAbsXY)
        {
            absAxisCount += 2;

            absinfo[ABS_X].exist = true;
            absinfo[ABS_Y].exist = true;

            ioctl(fd, EVIOCGABS(ABS_X), &absinfo[ABS_X].info);
            ioctl(fd, EVIOCGABS(ABS_Y), &absinfo[ABS_Y].info);
        }

        // If it has any of these buttons it's a touch device
        if (hasAbsXY &&
           (TEST_BIT(keyBits, BTN_STYLUS) ||
            TEST_BIT(keyBits, BTN_TOOL_PEN) ||
            TEST_BIT(keyBits, BTN_TOOL_FINGER) ||
            TEST_BIT(keyBits, BTN_TOUCH))) isTouch = true;

        // Absolute mice should really only exist with VMWare, but it shouldn't
        // matter if we support them
        else if (hasAbsXY && TEST_BIT(keyBits, BTN_MOUSE)) isMouse = true;

        // If any of the common joystick axis is present, we assume it's a gamepad
        else
        {
            for (int axis = (hasAbsXY? ABS_Z : ABS_X); axis < ABS_PRESSURE; axis++)
            {
                if (TEST_BIT(absBits, axis))
                {
                    absinfo[axis].exist = true;
                    isGamepad = true;
                    absAxisCount++;

                    ioctl(fd, EVIOCGABS(axis), &absinfo[axis].info);
                }
            }
        }

        // If the device has multitouch axes, it's a touch device
        if (TEST_BIT(absBits, ABS_MT_POSITION_X) &&
            TEST_BIT(absBits, ABS_MT_POSITION_Y)) isTouch = true;
    }

    if (TEST_BIT(evBits, EV_REL))
    {
        ioctl(fd, EVIOCGBIT(EV_REL, sizeof(relBits)), relBits);

        // If it has any of the gamepad or touch features we tested so far, it's not a mouse
        if (!isTouch &&
            !isGamepad &&
            TEST_BIT(relBits, REL_X) &&
            TEST_BIT(relBits, REL_Y) &&
            TEST_BIT(keyBits, BTN_MOUSE)) isMouse = true;
    }

    if (TEST_BIT(evBits, EV_KEY))
    {
        // The first 32 keys as defined in input-event-codes.h are pretty much
        // exclusive to keyboards, so we can test them using a mask
        // Leave out the first bit to not test KEY_RESERVED
        const unsigned long mask = 0xFFFFFFFE;
        if ((keyBits[0] & mask) == mask) isKeyboard = true;

        // If we find any of the common gamepad buttons we assume it's a gamepad
        else
        {
            for (int button = BTN_JOYSTICK; button < BTN_DIGI; ++button)
            {
                if (TEST_BIT(keyBits, button)) isGamepad = true;
            }

            for (int button = BTN_TRIGGER_HAPPY1; button <= BTN_TRIGGER_HAPPY40; button++)
            {
                if (TEST_BIT(keyBits, button)) isGamepad = true;
            }
        }
    }

    const char *deviceKindStr = "unknown";
    if (isMouse || isTouch)
    {
        deviceKindStr = "mouse";
        if (platform.mouseFd != -1) close(platform.mouseFd);
        platform.mouseFd = fd;

        if (absAxisCount > 0)
        {
            platform.absRange.x = absinfo[ABS_X].info.minimum;
            platform.absRange.width = absinfo[ABS_X].info.maximum - absinfo[ABS_X].info.minimum;

            platform.absRange.y = absinfo[ABS_Y].info.minimum;
            platform.absRange.height = absinfo[ABS_Y].info.maximum - absinfo[ABS_Y].info.minimum;
        }
    }
    else if (isGamepad && !isMouse && !isKeyboard && platform.gamepadCount < MAX_GAMEPADS)
    {
        deviceKindStr = "gamepad";
        int index = platform.gamepadCount++;

        platform.gamepadStreamFd[index] = fd;
        CORE.Input.Gamepad.ready[index] = true;

        ioctl(platform.gamepadStreamFd[index], EVIOCGNAME(64), &CORE.Input.Gamepad.name[index]);
        CORE.Input.Gamepad.axisCount[index] = absAxisCount;

        if (absAxisCount > 0)
        {
            // TODO / NOTE
            // So gamepad axis (as in the actual linux joydev.c) are just simply enumerated
            // and (at least for some input drivers like xpat) it's convention to use
            // ABS_X, ABX_Y for one joystick ABS_RX, ABS_RY for the other and the Z axes for the
            // shoulder buttons
            // If these are now enumerated you get LJOY_X, LJOY_Y, LEFT_SHOULDERB, RJOY_X, ...
            // That means they don't match the GamepadAxis enum
            // This could be fixed
            int axisIndex = 0;
            for (int axis = ABS_X; axis < ABS_PRESSURE; axis++)
            {
                if (absinfo[axis].exist)
                {
                    platform.gamepadAbsAxisRange[index][axisIndex][0] = absinfo[axisIndex].info.minimum;
                    platform.gamepadAbsAxisRange[index][axisIndex][1] = absinfo[axisIndex].info.maximum - absinfo[axisIndex].info.minimum;

                    platform.gamepadAbsAxisMap[index][axis] = axisIndex;
                    axisIndex++;
                }
            }
        }
    }
    else if (isKeyboard && (platform.keyboardFd == -1))
    {
        deviceKindStr = "keyboard";
        platform.keyboardFd = fd;
    }
    else
    {
        close(fd);
        return;
    }

    TRACELOG(LOG_INFO, "INPUT: Initialized input device %s as %s", device, deviceKindStr);
}

// Poll and process evdev keyboard events
static void PollKeyboardEvents(void)
{
    int fd = platform.keyboardFd;
    if (fd == -1) return;

    struct input_event event = { 0 };
    int keycode = -1;

    // Try to read data from the keyboard and only continue if successful
    while (read(fd, &event, sizeof(event)) == (int)sizeof(event))
    {
        // Check if the event is a key event
        if (event.type != EV_KEY) continue;

#if defined(SUPPORT_SSH_KEYBOARD_RPI)
        // If the event was a key, we know a working keyboard is connected, so disable the SSH keyboard
        platform.eventKeyboardMode = true;
#endif

        // Keyboard keys appear for codes 1 to 255, ignore everthing else
        if ((event.code >= 1) && (event.code <= 255))
        {

            // Lookup the scancode in the keymap to get a keycode
            keycode = linuxToRaylibMap[event.code];

            // Make sure we got a valid keycode
            if ((keycode > 0) && (keycode < MAX_KEYBOARD_KEYS))
            {

                // WARNING: https://www.kernel.org/doc/Documentation/input/input.txt
                // Event interface: 'value' is the value the event carries. Either a relative change for EV_REL,
                // absolute new value for EV_ABS (joysticks ...), or 0 for EV_KEY for release, 1 for keypress and 2 for autorepeat
                CORE.Input.Keyboard.currentKeyState[keycode] = (event.value >= 1);
                CORE.Input.Keyboard.keyRepeatInFrame[keycode] = (event.value == 2);

                // If the key is pressed add it to the queues
                if (event.value == 1)
                {
                    if (CORE.Input.Keyboard.keyPressedQueueCount < MAX_CHAR_PRESSED_QUEUE)
                    {
                        CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = keycode;
                        CORE.Input.Keyboard.keyPressedQueueCount++;
                    }

                    if (CORE.Input.Keyboard.charPressedQueueCount < MAX_CHAR_PRESSED_QUEUE)
                    {
                        // TODO/FIXME: This is not actually converting to unicode properly because it's not taking things like shift into account
                        CORE.Input.Keyboard.charPressedQueue[CORE.Input.Keyboard.charPressedQueueCount] = evkeyToUnicodeLUT[event.code];
                        CORE.Input.Keyboard.charPressedQueueCount++;
                    }
                }

                TRACELOG(LOG_DEBUG, "INPUT: KEY_%s Keycode(linux): %4i KeyCode(raylib): %4i", (event.value == 0)? "UP  " : "DOWN", event.code, keycode);
            }
        }
    }
}

// Poll gamepad input events
static void PollGamepadEvents(void)
{
    // Read gamepad event
    struct input_event event = { 0 };

    for (int i = 0; i < platform.gamepadCount; i++)
    {
        if (!CORE.Input.Gamepad.ready[i]) continue;

        // Register previous gamepad states
        for (int k = 0; k < MAX_GAMEPAD_BUTTONS; k++) CORE.Input.Gamepad.previousButtonState[i][k] = CORE.Input.Gamepad.currentButtonState[i][k];

        while (read(platform.gamepadStreamFd[i], &event, sizeof(event)) == (int)sizeof(event))
        {
            if (event.type == EV_KEY)
            {
                if (event.code < KEYMAP_SIZE)
                {
                    short keycodeRaylib = linuxToRaylibMap[event.code];

                    TRACELOG(LOG_DEBUG, "INPUT: Gamepad %2i: KEY_%s Keycode(linux): %4i Keycode(raylib): %4i", i, (event.value == 0)? "UP" : "DOWN", event.code, keycodeRaylib);

                    if ((keycodeRaylib != 0) && (keycodeRaylib < MAX_GAMEPAD_BUTTONS))
                    {
                        // 1 - button pressed, 0 - button released
                        CORE.Input.Gamepad.currentButtonState[i][keycodeRaylib] = event.value;

                        CORE.Input.Gamepad.lastButtonPressed = (event.value == 1)? keycodeRaylib : GAMEPAD_BUTTON_UNKNOWN;
                    }
                }
            }
            else if (event.type == EV_ABS)
            {
                if (event.code < ABS_CNT)
                {
                    int axisRaylib = platform.gamepadAbsAxisMap[i][event.code];

                    TRACELOG(LOG_DEBUG, "INPUT: Gamepad %2i: Axis: %2i Value: %i", i, axisRaylib, event.value);

                    if (axisRaylib < MAX_GAMEPAD_AXIS)
                    {
                        int min = platform.gamepadAbsAxisRange[i][event.code][0];
                        int range = platform.gamepadAbsAxisRange[i][event.code][1];

                        // NOTE: Scaling of event.value to get values between -1..1
                        CORE.Input.Gamepad.axisState[i][axisRaylib] = (2*(float)(event.value - min)/range) - 1;
                    }
                }
            }
        }
    }
}

// Poll mouse input events
static void PollMouseEvents(void)
{
    int fd = platform.mouseFd;
    if (fd == -1) return;

    struct input_event event = { 0 };
    int touchAction = -1;           // 0-TOUCH_ACTION_UP, 1-TOUCH_ACTION_DOWN, 2-TOUCH_ACTION_MOVE

    // Try to read data from the mouse/touch/gesture and only continue if successful
    while (read(fd, &event, sizeof(event)) == (int)sizeof(event))
    {
        // Relative movement parsing
        if (event.type == EV_REL)
        {
            if (event.code == REL_X)
            {
                if (platform.cursorRelative)
                {
                    CORE.Input.Mouse.currentPosition.x = event.value;
                    CORE.Input.Mouse.previousPosition.x = 0.0f;
                }
                else CORE.Input.Mouse.currentPosition.x += event.value;

                CORE.Input.Touch.position[0].x = CORE.Input.Mouse.currentPosition.x;
                touchAction = 2;    // TOUCH_ACTION_MOVE
            }

            if (event.code == REL_Y)
            {
                if (platform.cursorRelative)
                {
                    CORE.Input.Mouse.currentPosition.y = event.value;
                    CORE.Input.Mouse.previousPosition.y = 0.0f;
                }
                else CORE.Input.Mouse.currentPosition.y += event.value;

                CORE.Input.Touch.position[0].y = CORE.Input.Mouse.currentPosition.y;
                touchAction = 2;    // TOUCH_ACTION_MOVE
            }

            if (event.code == REL_WHEEL) platform.eventWheelMove.y += event.value;
        }

        // Absolute movement parsing
        if (event.type == EV_ABS)
        {
            // Basic movement
            if (event.code == ABS_X)
            {
                CORE.Input.Mouse.currentPosition.x = (event.value - platform.absRange.x)*CORE.Window.screen.width/platform.absRange.width;    // Scale according to absRange
                CORE.Input.Touch.position[0].x = (event.value - platform.absRange.x)*CORE.Window.screen.width/platform.absRange.width;        // Scale according to absRange

                touchAction = 2;    // TOUCH_ACTION_MOVE
            }

            if (event.code == ABS_Y)
            {
                CORE.Input.Mouse.currentPosition.y = (event.value - platform.absRange.y)*CORE.Window.screen.height/platform.absRange.height;  // Scale according to absRange
                CORE.Input.Touch.position[0].y = (event.value - platform.absRange.y)*CORE.Window.screen.height/platform.absRange.height;      // Scale according to absRange

                touchAction = 2;    // TOUCH_ACTION_MOVE
            }

            // Multitouch movement
            if (event.code == ABS_MT_SLOT) platform.touchSlot = event.value;   // Remember the slot number for the folowing events

            if (event.code == ABS_MT_POSITION_X)
            {
                if (platform.touchSlot < MAX_TOUCH_POINTS) CORE.Input.Touch.position[platform.touchSlot].x = (event.value - platform.absRange.x)*CORE.Window.screen.width/platform.absRange.width;    // Scale according to absRange
            }

            if (event.code == ABS_MT_POSITION_Y)
            {
                if (platform.touchSlot < MAX_TOUCH_POINTS) CORE.Input.Touch.position[platform.touchSlot].y = (event.value - platform.absRange.y)*CORE.Window.screen.height/platform.absRange.height;  // Scale according to absRange
            }

            if (event.code == ABS_MT_TRACKING_ID)
            {
                if ((event.value < 0) && (platform.touchSlot < MAX_TOUCH_POINTS))
                {
                    // Touch has ended for this point
                    CORE.Input.Touch.position[platform.touchSlot].x = -1;
                    CORE.Input.Touch.position[platform.touchSlot].y = -1;
                }
            }

            // Touchscreen tap
            if (event.code == ABS_PRESSURE)
            {
                int previousMouseLeftButtonState = platform.currentButtonStateEvdev[MOUSE_BUTTON_LEFT];

                if (!event.value && previousMouseLeftButtonState)
                {
                    platform.currentButtonStateEvdev[MOUSE_BUTTON_LEFT] = 0;
                    touchAction = 0;    // TOUCH_ACTION_UP
                }

                if (event.value && !previousMouseLeftButtonState)
                {
                    platform.currentButtonStateEvdev[MOUSE_BUTTON_LEFT] = 1;
                    touchAction = 1;    // TOUCH_ACTION_DOWN
                }
            }

        }

        // Button parsing
        if (event.type == EV_KEY)
        {
            // Mouse button parsing
            if ((event.code == BTN_TOUCH) || (event.code == BTN_LEFT))
            {
                platform.currentButtonStateEvdev[MOUSE_BUTTON_LEFT] = event.value;

                if (event.value > 0) touchAction = 1;   // TOUCH_ACTION_DOWN
                else touchAction = 0;       // TOUCH_ACTION_UP
            }

            if (event.code == BTN_RIGHT) platform.currentButtonStateEvdev[MOUSE_BUTTON_RIGHT] = event.value;
            if (event.code == BTN_MIDDLE) platform.currentButtonStateEvdev[MOUSE_BUTTON_MIDDLE] = event.value;
            if (event.code == BTN_SIDE) platform.currentButtonStateEvdev[MOUSE_BUTTON_SIDE] = event.value;
            if (event.code == BTN_EXTRA) platform.currentButtonStateEvdev[MOUSE_BUTTON_EXTRA] = event.value;
            if (event.code == BTN_FORWARD) platform.currentButtonStateEvdev[MOUSE_BUTTON_FORWARD] = event.value;
            if (event.code == BTN_BACK) platform.currentButtonStateEvdev[MOUSE_BUTTON_BACK] = event.value;
        }

        // Screen confinement
        if (!CORE.Input.Mouse.cursorHidden)
        {
            if (CORE.Input.Mouse.currentPosition.x < 0) CORE.Input.Mouse.currentPosition.x = 0;
            if (CORE.Input.Mouse.currentPosition.x > CORE.Window.screen.width/CORE.Input.Mouse.scale.x) CORE.Input.Mouse.currentPosition.x = CORE.Window.screen.width/CORE.Input.Mouse.scale.x;

            if (CORE.Input.Mouse.currentPosition.y < 0) CORE.Input.Mouse.currentPosition.y = 0;
            if (CORE.Input.Mouse.currentPosition.y > CORE.Window.screen.height/CORE.Input.Mouse.scale.y) CORE.Input.Mouse.currentPosition.y = CORE.Window.screen.height/CORE.Input.Mouse.scale.y;
        }

        // Update touch point count
        CORE.Input.Touch.pointCount = 0;
        for (int i = 0; i < MAX_TOUCH_POINTS; i++)
        {
            if (CORE.Input.Touch.position[i].x >= 0) CORE.Input.Touch.pointCount++;
        }

#if defined(SUPPORT_GESTURES_SYSTEM)
        if (touchAction > -1)
        {
            GestureEvent gestureEvent = { 0 };

            gestureEvent.touchAction = touchAction;
            gestureEvent.pointCount = CORE.Input.Touch.pointCount;

            for (int i = 0; i < MAX_TOUCH_POINTS; i++)
            {
                gestureEvent.pointId[i] = i;
                gestureEvent.position[i] = CORE.Input.Touch.position[i];
            }

            ProcessGestureEvent(gestureEvent);

            touchAction = -1;
        }
#endif
    }
}

// Search matching DRM mode in connector's mode list
static int FindMatchingConnectorMode(const drmModeConnector *connector, const drmModeModeInfo *mode)
{
    if (NULL == connector) return -1;
    if (NULL == mode) return -1;

    // safe bitwise comparison of two modes
    #define BINCMP(a, b) memcmp((a), (b), (sizeof(a) < sizeof(b))? sizeof(a) : sizeof(b))

    for (size_t i = 0; i < connector->count_modes; i++)
    {
        TRACELOG(LOG_TRACE, "DISPLAY: DRM mode: %d %ux%u@%u %s", i, connector->modes[i].hdisplay, connector->modes[i].vdisplay,
            connector->modes[i].vrefresh, (connector->modes[i].flags & DRM_MODE_FLAG_INTERLACE)? "interlaced" : "progressive");

        if (0 == BINCMP(&platform.crtc->mode, &platform.connector->modes[i])) return i;
    }

    return -1;

    #undef BINCMP
}

// Search exactly matching DRM connector mode in connector's list
static int FindExactConnectorMode(const drmModeConnector *connector, uint width, uint height, uint fps, bool allowInterlaced)
{
    TRACELOG(LOG_TRACE, "DISPLAY: Searching exact connector mode for %ux%u@%u, selecting an interlaced mode is allowed: %s", width, height, fps, allowInterlaced? "yes" : "no");

    if (NULL == connector) return -1;

    for (int i = 0; i < platform.connector->count_modes; i++)
    {
        const drmModeModeInfo *const mode = &platform.connector->modes[i];

        TRACELOG(LOG_TRACE, "DISPLAY: DRM Mode %d %ux%u@%u %s", i, mode->hdisplay, mode->vdisplay, mode->vrefresh, (mode->flags & DRM_MODE_FLAG_INTERLACE)? "interlaced" : "progressive");

        if ((mode->flags & DRM_MODE_FLAG_INTERLACE) && (!allowInterlaced)) continue;

        if ((mode->hdisplay == width) && (mode->vdisplay == height) && (mode->vrefresh == fps)) return i;
    }

    TRACELOG(LOG_TRACE, "DISPLAY: No DRM exact matching mode found");
    return -1;
}

// Search the nearest matching DRM connector mode in connector's list
static int FindNearestConnectorMode(const drmModeConnector *connector, uint width, uint height, uint fps, bool allowInterlaced)
{
    TRACELOG(LOG_TRACE, "DISPLAY: Searching nearest connector mode for %ux%u@%u, selecting an interlaced mode is allowed: %s", width, height, fps, allowInterlaced? "yes" : "no");

    if (NULL == connector) return -1;

    int nearestIndex = -1;
    for (int i = 0; i < platform.connector->count_modes; i++)
    {
        const drmModeModeInfo *const mode = &platform.connector->modes[i];

        TRACELOG(LOG_TRACE, "DISPLAY: DRM mode: %d %ux%u@%u %s", i, mode->hdisplay, mode->vdisplay, mode->vrefresh,
            (mode->flags & DRM_MODE_FLAG_INTERLACE)? "interlaced" : "progressive");

        if ((mode->hdisplay < width) || (mode->vdisplay < height))
        {
            TRACELOG(LOG_TRACE, "DISPLAY: DRM mode is too small");
            continue;
        }

        if ((mode->flags & DRM_MODE_FLAG_INTERLACE) && (!allowInterlaced))
        {
            TRACELOG(LOG_TRACE, "DISPLAY: DRM shouldn't choose an interlaced mode");
            continue;
        }

        if (nearestIndex < 0)
        {
            nearestIndex = i;
            continue;
        }

        const int widthDiff = abs(mode->hdisplay - width);
        const int heightDiff = abs(mode->vdisplay - height);
        const int fpsDiff = abs(mode->vrefresh - fps);

        const int nearestWidthDiff = abs(platform.connector->modes[nearestIndex].hdisplay - width);
        const int nearestHeightDiff = abs(platform.connector->modes[nearestIndex].vdisplay - height);
        const int nearestFpsDiff = abs(platform.connector->modes[nearestIndex].vrefresh - fps);

        if ((widthDiff < nearestWidthDiff) || (heightDiff < nearestHeightDiff) || (fpsDiff < nearestFpsDiff)) nearestIndex = i;
    }

    return nearestIndex;
}

// EOF
