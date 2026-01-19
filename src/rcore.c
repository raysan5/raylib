/**********************************************************************************************
*
*   rcore - Window/display management, Graphic device/context management and input management
*
*   PLATFORMS SUPPORTED:
*       > PLATFORM_DESKTOP_GLFW (GLFW backend):
*           - Windows (Win32, Win64)
*           - Linux (X11/Wayland desktop mode)
*           - macOS/OSX (x64, arm64)
*           - FreeBSD, OpenBSD, NetBSD, DragonFly (X11 desktop)
*       > PLATFORM_DESKTOP_SDL (SDL backend):
*           - Windows (Win32, Win64)
*           - Linux (X11/Wayland desktop mode)
*           - Others (not tested)
*       > PLATFORM_DESKTOP_RGFW (RGFW backend):
*           - Windows (Win32, Win64)
*           - Linux (X11/Wayland desktop mode)
*           - macOS/OSX (x64, arm64)
*           - Others (not tested)
*       > PLATFORM_WEB_RGFW:
*           - HTML5 (WebAssembly)
*       > PLATFORM_WEB:
*           - HTML5 (WebAssembly)
*       > PLATFORM_DRM:
*           - Raspberry Pi 0-5 (DRM/KMS)
*           - Linux DRM subsystem (KMS mode)
*       > PLATFORM_ANDROID:
*           - Android (ARM, ARM64)
*       > PLATFORM_DESKTOP_WIN32 (Native Win32):
*           - Windows (Win32, Win64)
*       > PLATFORM_MEMORY
*           - Memory framebuffer output, using software renderer, no OS required
*   CONFIGURATION:
*       #define SUPPORT_DEFAULT_FONT (default)
*           Default font is loaded on window initialization to be available for the user to render simple text
*           NOTE: If enabled, uses external module functions to load default raylib font (module: text)
*
*       #define SUPPORT_CAMERA_SYSTEM
*           Camera module is included (rcamera.h) and multiple predefined cameras are available:
*               free, 1st/3rd person, orbital, custom
*
*       #define SUPPORT_GESTURES_SYSTEM
*           Gestures module is included (rgestures.h) to support gestures detection: tap, hold, swipe, drag
*
*       #define SUPPORT_MOUSE_GESTURES
*           Mouse gestures are directly mapped like touches and processed by gestures system
*
*       #define SUPPORT_BUSY_WAIT_LOOP
*           Use busy wait loop for timing sync, if not defined, a high-resolution timer is setup and used
*
*       #define SUPPORT_PARTIALBUSY_WAIT_LOOP
*           Use a partial-busy wait loop, in this case frame sleeps for most of the time and runs a busy-wait-loop at the end
*
*       #define SUPPORT_SCREEN_CAPTURE
*           Allow automatic screen capture of current screen pressing F12, defined in KeyCallback()
*
*       #define SUPPORT_COMPRESSION_API
*           Support CompressData() and DecompressData() functions, those functions use zlib implementation
*           provided by stb_image and stb_image_write libraries, so, those libraries must be enabled on textures module
*           for linkage
*
*       #define SUPPORT_AUTOMATION_EVENTS
*           Support automatic events recording and playing, useful for automated testing systems or AI based game playing
*
*   DEPENDENCIES:
*       raymath  - 3D math functionality (Vector2, Vector3, Matrix, Quaternion)
*       camera   - Multiple 3D camera modes (free, orbital, 1st person, 3rd person)
*       gestures - Gestures system for touch-ready devices (or simulated from mouse inputs)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2026 Ramon Santamaria (@raysan5) and contributors
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

//----------------------------------------------------------------------------------
// Feature Test Macros required for this module
//----------------------------------------------------------------------------------
#if (defined(__linux__) || defined(PLATFORM_WEB) || defined(PLATFORM_WEB_RGFW)) && (_XOPEN_SOURCE < 500)
    #undef _XOPEN_SOURCE
    #define _XOPEN_SOURCE 500       // Required for: readlink if compiled with c99 without GNU extensions
#endif

#if (defined(__linux__) || defined(PLATFORM_WEB) || defined(PLATFORM_WEB_RGFW)) && (_POSIX_C_SOURCE < 199309L)
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 199309L // Required for: CLOCK_MONOTONIC if compiled with c99 without GNU extensions
#endif

#include "raylib.h"                 // Declares module functions

#include "config.h"                 // Defines module configuration flags

#include <stdlib.h>                 // Required for: srand(), rand(), atexit(), exit()
#include <stdio.h>                  // Required for: FILE, fopen(), fseek(), ftell(), fread(), fwrite(), fprintf(), vprintf(), fclose(), sprintf() [Used in OpenURL()]
#include <string.h>                 // Required for: strlen(), strncpy(), strcmp(), strrchr(), memset(), strcat()
#include <stdarg.h>                 // Required for: va_list, va_start(), va_end() [Used in TraceLog()]
#include <time.h>                   // Required for: time() [Used in InitTimer()]
#include <math.h>                   // Required for: tan() [Used in BeginMode3D()], atan2f() [Used in LoadVrStereoConfig()]

#if defined(PLATFORM_MEMORY) || defined(PLATFORM_WEB)
    #define SW_GL_FRAMEBUFFER_COPY_BGRA false
#endif
#define RLGL_IMPLEMENTATION
#include "rlgl.h"                   // OpenGL abstraction layer to OpenGL 1.1, 3.3+ or ES2

#define RAYMATH_IMPLEMENTATION
#include "raymath.h"                // Vector2, Vector3, Quaternion and Matrix functionality

#if defined(SUPPORT_GESTURES_SYSTEM)
    #define RGESTURES_IMPLEMENTATION
    #include "rgestures.h"          // Gestures detection functionality
#endif

#if defined(SUPPORT_CAMERA_SYSTEM)
    #define RCAMERA_IMPLEMENTATION
    #include "rcamera.h"            // Camera system functionality
#endif

#if defined(SUPPORT_COMPRESSION_API)
    #define SINFL_IMPLEMENTATION
    #define SINFL_NO_SIMD
    #include "external/sinfl.h"     // Deflate (RFC 1951) decompressor

    #define SDEFL_IMPLEMENTATION
    #include "external/sdefl.h"     // Deflate (RFC 1951) compressor
#endif

#if defined(SUPPORT_RPRAND_GENERATOR)
    #define RPRAND_IMPLEMENTATION
    #include "external/rprand.h"
#endif

#if defined(__linux__) && !defined(_GNU_SOURCE)
    #define _GNU_SOURCE
#endif

// Platform specific defines to handle GetApplicationDirectory()
#if defined(_WIN32)
    #if !defined(MAX_PATH)
        #define MAX_PATH 260
    #endif

    struct HINSTANCE__;
    #if defined(__cplusplus)
    extern "C" {
    #endif
    __declspec(dllimport) unsigned long __stdcall GetModuleFileNameA(struct HINSTANCE__ *hModule, char *lpFilename, unsigned long nSize);
    __declspec(dllimport) unsigned long __stdcall GetModuleFileNameW(struct HINSTANCE__ *hModule, wchar_t *lpFilename, unsigned long nSize);
    __declspec(dllimport) int __stdcall WideCharToMultiByte(unsigned int cp, unsigned long flags, const wchar_t *widestr, int cchwide, char *str, int cbmb, const char *defchar, int *used_default);
    __declspec(dllimport) unsigned int __stdcall timeBeginPeriod(unsigned int uPeriod);
    __declspec(dllimport) unsigned int __stdcall timeEndPeriod(unsigned int uPeriod);
    #if defined(__cplusplus)
    }
    #endif
#elif defined(__linux__)
    #include <unistd.h>
#elif defined(__FreeBSD__)
    #include <sys/types.h>
    #include <sys/sysctl.h>
    #include <unistd.h>
#elif defined(__APPLE__)
    #include <sys/syslimits.h>
    #include <mach-o/dyld.h>
#endif // OSs

#define _CRT_INTERNAL_NONSTDC_NAMES  1
#include <sys/stat.h>               // Required for: stat(), S_ISREG [Used in GetFileModTime(), IsFilePath()]

#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
    #define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

#if defined(_WIN32) && (defined(_MSC_VER) || defined(__TINYC__))
    #define DIRENT_MALLOC RL_MALLOC
    #define DIRENT_FREE RL_FREE

    #include "external/dirent.h"    // Required for: DIR, opendir(), closedir() [Used in LoadDirectoryFiles()]
#else
    #include <dirent.h>             // Required for: DIR, opendir(), closedir() [Used in LoadDirectoryFiles()]
#endif

#if defined(_WIN32)
    #include <io.h>                 // Required for: _access() [Used in FileExists()]
    #include <direct.h>             // Required for: _getch(), _chdir(), _mkdir()
    #define GETCWD _getcwd          // NOTE: MSDN recommends not to use getcwd(), chdir()
    #define CHDIR _chdir
    #define MKDIR(dir) _mkdir(dir)
    #define ACCESS(fn) _access(fn, 0)
#else
    #include <unistd.h>             // Required for: getch(), chdir(), mkdir(), access()
    #define GETCWD getcwd
    #define CHDIR chdir
    #define MKDIR(dir) mkdir(dir, 0777)
    #define ACCESS(fn) access(fn, F_OK)
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#ifndef MAX_TRACELOG_MSG_LENGTH
    #define MAX_TRACELOG_MSG_LENGTH      256        // Max length of one trace-log message
#endif

#ifndef MAX_FILEPATH_CAPACITY
    #define MAX_FILEPATH_CAPACITY       8192        // Maximum capacity for filepath
#endif
#ifndef MAX_FILEPATH_LENGTH
    #if defined(_WIN32)
        #define MAX_FILEPATH_LENGTH      256        // On Win32, MAX_PATH = 260 (limits.h) but Windows 10, Version 1607 enables long paths...
    #else
        #define MAX_FILEPATH_LENGTH     4096        // On Linux, PATH_MAX = 4096 by default (limits.h)
    #endif
#endif

#ifndef MAX_KEYBOARD_KEYS
    #define MAX_KEYBOARD_KEYS            512        // Maximum number of keyboard keys supported
#endif
#ifndef MAX_MOUSE_BUTTONS
    #define MAX_MOUSE_BUTTONS              8        // Maximum number of mouse buttons supported
#endif
#ifndef MAX_GAMEPADS
    #define MAX_GAMEPADS                   4        // Maximum number of gamepads supported
#endif
#ifndef MAX_GAMEPAD_NAME_LENGTH
    #define MAX_GAMEPAD_NAME_LENGTH      128        // Maximum number of characters in a gamepad name (byte size)
#endif
#ifndef MAX_GAMEPAD_AXES
    #define MAX_GAMEPAD_AXES               8        // Maximum number of axes supported (per gamepad)
#endif
#ifndef MAX_GAMEPAD_BUTTONS
    #define MAX_GAMEPAD_BUTTONS           32        // Maximum number of buttons supported (per gamepad)
#endif
#ifndef MAX_GAMEPAD_VIBRATION_TIME
    #define MAX_GAMEPAD_VIBRATION_TIME     2.0f     // Maximum vibration time in seconds
#endif
#ifndef MAX_TOUCH_POINTS
    #define MAX_TOUCH_POINTS               8        // Maximum number of touch points supported
#endif
#ifndef MAX_KEY_PRESSED_QUEUE
    #define MAX_KEY_PRESSED_QUEUE         16        // Maximum number of keys in the key input queue
#endif
#ifndef MAX_CHAR_PRESSED_QUEUE
    #define MAX_CHAR_PRESSED_QUEUE        16        // Maximum number of characters in the char input queue
#endif

#ifndef MAX_DECOMPRESSION_SIZE
    #define MAX_DECOMPRESSION_SIZE        64        // Maximum size allocated for decompression in MB
#endif

#ifndef MAX_AUTOMATION_EVENTS
    #define MAX_AUTOMATION_EVENTS      16384        // Maximum number of automation events to record
#endif

#ifndef DIRECTORY_FILTER_TAG
    #define DIRECTORY_FILTER_TAG       "DIR"        // Name tag used to request directory inclusion on directory scan
#endif                                              // NOTE: Used in ScanDirectoryFiles(), ScanDirectoryFilesRecursively() and LoadDirectoryFilesEx()

// Flags operation macros
#define FLAG_SET(n, f) ((n) |= (f))
#define FLAG_CLEAR(n, f) ((n) &= ~(f))
#define FLAG_TOGGLE(n, f) ((n) ^= (f))
#define FLAG_IS_SET(n, f) (((n) & (f)) == (f))

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct { int x; int y; } Point;
typedef struct { unsigned int width; unsigned int height; } Size;

// Core global state context data
typedef struct CoreData {
    struct {
        const char *title;                  // Window text title const pointer
        unsigned int flags;                 // Configuration flags (bit based), keeps window state
        bool ready;                         // Check if window has been initialized successfully
        bool shouldClose;                   // Check if window set for closing
        bool resizedLastFrame;              // Check if window has been resized last frame
        bool eventWaiting;                  // Wait for events before ending frame
        bool usingFbo;                      // Using FBO (RenderTexture) for rendering instead of default framebuffer

        Size display;                       // Display width and height (monitor, device-screen, LCD, ...)
        Size screen;                        // Screen current width and height
        Point position;                     // Window current position
        Size previousScreen;                // Screen previous width and height (required on fullscreen/borderless-windowed toggle)
        Point previousPosition;             // Window previous position (required on fullscreeen/borderless-windowed toggle)
        Size render;                        // Screen framebuffer width and height
        Point renderOffset;                 // Screen framebuffer render offset (Not required anymore?)
        Size currentFbo;                    // Current framebuffer render width and height (depends on active render texture)
        Size screenMin;                     // Screen minimum width and height (for resizable window)
        Size screenMax;                     // Screen maximum width and height (for resizable window)
        Matrix screenScale;                 // Matrix to scale screen (framebuffer rendering)

        char **dropFilepaths;               // Store dropped files paths pointers (provided by GLFW)
        unsigned int dropFileCount;         // Count dropped files strings

    } Window;
    struct {
        const char *basePath;               // Base path for data storage

    } Storage;
    struct {
        struct {
            int exitKey;                    // Default exit key
            char currentKeyState[MAX_KEYBOARD_KEYS]; // Registers current frame key state
            char previousKeyState[MAX_KEYBOARD_KEYS]; // Registers previous frame key state

            // NOTE: Since key press logic involves comparing previous vs currrent key state,
            // key repeats needs to be handled specially
            char keyRepeatInFrame[MAX_KEYBOARD_KEYS]; // Registers key repeats for current frame

            int keyPressedQueue[MAX_KEY_PRESSED_QUEUE]; // Input keys queue
            int keyPressedQueueCount;       // Input keys queue count

            int charPressedQueue[MAX_CHAR_PRESSED_QUEUE]; // Input characters queue (unicode)
            int charPressedQueueCount;      // Input characters queue count

        } Keyboard;
        struct {
            Vector2 offset;                 // Mouse offset
            Vector2 scale;                  // Mouse scaling
            Vector2 currentPosition;        // Mouse position on screen
            Vector2 previousPosition;       // Previous mouse position
            Vector2 lockedPosition;         // Mouse position when locked

            int cursor;                     // Tracks current mouse cursor
            bool cursorHidden;              // Track if cursor is hidden
            bool cursorLocked;              // Track if cursor is locked (disabled)
            bool cursorOnScreen;            // Tracks if cursor is inside client area

            char currentButtonState[MAX_MOUSE_BUTTONS]; // Registers current mouse button state
            char previousButtonState[MAX_MOUSE_BUTTONS]; // Registers previous mouse button state
            Vector2 currentWheelMove;       // Registers current mouse wheel variation
            Vector2 previousWheelMove;      // Registers previous mouse wheel variation

        } Mouse;
        struct {
            int pointCount;                                 // Number of touch points active
            int pointId[MAX_TOUCH_POINTS];                  // Point identifiers
            Vector2 position[MAX_TOUCH_POINTS];             // Touch position on screen
            Vector2 previousPosition[MAX_TOUCH_POINTS];     // Previous touch position on screen
            char currentTouchState[MAX_TOUCH_POINTS];       // Registers current touch state
            char previousTouchState[MAX_TOUCH_POINTS];      // Registers previous touch state

        } Touch;
        struct {
            int lastButtonPressed;          // Register last gamepad button pressed
            int axisCount[MAX_GAMEPADS];    // Register number of available gamepad axes
            bool ready[MAX_GAMEPADS];       // Flag to know if gamepad is ready
            char name[MAX_GAMEPADS][MAX_GAMEPAD_NAME_LENGTH];               // Gamepad name holder
            char currentButtonState[MAX_GAMEPADS][MAX_GAMEPAD_BUTTONS];     // Current gamepad buttons state
            char previousButtonState[MAX_GAMEPADS][MAX_GAMEPAD_BUTTONS];    // Previous gamepad buttons state
            float axisState[MAX_GAMEPADS][MAX_GAMEPAD_AXES];                // Gamepad axes state

        } Gamepad;
    } Input;
    struct {
        double current;                     // Current time measure
        double previous;                    // Previous time measure
        double update;                      // Time measure for frame update
        double draw;                        // Time measure for frame draw
        double frame;                       // Time measure for one frame
        double target;                      // Desired time for one frame, if 0 not applied
        unsigned long long int base;        // Base time measure for hi-res timer (PLATFORM_ANDROID, PLATFORM_DRM)
        unsigned int frameCounter;          // Frame counter

    } Time;
} CoreData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
RLAPI const char *raylib_version = RAYLIB_VERSION;  // raylib version exported symbol, required for some bindings

CoreData CORE = { 0 };                              // Global CORE state context

static int logTypeLevel = LOG_INFO;                 // Minimum log type level

static TraceLogCallback traceLog = NULL;            // TraceLog callback function pointer
static LoadFileDataCallback loadFileData = NULL;    // LoadFileData callback function pointer
static SaveFileDataCallback saveFileData = NULL;    // SaveFileText callback function pointer
static LoadFileTextCallback loadFileText = NULL;    // LoadFileText callback function pointer
static SaveFileTextCallback saveFileText = NULL;    // SaveFileText callback function pointer

#if defined(SUPPORT_SCREEN_CAPTURE)
static int screenshotCounter = 0;                   // Screenshots counter
#endif

#if defined(SUPPORT_AUTOMATION_EVENTS)
// Automation events type
typedef enum AutomationEventType {
    EVENT_NONE = 0,
    // Input events
    INPUT_KEY_UP,                   // param[0]: key
    INPUT_KEY_DOWN,                 // param[0]: key
    INPUT_KEY_PRESSED,              // param[0]: key
    INPUT_KEY_RELEASED,             // param[0]: key
    INPUT_MOUSE_BUTTON_UP,          // param[0]: button
    INPUT_MOUSE_BUTTON_DOWN,        // param[0]: button
    INPUT_MOUSE_POSITION,           // param[0]: x, param[1]: y
    INPUT_MOUSE_WHEEL_MOTION,       // param[0]: x delta, param[1]: y delta
    INPUT_GAMEPAD_CONNECT,          // param[0]: gamepad
    INPUT_GAMEPAD_DISCONNECT,       // param[0]: gamepad
    INPUT_GAMEPAD_BUTTON_UP,        // param[0]: button
    INPUT_GAMEPAD_BUTTON_DOWN,      // param[0]: button
    INPUT_GAMEPAD_AXIS_MOTION,      // param[0]: axis, param[1]: delta
    INPUT_TOUCH_UP,                 // param[0]: id
    INPUT_TOUCH_DOWN,               // param[0]: id
    INPUT_TOUCH_POSITION,           // param[0]: x, param[1]: y
    INPUT_GESTURE,                  // param[0]: gesture
    // Window events
    WINDOW_CLOSE,                   // no params
    WINDOW_MAXIMIZE,                // no params
    WINDOW_MINIMIZE,                // no params
    WINDOW_RESIZE,                  // param[0]: width, param[1]: height
    // Custom events
    ACTION_TAKE_SCREENSHOT,         // no params
    ACTION_SETTARGETFPS             // param[0]: fps
} AutomationEventType;

// Event type to config events flags
// WARNING: Not used at the moment
typedef enum {
    EVENT_INPUT_KEYBOARD    = 0,
    EVENT_INPUT_MOUSE       = 1,
    EVENT_INPUT_GAMEPAD     = 2,
    EVENT_INPUT_TOUCH       = 4,
    EVENT_INPUT_GESTURE     = 8,
    EVENT_WINDOW            = 16,
    EVENT_CUSTOM            = 32
} EventType;

// Event type name strings, required for export
static const char *autoEventTypeName[] = {
    "EVENT_NONE",
    "INPUT_KEY_UP",
    "INPUT_KEY_DOWN",
    "INPUT_KEY_PRESSED",
    "INPUT_KEY_RELEASED",
    "INPUT_MOUSE_BUTTON_UP",
    "INPUT_MOUSE_BUTTON_DOWN",
    "INPUT_MOUSE_POSITION",
    "INPUT_MOUSE_WHEEL_MOTION",
    "INPUT_GAMEPAD_CONNECT",
    "INPUT_GAMEPAD_DISCONNECT",
    "INPUT_GAMEPAD_BUTTON_UP",
    "INPUT_GAMEPAD_BUTTON_DOWN",
    "INPUT_GAMEPAD_AXIS_MOTION",
    "INPUT_TOUCH_UP",
    "INPUT_TOUCH_DOWN",
    "INPUT_TOUCH_POSITION",
    "INPUT_GESTURE",
    "WINDOW_CLOSE",
    "WINDOW_MAXIMIZE",
    "WINDOW_MINIMIZE",
    "WINDOW_RESIZE",
    "ACTION_TAKE_SCREENSHOT",
    "ACTION_SETTARGETFPS"
};

/*
// Automation event (24 bytes)
// NOTE: Opaque struct, internal to raylib
struct AutomationEvent {
    unsigned int frame;                 // Event frame
    unsigned int type;                  // Event type (AutomationEventType)
    int params[4];                      // Event parameters (if required)
};
*/

