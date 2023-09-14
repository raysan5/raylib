/**********************************************************************************************
*
*   rcore - Basic functions to manage windows, OpenGL context and input on multiple platforms
*
*   PLATFORMS SUPPORTED:
*       - PLATFORM_DESKTOP: Windows (Win32, Win64)
*       - PLATFORM_DESKTOP: Linux (X11 desktop mode)
*       - PLATFORM_DESKTOP: FreeBSD, OpenBSD, NetBSD, DragonFly (X11 desktop)
*       - PLATFORM_DESKTOP: OSX/macOS
*       - PLATFORM_ANDROID: Android (ARM, ARM64)
*       - PLATFORM_DRM:     Linux native mode, including Raspberry Pi 4 with V3D fkms driver
*       - PLATFORM_WEB:     HTML5 with WebAssembly
*
*   CONFIGURATION:
*       #define PLATFORM_DESKTOP
*           Windowing and input system configured for desktop platforms:
*               Windows, Linux, OSX, FreeBSD, OpenBSD, NetBSD, DragonFly
*
*       #define PLATFORM_ANDROID
*           Windowing and input system configured for Android device, app activity managed internally in this module.
*           NOTE: OpenGL ES 2.0 is required and graphic device is managed by EGL
*
*       #define PLATFORM_DRM
*           Windowing and input system configured for DRM native mode (RPI4 and other devices)
*           graphic device is managed by EGL and inputs are processed is raw mode, reading from /dev/input/
*
*       #define PLATFORM_WEB
*           Windowing and input system configured for HTML5 (run on browser), code converted from C to asm.js
*           using emscripten compiler. OpenGL ES 2.0 required for direct translation to WebGL equivalent code.
*
*       #define SUPPORT_DEFAULT_FONT (default)
*           Default font is loaded on window initialization to be available for the user to render simple text.
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
*           Mouse gestures are directly mapped like touches and processed by gestures system.
*
*       #define SUPPORT_SSH_KEYBOARD_RPI (Raspberry Pi only)
*           Reconfigure standard input to receive key inputs, works with SSH connection.
*           WARNING: Reconfiguring standard input could lead to undesired effects, like breaking other
*           running processes orblocking the device if not restored properly. Use with care.
*
*       #define SUPPORT_BUSY_WAIT_LOOP
*           Use busy wait loop for timing sync, if not defined, a high-resolution timer is setup and used
*
*       #define SUPPORT_PARTIALBUSY_WAIT_LOOP
*           Use a partial-busy wait loop, in this case frame sleeps for most of the time and runs a busy-wait-loop at the end
*
*       #define SUPPORT_EVENTS_WAITING
*           Wait for events passively (sleeping while no events) instead of polling them actively every frame
*
*       #define SUPPORT_SCREEN_CAPTURE
*           Allow automatic screen capture of current screen pressing F12, defined in KeyCallback()
*
*       #define SUPPORT_GIF_RECORDING
*           Allow automatic gif recording of current screen pressing CTRL+F12, defined in KeyCallback()
*
*       #define SUPPORT_COMPRESSION_API
*           Support CompressData() and DecompressData() functions, those functions use zlib implementation
*           provided by stb_image and stb_image_write libraries, so, those libraries must be enabled on textures module
*           for linkage
*
*       #define SUPPORT_EVENTS_AUTOMATION
*           Support automatic generated events, loading and recording of those events when required
*
*   DEPENDENCIES:
*       rglfw    - Manage graphic device, OpenGL context and inputs on PLATFORM_DESKTOP (Windows, Linux, OSX, FreeBSD...)
*       raymath  - 3D math functionality (Vector2, Vector3, Matrix, Quaternion)
*       camera   - Multiple 3D camera modes (free, orbital, 1st person, 3rd person)
*       gestures - Gestures system for touch-ready devices (or simulated from mouse inputs)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2023 Ramon Santamaria (@raysan5)
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

#include "raylib.h"                 // Declares module functions
#include "rcore.h"

// Check if config flags have been externally provided on compilation line
#if !defined(EXTERNAL_CONFIG_FLAGS)
    #include "config.h"             // Defines module configuration flags
#endif

#define RLGL_IMPLEMENTATION
#include "rlgl.h"                   // OpenGL abstraction layer to OpenGL 1.1, 3.3+ or ES2

#define RAYMATH_IMPLEMENTATION      // Define external out-of-line implementation
#include "raymath.h"                // Vector3, Quaternion and Matrix functionality

#if defined(SUPPORT_GESTURES_SYSTEM)
    #define RGESTURES_IMPLEMENTATION
    #include "rgestures.h"           // Gestures detection functionality
#endif

#if defined(SUPPORT_CAMERA_SYSTEM)
    #define RCAMERA_IMPLEMENTATION
    #include "rcamera.h"             // Camera system functionality
#endif

#if defined(SUPPORT_GIF_RECORDING)
    #define MSF_GIF_MALLOC(contextPointer, newSize) RL_MALLOC(newSize)
    #define MSF_GIF_REALLOC(contextPointer, oldMemory, oldSize, newSize) RL_REALLOC(oldMemory, newSize)
    #define MSF_GIF_FREE(contextPointer, oldMemory, oldSize) RL_FREE(oldMemory)

    #define MSF_GIF_IMPL
    #include "external/msf_gif.h"   // GIF recording functionality
#endif

#if defined(SUPPORT_COMPRESSION_API)
    #define SINFL_IMPLEMENTATION
    #define SINFL_NO_SIMD
    #include "external/sinfl.h"     // Deflate (RFC 1951) decompressor

    #define SDEFL_IMPLEMENTATION
    #include "external/sdefl.h"     // Deflate (RFC 1951) compressor
#endif

#if (defined(__linux__) || defined(PLATFORM_WEB)) && (_POSIX_C_SOURCE < 199309L)
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 199309L // Required for: CLOCK_MONOTONIC if compiled with c99 without gnu ext.
#endif
#if defined(__linux__) && !defined(_GNU_SOURCE)
    #define _GNU_SOURCE
#endif

// Platform specific defines to handle GetApplicationDirectory()
#if defined (PLATFORM_DESKTOP)
    #if defined(_WIN32)
        #ifndef MAX_PATH
            #define MAX_PATH 1025
        #endif
    __declspec(dllimport) unsigned long __stdcall GetModuleFileNameA(void *hModule, void *lpFilename, unsigned long nSize);
    __declspec(dllimport) unsigned long __stdcall GetModuleFileNameW(void *hModule, void *lpFilename, unsigned long nSize);
    __declspec(dllimport) int __stdcall WideCharToMultiByte(unsigned int cp, unsigned long flags, void *widestr, int cchwide, void *str, int cbmb, void *defchar, int *used_default);
    #elif defined(__linux__)
        #include <unistd.h>
    #elif defined(__APPLE__)
        #include <sys/syslimits.h>
        #include <mach-o/dyld.h>
    #endif // OSs
#endif // PLATFORM_DESKTOP

#define _CRT_INTERNAL_NONSTDC_NAMES  1
#include <sys/stat.h>               // Required for: stat(), S_ISREG [Used in GetFileModTime(), IsFilePath()]

#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
    #define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

#if defined(PLATFORM_DESKTOP) && defined(_WIN32) && (defined(_MSC_VER) || defined(__TINYC__))
    #define DIRENT_MALLOC RL_MALLOC
    #define DIRENT_FREE RL_FREE

    #include "external/dirent.h"    // Required for: DIR, opendir(), closedir() [Used in LoadDirectoryFiles()]
#else
    #include <dirent.h>             // Required for: DIR, opendir(), closedir() [Used in LoadDirectoryFiles()]
#endif

#if defined(_WIN32)
    #include <direct.h>             // Required for: _getch(), _chdir()
    #define GETCWD _getcwd          // NOTE: MSDN recommends not to use getcwd(), chdir()
    #define CHDIR _chdir
    #include <io.h>                 // Required for: _access() [Used in FileExists()]
#else
    #include <unistd.h>             // Required for: getch(), chdir() (POSIX), access()
    #define GETCWD getcwd
    #define CHDIR chdir
#endif

#if defined(PLATFORM_DESKTOP)
    #define GLFW_INCLUDE_NONE       // Disable the standard OpenGL header inclusion on GLFW3
                                    // NOTE: Already provided by rlgl implementation (on glad.h)
    #include "GLFW/glfw3.h"         // GLFW3 library: Windows, OpenGL context and Input management
                                    // NOTE: GLFW3 already includes gl.h (OpenGL) headers

    // Support retrieving native window handlers
    #if defined(_WIN32)
        typedef void *PVOID;
        typedef PVOID HANDLE;
        typedef HANDLE HWND;
        #define GLFW_EXPOSE_NATIVE_WIN32
        #define GLFW_NATIVE_INCLUDE_NONE // To avoid some symbols re-definition in windows.h
        #include "GLFW/glfw3native.h"

        #if defined(SUPPORT_WINMM_HIGHRES_TIMER) && !defined(SUPPORT_BUSY_WAIT_LOOP)
            // NOTE: Those functions require linking with winmm library
            unsigned int __stdcall timeBeginPeriod(unsigned int uPeriod);
            unsigned int __stdcall timeEndPeriod(unsigned int uPeriod);
        #endif
    #endif
    #if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
        #include <sys/time.h>               // Required for: timespec, nanosleep(), select() - POSIX

        //#define GLFW_EXPOSE_NATIVE_X11      // WARNING: Exposing Xlib.h > X.h results in dup symbols for Font type
        //#define GLFW_EXPOSE_NATIVE_WAYLAND
        //#define GLFW_EXPOSE_NATIVE_MIR
        #include "GLFW/glfw3native.h"       // Required for: glfwGetX11Window()
    #endif
    #if defined(__APPLE__)
        #include <unistd.h>                 // Required for: usleep()

        //#define GLFW_EXPOSE_NATIVE_COCOA    // WARNING: Fails due to type redefinition
        void *glfwGetCocoaWindow(GLFWwindow* handle);
        #include "GLFW/glfw3native.h"       // Required for: glfwGetCocoaWindow()
    #endif

    // TODO: HACK: Added flag if not provided by GLFW when using external library
    // Latest GLFW release (GLFW 3.3.8) does not implement this flag, it was added for 3.4.0-dev
    #if !defined(GLFW_MOUSE_PASSTHROUGH)
        #define GLFW_MOUSE_PASSTHROUGH      0x0002000D
    #endif
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
RLAPI const char *raylib_version = RAYLIB_VERSION;  // raylib version exported symbol, required for some bindings

CoreData CORE = { 0 };               // Global CORE state context

#if defined(SUPPORT_SCREEN_CAPTURE)
static int screenshotCounter = 0;           // Screenshots counter
#endif

#if defined(SUPPORT_GIF_RECORDING)
static int gifFrameCounter = 0;             // GIF frames counter
static bool gifRecording = false;           // GIF recording state
static MsfGifState gifState = { 0 };        // MSGIF context state
#endif

#if defined(SUPPORT_EVENTS_AUTOMATION)
#define MAX_CODE_AUTOMATION_EVENTS      16384

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
    ACTION_TAKE_SCREENSHOT,
    ACTION_SETTARGETFPS
} AutomationEventType;

// Event type
// Used to enable events flags
typedef enum {
    EVENT_INPUT_KEYBOARD    = 0,
    EVENT_INPUT_MOUSE       = 1,
    EVENT_INPUT_GAMEPAD     = 2,
    EVENT_INPUT_TOUCH       = 4,
    EVENT_INPUT_GESTURE     = 8,
    EVENT_WINDOW            = 16,
    EVENT_CUSTOM            = 32
} EventType;

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

// Automation event (24 bytes)
typedef struct AutomationEvent {
    unsigned int frame;                 // Event frame
    unsigned int type;                  // Event type (AutomationEventType)
    int params[4];                      // Event parameters (if required)
} AutomationEvent;

static AutomationEvent *events = NULL;  // Events array
static unsigned int eventCount = 0;     // Events count
static bool eventsPlaying = false;      // Play events
static bool eventsRecording = false;    // Record events

//static short eventsEnabled = 0b0000001111111111;    // Events enabled for checking
#endif
//-----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Other Modules Functions Declaration (required by core)
//----------------------------------------------------------------------------------
#if defined(SUPPORT_MODULE_RTEXT) && defined(SUPPORT_DEFAULT_FONT)
extern void LoadFontDefault(void);          // [Module: text] Loads default font on InitWindow()
extern void UnloadFontDefault(void);        // [Module: text] Unloads default font from GPU memory
#endif

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void InitTimer(void);                            // Initialize timer (hi-resolution if available)
static void SetupFramebuffer(int width, int height);    // Setup main framebuffer
static void SetupViewport(int width, int height);       // Set viewport for a provided width and height

static void ScanDirectoryFiles(const char *basePath, FilePathList *list, const char *filter);   // Scan all files and directories in a base path
static void ScanDirectoryFilesRecursively(const char *basePath, FilePathList *list, const char *filter);  // Scan all files and directories recursively from a base path

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
static void ErrorCallback(int error, const char *description);                             // GLFW3 Error Callback, runs on GLFW3 error
// Window callbacks events
static void WindowSizeCallback(GLFWwindow *window, int width, int height);                 // GLFW3 WindowSize Callback, runs when window is resized
#if !defined(PLATFORM_WEB)
static void WindowMaximizeCallback(GLFWwindow* window, int maximized);                     // GLFW3 Window Maximize Callback, runs when window is maximized
#endif
static void WindowIconifyCallback(GLFWwindow *window, int iconified);                      // GLFW3 WindowIconify Callback, runs when window is minimized/restored
static void WindowFocusCallback(GLFWwindow *window, int focused);                          // GLFW3 WindowFocus Callback, runs when window get/lose focus
static void WindowDropCallback(GLFWwindow *window, int count, const char **paths);         // GLFW3 Window Drop Callback, runs when drop files into window
// Input callbacks events
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);  // GLFW3 Keyboard Callback, runs on key pressed
static void CharCallback(GLFWwindow *window, unsigned int key);                            // GLFW3 Char Key Callback, runs on key pressed (get char value)
static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);     // GLFW3 Mouse Button Callback, runs on mouse button pressed
static void MouseCursorPosCallback(GLFWwindow *window, double x, double y);                // GLFW3 Cursor Position Callback, runs on mouse move
static void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);       // GLFW3 Srolling Callback, runs on mouse wheel
static void CursorEnterCallback(GLFWwindow *window, int enter);                            // GLFW3 Cursor Enter Callback, cursor enters client area
#endif

#if defined(PLATFORM_DRM)
static void InitKeyboard(void);                         // Initialize raw keyboard system
static void RestoreKeyboard(void);                      // Restore keyboard system
#if defined(SUPPORT_SSH_KEYBOARD_RPI)
static void ProcessKeyboard(void);                      // Process keyboard events
#endif

static void InitEvdevInput(void);                       // Initialize evdev inputs
static void ConfigureEvdevDevice(char *device);         // Identifies a input device and configures it for use if appropriate
static void PollKeyboardEvents(void);                   // Process evdev keyboard events.
static void *EventThread(void *arg);                    // Input device events reading thread

static void InitGamepad(void);                          // Initialize raw gamepad input
static void *GamepadThread(void *arg);                  // Mouse reading thread

static int FindMatchingConnectorMode(const drmModeConnector *connector, const drmModeModeInfo *mode);                               // Search matching DRM mode in connector's mode list
static int FindExactConnectorMode(const drmModeConnector *connector, uint width, uint height, uint fps, bool allowInterlaced);      // Search exactly matching DRM connector mode in connector's list
static int FindNearestConnectorMode(const drmModeConnector *connector, uint width, uint height, uint fps, bool allowInterlaced);    // Search the nearest matching DRM connector mode in connector's list

#endif  // PLATFORM_DRM

#if defined(SUPPORT_EVENTS_AUTOMATION)
static void LoadAutomationEvents(const char *fileName);     // Load automation events from file
static void ExportAutomationEvents(const char *fileName);   // Export recorded automation events into a file
static void RecordAutomationEvent(unsigned int frame);      // Record frame events (to internal events array)
static void PlayAutomationEvent(unsigned int frame);        // Play frame events (from internal events array)
#endif

#if defined(_WIN32)
// NOTE: We declare Sleep() function symbol to avoid including windows.h (kernel32.lib linkage required)
void __stdcall Sleep(unsigned long msTimeout);              // Required for: WaitTime()
#endif

#if !defined(SUPPORT_MODULE_RTEXT)
const char *TextFormat(const char *text, ...);       // Formatting of text with variables to 'embed'
#endif // !SUPPORT_MODULE_RTEXT

//----------------------------------------------------------------------------------
// Module Functions Definition - Window and OpenGL Context Functions
//----------------------------------------------------------------------------------

// Check if window has been initialized successfully
bool IsWindowReady(void)
{
    return CORE.Window.ready;
}

// Check if window is currently fullscreen
bool IsWindowFullscreen(void)
{
    return CORE.Window.fullscreen;
}

// Check if one specific window flag is enabled
bool IsWindowState(unsigned int flag)
{
    return ((CORE.Window.flags & flag) > 0);
}

// Toggle borderless windowed mode (only PLATFORM_DESKTOP)
void ToggleBorderlessWindowed(void)
{
#if defined(PLATFORM_DESKTOP)
    // Leave fullscreen before attempting to set borderless windowed mode and get screen position from it
    bool wasOnFullscreen = false;
    if (CORE.Window.fullscreen)
    {
        CORE.Window.previousPosition = CORE.Window.position;
        ToggleFullscreen();
        wasOnFullscreen = true;
    }

    const int monitor = GetCurrentMonitor();
    int monitorCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);
    if ((monitor >= 0) && (monitor < monitorCount))
    {
        const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);
        if (mode)
        {
            if (!IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE))
            {
                // Store screen position and size
                // NOTE: If it was on fullscreen, screen position was already stored, so skip setting it here
                if (!wasOnFullscreen) glfwGetWindowPos(CORE.Window.handle, &CORE.Window.previousPosition.x, &CORE.Window.previousPosition.y);
                CORE.Window.previousScreen = CORE.Window.screen;

                // Set undecorated and topmost modes and flags
                glfwSetWindowAttrib(CORE.Window.handle, GLFW_DECORATED, GLFW_FALSE);
                CORE.Window.flags |= FLAG_WINDOW_UNDECORATED;
                glfwSetWindowAttrib(CORE.Window.handle, GLFW_FLOATING, GLFW_TRUE);
                CORE.Window.flags |= FLAG_WINDOW_TOPMOST;

                // Get monitor position and size
                int monitorPosX = 0;
                int monitorPosY = 0;
                glfwGetMonitorPos(monitors[monitor], &monitorPosX, &monitorPosY);
                const int monitorWidth = mode->width;
                const int monitorHeight = mode->height;
                glfwSetWindowSize(CORE.Window.handle, monitorWidth, monitorHeight);

                // Set screen position and size
                glfwSetWindowPos(CORE.Window.handle, monitorPosX, monitorPosY);
                glfwSetWindowSize(CORE.Window.handle, monitorWidth, monitorHeight);

                // Refocus window
                glfwFocusWindow(CORE.Window.handle);

                CORE.Window.flags |= FLAG_BORDERLESS_WINDOWED_MODE;
            }
            else
            {
                // Remove topmost and undecorated modes and flags
                glfwSetWindowAttrib(CORE.Window.handle, GLFW_FLOATING, GLFW_FALSE);
                CORE.Window.flags &= ~FLAG_WINDOW_TOPMOST;
                glfwSetWindowAttrib(CORE.Window.handle, GLFW_DECORATED, GLFW_TRUE);
                CORE.Window.flags &= ~FLAG_WINDOW_UNDECORATED;

                // Return previous screen size and position
                // NOTE: The order matters here, it must set size first, then set position, otherwise the screen will be positioned incorrectly
                glfwSetWindowSize(CORE.Window.handle,  CORE.Window.previousScreen.width, CORE.Window.previousScreen.height);
                glfwSetWindowPos(CORE.Window.handle, CORE.Window.previousPosition.x, CORE.Window.previousPosition.y);

                // Refocus window
                glfwFocusWindow(CORE.Window.handle);

                CORE.Window.flags &= ~FLAG_BORDERLESS_WINDOWED_MODE;
            }
        }
        else TRACELOG(LOG_WARNING, "GLFW: Failed to find video mode for selected monitor");
    }
    else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
#endif
}

// Set window configuration state using flags
void SetWindowState(unsigned int flags)
{
#if defined(PLATFORM_DESKTOP)
    // Check previous state and requested state to apply required changes
    // NOTE: In most cases the functions already change the flags internally

    // State change: FLAG_VSYNC_HINT
    if (((CORE.Window.flags & FLAG_VSYNC_HINT) != (flags & FLAG_VSYNC_HINT)) && ((flags & FLAG_VSYNC_HINT) > 0))
    {
        glfwSwapInterval(1);
        CORE.Window.flags |= FLAG_VSYNC_HINT;
    }

    // State change: FLAG_BORDERLESS_WINDOWED_MODE
    // NOTE: This must be handled before FLAG_FULLSCREEN_MODE because ToggleBorderlessWindowed() needs to get some fullscreen values if fullscreen is running
    if (((CORE.Window.flags & FLAG_BORDERLESS_WINDOWED_MODE) != (flags & FLAG_BORDERLESS_WINDOWED_MODE)) && ((flags & FLAG_BORDERLESS_WINDOWED_MODE) > 0))
    {
        ToggleBorderlessWindowed();     // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_FULLSCREEN_MODE
    if ((CORE.Window.flags & FLAG_FULLSCREEN_MODE) != (flags & FLAG_FULLSCREEN_MODE))
    {
        ToggleFullscreen();     // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_RESIZABLE
    if (((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) != (flags & FLAG_WINDOW_RESIZABLE)) && ((flags & FLAG_WINDOW_RESIZABLE) > 0))
    {
        glfwSetWindowAttrib(CORE.Window.handle, GLFW_RESIZABLE, GLFW_TRUE);
        CORE.Window.flags |= FLAG_WINDOW_RESIZABLE;
    }

    // State change: FLAG_WINDOW_UNDECORATED
    if (((CORE.Window.flags & FLAG_WINDOW_UNDECORATED) != (flags & FLAG_WINDOW_UNDECORATED)) && (flags & FLAG_WINDOW_UNDECORATED))
    {
        glfwSetWindowAttrib(CORE.Window.handle, GLFW_DECORATED, GLFW_FALSE);
        CORE.Window.flags |= FLAG_WINDOW_UNDECORATED;
    }

    // State change: FLAG_WINDOW_HIDDEN
    if (((CORE.Window.flags & FLAG_WINDOW_HIDDEN) != (flags & FLAG_WINDOW_HIDDEN)) && ((flags & FLAG_WINDOW_HIDDEN) > 0))
    {
        glfwHideWindow(CORE.Window.handle);
        CORE.Window.flags |= FLAG_WINDOW_HIDDEN;
    }

    // State change: FLAG_WINDOW_MINIMIZED
    if (((CORE.Window.flags & FLAG_WINDOW_MINIMIZED) != (flags & FLAG_WINDOW_MINIMIZED)) && ((flags & FLAG_WINDOW_MINIMIZED) > 0))
    {
        //GLFW_ICONIFIED
        MinimizeWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_MAXIMIZED
    if (((CORE.Window.flags & FLAG_WINDOW_MAXIMIZED) != (flags & FLAG_WINDOW_MAXIMIZED)) && ((flags & FLAG_WINDOW_MAXIMIZED) > 0))
    {
        //GLFW_MAXIMIZED
        MaximizeWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_UNFOCUSED
    if (((CORE.Window.flags & FLAG_WINDOW_UNFOCUSED) != (flags & FLAG_WINDOW_UNFOCUSED)) && ((flags & FLAG_WINDOW_UNFOCUSED) > 0))
    {
        glfwSetWindowAttrib(CORE.Window.handle, GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
        CORE.Window.flags |= FLAG_WINDOW_UNFOCUSED;
    }

    // State change: FLAG_WINDOW_TOPMOST
    if (((CORE.Window.flags & FLAG_WINDOW_TOPMOST) != (flags & FLAG_WINDOW_TOPMOST)) && ((flags & FLAG_WINDOW_TOPMOST) > 0))
    {
        glfwSetWindowAttrib(CORE.Window.handle, GLFW_FLOATING, GLFW_TRUE);
        CORE.Window.flags |= FLAG_WINDOW_TOPMOST;
    }

    // State change: FLAG_WINDOW_ALWAYS_RUN
    if (((CORE.Window.flags & FLAG_WINDOW_ALWAYS_RUN) != (flags & FLAG_WINDOW_ALWAYS_RUN)) && ((flags & FLAG_WINDOW_ALWAYS_RUN) > 0))
    {
        CORE.Window.flags |= FLAG_WINDOW_ALWAYS_RUN;
    }

    // The following states can not be changed after window creation

    // State change: FLAG_WINDOW_TRANSPARENT
    if (((CORE.Window.flags & FLAG_WINDOW_TRANSPARENT) != (flags & FLAG_WINDOW_TRANSPARENT)) && ((flags & FLAG_WINDOW_TRANSPARENT) > 0))
    {
        TRACELOG(LOG_WARNING, "WINDOW: Framebuffer transparency can only be configured before window initialization");
    }

    // State change: FLAG_WINDOW_HIGHDPI
    if (((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) != (flags & FLAG_WINDOW_HIGHDPI)) && ((flags & FLAG_WINDOW_HIGHDPI) > 0))
    {
        TRACELOG(LOG_WARNING, "WINDOW: High DPI can only be configured before window initialization");
    }

    // State change: FLAG_WINDOW_MOUSE_PASSTHROUGH
    if (((CORE.Window.flags & FLAG_WINDOW_MOUSE_PASSTHROUGH) != (flags & FLAG_WINDOW_MOUSE_PASSTHROUGH)) && ((flags & FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0))
    {
        glfwSetWindowAttrib(CORE.Window.handle, GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
        CORE.Window.flags |= FLAG_WINDOW_MOUSE_PASSTHROUGH;
    }

    // State change: FLAG_MSAA_4X_HINT
    if (((CORE.Window.flags & FLAG_MSAA_4X_HINT) != (flags & FLAG_MSAA_4X_HINT)) && ((flags & FLAG_MSAA_4X_HINT) > 0))
    {
        TRACELOG(LOG_WARNING, "WINDOW: MSAA can only be configured before window initialization");
    }

    // State change: FLAG_INTERLACED_HINT
    if (((CORE.Window.flags & FLAG_INTERLACED_HINT) != (flags & FLAG_INTERLACED_HINT)) && ((flags & FLAG_INTERLACED_HINT) > 0))
    {
        TRACELOG(LOG_WARNING, "RPI: Interlaced mode can only be configured before window initialization");
    }
#endif
}

// Clear window configuration state flags
void ClearWindowState(unsigned int flags)
{
#if defined(PLATFORM_DESKTOP)
    // Check previous state and requested state to apply required changes
    // NOTE: In most cases the functions already change the flags internally

    // State change: FLAG_VSYNC_HINT
    if (((CORE.Window.flags & FLAG_VSYNC_HINT) > 0) && ((flags & FLAG_VSYNC_HINT) > 0))
    {
        glfwSwapInterval(0);
        CORE.Window.flags &= ~FLAG_VSYNC_HINT;
    }

    // State change: FLAG_BORDERLESS_WINDOWED_MODE
    // NOTE: This must be handled before FLAG_FULLSCREEN_MODE because ToggleBorderlessWindowed() needs to get some fullscreen values if fullscreen is running
    if (((CORE.Window.flags & FLAG_BORDERLESS_WINDOWED_MODE) > 0) && ((flags & FLAG_BORDERLESS_WINDOWED_MODE) > 0))
    {
        ToggleBorderlessWindowed();     // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_FULLSCREEN_MODE
    if (((CORE.Window.flags & FLAG_FULLSCREEN_MODE) > 0) && ((flags & FLAG_FULLSCREEN_MODE) > 0))
    {
        ToggleFullscreen();     // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_RESIZABLE
    if (((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) > 0) && ((flags & FLAG_WINDOW_RESIZABLE) > 0))
    {
        glfwSetWindowAttrib(CORE.Window.handle, GLFW_RESIZABLE, GLFW_FALSE);
        CORE.Window.flags &= ~FLAG_WINDOW_RESIZABLE;
    }

    // State change: FLAG_WINDOW_HIDDEN
    if (((CORE.Window.flags & FLAG_WINDOW_HIDDEN) > 0) && ((flags & FLAG_WINDOW_HIDDEN) > 0))
    {
        glfwShowWindow(CORE.Window.handle);
        CORE.Window.flags &= ~FLAG_WINDOW_HIDDEN;
    }

    // State change: FLAG_WINDOW_MINIMIZED
    if (((CORE.Window.flags & FLAG_WINDOW_MINIMIZED) > 0) && ((flags & FLAG_WINDOW_MINIMIZED) > 0))
    {
        RestoreWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_MAXIMIZED
    if (((CORE.Window.flags & FLAG_WINDOW_MAXIMIZED) > 0) && ((flags & FLAG_WINDOW_MAXIMIZED) > 0))
    {
        RestoreWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: FLAG_WINDOW_UNDECORATED
    if (((CORE.Window.flags & FLAG_WINDOW_UNDECORATED) > 0) && ((flags & FLAG_WINDOW_UNDECORATED) > 0))
    {
        glfwSetWindowAttrib(CORE.Window.handle, GLFW_DECORATED, GLFW_TRUE);
        CORE.Window.flags &= ~FLAG_WINDOW_UNDECORATED;
    }

    // State change: FLAG_WINDOW_UNFOCUSED
    if (((CORE.Window.flags & FLAG_WINDOW_UNFOCUSED) > 0) && ((flags & FLAG_WINDOW_UNFOCUSED) > 0))
    {
        glfwSetWindowAttrib(CORE.Window.handle, GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
        CORE.Window.flags &= ~FLAG_WINDOW_UNFOCUSED;
    }

    // State change: FLAG_WINDOW_TOPMOST
    if (((CORE.Window.flags & FLAG_WINDOW_TOPMOST) > 0) && ((flags & FLAG_WINDOW_TOPMOST) > 0))
    {
        glfwSetWindowAttrib(CORE.Window.handle, GLFW_FLOATING, GLFW_FALSE);
        CORE.Window.flags &= ~FLAG_WINDOW_TOPMOST;
    }

    // State change: FLAG_WINDOW_ALWAYS_RUN
    if (((CORE.Window.flags & FLAG_WINDOW_ALWAYS_RUN) > 0) && ((flags & FLAG_WINDOW_ALWAYS_RUN) > 0))
    {
        CORE.Window.flags &= ~FLAG_WINDOW_ALWAYS_RUN;
    }

    // The following states can not be changed after window creation

    // State change: FLAG_WINDOW_TRANSPARENT
    if (((CORE.Window.flags & FLAG_WINDOW_TRANSPARENT) > 0) && ((flags & FLAG_WINDOW_TRANSPARENT) > 0))
    {
        TRACELOG(LOG_WARNING, "WINDOW: Framebuffer transparency can only be configured before window initialization");
    }

    // State change: FLAG_WINDOW_HIGHDPI
    if (((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) > 0) && ((flags & FLAG_WINDOW_HIGHDPI) > 0))
    {
        TRACELOG(LOG_WARNING, "WINDOW: High DPI can only be configured before window initialization");
    }

    // State change: FLAG_WINDOW_MOUSE_PASSTHROUGH
    if (((CORE.Window.flags & FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0) && ((flags & FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0))
    {
        glfwSetWindowAttrib(CORE.Window.handle, GLFW_MOUSE_PASSTHROUGH, GLFW_FALSE);
        CORE.Window.flags &= ~FLAG_WINDOW_MOUSE_PASSTHROUGH;
    }

    // State change: FLAG_MSAA_4X_HINT
    if (((CORE.Window.flags & FLAG_MSAA_4X_HINT) > 0) && ((flags & FLAG_MSAA_4X_HINT) > 0))
    {
        TRACELOG(LOG_WARNING, "WINDOW: MSAA can only be configured before window initialization");
    }

    // State change: FLAG_INTERLACED_HINT
    if (((CORE.Window.flags & FLAG_INTERLACED_HINT) > 0) && ((flags & FLAG_INTERLACED_HINT) > 0))
    {
        TRACELOG(LOG_WARNING, "RPI: Interlaced mode can only be configured before window initialization");
    }
#endif
}

// Set icon for window (only PLATFORM_DESKTOP)
// NOTE 1: Image must be in RGBA format, 8bit per channel
// NOTE 2: Image is scaled by the OS for all required sizes
void SetWindowIcon(Image image)
{
#if defined(PLATFORM_DESKTOP)
    if (image.data == NULL)
    {
        // Revert to the default window icon, pass in an empty image array
        glfwSetWindowIcon(CORE.Window.handle, 0, NULL);
    }
    else
    {
        if (image.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
        {
            GLFWimage icon[1] = { 0 };

            icon[0].width = image.width;
            icon[0].height = image.height;
            icon[0].pixels = (unsigned char *)image.data;

            // NOTE 1: We only support one image icon
            // NOTE 2: The specified image data is copied before this function returns
            glfwSetWindowIcon(CORE.Window.handle, 1, icon);
        }
        else TRACELOG(LOG_WARNING, "GLFW: Window icon image must be in R8G8B8A8 pixel format");
    }
#endif
}

// Set icon for window (multiple images, only PLATFORM_DESKTOP)
// NOTE 1: Images must be in RGBA format, 8bit per channel
// NOTE 2: The multiple images are used depending on provided sizes
// Standard Windows icon sizes: 256, 128, 96, 64, 48, 32, 24, 16
void SetWindowIcons(Image *images, int count)
{
#if defined(PLATFORM_DESKTOP)
    if ((images == NULL) || (count <= 0))
    {
        // Revert to the default window icon, pass in an empty image array
        glfwSetWindowIcon(CORE.Window.handle, 0, NULL);
    }
    else
    {
        int valid = 0;
        GLFWimage *icons = RL_CALLOC(count, sizeof(GLFWimage));

        for (int i = 0; i < count; i++)
        {
            if (images[i].format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
            {
                icons[valid].width = images[i].width;
                icons[valid].height = images[i].height;
                icons[valid].pixels = (unsigned char *)images[i].data;

                valid++;
            }
            else TRACELOG(LOG_WARNING, "GLFW: Window icon image must be in R8G8B8A8 pixel format");
        }
        // NOTE: Images data is copied internally before this function returns
        glfwSetWindowIcon(CORE.Window.handle, valid, icons);

        RL_FREE(icons);
    }
#endif
}

// Set title for window (only PLATFORM_DESKTOP and PLATFORM_WEB)
void SetWindowTitle(const char *title)
{
    CORE.Window.title = title;
#if defined(PLATFORM_DESKTOP)
    glfwSetWindowTitle(CORE.Window.handle, title);
#endif
#if defined(PLATFORM_WEB)
    emscripten_set_window_title(title);
#endif
}

// Set window position on screen (windowed mode)
void SetWindowPosition(int x, int y)
{
#if defined(PLATFORM_DESKTOP)
    glfwSetWindowPos(CORE.Window.handle, x, y);
#endif
}

// Set monitor for the current window
void SetWindowMonitor(int monitor)
{
#if defined(PLATFORM_DESKTOP)
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        if (CORE.Window.fullscreen)
        {
            TRACELOG(LOG_INFO, "GLFW: Selected fullscreen monitor: [%i] %s", monitor, glfwGetMonitorName(monitors[monitor]));

            const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);
            glfwSetWindowMonitor(CORE.Window.handle, monitors[monitor], 0, 0, mode->width, mode->height, mode->refreshRate);
        }
        else
        {
            TRACELOG(LOG_INFO, "GLFW: Selected monitor: [%i] %s", monitor, glfwGetMonitorName(monitors[monitor]));

            const int screenWidth = CORE.Window.screen.width;
            const int screenHeight = CORE.Window.screen.height;
            int monitorWorkareaX = 0;
            int monitorWorkareaY = 0;
            int monitorWorkareaWidth = 0;
            int monitorWorkareaHeight = 0;
            glfwGetMonitorWorkarea(monitors[monitor], &monitorWorkareaX, &monitorWorkareaY, &monitorWorkareaWidth, &monitorWorkareaHeight);

            // If the screen size is larger than the monitor workarea, anchor it on the top left corner, otherwise, center it
            if ((screenWidth >= monitorWorkareaWidth) || (screenHeight >= monitorWorkareaHeight)) glfwSetWindowPos(CORE.Window.handle, monitorWorkareaX, monitorWorkareaY);
            else
            {
                const int x = monitorWorkareaX + (monitorWorkareaWidth/2) - (screenWidth/2);
                const int y = monitorWorkareaY + (monitorWorkareaHeight/2) - (screenHeight/2);
                glfwSetWindowPos(CORE.Window.handle, x, y);
            }
        }
    }
    else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
#endif
}

// Set window minimum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMinSize(int width, int height)
{
    CORE.Window.windowMin.width = width;
    CORE.Window.windowMin.height = height;
#if defined(PLATFORM_DESKTOP)
    int minWidth  = (CORE.Window.windowMin.width  == 0) ? GLFW_DONT_CARE : CORE.Window.windowMin.width;
    int minHeight = (CORE.Window.windowMin.height == 0) ? GLFW_DONT_CARE : CORE.Window.windowMin.height;
    int maxWidth  = (CORE.Window.windowMax.width  == 0) ? GLFW_DONT_CARE : CORE.Window.windowMax.width;
    int maxHeight = (CORE.Window.windowMax.height == 0) ? GLFW_DONT_CARE : CORE.Window.windowMax.height;
    glfwSetWindowSizeLimits(CORE.Window.handle, minWidth, minHeight, maxWidth, maxHeight);
#endif
#if defined(PLATFORM_WEB)
    // Trigger the resize event once to update the window minimum width and height
    if ((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) != 0) EmscriptenResizeCallback(EMSCRIPTEN_EVENT_RESIZE, NULL, NULL);
#endif
}

// Set window maximum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMaxSize(int width, int height)
{
    CORE.Window.windowMax.width = width;
    CORE.Window.windowMax.height = height;
#if defined(PLATFORM_DESKTOP)
    int minWidth  = (CORE.Window.windowMin.width  == 0) ? GLFW_DONT_CARE : CORE.Window.windowMin.width;
    int minHeight = (CORE.Window.windowMin.height == 0) ? GLFW_DONT_CARE : CORE.Window.windowMin.height;
    int maxWidth  = (CORE.Window.windowMax.width  == 0) ? GLFW_DONT_CARE : CORE.Window.windowMax.width;
    int maxHeight = (CORE.Window.windowMax.height == 0) ? GLFW_DONT_CARE : CORE.Window.windowMax.height;
    glfwSetWindowSizeLimits(CORE.Window.handle, minWidth, minHeight, maxWidth, maxHeight);
#endif
#if defined(PLATFORM_WEB)
    // Trigger the resize event once to update the window maximum width and height
    if ((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) != 0) EmscriptenResizeCallback(EMSCRIPTEN_EVENT_RESIZE, NULL, NULL);
#endif
}

// Set window dimensions
void SetWindowSize(int width, int height)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    glfwSetWindowSize(CORE.Window.handle, width, height);
#endif
}

// Set window opacity, value opacity is between 0.0 and 1.0
void SetWindowOpacity(float opacity)
{
#if defined(PLATFORM_DESKTOP)
    if (opacity >= 1.0f) opacity = 1.0f;
    else if (opacity <= 0.0f) opacity = 0.0f;
    glfwSetWindowOpacity(CORE.Window.handle, opacity);
#endif
}

// Set window focused
void SetWindowFocused(void)
{
#if defined(PLATFORM_DESKTOP)
    glfwFocusWindow(CORE.Window.handle);
#endif
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

// Get current render width which is equal to screen width * dpi scale
int GetRenderWidth(void)
{
    return CORE.Window.render.width;
}

// Get current screen height which is equal to screen height * dpi scale
int GetRenderHeight(void)
{
    return CORE.Window.render.height;
}

// Get native window handle
void *GetWindowHandle(void)
{
#if defined(PLATFORM_DESKTOP) && defined(_WIN32)
    // NOTE: Returned handle is: void *HWND (windows.h)
    return glfwGetWin32Window(CORE.Window.handle);
#endif
#if defined(PLATFORM_DESKTOP) && defined(__linux__)
    // NOTE: Returned handle is: unsigned long Window (X.h)
    // typedef unsigned long XID;
    // typedef XID Window;
    //unsigned long id = (unsigned long)glfwGetX11Window(CORE.Window.handle);
    //return NULL;    // TODO: Find a way to return value... cast to void *?
    return (void *)CORE.Window.handle;
#endif
#if defined(__APPLE__)
    // NOTE: Returned handle is: (objc_object *)
    return (void *)glfwGetCocoaWindow(CORE.Window.handle);
#endif

    return NULL;
}

// Get number of monitors
int GetMonitorCount(void)
{
#if defined(PLATFORM_DESKTOP)
    int monitorCount;
    glfwGetMonitors(&monitorCount);
    return monitorCount;
#else
    return 1;
#endif
}

// Get number of monitors
int GetCurrentMonitor(void)
{
    int index = 0;

#if defined(PLATFORM_DESKTOP)
    int monitorCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);
    GLFWmonitor *monitor = NULL;

    if (monitorCount > 1)
    {
        if (IsWindowFullscreen())
        {
            // Get the handle of the monitor that the specified window is in full screen on
            monitor = glfwGetWindowMonitor(CORE.Window.handle);

            for (int i = 0; i < monitorCount; i++)
            {
                if (monitors[i] == monitor)
                {
                    index = i;
                    break;
                }
            }
        }
        else
        {
            int x = 0;
            int y = 0;

            glfwGetWindowPos(CORE.Window.handle, &x, &y);

            for (int i = 0; i < monitorCount; i++)
            {
                int mx = 0;
                int my = 0;

                monitor = monitors[i];
                glfwGetMonitorPos(monitor, &mx, &my);
                const GLFWvidmode *mode = glfwGetVideoMode(monitor);
                if (mode)
                {
                    const int width = mode->width;
                    const int height = mode->height;

                    if ((x >= mx) &&
                        (x < (mx + width)) &&
                        (y >= my) &&
                        (y < (my + height)))
                    {
                        index = i;
                        break;
                    }
                }
                else TRACELOG(LOG_WARNING, "GLFW: Failed to find video mode for selected monitor");
            }
        }
    }
#endif

    return index;
}

// Get selected monitor position
Vector2 GetMonitorPosition(int monitor)
{
#if defined(PLATFORM_DESKTOP)
    int monitorCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        int x, y;
        glfwGetMonitorPos(monitors[monitor], &x, &y);

        return (Vector2){ (float)x, (float)y };
    }
    else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
#endif
    return (Vector2){ 0, 0 };
}

// Get selected monitor width (currently used by monitor)
int GetMonitorWidth(int monitor)
{
#if defined(PLATFORM_DESKTOP)
    int monitorCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);

        if (mode) return mode->width;
        else TRACELOG(LOG_WARNING, "GLFW: Failed to find video mode for selected monitor");
    }
    else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
#endif
#if defined(PLATFORM_ANDROID)
    if (CORE.Android.app->window != NULL)
    {
        return ANativeWindow_getWidth(CORE.Android.app->window);
    }
#endif
    return 0;
}

// Get selected monitor height (currently used by monitor)
int GetMonitorHeight(int monitor)
{
#if defined(PLATFORM_DESKTOP)
    int monitorCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);

        if (mode) return mode->height;
        else TRACELOG(LOG_WARNING, "GLFW: Failed to find video mode for selected monitor");
    }
    else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
#endif
#if defined(PLATFORM_ANDROID)
    if (CORE.Android.app->window != NULL)
    {
        return ANativeWindow_getHeight(CORE.Android.app->window);
    }
#endif
    return 0;
}

// Get selected monitor physical width in millimetres
int GetMonitorPhysicalWidth(int monitor)
{
#if defined(PLATFORM_DESKTOP)
    int monitorCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        int physicalWidth;
        glfwGetMonitorPhysicalSize(monitors[monitor], &physicalWidth, NULL);
        return physicalWidth;
    }
    else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
#endif
    return 0;
}

// Get selected monitor physical height in millimetres
int GetMonitorPhysicalHeight(int monitor)
{
#if defined(PLATFORM_DESKTOP)
    int monitorCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        int physicalHeight;
        glfwGetMonitorPhysicalSize(monitors[monitor], NULL, &physicalHeight);
        return physicalHeight;
    }
    else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
#endif
    return 0;
}

// Get selected monitor refresh rate
int GetMonitorRefreshRate(int monitor)
{
#if defined(PLATFORM_DESKTOP)
    int monitorCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        const GLFWvidmode *vidmode = glfwGetVideoMode(monitors[monitor]);
        return vidmode->refreshRate;
    }
    else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
#endif
#if defined(PLATFORM_DRM)
    if ((CORE.Window.connector) && (CORE.Window.modeIndex >= 0))
    {
        return CORE.Window.connector->modes[CORE.Window.modeIndex].vrefresh;
    }
#endif
    return 0;
}

// Get window position XY on monitor
Vector2 GetWindowPosition(void)
{
    int x = 0;
    int y = 0;
#if defined(PLATFORM_DESKTOP)
    glfwGetWindowPos(CORE.Window.handle, &x, &y);
#endif
    return (Vector2){ (float)x, (float)y };
}

// Get window scale DPI factor for current monitor
Vector2 GetWindowScaleDPI(void)
{
    Vector2 scale = { 1.0f, 1.0f };

#if defined(PLATFORM_DESKTOP)
    float xdpi = 1.0;
    float ydpi = 1.0;
    Vector2 windowPos = GetWindowPosition();

    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    // Check window monitor
    for (int i = 0; i < monitorCount; i++)
    {
        glfwGetMonitorContentScale(monitors[i], &xdpi, &ydpi);

        int xpos, ypos, width, height;
        glfwGetMonitorWorkarea(monitors[i], &xpos, &ypos, &width, &height);

        if ((windowPos.x >= xpos) && (windowPos.x < xpos + width) &&
            (windowPos.y >= ypos) && (windowPos.y < ypos + height))
        {
            scale.x = xdpi;
            scale.y = ydpi;
            break;
        }
    }
#endif

    return scale;
}

// Get the human-readable, UTF-8 encoded name of the selected monitor
const char *GetMonitorName(int monitor)
{
#if defined(PLATFORM_DESKTOP)
    int monitorCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        return glfwGetMonitorName(monitors[monitor]);
    }
    else TRACELOG(LOG_WARNING, "GLFW: Failed to find selected monitor");
#endif
    return "";
}

// Set clipboard text content
void SetClipboardText(const char *text)
{
#if defined(PLATFORM_DESKTOP)
    glfwSetClipboardString(CORE.Window.handle, text);
#endif
#if defined(PLATFORM_WEB)
    // Security check to (partially) avoid malicious code
    if (strchr(text, '\'') != NULL) TRACELOG(LOG_WARNING, "SYSTEM: Provided Clipboard could be potentially malicious, avoid [\'] character");
    else EM_ASM( { navigator.clipboard.writeText(UTF8ToString($0)); }, text);
#endif
}

// Get clipboard text content
// NOTE: returned string is allocated and freed by GLFW
const char *GetClipboardText(void)
{
#if defined(PLATFORM_DESKTOP)
    return glfwGetClipboardString(CORE.Window.handle);
#endif
#if defined(PLATFORM_WEB)
/*
    // Accessing clipboard data from browser is tricky due to security reasons
    // The method to use is navigator.clipboard.readText() but this is an asynchronous method
    // that will return at some moment after the function is called with the required data
    emscripten_run_script_string("navigator.clipboard.readText() \
        .then(text => { document.getElementById('clipboard').innerText = text; console.log('Pasted content: ', text); }) \
        .catch(err => { console.error('Failed to read clipboard contents: ', err); });"
    );

    // The main issue is getting that data, one approach could be using ASYNCIFY and wait
    // for the data but it requires adding Asyncify emscripten library on compilation

    // Another approach could be just copy the data in a HTML text field and try to retrieve it
    // later on if available... and clean it for future accesses
*/
    return NULL;