static AutomationEventList *currentEventList = NULL;        // Current automation events list, set by user, keep internal pointer
static bool automationEventRecording = false;               // Recording automation events flag
//static short automationEventEnabled = 0b0000001111111111; // TODO: Automation events enabled for recording/playing
#endif
//-----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Module Functions Declaration
// NOTE: Those functions are common for all platforms!
//----------------------------------------------------------------------------------

#if defined(SUPPORT_MODULE_RTEXT) && defined(SUPPORT_DEFAULT_FONT)
extern void LoadFontDefault(void);      // [Module: text] Loads default font on InitWindow()
extern void UnloadFontDefault(void);    // [Module: text] Unloads default font from GPU memory
#endif

extern int InitPlatform(void);          // Initialize platform (graphics, inputs and more)
extern void ClosePlatform(void);        // Close platform

static void InitTimer(void);                                // Initialize timer, hi-resolution if available (required by InitPlatform())
static void SetupViewport(int width, int height);           // Set viewport for a provided width and height

static void ScanDirectoryFiles(const char *basePath, FilePathList *list, const char *filter);   // Scan all files and directories in a base path
static void ScanDirectoryFilesRecursively(const char *basePath, FilePathList *list, const char *filter);  // Scan all files and directories recursively from a base path

#if defined(SUPPORT_AUTOMATION_EVENTS)
static void RecordAutomationEvent(void); // Record frame events (to internal events array)
#endif

#if defined(_WIN32) && !defined(PLATFORM_DESKTOP_RGFW)
// NOTE: We declare Sleep() function symbol to avoid including windows.h (kernel32.lib linkage required)
__declspec(dllimport) void __stdcall Sleep(unsigned long msTimeout); // Required for: WaitTime()
#endif

#if !defined(SUPPORT_MODULE_RTEXT)
const char *TextFormat(const char *text, ...); // Formatting of text with variables to 'embed'
#endif // !SUPPORT_MODULE_RTEXT

#if defined(PLATFORM_DESKTOP)
    #define PLATFORM_DESKTOP_GLFW
#endif

// Using '#pragma message' because '#warning' is not adopted by MSVC
#if defined(SUPPORT_CLIPBOARD_IMAGE)
    #if !defined(SUPPORT_MODULE_RTEXTURES)
        #pragma message ("WARNING: Enabling SUPPORT_CLIPBOARD_IMAGE requires SUPPORT_MODULE_RTEXTURES to work properly")
    #endif

    // It's nice to have support Bitmap on Linux as well, but not as necessary as Windows
    #if !defined(SUPPORT_FILEFORMAT_BMP) && defined(_WIN32)
        #pragma message ("WARNING: Enabling SUPPORT_CLIPBOARD_IMAGE requires SUPPORT_FILEFORMAT_BMP, specially on Windows")
    #endif

    // From what I've tested applications on Wayland saves images on clipboard as PNG
    #if (!defined(SUPPORT_FILEFORMAT_PNG) || !defined(SUPPORT_FILEFORMAT_JPG)) && !defined(_WIN32)
        #pragma message ("WARNING: Getting image from the clipboard might not work without SUPPORT_FILEFORMAT_PNG or SUPPORT_FILEFORMAT_JPG")
    #endif
#endif // SUPPORT_CLIPBOARD_IMAGE

// Include platform-specific submodules
#if defined(PLATFORM_DESKTOP_GLFW)
    #include "platforms/rcore_desktop_glfw.c"
#elif defined(PLATFORM_DESKTOP_SDL)
    #include "platforms/rcore_desktop_sdl.c"
#elif (defined(PLATFORM_DESKTOP_RGFW) || defined(PLATFORM_WEB_RGFW))
    #include "platforms/rcore_desktop_rgfw.c"
#elif defined(PLATFORM_DESKTOP_WIN32)
    #include "platforms/rcore_desktop_win32.c"
#elif defined(PLATFORM_WEB)
    #include "platforms/rcore_web.c"
#elif defined(PLATFORM_DRM)
    #include "platforms/rcore_drm.c"
#elif defined(PLATFORM_ANDROID)
    #include "platforms/rcore_android.c"
#elif defined(PLATFORM_MEMORY)
    #include "platforms/rcore_memory.c"
#else
    // TODO: Include your custom platform backend!
    // i.e software rendering backend or console backend!
    #pragma message ("WARNING: No [rcore] platform defined")
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition: Window and Graphics Device
//----------------------------------------------------------------------------------

// NOTE: Functions with a platform-specific implementation on rcore_<platform>.c
//bool WindowShouldClose(void)
//void ToggleFullscreen(void)
//void ToggleBorderlessWindowed(void)
//void MaximizeWindow(void)
//void MinimizeWindow(void)
//void RestoreWindow(void)

//void SetWindowState(unsigned int flags)
//void ClearWindowState(unsigned int flags)

//void SetWindowIcon(Image image)
//void SetWindowIcons(Image *images, int count)
//void SetWindowTitle(const char *title)
//void SetWindowPosition(int x, int y)
//void SetWindowMonitor(int monitor)
//void SetWindowMinSize(int width, int height)
//void SetWindowMaxSize(int width, int height)
//void SetWindowSize(int width, int height)
//void SetWindowOpacity(float opacity)
//void SetWindowFocused(void)
//void *GetWindowHandle(void)
//Vector2 GetWindowPosition(void)
//Vector2 GetWindowScaleDPI(void)

//int GetMonitorCount(void)
//int GetCurrentMonitor(void)
//int GetMonitorWidth(int monitor)
//int GetMonitorHeight(int monitor)
//int GetMonitorPhysicalWidth(int monitor)
//int GetMonitorPhysicalHeight(int monitor)
//int GetMonitorRefreshRate(int monitor)
//Vector2 GetMonitorPosition(int monitor)
//const char *GetMonitorName(int monitor)

//void SetClipboardText(const char *text)
//const char *GetClipboardText(void)

//void ShowCursor(void)
//void HideCursor(void)
//void EnableCursor(void)
//void DisableCursor(void)

// Initialize window and OpenGL context
void InitWindow(int width, int height, const char *title)
{
    TRACELOG(LOG_INFO, "Initializing raylib %s", RAYLIB_VERSION);

#if defined(PLATFORM_DESKTOP_GLFW)
    TRACELOG(LOG_INFO, "Platform backend: DESKTOP (GLFW)");
#elif defined(PLATFORM_DESKTOP_SDL)
    TRACELOG(LOG_INFO, "Platform backend: DESKTOP (SDL)");
#elif defined(PLATFORM_DESKTOP_RGFW)
    TRACELOG(LOG_INFO, "Platform backend: DESKTOP (RGFW)");
#elif defined(PLATFORM_DESKTOP_WIN32)
    TRACELOG(LOG_INFO, "Platform backend: DESKTOP (WIN32)");
#elif defined(PLATFORM_WEB_RGFW)
    TRACELOG(LOG_INFO, "Platform backend: WEB (RGFW) (HTML5)");
#elif defined(PLATFORM_WEB)
    TRACELOG(LOG_INFO, "Platform backend: WEB (HTML5)");
#elif defined(PLATFORM_DRM)
    TRACELOG(LOG_INFO, "Platform backend: NATIVE DRM");
#elif defined(PLATFORM_ANDROID)
    TRACELOG(LOG_INFO, "Platform backend: ANDROID");
#elif defined(PLATFORM_MEMORY)
    TRACELOG(LOG_INFO, "Platform backend: MEMORY (No OS)");
#else
    // TODO: Include your custom platform backend!
    // i.e software rendering backend or console backend!
    TRACELOG(LOG_INFO, "Platform backend: CUSTOM");
#endif

    TRACELOG(LOG_INFO, "Supported raylib modules:");
    TRACELOG(LOG_INFO, "    > rcore:..... loaded (mandatory)");
    TRACELOG(LOG_INFO, "    > rlgl:...... loaded (mandatory)");
#if defined(SUPPORT_MODULE_RSHAPES)
    TRACELOG(LOG_INFO, "    > rshapes:... loaded (optional)");
#else
    TRACELOG(LOG_INFO, "    > rshapes:... not loaded (optional)");
#endif
#if defined(SUPPORT_MODULE_RTEXTURES)
    TRACELOG(LOG_INFO, "    > rtextures:. loaded (optional)");
#else
    TRACELOG(LOG_INFO, "    > rtextures:. not loaded (optional)");
#endif
#if defined(SUPPORT_MODULE_RTEXT)
    TRACELOG(LOG_INFO, "    > rtext:..... loaded (optional)");
#else
    TRACELOG(LOG_INFO, "    > rtext:..... not loaded (optional)");
#endif
#if defined(SUPPORT_MODULE_RMODELS)
    TRACELOG(LOG_INFO, "    > rmodels:... loaded (optional)");
#else
    TRACELOG(LOG_INFO, "    > rmodels:... not loaded (optional)");
#endif
#if defined(SUPPORT_MODULE_RAUDIO)
    TRACELOG(LOG_INFO, "    > raudio:.... loaded (optional)");
#else
    TRACELOG(LOG_INFO, "    > raudio:.... not loaded (optional)");
#endif

    // Initialize window data
    CORE.Window.screen.width = width;
    CORE.Window.screen.height = height;
    CORE.Window.currentFbo.width = CORE.Window.screen.width;
    CORE.Window.currentFbo.height = CORE.Window.screen.height;

    CORE.Window.eventWaiting = false;
    CORE.Window.screenScale = MatrixIdentity(); // No draw scaling required by default
    if ((title != NULL) && (title[0] != 0)) CORE.Window.title = title;

    // Initialize global input state
    memset(&CORE.Input, 0, sizeof(CORE.Input)); // Reset CORE.Input structure to 0
    CORE.Input.Keyboard.exitKey = KEY_ESCAPE;
    CORE.Input.Mouse.scale = (Vector2){ 1.0f, 1.0f };
    CORE.Input.Mouse.cursor = MOUSE_CURSOR_ARROW;
    CORE.Input.Gamepad.lastButtonPressed = GAMEPAD_BUTTON_UNKNOWN;

    // Initialize platform
    //--------------------------------------------------------------
    int result = InitPlatform();

    if (result != 0)
    {
        TRACELOG(LOG_WARNING, "SYSTEM: Failed to initialize platform");
        return;
    }
    //--------------------------------------------------------------

    // Initialize rlgl default data (buffers and shaders)
    // NOTE: Current fbo size stored as globals in rlgl for convenience
    rlglInit(CORE.Window.currentFbo.width, CORE.Window.currentFbo.height);

    // Setup default viewport
    SetupViewport(CORE.Window.currentFbo.width, CORE.Window.currentFbo.height);

#if defined(SUPPORT_MODULE_RTEXT)
    #if defined(SUPPORT_DEFAULT_FONT)
        // Load default font
        // WARNING: External function: Module required: rtext
        LoadFontDefault();
        #if defined(SUPPORT_MODULE_RSHAPES)
        // Set font white rectangle for shapes drawing, so shapes and text can be batched together
        // WARNING: rshapes module is required, if not available, default internal white rectangle is used
        Rectangle rec = GetFontDefault().recs[95];
        if (FLAG_IS_SET(CORE.Window.flags, FLAG_MSAA_4X_HINT))
        {
            // NOTE: We try to maxime rec padding to avoid pixel bleeding on MSAA filtering
            SetShapesTexture(GetFontDefault().texture, (Rectangle){ rec.x + 2, rec.y + 2, 1, 1 });
        }
        else
        {
            // NOTE: We set up a 1px padding on char rectangle to avoid pixel bleeding
            SetShapesTexture(GetFontDefault().texture, (Rectangle){ rec.x + 1, rec.y + 1, rec.width - 2, rec.height - 2 });
        }
        #endif
    #endif
#else
    #if defined(SUPPORT_MODULE_RSHAPES)
    // Set default texture and rectangle to be used for shapes drawing
    // NOTE: rlgl default texture is a 1x1 pixel UNCOMPRESSED_R8G8B8A8
    Texture2D texture = { rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
    SetShapesTexture(texture, (Rectangle){ 0.0f, 0.0f, 1.0f, 1.0f });    // WARNING: Module required: rshapes
    #endif
#endif

    CORE.Time.frameCounter = 0;
    CORE.Window.shouldClose = false;

    // Initialize random seed
    SetRandomSeed((unsigned int)time(NULL));

    TRACELOG(LOG_INFO, "SYSTEM: Working Directory: %s", GetWorkingDirectory());
}

// Close window and unload OpenGL context
void CloseWindow(void)
{
#if defined(SUPPORT_MODULE_RTEXT) && defined(SUPPORT_DEFAULT_FONT)
    UnloadFontDefault();        // WARNING: Module required: rtext
#endif

    rlglClose();                // De-init rlgl

    // De-initialize platform
    //--------------------------------------------------------------
    ClosePlatform();
    //--------------------------------------------------------------

    CORE.Window.ready = false;
    TRACELOG(LOG_INFO, "Window closed successfully");
}

// Check if window has been initialized successfully
bool IsWindowReady(void)
{
    return CORE.Window.ready;
}

// Check if window is currently fullscreen
bool IsWindowFullscreen(void)
{
    return FLAG_IS_SET(CORE.Window.flags, FLAG_FULLSCREEN_MODE);
}

// Check if window is currently hidden
bool IsWindowHidden(void)
{
    return FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIDDEN);
}

// Check if window has been minimized
bool IsWindowMinimized(void)
{
    return FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MINIMIZED);
}

// Check if window has been maximized
bool IsWindowMaximized(void)
{
    return FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_MAXIMIZED);
}

// Check if window has the focus
bool IsWindowFocused(void)
{
    return !FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_UNFOCUSED);
}

// Check if window has been resizedLastFrame
bool IsWindowResized(void)
{
    return CORE.Window.resizedLastFrame;
}

// Check if one specific window flag is enabled
bool IsWindowState(unsigned int flag)
{
    return FLAG_IS_SET(CORE.Window.flags, flag);
}

// Get current screen width
int GetScreenWidth(void)
{
    return CORE.Window.screen.width;
}

// Get current screen height
int GetScreenHeight(void)
{
    return CORE.Window.screen.height;
}

// Get current render width which is equal to screen width*dpi scale
int GetRenderWidth(void)
{
    int width = 0;

    if (CORE.Window.usingFbo) return CORE.Window.currentFbo.width;
    else width = CORE.Window.render.width;

    return width;
}

// Get current screen height which is equal to screen height*dpi scale
int GetRenderHeight(void)
{
    int height = 0;

    if (CORE.Window.usingFbo) return CORE.Window.currentFbo.height;
    else height = CORE.Window.render.height;

    return height;
}

// Enable waiting for events on EndDrawing(), no automatic event polling
void EnableEventWaiting(void)
{
    CORE.Window.eventWaiting = true;
}

// Disable waiting for events on EndDrawing(), automatic events polling
void DisableEventWaiting(void)
{
    CORE.Window.eventWaiting = false;
}

// Check if cursor is not visible
bool IsCursorHidden(void)
{
    return CORE.Input.Mouse.cursorHidden;
}

// Check if cursor is on the current screen
bool IsCursorOnScreen(void)
{
    return CORE.Input.Mouse.cursorOnScreen;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Screen Drawing
//----------------------------------------------------------------------------------

// Set background color (framebuffer clear color)
void ClearBackground(Color color)
{
    rlClearColor(color.r, color.g, color.b, color.a);   // Set clear color
    rlClearScreenBuffers();                             // Clear current framebuffers
}

// Setup canvas (framebuffer) to start drawing
void BeginDrawing(void)
{
    // WARNING: Previously to BeginDrawing() other render textures drawing could happen,
    // consequently the measure for update vs draw is not accurate (only the total frame time is accurate)

    CORE.Time.current = GetTime();      // Number of elapsed seconds since InitTimer()
    CORE.Time.update = CORE.Time.current - CORE.Time.previous;
    CORE.Time.previous = CORE.Time.current;

    rlLoadIdentity();                   // Reset current matrix (modelview)
    rlMultMatrixf(MatrixToFloat(CORE.Window.screenScale)); // Apply screen scaling

    //rlTranslatef(0.375, 0.375, 0);    // HACK to have 2D pixel-perfect drawing on OpenGL 1.1
                                        // NOTE: Not required with OpenGL 3.3+
}

// End canvas drawing and swap buffers (double buffering)
void EndDrawing(void)
{
    rlDrawRenderBatchActive();      // Update and draw internal render batch

#if defined(SUPPORT_AUTOMATION_EVENTS)
    if (automationEventRecording) RecordAutomationEvent();    // Event recording
#endif

#if !defined(SUPPORT_CUSTOM_FRAME_CONTROL)
    SwapScreenBuffer();                  // Copy back buffer to front buffer (screen)

    // Frame time control system
    CORE.Time.current = GetTime();
    CORE.Time.draw = CORE.Time.current - CORE.Time.previous;
    CORE.Time.previous = CORE.Time.current;

    CORE.Time.frame = CORE.Time.update + CORE.Time.draw;

    // Wait for some milliseconds...
    if (CORE.Time.frame < CORE.Time.target)
    {
        WaitTime(CORE.Time.target - CORE.Time.frame);

        CORE.Time.current = GetTime();
        double waitTime = CORE.Time.current - CORE.Time.previous;
        CORE.Time.previous = CORE.Time.current;

        CORE.Time.frame += waitTime;    // Total frame time: update + draw + wait
    }

    PollInputEvents();      // Poll user events (before next frame update)
#endif

#if defined(SUPPORT_SCREEN_CAPTURE)
    if (IsKeyPressed(KEY_F12))
    {
        TakeScreenshot(TextFormat("screenshot%03i.png", screenshotCounter));
        screenshotCounter++;
    }
#endif  // SUPPORT_SCREEN_CAPTURE

    CORE.Time.frameCounter++;
}

// Initialize 2D mode with custom camera (2D)
void BeginMode2D(Camera2D camera)
{
    rlDrawRenderBatchActive();      // Update and draw internal render batch

    rlLoadIdentity();               // Reset current matrix (modelview)

    // Apply 2d camera transformation to modelview
    rlMultMatrixf(MatrixToFloat(GetCameraMatrix2D(camera)));
}

// Ends 2D mode with custom camera
void EndMode2D(void)
{
    rlDrawRenderBatchActive();      // Update and draw internal render batch

    rlLoadIdentity();               // Reset current matrix (modelview)

    if (rlGetActiveFramebuffer() == 0) rlMultMatrixf(MatrixToFloat(CORE.Window.screenScale)); // Apply screen scaling if required
}

// Initializes 3D mode with custom camera (3D)
void BeginMode3D(Camera camera)
{
    rlDrawRenderBatchActive();      // Update and draw internal render batch

    rlMatrixMode(RL_PROJECTION);    // Switch to projection matrix
    rlPushMatrix();                 // Save previous matrix, which contains the settings for the 2d ortho projection
    rlLoadIdentity();               // Reset current matrix (projection)

    float aspect = (float)CORE.Window.currentFbo.width/(float)CORE.Window.currentFbo.height;

    // NOTE: zNear and zFar values are important when computing depth buffer values
    if (camera.projection == CAMERA_PERSPECTIVE)
    {
        // Setup perspective projection
        double top = rlGetCullDistanceNear()*tan(camera.fovy*0.5*DEG2RAD);
        double right = top*aspect;

        rlFrustum(-right, right, -top, top, rlGetCullDistanceNear(), rlGetCullDistanceFar());
    }
    else if (camera.projection == CAMERA_ORTHOGRAPHIC)
    {
        // Setup orthographic projection
        double top = camera.fovy/2.0;
        double right = top*aspect;

        rlOrtho(-right, right, -top,top, rlGetCullDistanceNear(), rlGetCullDistanceFar());
    }

    rlMatrixMode(RL_MODELVIEW);     // Switch back to modelview matrix
    rlLoadIdentity();               // Reset current matrix (modelview)

    // Setup Camera view
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
    rlMultMatrixf(MatrixToFloat(matView));      // Multiply modelview matrix by view matrix (camera)

    rlEnableDepthTest();            // Enable DEPTH_TEST for 3D
}

// Ends 3D mode and returns to default 2D orthographic mode
void EndMode3D(void)
{
    rlDrawRenderBatchActive();      // Update and draw internal render batch

    rlMatrixMode(RL_PROJECTION);    // Switch to projection matrix
    rlPopMatrix();                  // Restore previous matrix (projection) from matrix stack

    rlMatrixMode(RL_MODELVIEW);     // Switch back to modelview matrix
    rlLoadIdentity();               // Reset current matrix (modelview)

    if (rlGetActiveFramebuffer() == 0) rlMultMatrixf(MatrixToFloat(CORE.Window.screenScale)); // Apply screen scaling if required

    rlDisableDepthTest();           // Disable DEPTH_TEST for 2D
}

// Initializes render texture for drawing
void BeginTextureMode(RenderTexture2D target)
{
    rlDrawRenderBatchActive();      // Update and draw internal render batch

    rlEnableFramebuffer(target.id); // Enable render target

    // Set viewport and RLGL internal framebuffer size
    rlViewport(0, 0, target.texture.width, target.texture.height);
    rlSetFramebufferWidth(target.texture.width);
    rlSetFramebufferHeight(target.texture.height);

    rlMatrixMode(RL_PROJECTION);    // Switch to projection matrix
    rlLoadIdentity();               // Reset current matrix (projection)

    // Set orthographic projection to current framebuffer size
    // NOTE: Configured top-left corner as (0, 0)
    rlOrtho(0, target.texture.width, target.texture.height, 0, 0.0f, 1.0f);

    rlMatrixMode(RL_MODELVIEW);     // Switch back to modelview matrix
    rlLoadIdentity();               // Reset current matrix (modelview)

    //rlScalef(0.0f, -1.0f, 0.0f);  // Flip Y-drawing (?)

    // Setup current width/height for proper aspect ratio
    // calculation when using BeginTextureMode()
    CORE.Window.currentFbo.width = target.texture.width;
    CORE.Window.currentFbo.height = target.texture.height;
    CORE.Window.usingFbo = true;
}

// Ends drawing to render texture
void EndTextureMode(void)
{
    rlDrawRenderBatchActive();      // Update and draw internal render batch

    rlDisableFramebuffer();         // Disable render target (fbo)

    // Set viewport to default framebuffer size
    SetupViewport(CORE.Window.render.width, CORE.Window.render.height);

    // Go back to the modelview state from BeginDrawing since we are back to the default FBO
    rlMatrixMode(RL_MODELVIEW);     // Switch back to modelview matrix
    rlLoadIdentity();               // Reset current matrix (modelview)
    rlMultMatrixf(MatrixToFloat(CORE.Window.screenScale)); // Apply screen scaling if required

    // Reset current fbo to screen size
    CORE.Window.currentFbo.width = CORE.Window.render.width;
    CORE.Window.currentFbo.height = CORE.Window.render.height;
    CORE.Window.usingFbo = false;
}

// Begin custom shader mode
void BeginShaderMode(Shader shader)
{
    rlSetShader(shader.id, shader.locs);
}

// End custom shader mode (returns to default shader)
void EndShaderMode(void)
{
    rlSetShader(rlGetShaderIdDefault(), rlGetShaderLocsDefault());
}

// Begin blending mode (alpha, additive, multiplied, subtract, custom)
// NOTE: Blend modes supported are enumerated in BlendMode enum
void BeginBlendMode(int mode)
{
    rlSetBlendMode(mode);
}

// End blending mode (reset to default: alpha blending)
void EndBlendMode(void)
{
    rlSetBlendMode(BLEND_ALPHA);
}

// Begin scissor mode (define screen area for following drawing)
// NOTE: Scissor rec refers to bottom-left corner, we change it to upper-left
void BeginScissorMode(int x, int y, int width, int height)
{
    rlDrawRenderBatchActive();      // Update and draw internal render batch

    rlEnableScissorTest();

#if defined(__APPLE__)
    if (!CORE.Window.usingFbo)
    {
        Vector2 scale = GetWindowScaleDPI();
        rlScissor((int)(x*scale.x), (int)(GetScreenHeight()*scale.y - (((y + height)*scale.y))), (int)(width*scale.x), (int)(height*scale.y));
    }
#else
    if (!CORE.Window.usingFbo && FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIGHDPI))
    {
        Vector2 scale = GetWindowScaleDPI();
        rlScissor((int)(x*scale.x), (int)(CORE.Window.currentFbo.height - (y + height)*scale.y), (int)(width*scale.x), (int)(height*scale.y));
    }
#endif
    else
    {
        rlScissor(x, CORE.Window.currentFbo.height - (y + height), width, height);
    }
}

// End scissor mode
void EndScissorMode(void)
{
    rlDrawRenderBatchActive();      // Update and draw internal render batch
    rlDisableScissorTest();
}

//----------------------------------------------------------------------------------
// Module Functions Definition: VR Stereo Rendering
//----------------------------------------------------------------------------------

// Begin VR drawing configuration
void BeginVrStereoMode(VrStereoConfig config)
{
    rlEnableStereoRender();

    // Set stereo render matrices
    rlSetMatrixProjectionStereo(config.projection[0], config.projection[1]);
    rlSetMatrixViewOffsetStereo(config.viewOffset[0], config.viewOffset[1]);
}

// End VR drawing process (and desktop mirror)
void EndVrStereoMode(void)
{
    rlDisableStereoRender();
}

// Load VR stereo config for VR simulator device parameters
VrStereoConfig LoadVrStereoConfig(VrDeviceInfo device)
{
    VrStereoConfig config = { 0 };

    if (rlGetVersion() != RL_OPENGL_11)
    {
        // Compute aspect ratio
        float aspect = ((float)device.hResolution*0.5f)/(float)device.vResolution;

        // Compute lens parameters
        float lensShift = (device.hScreenSize*0.25f - device.lensSeparationDistance*0.5f)/device.hScreenSize;
        config.leftLensCenter[0] = 0.25f + lensShift;
        config.leftLensCenter[1] = 0.5f;
        config.rightLensCenter[0] = 0.75f - lensShift;
        config.rightLensCenter[1] = 0.5f;
        config.leftScreenCenter[0] = 0.25f;
        config.leftScreenCenter[1] = 0.5f;
        config.rightScreenCenter[0] = 0.75f;
        config.rightScreenCenter[1] = 0.5f;

        // Compute distortion scale parameters
        // NOTE: To get lens max radius, lensShift must be normalized to [-1..1]
        float lensRadius = fabsf(-1.0f - 4.0f*lensShift);
        float lensRadiusSq = lensRadius*lensRadius;
        float distortionScale = device.lensDistortionValues[0] +
                                device.lensDistortionValues[1]*lensRadiusSq +
                                device.lensDistortionValues[2]*lensRadiusSq*lensRadiusSq +
                                device.lensDistortionValues[3]*lensRadiusSq*lensRadiusSq*lensRadiusSq;

        float normScreenWidth = 0.5f;
        float normScreenHeight = 1.0f;
        config.scaleIn[0] = 2.0f/normScreenWidth;
        config.scaleIn[1] = 2.0f/normScreenHeight/aspect;
        config.scale[0] = normScreenWidth*0.5f/distortionScale;
        config.scale[1] = normScreenHeight*0.5f*aspect/distortionScale;

        // Fovy is normally computed with: 2*atan2f(device.vScreenSize, 2*device.eyeToScreenDistance)
        // ...but with lens distortion it is increased (see Oculus SDK Documentation)
        float fovy = 2.0f*atan2f(device.vScreenSize*0.5f*distortionScale, device.eyeToScreenDistance);     // Really need distortionScale?
       // float fovy = 2.0f*(float)atan2f(device.vScreenSize*0.5f, device.eyeToScreenDistance);

        // Compute camera projection matrices
        float projOffset = 4.0f*lensShift;      // Scaled to projection space coordinates [-1..1]
        Matrix proj = MatrixPerspective(fovy, aspect, rlGetCullDistanceNear(), rlGetCullDistanceFar());

        config.projection[0] = MatrixMultiply(proj, MatrixTranslate(projOffset, 0.0f, 0.0f));
        config.projection[1] = MatrixMultiply(proj, MatrixTranslate(-projOffset, 0.0f, 0.0f));

        // Compute camera transformation matrices
        // NOTE: Camera movement might seem more natural if we model the head
        // Our axis of rotation is the base of our head, so we might want to add
        // some y (base of head to eye level) and -z (center of head to eye protrusion) to the camera positions
        config.viewOffset[0] = MatrixTranslate(device.interpupillaryDistance*0.5f, 0.075f, 0.045f);
        config.viewOffset[1] = MatrixTranslate(-device.interpupillaryDistance*0.5f, 0.075f, 0.045f);

        // Compute eyes Viewports
        /*
        config.eyeViewportRight[0] = 0;
        config.eyeViewportRight[1] = 0;
        config.eyeViewportRight[2] = device.hResolution/2;
        config.eyeViewportRight[3] = device.vResolution;

        config.eyeViewportLeft[0] = device.hResolution/2;
        config.eyeViewportLeft[1] = 0;
        config.eyeViewportLeft[2] = device.hResolution/2;
        config.eyeViewportLeft[3] = device.vResolution;
        */
    }
    else TRACELOG(LOG_WARNING, "RLGL: VR Simulator not supported on OpenGL 1.1");

    return config;
}

// Unload VR stereo config properties
void UnloadVrStereoConfig(VrStereoConfig config)
{
    TRACELOG(LOG_INFO, "UnloadVrStereoConfig not implemented in rcore.c");
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Shaders Management
//----------------------------------------------------------------------------------

// Load shader from files and bind default locations
// NOTE: If shader string is NULL, using default vertex/fragment shaders
Shader LoadShader(const char *vsFileName, const char *fsFileName)
{
    Shader shader = { 0 };

    char *vShaderStr = NULL;
    char *fShaderStr = NULL;

    if (vsFileName != NULL) vShaderStr = LoadFileText(vsFileName);
    if (fsFileName != NULL) fShaderStr = LoadFileText(fsFileName);

    if ((vShaderStr == NULL) && (fShaderStr == NULL)) TRACELOG(LOG_WARNING, "SHADER: Shader files provided are not valid, using default shader");

    shader = LoadShaderFromMemory(vShaderStr, fShaderStr);

    UnloadFileText(vShaderStr);
    UnloadFileText(fShaderStr);

    return shader;
}

// Load shader from code strings and bind default locations
Shader LoadShaderFromMemory(const char *vsCode, const char *fsCode)
{
    Shader shader = { 0 };

    shader.id = rlLoadShaderCode(vsCode, fsCode);

    if (shader.id == 0)
    {
        // Shader could not be loaded but we still load the location points to avoid potential crashes
        // NOTE: All locations set to -1 (no location)
        shader.locs = (int *)RL_CALLOC(RL_MAX_SHADER_LOCATIONS, sizeof(int));
        for (int i = 0; i < RL_MAX_SHADER_LOCATIONS; i++) shader.locs[i] = -1;
    }
    else if (shader.id == rlGetShaderIdDefault()) shader.locs = rlGetShaderLocsDefault();
    else if (shader.id > 0)
    {
        // After custom shader loading, we TRY to set default location names
        // Default shader attribute locations have been binded before linking:
        //  - vertex position location    = 0
        //  - vertex texcoord location    = 1
        //  - vertex normal location      = 2
        //  - vertex color location       = 3
        //  - vertex tangent location     = 4
        //  - vertex texcoord2 location   = 5
        //  - vertex boneIds location     = 6
        //  - vertex boneWeights location = 7

        // NOTE: If any location is not found, loc point becomes -1

        // Load shader locations array
        // NOTE: All locations set to -1 (no location)
        shader.locs = (int *)RL_CALLOC(RL_MAX_SHADER_LOCATIONS, sizeof(int));
        for (int i = 0; i < RL_MAX_SHADER_LOCATIONS; i++) shader.locs[i] = -1;

        // Get handles to GLSL input attribute locations
        shader.locs[SHADER_LOC_VERTEX_POSITION] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION);
        shader.locs[SHADER_LOC_VERTEX_TEXCOORD01] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD);
        shader.locs[SHADER_LOC_VERTEX_TEXCOORD02] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2);
        shader.locs[SHADER_LOC_VERTEX_NORMAL] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL);
        shader.locs[SHADER_LOC_VERTEX_TANGENT] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT);
        shader.locs[SHADER_LOC_VERTEX_COLOR] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR);
        shader.locs[SHADER_LOC_VERTEX_BONEIDS] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_BONEIDS);
        shader.locs[SHADER_LOC_VERTEX_BONEWEIGHTS] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_BONEWEIGHTS);
        shader.locs[SHADER_LOC_VERTEX_INSTANCE_TX] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_INSTANCE_TX);

        // Get handles to GLSL uniform locations (vertex shader)
        shader.locs[SHADER_LOC_MATRIX_MVP] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_MVP);
        shader.locs[SHADER_LOC_MATRIX_VIEW] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW);
        shader.locs[SHADER_LOC_MATRIX_PROJECTION] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION);
        shader.locs[SHADER_LOC_MATRIX_MODEL] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL);
        shader.locs[SHADER_LOC_MATRIX_NORMAL] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL);
        shader.locs[SHADER_LOC_BONE_MATRICES] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_BONE_MATRICES);

        // Get handles to GLSL uniform locations (fragment shader)
        shader.locs[SHADER_LOC_COLOR_DIFFUSE] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR);
        shader.locs[SHADER_LOC_MAP_DIFFUSE] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0);  // SHADER_LOC_MAP_ALBEDO
        shader.locs[SHADER_LOC_MAP_SPECULAR] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1); // SHADER_LOC_MAP_METALNESS
        shader.locs[SHADER_LOC_MAP_NORMAL] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2);
    }

    return shader;
}

// Check if a shader is valid (loaded on GPU)
bool IsShaderValid(Shader shader)
{
    return ((shader.id > 0) &&          // Validate shader id (GPU loaded successfully)
            (shader.locs != NULL));     // Validate memory has been allocated for default shader locations

    // The following locations are tried to be set automatically (locs[i] >= 0),
    // any of them can be checked for validation but the only mandatory one is, afaik, SHADER_LOC_VERTEX_POSITION
    // NOTE: Users can also setup manually their own attributes/uniforms and do not used the default raylib ones

    // Vertex shader attribute locations (default)
    // shader.locs[SHADER_LOC_VERTEX_POSITION]      // Set by default internal shader
    // shader.locs[SHADER_LOC_VERTEX_TEXCOORD01]    // Set by default internal shader
    // shader.locs[SHADER_LOC_VERTEX_TEXCOORD02]
    // shader.locs[SHADER_LOC_VERTEX_NORMAL]
    // shader.locs[SHADER_LOC_VERTEX_TANGENT]
    // shader.locs[SHADER_LOC_VERTEX_COLOR]         // Set by default internal shader

    // Vertex shader uniform locations (default)
    // shader.locs[SHADER_LOC_MATRIX_MVP]           // Set by default internal shader
    // shader.locs[SHADER_LOC_MATRIX_VIEW]
    // shader.locs[SHADER_LOC_MATRIX_PROJECTION]
    // shader.locs[SHADER_LOC_MATRIX_MODEL]
    // shader.locs[SHADER_LOC_MATRIX_NORMAL]

    // Fragment shader uniform locations (default)
    // shader.locs[SHADER_LOC_COLOR_DIFFUSE]        // Set by default internal shader
    // shader.locs[SHADER_LOC_MAP_DIFFUSE]          // Set by default internal shader
    // shader.locs[SHADER_LOC_MAP_SPECULAR]
    // shader.locs[SHADER_LOC_MAP_NORMAL]
}

// Unload shader from GPU memory (VRAM)
void UnloadShader(Shader shader)
{
    if (shader.id != rlGetShaderIdDefault())
    {
        rlUnloadShaderProgram(shader.id);

        // NOTE: If shader loading failed, it should be 0
        RL_FREE(shader.locs);
    }
}

// Get shader uniform location
int GetShaderLocation(Shader shader, const char *uniformName)
{
    return rlGetLocationUniform(shader.id, uniformName);
}

// Get shader attribute location
int GetShaderLocationAttrib(Shader shader, const char *attribName)
{
    return rlGetLocationAttrib(shader.id, attribName);
}

// Set shader uniform value
void SetShaderValue(Shader shader, int locIndex, const void *value, int uniformType)
{
    SetShaderValueV(shader, locIndex, value, uniformType, 1);
}

// Set shader uniform value vector
void SetShaderValueV(Shader shader, int locIndex, const void *value, int uniformType, int count)
{
    if (locIndex > -1)
    {
        rlEnableShader(shader.id);
        rlSetUniform(locIndex, value, uniformType, count);
        //rlDisableShader();      // Avoid resetting current shader program, in case other uniforms are set
    }
}

// Set shader uniform value (matrix 4x4)
void SetShaderValueMatrix(Shader shader, int locIndex, Matrix mat)
{
    if (locIndex > -1)
    {
        rlEnableShader(shader.id);
        rlSetUniformMatrix(locIndex, mat);
        //rlDisableShader();
    }
}

// Set shader uniform value for texture
void SetShaderValueTexture(Shader shader, int locIndex, Texture2D texture)
{
    if (locIndex > -1)
    {
        rlEnableShader(shader.id);
        rlSetUniformSampler(locIndex, texture.id);
        //rlDisableShader();
    }
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Screen-space Queries
//----------------------------------------------------------------------------------

// Get a ray trace from screen position (i.e mouse)
Ray GetScreenToWorldRay(Vector2 position, Camera camera)
{
    Ray ray = GetScreenToWorldRayEx(position, camera, GetScreenWidth(), GetScreenHeight());

    return ray;
}

// Get a ray trace from the screen position (i.e mouse) within a specific section of the screen
Ray GetScreenToWorldRayEx(Vector2 position, Camera camera, int width, int height)
{
    Ray ray = { 0 };

    // Calculate normalized device coordinates
    // NOTE: y value is negative
    float x = (2.0f*position.x)/(float)width - 1.0f;
    float y = 1.0f - (2.0f*position.y)/(float)height;
    float z = 1.0f;

    // Store values in a vector
    Vector3 deviceCoords = { x, y, z };

    // Calculate view matrix from camera look at
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);

    Matrix matProj = MatrixIdentity();

    if (camera.projection == CAMERA_PERSPECTIVE)
    {
        // Calculate projection matrix from perspective
        matProj = MatrixPerspective(camera.fovy*DEG2RAD, ((double)width/(double)height), rlGetCullDistanceNear(), rlGetCullDistanceFar());
    }
    else if (camera.projection == CAMERA_ORTHOGRAPHIC)
    {
        double aspect = (double)width/(double)height;
        double top = camera.fovy/2.0;
        double right = top*aspect;

        // Calculate projection matrix from orthographic
        matProj = MatrixOrtho(-right, right, -top, top, rlGetCullDistanceNear(), rlGetCullDistanceFar());
    }

    // Unproject far/near points
    Vector3 nearPoint = Vector3Unproject((Vector3){ deviceCoords.x, deviceCoords.y, 0.0f }, matProj, matView);
    Vector3 farPoint = Vector3Unproject((Vector3){ deviceCoords.x, deviceCoords.y, 1.0f }, matProj, matView);

    // Unproject the mouse cursor in the near plane
    // We need this as the source position because orthographic projects,
    // compared to perspective doesn't have a convergence point,
    // meaning that the "eye" of the camera is more like a plane than a point
    Vector3 cameraPlanePointerPos = Vector3Unproject((Vector3){ deviceCoords.x, deviceCoords.y, -1.0f }, matProj, matView);

    // Calculate normalized direction vector
    Vector3 direction = Vector3Normalize(Vector3Subtract(farPoint, nearPoint));

    if (camera.projection == CAMERA_PERSPECTIVE) ray.position = camera.position;
    else if (camera.projection == CAMERA_ORTHOGRAPHIC) ray.position = cameraPlanePointerPos;

    // Apply calculated vectors to ray
    ray.direction = direction;

    return ray;
}

// Get transform matrix for camera
Matrix GetCameraMatrix(Camera camera)
{
    Matrix mat = MatrixLookAt(camera.position, camera.target, camera.up);

    return mat;
}

// Get camera 2d transform matrix
Matrix GetCameraMatrix2D(Camera2D camera)
{
    Matrix matTransform = { 0 };
    // The camera in world-space is set by
    //   1. Move it to target
    //   2. Rotate by -rotation and scale by (1/zoom)
    //      When setting higher scale, it's more intuitive for the world to become bigger (= camera become smaller),
    //      not for the camera getting bigger, hence the invert. Same deal with rotation
    //   3. Move it by (-offset);
    //      Offset defines target transform relative to screen, but since effectively "moving" screen (camera)
    //      we need to do it into opposite direction (inverse transform)

    // Having camera transform in world-space, inverse of it gives the modelview transform
    // Since (A*B*C)' = C'*B'*A', the modelview is
    //   1. Move to offset
    //   2. Rotate and Scale
    //   3. Move by -target
    Matrix matOrigin = MatrixTranslate(-camera.target.x, -camera.target.y, 0.0f);
    Matrix matRotation = MatrixRotate((Vector3){ 0.0f, 0.0f, 1.0f }, camera.rotation*DEG2RAD);
    Matrix matScale = MatrixScale(camera.zoom, camera.zoom, 1.0f);
    Matrix matTranslation = MatrixTranslate(camera.offset.x, camera.offset.y, 0.0f);

    matTransform = MatrixMultiply(MatrixMultiply(matOrigin, MatrixMultiply(matScale, matRotation)), matTranslation);

    return matTransform;
}

// Get the screen space position from a 3d world space position
Vector2 GetWorldToScreen(Vector3 position, Camera camera)
{
    Vector2 screenPosition = GetWorldToScreenEx(position, camera, GetScreenWidth(), GetScreenHeight());

    return screenPosition;
}

// Get size position for a 3d world space position (useful for texture drawing)
Vector2 GetWorldToScreenEx(Vector3 position, Camera camera, int width, int height)
{
    // Calculate projection matrix (from perspective instead of frustum
    Matrix matProj = MatrixIdentity();

    if (camera.projection == CAMERA_PERSPECTIVE)
    {
        // Calculate projection matrix from perspective
        matProj = MatrixPerspective(camera.fovy*DEG2RAD, ((double)width/(double)height), rlGetCullDistanceNear(), rlGetCullDistanceFar());
    }
    else if (camera.projection == CAMERA_ORTHOGRAPHIC)
    {
        double aspect = (double)width/(double)height;
        double top = camera.fovy/2.0;
        double right = top*aspect;

        // Calculate projection matrix from orthographic
        matProj = MatrixOrtho(-right, right, -top, top, rlGetCullDistanceNear(), rlGetCullDistanceFar());
    }

    // Calculate view matrix from camera look at (and transpose it)
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);

    // Convert world position vector to quaternion
    Quaternion worldPos = { position.x, position.y, position.z, 1.0f };

    // Transform world position to view
    worldPos = QuaternionTransform(worldPos, matView);

    // Transform result to projection (clip space position)
    worldPos = QuaternionTransform(worldPos, matProj);

    // Calculate normalized device coordinates (inverted y)
    Vector3 ndcPos = { worldPos.x/worldPos.w, -worldPos.y/worldPos.w, worldPos.z/worldPos.w };

    // Calculate 2d screen position vector
    Vector2 screenPosition = { (ndcPos.x + 1.0f)/2.0f*(float)width, (ndcPos.y + 1.0f)/2.0f*(float)height };

    return screenPosition;
}