#endif
    return NULL;
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

// Show mouse cursor
void ShowCursor(void)
{
#if defined(PLATFORM_DESKTOP)
    glfwSetInputMode(CORE.Window.handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
#endif

    CORE.Input.Mouse.cursorHidden = false;
}

// Hides mouse cursor
void HideCursor(void)
{
#if defined(PLATFORM_DESKTOP)
    glfwSetInputMode(CORE.Window.handle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
#endif

    CORE.Input.Mouse.cursorHidden = true;
}

// Check if cursor is not visible
bool IsCursorHidden(void)
{
    return CORE.Input.Mouse.cursorHidden;
}

// Enables cursor (unlock cursor)
void EnableCursor(void)
{
#if defined(PLATFORM_DESKTOP)
    glfwSetInputMode(CORE.Window.handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
#endif
#if defined(PLATFORM_WEB)
    emscripten_exit_pointerlock();
#endif
    // Set cursor position in the middle
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);

    CORE.Input.Mouse.cursorHidden = false;
}

// Disables cursor (lock cursor)
void DisableCursor(void)
{
#if defined(PLATFORM_DESKTOP)
    glfwSetInputMode(CORE.Window.handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif
#if defined(PLATFORM_WEB)
    emscripten_request_pointerlock("#canvas", 1);
#endif
    // Set cursor position in the middle
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);

    CORE.Input.Mouse.cursorHidden = true;
}

// Check if cursor is on the current screen.
bool IsCursorOnScreen(void)
{
    return CORE.Input.Mouse.cursorOnScreen;
}

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

#if defined(SUPPORT_GIF_RECORDING)
    // Draw record indicator
    if (gifRecording)
    {
        #define GIF_RECORD_FRAMERATE    10
        gifFrameCounter++;

        // NOTE: We record one gif frame every 10 game frames
        if ((gifFrameCounter%GIF_RECORD_FRAMERATE) == 0)
        {
            // Get image data for the current frame (from backbuffer)
            // NOTE: This process is quite slow... :(
            Vector2 scale = GetWindowScaleDPI();
            unsigned char *screenData = rlReadScreenPixels((int)((float)CORE.Window.render.width*scale.x), (int)((float)CORE.Window.render.height*scale.y));
            msf_gif_frame(&gifState, screenData, 10, 16, (int)((float)CORE.Window.render.width*scale.x)*4);

            RL_FREE(screenData);    // Free image data
        }

    #if defined(SUPPORT_MODULE_RSHAPES) && defined(SUPPORT_MODULE_RTEXT)
        if (((gifFrameCounter/15)%2) == 1)
        {
            DrawCircle(30, CORE.Window.screen.height - 20, 10, MAROON);                 // WARNING: Module required: rshapes
            DrawText("GIF RECORDING", 50, CORE.Window.screen.height - 25, 10, RED);     // WARNING: Module required: rtext
        }
    #endif

        rlDrawRenderBatchActive();  // Update and draw internal render batch
    }
#endif

#if defined(SUPPORT_EVENTS_AUTOMATION)
    // Draw record/play indicator
    if (eventsRecording)
    {
        gifFrameCounter++;

        if (((gifFrameCounter/15)%2) == 1)
        {
            DrawCircle(30, CORE.Window.screen.height - 20, 10, MAROON);
            DrawText("EVENTS RECORDING", 50, CORE.Window.screen.height - 25, 10, RED);
        }

        rlDrawRenderBatchActive();  // Update and draw internal render batch
    }
    else if (eventsPlaying)
    {
        gifFrameCounter++;

        if (((gifFrameCounter/15)%2) == 1)
        {
            DrawCircle(30, CORE.Window.screen.height - 20, 10, LIME);
            DrawText("EVENTS PLAYING", 50, CORE.Window.screen.height - 25, 10, GREEN);
        }

        rlDrawRenderBatchActive();  // Update and draw internal render batch
    }
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

#if defined(SUPPORT_EVENTS_AUTOMATION)
    // Events recording and playing logic
    if (eventsRecording) RecordAutomationEvent(CORE.Time.frameCounter);
    else if (eventsPlaying)
    {
        // TODO: When should we play? After/before/replace PollInputEvents()?
        if (CORE.Time.frameCounter >= eventCount) eventsPlaying = false;
        PlayAutomationEvent(CORE.Time.frameCounter);
    }
#endif

    CORE.Time.frameCounter++;
}

// Initialize 2D mode with custom camera (2D)
void BeginMode2D(Camera2D camera)
{
    rlDrawRenderBatchActive();      // Update and draw internal render batch

    rlLoadIdentity();               // Reset current matrix (modelview)

    // Apply 2d camera transformation to modelview
    rlMultMatrixf(MatrixToFloat(GetCameraMatrix2D(camera)));

    // Apply screen scaling if required
    rlMultMatrixf(MatrixToFloat(CORE.Window.screenScale));
}

// Ends 2D mode with custom camera
void EndMode2D(void)
{
    rlDrawRenderBatchActive();      // Update and draw internal render batch

    rlLoadIdentity();               // Reset current matrix (modelview)
    rlMultMatrixf(MatrixToFloat(CORE.Window.screenScale)); // Apply screen scaling if required
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
        double top = RL_CULL_DISTANCE_NEAR*tan(camera.fovy*0.5*DEG2RAD);
        double right = top*aspect;

        rlFrustum(-right, right, -top, top, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    }
    else if (camera.projection == CAMERA_ORTHOGRAPHIC)
    {
        // Setup orthographic projection
        double top = camera.fovy/2.0;
        double right = top*aspect;

        rlOrtho(-right, right, -top,top, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
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

    rlMultMatrixf(MatrixToFloat(CORE.Window.screenScale)); // Apply screen scaling if required

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
    // calculation when using BeginMode3D()
    CORE.Window.currentFbo.width = target.texture.width;
    CORE.Window.currentFbo.height = target.texture.height;
}

// Ends drawing to render texture
void EndTextureMode(void)
{
    rlDrawRenderBatchActive();      // Update and draw internal render batch

    rlDisableFramebuffer();         // Disable render target (fbo)

    // Set viewport to default framebuffer size
    SetupViewport(CORE.Window.render.width, CORE.Window.render.height);

    // Reset current fbo to screen size
    CORE.Window.currentFbo.width = CORE.Window.render.width;
    CORE.Window.currentFbo.height = CORE.Window.render.height;
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
    Vector2 scale = GetWindowScaleDPI();
    rlScissor((int)(x*scale.x), (int)(GetScreenHeight()*scale.y - (((y + height)*scale.y))), (int)(width*scale.x), (int)(height*scale.y));
#else
    if ((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) > 0)
    {
        Vector2 scale = GetWindowScaleDPI();
        rlScissor((int)(x*scale.x), (int)(CORE.Window.currentFbo.height - (y + height)*scale.y), (int)(width*scale.x), (int)(height*scale.y));
    }
    else
    {
        rlScissor(x, CORE.Window.currentFbo.height - (y + height), width, height);
    }
#endif
}

// End scissor mode
void EndScissorMode(void)
{
    rlDrawRenderBatchActive();      // Update and draw internal render batch
    rlDisableScissorTest();
}

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

    if ((rlGetVersion() == RL_OPENGL_33) || (rlGetVersion() >= RL_OPENGL_ES_20))
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
        Matrix proj = MatrixPerspective(fovy, aspect, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);

        config.projection[0] = MatrixMultiply(proj, MatrixTranslate(projOffset, 0.0f, 0.0f));
        config.projection[1] = MatrixMultiply(proj, MatrixTranslate(-projOffset, 0.0f, 0.0f));

        // Compute camera transformation matrices
        // NOTE: Camera movement might seem more natural if we model the head.
        // Our axis of rotation is the base of our head, so we might want to add
        // some y (base of head to eye level) and -z (center of head to eye protrusion) to the camera positions.
        config.viewOffset[0] = MatrixTranslate(-device.interpupillaryDistance*0.5f, 0.075f, 0.045f);
        config.viewOffset[1] = MatrixTranslate(device.interpupillaryDistance*0.5f, 0.075f, 0.045f);

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
    //...
}

// Load shader from files and bind default locations
// NOTE: If shader string is NULL, using default vertex/fragment shaders
Shader LoadShader(const char *vsFileName, const char *fsFileName)
{
    Shader shader = { 0 };

    char *vShaderStr = NULL;
    char *fShaderStr = NULL;

    if (vsFileName != NULL) vShaderStr = LoadFileText(vsFileName);
    if (fsFileName != NULL) fShaderStr = LoadFileText(fsFileName);

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

    // After shader loading, we TRY to set default location names
    if (shader.id > 0)
    {
        // Default shader attribute locations have been binded before linking:
        //          vertex position location    = 0
        //          vertex texcoord location    = 1
        //          vertex normal location      = 2
        //          vertex color location       = 3
        //          vertex tangent location     = 4
        //          vertex texcoord2 location   = 5

        // NOTE: If any location is not found, loc point becomes -1

        shader.locs = (int *)RL_CALLOC(RL_MAX_SHADER_LOCATIONS, sizeof(int));

        // All locations reset to -1 (no location)
        for (int i = 0; i < RL_MAX_SHADER_LOCATIONS; i++) shader.locs[i] = -1;

        // Get handles to GLSL input attribute locations
        shader.locs[SHADER_LOC_VERTEX_POSITION] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION);
        shader.locs[SHADER_LOC_VERTEX_TEXCOORD01] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD);
        shader.locs[SHADER_LOC_VERTEX_TEXCOORD02] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2);
        shader.locs[SHADER_LOC_VERTEX_NORMAL] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL);
        shader.locs[SHADER_LOC_VERTEX_TANGENT] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT);
        shader.locs[SHADER_LOC_VERTEX_COLOR] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR);

        // Get handles to GLSL uniform locations (vertex shader)
        shader.locs[SHADER_LOC_MATRIX_MVP] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_MVP);
        shader.locs[SHADER_LOC_MATRIX_VIEW] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW);
        shader.locs[SHADER_LOC_MATRIX_PROJECTION] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION);
        shader.locs[SHADER_LOC_MATRIX_MODEL] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL);
        shader.locs[SHADER_LOC_MATRIX_NORMAL] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL);

        // Get handles to GLSL uniform locations (fragment shader)
        shader.locs[SHADER_LOC_COLOR_DIFFUSE] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR);
        shader.locs[SHADER_LOC_MAP_DIFFUSE] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0);  // SHADER_LOC_MAP_ALBEDO
        shader.locs[SHADER_LOC_MAP_SPECULAR] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1); // SHADER_LOC_MAP_METALNESS
        shader.locs[SHADER_LOC_MAP_NORMAL] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2);
    }

    return shader;
}