// Get the screen space position for a 2d camera world space position
Vector2 GetWorldToScreen2D(Vector2 position, Camera2D camera)
{
    Matrix matCamera = GetCameraMatrix2D(camera);
    Vector3 transform = Vector3Transform((Vector3){ position.x, position.y, 0 }, matCamera);

    return (Vector2){ transform.x, transform.y };
}

// Get the world space position for a 2d camera screen space position
Vector2 GetScreenToWorld2D(Vector2 position, Camera2D camera)
{
    Matrix invMatCamera = MatrixInvert(GetCameraMatrix2D(camera));
    Vector3 transform = Vector3Transform((Vector3){ position.x, position.y, 0 }, invMatCamera);

    return (Vector2){ transform.x, transform.y };
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Timming
//----------------------------------------------------------------------------------

// NOTE: Functions with a platform-specific implementation on rcore_<platform>.c
//double GetTime(void)

// Set target FPS (maximum)
void SetTargetFPS(int fps)
{
    if (fps < 1) CORE.Time.target = 0.0;
    else CORE.Time.target = 1.0/(double)fps;

    TRACELOG(LOG_INFO, "TIMER: Target time per frame: %02.03f milliseconds", (float)CORE.Time.target*1000.0f);
}

// Get current FPS
// NOTE: We calculate an average framerate
int GetFPS(void)
{
    int fps = 0;

#if !defined(SUPPORT_CUSTOM_FRAME_CONTROL)
    #define FPS_CAPTURE_FRAMES_COUNT    30      // 30 captures
    #define FPS_AVERAGE_TIME_SECONDS   0.5f     // 500 milliseconds
    #define FPS_STEP (FPS_AVERAGE_TIME_SECONDS/FPS_CAPTURE_FRAMES_COUNT)

    static int index = 0;
    static float history[FPS_CAPTURE_FRAMES_COUNT] = { 0 };
    static float average = 0, last = 0;
    float fpsFrame = GetFrameTime();

    // if we reset the window, reset the FPS info
    if (CORE.Time.frameCounter == 0)
    {
        average = 0;
        last = 0;
        index = 0;

        for (int i = 0; i < FPS_CAPTURE_FRAMES_COUNT; i++) history[i] = 0;
    }

    if (fpsFrame == 0) return 0;

    if ((GetTime() - last) > FPS_STEP)
    {
        last = (float)GetTime();
        index = (index + 1)%FPS_CAPTURE_FRAMES_COUNT;
        average -= history[index];
        history[index] = fpsFrame/FPS_CAPTURE_FRAMES_COUNT;
        average += history[index];
    }

    fps = (int)roundf(1.0f/average);
#endif

    return fps;
}

// Get time in seconds for last frame drawn (delta time)
float GetFrameTime(void)
{
    return (float)CORE.Time.frame;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Custom frame control
//----------------------------------------------------------------------------------

// NOTE: Functions with a platform-specific implementation on rcore_<platform>.c
//void SwapScreenBuffer(void);
//void PollInputEvents(void);

// Wait for some time (stop program execution)
// NOTE: Sleep() granularity could be around 10 ms, it means, Sleep() could
// take longer than expected... for that reason we use the busy wait loop
// REF: http://stackoverflow.com/questions/43057578/c-programming-win32-games-sleep-taking-longer-than-expected
// REF: http://www.geisswerks.com/ryan/FAQS/timing.html --> All about timing on Win32!
void WaitTime(double seconds)
{
    if (seconds < 0) return;    // Security check

#if defined(SUPPORT_BUSY_WAIT_LOOP) || defined(SUPPORT_PARTIALBUSY_WAIT_LOOP)
    double destinationTime = GetTime() + seconds;
#endif

#if defined(SUPPORT_BUSY_WAIT_LOOP)
    while (GetTime() < destinationTime) { }
#else
    #if defined(SUPPORT_PARTIALBUSY_WAIT_LOOP)
        double sleepSeconds = seconds - seconds*0.05;  // NOTE: We reserve a percentage of the time for busy waiting
    #else
        double sleepSeconds = seconds;
    #endif

    // System halt functions
    #if defined(_WIN32)
        Sleep((unsigned long)(sleepSeconds*1000.0));
    #endif
    #if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__EMSCRIPTEN__)
        struct timespec req = { 0 };
        time_t sec = sleepSeconds;
        long nsec = (sleepSeconds - sec)*1000000000L;
        req.tv_sec = sec;
        req.tv_nsec = nsec;

        // NOTE: Use nanosleep() on Unix platforms... usleep() it's deprecated
        while (nanosleep(&req, &req) == -1) continue;
    #endif
    #if defined(__APPLE__)
        usleep(sleepSeconds*1000000.0);
    #endif

    #if defined(SUPPORT_PARTIALBUSY_WAIT_LOOP)
        while (GetTime() < destinationTime) { }
    #endif
#endif
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Misc
//----------------------------------------------------------------------------------

// NOTE: Functions with a platform-specific implementation on rcore_<platform>.c
//void OpenURL(const char *url)

// Set the seed for the random number generator
void SetRandomSeed(unsigned int seed)
{
#if defined(SUPPORT_RPRAND_GENERATOR)
    rprand_set_seed(seed);
#else
    srand(seed);
#endif
}

// Get a random value between min and max included
int GetRandomValue(int min, int max)
{
    int value = 0;

    if (min > max)
    {
        int tmp = max;
        max = min;
        min = tmp;
    }

#if defined(SUPPORT_RPRAND_GENERATOR)
    value = rprand_get_value(min, max);
#else
    // WARNING: Ranges higher than RAND_MAX will return invalid results
    // More specifically, if (max - min) > INT_MAX there will be an overflow,
    // and otherwise if (max - min) > RAND_MAX the random value will incorrectly never exceed a certain threshold
    // NOTE: Depending on the library it can be as low as 32767
    if ((unsigned int)(max - min) > (unsigned int)RAND_MAX)
    {
        TRACELOG(LOG_WARNING, "Invalid GetRandomValue() arguments, range should not be higher than %i", RAND_MAX);
    }

    // NOTE: This one-line approach produces a non-uniform distribution,
    // as stated by Donald Knuth in the book The Art of Programming, so
    // using below approach for more uniform results
    //value = (rand()%(abs(max - min) + 1) + min);

    // More uniform range solution
    int range = (max - min) + 1;

    // Degenerate/overflow case: fall back to min (same behavior as "always min" instead of UB)
    if (range <= 0) value = min;
    else
    {
        // Rejection sampling to get a uniform integer in [min, max]
        unsigned long c = (unsigned long)RAND_MAX + 1UL;  // number of possible rand() results
        unsigned long m = (unsigned long)range;           // size of the target interval
        unsigned long t = c - (c%m);                    // largest multiple of m <= c
        unsigned long r = 0;

        for (;;)
        {
            r = (unsigned long)rand();
            if (r < t) break;   // Only accept values within the fair region
        }

        value = min + (int)(r%m);
    }
#endif
    return value;
}

// Load random values sequence, no values repeated, min and max included
int *LoadRandomSequence(unsigned int count, int min, int max)
{
    int *values = NULL;

#if defined(SUPPORT_RPRAND_GENERATOR)
    values = rprand_load_sequence(count, min, max);
#else
    if (count > ((unsigned int)abs(max - min) + 1)) return values;  // Security check

    values = (int *)RL_CALLOC(count, sizeof(int));

    int value = 0;
    bool dupValue = false;

    for (int i = 0; i < (int)count;)
    {
        value = GetRandomValue(min, max);
        dupValue = false;

        for (int j = 0; j < i; j++)
        {
            if (values[j] == value)
            {
                dupValue = true;
                break;
            }
        }

        if (!dupValue)
        {
            values[i] = value;
            i++;
        }
    }
#endif
    return values;
}

// Unload random values sequence
void UnloadRandomSequence(int *sequence)
{
#if defined(SUPPORT_RPRAND_GENERATOR)
    rprand_unload_sequence(sequence);
#else
    RL_FREE(sequence);
#endif
}

// Takes a screenshot of current screen
// NOTE: Provided fileName should not contain paths, saving to working directory
void TakeScreenshot(const char *fileName)
{
#if defined(SUPPORT_MODULE_RTEXTURES)
    // Security check to (partially) avoid malicious code
    if (strchr(fileName, '\'') != NULL) { TRACELOG(LOG_WARNING, "SYSTEM: Provided fileName could be potentially malicious, avoid [\'] character"); return; }

    // Apply a scale if we are doing HIGHDPI auto-scaling
    Vector2 scale = { 1.0f, 1.0f };
    if (FLAG_IS_SET(CORE.Window.flags, FLAG_WINDOW_HIGHDPI)) scale = GetWindowScaleDPI();

    unsigned char *imgData = rlReadScreenPixels((int)((float)CORE.Window.render.width*scale.x), (int)((float)CORE.Window.render.height*scale.y));
    Image image = { imgData, (int)((float)CORE.Window.render.width*scale.x), (int)((float)CORE.Window.render.height*scale.y), 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };

    char path[MAX_FILEPATH_LENGTH] = { 0 };
    strncpy(path, TextFormat("%s/%s", CORE.Storage.basePath, fileName), MAX_FILEPATH_LENGTH - 1);

    ExportImage(image, path); // WARNING: Module required: rtextures
    RL_FREE(imgData);

    if (FileExists(path)) TRACELOG(LOG_INFO, "SYSTEM: [%s] Screenshot taken successfully", path);
    else TRACELOG(LOG_WARNING, "SYSTEM: [%s] Screenshot could not be saved", path);
#else
    TRACELOG(LOG_WARNING,"IMAGE: ExportImage() requires module: rtextures");
#endif
}

// Setup window configuration flags (view FLAGS)
// NOTE: This function is expected to be called before window creation,
// because it sets up some flags for the window creation process
// To configure window states after creation, just use SetWindowState()
void SetConfigFlags(unsigned int flags)
{
    if (CORE.Window.ready) TRACELOG(LOG_WARNING, "WINDOW: SetConfigFlags called after window initialization, Use \"SetWindowState\" to set flags instead");

    // Selected flags are set but not evaluated at this point,
    // flag evaluation happens at InitWindow() or SetWindowState()
    FLAG_SET(CORE.Window.flags, flags);
}

// void OpenURL(const char *url);   // Defined per platform

//----------------------------------------------------------------------------------
// Module Functions Definition: Logging system
//----------------------------------------------------------------------------------
// Set the current threshold (minimum) log level
void SetTraceLogLevel(int logType) { logTypeLevel = logType; }

// Show trace log messages (LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG)
void TraceLog(int logType, const char *text, ...)
{
#if defined(SUPPORT_TRACELOG)
    // Message has level below current threshold, don't emit
    if ((logType < logTypeLevel) || (text == NULL)) return;

    va_list args;
    va_start(args, text);

    if (traceLog)
    {
        traceLog(logType, text, args);
        va_end(args);
        return;
    }

#if defined(PLATFORM_ANDROID)
    switch (logType)
    {
        case LOG_TRACE: __android_log_vprint(ANDROID_LOG_VERBOSE, "raylib", text, args); break;
        case LOG_DEBUG: __android_log_vprint(ANDROID_LOG_DEBUG, "raylib", text, args); break;
        case LOG_INFO: __android_log_vprint(ANDROID_LOG_INFO, "raylib", text, args); break;
        case LOG_WARNING: __android_log_vprint(ANDROID_LOG_WARN, "raylib", text, args); break;
        case LOG_ERROR: __android_log_vprint(ANDROID_LOG_ERROR, "raylib", text, args); break;
        case LOG_FATAL: __android_log_vprint(ANDROID_LOG_FATAL, "raylib", text, args); break;
        default: break;
    }
#else
    char buffer[MAX_TRACELOG_MSG_LENGTH] = { 0 };

    switch (logType)
    {
        case LOG_TRACE: strncpy(buffer, "TRACE: ", 8); break;
        case LOG_DEBUG: strncpy(buffer, "DEBUG: ", 8); break;
        case LOG_INFO: strncpy(buffer, "INFO: ", 7); break;
        case LOG_WARNING: strncpy(buffer, "WARNING: ", 10); break;
        case LOG_ERROR: strncpy(buffer, "ERROR: ", 8); break;
        case LOG_FATAL: strncpy(buffer, "FATAL: ", 8); break;
        default: break;
    }

    unsigned int textLength = (unsigned int)strlen(text);
    memcpy(buffer + strlen(buffer), text, (textLength < (MAX_TRACELOG_MSG_LENGTH - 12))? textLength : (MAX_TRACELOG_MSG_LENGTH - 12));
    strcat(buffer, "\n");
    vprintf(buffer, args);
    fflush(stdout);
#endif

    va_end(args);

    if (logType == LOG_FATAL) exit(EXIT_FAILURE);  // If fatal logging, exit program

#endif  // SUPPORT_TRACELOG
}

// Set custom trace log
void SetTraceLogCallback(TraceLogCallback callback)
{ 
    traceLog = callback;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Memory management
//----------------------------------------------------------------------------------
// Internal memory allocator
// NOTE: Initializes to zero by default
void *MemAlloc(unsigned int size)
{
    void *ptr = RL_CALLOC(size, 1);
    return ptr;
}

// Internal memory reallocator
void *MemRealloc(void *ptr, unsigned int size)
{
    void *ret = RL_REALLOC(ptr, size);
    return ret;
}

// Internal memory free
void MemFree(void *ptr)
{
    RL_FREE(ptr);
}

//----------------------------------------------------------------------------------
// Module Functions Definition: File System management
//----------------------------------------------------------------------------------
// Load data from file into a buffer
unsigned char *LoadFileData(const char *fileName, int *dataSize)
{
    unsigned char *data = NULL;
    *dataSize = 0;

    if (fileName != NULL)
    {
        if (loadFileData)
        {
            data = loadFileData(fileName, dataSize);
            return data;
        }
#if defined(SUPPORT_STANDARD_FILEIO)
        FILE *file = fopen(fileName, "rb");

        if (file != NULL)
        {
            // WARNING: On binary streams SEEK_END could not be found,
            // using fseek() and ftell() could not work in some (rare) cases
            fseek(file, 0, SEEK_END);
            int size = ftell(file);     // WARNING: ftell() returns 'long int', maximum size returned is INT_MAX (2147483647 bytes)
            fseek(file, 0, SEEK_SET);

            if (size > 0)
            {
                data = (unsigned char *)RL_CALLOC(size, sizeof(unsigned char));

                if (data != NULL)
                {
                    // NOTE: fread() returns number of read elements instead of bytes, so we read [1 byte, size elements]
                    size_t count = fread(data, sizeof(unsigned char), size, file);

                    // WARNING: fread() returns a size_t value, usually 'unsigned int' (32bit compilation) and 'unsigned long long' (64bit compilation)
                    // dataSize is unified along raylib as a 'int' type, so, for file-sizes > INT_MAX (2147483647 bytes) we have a limitation
                    if (count > 2147483647)
                    {
                        TRACELOG(LOG_WARNING, "FILEIO: [%s] File is bigger than 2147483647 bytes, avoid using LoadFileData()", fileName);

                        RL_FREE(data);
                        data = NULL;
                    }
                    else
                    {
                        *dataSize = (int)count;

                        if ((*dataSize) != size) TRACELOG(LOG_WARNING, "FILEIO: [%s] File partially loaded (%i bytes out of %i)", fileName, dataSize, count);
                        else TRACELOG(LOG_INFO, "FILEIO: [%s] File loaded successfully", fileName);
                    }
                }
                else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to allocated memory for file reading", fileName);
            }
            else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to read file", fileName);

            fclose(file);
        }
        else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open file", fileName);
#else
    TRACELOG(LOG_WARNING, "FILEIO: Standard file io not supported, use custom file callback");
#endif
    }
    else TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");

    return data;
}

// Unload file data allocated by LoadFileData()
void UnloadFileData(unsigned char *data)
{
    RL_FREE(data);
}

// Save data to file from buffer
bool SaveFileData(const char *fileName, void *data, int dataSize)
{
    bool success = false;

    if (fileName != NULL)
    {
        if (saveFileData)
        {
            return saveFileData(fileName, data, dataSize);
        }
#if defined(SUPPORT_STANDARD_FILEIO)
        FILE *file = fopen(fileName, "wb");

        if (file != NULL)
        {
            // WARNING: fwrite() returns a size_t value, usually 'unsigned int' (32bit compilation) and 'unsigned long long' (64bit compilation)
            // and expects a size_t input value but as dataSize is limited to INT_MAX (2147483647 bytes), there shouldn't be a problem
            int count = (int)fwrite(data, sizeof(unsigned char), dataSize, file);

            if (count == 0) TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to write file", fileName);
            else if (count != dataSize) TRACELOG(LOG_WARNING, "FILEIO: [%s] File partially written", fileName);
            else TRACELOG(LOG_INFO, "FILEIO: [%s] File saved successfully", fileName);

            int result = fclose(file);
            if (result == 0) success = true;
        }
        else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open file", fileName);
#else
    TRACELOG(LOG_WARNING, "FILEIO: Standard file io not supported, use custom file callback");
#endif
    }
    else TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");

    return success;
}

// Export data to code (.h), returns true on success
bool ExportDataAsCode(const unsigned char *data, int dataSize, const char *fileName)
{
    bool success = false;

#ifndef TEXT_BYTES_PER_LINE
    #define TEXT_BYTES_PER_LINE     20
#endif

    // NOTE: Text data buffer size is estimated considering raw data size in bytes
    // and requiring 6 char bytes for every byte: "0x00, "
    char *txtData = (char *)RL_CALLOC(dataSize*6 + 2000, sizeof(char));

    int byteCount = 0;
    byteCount += sprintf(txtData + byteCount, "////////////////////////////////////////////////////////////////////////////////////////\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                                    //\n");
    byteCount += sprintf(txtData + byteCount, "// DataAsCode exporter v1.0 - Raw data exported as an array of bytes                  //\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                                    //\n");
    byteCount += sprintf(txtData + byteCount, "// more info and bugs-report:  github.com/raysan5/raylib                              //\n");
    byteCount += sprintf(txtData + byteCount, "// feedback and support:       ray[at]raylib.com                                      //\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                                    //\n");
    byteCount += sprintf(txtData + byteCount, "// Copyright (c) 2022-2026 Ramon Santamaria (@raysan5)                                //\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                                    //\n");
    byteCount += sprintf(txtData + byteCount, "////////////////////////////////////////////////////////////////////////////////////////\n\n");

    // Get file name from path
    char varFileName[256] = { 0 };
    strncpy(varFileName, GetFileNameWithoutExt(fileName), 256 - 1);
    for (int i = 0; varFileName[i] != '\0'; i++)
    {
        // Convert variable name to uppercase
        if ((varFileName[i] >= 'a') && (varFileName[i] <= 'z')) { varFileName[i] = varFileName[i] - 32; }
        // Replace non valid character for C identifier with '_'
        else if (varFileName[i] == '.' || varFileName[i] == '-' || varFileName[i] == '?' || varFileName[i] == '!' || varFileName[i] == '+') { varFileName[i] = '_'; }
    }

    byteCount += sprintf(txtData + byteCount, "#define %s_DATA_SIZE     %i\n\n", varFileName, dataSize);

    byteCount += sprintf(txtData + byteCount, "static unsigned char %s_DATA[%s_DATA_SIZE] = { ", varFileName, varFileName);
    for (int i = 0; i < (dataSize - 1); i++) byteCount += sprintf(txtData + byteCount, ((i%TEXT_BYTES_PER_LINE == 0)? "0x%x,\n" : "0x%x, "), data[i]);
    byteCount += sprintf(txtData + byteCount, "0x%x };\n", data[dataSize - 1]);

    // NOTE: Text data size exported is determined by '\0' (NULL) character
    success = SaveFileText(fileName, txtData);

    RL_FREE(txtData);

    if (success != 0) TRACELOG(LOG_INFO, "FILEIO: [%s] Data as code exported successfully", fileName);
    else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to export data as code", fileName);

    return success;
}

// Load text data from file, returns a '\0' terminated string
// NOTE: text chars array should be freed manually
char *LoadFileText(const char *fileName)
{
    char *text = NULL;

    if (fileName != NULL)
    {
        if (loadFileText)
        {
            text = loadFileText(fileName);
            return text;
        }
#if defined(SUPPORT_STANDARD_FILEIO)
        FILE *file = fopen(fileName, "rt");

        if (file != NULL)
        {
            // WARNING: When reading a file as 'text' file,
            // text mode causes carriage return-linefeed translation...
            // ...but using fseek() should return correct byte-offset
            fseek(file, 0, SEEK_END);
            unsigned int size = (unsigned int)ftell(file);
            fseek(file, 0, SEEK_SET);

            if (size > 0)
            {
                text = (char *)RL_CALLOC(size + 1, sizeof(char));

                if (text != NULL)
                {
                    unsigned int count = (unsigned int)fread(text, sizeof(char), size, file);

                    // WARNING: \r\n is converted to \n on reading, so,
                    // read bytes count gets reduced by the number of lines
                    if (count < size) text = (char *)RL_REALLOC(text, count + 1);

                    // Zero-terminate the string
                    text[count] = '\0';

                    TRACELOG(LOG_INFO, "FILEIO: [%s] Text file loaded successfully", fileName);
                }
                else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to allocated memory for file reading", fileName);
            }
            else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to read text file", fileName);

            fclose(file);
        }
        else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open text file", fileName);
#else
    TRACELOG(LOG_WARNING, "FILEIO: Standard file io not supported, use custom file callback");
#endif
    }
    else TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");

    return text;
}

// Unload file text data allocated by LoadFileText()
void UnloadFileText(char *text)
{
    RL_FREE(text);
}

// Save text data to file (write), string must be '\0' terminated
bool SaveFileText(const char *fileName, const char *text)
{
    bool success = false;

    if (fileName != NULL)
    {
        if (saveFileText)
        {
            return saveFileText(fileName, text);
        }
#if defined(SUPPORT_STANDARD_FILEIO)
        FILE *file = fopen(fileName, "wt");

        if (file != NULL)
        {
            int count = fprintf(file, "%s", text);

            if (count < 0) TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to write text file", fileName);
            else TRACELOG(LOG_INFO, "FILEIO: [%s] Text file saved successfully", fileName);

            int result = fclose(file);
            if (result == 0) success = true;
        }
        else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open text file", fileName);
#else
    TRACELOG(LOG_WARNING, "FILEIO: Standard file io not supported, use custom file callback");
#endif
    }
    else TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");

    return success;
}

// File access custom callbacks
// WARNING: Callbacks setup is intended for advanced users

// Set custom file binary data loader
void SetLoadFileDataCallback(LoadFileDataCallback callback)
{
    loadFileData = callback;
}

// Set custom file binary data saver
void SetSaveFileDataCallback(SaveFileDataCallback callback)
{
    saveFileData = callback;
}

// Set custom file text data loader
void SetLoadFileTextCallback(LoadFileTextCallback callback)
{
    loadFileText = callback;
}

// Set custom file text data saver
void SetSaveFileTextCallback(SaveFileTextCallback callback)
{
    saveFileText = callback;
}

// Rename file (if exists)
// NOTE: Only rename file name required, not full path
int FileRename(const char *fileName, const char *fileRename)
{
    int result = 0;

    if (FileExists(fileName))
    {
        result = rename(fileName, fileRename);
    }
    else result = -1;

    return result;
}

// Remove file (if exists)
int FileRemove(const char *fileName)
{
    int result = 0;

    if (FileExists(fileName))
    {
        result = remove(fileName);
    }
    else result = -1;

    return result;
}

// Copy file from one path to another
// NOTE: If destination path does not exist, it is created!
int FileCopy(const char *srcPath, const char *dstPath)
{
    int result = 0;
    int srcDataSize = 0;
    unsigned char *srcFileData = LoadFileData(srcPath, &srcDataSize);

    // Create required paths if they do not exist
    if (!DirectoryExists(GetDirectoryPath(dstPath)))
        result = MakeDirectory(GetDirectoryPath(dstPath));

    if (result == 0) // Directory created successfully (or already exists)
    {
        if ((srcFileData != NULL) && (srcDataSize > 0))
            result = SaveFileData(dstPath, srcFileData, srcDataSize);
    }

    UnloadFileData(srcFileData);

    return result;
}

// Move file from one directory to another
// NOTE: If dst directories do not exists they are created
int FileMove(const char *srcPath, const char *dstPath)
{
    int result = 0;

    if (FileExists(srcPath))
    {
        FileCopy(srcPath, dstPath);
        FileRemove(srcPath);
    }
    else result = -1;

    return result;
}

// Replace text in an existing file
// WARNING: DEPENDENCY: [rtext] module
int FileTextReplace(const char *fileName, const char *search, const char *replacement)
{
    int result = 0;
    char *fileText = NULL;
    char *fileTextUpdated = { 0 };

#if defined(SUPPORT_MODULE_RTEXT)
    if (FileExists(fileName))
    {
        fileText = LoadFileText(fileName);
        fileTextUpdated = TextReplace(fileText, search, replacement);
        result = SaveFileText(fileName, fileTextUpdated);
        MemFree(fileTextUpdated);
        UnloadFileText(fileText);
    }
#else
    TRACELOG(LOG_WARNING, "FILE: File text replace requires [rtext] module");
#endif

    return result;
}

// Find text index position in existing file
// WARNING: DEPENDENCY: [rtext] module
int FileTextFindIndex(const char *fileName, const char *search)
{
    int result = -1;

    if (FileExists(fileName))
    {
        char *fileText = LoadFileText(fileName);
        char *ptr = strstr(fileText, search);
        if (ptr != NULL) result = (int)(ptr - fileText);
        UnloadFileText(fileText);
    }

    return result;
}

// Check if the file exists
bool FileExists(const char *fileName)
{
    bool result = false;

    if (ACCESS(fileName) != -1) result = true;

    // NOTE: Alternatively, stat() can be used instead of access()
    //#include <sys/stat.h>
    //struct stat statbuf;
    //if (stat(filename, &statbuf) == 0) result = true;

    return result;
}

// Check file extension
bool IsFileExtension(const char *fileName, const char *ext)
{
    #define MAX_FILE_EXTENSIONS  32

    bool result = false;
    const char *fileExt = GetFileExtension(fileName);

    // WARNING: fileExt points to last '.' on fileName string but it could happen
    // that fileName is not correct: "myfile.png more text following\n"

    if (fileExt != NULL)
    {
        int fileExtLength = (int)strlen(fileExt);
        char fileExtLower[16] = { 0 };
        char *fileExtLowerPtr = fileExtLower;
        for (int i = 0; (i < fileExtLength) && (i < 16); i++)
        {
            // Copy and convert to lower-case
            if ((fileExt[i] >= 'A') && (fileExt[i] <= 'Z')) fileExtLower[i] =  fileExt[i] + 32;
            else fileExtLower[i] =  fileExt[i];
        }

        int extCount = 1;
        int extLength = (int)strlen(ext);
        char *extList = (char *)RL_CALLOC(extLength + 1, 1);
        char *extListPtrs[MAX_FILE_EXTENSIONS] = { 0 };
        strncpy(extList, ext, extLength);
        extListPtrs[0] = extList;

        for (int i = 0; i < extLength; i++)
        {
            // Convert to lower-case if extension is upper-case
            if ((extList[i] >= 'A') && (extList[i] <= 'Z')) extList[i] += 32;

            // Get pointer to next extension and add null-terminator
            if ((extList[i] == ';') && (extCount < (MAX_FILE_EXTENSIONS - 1)))
            {
                extList[i] = '\0';
                extListPtrs[extCount] = extList + i + 1;
                extCount++;
            }
        }

        for (int i = 0; i < extCount; i++)
        {
            // Consider the case where extension provided
            // does not start with the '.'
            fileExtLowerPtr = fileExtLower;
            if (extListPtrs[i][0] != '.') fileExtLowerPtr++;

            if (strcmp(fileExtLowerPtr, extListPtrs[i]) == 0)
            {
                result = true;
                break;
            }
        }

        RL_FREE(extList);
    }

    return result;
}

// Check if a directory path exists
bool DirectoryExists(const char *dirPath)
{
    bool result = false;
    DIR *dir = opendir(dirPath);

    if (dir != NULL)
    {
        result = true;
        closedir(dir);
    }

    return result;
}

// Get file length in bytes
// NOTE: GetFileSize() conflicts with windows.h
int GetFileLength(const char *fileName)
{
    int size = 0;

    // NOTE: On Unix-like systems, it can by used the POSIX system call: stat(),
    // but depending on the platform that call could not be available
    //struct stat result = { 0 };
    //stat(fileName, &result);
    //return result.st_size;

    FILE *file = fopen(fileName, "rb");

    if (file != NULL)
    {
        fseek(file, 0L, SEEK_END);
        long int fileSize = ftell(file);

        // Check for size overflow (INT_MAX)
        if (fileSize > 2147483647) TRACELOG(LOG_WARNING, "[%s] File size overflows expected limit, do not use GetFileLength()", fileName);
        else size = (int)fileSize;

        fclose(file);
    }

    return size;
}

// Get file modification time (last write time)
long GetFileModTime(const char *fileName)
{
    struct stat result = { 0 };
    long modTime = 0;

    if (stat(fileName, &result) == 0)
    {
        time_t mod = result.st_mtime;
        modTime = (long)mod;
    }

    return modTime;
}

// Get pointer to extension for a filename string (includes the dot: .png)
// WARNING: We just get the ptr but not the extension as a separate string
const char *GetFileExtension(const char *fileName)
{
    const char *dot = strrchr(fileName, '.');

    if (!dot || (dot == fileName)) return NULL;

    return dot;
}

// String pointer reverse break: returns right-most occurrence of charset in s
static const char *strprbrk(const char *text, const char *charset)
{
    const char *latestMatch = NULL;

    for (; (text != NULL) && (text = strpbrk(text, charset)); latestMatch = text++) { }

    return latestMatch;
}

// Get pointer to filename for a path string
const char *GetFileName(const char *filePath)
{
    const char *fileName = NULL;

    if (filePath != NULL) fileName = strprbrk(filePath, "\\/");

    if (fileName == NULL) return filePath;

    return fileName + 1;
}

// Get filename string without extension (uses static string)
const char *GetFileNameWithoutExt(const char *filePath)
{
    #define MAX_FILENAME_LENGTH     256

    static char fileName[MAX_FILENAME_LENGTH] = { 0 };
    memset(fileName, 0, MAX_FILENAME_LENGTH);

    if (filePath != NULL)
    {
        strncpy(fileName, GetFileName(filePath), MAX_FILENAME_LENGTH - 1); // Get filename.ext without path
        int fileNameLenght = (int)strlen(fileName); // Get size in bytes

        for (int i = fileNameLenght; i > 0; i--) // Reverse search '.'
        {
            if (fileName[i] == '.')
            {
                // NOTE: We break on first '.' found
                fileName[i] = '\0';
                break;
            }
        }
    }

    return fileName;
}

// Get directory for a given filePath
const char *GetDirectoryPath(const char *filePath)
{
    /*
    // NOTE: Directory separator is different in Windows and other platforms,
    // fortunately, Windows also support the '/' separator, that's the one should be used
    #if defined(_WIN32)
        char separator = '\\';
    #else
        char separator = '/';
    #endif
    */
    const char *lastSlash = NULL;
    static char dirPath[MAX_FILEPATH_LENGTH] = { 0 };
    memset(dirPath, 0, MAX_FILEPATH_LENGTH);

    // In case provided path does not contain a root drive letter (C:\, D:\) nor leading path separator (\, /),
    // we add the current directory path to dirPath
    if ((filePath[1] != ':') && (filePath[0] != '\\') && (filePath[0] != '/'))
    {
        // For security, we set starting path to current directory,
        // obtained path will be concatenated to this
        dirPath[0] = '.';
        dirPath[1] = '/';
    }

    lastSlash = strprbrk(filePath, "\\/");
    if (lastSlash)
    {
        if (lastSlash == filePath)
        {
            // The last and only slash is the leading one: path is in a root directory
            dirPath[0] = filePath[0];
            dirPath[1] = '\0';
        }
        else
        {
            // NOTE: Be careful, strncpy() is not safe, it does not care about '\0'
            char *dirPathPtr = dirPath;
            if ((filePath[1] != ':') && (filePath[0] != '\\') && (filePath[0] != '/')) dirPathPtr += 2;     // Skip drive letter, "C:"
            memcpy(dirPathPtr, filePath, strlen(filePath) - (strlen(lastSlash) - 1));
            dirPath[strlen(filePath) - strlen(lastSlash) + (((filePath[1] != ':') && (filePath[0] != '\\') && (filePath[0] != '/'))? 2 : 0)] = '\0';  // Add '\0' manually
        }
    }

    return dirPath;
}

// Get previous directory path for a given path
const char *GetPrevDirectoryPath(const char *dirPath)
{
    static char prevDirPath[MAX_FILEPATH_LENGTH] = { 0 };
    memset(prevDirPath, 0, MAX_FILEPATH_LENGTH);
    int dirPathLength = (int)strlen(dirPath);

    if (dirPathLength <= 3) strncpy(prevDirPath, dirPath, MAX_FILEPATH_LENGTH  - 1);

    for (int i = (dirPathLength - 1); (i >= 0) && (dirPathLength > 3); i--)
    {
        if ((dirPath[i] == '\\') || (dirPath[i] == '/'))
        {
            // Check for root: "C:\" or "/"
            if (((i == 2) && (dirPath[1] ==':')) || (i == 0)) i++;

            strncpy(prevDirPath, dirPath, i);
            break;
        }
    }

    return prevDirPath;
}

// Get current working directory
const char *GetWorkingDirectory(void)
{
    static char currentDir[MAX_FILEPATH_LENGTH] = { 0 };
    memset(currentDir, 0, MAX_FILEPATH_LENGTH);

    char *path = GETCWD(currentDir, MAX_FILEPATH_LENGTH - 1);

    return path;
}

const char *GetApplicationDirectory(void)
{
    static char appDir[MAX_FILEPATH_LENGTH] = { 0 };
    memset(appDir, 0, MAX_FILEPATH_LENGTH);

#if defined(_WIN32)
    int len = 0;

    #if defined(UNICODE)
    unsigned short widePath[MAX_PATH];
    len = GetModuleFileNameW(NULL, (wchar_t *)widePath, MAX_PATH);
    len = WideCharToMultiByte(0, 0, (wchar_t *)widePath, len, appDir, MAX_PATH, NULL, NULL);
    #else
    len = GetModuleFileNameA(NULL, appDir, MAX_PATH);
    #endif

    if (len > 0)
    {
        for (int i = len; i >= 0; --i)
        {
            if (appDir[i] == '\\')
            {
                appDir[i + 1] = '\0';
                break;
            }
        }
    }
    else
    {
        appDir[0] = '.';
        appDir[1] = '\\';
    }

#elif defined(__linux__)

    unsigned int size = sizeof(appDir);
    ssize_t len = readlink("/proc/self/exe", appDir, size);

    if (len > 0)
    {
        for (int i = len; i >= 0; --i)
        {
            if (appDir[i] == '/')
            {
                appDir[i + 1] = '\0';
                break;
            }
        }
    }
    else
    {
        appDir[0] = '.';
        appDir[1] = '/';
    }

#elif defined(__APPLE__)

    uint32_t size = sizeof(appDir);

    if (_NSGetExecutablePath(appDir, &size) == 0)
    {
        int appDirLength = (int)strlen(appDir);
        for (int i = appDirLength; i >= 0; --i)
        {
            if (appDir[i] == '/')
            {
                appDir[i + 1] = '\0';
                break;
            }
        }
    }
    else
    {
        appDir[0] = '.';
        appDir[1] = '/';
    }

#elif defined(__FreeBSD__)

    size_t size = sizeof(appDir);
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};

    if (sysctl(mib, 4, appDir, &size, NULL, 0) == 0)
    {
        int appDirLength = (int)strlen(appDir);
        for (int i = appDirLength; i >= 0; --i)
        {
            if (appDir[i] == '/')
            {
                appDir[i + 1] = '\0';
                break;
            }
        }
    }
    else
    {
        appDir[0] = '.';
        appDir[1] = '/';
    }

#elif defined(__wasm__)

    appDir[0] = '/';
#endif

    return appDir;
}

// Load directory filepaths
// NOTE: Base path is prepended to the scanned filepaths
// WARNING: Directory is scanned twice, first time to get files count
// No recursive scanning is done!
FilePathList LoadDirectoryFiles(const char *dirPath)
{
    FilePathList files = { 0 };
    unsigned int fileCounter = 0;

    struct dirent *entity;
    DIR *dir = opendir(dirPath);

    if (dir != NULL) // It's a directory
    {
        // SCAN 1: Count files
        while ((entity = readdir(dir)) != NULL)
        {
            // NOTE: We skip '.' (current dir) and '..' (parent dir) filepaths
            if ((strcmp(entity->d_name, ".") != 0) && (strcmp(entity->d_name, "..") != 0)) fileCounter++;
        }

        // Memory allocation for dirFileCount
        files.capacity = fileCounter;
        files.paths = (char **)RL_CALLOC(files.capacity, sizeof(char *));
        for (unsigned int i = 0; i < files.capacity; i++) files.paths[i] = (char *)RL_CALLOC(MAX_FILEPATH_LENGTH, sizeof(char));

        closedir(dir);

        // SCAN 2: Read filepaths
        // NOTE: Directory paths are also registered
        ScanDirectoryFiles(dirPath, &files, NULL);

        // Security check: read files.count should match fileCounter
        if (files.count != files.capacity) TRACELOG(LOG_WARNING, "FILEIO: Read files count do not match capacity allocated");
    }
    else TRACELOG(LOG_WARNING, "FILEIO: Failed to open requested directory");  // Maybe it's a file...

    return files;
}

// Load directory filepaths with extension filtering and recursive directory scan
// NOTE: On recursive loading we do not pre-scan for file count, we use MAX_FILEPATH_CAPACITY
FilePathList LoadDirectoryFilesEx(const char *basePath, const char *filter, bool scanSubdirs)
{
    FilePathList files = { 0 };

    files.capacity = MAX_FILEPATH_CAPACITY;
    files.paths = (char **)RL_CALLOC(files.capacity, sizeof(char *));
    for (unsigned int i = 0; i < files.capacity; i++) files.paths[i] = (char *)RL_CALLOC(MAX_FILEPATH_LENGTH, sizeof(char));

    // WARNING: basePath is always prepended to scanned paths
    if (scanSubdirs) ScanDirectoryFilesRecursively(basePath, &files, filter);
    else ScanDirectoryFiles(basePath, &files, filter);

    return files;
}

// Unload directory filepaths
// WARNING: files.count is not reseted to 0 after unloading
void UnloadDirectoryFiles(FilePathList files)
{
    if (files.paths != NULL)
    {
        for (unsigned int i = 0; i < files.capacity; i++) RL_FREE(files.paths[i]);

        RL_FREE(files.paths);
    }
}

// Create directories (including full path requested), returns 0 on success
int MakeDirectory(const char *dirPath)
{
    if ((dirPath == NULL) || (dirPath[0] == '\0')) return -1; // Path is not valid
    if (DirectoryExists(dirPath)) return 0; // Path already exists (is valid)

    // Copy path string to avoid modifying original
    int dirPathLength = (int)strlen(dirPath) + 1;
    char *pathcpy = (char *)RL_CALLOC(dirPathLength, 1);
    memcpy(pathcpy, dirPath, dirPathLength);

    // Iterate over pathcpy, create each subdirectory as needed
    for (int i = 0; (i < dirPathLength) && (pathcpy[i] != '\0'); i++)
    {
        if (pathcpy[i] == ':') i++;
        else
        {
            if ((pathcpy[i] == '\\') || (pathcpy[i] == '/'))
            {
                pathcpy[i] = '\0';
                if (!DirectoryExists(pathcpy)) MKDIR(pathcpy);
                pathcpy[i] = '/';
            }
        }
    }

    // Create final directory
    if (!DirectoryExists(pathcpy)) MKDIR(pathcpy);
    RL_FREE(pathcpy);

    // In case something failed and requested directory
    // was not successfully created, return -1
    if (!DirectoryExists(dirPath)) return -1;

    return 0;
}

// Change working directory, returns true on success
bool ChangeDirectory(const char *dirPath)
{
    bool result = CHDIR(dirPath);

    if (result != 0) TRACELOG(LOG_WARNING, "SYSTEM: Failed to change to directory: %s", dirPath);
    else TRACELOG(LOG_INFO, "SYSTEM: Working Directory: %s", dirPath);

    return (result == 0);
}

// Check if a given path point to a file
bool IsPathFile(const char *path)
{
    struct stat result = { 0 };
    stat(path, &result);

    return S_ISREG(result.st_mode);
}

// Check if fileName is valid for the platform/OS
bool IsFileNameValid(const char *fileName)
{
    bool valid = true;

    if ((fileName != NULL) && (fileName[0] != '\0'))
    {
        int fileNameLength = (int)strlen(fileName);
        bool allPeriods = true;

        for (int i = 0; i < fileNameLength; i++)
        {
            // Check invalid characters
            if ((fileName[i] == '<') ||
                (fileName[i] == '>') ||
                (fileName[i] == ':') ||
                (fileName[i] == '\"') ||
                (fileName[i] == '/') ||
                (fileName[i] == '\\') ||
                (fileName[i] == '|') ||
                (fileName[i] == '?') ||
                (fileName[i] == '*')) { valid = false; break; }

            // Check non-glyph characters
            if ((unsigned char)fileName[i] < 32) { valid = false; break; }

            // Check if filename is not all periods
            if (fileName[i] != '.') allPeriods = false;
        }

        if (allPeriods) valid = false;

/*
        if (valid)
        {
            // Check invalid DOS names
            if (fileNameLength >= 3)
            {
                if (((fileName[0] == 'C') && (fileName[1] == 'O') && (fileName[2] == 'N')) ||   // CON
                    ((fileName[0] == 'P') && (fileName[1] == 'R') && (fileName[2] == 'N')) ||   // PRN
                    ((fileName[0] == 'A') && (fileName[1] == 'U') && (fileName[2] == 'X')) ||   // AUX
                    ((fileName[0] == 'N') && (fileName[1] == 'U') && (fileName[2] == 'L'))) valid = false; // NUL
            }

            if (fileNameLength >= 4)
            {
                if (((fileName[0] == 'C') && (fileName[1] == 'O') && (fileName[2] == 'M') && ((fileName[3] >= '0') && (fileName[3] <= '9'))) ||  // COM0-9
                    ((fileName[0] == 'L') && (fileName[1] == 'P') && (fileName[2] == 'T') && ((fileName[3] >= '0') && (fileName[3] <= '9')))) valid = false; // LPT0-9
            }
        }
*/
    }

    return valid;
}

// Check if a file has been dropped into window
bool IsFileDropped(void)
{
    bool result = false;

    if (CORE.Window.dropFileCount > 0) result = true;

    return result;
}

// Load dropped filepaths
FilePathList LoadDroppedFiles(void)
{
    FilePathList files = { 0 };

    files.count = CORE.Window.dropFileCount;
    files.paths = CORE.Window.dropFilepaths;

    return files;
}

// Unload dropped filepaths
void UnloadDroppedFiles(FilePathList files)
{
    // WARNING: files pointers are the same as internal ones

    if (files.count > 0)
    {
        for (unsigned int i = 0; i < files.count; i++) RL_FREE(files.paths[i]);

        RL_FREE(files.paths);

        CORE.Window.dropFileCount = 0;
        CORE.Window.dropFilepaths = NULL;
    }
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Compression and Encoding
//----------------------------------------------------------------------------------

// Compress data (DEFLATE algorithm)
unsigned char *CompressData(const unsigned char *data, int dataSize, int *compDataSize)
{
    #define COMPRESSION_QUALITY_DEFLATE  8

    unsigned char *compData = NULL;

#if defined(SUPPORT_COMPRESSION_API)
    // Compress data and generate a valid DEFLATE stream
    struct sdefl *sdefl = (struct sdefl *)RL_CALLOC(1, sizeof(struct sdefl));   // WARNING: Possible stack overflow, struct sdefl is almost 1MB
    int bounds = sdefl_bound(dataSize);
    compData = (unsigned char *)RL_CALLOC(bounds, 1);

    *compDataSize = sdeflate(sdefl, compData, data, dataSize, COMPRESSION_QUALITY_DEFLATE);   // Compression level 8, same as stbiw
    RL_FREE(sdefl);

    TRACELOG(LOG_INFO, "SYSTEM: Compress data: Original size: %i -> Comp. size: %i", dataSize, *compDataSize);
#endif

    return compData;
}

// Decompress data (DEFLATE algorithm)
unsigned char *DecompressData(const unsigned char *compData, int compDataSize, int *dataSize)
{
    unsigned char *data = NULL;

#if defined(SUPPORT_COMPRESSION_API)
    // Decompress data from a valid DEFLATE stream
    unsigned char *data0 = (unsigned char *)RL_CALLOC(MAX_DECOMPRESSION_SIZE*1024*1024, 1);
    int size = sinflate(data0, MAX_DECOMPRESSION_SIZE*1024*1024, compData, compDataSize);

    // WARNING: RL_REALLOC can make (and leave) data copies in memory,
    // that can be a security concern in case of compression of sensitive data
    // So, we use a second buffer to copy data manually, wiping original buffer memory
    data = (unsigned char *)RL_CALLOC(size, 1);
    memcpy(data, data0, size);
    memset(data0, 0, MAX_DECOMPRESSION_SIZE*1024*1024); // Wipe memory, is memset() safe?
    RL_FREE(data0);

    TRACELOG(LOG_INFO, "SYSTEM: Decompress data: Comp. size: %i -> Original size: %i", compDataSize, size);

    *dataSize = size;
#endif

    return data;
}

// Encode data to Base64 string
// NOTE: Returned string includes NULL terminator, considered on outputSize
char *EncodeDataBase64(const unsigned char *data, int dataSize, int *outputSize)
{
    // Base64 conversion table from RFC 4648 [0..63]
    // NOTE: They represent 64 values (6 bits), to encode 3 bytes of data into 4 "sixtets" (6bit characters)
    static const char base64EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    // Compute expected size and padding
    int paddedSize = dataSize;
    while (paddedSize%3 != 0) paddedSize++; // Padding bytes to round 4*(dataSize/3) to 4 bytes
    int estimatedOutputSize = 4*(paddedSize/3);
    int padding = paddedSize - dataSize;

    // Adding null terminator to string
    estimatedOutputSize += 1;

    // Load some memory to store encoded string
    char *encodedData = (char *)RL_CALLOC(estimatedOutputSize, 1);
    if (encodedData == NULL) return NULL;

    int outputCount = 0;
    for (int i = 0; i < dataSize;)
    {
        unsigned int octetA = 0;
        unsigned int octetB = 0;
        unsigned int octetC = 0;
        unsigned int octetPack = 0;

        octetA = data[i]; // Generates 2 sextets
        octetB = ((i + 1) < dataSize)? data[i + 1] : 0; // Generates 3 sextets
        octetC = ((i + 2) < dataSize)? data[i + 2] : 0; // Generates 4 sextets

        octetPack = (octetA << 16) | (octetB << 8) | octetC;

        encodedData[outputCount + 0] = (unsigned char)(base64EncodeTable[(octetPack >> 18) & 0x3f]);
        encodedData[outputCount + 1] = (unsigned char)(base64EncodeTable[(octetPack >> 12) & 0x3f]);
        encodedData[outputCount + 2] = (unsigned char)(base64EncodeTable[(octetPack >> 6) & 0x3f]);
        encodedData[outputCount + 3] = (unsigned char)(base64EncodeTable[octetPack & 0x3f]);
        outputCount += 4;
        i += 3;
    }

    // Add required padding bytes
    for (int p = 0; p < padding; p++) encodedData[outputCount - p - 1] = '=';

    // Add null terminator to string
    encodedData[outputCount] = '\0';
    outputCount++;

    if (outputCount != estimatedOutputSize) TRACELOG(LOG_WARNING, "BASE64: Output size differs from estimation");

    *outputSize = estimatedOutputSize;
    return encodedData;
}

// Decode Base64 string (expected NULL terminated)
unsigned char *DecodeDataBase64(const char *text, int *outputSize)
{
    // Base64 decode table
    // NOTE: Following ASCII order [0..255] assigning the expected sixtet value to
    // every character in the corresponding ASCII position
    static const unsigned char base64DecodeTable[256] = {
        ['A'] =  0, ['B'] =  1, ['C'] =  2, ['D'] =  3, ['E'] =  4, ['F'] =  5, ['G'] =  6, ['H'] =  7,
        ['I'] =  8, ['J'] =  9, ['K'] = 10, ['L'] = 11, ['M'] = 12, ['N'] = 13, ['O'] = 14, ['P'] = 15,
        ['Q'] = 16, ['R'] = 17, ['S'] = 18, ['T'] = 19, ['U'] = 20, ['V'] = 21, ['W'] = 22, ['X'] = 23, ['Y'] = 24, ['Z'] = 25,
        ['a'] = 26, ['b'] = 27, ['c'] = 28, ['d'] = 29, ['e'] = 30, ['f'] = 31, ['g'] = 32, ['h'] = 33,
        ['i'] = 34, ['j'] = 35, ['k'] = 36, ['l'] = 37, ['m'] = 38, ['n'] = 39, ['o'] = 40, ['p'] = 41,
        ['q'] = 42, ['r'] = 43, ['s'] = 44, ['t'] = 45, ['u'] = 46, ['v'] = 47, ['w'] = 48, ['x'] = 49, ['y'] = 50, ['z'] = 51,
        ['0'] = 52, ['1'] = 53, ['2'] = 54, ['3'] = 55, ['4'] = 56, ['5'] = 57, ['6'] = 58, ['7'] = 59,
        ['8'] = 60, ['9'] = 61, ['+'] = 62, ['/'] = 63
    };

    *outputSize = 0;
    if (text == NULL) return NULL;

    // Compute expected size and padding
    int dataSize = (int)strlen(text); // WARNING: Expecting NULL terminated strings!
    int ending = dataSize - 1;
    int padding = 0;
    while (text[ending] == '=') { padding++; ending--; }
    int estimatedOutputSize = 3*(dataSize/4) - padding;
    int maxOutputSize = 3*(dataSize/4);

    // Load some memory to store decoded data
    // NOTE: Allocated enough size to include padding
    unsigned char *decodedData = (unsigned char *)RL_CALLOC(maxOutputSize, 1);
    if (decodedData == NULL) return NULL;

    int outputCount = 0;
    for (int i = 0; i < dataSize;)
    {
        // Every 4 sixtets must generate 3 octets
        if ((i + 2) >= dataSize)
        {
            TRACELOG(LOG_WARNING, "BASE64: Decoding error: Input data size is not valid");
            break;
        }

        unsigned int sixtetA = base64DecodeTable[(unsigned char)text[i]];
        unsigned int sixtetB = base64DecodeTable[(unsigned char)text[i + 1]];
        unsigned int sixtetC = (((i + 2) < dataSize) && (unsigned char)text[i + 2] != '=')? base64DecodeTable[(unsigned char)text[i + 2]] : 0;
        unsigned int sixtetD = (((i + 3) < dataSize) && (unsigned char)text[i + 3] != '=')? base64DecodeTable[(unsigned char)text[i + 3]] : 0;

        unsigned int octetPack = (sixtetA << 18) | (sixtetB << 12)  | (sixtetC << 6) | sixtetD;

        if ((outputCount + 3) > maxOutputSize)
        {
            TRACELOG(LOG_WARNING, "BASE64: Decoding error: Output data size is too small");
            break;
        }

        decodedData[outputCount + 0] = (octetPack >> 16) & 0xff;
        decodedData[outputCount + 1] = (octetPack >> 8) & 0xff;
        decodedData[outputCount + 2] = octetPack & 0xff;
        outputCount += 3;
        i += 4;
    }

    if (estimatedOutputSize != (outputCount - padding)) TRACELOG(LOG_WARNING, "BASE64: Decoded size differs from estimation");

    *outputSize = estimatedOutputSize;
    return decodedData;
}

// Compute CRC32 hash code
unsigned int ComputeCRC32(unsigned char *data, int dataSize)
{
    static unsigned int crcTable[256] = {
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
        0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
        0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
        0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
        0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
        0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
        0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
        0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
        0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
        0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
        0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
        0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
        0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
        0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
        0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
        0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
        0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
        0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
        0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
        0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
        0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
        0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
        0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
        0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
        0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
        0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
        0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
        0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
        0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
        0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
    };

    unsigned int crc = ~0u;

    for (int i = 0; i < dataSize; i++) crc = (crc >> 8) ^ crcTable[data[i] ^ (crc & 0xff)];

    return ~crc;
}

// Compute MD5 hash code
// NOTE: Returns a static int[4] array (16 bytes)
unsigned int *ComputeMD5(unsigned char *data, int dataSize)
{
    #define ROTATE_LEFT(x, c) (((x) << (c)) | ((x) >> (32 - (c))))

    static unsigned int hash[4] = { 0 };  // Hash to be returned

    // WARNING: All variables are unsigned 32 bit and wrap modulo 2^32 when calculating

    // NOTE: r specifies the per-round shift amounts
    unsigned int r[] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
    };

    // Using binary integer part of the sines of integers (in radians) as constants
    unsigned int k[] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
    };

    hash[0] = 0x67452301;
    hash[1] = 0xefcdab89;
    hash[2] = 0x98badcfe;
    hash[3] = 0x10325476;

    // Pre-processing: adding a single 1 bit
    // Append '1' bit to message
    // NOTE: The input bytes are considered as bits strings,
    // where the first bit is the most significant bit of the byte

    // Pre-processing: padding with zeros
    // Append '0' bit until message length in bit 448 (mod 512)
    // Append length mod (2 pow 64) to message

    int newDataSize = ((((dataSize + 8)/64) + 1)*64) - 8;

    unsigned char *msg = (unsigned char *)RL_CALLOC(newDataSize + 64, 1); // Initialize with '0' bits, allocating 64 extra bytes
    memcpy(msg, data, dataSize);
    msg[dataSize] = 128; // Write the '1' bit

    unsigned int bitsLen = 8*dataSize;
    memcpy(msg + newDataSize, &bitsLen, 4); // Append the len in bits at the end of the buffer

    // Process the message in successive 512-bit chunks for each 512-bit chunk of message
    for (int offset = 0; offset < newDataSize; offset += (512/8))
    {
        // Break chunk into sixteen 32-bit words w[j], 0 <= j <= 15
        unsigned int *w = (unsigned int *)(msg + offset);

        // Initialize hash value for this chunk
        unsigned int a = hash[0];
        unsigned int b = hash[1];
        unsigned int c = hash[2];
        unsigned int d = hash[3];

        for (int i = 0; i < 64; i++)
        {
            unsigned int f = 0;
            unsigned int g = 0;

            if (i < 16)
            {
                f = (b & c) | ((~b) & d);
                g = i;
            }
            else if (i < 32)
            {
                f = (d & b) | ((~d) & c);
                g = (5*i + 1)%16;
            }
            else if (i < 48)
            {
                f = b ^ c ^ d;
                g = (3*i + 5)%16;
            }
            else
            {
                f = c ^ (b | (~d));
                g = (7*i)%16;
            }

            unsigned int temp = d;
            d = c;
            c = b;
            b = b + ROTATE_LEFT((a + f + k[i] + w[g]), r[i]);
            a = temp;
        }

        // Add chunk's hash to result so far
        hash[0] += a;
        hash[1] += b;
        hash[2] += c;
        hash[3] += d;
    }

    RL_FREE(msg);

    return hash;
}

// Compute SHA-1 hash code
// NOTE: Returns a static int[5] array (20 bytes)
unsigned int *ComputeSHA1(unsigned char *data, int dataSize)
{
    #define SHA1_ROTATE_LEFT(x, c) (((x) << (c)) | ((x) >> (32 - (c))))

    static unsigned int hash[5] = { 0 };  // Hash to be returned

    // Initialize hash values
    hash[0] = 0x67452301;
    hash[1] = 0xEFCDAB89;
    hash[2] = 0x98BADCFE;
    hash[3] = 0x10325476;
    hash[4] = 0xC3D2E1F0;

    // Pre-processing: adding a single 1 bit
    // Append '1' bit to message
    // NOTE: The input bytes are considered as bits strings,
    // where the first bit is the most significant bit of the byte

    // Pre-processing: padding with zeros
    // Append '0' bit until message length in bit 448 (mod 512)
    // Append length mod (2 pow 64) to message

    int newDataSize = ((((dataSize + 8)/64) + 1)*64);

    unsigned char *msg = (unsigned char *)RL_CALLOC(newDataSize, 1); // Initialize with '0' bits
    memcpy(msg, data, dataSize);
    msg[dataSize] = 128; // Write the '1' bit

    unsigned long long bitsLen = 8ULL * dataSize;
    msg[newDataSize-1] = (unsigned char)(bitsLen);
    msg[newDataSize-2] = (unsigned char)(bitsLen >> 8);
    msg[newDataSize-3] = (unsigned char)(bitsLen >> 16);
    msg[newDataSize-4] = (unsigned char)(bitsLen >> 24);
    msg[newDataSize-5] = (unsigned char)(bitsLen >> 32);
    msg[newDataSize-6] = (unsigned char)(bitsLen >> 40);
    msg[newDataSize-7] = (unsigned char)(bitsLen >> 48);
    msg[newDataSize-8] = (unsigned char)(bitsLen >> 56);

    // Process the message in successive 512-bit chunks
    for (int offset = 0; offset < newDataSize; offset += (512/8))
    {
        // Break chunk into sixteen 32-bit words w[j], 0 <= j <= 15
        unsigned int w[80] = { 0 };
        for (int i = 0; i < 16; i++)
        {
            w[i] = (msg[offset + (i*4) + 0] << 24) |
                   (msg[offset + (i*4) + 1] << 16) |
                   (msg[offset + (i*4) + 2] << 8) |
                   (msg[offset + (i*4) + 3]);
        }

        // Message schedule: extend the sixteen 32-bit words into eighty 32-bit words:
        for (int i = 16; i < 80; i++) w[i] = SHA1_ROTATE_LEFT(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);

        // Initialize hash value for this chunk
        unsigned int a = hash[0];
        unsigned int b = hash[1];
        unsigned int c = hash[2];
        unsigned int d = hash[3];
        unsigned int e = hash[4];

        for (int i = 0; i < 80; i++)
        {
            unsigned int f = 0;
            unsigned int k = 0;

            if (i < 20)
            {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            }
            else if (i < 40)
            {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            }
            else if (i < 60)
            {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            }
            else
            {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }

            unsigned int temp = SHA1_ROTATE_LEFT(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = SHA1_ROTATE_LEFT(b, 30);
            b = a;
            a = temp;
        }

        // Add this chunk's hash to result so far
        hash[0] += a;
        hash[1] += b;
        hash[2] += c;
        hash[3] += d;
        hash[4] += e;
    }

    RL_FREE(msg);

    return hash;
}

// Compute SHA-256 hash code
// NOTE: Returns a static int[8] array (32 bytes)
unsigned int *ComputeSHA256(unsigned char *data, int dataSize)
{
    #define SHA256_ROTATE_RIGHT(x, c) ((x >> c) | (x << ((sizeof(unsigned int)*8) - c)))
    #define SHA256_A0(x) (SHA256_ROTATE_RIGHT(x, 7) ^ SHA256_ROTATE_RIGHT(x, 18) ^ (x >> 3))
    #define SHA256_A1(x) (SHA256_ROTATE_RIGHT(x, 17) ^ SHA256_ROTATE_RIGHT(x, 19) ^ (x >> 10))

    static const unsigned int k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    static unsigned int hash[8] = { 0 };
    hash[0] = 0x6A09e667;
    hash[1] = 0xbb67ae85;
    hash[2] = 0x3c6ef372;
    hash[3] = 0xa54ff53a;
    hash[4] = 0x510e527f;
    hash[5] = 0x9b05688c;
    hash[6] = 0x1f83d9ab;
    hash[7] = 0x5be0cd19;

    const unsigned long long int bitLen = ((unsigned long long int)dataSize)*8;
    unsigned long long int paddedSize = dataSize + sizeof(dataSize);
    paddedSize += (64 - (paddedSize%64));
    unsigned char *buffer = (unsigned char *)RL_CALLOC(paddedSize, sizeof(unsigned char));

    memcpy(buffer, data, dataSize);
    buffer[dataSize] = 0x80;
    for (int i = 1; i <= sizeof(bitLen); i++)
    {
        buffer[(paddedSize - sizeof(bitLen)) + (i - 1)] = (bitLen >> (8*(sizeof(bitLen) - i))) & 0xFF;
    }

    for (unsigned long long int blockN = 0; blockN < paddedSize/64; blockN++)
    {
        unsigned int a = hash[0];
        unsigned int b = hash[1];
        unsigned int c = hash[2];
        unsigned int d = hash[3];
        unsigned int e = hash[4];
        unsigned int f = hash[5];
        unsigned int g = hash[6];
        unsigned int h = hash[7];

        unsigned char *block = buffer + (blockN*64);
        unsigned int w[64] = { 0 };
        for (int i = 0; i < 16; i++)
        {
            w[i] = ((unsigned int)block[i*4 + 0] << 24) |
                   ((unsigned int)block[i*4 + 1] << 16) |
                   ((unsigned int)block[i*4 + 2] << 8)  |
                   ((unsigned int)block[i*4 + 3]);
        }
        for (int t = 16; t < 64; t++) w[t] = SHA256_A1(w[t - 2]) + w[t - 7] + SHA256_A0(w[t - 15]) + w[t - 16];

        for (unsigned long long int t = 0; t < 64; t++)
        {
            unsigned int e1 = (SHA256_ROTATE_RIGHT(e, 6) ^ SHA256_ROTATE_RIGHT(e, 11) ^ SHA256_ROTATE_RIGHT(e, 25));
            unsigned int ch = ((e & f) ^ (~e & g));
            unsigned int t1 = (h + e1 + ch + k[t] + w[t]);
            unsigned int e0 = (SHA256_ROTATE_RIGHT(a, 2) ^ SHA256_ROTATE_RIGHT(a, 13) ^ SHA256_ROTATE_RIGHT(a, 22));
            unsigned int maj = ((a & b) ^ (a & c) ^ (b & c));
            unsigned int t2 = e0 + maj;

            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        hash[0] += a;
        hash[1] += b;
        hash[2] += c;
        hash[3] += d;
        hash[4] += e;
        hash[5] += f;
        hash[6] += g;
        hash[7] += h;
    }

    RL_FREE(buffer);

    return hash;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Automation Events Recording and Playing
//----------------------------------------------------------------------------------

// Load automation events list from file, NULL for empty list, capacity = MAX_AUTOMATION_EVENTS
AutomationEventList LoadAutomationEventList(const char *fileName)
{
    AutomationEventList list = { 0 };

    // Allocate and empty automation event list, ready to record new events
    list.events = (AutomationEvent *)RL_CALLOC(MAX_AUTOMATION_EVENTS, sizeof(AutomationEvent));
    list.capacity = MAX_AUTOMATION_EVENTS;

#if defined(SUPPORT_AUTOMATION_EVENTS)
    if (fileName == NULL) TRACELOG(LOG_INFO, "AUTOMATION: New empty events list loaded successfully");
    else
    {
        // Load automation events file (binary)
        /*
        //int dataSize = 0;
        //unsigned char *data = LoadFileData(fileName, &dataSize);

        FILE *raeFile = fopen(fileName, "rb");
        unsigned char fileId[4] = { 0 };

        fread(fileId, 1, 4, raeFile);

        if ((fileId[0] == 'r') && (fileId[1] == 'A') && (fileId[2] == 'E') && (fileId[1] == ' '))
        {
            fread(&eventCount, sizeof(int), 1, raeFile);
            TRACELOG(LOG_WARNING, "Events loaded: %i\n", eventCount);
            fread(events, sizeof(AutomationEvent), eventCount, raeFile);
        }

        fclose(raeFile);
        */

        // Load events file (text)
        //unsigned char *buffer = LoadFileText(fileName);
        FILE *raeFile = fopen(fileName, "rt");

        if (raeFile != NULL)
        {
            unsigned int counter = 0;
            char buffer[256] = { 0 };
            char eventDesc[64] = { 0 };

            char *result = fgets(buffer, 256, raeFile);
            if (result != buffer) TRACELOG(LOG_WARNING, "AUTOMATION: [%s] Issue reading line to buffer", fileName);

            while (!feof(raeFile))
            {
                switch (buffer[0])
                {
                    case 'c': sscanf(buffer, "c %i", &list.count); break;
                    case 'e':
                    {
                        sscanf(buffer, "e %d %d %d %d %d %d %[^\n]s", &list.events[counter].frame, &list.events[counter].type,
                               &list.events[counter].params[0], &list.events[counter].params[1], &list.events[counter].params[2], &list.events[counter].params[3], eventDesc);

                        counter++;
                    } break;
                    default: break;
                }

                result = fgets(buffer, 256, raeFile);
                if (result != buffer) TRACELOG(LOG_WARNING, "AUTOMATION: [%s] Issue reading line to buffer", fileName);
            }

            if (counter != list.count)
            {
                TRACELOG(LOG_WARNING, "AUTOMATION: Events read from file [%i] do not mach event count specified [%i]", counter, list.count);
                list.count = counter;
            }

            fclose(raeFile);

            TRACELOG(LOG_INFO, "AUTOMATION: Events file loaded successfully");
        }

        TRACELOG(LOG_INFO, "AUTOMATION: Events loaded from file: %i", list.count);
    }
#endif
    return list;
}

// Unload automation events list from file
void UnloadAutomationEventList(AutomationEventList list)
{
#if defined(SUPPORT_AUTOMATION_EVENTS)
    RL_FREE(list.events);
#endif
}

// Export automation events list as text file
bool ExportAutomationEventList(AutomationEventList list, const char *fileName)
{
    bool success = false;

#if defined(SUPPORT_AUTOMATION_EVENTS)
    // Export events as binary file
    // NOTE: Code not used, only for reference if required in the future
    /*
    if (list.count > 0)
    {
        int binarySize = 4 + sizeof(int) + sizeof(AutomationEvent)*list.count;
        unsigned char *binBuffer = (unsigned char *)RL_CALLOC(binarySize, 1);
        int offset = 0;
        memcpy(binBuffer + offset, "rAE ", 4); 
        offset += 4;
        memcpy(binBuffer + offset, &list.count, sizeof(int)); 
        offset += sizeof(int);
        memcpy(binBuffer + offset, list.events, sizeof(AutomationEvent)*list.count);
        offset += sizeof(AutomationEvent)*list.count;
        
        success = SaveFileData(TextFormat("%s.rae",fileName), binBuffer, binarySize);
        RL_FREE(binBuffer);
    }
    */

    // Export events as text
    // NOTE: Save to memory buffer and SaveFileText()
    char *txtData = (char *)RL_CALLOC(256*list.count + 2048, sizeof(char)); // 256 characters per line plus some header

    int byteCount = 0;
    byteCount += sprintf(txtData + byteCount, "#\n");
    byteCount += sprintf(txtData + byteCount, "# Automation events exporter v1.0 - raylib automation events list\n");
    byteCount += sprintf(txtData + byteCount, "#\n");
    byteCount += sprintf(txtData + byteCount, "#    c <events_count>\n");
    byteCount += sprintf(txtData + byteCount, "#    e <frame> <event_type> <param0> <param1> <param2> <param3> // <event_type_name>\n");
    byteCount += sprintf(txtData + byteCount, "#\n");
    byteCount += sprintf(txtData + byteCount, "# more info and bugs-report:  github.com/raysan5/raylib\n");
    byteCount += sprintf(txtData + byteCount, "# feedback and support:       ray[at]raylib.com\n");
    byteCount += sprintf(txtData + byteCount, "#\n");
    byteCount += sprintf(txtData + byteCount, "# Copyright (c) 2023-2026 Ramon Santamaria (@raysan5)\n");
    byteCount += sprintf(txtData + byteCount, "#\n\n");

    // Add events data
    byteCount += sprintf(txtData + byteCount, "c %i\n", list.count);
    for (unsigned int i = 0; i < list.count; i++)
    {
        byteCount += snprintf(txtData + byteCount, 256, "e %i %i %i %i %i %i // Event: %s\n", list.events[i].frame, list.events[i].type,
            list.events[i].params[0], list.events[i].params[1], list.events[i].params[2], list.events[i].params[3], autoEventTypeName[list.events[i].type]);
    }

    // NOTE: Text data size exported is determined by '\0' (NULL) character
    success = SaveFileText(fileName, txtData);

    RL_FREE(txtData);
#endif

    return success;
}

// Setup automation event list to record to
void SetAutomationEventList(AutomationEventList *list)
{
#if defined(SUPPORT_AUTOMATION_EVENTS)
    currentEventList = list;
#endif
}

// Set automation event internal base frame to start recording
void SetAutomationEventBaseFrame(int frame)
{
    CORE.Time.frameCounter = frame;
}

// Start recording automation events (AutomationEventList must be set)
void StartAutomationEventRecording(void)
{
#if defined(SUPPORT_AUTOMATION_EVENTS)
    automationEventRecording = true;
#endif
}

// Stop recording automation events
void StopAutomationEventRecording(void)
{
#if defined(SUPPORT_AUTOMATION_EVENTS)
    automationEventRecording = false;
#endif
}

// Play a recorded automation event
void PlayAutomationEvent(AutomationEvent event)
{
#if defined(SUPPORT_AUTOMATION_EVENTS)
    // WARNING: When should event be played? After/before/replace PollInputEvents()? -> Up to the user!

    if (!automationEventRecording)
    {
        switch (event.type)
        {
            // Input event
            case INPUT_KEY_UP: CORE.Input.Keyboard.currentKeyState[event.params[0]] = false; break;             // param[0]: key
            case INPUT_KEY_DOWN: {                                                                              // param[0]: key
                CORE.Input.Keyboard.currentKeyState[event.params[0]] = true;

                if (CORE.Input.Keyboard.previousKeyState[event.params[0]] == false)
                {
                    if (CORE.Input.Keyboard.keyPressedQueueCount < MAX_KEY_PRESSED_QUEUE)
                    {
                        // Add character to the queue
                        CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = event.params[0];
                        CORE.Input.Keyboard.keyPressedQueueCount++;
                    }
                }
            } break;
            case INPUT_MOUSE_BUTTON_UP: CORE.Input.Mouse.currentButtonState[event.params[0]] = false; break;    // param[0]: key
            case INPUT_MOUSE_BUTTON_DOWN: CORE.Input.Mouse.currentButtonState[event.params[0]] = true; break;   // param[0]: key
            case INPUT_MOUSE_POSITION:      // param[0]: x, param[1]: y
            {
                CORE.Input.Mouse.currentPosition.x = (float)event.params[0];
                CORE.Input.Mouse.currentPosition.y = (float)event.params[1];
            } break;
            case INPUT_MOUSE_WHEEL_MOTION:  // param[0]: x delta, param[1]: y delta
            {
                CORE.Input.Mouse.currentWheelMove.x = (float)event.params[0];
                CORE.Input.Mouse.currentWheelMove.y = (float)event.params[1];
            } break;
            case INPUT_TOUCH_UP: CORE.Input.Touch.currentTouchState[event.params[0]] = false; break;            // param[0]: id
            case INPUT_TOUCH_DOWN: CORE.Input.Touch.currentTouchState[event.params[0]] = true; break;           // param[0]: id
            case INPUT_TOUCH_POSITION:      // param[0]: id, param[1]: x, param[2]: y
            {
                CORE.Input.Touch.position[event.params[0]].x = (float)event.params[1];
                CORE.Input.Touch.position[event.params[0]].y = (float)event.params[2];
            } break;
            case INPUT_GAMEPAD_CONNECT: CORE.Input.Gamepad.ready[event.params[0]] = true; break;                // param[0]: gamepad
            case INPUT_GAMEPAD_DISCONNECT: CORE.Input.Gamepad.ready[event.params[0]] = false; break;            // param[0]: gamepad
            case INPUT_GAMEPAD_BUTTON_UP: CORE.Input.Gamepad.currentButtonState[event.params[0]][event.params[1]] = false; break;    // param[0]: gamepad, param[1]: button
            case INPUT_GAMEPAD_BUTTON_DOWN: CORE.Input.Gamepad.currentButtonState[event.params[0]][event.params[1]] = true; break;   // param[0]: gamepad, param[1]: button
            case INPUT_GAMEPAD_AXIS_MOTION: // param[0]: gamepad, param[1]: axis, param[2]: delta
            {
                CORE.Input.Gamepad.axisState[event.params[0]][event.params[1]] = ((float)event.params[2]/32768.0f);
            } break;
    #if defined(SUPPORT_GESTURES_SYSTEM)
            case INPUT_GESTURE: GESTURES.current = event.params[0]; break;     // param[0]: gesture (enum Gesture) -> rgestures.h: GESTURES.current
    #endif
            // Window event
            case WINDOW_CLOSE: CORE.Window.shouldClose = true; break;
            case WINDOW_MAXIMIZE: MaximizeWindow(); break;
            case WINDOW_MINIMIZE: MinimizeWindow(); break;
            case WINDOW_RESIZE: SetWindowSize(event.params[0], event.params[1]); break;

            // Custom event
    #if defined(SUPPORT_SCREEN_CAPTURE)
            case ACTION_TAKE_SCREENSHOT:
            {
                TakeScreenshot(TextFormat("screenshot%03i.png", screenshotCounter));
                screenshotCounter++;
            } break;
    #endif
            case ACTION_SETTARGETFPS: SetTargetFPS(event.params[0]); break;
            default: break;
        }

        TRACELOG(LOG_INFO, "AUTOMATION PLAY: Frame: %i | Event type: %i | Event parameters: %i, %i, %i", event.frame, event.type, event.params[0], event.params[1], event.params[2]);
    }
#endif
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Input Handling: Keyboard
//----------------------------------------------------------------------------------

// Check if a key has been pressed once
bool IsKeyPressed(int key)
{

    bool pressed = false;

    if ((key > 0) && (key < MAX_KEYBOARD_KEYS))
    {
        if ((CORE.Input.Keyboard.previousKeyState[key] == 0) && (CORE.Input.Keyboard.currentKeyState[key] == 1)) pressed = true;
    }

    return pressed;
}

// Check if a key has been pressed again
bool IsKeyPressedRepeat(int key)
{
    bool repeat = false;

    if ((key > 0) && (key < MAX_KEYBOARD_KEYS))
    {
        if (CORE.Input.Keyboard.keyRepeatInFrame[key] == 1) repeat = true;
    }

    return repeat;
}

// Check if a key is being pressed (key held down)
bool IsKeyDown(int key)
{
    bool down = false;

    if ((key > 0) && (key < MAX_KEYBOARD_KEYS))
    {
        if (CORE.Input.Keyboard.currentKeyState[key] == 1) down = true;
    }

    return down;
}

// Check if a key has been released once
bool IsKeyReleased(int key)
{
    bool released = false;

    if ((key > 0) && (key < MAX_KEYBOARD_KEYS))
    {
        if ((CORE.Input.Keyboard.previousKeyState[key] == 1) && (CORE.Input.Keyboard.currentKeyState[key] == 0)) released = true;
    }

    return released;
}

// Check if a key is NOT being pressed (key not held down)
bool IsKeyUp(int key)
{
    bool up = false;

    if ((key > 0) && (key < MAX_KEYBOARD_KEYS))
    {
        if (CORE.Input.Keyboard.currentKeyState[key] == 0) up = true;
    }

    return up;
}

// Get the last key pressed
int GetKeyPressed(void)
{
    int value = 0;

    if (CORE.Input.Keyboard.keyPressedQueueCount > 0)
    {
        // Get character from the queue head
        value = CORE.Input.Keyboard.keyPressedQueue[0];

        // Shift elements 1 step toward the head
        for (int i = 0; i < (CORE.Input.Keyboard.keyPressedQueueCount - 1); i++)
            CORE.Input.Keyboard.keyPressedQueue[i] = CORE.Input.Keyboard.keyPressedQueue[i + 1];

        // Reset last character in the queue
        CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount - 1] = 0;
        CORE.Input.Keyboard.keyPressedQueueCount--;
    }

    return value;
}

// Get the last char pressed
int GetCharPressed(void)
{
    int value = 0;

    if (CORE.Input.Keyboard.charPressedQueueCount > 0)
    {
        // Get character from the queue head
        value = CORE.Input.Keyboard.charPressedQueue[0];

        // Shift elements 1 step toward the head
        for (int i = 0; i < (CORE.Input.Keyboard.charPressedQueueCount - 1); i++)
            CORE.Input.Keyboard.charPressedQueue[i] = CORE.Input.Keyboard.charPressedQueue[i + 1];

        // Reset last character in the queue
        CORE.Input.Keyboard.charPressedQueue[CORE.Input.Keyboard.charPressedQueueCount - 1] = 0;
        CORE.Input.Keyboard.charPressedQueueCount--;
    }

    return value;
}

// Set a custom key to exit program
// NOTE: default exitKey is set to ESCAPE
void SetExitKey(int key)
{
    CORE.Input.Keyboard.exitKey = key;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Input Handling: Gamepad
//----------------------------------------------------------------------------------

// NOTE: Functions with a platform-specific implementation on rcore_<platform>.c
//int SetGamepadMappings(const char *mappings)

// Check if a gamepad is available
bool IsGamepadAvailable(int gamepad)
{
    bool result = false;

    if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad]) result = true;

    return result;
}

// Get gamepad internal name id
const char *GetGamepadName(int gamepad)
{
    return CORE.Input.Gamepad.name[gamepad];
}

// Check if a gamepad button has been pressed once
bool IsGamepadButtonPressed(int gamepad, int button)
{
    bool pressed = false;

    if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad] && (button < MAX_GAMEPAD_BUTTONS) &&
        (CORE.Input.Gamepad.previousButtonState[gamepad][button] == 0) && (CORE.Input.Gamepad.currentButtonState[gamepad][button] == 1)) pressed = true;

    return pressed;
}

// Check if a gamepad button is being pressed
bool IsGamepadButtonDown(int gamepad, int button)
{
    bool down = false;

    if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad] && (button < MAX_GAMEPAD_BUTTONS) &&
        (CORE.Input.Gamepad.currentButtonState[gamepad][button] == 1)) down = true;

    return down;
}

// Check if a gamepad button has NOT been pressed once
bool IsGamepadButtonReleased(int gamepad, int button)
{
    bool released = false;

    if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad] && (button < MAX_GAMEPAD_BUTTONS) &&
        (CORE.Input.Gamepad.previousButtonState[gamepad][button] == 1) && (CORE.Input.Gamepad.currentButtonState[gamepad][button] == 0)) released = true;

    return released;
}

// Check if a gamepad button is NOT being pressed
bool IsGamepadButtonUp(int gamepad, int button)
{
    bool up = false;

    if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad] && (button < MAX_GAMEPAD_BUTTONS) &&
        (CORE.Input.Gamepad.currentButtonState[gamepad][button] == 0)) up = true;

    return up;
}

// Get the last gamepad button pressed
int GetGamepadButtonPressed(void)
{
    return CORE.Input.Gamepad.lastButtonPressed;
}

// Get gamepad axis count
int GetGamepadAxisCount(int gamepad)
{
    return CORE.Input.Gamepad.axisCount[gamepad];
}

// Get axis movement vector for a gamepad
float GetGamepadAxisMovement(int gamepad, int axis)
{
    float value = ((axis == GAMEPAD_AXIS_LEFT_TRIGGER) || (axis == GAMEPAD_AXIS_RIGHT_TRIGGER))? -1.0f : 0.0f;

    if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad] && (axis < MAX_GAMEPAD_AXES))
    {
        float movement = (value < 0.0f)? CORE.Input.Gamepad.axisState[gamepad][axis] : fabsf(CORE.Input.Gamepad.axisState[gamepad][axis]);

        if (movement > value) value = CORE.Input.Gamepad.axisState[gamepad][axis];
    }

    return value;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Input Handling: Mouse