// Check if a shader is ready
bool IsShaderReady(Shader shader)
{
    return ((shader.id > 0) &&          // Validate shader id (loaded successfully)
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

// Get a ray trace from mouse position
Ray GetMouseRay(Vector2 mouse, Camera camera)
{
    Ray ray = { 0 };

    // Calculate normalized device coordinates
    // NOTE: y value is negative
    float x = (2.0f*mouse.x)/(float)GetScreenWidth() - 1.0f;
    float y = 1.0f - (2.0f*mouse.y)/(float)GetScreenHeight();
    float z = 1.0f;

    // Store values in a vector
    Vector3 deviceCoords = { x, y, z };

    // Calculate view matrix from camera look at
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);

    Matrix matProj = MatrixIdentity();

    if (camera.projection == CAMERA_PERSPECTIVE)
    {
        // Calculate projection matrix from perspective
        matProj = MatrixPerspective(camera.fovy*DEG2RAD, ((double)GetScreenWidth()/(double)GetScreenHeight()), RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    }
    else if (camera.projection == CAMERA_ORTHOGRAPHIC)
    {
        float aspect = (float)CORE.Window.screen.width/(float)CORE.Window.screen.height;
        double top = camera.fovy/2.0;
        double right = top*aspect;

        // Calculate projection matrix from orthographic
        matProj = MatrixOrtho(-right, right, -top, top, 0.01, 1000.0);
    }

    // Unproject far/near points
    Vector3 nearPoint = Vector3Unproject((Vector3){ deviceCoords.x, deviceCoords.y, 0.0f }, matProj, matView);
    Vector3 farPoint = Vector3Unproject((Vector3){ deviceCoords.x, deviceCoords.y, 1.0f }, matProj, matView);

    // Unproject the mouse cursor in the near plane.
    // We need this as the source position because orthographic projects, compared to perspective doesn't have a
    // convergence point, meaning that the "eye" of the camera is more like a plane than a point.
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
    return MatrixLookAt(camera.position, camera.target, camera.up);
}

// Get camera 2d transform matrix
Matrix GetCameraMatrix2D(Camera2D camera)
{
    Matrix matTransform = { 0 };
    // The camera in world-space is set by
    //   1. Move it to target
    //   2. Rotate by -rotation and scale by (1/zoom)
    //      When setting higher scale, it's more intuitive for the world to become bigger (= camera become smaller),
    //      not for the camera getting bigger, hence the invert. Same deal with rotation.
    //   3. Move it by (-offset);
    //      Offset defines target transform relative to screen, but since we're effectively "moving" screen (camera)
    //      we need to do it into opposite direction (inverse transform)

    // Having camera transform in world-space, inverse of it gives the modelview transform.
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
        matProj = MatrixPerspective(camera.fovy*DEG2RAD, ((double)width/(double)height), RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    }
    else if (camera.projection == CAMERA_ORTHOGRAPHIC)
    {
        float aspect = (float)CORE.Window.screen.width/(float)CORE.Window.screen.height;
        double top = camera.fovy/2.0;
        double right = top*aspect;

        // Calculate projection matrix from orthographic
        matProj = MatrixOrtho(-right, right, -top, top, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    }

    // Calculate view matrix from camera look at (and transpose it)
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);

    // TODO: Why not use Vector3Transform(Vector3 v, Matrix mat)?

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

// Get elapsed time measure in seconds since InitTimer()
// NOTE: On PLATFORM_DESKTOP InitTimer() is called on InitWindow()
// NOTE: On PLATFORM_DESKTOP, timer is initialized on glfwInit()
double GetTime(void)
{
    double time = 0.0;
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    time = glfwGetTime();   // Elapsed time since glfwInit()
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_DRM)
    struct timespec ts = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &ts);
    unsigned long long int nanoSeconds = (unsigned long long int)ts.tv_sec*1000000000LLU + (unsigned long long int)ts.tv_nsec;

    time = (double)(nanoSeconds - CORE.Time.base)*1e-9;  // Elapsed time since InitTimer()
#endif
    return time;
}

// Setup window configuration flags (view FLAGS)
// NOTE: This function is expected to be called before window creation,
// because it sets up some flags for the window creation process.
// To configure window states after creation, just use SetWindowState()
void SetConfigFlags(unsigned int flags)
{
    // Selected flags are set but not evaluated at this point,
    // flag evaluation happens at InitWindow() or SetWindowState()
    CORE.Window.flags |= flags;
}

// NOTE TRACELOG() function is located in [utils.h]

// Takes a screenshot of current screen (saved a .png)
void TakeScreenshot(const char *fileName)
{
#if defined(SUPPORT_MODULE_RTEXTURES)
    // Security check to (partially) avoid malicious code on PLATFORM_WEB
    if (strchr(fileName, '\'') != NULL) { TRACELOG(LOG_WARNING, "SYSTEM: Provided fileName could be potentially malicious, avoid [\'] character");  return; }

    Vector2 scale = GetWindowScaleDPI();
    unsigned char *imgData = rlReadScreenPixels((int)((float)CORE.Window.render.width*scale.x), (int)((float)CORE.Window.render.height*scale.y));
    Image image = { imgData, (int)((float)CORE.Window.render.width*scale.x), (int)((float)CORE.Window.render.height*scale.y), 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };

    char path[2048] = { 0 };
    strcpy(path, TextFormat("%s/%s", CORE.Storage.basePath, fileName));

    ExportImage(image, path);           // WARNING: Module required: rtextures
    RL_FREE(imgData);

#if defined(PLATFORM_WEB)
    // Download file from MEMFS (emscripten memory filesystem)
    // saveFileFromMEMFSToDisk() function is defined in raylib/src/shell.html
    emscripten_run_script(TextFormat("saveFileFromMEMFSToDisk('%s','%s')", GetFileName(path), GetFileName(path)));
#endif

    TRACELOG(LOG_INFO, "SYSTEM: [%s] Screenshot taken successfully", path);
#else
    TRACELOG(LOG_WARNING,"IMAGE: ExportImage() requires module: rtextures");
#endif
}

// Get a random value between min and max (both included)
// WARNING: Ranges higher than RAND_MAX will return invalid results
// More specifically, if (max - min) > INT_MAX there will be an overflow,
// and otherwise if (max - min) > RAND_MAX the random value will incorrectly never exceed a certain threshold
int GetRandomValue(int min, int max)
{
    if (min > max)
    {
        int tmp = max;
        max = min;
        min = tmp;
    }

    if ((unsigned int)(max - min) > (unsigned int)RAND_MAX)
    {
        TRACELOG(LOG_WARNING, "Invalid GetRandomValue() arguments, range should not be higher than %i", RAND_MAX);
    }

    return (rand()%(abs(max - min) + 1) + min);
}

// Set the seed for the random number generator
void SetRandomSeed(unsigned int seed)
{
    srand(seed);
}

// Check if the file exists
bool FileExists(const char *fileName)
{
    bool result = false;

#if defined(_WIN32)
    if (_access(fileName, 0) != -1) result = true;
#else
    if (access(fileName, F_OK) != -1) result = true;
#endif

    // NOTE: Alternatively, stat() can be used instead of access()
    //#include <sys/stat.h>
    //struct stat statbuf;
    //if (stat(filename, &statbuf) == 0) result = true;

    return result;
}

// Check file extension
// NOTE: Extensions checking is not case-sensitive
bool IsFileExtension(const char *fileName, const char *ext)
{
    #define MAX_FILE_EXTENSION_SIZE  16

    bool result = false;
    const char *fileExt = GetFileExtension(fileName);

    if (fileExt != NULL)
    {
#if defined(SUPPORT_MODULE_RTEXT) && defined(SUPPORT_TEXT_MANIPULATION)
        int extCount = 0;
        const char **checkExts = TextSplit(ext, ';', &extCount);  // WARNING: Module required: rtext

        char fileExtLower[MAX_FILE_EXTENSION_SIZE + 1] = { 0 };
        strncpy(fileExtLower, TextToLower(fileExt), MAX_FILE_EXTENSION_SIZE);  // WARNING: Module required: rtext

        for (int i = 0; i < extCount; i++)
        {
            if (strcmp(fileExtLower, TextToLower(checkExts[i])) == 0)
            {
                result = true;
                break;
            }
        }
#else
        if (strcmp(fileExt, ext) == 0) result = true;
#endif
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

// Get pointer to extension for a filename string (includes the dot: .png)
const char *GetFileExtension(const char *fileName)
{
    const char *dot = strrchr(fileName, '.');

    if (!dot || dot == fileName) return NULL;

    return dot;
}

// String pointer reverse break: returns right-most occurrence of charset in s
static const char *strprbrk(const char *s, const char *charset)
{
    const char *latestMatch = NULL;
    for (; s = strpbrk(s, charset), s != NULL; latestMatch = s++) { }
    return latestMatch;
}

// Get pointer to filename for a path string
const char *GetFileName(const char *filePath)
{
    const char *fileName = NULL;
    if (filePath != NULL) fileName = strprbrk(filePath, "\\/");

    if (!fileName) return filePath;

    return fileName + 1;
}

// Get filename string without extension (uses static string)
const char *GetFileNameWithoutExt(const char *filePath)
{
    #define MAX_FILENAMEWITHOUTEXT_LENGTH   256

    static char fileName[MAX_FILENAMEWITHOUTEXT_LENGTH] = { 0 };
    memset(fileName, 0, MAX_FILENAMEWITHOUTEXT_LENGTH);

    if (filePath != NULL) strcpy(fileName, GetFileName(filePath));   // Get filename with extension

    int size = (int)strlen(fileName);   // Get size in bytes

    for (int i = 0; (i < size) && (i < MAX_FILENAMEWITHOUTEXT_LENGTH); i++)
    {
        if (fileName[i] == '.')
        {
            // NOTE: We break on first '.' found
            fileName[i] = '\0';
            break;
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
    if (filePath[1] != ':' && filePath[0] != '\\' && filePath[0] != '/')
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
            memcpy(dirPath + (filePath[1] != ':' && filePath[0] != '\\' && filePath[0] != '/' ? 2 : 0), filePath, strlen(filePath) - (strlen(lastSlash) - 1));
            dirPath[strlen(filePath) - strlen(lastSlash) + (filePath[1] != ':' && filePath[0] != '\\' && filePath[0] != '/' ? 2 : 0)] = '\0';  // Add '\0' manually
        }
    }

    return dirPath;
}

// Get previous directory path for a given path
const char *GetPrevDirectoryPath(const char *dirPath)
{
    static char prevDirPath[MAX_FILEPATH_LENGTH] = { 0 };
    memset(prevDirPath, 0, MAX_FILEPATH_LENGTH);
    int pathLen = (int)strlen(dirPath);

    if (pathLen <= 3) strcpy(prevDirPath, dirPath);

    for (int i = (pathLen - 1); (i >= 0) && (pathLen > 3); i--)
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
    len = GetModuleFileNameW(NULL, widePath, MAX_PATH);
    len = WideCharToMultiByte(0, 0, widePath, len, appDir, MAX_PATH, NULL, NULL);
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
        int len = strlen(appDir);
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
        files.paths = (char **)RL_MALLOC(files.capacity*sizeof(char *));
        for (unsigned int i = 0; i < files.capacity; i++) files.paths[i] = (char *)RL_MALLOC(MAX_FILEPATH_LENGTH*sizeof(char));

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
    for (unsigned int i = 0; i < files.capacity; i++) RL_FREE(files.paths[i]);

    RL_FREE(files.paths);
}

// Change working directory, returns true on success
bool ChangeDirectory(const char *dir)
{
    bool result = CHDIR(dir);

    if (result != 0) TRACELOG(LOG_WARNING, "SYSTEM: Failed to change to directory: %s", dir);

    return (result == 0);
}

// Check if a given path point to a file
bool IsPathFile(const char *path)
{
    struct stat result = { 0 };
    stat(path, &result);

    return S_ISREG(result.st_mode);
}

// Check if a file has been dropped into window
bool IsFileDropped(void)
{
    if (CORE.Window.dropFileCount > 0) return true;
    else return false;
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

// Get file modification time (last write time)
long GetFileModTime(const char *fileName)
{
    struct stat result = { 0 };

    if (stat(fileName, &result) == 0)
    {
        time_t mod = result.st_mtime;

        return (long)mod;
    }

    return 0;
}

// Compress data (DEFLATE algorithm)
unsigned char *CompressData(const unsigned char *data, int dataSize, int *compDataSize)
{
    #define COMPRESSION_QUALITY_DEFLATE  8

    unsigned char *compData = NULL;

#if defined(SUPPORT_COMPRESSION_API)
    // Compress data and generate a valid DEFLATE stream
    struct sdefl sdefl = { 0 };
    int bounds = sdefl_bound(dataSize);
    compData = (unsigned char *)RL_CALLOC(bounds, 1);
    *compDataSize = sdeflate(&sdefl, compData, data, dataSize, COMPRESSION_QUALITY_DEFLATE);   // Compression level 8, same as stbiw

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
    data = (unsigned char *)RL_CALLOC(MAX_DECOMPRESSION_SIZE*1024*1024, 1);
    int length = sinflate(data, MAX_DECOMPRESSION_SIZE*1024*1024, compData, compDataSize);

    // WARNING: RL_REALLOC can make (and leave) data copies in memory, be careful with sensitive compressed data!
    // TODO: Use a different approach, create another buffer, copy data manually to it and wipe original buffer memory
    unsigned char *temp = (unsigned char *)RL_REALLOC(data, length);

    if (temp != NULL) data = temp;
    else TRACELOG(LOG_WARNING, "SYSTEM: Failed to re-allocate required decompression memory");

    *dataSize = length;

    TRACELOG(LOG_INFO, "SYSTEM: Decompress data: Comp. size: %i -> Original size: %i", compDataSize, *dataSize);
#endif

    return data;
}

// Encode data to Base64 string
char *EncodeDataBase64(const unsigned char *data, int dataSize, int *outputSize)
{
    static const unsigned char base64encodeTable[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
    };

    static const int modTable[] = { 0, 2, 1 };

    *outputSize = 4*((dataSize + 2)/3);

    char *encodedData = (char *)RL_MALLOC(*outputSize);

    if (encodedData == NULL) return NULL;

    for (int i = 0, j = 0; i < dataSize;)
    {
        unsigned int octetA = (i < dataSize)? (unsigned char)data[i++] : 0;
        unsigned int octetB = (i < dataSize)? (unsigned char)data[i++] : 0;
        unsigned int octetC = (i < dataSize)? (unsigned char)data[i++] : 0;

        unsigned int triple = (octetA << 0x10) + (octetB << 0x08) + octetC;

        encodedData[j++] = base64encodeTable[(triple >> 3*6) & 0x3F];
        encodedData[j++] = base64encodeTable[(triple >> 2*6) & 0x3F];
        encodedData[j++] = base64encodeTable[(triple >> 1*6) & 0x3F];
        encodedData[j++] = base64encodeTable[(triple >> 0*6) & 0x3F];
    }

    for (int i = 0; i < modTable[dataSize%3]; i++) encodedData[*outputSize - 1 - i] = '=';  // Padding character

    return encodedData;
}

// Decode Base64 string data
unsigned char *DecodeDataBase64(const unsigned char *data, int *outputSize)
{
    static const unsigned char base64decodeTable[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 62, 0, 0, 0, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 0, 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
        37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
    };

    // Get output size of Base64 input data
    int outSize = 0;
    for (int i = 0; data[4*i] != 0; i++)
    {
        if (data[4*i + 3] == '=')
        {
            if (data[4*i + 2] == '=') outSize += 1;
            else outSize += 2;
        }
        else outSize += 3;
    }

    // Allocate memory to store decoded Base64 data
    unsigned char *decodedData = (unsigned char *)RL_MALLOC(outSize);

    for (int i = 0; i < outSize/3; i++)
    {
        unsigned char a = base64decodeTable[(int)data[4*i]];
        unsigned char b = base64decodeTable[(int)data[4*i + 1]];
        unsigned char c = base64decodeTable[(int)data[4*i + 2]];
        unsigned char d = base64decodeTable[(int)data[4*i + 3]];

        decodedData[3*i] = (a << 2) | (b >> 4);
        decodedData[3*i + 1] = (b << 4) | (c >> 2);
        decodedData[3*i + 2] = (c << 6) | d;
    }

    if (outSize%3 == 1)
    {
        int n = outSize/3;
        unsigned char a = base64decodeTable[(int)data[4*n]];
        unsigned char b = base64decodeTable[(int)data[4*n + 1]];
        decodedData[outSize - 1] = (a << 2) | (b >> 4);
    }
    else if (outSize%3 == 2)
    {
        int n = outSize/3;
        unsigned char a = base64decodeTable[(int)data[4*n]];
        unsigned char b = base64decodeTable[(int)data[4*n + 1]];
        unsigned char c = base64decodeTable[(int)data[4*n + 2]];
        decodedData[outSize - 2] = (a << 2) | (b >> 4);
        decodedData[outSize - 1] = (b << 4) | (c >> 2);
    }

    *outputSize = outSize;
    return decodedData;
}

// Open URL with default system browser (if available)
// NOTE: This function is only safe to use if you control the URL given.
// A user could craft a malicious string performing another action.
// Only call this function yourself not with user input or make sure to check the string yourself.
// Ref: https://github.com/raysan5/raylib/issues/686
void OpenURL(const char *url)
{
    // Security check to (partially) avoid malicious code on PLATFORM_WEB
    if (strchr(url, '\'') != NULL) TRACELOG(LOG_WARNING, "SYSTEM: Provided URL could be potentially malicious, avoid [\'] character");
    else
    {
#if defined(PLATFORM_DESKTOP)
        char *cmd = (char *)RL_CALLOC(strlen(url) + 32, sizeof(char));
    #if defined(_WIN32)
        sprintf(cmd, "explorer \"%s\"", url);
    #endif
    #if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
        sprintf(cmd, "xdg-open '%s'", url); // Alternatives: firefox, x-www-browser
    #endif
    #if defined(__APPLE__)
        sprintf(cmd, "open '%s'", url);
    #endif
        int result = system(cmd);
        if (result == -1) TRACELOG(LOG_WARNING, "OpenURL() child process could not be created");
        RL_FREE(cmd);
#endif
#if defined(PLATFORM_WEB)
        emscripten_run_script(TextFormat("window.open('%s', '_blank')", url));
#endif
#if defined(PLATFORM_ANDROID)
        JNIEnv *env = NULL;
        JavaVM *vm = CORE.Android.app->activity->vm;
        (*vm)->AttachCurrentThread(vm, &env, NULL);

        jstring urlString = (*env)->NewStringUTF(env, url);
        jclass uriClass = (*env)->FindClass(env, "android/net/Uri");
        jmethodID uriParse = (*env)->GetStaticMethodID(env, uriClass, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
        jobject uri = (*env)->CallStaticObjectMethod(env, uriClass, uriParse, urlString);

        jclass intentClass = (*env)->FindClass(env, "android/content/Intent");
        jfieldID actionViewId = (*env)->GetStaticFieldID(env, intentClass, "ACTION_VIEW", "Ljava/lang/String;");
        jobject actionView = (*env)->GetStaticObjectField(env, intentClass, actionViewId);
        jmethodID newIntent = (*env)->GetMethodID(env, intentClass, "<init>", "(Ljava/lang/String;Landroid/net/Uri;)V");
        jobject intent = (*env)->AllocObject(env, intentClass);

        (*env)->CallVoidMethod(env, intent, newIntent, actionView, uri);
        jclass activityClass = (*env)->FindClass(env, "android/app/Activity");
        jmethodID startActivity = (*env)->GetMethodID(env, activityClass, "startActivity", "(Landroid/content/Intent;)V");
        (*env)->CallVoidMethod(env, CORE.Android.app->activity->clazz, startActivity, intent);

        (*vm)->DetachCurrentThread(vm);
#endif
    }
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Input (Keyboard, Mouse, Gamepad) Functions
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

// Check if a key has been pressed again (only PLATFORM_DESKTOP)
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
// NOTE: default exitKey is ESCAPE
void SetExitKey(int key)
{
#if !defined(PLATFORM_ANDROID)
    CORE.Input.Keyboard.exitKey = key;
#endif
}

// NOTE: Gamepad support not implemented in emscripten GLFW3 (PLATFORM_WEB)

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
    const char *name = NULL;

#if defined(PLATFORM_DESKTOP)
    if (CORE.Input.Gamepad.ready[gamepad]) name = glfwGetJoystickName(gamepad);
#endif
#if defined(PLATFORM_DRM)
    if (CORE.Input.Gamepad.ready[gamepad])
    {
        ioctl(CORE.Input.Gamepad.streamId[gamepad], JSIOCGNAME(64), &CORE.Input.Gamepad.name[gamepad]);
        name = CORE.Input.Gamepad.name[gamepad];
    }
#endif
#if defined(PLATFORM_WEB)
    name = CORE.Input.Gamepad.name[gamepad];
#endif

    return name;
}

// Get gamepad axis count
int GetGamepadAxisCount(int gamepad)
{
#if defined(PLATFORM_DRM)
    int axisCount = 0;
    if (CORE.Input.Gamepad.ready[gamepad]) ioctl(CORE.Input.Gamepad.streamId[gamepad], JSIOCGAXES, &axisCount);
    CORE.Input.Gamepad.axisCount = axisCount;
#endif

    return CORE.Input.Gamepad.axisCount;
}

// Get axis movement vector for a gamepad
float GetGamepadAxisMovement(int gamepad, int axis)
{
    float value = 0;

    if ((gamepad < MAX_GAMEPADS) && CORE.Input.Gamepad.ready[gamepad] && (axis < MAX_GAMEPAD_AXIS) &&
        (fabsf(CORE.Input.Gamepad.axisState[gamepad][axis]) > 0.1f)) value = CORE.Input.Gamepad.axisState[gamepad][axis];      // 0.1f = GAMEPAD_AXIS_MINIMUM_DRIFT/DELTA

    return value;
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

// Set internal gamepad mappings
int SetGamepadMappings(const char *mappings)
{
    int result = 0;

#if defined(PLATFORM_DESKTOP)
    result = glfwUpdateGamepadMappings(mappings);
#endif

    return result;
}

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
#if defined(PLATFORM_ANDROID)
    return (int)CORE.Input.Touch.position[0].x;
#else
    return (int)((CORE.Input.Mouse.currentPosition.x + CORE.Input.Mouse.offset.x)*CORE.Input.Mouse.scale.x);
#endif
}

// Get mouse position Y
int GetMouseY(void)
{
#if defined(PLATFORM_ANDROID)
    return (int)CORE.Input.Touch.position[0].y;
#else
    return (int)((CORE.Input.Mouse.currentPosition.y + CORE.Input.Mouse.offset.y)*CORE.Input.Mouse.scale.y);
#endif
}

// Get mouse position XY
Vector2 GetMousePosition(void)
{
    Vector2 position = { 0 };

    // TODO: Review touch position on PLATFORM_WEB

#if defined(PLATFORM_ANDROID) //|| defined(PLATFORM_WEB)
    position = GetTouchPosition(0);
#else
    // NOTE: On PLATFORM_WEB, even on canvas scaling, mouse position is proportionally returned
    position.x = (CORE.Input.Mouse.currentPosition.x + CORE.Input.Mouse.offset.x)*CORE.Input.Mouse.scale.x;
    position.y = (CORE.Input.Mouse.currentPosition.y + CORE.Input.Mouse.offset.y)*CORE.Input.Mouse.scale.y;
#endif

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

// Set mouse position XY
void SetMousePosition(int x, int y)
{
    CORE.Input.Mouse.currentPosition = (Vector2){ (float)x, (float)y };
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    // NOTE: emscripten not implemented
    glfwSetCursorPos(CORE.Window.handle, CORE.Input.Mouse.currentPosition.x, CORE.Input.Mouse.currentPosition.y);
#endif
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

#if !defined(PLATFORM_ANDROID)
    if (fabsf(CORE.Input.Mouse.currentWheelMove.x) > fabsf(CORE.Input.Mouse.currentWheelMove.y)) result = (float)CORE.Input.Mouse.currentWheelMove.x;
    else result = (float)CORE.Input.Mouse.currentWheelMove.y;
#endif

    return result;
}

// Get mouse wheel movement X/Y as a vector
Vector2 GetMouseWheelMoveV(void)
{
    Vector2 result = { 0 };

    result = CORE.Input.Mouse.currentWheelMove;

    return result;
}

// Set mouse cursor
// NOTE: This is a no-op on platforms other than PLATFORM_DESKTOP
void SetMouseCursor(int cursor)
{
#if defined(PLATFORM_DESKTOP)
    CORE.Input.Mouse.cursor = cursor;
    if (cursor == MOUSE_CURSOR_DEFAULT) glfwSetCursor(CORE.Window.handle, NULL);
    else
    {
        // NOTE: We are relating internal GLFW enum values to our MouseCursor enum values
        glfwSetCursor(CORE.Window.handle, glfwCreateStandardCursor(0x00036000 + cursor));
    }
#endif
}

// Get touch position X for touch point 0 (relative to screen size)
int GetTouchX(void)
{
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
    return (int)CORE.Input.Touch.position[0].x;
#else   // PLATFORM_DESKTOP, PLATFORM_DRM
    return GetMouseX();
#endif
}

// Get touch position Y for touch point 0 (relative to screen size)
int GetTouchY(void)
{
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
    return (int)CORE.Input.Touch.position[0].y;
#else   // PLATFORM_DESKTOP, PLATFORM_DRM
    return GetMouseY();
#endif
}

// Get touch position XY for a touch point index (relative to screen size)
// TODO: Touch position should be scaled depending on display size and render size
Vector2 GetTouchPosition(int index)
{
    Vector2 position = { -1.0f, -1.0f };

#if defined(PLATFORM_DESKTOP)
    // TODO: GLFW does not support multi-touch input just yet
    // https://www.codeproject.com/Articles/668404/Programming-for-Multi-Touch
    // https://docs.microsoft.com/en-us/windows/win32/wintouch/getting-started-with-multi-touch-messages
    if (index == 0) position = GetMousePosition();
#endif
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB) || defined(PLATFORM_DRM)
    if (index < MAX_TOUCH_POINTS) position = CORE.Input.Touch.position[index];
    else TRACELOG(LOG_WARNING, "INPUT: Required touch point out of range (Max touch points: %i)", MAX_TOUCH_POINTS);
#endif

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
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Set viewport for a provided width and height
static void SetupViewport(int width, int height)
{
    CORE.Window.render.width = width;
    CORE.Window.render.height = height;

    // Set viewport width and height
    // NOTE: We consider render size (scaled) and offset in case black bars are required and
    // render area does not match full display area (this situation is only applicable on fullscreen mode)
#if defined(__APPLE__)
    float xScale = 1.0f, yScale = 1.0f;
    glfwGetWindowContentScale(CORE.Window.handle, &xScale, &yScale);
    rlViewport(CORE.Window.renderOffset.x/2*xScale, CORE.Window.renderOffset.y/2*yScale, (CORE.Window.render.width)*xScale, (CORE.Window.render.height)*yScale);
#else
    rlViewport(CORE.Window.renderOffset.x/2, CORE.Window.renderOffset.y/2, CORE.Window.render.width, CORE.Window.render.height);
#endif

    rlMatrixMode(RL_PROJECTION);        // Switch to projection matrix
    rlLoadIdentity();                   // Reset current matrix (projection)

    // Set orthographic projection to current framebuffer size
    // NOTE: Configured top-left corner as (0, 0)
    rlOrtho(0, CORE.Window.render.width, CORE.Window.render.height, 0, 0.0f, 1.0f);

    rlMatrixMode(RL_MODELVIEW);         // Switch back to modelview matrix
    rlLoadIdentity();                   // Reset current matrix (modelview)
}

// Compute framebuffer size relative to screen size and display size
// NOTE: Global variables CORE.Window.render.width/CORE.Window.render.height and CORE.Window.renderOffset.x/CORE.Window.renderOffset.y can be modified
static void SetupFramebuffer(int width, int height)
{
    // Calculate CORE.Window.render.width and CORE.Window.render.height, we have the display size (input params) and the desired screen size (global var)
    if ((CORE.Window.screen.width > CORE.Window.display.width) || (CORE.Window.screen.height > CORE.Window.display.height))
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Downscaling required: Screen size (%ix%i) is bigger than display size (%ix%i)", CORE.Window.screen.width, CORE.Window.screen.height, CORE.Window.display.width, CORE.Window.display.height);

        // Downscaling to fit display with border-bars
        float widthRatio = (float)CORE.Window.display.width/(float)CORE.Window.screen.width;
        float heightRatio = (float)CORE.Window.display.height/(float)CORE.Window.screen.height;

        if (widthRatio <= heightRatio)
        {
            CORE.Window.render.width = CORE.Window.display.width;
            CORE.Window.render.height = (int)round((float)CORE.Window.screen.height*widthRatio);
            CORE.Window.renderOffset.x = 0;
            CORE.Window.renderOffset.y = (CORE.Window.display.height - CORE.Window.render.height);
        }
        else
        {
            CORE.Window.render.width = (int)round((float)CORE.Window.screen.width*heightRatio);
            CORE.Window.render.height = CORE.Window.display.height;
            CORE.Window.renderOffset.x = (CORE.Window.display.width - CORE.Window.render.width);
            CORE.Window.renderOffset.y = 0;
        }

        // Screen scaling required
        float scaleRatio = (float)CORE.Window.render.width/(float)CORE.Window.screen.width;
        CORE.Window.screenScale = MatrixScale(scaleRatio, scaleRatio, 1.0f);

        // NOTE: We render to full display resolution!
        // We just need to calculate above parameters for downscale matrix and offsets
        CORE.Window.render.width = CORE.Window.display.width;
        CORE.Window.render.height = CORE.Window.display.height;

        TRACELOG(LOG_WARNING, "DISPLAY: Downscale matrix generated, content will be rendered at (%ix%i)", CORE.Window.render.width, CORE.Window.render.height);
    }
    else if ((CORE.Window.screen.width < CORE.Window.display.width) || (CORE.Window.screen.height < CORE.Window.display.height))
    {
        // Required screen size is smaller than display size
        TRACELOG(LOG_INFO, "DISPLAY: Upscaling required: Screen size (%ix%i) smaller than display size (%ix%i)", CORE.Window.screen.width, CORE.Window.screen.height, CORE.Window.display.width, CORE.Window.display.height);

        if ((CORE.Window.screen.width == 0) || (CORE.Window.screen.height == 0))
        {
            CORE.Window.screen.width = CORE.Window.display.width;
            CORE.Window.screen.height = CORE.Window.display.height;
        }

        // Upscaling to fit display with border-bars
        float displayRatio = (float)CORE.Window.display.width/(float)CORE.Window.display.height;
        float screenRatio = (float)CORE.Window.screen.width/(float)CORE.Window.screen.height;

        if (displayRatio <= screenRatio)
        {
            CORE.Window.render.width = CORE.Window.screen.width;
            CORE.Window.render.height = (int)round((float)CORE.Window.screen.width/displayRatio);
            CORE.Window.renderOffset.x = 0;
            CORE.Window.renderOffset.y = (CORE.Window.render.height - CORE.Window.screen.height);
        }
        else
        {
            CORE.Window.render.width = (int)round((float)CORE.Window.screen.height*displayRatio);
            CORE.Window.render.height = CORE.Window.screen.height;
            CORE.Window.renderOffset.x = (CORE.Window.render.width - CORE.Window.screen.width);
            CORE.Window.renderOffset.y = 0;
        }
    }
    else
    {
        CORE.Window.render.width = CORE.Window.screen.width;
        CORE.Window.render.height = CORE.Window.screen.height;
        CORE.Window.renderOffset.x = 0;
        CORE.Window.renderOffset.y = 0;
    }
}

// Initialize hi-resolution timer
static void InitTimer(void)
{
// Setting a higher resolution can improve the accuracy of time-out intervals in wait functions.
// However, it can also reduce overall system performance, because the thread scheduler switches tasks more often.
// High resolutions can also prevent the CPU power management system from entering power-saving modes.
// Setting a higher resolution does not improve the accuracy of the high-resolution performance counter.
#if defined(_WIN32) && defined(SUPPORT_WINMM_HIGHRES_TIMER) && !defined(SUPPORT_BUSY_WAIT_LOOP)
    timeBeginPeriod(1);                 // Setup high-resolution timer to 1ms (granularity of 1-2 ms)
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_DRM)
    struct timespec now = { 0 };

    if (clock_gettime(CLOCK_MONOTONIC, &now) == 0)  // Success
    {
        CORE.Time.base = (unsigned long long int)now.tv_sec*1000000000LLU + (unsigned long long int)now.tv_nsec;
    }
    else TRACELOG(LOG_WARNING, "TIMER: Hi-resolution timer not available");
#endif

    CORE.Time.previous = GetTime();     // Get time as double
}

// Wait for some time (stop program execution)
// NOTE: Sleep() granularity could be around 10 ms, it means, Sleep() could
// take longer than expected... for that reason we use the busy wait loop
// Ref: http://stackoverflow.com/questions/43057578/c-programming-win32-games-sleep-taking-longer-than-expected
// Ref: http://www.geisswerks.com/ryan/FAQS/timing.html --> All about timing on Win32!
void WaitTime(double seconds)
{
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

        // NOTE: Use nanosleep() on Unix platforms... usleep() it's deprecated.
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

// Swap back buffer with front buffer (screen drawing)
void SwapScreenBuffer(void)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    glfwSwapBuffers(CORE.Window.handle);
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_DRM)
    eglSwapBuffers(CORE.Window.device, CORE.Window.surface);

#if defined(PLATFORM_DRM)

    if (!CORE.Window.gbmSurface || (-1 == CORE.Window.fd) || !CORE.Window.connector || !CORE.Window.crtc) TRACELOG(LOG_ERROR, "DISPLAY: DRM initialization failed to swap");

    struct gbm_bo *bo = gbm_surface_lock_front_buffer(CORE.Window.gbmSurface);
    if (!bo) TRACELOG(LOG_ERROR, "DISPLAY: Failed GBM to lock front buffer");

    uint32_t fb = 0;
    int result = drmModeAddFB(CORE.Window.fd, CORE.Window.connector->modes[CORE.Window.modeIndex].hdisplay, CORE.Window.connector->modes[CORE.Window.modeIndex].vdisplay, 24, 32, gbm_bo_get_stride(bo), gbm_bo_get_handle(bo).u32, &fb);
    if (result != 0) TRACELOG(LOG_ERROR, "DISPLAY: drmModeAddFB() failed with result: %d", result);

    result = drmModeSetCrtc(CORE.Window.fd, CORE.Window.crtc->crtc_id, fb, 0, 0, &CORE.Window.connector->connector_id, 1, &CORE.Window.connector->modes[CORE.Window.modeIndex]);
    if (result != 0) TRACELOG(LOG_ERROR, "DISPLAY: drmModeSetCrtc() failed with result: %d", result);

    if (CORE.Window.prevFB)
    {
        result = drmModeRmFB(CORE.Window.fd, CORE.Window.prevFB);
        if (result != 0) TRACELOG(LOG_ERROR, "DISPLAY: drmModeRmFB() failed with result: %d", result);
    }

    CORE.Window.prevFB = fb;

    if (CORE.Window.prevBO) gbm_surface_release_buffer(CORE.Window.gbmSurface, CORE.Window.prevBO);

    CORE.Window.prevBO = bo;

#endif  // PLATFORM_DRM
#endif  // PLATFORM_ANDROID || PLATFORM_DRM
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
    // Reset key repeats
    for (int i = 0; i < MAX_KEYBOARD_KEYS; i++) CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;

    // Reset last gamepad button/axis registered state
    CORE.Input.Gamepad.lastButtonPressed = 0;       // GAMEPAD_BUTTON_UNKNOWN
    CORE.Input.Gamepad.axisCount = 0;

#if defined(PLATFORM_DRM)
    // Register previous keys states
    for (int i = 0; i < MAX_KEYBOARD_KEYS; i++)
    {
        CORE.Input.Keyboard.previousKeyState[i] = CORE.Input.Keyboard.currentKeyState[i];
        CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }

    PollKeyboardEvents();

    // Register previous mouse states
    CORE.Input.Mouse.previousWheelMove = CORE.Input.Mouse.currentWheelMove;
    CORE.Input.Mouse.currentWheelMove = CORE.Input.Mouse.eventWheelMove;
    CORE.Input.Mouse.eventWheelMove = (Vector2){ 0.0f, 0.0f };
    for (int i = 0; i < MAX_MOUSE_BUTTONS; i++)
    {
        CORE.Input.Mouse.previousButtonState[i] = CORE.Input.Mouse.currentButtonState[i];
        CORE.Input.Mouse.currentButtonState[i] = CORE.Input.Mouse.currentButtonStateEvdev[i];
    }

    // Register gamepads buttons events
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        if (CORE.Input.Gamepad.ready[i])
        {
            // Register previous gamepad states
            for (int k = 0; k < MAX_GAMEPAD_BUTTONS; k++) CORE.Input.Gamepad.previousButtonState[i][k] = CORE.Input.Gamepad.currentButtonState[i][k];
        }
    }
#endif

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    // Keyboard/Mouse input polling (automatically managed by GLFW3 through callback)

    // Register previous keys states
    for (int i = 0; i < MAX_KEYBOARD_KEYS; i++)
    {
        CORE.Input.Keyboard.previousKeyState[i] = CORE.Input.Keyboard.currentKeyState[i];
        CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }

    // Register previous mouse states
    for (int i = 0; i < MAX_MOUSE_BUTTONS; i++) CORE.Input.Mouse.previousButtonState[i] = CORE.Input.Mouse.currentButtonState[i];

    // Register previous mouse wheel state
    CORE.Input.Mouse.previousWheelMove = CORE.Input.Mouse.currentWheelMove;
    CORE.Input.Mouse.currentWheelMove = (Vector2){ 0.0f, 0.0f };

    // Register previous mouse position
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
#endif

    // Register previous touch states
    for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.previousTouchState[i] = CORE.Input.Touch.currentTouchState[i];

    // Reset touch positions
    // TODO: It resets on PLATFORM_WEB the mouse position and not filled again until a move-event,
    // so, if mouse is not moved it returns a (0, 0) position... this behaviour should be reviewed!
    //for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.position[i] = (Vector2){ 0, 0 };

#if defined(PLATFORM_DESKTOP)
    // Check if gamepads are ready
    // NOTE: We do it here in case of disconnection
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        if (glfwJoystickPresent(i)) CORE.Input.Gamepad.ready[i] = true;
        else CORE.Input.Gamepad.ready[i] = false;
    }

    // Register gamepads buttons events
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        if (CORE.Input.Gamepad.ready[i])     // Check if gamepad is available
        {
            // Register previous gamepad states
            for (int k = 0; k < MAX_GAMEPAD_BUTTONS; k++) CORE.Input.Gamepad.previousButtonState[i][k] = CORE.Input.Gamepad.currentButtonState[i][k];

            // Get current gamepad state
            // NOTE: There is no callback available, so we get it manually
            GLFWgamepadstate state = { 0 };
            glfwGetGamepadState(i, &state); // This remapps all gamepads so they have their buttons mapped like an xbox controller

            const unsigned char *buttons = state.buttons;

            for (int k = 0; (buttons != NULL) && (k < GLFW_GAMEPAD_BUTTON_DPAD_LEFT + 1) && (k < MAX_GAMEPAD_BUTTONS); k++)
            {
                int button = -1;        // GamepadButton enum values assigned

                switch (k)
                {
                    case GLFW_GAMEPAD_BUTTON_Y: button = GAMEPAD_BUTTON_RIGHT_FACE_UP; break;
                    case GLFW_GAMEPAD_BUTTON_B: button = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT; break;
                    case GLFW_GAMEPAD_BUTTON_A: button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN; break;
                    case GLFW_GAMEPAD_BUTTON_X: button = GAMEPAD_BUTTON_RIGHT_FACE_LEFT; break;

                    case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER: button = GAMEPAD_BUTTON_LEFT_TRIGGER_1; break;
                    case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER: button = GAMEPAD_BUTTON_RIGHT_TRIGGER_1; break;

                    case GLFW_GAMEPAD_BUTTON_BACK: button = GAMEPAD_BUTTON_MIDDLE_LEFT; break;
                    case GLFW_GAMEPAD_BUTTON_GUIDE: button = GAMEPAD_BUTTON_MIDDLE; break;
                    case GLFW_GAMEPAD_BUTTON_START: button = GAMEPAD_BUTTON_MIDDLE_RIGHT; break;

                    case GLFW_GAMEPAD_BUTTON_DPAD_UP: button = GAMEPAD_BUTTON_LEFT_FACE_UP; break;
                    case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT: button = GAMEPAD_BUTTON_LEFT_FACE_RIGHT; break;
                    case GLFW_GAMEPAD_BUTTON_DPAD_DOWN: button = GAMEPAD_BUTTON_LEFT_FACE_DOWN; break;
                    case GLFW_GAMEPAD_BUTTON_DPAD_LEFT: button = GAMEPAD_BUTTON_LEFT_FACE_LEFT; break;

                    case GLFW_GAMEPAD_BUTTON_LEFT_THUMB: button = GAMEPAD_BUTTON_LEFT_THUMB; break;
                    case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB: button = GAMEPAD_BUTTON_RIGHT_THUMB; break;
                    default: break;
                }

                if (button != -1)   // Check for valid button
                {
                    if (buttons[k] == GLFW_PRESS)
                    {
                        CORE.Input.Gamepad.currentButtonState[i][button] = 1;
                        CORE.Input.Gamepad.lastButtonPressed = button;
                    }
                    else CORE.Input.Gamepad.currentButtonState[i][button] = 0;
                }
            }

            // Get current axis state
            const float *axes = state.axes;

            for (int k = 0; (axes != NULL) && (k < GLFW_GAMEPAD_AXIS_LAST + 1) && (k < MAX_GAMEPAD_AXIS); k++)
            {
                CORE.Input.Gamepad.axisState[i][k] = axes[k];
            }

            // Register buttons for 2nd triggers (because GLFW doesn't count these as buttons but rather axis)
            CORE.Input.Gamepad.currentButtonState[i][GAMEPAD_BUTTON_LEFT_TRIGGER_2] = (char)(CORE.Input.Gamepad.axisState[i][GAMEPAD_AXIS_LEFT_TRIGGER] > 0.1f);
            CORE.Input.Gamepad.currentButtonState[i][GAMEPAD_BUTTON_RIGHT_TRIGGER_2] = (char)(CORE.Input.Gamepad.axisState[i][GAMEPAD_AXIS_RIGHT_TRIGGER] > 0.1f);

            CORE.Input.Gamepad.axisCount = GLFW_GAMEPAD_AXIS_LAST + 1;
        }
    }

    CORE.Window.resizedLastFrame = false;

    if (CORE.Window.eventWaiting) glfwWaitEvents();     // Wait for in input events before continue (drawing is paused)
    else glfwPollEvents();      // Poll input events: keyboard/mouse/window events (callbacks)
#endif  // PLATFORM_DESKTOP

#if defined(PLATFORM_WEB)
    CORE.Window.resizedLastFrame = false;
#endif  // PLATFORM_WEB

// Gamepad support using emscripten API
// NOTE: GLFW3 joystick functionality not available in web
#if defined(PLATFORM_WEB)
    // Get number of gamepads connected
    int numGamepads = 0;
    if (emscripten_sample_gamepad_data() == EMSCRIPTEN_RESULT_SUCCESS) numGamepads = emscripten_get_num_gamepads();

    for (int i = 0; (i < numGamepads) && (i < MAX_GAMEPADS); i++)
    {
        // Register previous gamepad button states
        for (int k = 0; k < MAX_GAMEPAD_BUTTONS; k++) CORE.Input.Gamepad.previousButtonState[i][k] = CORE.Input.Gamepad.currentButtonState[i][k];

        EmscriptenGamepadEvent gamepadState;

        int result = emscripten_get_gamepad_status(i, &gamepadState);

        if (result == EMSCRIPTEN_RESULT_SUCCESS)
        {
            // Register buttons data for every connected gamepad
            for (int j = 0; (j < gamepadState.numButtons) && (j < MAX_GAMEPAD_BUTTONS); j++)
            {
                GamepadButton button = -1;

                // Gamepad Buttons reference: https://www.w3.org/TR/gamepad/#gamepad-interface
                switch (j)
                {
                    case 0: button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN; break;
                    case 1: button = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT; break;
                    case 2: button = GAMEPAD_BUTTON_RIGHT_FACE_LEFT; break;
                    case 3: button = GAMEPAD_BUTTON_RIGHT_FACE_UP; break;
                    case 4: button = GAMEPAD_BUTTON_LEFT_TRIGGER_1; break;
                    case 5: button = GAMEPAD_BUTTON_RIGHT_TRIGGER_1; break;
                    case 6: button = GAMEPAD_BUTTON_LEFT_TRIGGER_2; break;
                    case 7: button = GAMEPAD_BUTTON_RIGHT_TRIGGER_2; break;
                    case 8: button = GAMEPAD_BUTTON_MIDDLE_LEFT; break;
                    case 9: button = GAMEPAD_BUTTON_MIDDLE_RIGHT; break;
                    case 10: button = GAMEPAD_BUTTON_LEFT_THUMB; break;
                    case 11: button = GAMEPAD_BUTTON_RIGHT_THUMB; break;
                    case 12: button = GAMEPAD_BUTTON_LEFT_FACE_UP; break;
                    case 13: button = GAMEPAD_BUTTON_LEFT_FACE_DOWN; break;
                    case 14: button = GAMEPAD_BUTTON_LEFT_FACE_LEFT; break;
                    case 15: button = GAMEPAD_BUTTON_LEFT_FACE_RIGHT; break;
                    default: break;
                }

                if (button != -1)   // Check for valid button
                {
                    if (gamepadState.digitalButton[j] == 1)
                    {
                        CORE.Input.Gamepad.currentButtonState[i][button] = 1;
                        CORE.Input.Gamepad.lastButtonPressed = button;
                    }
                    else CORE.Input.Gamepad.currentButtonState[i][button] = 0;
                }

                //TRACELOGD("INPUT: Gamepad %d, button %d: Digital: %d, Analog: %g", gamepadState.index, j, gamepadState.digitalButton[j], gamepadState.analogButton[j]);
            }

            // Register axis data for every connected gamepad
            for (int j = 0; (j < gamepadState.numAxes) && (j < MAX_GAMEPAD_AXIS); j++)
            {
                CORE.Input.Gamepad.axisState[i][j] = gamepadState.axis[j];
            }

            CORE.Input.Gamepad.axisCount = gamepadState.numAxes;
        }
    }
#endif

#if defined(PLATFORM_ANDROID)
    // Register previous keys states
    // NOTE: Android supports up to 260 keys
    for (int i = 0; i < 260; i++)
    {
        CORE.Input.Keyboard.previousKeyState[i] = CORE.Input.Keyboard.currentKeyState[i];
        CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }

    // Android ALooper_pollAll() variables
    int pollResult = 0;
    int pollEvents = 0;

    // Poll Events (registered events)
    // NOTE: Activity is paused if not enabled (CORE.Android.appEnabled)
    while ((pollResult = ALooper_pollAll(CORE.Android.appEnabled? 0 : -1, NULL, &pollEvents, (void**)&CORE.Android.source)) >= 0)
    {
        // Process this event
        if (CORE.Android.source != NULL) CORE.Android.source->process(CORE.Android.app, CORE.Android.source);

        // NOTE: Never close window, native activity is controlled by the system!
        if (CORE.Android.app->destroyRequested != 0)
        {
            //CORE.Window.shouldClose = true;
            //ANativeActivity_finish(CORE.Android.app->activity);
        }
    }
#endif

#if defined(PLATFORM_DRM) && defined(SUPPORT_SSH_KEYBOARD_RPI)
    // NOTE: Keyboard reading could be done using input_event(s) or just read from stdin, both methods are used here.
    // stdin reading is still used for legacy purposes, it allows keyboard input trough SSH console

    if (!CORE.Input.Keyboard.evtMode) ProcessKeyboard();

    // NOTE: Mouse input events polling is done asynchronously in another pthread - EventThread()
    // NOTE: Gamepad (Joystick) input events polling is done asynchonously in another pthread - GamepadThread()
#endif
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
                sprintf(path, "%s/%s", basePath, dp->d_name);

                if (filter != NULL)
                {
                    if (IsFileExtension(path, filter))
                    {
                        strcpy(files->paths[files->count], path);
                        files->count++;
                    }
                }
                else
                {
                    strcpy(files->paths[files->count], path);
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
                sprintf(path, "%s/%s", basePath, dp->d_name);

                if (IsPathFile(path))
                {
                    if (filter != NULL)
                    {
                        if (IsFileExtension(path, filter))
                        {
                            strcpy(files->paths[files->count], path);
                            files->count++;
                        }
                    }
                    else
                    {
                        strcpy(files->paths[files->count], path);
                        files->count++;
                    }

                    if (files->count >= files->capacity)
                    {
                        TRACELOG(LOG_WARNING, "FILEIO: Maximum filepath scan capacity reached (%i files)", files->capacity);
                        break;
                    }
                }
                else ScanDirectoryFilesRecursively(path, files, filter);
            }
        }

        closedir(dir);
    }
    else TRACELOG(LOG_WARNING, "FILEIO: Directory cannot be opened (%s)", basePath);
}

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
// GLFW3 Error Callback, runs on GLFW3 error
static void ErrorCallback(int error, const char *description)
{
    TRACELOG(LOG_WARNING, "GLFW: Error: %i Description: %s", error, description);
}

// GLFW3 WindowSize Callback, runs when window is resizedLastFrame
// NOTE: Window resizing not allowed by default
static void WindowSizeCallback(GLFWwindow *window, int width, int height)
{
    // Reset viewport and projection matrix for new size
    SetupViewport(width, height);

    CORE.Window.currentFbo.width = width;
    CORE.Window.currentFbo.height = height;
    CORE.Window.resizedLastFrame = true;

    if (IsWindowFullscreen()) return;

    // Set current screen size
#if defined(__APPLE__)
    CORE.Window.screen.width = width;
    CORE.Window.screen.height = height;
#else
    if ((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) > 0)
    {
        Vector2 windowScaleDPI = GetWindowScaleDPI();

        CORE.Window.screen.width = (unsigned int)(width/windowScaleDPI.x);
        CORE.Window.screen.height = (unsigned int)(height/windowScaleDPI.y);
    }
    else
    {
        CORE.Window.screen.width = width;
        CORE.Window.screen.height = height;
    }
#endif

    // NOTE: Postprocessing texture is not scaled to new size
}

// GLFW3 WindowIconify Callback, runs when window is minimized/restored
static void WindowIconifyCallback(GLFWwindow *window, int iconified)
{
    if (iconified) CORE.Window.flags |= FLAG_WINDOW_MINIMIZED;  // The window was iconified
    else CORE.Window.flags &= ~FLAG_WINDOW_MINIMIZED;           // The window was restored
}