//----------------------------------------------------------------------------------

// NOTE: Functions with a platform-specific implementation on rcore_<platform>.c
//void SetMousePosition(int x, int y)
//void SetMouseCursor(int cursor)

// Check if a mouse button has been pressed once
bool IsMouseButtonPressed(int button)
{
    bool pressed = false;

    if ((CORE.Input.Mouse.currentButtonState[button] == 1) && (CORE.Input.Mouse.previousButtonState[button] == 0)) pressed = true;

    // Map touches to mouse buttons checking
    if ((CORE.Input.Touch.currentTouchState[button] == 1) && (CORE.Input.Touch.previousTouchState[button] == 0)) pressed = true;

    return pressed;
}

// Check if a mouse button is being pressed
bool IsMouseButtonDown(int button)
{
    bool down = false;

    if (CORE.Input.Mouse.currentButtonState[button] == 1) down = true;

    // NOTE: Touches are considered like mouse buttons
    if (CORE.Input.Touch.currentTouchState[button] == 1) down = true;

    return down;
}

// Check if a mouse button has been released once
bool IsMouseButtonReleased(int button)
{
    bool released = false;

    if ((CORE.Input.Mouse.currentButtonState[button] == 0) && (CORE.Input.Mouse.previousButtonState[button] == 1)) released = true;

    // Map touches to mouse buttons checking
    if ((CORE.Input.Touch.currentTouchState[button] == 0) && (CORE.Input.Touch.previousTouchState[button] == 1)) released = true;

    return released;
}