#if !defined(PLATFORM_WEB)
// GLFW3 WindowMaximize Callback, runs when window is maximized/restored
static void WindowMaximizeCallback(GLFWwindow *window, int maximized)
{
    if (maximized) CORE.Window.flags |= FLAG_WINDOW_MAXIMIZED;  // The window was maximized
    else CORE.Window.flags &= ~FLAG_WINDOW_MAXIMIZED;           // The window was restored
}
#endif

// GLFW3 WindowFocus Callback, runs when window get/lose focus
static void WindowFocusCallback(GLFWwindow *window, int focused)
{
    if (focused) CORE.Window.flags &= ~FLAG_WINDOW_UNFOCUSED;   // The window was focused
    else CORE.Window.flags |= FLAG_WINDOW_UNFOCUSED;            // The window lost focus
}

// GLFW3 Keyboard Callback, runs on key pressed
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key < 0) return;    // Security check, macOS fn key generates -1

    // WARNING: GLFW could return GLFW_REPEAT, we need to consider it as 1
    // to work properly with our implementation (IsKeyDown/IsKeyUp checks)
    if (action == GLFW_RELEASE) CORE.Input.Keyboard.currentKeyState[key] = 0;
    else if(action == GLFW_PRESS) CORE.Input.Keyboard.currentKeyState[key] = 1;
    else if(action == GLFW_REPEAT) CORE.Input.Keyboard.keyRepeatInFrame[key] = 1;

#if !defined(PLATFORM_WEB)
    // WARNING: Check if CAPS/NUM key modifiers are enabled and force down state for those keys
    if (((key == KEY_CAPS_LOCK) && ((mods & GLFW_MOD_CAPS_LOCK) > 0)) ||
        ((key == KEY_NUM_LOCK) && ((mods & GLFW_MOD_NUM_LOCK) > 0))) CORE.Input.Keyboard.currentKeyState[key] = 1;
#endif

    // Check if there is space available in the key queue
    if ((CORE.Input.Keyboard.keyPressedQueueCount < MAX_KEY_PRESSED_QUEUE) && (action == GLFW_PRESS))
    {
        // Add character to the queue
        CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = key;
        CORE.Input.Keyboard.keyPressedQueueCount++;
    }

    // Check the exit key to set close window
    if ((key == CORE.Input.Keyboard.exitKey) && (action == GLFW_PRESS)) glfwSetWindowShouldClose(CORE.Window.handle, GLFW_TRUE);

#if defined(SUPPORT_SCREEN_CAPTURE)
    if ((key == GLFW_KEY_F12) && (action == GLFW_PRESS))
    {
#if defined(SUPPORT_GIF_RECORDING)
        if (mods & GLFW_MOD_CONTROL)
        {
            if (gifRecording)
            {
                gifRecording = false;

                MsfGifResult result = msf_gif_end(&gifState);

                SaveFileData(TextFormat("%s/screenrec%03i.gif", CORE.Storage.basePath, screenshotCounter), result.data, (unsigned int)result.dataSize);
                msf_gif_free(result);

            #if defined(PLATFORM_WEB)
                // Download file from MEMFS (emscripten memory filesystem)
                // saveFileFromMEMFSToDisk() function is defined in raylib/templates/web_shel/shell.html
                emscripten_run_script(TextFormat("saveFileFromMEMFSToDisk('%s','%s')", TextFormat("screenrec%03i.gif", screenshotCounter - 1), TextFormat("screenrec%03i.gif", screenshotCounter - 1)));
            #endif

                TRACELOG(LOG_INFO, "SYSTEM: Finish animated GIF recording");
            }
            else
            {
                gifRecording = true;
                gifFrameCounter = 0;

                Vector2 scale = GetWindowScaleDPI();
                msf_gif_begin(&gifState, (int)((float)CORE.Window.render.width*scale.x), (int)((float)CORE.Window.render.height*scale.y));
                screenshotCounter++;

                TRACELOG(LOG_INFO, "SYSTEM: Start animated GIF recording: %s", TextFormat("screenrec%03i.gif", screenshotCounter));
            }
        }
        else
#endif  // SUPPORT_GIF_RECORDING
        {
            TakeScreenshot(TextFormat("screenshot%03i.png", screenshotCounter));
            screenshotCounter++;
        }
    }
#endif  // SUPPORT_SCREEN_CAPTURE

#if defined(SUPPORT_EVENTS_AUTOMATION)
    if ((key == GLFW_KEY_F11) && (action == GLFW_PRESS))
    {
        eventsRecording = !eventsRecording;

        // On finish recording, we export events into a file
        if (!eventsRecording) ExportAutomationEvents("eventsrec.rep");
    }
    else if ((key == GLFW_KEY_F9) && (action == GLFW_PRESS))
    {
        LoadAutomationEvents("eventsrec.rep");
        eventsPlaying = true;

        TRACELOG(LOG_WARNING, "eventsPlaying enabled!");
    }
#endif
}

// GLFW3 Char Key Callback, runs on key down (gets equivalent unicode char value)
static void CharCallback(GLFWwindow *window, unsigned int key)
{
    //TRACELOG(LOG_DEBUG, "Char Callback: KEY:%i(%c)", key, key);

    // NOTE: Registers any key down considering OS keyboard layout but
    // does not detect action events, those should be managed by user...
    // Ref: https://github.com/glfw/glfw/issues/668#issuecomment-166794907
    // Ref: https://www.glfw.org/docs/latest/input_guide.html#input_char

    // Check if there is space available in the queue
    if (CORE.Input.Keyboard.charPressedQueueCount < MAX_CHAR_PRESSED_QUEUE)
    {
        // Add character to the queue
        CORE.Input.Keyboard.charPressedQueue[CORE.Input.Keyboard.charPressedQueueCount] = key;
        CORE.Input.Keyboard.charPressedQueueCount++;
    }
}

// GLFW3 Mouse Button Callback, runs on mouse button pressed
static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    // WARNING: GLFW could only return GLFW_PRESS (1) or GLFW_RELEASE (0) for now,
    // but future releases may add more actions (i.e. GLFW_REPEAT)
    CORE.Input.Mouse.currentButtonState[button] = action;

#if defined(SUPPORT_GESTURES_SYSTEM) && defined(SUPPORT_MOUSE_GESTURES)         // PLATFORM_DESKTOP
    // Process mouse events as touches to be able to use mouse-gestures
    GestureEvent gestureEvent = { 0 };

    // Register touch actions
    if ((CORE.Input.Mouse.currentButtonState[button] == 1) && (CORE.Input.Mouse.previousButtonState[button] == 0)) gestureEvent.touchAction = TOUCH_ACTION_DOWN;
    else if ((CORE.Input.Mouse.currentButtonState[button] == 0) && (CORE.Input.Mouse.previousButtonState[button] == 1)) gestureEvent.touchAction = TOUCH_ACTION_UP;

    // NOTE: TOUCH_ACTION_MOVE event is registered in MouseCursorPosCallback()

    // Assign a pointer ID
    gestureEvent.pointId[0] = 0;

    // Register touch points count
    gestureEvent.pointCount = 1;

    // Register touch points position, only one point registered
    gestureEvent.position[0] = GetMousePosition();

    // Normalize gestureEvent.position[0] for CORE.Window.screen.width and CORE.Window.screen.height
    gestureEvent.position[0].x /= (float)GetScreenWidth();
    gestureEvent.position[0].y /= (float)GetScreenHeight();

    // Gesture data is sent to gestures-system for processing
#if defined(PLATFORM_WEB)
    // Prevent calling ProcessGestureEvent() when Emscripten is present and there's a touch gesture, so EmscriptenTouchCallback() can handle it itself
    if (GetMouseX() != 0 || GetMouseY() != 0) ProcessGestureEvent(gestureEvent);
#else
    ProcessGestureEvent(gestureEvent);
#endif

#endif
}

// GLFW3 Cursor Position Callback, runs on mouse move
static void MouseCursorPosCallback(GLFWwindow *window, double x, double y)
{
    CORE.Input.Mouse.currentPosition.x = (float)x;
    CORE.Input.Mouse.currentPosition.y = (float)y;
    CORE.Input.Touch.position[0] = CORE.Input.Mouse.currentPosition;

#if defined(SUPPORT_GESTURES_SYSTEM) && defined(SUPPORT_MOUSE_GESTURES)         // PLATFORM_DESKTOP
    // Process mouse events as touches to be able to use mouse-gestures
    GestureEvent gestureEvent = { 0 };

    gestureEvent.touchAction = TOUCH_ACTION_MOVE;

    // Assign a pointer ID
    gestureEvent.pointId[0] = 0;

    // Register touch points count
    gestureEvent.pointCount = 1;

    // Register touch points position, only one point registered
    gestureEvent.position[0] = CORE.Input.Touch.position[0];

    // Normalize gestureEvent.position[0] for CORE.Window.screen.width and CORE.Window.screen.height
    gestureEvent.position[0].x /= (float)GetScreenWidth();
    gestureEvent.position[0].y /= (float)GetScreenHeight();

    // Gesture data is sent to gestures-system for processing
    ProcessGestureEvent(gestureEvent);
#endif
}

// GLFW3 Scrolling Callback, runs on mouse wheel
static void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    CORE.Input.Mouse.currentWheelMove = (Vector2){ (float)xoffset, (float)yoffset };
}

// GLFW3 CursorEnter Callback, when cursor enters the window
static void CursorEnterCallback(GLFWwindow *window, int enter)
{
    if (enter == true) CORE.Input.Mouse.cursorOnScreen = true;
    else CORE.Input.Mouse.cursorOnScreen = false;
}

// GLFW3 Window Drop Callback, runs when drop files into window
static void WindowDropCallback(GLFWwindow *window, int count, const char **paths)
{
    if (count > 0)
    {
        // In case previous dropped filepaths have not been freed, we free them
        if (CORE.Window.dropFileCount > 0)
        {
            for (unsigned int i = 0; i < CORE.Window.dropFileCount; i++) RL_FREE(CORE.Window.dropFilepaths[i]);

            RL_FREE(CORE.Window.dropFilepaths);

            CORE.Window.dropFileCount = 0;
            CORE.Window.dropFilepaths = NULL;
        }

        // WARNING: Paths are freed by GLFW when the callback returns, we must keep an internal copy
        CORE.Window.dropFileCount = count;
        CORE.Window.dropFilepaths = (char **)RL_CALLOC(CORE.Window.dropFileCount, sizeof(char *));

        for (unsigned int i = 0; i < CORE.Window.dropFileCount; i++)
        {
            CORE.Window.dropFilepaths[i] = (char *)RL_CALLOC(MAX_FILEPATH_LENGTH, sizeof(char));
            strcpy(CORE.Window.dropFilepaths[i], paths[i]);
        }
    }
}
#endif

#if defined(PLATFORM_DRM)
// Initialize Keyboard system (using standard input)
static void InitKeyboard(void)
{
    // NOTE: We read directly from Standard Input (stdin) - STDIN_FILENO file descriptor,
    // Reading directly from stdin will give chars already key-mapped by kernel to ASCII or UNICODE

    // Save terminal keyboard settings
    tcgetattr(STDIN_FILENO, &CORE.Input.Keyboard.defaultSettings);

    // Reconfigure terminal with new settings
    struct termios keyboardNewSettings = { 0 };
    keyboardNewSettings = CORE.Input.Keyboard.defaultSettings;

    // New terminal settings for keyboard: turn off buffering (non-canonical mode), echo and key processing
    // NOTE: ISIG controls if ^C and ^Z generate break signals or not
    keyboardNewSettings.c_lflag &= ~(ICANON | ECHO | ISIG);
    //keyboardNewSettings.c_iflag &= ~(ISTRIP | INLCR | ICRNL | IGNCR | IXON | IXOFF);
    keyboardNewSettings.c_cc[VMIN] = 1;
    keyboardNewSettings.c_cc[VTIME] = 0;

    // Set new keyboard settings (change occurs immediately)
    tcsetattr(STDIN_FILENO, TCSANOW, &keyboardNewSettings);

    // Save old keyboard mode to restore it at the end
    CORE.Input.Keyboard.defaultFileFlags = fcntl(STDIN_FILENO, F_GETFL, 0);          // F_GETFL: Get the file access mode and the file status flags
    fcntl(STDIN_FILENO, F_SETFL, CORE.Input.Keyboard.defaultFileFlags | O_NONBLOCK); // F_SETFL: Set the file status flags to the value specified

    // NOTE: If ioctl() returns -1, it means the call failed for some reason (error code set in errno)
    int result = ioctl(STDIN_FILENO, KDGKBMODE, &CORE.Input.Keyboard.defaultMode);

    // In case of failure, it could mean a remote keyboard is used (SSH)
    if (result < 0) TRACELOG(LOG_WARNING, "RPI: Failed to change keyboard mode, an SSH keyboard is probably used");
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
    tcsetattr(STDIN_FILENO, TCSANOW, &CORE.Input.Keyboard.defaultSettings);

    // Reconfigure keyboard to default mode
    fcntl(STDIN_FILENO, F_SETFL, CORE.Input.Keyboard.defaultFileFlags);
    ioctl(STDIN_FILENO, KDSKBMODE, CORE.Input.Keyboard.defaultMode);
}

#if defined(SUPPORT_SSH_KEYBOARD_RPI)
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

    // Check exit key (same functionality as GLFW3 KeyCallback())
    if (CORE.Input.Keyboard.currentKeyState[CORE.Input.Keyboard.exitKey] == 1) CORE.Window.shouldClose = true;

#if defined(SUPPORT_SCREEN_CAPTURE)
    // Check screen capture key (raylib key: KEY_F12)
    if (CORE.Input.Keyboard.currentKeyState[301] == 1)
    {
        TakeScreenshot(TextFormat("screenshot%03i.png", screenshotCounter));
        screenshotCounter++;
    }
#endif
}
#endif  // SUPPORT_SSH_KEYBOARD_RPI