// Check if a mouse button is NOT being pressed
bool IsMouseButtonUp(int button)
{
    bool up = false;

    if (CORE.Input.Mouse.currentButtonState[button] == 0) up = true;

    // NOTE: Touches are considered like mouse buttons
    if (CORE.Input.Touch.currentTouchState[button] == 0) up = true;

    return up;
}

// Get mouse position X
int GetMouseX(void)
{
    int mouseX = (int)((CORE.Input.Mouse.currentPosition.x + CORE.Input.Mouse.offset.x)*CORE.Input.Mouse.scale.x);
    return mouseX;
}

// Get mouse position Y
int GetMouseY(void)
{
    int mouseY = (int)((CORE.Input.Mouse.currentPosition.y + CORE.Input.Mouse.offset.y)*CORE.Input.Mouse.scale.y);
    return mouseY;
}

// Get mouse position XY
Vector2 GetMousePosition(void)
{
    Vector2 position = { 0 };

    position.x = (CORE.Input.Mouse.currentPosition.x + CORE.Input.Mouse.offset.x)*CORE.Input.Mouse.scale.x;
    position.y = (CORE.Input.Mouse.currentPosition.y + CORE.Input.Mouse.offset.y)*CORE.Input.Mouse.scale.y;

    return position;
}

// Get mouse delta between frames
Vector2 GetMouseDelta(void)
{
    Vector2 delta = { 0 };

    delta.x = CORE.Input.Mouse.currentPosition.x - CORE.Input.Mouse.previousPosition.x;
    delta.y = CORE.Input.Mouse.currentPosition.y - CORE.Input.Mouse.previousPosition.y;

    return delta;
}

// Set mouse offset
// NOTE: Useful when rendering to different size targets
void SetMouseOffset(int offsetX, int offsetY)
{
    CORE.Input.Mouse.offset = (Vector2){ (float)offsetX, (float)offsetY };
}

// Set mouse scaling
// NOTE: Useful when rendering to different size targets
void SetMouseScale(float scaleX, float scaleY)
{
    CORE.Input.Mouse.scale = (Vector2){ scaleX, scaleY };
}

// Get mouse wheel movement Y
float GetMouseWheelMove(void)
{
    float result = 0.0f;

    if (fabsf(CORE.Input.Mouse.currentWheelMove.x) > fabsf(CORE.Input.Mouse.currentWheelMove.y)) result = (float)CORE.Input.Mouse.currentWheelMove.x;
    else result = (float)CORE.Input.Mouse.currentWheelMove.y;

    return result;
}