// Initialise user input from evdev(/dev/input/event<N>) this means mouse, keyboard or gamepad devices
static void InitEvdevInput(void)
{
    char path[MAX_FILEPATH_LENGTH] = { 0 };
    DIR *directory = NULL;
    struct dirent *entity = NULL;

    // Initialise keyboard file descriptor
    CORE.Input.Keyboard.fd = -1;

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
    else TRACELOG(LOG_WARNING, "RPI: Failed to open linux event directory: %s", DEFAULT_EVDEV_PATH);
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

    struct input_absinfo absinfo = { 0 };
    unsigned long evBits[NBITS(EV_MAX)] = { 0 };
    unsigned long absBits[NBITS(ABS_MAX)] = { 0 };
    unsigned long relBits[NBITS(REL_MAX)] = { 0 };
    unsigned long keyBits[NBITS(KEY_MAX)] = { 0 };
    bool hasAbs = false;
    bool hasRel = false;
    bool hasAbsMulti = false;
    int freeWorkerId = -1;
    int fd = -1;

    InputEventWorker *worker = NULL;

    // Open the device and allocate worker
    //-------------------------------------------------------------------------------------------------------
    // Find a free spot in the workers array
    for (int i = 0; i < sizeof(CORE.Input.eventWorker)/sizeof(InputEventWorker); ++i)
    {
        if (CORE.Input.eventWorker[i].threadId == 0)
        {
            freeWorkerId = i;
            break;
        }
    }

    // Select the free worker from array
    if (freeWorkerId >= 0)
    {
        worker = &(CORE.Input.eventWorker[freeWorkerId]);       // Grab a pointer to the worker
        memset(worker, 0, sizeof(InputEventWorker));  // Clear the worker
    }
    else
    {
        TRACELOG(LOG_WARNING, "RPI: Failed to create input device thread for %s, out of worker slots", device);
        return;
    }

    // Open the device
    fd = open(device, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        TRACELOG(LOG_WARNING, "RPI: Failed to open input device: %s", device);
        return;
    }
    worker->fd = fd;

    // Grab number on the end of the devices name "event<N>"
    int devNum = 0;
    char *ptrDevName = strrchr(device, 't');
    worker->eventNum = -1;

    if (ptrDevName != NULL)
    {
        if (sscanf(ptrDevName, "t%d", &devNum) == 1) worker->eventNum = devNum;
    }
    else worker->eventNum = 0;      // TODO: HACK: Grab number for mouse0 device!

    // At this point we have a connection to the device, but we don't yet know what the device is.
    // It could be many things, even as simple as a power button...
    //-------------------------------------------------------------------------------------------------------

    // Identify the device
    //-------------------------------------------------------------------------------------------------------
    ioctl(fd, EVIOCGBIT(0, sizeof(evBits)), evBits);    // Read a bitfield of the available device properties

    // Check for absolute input devices
    if (TEST_BIT(evBits, EV_ABS))
    {
        ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absBits)), absBits);

        // Check for absolute movement support (usually touchscreens, but also joysticks)
        if (TEST_BIT(absBits, ABS_X) && TEST_BIT(absBits, ABS_Y))
        {
            hasAbs = true;

            // Get the scaling values
            ioctl(fd, EVIOCGABS(ABS_X), &absinfo);
            worker->absRange.x = absinfo.minimum;
            worker->absRange.width = absinfo.maximum - absinfo.minimum;
            ioctl(fd, EVIOCGABS(ABS_Y), &absinfo);
            worker->absRange.y = absinfo.minimum;
            worker->absRange.height = absinfo.maximum - absinfo.minimum;
        }

        // Check for multiple absolute movement support (usually multitouch touchscreens)
        if (TEST_BIT(absBits, ABS_MT_POSITION_X) && TEST_BIT(absBits, ABS_MT_POSITION_Y))
        {
            hasAbsMulti = true;

            // Get the scaling values
            ioctl(fd, EVIOCGABS(ABS_X), &absinfo);
            worker->absRange.x = absinfo.minimum;
            worker->absRange.width = absinfo.maximum - absinfo.minimum;
            ioctl(fd, EVIOCGABS(ABS_Y), &absinfo);
            worker->absRange.y = absinfo.minimum;
            worker->absRange.height = absinfo.maximum - absinfo.minimum;
        }
    }

    // Check for relative movement support (usually mouse)
    if (TEST_BIT(evBits, EV_REL))
    {
        ioctl(fd, EVIOCGBIT(EV_REL, sizeof(relBits)), relBits);

        if (TEST_BIT(relBits, REL_X) && TEST_BIT(relBits, REL_Y)) hasRel = true;
    }

    // Check for button support to determine the device type(usually on all input devices)
    if (TEST_BIT(evBits, EV_KEY))
    {
        ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyBits)), keyBits);

        if (hasAbs || hasAbsMulti)
        {
            if (TEST_BIT(keyBits, BTN_TOUCH)) worker->isTouch = true;          // This is a touchscreen
            if (TEST_BIT(keyBits, BTN_TOOL_FINGER)) worker->isTouch = true;    // This is a drawing tablet
            if (TEST_BIT(keyBits, BTN_TOOL_PEN)) worker->isTouch = true;       // This is a drawing tablet
            if (TEST_BIT(keyBits, BTN_STYLUS)) worker->isTouch = true;         // This is a drawing tablet
            if (worker->isTouch || hasAbsMulti) worker->isMultitouch = true;   // This is a multitouch capable device
        }

        if (hasRel)
        {
            if (TEST_BIT(keyBits, BTN_LEFT)) worker->isMouse = true;           // This is a mouse
            if (TEST_BIT(keyBits, BTN_RIGHT)) worker->isMouse = true;          // This is a mouse
        }

        if (TEST_BIT(keyBits, BTN_A)) worker->isGamepad = true;                // This is a gamepad
        if (TEST_BIT(keyBits, BTN_TRIGGER)) worker->isGamepad = true;          // This is a gamepad
        if (TEST_BIT(keyBits, BTN_START)) worker->isGamepad = true;            // This is a gamepad
        if (TEST_BIT(keyBits, BTN_TL)) worker->isGamepad = true;               // This is a gamepad
        if (TEST_BIT(keyBits, BTN_TL)) worker->isGamepad = true;               // This is a gamepad

        if (TEST_BIT(keyBits, KEY_SPACE)) worker->isKeyboard = true;           // This is a keyboard
    }
    //-------------------------------------------------------------------------------------------------------

    // Decide what to do with the device
    //-------------------------------------------------------------------------------------------------------
    if (worker->isKeyboard && (CORE.Input.Keyboard.fd == -1))
    {
        // Use the first keyboard encountered. This assumes that a device that says it's a keyboard is just a
        // keyboard. The keyboard is polled synchronously, whereas other input devices are polled in separate
        // threads so that they don't drop events when the frame rate is slow.
        TRACELOG(LOG_INFO, "RPI: Opening keyboard device: %s", device);
        CORE.Input.Keyboard.fd = worker->fd;
    }
    else if (worker->isTouch || worker->isMouse)
    {
        // Looks like an interesting device
        TRACELOG(LOG_INFO, "RPI: Opening input device: %s (%s%s%s%s)", device,
            worker->isMouse? "mouse " : "",
            worker->isMultitouch? "multitouch " : "",
            worker->isTouch? "touchscreen " : "",
            worker->isGamepad? "gamepad " : "");

        // Create a thread for this device
        int error = pthread_create(&worker->threadId, NULL, &EventThread, (void *)worker);
        if (error != 0)
        {
            TRACELOG(LOG_WARNING, "RPI: Failed to create input device thread: %s (error: %d)", device, error);
            worker->threadId = 0;
            close(fd);
        }

#if defined(USE_LAST_TOUCH_DEVICE)
        // Find touchscreen with the highest index
        int maxTouchNumber = -1;

        for (int i = 0; i < sizeof(CORE.Input.eventWorker)/sizeof(InputEventWorker); ++i)
        {
            if (CORE.Input.eventWorker[i].isTouch && (CORE.Input.eventWorker[i].eventNum > maxTouchNumber)) maxTouchNumber = CORE.Input.eventWorker[i].eventNum;
        }

        // Find touchscreens with lower indexes
        for (int i = 0; i < sizeof(CORE.Input.eventWorker)/sizeof(InputEventWorker); ++i)
        {
            if (CORE.Input.eventWorker[i].isTouch && (CORE.Input.eventWorker[i].eventNum < maxTouchNumber))
            {
                if (CORE.Input.eventWorker[i].threadId != 0)
                {
                    TRACELOG(LOG_WARNING, "RPI: Found duplicate touchscreen, killing touchscreen on event: %d", i);
                    pthread_cancel(CORE.Input.eventWorker[i].threadId);
                    close(CORE.Input.eventWorker[i].fd);
                }
            }
        }
#endif
    }
    else close(fd);  // We are not interested in this device
    //-------------------------------------------------------------------------------------------------------
}

static void PollKeyboardEvents(void)
{
    // Scancode to keycode mapping for US keyboards
    // TODO: Replace this with a keymap from the X11 to get the correct regional map for the keyboard:
    // Currently non US keyboards will have the wrong mapping for some keys
    static const int keymapUS[] = {
        0, 256, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 45, 61, 259, 258, 81, 87, 69, 82, 84,
        89, 85, 73, 79, 80, 91, 93, 257, 341, 65, 83, 68, 70, 71, 72, 74, 75, 76, 59, 39, 96,
        340, 92, 90, 88, 67, 86, 66, 78, 77, 44, 46, 47, 344, 332, 342, 32, 280, 290, 291,
        292, 293, 294, 295, 296, 297, 298, 299, 282, 281, 327, 328, 329, 333, 324, 325,
        326, 334, 321, 322, 323, 320, 330, 0, 85, 86, 300, 301, 89, 90, 91, 92, 93, 94, 95,
        335, 345, 331, 283, 346, 101, 268, 265, 266, 263, 262, 269, 264, 267, 260, 261,
        112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 347, 127,
        128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
        144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
        160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
        176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
        192, 193, 194, 0, 0, 0, 0, 0, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210,
        211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226,
        227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242,
        243, 244, 245, 246, 247, 248, 0, 0, 0, 0, 0, 0, 0
    };

    int fd = CORE.Input.Keyboard.fd;
    if (fd == -1) return;

    struct input_event event = { 0 };
    int keycode = -1;

    // Try to read data from the keyboard and only continue if successful
    while (read(fd, &event, sizeof(event)) == (int)sizeof(event))
    {
        // Button parsing
        if (event.type == EV_KEY)
        {
#if defined(SUPPORT_SSH_KEYBOARD_RPI)
            // Change keyboard mode to events
            CORE.Input.Keyboard.evtMode = true;
#endif
            // Keyboard button parsing
            if ((event.code >= 1) && (event.code <= 255))     //Keyboard keys appear for codes 1 to 255
            {
                keycode = keymapUS[event.code & 0xFF];     // The code we get is a scancode so we look up the appropriate keycode

                // Make sure we got a valid keycode
                if ((keycode > 0) && (keycode < sizeof(CORE.Input.Keyboard.currentKeyState)))
                {
                    // WARNING: https://www.kernel.org/doc/Documentation/input/input.txt
                    // Event interface: 'value' is the value the event carries. Either a relative change for EV_REL,
                    // absolute new value for EV_ABS (joysticks ...), or 0 for EV_KEY for release, 1 for keypress and 2 for autorepeat
                    CORE.Input.Keyboard.currentKeyState[keycode] = (event.value >= 1)? 1 : 0;
                    if (event.value >= 1)
                    {
                        CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = keycode;     // Register last key pressed
                        CORE.Input.Keyboard.keyPressedQueueCount++;
                    }

                #if defined(SUPPORT_SCREEN_CAPTURE)
                    // Check screen capture key (raylib key: KEY_F12)
                    if (CORE.Input.Keyboard.currentKeyState[301] == 1)
                    {
                        TakeScreenshot(TextFormat("screenshot%03i.png", screenshotCounter));
                        screenshotCounter++;
                    }
                #endif

                    if (CORE.Input.Keyboard.currentKeyState[CORE.Input.Keyboard.exitKey] == 1) CORE.Window.shouldClose = true;

                    TRACELOGD("RPI: KEY_%s ScanCode: %4i KeyCode: %4i", event.value == 0 ? "UP":"DOWN", event.code, keycode);
                }
            }
        }
    }
}

// Input device events reading thread
static void *EventThread(void *arg)
{
    struct input_event event = { 0 };
    InputEventWorker *worker = (InputEventWorker *)arg;

    int touchAction = -1;           // 0-TOUCH_ACTION_UP, 1-TOUCH_ACTION_DOWN, 2-TOUCH_ACTION_MOVE
    bool gestureUpdate = false;     // Flag to note gestures require to update

    while (!CORE.Window.shouldClose)
    {
        // Try to read data from the device and only continue if successful
        while (read(worker->fd, &event, sizeof(event)) == (int)sizeof(event))
        {
            // Relative movement parsing
            if (event.type == EV_REL)
            {
                if (event.code == REL_X)
                {
                    CORE.Input.Mouse.currentPosition.x += event.value;
                    CORE.Input.Touch.position[0].x = CORE.Input.Mouse.currentPosition.x;

                    touchAction = 2;    // TOUCH_ACTION_MOVE
                    gestureUpdate = true;
                }

                if (event.code == REL_Y)
                {
                    CORE.Input.Mouse.currentPosition.y += event.value;
                    CORE.Input.Touch.position[0].y = CORE.Input.Mouse.currentPosition.y;

                    touchAction = 2;    // TOUCH_ACTION_MOVE
                    gestureUpdate = true;
                }

                if (event.code == REL_WHEEL) CORE.Input.Mouse.eventWheelMove.y += event.value;
            }

            // Absolute movement parsing
            if (event.type == EV_ABS)
            {
                // Basic movement
                if (event.code == ABS_X)
                {
                    CORE.Input.Mouse.currentPosition.x = (event.value - worker->absRange.x)*CORE.Window.screen.width/worker->absRange.width;    // Scale according to absRange
                    CORE.Input.Touch.position[0].x = (event.value - worker->absRange.x)*CORE.Window.screen.width/worker->absRange.width;        // Scale according to absRange

                    touchAction = 2;    // TOUCH_ACTION_MOVE
                    gestureUpdate = true;
                }

                if (event.code == ABS_Y)
                {
                    CORE.Input.Mouse.currentPosition.y = (event.value - worker->absRange.y)*CORE.Window.screen.height/worker->absRange.height;  // Scale according to absRange
                    CORE.Input.Touch.position[0].y = (event.value - worker->absRange.y)*CORE.Window.screen.height/worker->absRange.height;      // Scale according to absRange

                    touchAction = 2;    // TOUCH_ACTION_MOVE
                    gestureUpdate = true;
                }

                // Multitouch movement
                if (event.code == ABS_MT_SLOT) worker->touchSlot = event.value;   // Remember the slot number for the folowing events

                if (event.code == ABS_MT_POSITION_X)
                {
                    if (worker->touchSlot < MAX_TOUCH_POINTS) CORE.Input.Touch.position[worker->touchSlot].x = (event.value - worker->absRange.x)*CORE.Window.screen.width/worker->absRange.width;    // Scale according to absRange
                }

                if (event.code == ABS_MT_POSITION_Y)
                {
                    if (worker->touchSlot < MAX_TOUCH_POINTS) CORE.Input.Touch.position[worker->touchSlot].y = (event.value - worker->absRange.y)*CORE.Window.screen.height/worker->absRange.height;  // Scale according to absRange
                }

                if (event.code == ABS_MT_TRACKING_ID)
                {
                    if ((event.value < 0) && (worker->touchSlot < MAX_TOUCH_POINTS))
                    {
                        // Touch has ended for this point
                        CORE.Input.Touch.position[worker->touchSlot].x = -1;
                        CORE.Input.Touch.position[worker->touchSlot].y = -1;
                    }
                }

                // Touchscreen tap
                if (event.code == ABS_PRESSURE)
                {
                    int previousMouseLeftButtonState = CORE.Input.Mouse.currentButtonStateEvdev[MOUSE_BUTTON_LEFT];

                    if (!event.value && previousMouseLeftButtonState)
                    {
                        CORE.Input.Mouse.currentButtonStateEvdev[MOUSE_BUTTON_LEFT] = 0;

                        touchAction = 0;    // TOUCH_ACTION_UP
                        gestureUpdate = true;
                    }

                    if (event.value && !previousMouseLeftButtonState)
                    {
                        CORE.Input.Mouse.currentButtonStateEvdev[MOUSE_BUTTON_LEFT] = 1;

                        touchAction = 1;    // TOUCH_ACTION_DOWN
                        gestureUpdate = true;
                    }
                }

            }

            // Button parsing
            if (event.type == EV_KEY)
            {
                // Mouse button parsing
                if ((event.code == BTN_TOUCH) || (event.code == BTN_LEFT))
                {
                    CORE.Input.Mouse.currentButtonStateEvdev[MOUSE_BUTTON_LEFT] = event.value;

                    if (event.value > 0) touchAction = 1;   // TOUCH_ACTION_DOWN
                    else touchAction = 0;       // TOUCH_ACTION_UP
                    gestureUpdate = true;
                }

                if (event.code == BTN_RIGHT) CORE.Input.Mouse.currentButtonStateEvdev[MOUSE_BUTTON_RIGHT] = event.value;
                if (event.code == BTN_MIDDLE) CORE.Input.Mouse.currentButtonStateEvdev[MOUSE_BUTTON_MIDDLE] = event.value;
                if (event.code == BTN_SIDE) CORE.Input.Mouse.currentButtonStateEvdev[MOUSE_BUTTON_SIDE] = event.value;
                if (event.code == BTN_EXTRA) CORE.Input.Mouse.currentButtonStateEvdev[MOUSE_BUTTON_EXTRA] = event.value;
                if (event.code == BTN_FORWARD) CORE.Input.Mouse.currentButtonStateEvdev[MOUSE_BUTTON_FORWARD] = event.value;
                if (event.code == BTN_BACK) CORE.Input.Mouse.currentButtonStateEvdev[MOUSE_BUTTON_BACK] = event.value;
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
            if (CORE.Input.Touch.position[0].x >= 0) CORE.Input.Touch.pointCount++;
            if (CORE.Input.Touch.position[1].x >= 0) CORE.Input.Touch.pointCount++;
            if (CORE.Input.Touch.position[2].x >= 0) CORE.Input.Touch.pointCount++;
            if (CORE.Input.Touch.position[3].x >= 0) CORE.Input.Touch.pointCount++;

#if defined(SUPPORT_GESTURES_SYSTEM)        // PLATFORM_DRM
            if (gestureUpdate)
            {
                GestureEvent gestureEvent = { 0 };

                gestureEvent.touchAction = touchAction;
                gestureEvent.pointCount = CORE.Input.Touch.pointCount;

                gestureEvent.pointId[0] = 0;
                gestureEvent.pointId[1] = 1;
                gestureEvent.pointId[2] = 2;
                gestureEvent.pointId[3] = 3;

                gestureEvent.position[0] = CORE.Input.Touch.position[0];
                gestureEvent.position[1] = CORE.Input.Touch.position[1];
                gestureEvent.position[2] = CORE.Input.Touch.position[2];
                gestureEvent.position[3] = CORE.Input.Touch.position[3];

                ProcessGestureEvent(gestureEvent);
            }
#endif
        }

        WaitTime(0.005);    // Sleep for 5ms to avoid hogging CPU time
    }

    close(worker->fd);

    return NULL;
}

// Initialize gamepad system
static void InitGamepad(void)
{
    char gamepadDev[128] = { 0 };

    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        sprintf(gamepadDev, "%s%i", DEFAULT_GAMEPAD_DEV, i);

        if ((CORE.Input.Gamepad.streamId[i] = open(gamepadDev, O_RDONLY | O_NONBLOCK)) < 0)
        {
            // NOTE: Only show message for first gamepad
            if (i == 0) TRACELOG(LOG_WARNING, "RPI: Failed to open Gamepad device, no gamepad available");
        }
        else
        {
            CORE.Input.Gamepad.ready[i] = true;

            // NOTE: Only create one thread
            if (i == 0)
            {
                int error = pthread_create(&CORE.Input.Gamepad.threadId, NULL, &GamepadThread, NULL);

                if (error != 0) TRACELOG(LOG_WARNING, "RPI: Failed to create gamepad input event thread");
                else  TRACELOG(LOG_INFO, "RPI: Gamepad device initialized successfully");
            }
        }
    }
}

// Process Gamepad (/dev/input/js0)
static void *GamepadThread(void *arg)
{
    #define JS_EVENT_BUTTON         0x01    // Button pressed/released
    #define JS_EVENT_AXIS           0x02    // Joystick axis moved
    #define JS_EVENT_INIT           0x80    // Initial state of device

    struct js_event {
        unsigned int time;      // event timestamp in milliseconds
        short value;            // event value
        unsigned char type;     // event type
        unsigned char number;   // event axis/button number
    };

    // Read gamepad event
    struct js_event gamepadEvent = { 0 };

    while (!CORE.Window.shouldClose)
    {
        for (int i = 0; i < MAX_GAMEPADS; i++)
        {
            if (read(CORE.Input.Gamepad.streamId[i], &gamepadEvent, sizeof(struct js_event)) == (int)sizeof(struct js_event))
            {
                gamepadEvent.type &= ~JS_EVENT_INIT;     // Ignore synthetic events

                // Process gamepad events by type
                if (gamepadEvent.type == JS_EVENT_BUTTON)
                {
                    //TRACELOG(LOG_WARNING, "RPI: Gamepad button: %i, value: %i", gamepadEvent.number, gamepadEvent.value);

                    if (gamepadEvent.number < MAX_GAMEPAD_BUTTONS)
                    {
                        // 1 - button pressed, 0 - button released
                        CORE.Input.Gamepad.currentButtonState[i][gamepadEvent.number] = (int)gamepadEvent.value;

                        if ((int)gamepadEvent.value == 1) CORE.Input.Gamepad.lastButtonPressed = gamepadEvent.number;
                        else CORE.Input.Gamepad.lastButtonPressed = 0;       // GAMEPAD_BUTTON_UNKNOWN
                    }
                }
                else if (gamepadEvent.type == JS_EVENT_AXIS)
                {
                    //TRACELOG(LOG_WARNING, "RPI: Gamepad axis: %i, value: %i", gamepadEvent.number, gamepadEvent.value);

                    if (gamepadEvent.number < MAX_GAMEPAD_AXIS)
                    {
                        // NOTE: Scaling of gamepadEvent.value to get values between -1..1
                        CORE.Input.Gamepad.axisState[i][gamepadEvent.number] = (float)gamepadEvent.value/32768;
                    }
                }
            }
            else WaitTime(0.001);    // Sleep for 1 ms to avoid hogging CPU time
        }
    }

    return NULL;
}
#endif  // PLATFORM_DRM

#if defined(PLATFORM_DRM)
// Search matching DRM mode in connector's mode list
static int FindMatchingConnectorMode(const drmModeConnector *connector, const drmModeModeInfo *mode)
{
    if (NULL == connector) return -1;
    if (NULL == mode) return -1;

    // safe bitwise comparison of two modes
    #define BINCMP(a, b) memcmp((a), (b), (sizeof(a) < sizeof(b)) ? sizeof(a) : sizeof(b))

    for (size_t i = 0; i < connector->count_modes; i++)
    {
        TRACELOG(LOG_TRACE, "DISPLAY: DRM mode: %d %ux%u@%u %s", i, connector->modes[i].hdisplay, connector->modes[i].vdisplay,
            connector->modes[i].vrefresh, (connector->modes[i].flags & DRM_MODE_FLAG_INTERLACE) ? "interlaced" : "progressive");

        if (0 == BINCMP(&CORE.Window.crtc->mode, &CORE.Window.connector->modes[i])) return i;
    }

    return -1;

    #undef BINCMP
}

// Search exactly matching DRM connector mode in connector's list
static int FindExactConnectorMode(const drmModeConnector *connector, uint width, uint height, uint fps, bool allowInterlaced)
{
    TRACELOG(LOG_TRACE, "DISPLAY: Searching exact connector mode for %ux%u@%u, selecting an interlaced mode is allowed: %s", width, height, fps, allowInterlaced ? "yes" : "no");

    if (NULL == connector) return -1;

    for (int i = 0; i < CORE.Window.connector->count_modes; i++)
    {
        const drmModeModeInfo *const mode = &CORE.Window.connector->modes[i];

        TRACELOG(LOG_TRACE, "DISPLAY: DRM Mode %d %ux%u@%u %s", i, mode->hdisplay, mode->vdisplay, mode->vrefresh, (mode->flags & DRM_MODE_FLAG_INTERLACE) ? "interlaced" : "progressive");

        if ((mode->flags & DRM_MODE_FLAG_INTERLACE) && (!allowInterlaced)) continue;

        if ((mode->hdisplay == width) && (mode->vdisplay == height) && (mode->vrefresh == fps)) return i;
    }

    TRACELOG(LOG_TRACE, "DISPLAY: No DRM exact matching mode found");
    return -1;
}

// Search the nearest matching DRM connector mode in connector's list
static int FindNearestConnectorMode(const drmModeConnector *connector, uint width, uint height, uint fps, bool allowInterlaced)
{
    TRACELOG(LOG_TRACE, "DISPLAY: Searching nearest connector mode for %ux%u@%u, selecting an interlaced mode is allowed: %s", width, height, fps, allowInterlaced ? "yes" : "no");

    if (NULL == connector) return -1;

    int nearestIndex = -1;
    for (int i = 0; i < CORE.Window.connector->count_modes; i++)
    {
        const drmModeModeInfo *const mode = &CORE.Window.connector->modes[i];

        TRACELOG(LOG_TRACE, "DISPLAY: DRM mode: %d %ux%u@%u %s", i, mode->hdisplay, mode->vdisplay, mode->vrefresh,
            (mode->flags & DRM_MODE_FLAG_INTERLACE) ? "interlaced" : "progressive");

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

        const int nearestWidthDiff = abs(CORE.Window.connector->modes[nearestIndex].hdisplay - width);
        const int nearestHeightDiff = abs(CORE.Window.connector->modes[nearestIndex].vdisplay - height);
        const int nearestFpsDiff = abs(CORE.Window.connector->modes[nearestIndex].vrefresh - fps);

        if ((widthDiff < nearestWidthDiff) || (heightDiff < nearestHeightDiff) || (fpsDiff < nearestFpsDiff)) {
            nearestIndex = i;
        }
    }

    return nearestIndex;
}
#endif

#if defined(SUPPORT_EVENTS_AUTOMATION)
// NOTE: Loading happens over AutomationEvent *events
// TODO: This system should probably be redesigned
static void LoadAutomationEvents(const char *fileName)
{
    // Load events file (binary)
    /*
    FILE *repFile = fopen(fileName, "rb");
    unsigned char fileId[4] = { 0 };

    fread(fileId, 1, 4, repFile);

    if ((fileId[0] == 'r') && (fileId[1] == 'E') && (fileId[2] == 'P') && (fileId[1] == ' '))
    {
        fread(&eventCount, sizeof(int), 1, repFile);
        TRACELOG(LOG_WARNING, "Events loaded: %i\n", eventCount);
        fread(events, sizeof(AutomationEvent), eventCount, repFile);
    }

    fclose(repFile);
    */

    // Load events file (text)
    FILE *repFile = fopen(fileName, "rt");

    if (repFile != NULL)
    {
        unsigned int count = 0;
        char buffer[256] = { 0 };

        fgets(buffer, 256, repFile);

        while (!feof(repFile))
        {
            if (buffer[0] == 'c') sscanf(buffer, "c %i", &eventCount);
            else if (buffer[0] == 'e')
            {
                sscanf(buffer, "e %d %d %d %d %d", &events[count].frame, &events[count].type,
                       &events[count].params[0], &events[count].params[1], &events[count].params[2]);

                count++;
            }

            fgets(buffer, 256, repFile);
        }

        if (count != eventCount) TRACELOG(LOG_WARNING, "Events count provided is different than count");

        fclose(repFile);
    }

    TRACELOG(LOG_WARNING, "Events loaded: %i", eventCount);
}

// Export recorded events into a file
static void ExportAutomationEvents(const char *fileName)
{
    unsigned char fileId[4] = "rEP ";

    // Save as binary
    /*
    FILE *repFile = fopen(fileName, "wb");
    fwrite(fileId, sizeof(unsigned char), 4, repFile);
    fwrite(&eventCount, sizeof(int), 1, repFile);
    fwrite(events, sizeof(AutomationEvent), eventCount, repFile);
    fclose(repFile);
    */

    // Export events as text
    FILE *repFile = fopen(fileName, "wt");

    if (repFile != NULL)
    {
        fprintf(repFile, "# Automation events list\n");
        fprintf(repFile, "#    c <events_count>\n");
        fprintf(repFile, "#    e <frame> <event_type> <param0> <param1> <param2> // <event_type_name>\n");

        fprintf(repFile, "c %i\n", eventCount);
        for (int i = 0; i < eventCount; i++)
        {
            fprintf(repFile, "e %i %i %i %i %i // %s\n", events[i].frame, events[i].type,
                    events[i].params[0], events[i].params[1], events[i].params[2], autoEventTypeName[events[i].type]);
        }

        fclose(repFile);
    }
}

// EndDrawing() -> After PollInputEvents()
// Check event in current frame and save into the events[i] array
static void RecordAutomationEvent(unsigned int frame)
{
    for (int key = 0; key < MAX_KEYBOARD_KEYS; key++)
    {
        // INPUT_KEY_UP (only saved once)
        if (CORE.Input.Keyboard.previousKeyState[key] && !CORE.Input.Keyboard.currentKeyState[key])
        {
            events[eventCount].frame = frame;
            events[eventCount].type = INPUT_KEY_UP;
            events[eventCount].params[0] = key;
            events[eventCount].params[1] = 0;
            events[eventCount].params[2] = 0;

            TRACELOG(LOG_INFO, "[%i] INPUT_KEY_UP: %i, %i, %i", events[eventCount].frame, events[eventCount].params[0], events[eventCount].params[1], events[eventCount].params[2]);
            eventCount++;
        }

        // INPUT_KEY_DOWN
        if (CORE.Input.Keyboard.currentKeyState[key])
        {
            events[eventCount].frame = frame;
            events[eventCount].type = INPUT_KEY_DOWN;
            events[eventCount].params[0] = key;
            events[eventCount].params[1] = 0;
            events[eventCount].params[2] = 0;

            TRACELOG(LOG_INFO, "[%i] INPUT_KEY_DOWN: %i, %i, %i", events[eventCount].frame, events[eventCount].params[0], events[eventCount].params[1], events[eventCount].params[2]);
            eventCount++;
        }
    }

    for (int button = 0; button < MAX_MOUSE_BUTTONS; button++)
    {
        // INPUT_MOUSE_BUTTON_UP
        if (CORE.Input.Mouse.previousButtonState[button] && !CORE.Input.Mouse.currentButtonState[button])
        {
            events[eventCount].frame = frame;
            events[eventCount].type = INPUT_MOUSE_BUTTON_UP;
            events[eventCount].params[0] = button;
            events[eventCount].params[1] = 0;
            events[eventCount].params[2] = 0;

            TRACELOG(LOG_INFO, "[%i] INPUT_MOUSE_BUTTON_UP: %i, %i, %i", events[eventCount].frame, events[eventCount].params[0], events[eventCount].params[1], events[eventCount].params[2]);
            eventCount++;
        }

        // INPUT_MOUSE_BUTTON_DOWN
        if (CORE.Input.Mouse.currentButtonState[button])
        {
            events[eventCount].frame = frame;
            events[eventCount].type = INPUT_MOUSE_BUTTON_DOWN;
            events[eventCount].params[0] = button;
            events[eventCount].params[1] = 0;
            events[eventCount].params[2] = 0;

            TRACELOG(LOG_INFO, "[%i] INPUT_MOUSE_BUTTON_DOWN: %i, %i, %i", events[eventCount].frame, events[eventCount].params[0], events[eventCount].params[1], events[eventCount].params[2]);
            eventCount++;
        }
    }

    // INPUT_MOUSE_POSITION (only saved if changed)
    if (((int)CORE.Input.Mouse.currentPosition.x != (int)CORE.Input.Mouse.previousPosition.x) ||
        ((int)CORE.Input.Mouse.currentPosition.y != (int)CORE.Input.Mouse.previousPosition.y))
    {
        events[eventCount].frame = frame;
        events[eventCount].type = INPUT_MOUSE_POSITION;
        events[eventCount].params[0] = (int)CORE.Input.Mouse.currentPosition.x;
        events[eventCount].params[1] = (int)CORE.Input.Mouse.currentPosition.y;
        events[eventCount].params[2] = 0;

        TRACELOG(LOG_INFO, "[%i] INPUT_MOUSE_POSITION: %i, %i, %i", events[eventCount].frame, events[eventCount].params[0], events[eventCount].params[1], events[eventCount].params[2]);
        eventCount++;
    }

    // INPUT_MOUSE_WHEEL_MOTION
    if (((int)CORE.Input.Mouse.currentWheelMove.x != (int)CORE.Input.Mouse.previousWheelMove.x) ||
        ((int)CORE.Input.Mouse.currentWheelMove.y != (int)CORE.Input.Mouse.previousWheelMove.y))
    {
        events[eventCount].frame = frame;
        events[eventCount].type = INPUT_MOUSE_WHEEL_MOTION;
        events[eventCount].params[0] = (int)CORE.Input.Mouse.currentWheelMove.x;
        events[eventCount].params[1] = (int)CORE.Input.Mouse.currentWheelMove.y;;
        events[eventCount].params[2] = 0;

        TRACELOG(LOG_INFO, "[%i] INPUT_MOUSE_WHEEL_MOTION: %i, %i, %i", events[eventCount].frame, events[eventCount].params[0], events[eventCount].params[1], events[eventCount].params[2]);
        eventCount++;
    }

    for (int id = 0; id < MAX_TOUCH_POINTS; id++)
    {
        // INPUT_TOUCH_UP
        if (CORE.Input.Touch.previousTouchState[id] && !CORE.Input.Touch.currentTouchState[id])
        {
            events[eventCount].frame = frame;
            events[eventCount].type = INPUT_TOUCH_UP;
            events[eventCount].params[0] = id;
            events[eventCount].params[1] = 0;
            events[eventCount].params[2] = 0;

            TRACELOG(LOG_INFO, "[%i] INPUT_TOUCH_UP: %i, %i, %i", events[eventCount].frame, events[eventCount].params[0], events[eventCount].params[1], events[eventCount].params[2]);
            eventCount++;
        }

        // INPUT_TOUCH_DOWN
        if (CORE.Input.Touch.currentTouchState[id])
        {
            events[eventCount].frame = frame;
            events[eventCount].type = INPUT_TOUCH_DOWN;
            events[eventCount].params[0] = id;
            events[eventCount].params[1] = 0;
            events[eventCount].params[2] = 0;

            TRACELOG(LOG_INFO, "[%i] INPUT_TOUCH_DOWN: %i, %i, %i", events[eventCount].frame, events[eventCount].params[0], events[eventCount].params[1], events[eventCount].params[2]);
            eventCount++;
        }

        // INPUT_TOUCH_POSITION
        // TODO: It requires the id!
        /*
        if (((int)CORE.Input.Touch.currentPosition[id].x != (int)CORE.Input.Touch.previousPosition[id].x) ||
            ((int)CORE.Input.Touch.currentPosition[id].y != (int)CORE.Input.Touch.previousPosition[id].y))
        {
            events[eventCount].frame = frame;
            events[eventCount].type = INPUT_TOUCH_POSITION;
            events[eventCount].params[0] = id;
            events[eventCount].params[1] = (int)CORE.Input.Touch.currentPosition[id].x;
            events[eventCount].params[2] = (int)CORE.Input.Touch.currentPosition[id].y;

            TRACELOG(LOG_INFO, "[%i] INPUT_TOUCH_POSITION: %i, %i, %i", events[eventCount].frame, events[eventCount].params[0], events[eventCount].params[1], events[eventCount].params[2]);
            eventCount++;
        }
        */
    }

    for (int gamepad = 0; gamepad < MAX_GAMEPADS; gamepad++)
    {
        // INPUT_GAMEPAD_CONNECT
        /*
        if ((CORE.Input.Gamepad.currentState[gamepad] != CORE.Input.Gamepad.previousState[gamepad]) &&
            (CORE.Input.Gamepad.currentState[gamepad] == true)) // Check if changed to ready
        {
            // TODO: Save gamepad connect event
        }
        */

        // INPUT_GAMEPAD_DISCONNECT
        /*
        if ((CORE.Input.Gamepad.currentState[gamepad] != CORE.Input.Gamepad.previousState[gamepad]) &&
            (CORE.Input.Gamepad.currentState[gamepad] == false)) // Check if changed to not-ready
        {
            // TODO: Save gamepad disconnect event
        }
        */

        for (int button = 0; button < MAX_GAMEPAD_BUTTONS; button++)
        {
            // INPUT_GAMEPAD_BUTTON_UP
            if (CORE.Input.Gamepad.previousButtonState[gamepad][button] && !CORE.Input.Gamepad.currentButtonState[gamepad][button])
            {
                events[eventCount].frame = frame;
                events[eventCount].type = INPUT_GAMEPAD_BUTTON_UP;
                events[eventCount].params[0] = gamepad;
                events[eventCount].params[1] = button;
                events[eventCount].params[2] = 0;

                TRACELOG(LOG_INFO, "[%i] INPUT_GAMEPAD_BUTTON_UP: %i, %i, %i", events[eventCount].frame, events[eventCount].params[0], events[eventCount].params[1], events[eventCount].params[2]);
                eventCount++;
            }

            // INPUT_GAMEPAD_BUTTON_DOWN
            if (CORE.Input.Gamepad.currentButtonState[gamepad][button])
            {
                events[eventCount].frame = frame;
                events[eventCount].type = INPUT_GAMEPAD_BUTTON_DOWN;
                events[eventCount].params[0] = gamepad;
                events[eventCount].params[1] = button;
                events[eventCount].params[2] = 0;

                TRACELOG(LOG_INFO, "[%i] INPUT_GAMEPAD_BUTTON_DOWN: %i, %i, %i", events[eventCount].frame, events[eventCount].params[0], events[eventCount].params[1], events[eventCount].params[2]);
                eventCount++;
            }
        }

        for (int axis = 0; axis < MAX_GAMEPAD_AXIS; axis++)
        {
            // INPUT_GAMEPAD_AXIS_MOTION
            if (CORE.Input.Gamepad.axisState[gamepad][axis] > 0.1f)
            {
                events[eventCount].frame = frame;
                events[eventCount].type = INPUT_GAMEPAD_AXIS_MOTION;
                events[eventCount].params[0] = gamepad;
                events[eventCount].params[1] = axis;
                events[eventCount].params[2] = (int)(CORE.Input.Gamepad.axisState[gamepad][axis]*32768.0f);

                TRACELOG(LOG_INFO, "[%i] INPUT_GAMEPAD_AXIS_MOTION: %i, %i, %i", events[eventCount].frame, events[eventCount].params[0], events[eventCount].params[1], events[eventCount].params[2]);
                eventCount++;
            }
        }
    }

    // INPUT_GESTURE
    if (GESTURES.current != GESTURE_NONE)
    {
        events[eventCount].frame = frame;
        events[eventCount].type = INPUT_GESTURE;
        events[eventCount].params[0] = GESTURES.current;
        events[eventCount].params[1] = 0;
        events[eventCount].params[2] = 0;

        TRACELOG(LOG_INFO, "[%i] INPUT_GESTURE: %i, %i, %i", events[eventCount].frame, events[eventCount].params[0], events[eventCount].params[1], events[eventCount].params[2]);
        eventCount++;
    }
}

// Play automation event
static void PlayAutomationEvent(unsigned int frame)
{
    for (unsigned int i = 0; i < eventCount; i++)
    {
        if (events[i].frame == frame)
        {
            switch (events[i].type)
            {
                // Input events
                case INPUT_KEY_UP: CORE.Input.Keyboard.currentKeyState[events[i].params[0]] = false; break;             // param[0]: key
                case INPUT_KEY_DOWN: CORE.Input.Keyboard.currentKeyState[events[i].params[0]] = true; break;            // param[0]: key
                case INPUT_MOUSE_BUTTON_UP: CORE.Input.Mouse.currentButtonState[events[i].params[0]] = false; break;    // param[0]: key
                case INPUT_MOUSE_BUTTON_DOWN: CORE.Input.Mouse.currentButtonState[events[i].params[0]] = true; break;   // param[0]: key
                case INPUT_MOUSE_POSITION:      // param[0]: x, param[1]: y
                {
                    CORE.Input.Mouse.currentPosition.x = (float)events[i].params[0];
                    CORE.Input.Mouse.currentPosition.y = (float)events[i].params[1];
                } break;
                case INPUT_MOUSE_WHEEL_MOTION:  // param[0]: x delta, param[1]: y delta
                {
                    CORE.Input.Mouse.currentWheelMove.x = (float)events[i].params[0]; break;
                    CORE.Input.Mouse.currentWheelMove.y = (float)events[i].params[1]; break;
                } break;
                case INPUT_TOUCH_UP: CORE.Input.Touch.currentTouchState[events[i].params[0]] = false; break;            // param[0]: id
                case INPUT_TOUCH_DOWN: CORE.Input.Touch.currentTouchState[events[i].params[0]] = true; break;           // param[0]: id
                case INPUT_TOUCH_POSITION:      // param[0]: id, param[1]: x, param[2]: y
                {
                    CORE.Input.Touch.position[events[i].params[0]].x = (float)events[i].params[1];
                    CORE.Input.Touch.position[events[i].params[0]].y = (float)events[i].params[2];
                } break;
                case INPUT_GAMEPAD_CONNECT: CORE.Input.Gamepad.ready[events[i].params[0]] = true; break;                // param[0]: gamepad
                case INPUT_GAMEPAD_DISCONNECT: CORE.Input.Gamepad.ready[events[i].params[0]] = false; break;            // param[0]: gamepad
                case INPUT_GAMEPAD_BUTTON_UP: CORE.Input.Gamepad.currentButtonState[events[i].params[0]][events[i].params[1]] = false; break;    // param[0]: gamepad, param[1]: button
                case INPUT_GAMEPAD_BUTTON_DOWN: CORE.Input.Gamepad.currentButtonState[events[i].params[0]][events[i].params[1]] = true; break;   // param[0]: gamepad, param[1]: button
                case INPUT_GAMEPAD_AXIS_MOTION: // param[0]: gamepad, param[1]: axis, param[2]: delta
                {
                    CORE.Input.Gamepad.axisState[events[i].params[0]][events[i].params[1]] = ((float)events[i].params[2]/32768.0f);
                } break;
                case INPUT_GESTURE: GESTURES.current = events[i].params[0]; break;     // param[0]: gesture (enum Gesture) -> rgestures.h: GESTURES.current

                // Window events
                case WINDOW_CLOSE: CORE.Window.shouldClose = true; break;
                case WINDOW_MAXIMIZE: MaximizeWindow(); break;
                case WINDOW_MINIMIZE: MinimizeWindow(); break;
                case WINDOW_RESIZE: SetWindowSize(events[i].params[0], events[i].params[1]); break;

                // Custom events
                case ACTION_TAKE_SCREENSHOT:
                {
                    TakeScreenshot(TextFormat("screenshot%03i.png", screenshotCounter));
                    screenshotCounter++;
                } break;
                case ACTION_SETTARGETFPS: SetTargetFPS(events[i].params[0]); break;
                default: break;
            }
        }
    }
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

    va_list args;
    va_start(args, text);
    vsnprintf(currentBuffer, MAX_TEXT_BUFFER_LENGTH, text, args);
    va_end(args);

    index += 1;     // Move to next buffer for next function call
    if (index >= MAX_TEXTFORMAT_BUFFERS) index = 0;

    return currentBuffer;
}
#endif // !SUPPORT_MODULE_RTEXT