// Get mouse wheel movement X/Y as a vector
Vector2 GetMouseWheelMoveV(void)
{
    Vector2 result = { 0 };

    result = CORE.Input.Mouse.currentWheelMove;

    return result;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Input Handling: Touch
//----------------------------------------------------------------------------------

// Get touch position X for touch point 0 (relative to screen size)
int GetTouchX(void)
{
    int touchX = (int)CORE.Input.Touch.position[0].x;
    return touchX;
}

// Get touch position Y for touch point 0 (relative to screen size)
int GetTouchY(void)
{
    int touchY = (int)CORE.Input.Touch.position[0].y;
    return touchY;
}

// Get touch position XY for a touch point index (relative to screen size)
Vector2 GetTouchPosition(int index)
{
    Vector2 position = { -1.0f, -1.0f };

    if (index < MAX_TOUCH_POINTS) position = CORE.Input.Touch.position[index];
    else TRACELOG(LOG_WARNING, "INPUT: Required touch point out of range (Max touch points: %i)", MAX_TOUCH_POINTS);

    return position;
}

// Get touch point identifier for given index
int GetTouchPointId(int index)
{
    int id = -1;

    if (index < MAX_TOUCH_POINTS) id = CORE.Input.Touch.pointId[index];

    return id;
}

// Get number of touch points
int GetTouchPointCount(void)
{
    return CORE.Input.Touch.pointCount;
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

// NOTE: Functions with a platform-specific implementation on rcore_<platform>.c
//int InitPlatform(void)
//void ClosePlatform(void)

// Initialize hi-resolution timer
void InitTimer(void)
{
    // Setting a higher resolution can improve the accuracy of time-out intervals in wait functions
    // However, it can also reduce overall system performance, because the thread scheduler switches tasks more often
    // High resolutions can also prevent the CPU power management system from entering power-saving modes
    // Setting a higher resolution does not improve the accuracy of the high-resolution performance counter
#if defined(_WIN32) && defined(SUPPORT_WINMM_HIGHRES_TIMER) && !defined(SUPPORT_BUSY_WAIT_LOOP) && !defined(PLATFORM_DESKTOP_SDL)
    timeBeginPeriod(1); // Setup high-resolution timer to 1ms (granularity of 1-2 ms)
#endif

#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__EMSCRIPTEN__)
    struct timespec now = { 0 };

    if (clock_gettime(CLOCK_MONOTONIC, &now) == 0) // Success
    {
        CORE.Time.base = (unsigned long long int)now.tv_sec*1000000000LLU + (unsigned long long int)now.tv_nsec;
    }
    else TRACELOG(LOG_WARNING, "TIMER: Hi-resolution timer not available");
#endif

    CORE.Time.previous = GetTime(); // Get time as double
}

// Set viewport for a provided width and height
void SetupViewport(int width, int height)
{
    CORE.Window.render.width = width;
    CORE.Window.render.height = height;

    // Set viewport width and height
    rlViewport(CORE.Window.renderOffset.x/2, CORE.Window.renderOffset.y/2, CORE.Window.render.width, CORE.Window.render.height);

    rlMatrixMode(RL_PROJECTION);        // Switch to projection matrix
    rlLoadIdentity();                   // Reset current matrix (projection)

    // Set orthographic projection to current framebuffer size
    // NOTE: Configured top-left corner as (0, 0)
    rlOrtho(0, CORE.Window.render.width, CORE.Window.render.height, 0, 0.0f, 1.0f);

    rlMatrixMode(RL_MODELVIEW);         // Switch back to modelview matrix
    rlLoadIdentity();                   // Reset current matrix (modelview)
}

// Scan all files and directories in a base path
// WARNING: files.paths[] must be previously allocated and
// contain enough space to store all required paths
static void ScanDirectoryFiles(const char *basePath, FilePathList *files, const char *filter)
{
    static char path[MAX_FILEPATH_LENGTH] = { 0 };
    memset(path, 0, MAX_FILEPATH_LENGTH);

    struct dirent *dp = NULL;
    DIR *dir = opendir(basePath);

    if (dir != NULL)
    {
        while ((dp = readdir(dir)) != NULL)
        {
            if ((strcmp(dp->d_name, ".") != 0) &&
                (strcmp(dp->d_name, "..") != 0))
            {
                // Construct new path from our base path
            #if defined(_WIN32)
                int pathLength = snprintf(path, MAX_FILEPATH_LENGTH - 1, "%s\\%s", basePath, dp->d_name);
            #else
                int pathLength = snprintf(path, MAX_FILEPATH_LENGTH - 1, "%s/%s", basePath, dp->d_name);
            #endif

                if ((pathLength < 0) || (pathLength >= MAX_FILEPATH_LENGTH))
                {
                    TRACELOG(LOG_WARNING, "FILEIO: Path longer than %d characters (%s...)", MAX_FILEPATH_LENGTH, basePath);
                }
                else if (filter != NULL)
                {
                    if (IsPathFile(path))
                    {
                        if (IsFileExtension(path, filter))
                        {
                            strncpy(files->paths[files->count], path, MAX_FILEPATH_LENGTH - 1);
                            files->count++;
                        }
                    }
                    else
                    {
                        if (strstr(filter, DIRECTORY_FILTER_TAG) != NULL)
                        {
                            strncpy(files->paths[files->count], path, MAX_FILEPATH_LENGTH - 1);
                            files->count++;
                        }
                    }
                }
                else
                {
                    strncpy(files->paths[files->count], path, MAX_FILEPATH_LENGTH - 1);
                    files->count++;
                }
            }
        }

        closedir(dir);
    }
    else TRACELOG(LOG_WARNING, "FILEIO: Directory cannot be opened (%s)", basePath);
}

// Scan all files and directories recursively from a base path
static void ScanDirectoryFilesRecursively(const char *basePath, FilePathList *files, const char *filter)
{
    // WARNING: Path can not be static or it will be reused between recursive function calls!
    char path[MAX_FILEPATH_LENGTH] = { 0 };
    memset(path, 0, MAX_FILEPATH_LENGTH);

    struct dirent *dp = NULL;
    DIR *dir = opendir(basePath);

    if (dir != NULL)
    {
        while (((dp = readdir(dir)) != NULL) && (files->count < files->capacity))
        {
            if ((strcmp(dp->d_name, ".") != 0) && (strcmp(dp->d_name, "..") != 0))
            {
                // Construct new path from our base path
            #if defined(_WIN32)
                int pathLength = snprintf(path, MAX_FILEPATH_LENGTH - 1, "%s\\%s", basePath, dp->d_name);
            #else
                int pathLength = snprintf(path, MAX_FILEPATH_LENGTH - 1, "%s/%s", basePath, dp->d_name);
            #endif

                if ((pathLength < 0) || (pathLength >= MAX_FILEPATH_LENGTH))
                {
                    TRACELOG(LOG_WARNING, "FILEIO: Path longer than %d characters (%s...)", MAX_FILEPATH_LENGTH, basePath);
                }
                else if (IsPathFile(path))
                {
                    if (filter != NULL)
                    {
                        if (IsFileExtension(path, filter))
                        {
                            strncpy(files->paths[files->count], path, MAX_FILEPATH_LENGTH - 1);
                            files->count++;
                        }
                    }
                    else
                    {
                        strncpy(files->paths[files->count], path, MAX_FILEPATH_LENGTH - 1);
                        files->count++;
                    }

                    if (files->count >= files->capacity)
                    {
                        TRACELOG(LOG_WARNING, "FILEIO: Maximum filepath scan capacity reached (%i files)", files->capacity);
                        break;
                    }
                }
                else
                {
                    if ((filter != NULL) && (strstr(filter, DIRECTORY_FILTER_TAG) != NULL))
                    {
                        strncpy(files->paths[files->count], path, MAX_FILEPATH_LENGTH - 1);
                        files->count++;
                    }

                    if (files->count >= files->capacity)
                    {
                        TRACELOG(LOG_WARNING, "FILEIO: Maximum filepath scan capacity reached (%i files)", files->capacity);
                        break;
                    }

                    ScanDirectoryFilesRecursively(path, files, filter);
                }
            }
        }

        closedir(dir);
    }
    else TRACELOG(LOG_WARNING, "FILEIO: Directory cannot be opened (%s)", basePath);
}

#if defined(SUPPORT_AUTOMATION_EVENTS)
// Automation event recording
// Checking events in current frame and save them into currentEventList
// NOTE: Recording is by default done at EndDrawing(), before PollInputEvents()
static void RecordAutomationEvent(void)
{
    if (currentEventList->count == currentEventList->capacity) return;

    // Keyboard input events recording
    //-------------------------------------------------------------------------------------
    for (int key = 0; key < MAX_KEYBOARD_KEYS; key++)
    {
        // Event type: INPUT_KEY_UP (only saved once)
        if (CORE.Input.Keyboard.previousKeyState[key] && !CORE.Input.Keyboard.currentKeyState[key])
        {
            currentEventList->events[currentEventList->count].frame = CORE.Time.frameCounter;
            currentEventList->events[currentEventList->count].type = INPUT_KEY_UP;
            currentEventList->events[currentEventList->count].params[0] = key;
            currentEventList->events[currentEventList->count].params[1] = 0;
            currentEventList->events[currentEventList->count].params[2] = 0;

            TRACELOG(LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_KEY_UP | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
            currentEventList->count++;
        }

        if (currentEventList->count == currentEventList->capacity) return;    // Security check

        // Event type: INPUT_KEY_DOWN
        if (CORE.Input.Keyboard.currentKeyState[key])
        {
            currentEventList->events[currentEventList->count].frame = CORE.Time.frameCounter;
            currentEventList->events[currentEventList->count].type = INPUT_KEY_DOWN;
            currentEventList->events[currentEventList->count].params[0] = key;
            currentEventList->events[currentEventList->count].params[1] = 0;
            currentEventList->events[currentEventList->count].params[2] = 0;

            TRACELOG(LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_KEY_DOWN | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
            currentEventList->count++;
        }

        if (currentEventList->count == currentEventList->capacity) return;    // Security check
    }
    //-------------------------------------------------------------------------------------

    // Mouse input currentEventList->events recording
    //-------------------------------------------------------------------------------------
    for (int button = 0; button < MAX_MOUSE_BUTTONS; button++)
    {
        // Event type: INPUT_MOUSE_BUTTON_UP
        if (CORE.Input.Mouse.previousButtonState[button] && !CORE.Input.Mouse.currentButtonState[button])
        {
            currentEventList->events[currentEventList->count].frame = CORE.Time.frameCounter;
            currentEventList->events[currentEventList->count].type = INPUT_MOUSE_BUTTON_UP;
            currentEventList->events[currentEventList->count].params[0] = button;
            currentEventList->events[currentEventList->count].params[1] = 0;
            currentEventList->events[currentEventList->count].params[2] = 0;

            TRACELOG(LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_MOUSE_BUTTON_UP | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
            currentEventList->count++;
        }

        if (currentEventList->count == currentEventList->capacity) return;    // Security check

        // Event type: INPUT_MOUSE_BUTTON_DOWN
        if (CORE.Input.Mouse.currentButtonState[button])
        {
            currentEventList->events[currentEventList->count].frame = CORE.Time.frameCounter;
            currentEventList->events[currentEventList->count].type = INPUT_MOUSE_BUTTON_DOWN;
            currentEventList->events[currentEventList->count].params[0] = button;
            currentEventList->events[currentEventList->count].params[1] = 0;
            currentEventList->events[currentEventList->count].params[2] = 0;

            TRACELOG(LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_MOUSE_BUTTON_DOWN | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
            currentEventList->count++;
        }

        if (currentEventList->count == currentEventList->capacity) return;    // Security check
    }

    // Event type: INPUT_MOUSE_POSITION (only saved if changed)
    if (((int)CORE.Input.Mouse.currentPosition.x != (int)CORE.Input.Mouse.previousPosition.x) ||
        ((int)CORE.Input.Mouse.currentPosition.y != (int)CORE.Input.Mouse.previousPosition.y))
    {
        currentEventList->events[currentEventList->count].frame = CORE.Time.frameCounter;
        currentEventList->events[currentEventList->count].type = INPUT_MOUSE_POSITION;
        currentEventList->events[currentEventList->count].params[0] = (int)CORE.Input.Mouse.currentPosition.x;
        currentEventList->events[currentEventList->count].params[1] = (int)CORE.Input.Mouse.currentPosition.y;
        currentEventList->events[currentEventList->count].params[2] = 0;

        TRACELOG(LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_MOUSE_POSITION | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
        currentEventList->count++;

        if (currentEventList->count == currentEventList->capacity) return;    // Security check
    }

    // Event type: INPUT_MOUSE_WHEEL_MOTION
    if (((int)CORE.Input.Mouse.currentWheelMove.x != (int)CORE.Input.Mouse.previousWheelMove.x) ||
        ((int)CORE.Input.Mouse.currentWheelMove.y != (int)CORE.Input.Mouse.previousWheelMove.y))
    {
        currentEventList->events[currentEventList->count].frame = CORE.Time.frameCounter;
        currentEventList->events[currentEventList->count].type = INPUT_MOUSE_WHEEL_MOTION;
        currentEventList->events[currentEventList->count].params[0] = (int)CORE.Input.Mouse.currentWheelMove.x;
        currentEventList->events[currentEventList->count].params[1] = (int)CORE.Input.Mouse.currentWheelMove.y;
        currentEventList->events[currentEventList->count].params[2] = 0;

        TRACELOG(LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_MOUSE_WHEEL_MOTION | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
        currentEventList->count++;

        if (currentEventList->count == currentEventList->capacity) return;    // Security check
    }
    //-------------------------------------------------------------------------------------

    // Touch input currentEventList->events recording
    //-------------------------------------------------------------------------------------
    for (int id = 0; id < MAX_TOUCH_POINTS; id++)
    {
        // Event type: INPUT_TOUCH_UP
        if (CORE.Input.Touch.previousTouchState[id] && !CORE.Input.Touch.currentTouchState[id])
        {
            currentEventList->events[currentEventList->count].frame = CORE.Time.frameCounter;
            currentEventList->events[currentEventList->count].type = INPUT_TOUCH_UP;
            currentEventList->events[currentEventList->count].params[0] = id;
            currentEventList->events[currentEventList->count].params[1] = 0;
            currentEventList->events[currentEventList->count].params[2] = 0;

            TRACELOG(LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_TOUCH_UP | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
            currentEventList->count++;
        }

        if (currentEventList->count == currentEventList->capacity) return;    // Security check

        // Event type: INPUT_TOUCH_DOWN
        if (CORE.Input.Touch.currentTouchState[id])
        {
            currentEventList->events[currentEventList->count].frame = CORE.Time.frameCounter;
            currentEventList->events[currentEventList->count].type = INPUT_TOUCH_DOWN;
            currentEventList->events[currentEventList->count].params[0] = id;
            currentEventList->events[currentEventList->count].params[1] = 0;
            currentEventList->events[currentEventList->count].params[2] = 0;

            TRACELOG(LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_TOUCH_DOWN | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
            currentEventList->count++;
        }

        if (currentEventList->count == currentEventList->capacity) return;    // Security check

        // Event type: INPUT_TOUCH_POSITION         
        if (((int)CORE.Input.Touch.position[id].x != (int)CORE.Input.Touch.previousPosition[id].x) ||
            ((int)CORE.Input.Touch.position[id].y != (int)CORE.Input.Touch.previousPosition[id].y))
        {
            currentEventList->events[currentEventList->count].frame = CORE.Time.frameCounter;
            currentEventList->events[currentEventList->count].type = INPUT_TOUCH_POSITION;
            currentEventList->events[currentEventList->count].params[0] = id;
            currentEventList->events[currentEventList->count].params[1] = (int)CORE.Input.Touch.position[id].x;
            currentEventList->events[currentEventList->count].params[2] = (int)CORE.Input.Touch.position[id].y;

            TRACELOG(LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_TOUCH_POSITION | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
            currentEventList->count++;
        }
        

        if (currentEventList->count == currentEventList->capacity) return;    // Security check
    }
    //-------------------------------------------------------------------------------------

    // Gamepad input currentEventList->events recording
    //-------------------------------------------------------------------------------------
    for (int gamepad = 0; gamepad < MAX_GAMEPADS; gamepad++)
    {
        // Event type: INPUT_GAMEPAD_CONNECT
        /*
        if ((CORE.Input.Gamepad.currentState[gamepad] != CORE.Input.Gamepad.previousState[gamepad]) &&
            (CORE.Input.Gamepad.currentState[gamepad])) // Check if changed to ready
        {
            // TODO: Save gamepad connect event
        }
        */

        // Event type: INPUT_GAMEPAD_DISCONNECT
        /*
        if ((CORE.Input.Gamepad.currentState[gamepad] != CORE.Input.Gamepad.previousState[gamepad]) &&
            (!CORE.Input.Gamepad.currentState[gamepad])) // Check if changed to not-ready
        {
            // TODO: Save gamepad disconnect event
        }
        */

        for (int button = 0; button < MAX_GAMEPAD_BUTTONS; button++)
        {
            // Event type: INPUT_GAMEPAD_BUTTON_UP
            if (CORE.Input.Gamepad.previousButtonState[gamepad][button] && !CORE.Input.Gamepad.currentButtonState[gamepad][button])
            {
                currentEventList->events[currentEventList->count].frame = CORE.Time.frameCounter;
                currentEventList->events[currentEventList->count].type = INPUT_GAMEPAD_BUTTON_UP;
                currentEventList->events[currentEventList->count].params[0] = gamepad;
                currentEventList->events[currentEventList->count].params[1] = button;
                currentEventList->events[currentEventList->count].params[2] = 0;

                TRACELOG(LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_GAMEPAD_BUTTON_UP | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
                currentEventList->count++;
            }

            if (currentEventList->count == currentEventList->capacity) return;    // Security check

            // Event type: INPUT_GAMEPAD_BUTTON_DOWN
            if (CORE.Input.Gamepad.currentButtonState[gamepad][button])
            {
                currentEventList->events[currentEventList->count].frame = CORE.Time.frameCounter;
                currentEventList->events[currentEventList->count].type = INPUT_GAMEPAD_BUTTON_DOWN;
                currentEventList->events[currentEventList->count].params[0] = gamepad;
                currentEventList->events[currentEventList->count].params[1] = button;
                currentEventList->events[currentEventList->count].params[2] = 0;

                TRACELOG(LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_GAMEPAD_BUTTON_DOWN | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
                currentEventList->count++;
            }

            if (currentEventList->count == currentEventList->capacity) return;    // Security check
        }

        for (int axis = 0; axis < MAX_GAMEPAD_AXES; axis++)
        {
            // Event type: INPUT_GAMEPAD_AXIS_MOTION
            float defaultMovement = ((axis == GAMEPAD_AXIS_LEFT_TRIGGER) || (axis == GAMEPAD_AXIS_RIGHT_TRIGGER))? -1.0f : 0.0f;
            if (GetGamepadAxisMovement(gamepad, axis) != defaultMovement)
            {
                currentEventList->events[currentEventList->count].frame = CORE.Time.frameCounter;
                currentEventList->events[currentEventList->count].type = INPUT_GAMEPAD_AXIS_MOTION;
                currentEventList->events[currentEventList->count].params[0] = gamepad;
                currentEventList->events[currentEventList->count].params[1] = axis;
                currentEventList->events[currentEventList->count].params[2] = (int)(CORE.Input.Gamepad.axisState[gamepad][axis]*32768.0f);

                TRACELOG(LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_GAMEPAD_AXIS_MOTION | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
                currentEventList->count++;
            }

            if (currentEventList->count == currentEventList->capacity) return;    // Security check
        }
    }
    //-------------------------------------------------------------------------------------

#if defined(SUPPORT_GESTURES_SYSTEM)
    // Gestures input currentEventList->events recording
    //-------------------------------------------------------------------------------------
    if (GESTURES.current != GESTURE_NONE)
    {
        // Event type: INPUT_GESTURE
        currentEventList->events[currentEventList->count].frame = CORE.Time.frameCounter;
        currentEventList->events[currentEventList->count].type = INPUT_GESTURE;
        currentEventList->events[currentEventList->count].params[0] = GESTURES.current;
        currentEventList->events[currentEventList->count].params[1] = 0;
        currentEventList->events[currentEventList->count].params[2] = 0;

        TRACELOG(LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_GESTURE | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
        currentEventList->count++;

        if (currentEventList->count == currentEventList->capacity) return;    // Security check
    }
    //-------------------------------------------------------------------------------------
#endif
}
#endif

#if !defined(SUPPORT_MODULE_RTEXT)
// Formatting of text with variables to 'embed'
// WARNING: String returned will expire after this function is called MAX_TEXTFORMAT_BUFFERS times
const char *TextFormat(const char *text, ...)
{
#ifndef MAX_TEXTFORMAT_BUFFERS
    #define MAX_TEXTFORMAT_BUFFERS      4        // Maximum number of static buffers for text formatting
#endif
#ifndef MAX_TEXT_BUFFER_LENGTH
    #define MAX_TEXT_BUFFER_LENGTH   1024        // Maximum size of static text buffer
#endif

    // We create an array of buffers so strings don't expire until MAX_TEXTFORMAT_BUFFERS invocations
    static char buffers[MAX_TEXTFORMAT_BUFFERS][MAX_TEXT_BUFFER_LENGTH] = { 0 };
    static int index = 0;

    char *currentBuffer = buffers[index];
    memset(currentBuffer, 0, MAX_TEXT_BUFFER_LENGTH);   // Clear buffer before using

    if (text != NULL)
    {
        va_list args;
        va_start(args, text);
        int requiredByteCount = vsnprintf(currentBuffer, MAX_TEXT_BUFFER_LENGTH, text, args);
        va_end(args);

        // If requiredByteCount is larger than the MAX_TEXT_BUFFER_LENGTH, then overflow occurred
        if (requiredByteCount >= MAX_TEXT_BUFFER_LENGTH)
        {
            // Inserting "..." at the end of the string to mark as truncated
            char *truncBuffer = buffers[index] + MAX_TEXT_BUFFER_LENGTH - 4; // Adding 4 bytes = "...\0"
            snprintf(truncBuffer, 4, "...");
        }

        index += 1; // Move to next buffer for next function call
        if (index >= MAX_TEXTFORMAT_BUFFERS) index = 0;
    }

    return currentBuffer;
}

#endif // !SUPPORT_MODULE_RTEXT
