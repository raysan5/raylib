/**********************************************************************************************
*
*   raylib.core - Basic functions to manage windows, OpenGL context and input on multiple platforms
*
*   PLATFORMS SUPPORTED:
*       - PLATFORM_DESKTOP: Windows (Win32, Win64)
*       - PLATFORM_DESKTOP: Linux (X11 desktop mode)
*       - PLATFORM_DESKTOP: FreeBSD, OpenBSD, NetBSD, DragonFly (X11 desktop)
*       - PLATFORM_DESKTOP: OSX/macOS
*       - PLATFORM_ANDROID: Android 4.0 (ARM, ARM64)
*       - PLATFORM_RPI:     Raspberry Pi 0,1,2,3 (Raspbian)
*       - PLATFORM_WEB:     HTML5 with asm.js (Chrome, Firefox)
*       - PLATFORM_UWP:     Windows 10 App, Windows Phone, Xbox One
*
*   CONFIGURATION:
*
*   #define PLATFORM_DESKTOP
*       Windowing and input system configured for desktop platforms: Windows, Linux, OSX, FreeBSD, OpenBSD, NetBSD, DragonFly
*       NOTE: Oculus Rift CV1 requires PLATFORM_DESKTOP for mirror rendering - View [rlgl] module to enable it
*
*   #define PLATFORM_ANDROID
*       Windowing and input system configured for Android device, app activity managed internally in this module.
*       NOTE: OpenGL ES 2.0 is required and graphic device is managed by EGL
*
*   #define PLATFORM_RPI
*       Windowing and input system configured for Raspberry Pi i native mode (no X.org required, tested on Raspbian),
*       graphic device is managed by EGL and inputs are processed is raw mode, reading from /dev/input/
*
*   #define PLATFORM_WEB
*       Windowing and input system configured for HTML5 (run on browser), code converted from C to asm.js
*       using emscripten compiler. OpenGL ES 2.0 required for direct translation to WebGL equivalent code.
*
*   #define PLATFORM_UWP
*       Universal Windows Platform support, using OpenGL ES 2.0 through ANGLE on multiple Windows platforms,
*       including Windows 10 App, Windows Phone and Xbox One platforms.
*
*   #define SUPPORT_DEFAULT_FONT (default)
*       Default font is loaded on window initialization to be available for the user to render simple text.
*       NOTE: If enabled, uses external module functions to load default raylib font (module: text)
*
*   #define SUPPORT_CAMERA_SYSTEM
*       Camera module is included (camera.h) and multiple predefined cameras are available: free, 1st/3rd person, orbital
*
*   #define SUPPORT_GESTURES_SYSTEM
*       Gestures module is included (gestures.h) to support gestures detection: tap, hold, swipe, drag
*
*   #define SUPPORT_MOUSE_GESTURES
*       Mouse gestures are directly mapped like touches and processed by gestures system.
*
*   #define SUPPORT_BUSY_WAIT_LOOP
*       Use busy wait loop for timing sync, if not defined, a high-resolution timer is setup and used
*
*   #define SUPPORT_EVENTS_WAITING
*       Wait for events passively (sleeping while no events) instead of polling them actively every frame
*
*   #define SUPPORT_SCREEN_CAPTURE
*       Allow automatic screen capture of current screen pressing F12, defined in KeyCallback()
*
*   #define SUPPORT_GIF_RECORDING
*       Allow automatic gif recording of current screen pressing CTRL+F12, defined in KeyCallback()
*
*   DEPENDENCIES:
*       rglfw    - Manage graphic device, OpenGL context and inputs on PLATFORM_DESKTOP (Windows, Linux, OSX. FreeBSD, OpenBSD, NetBSD, DragonFly)
*       raymath  - 3D math functionality (Vector2, Vector3, Matrix, Quaternion)
*       camera   - Multiple 3D camera modes (free, orbital, 1st person, 3rd person)
*       gestures - Gestures system for touch-ready devices (or simulated from mouse inputs)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2019 Ramon Santamaria (@raysan5)
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

#include "raylib.h"             // Declares module functions

// Check if config flags have been externally provided on compilation line
#if !defined(EXTERNAL_CONFIG_FLAGS)
    #include "config.h"         // Defines module configuration flags
#else
    #define RAYLIB_VERSION  "2.5"
#endif

#if (defined(__linux__) || defined(PLATFORM_WEB)) && _POSIX_C_SOURCE < 199309L
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 199309L // Required for CLOCK_MONOTONIC if compiled with c99 without gnu ext.
#endif

#define RAYMATH_IMPLEMENTATION  // Define external out-of-line implementation of raymath here
#include "raymath.h"            // Required for: Vector3 and Matrix functions

#define RLGL_IMPLEMENTATION
#include "rlgl.h"               // raylib OpenGL abstraction layer to OpenGL 1.1, 3.3+ or ES2

#include "utils.h"              // Required for: fopen() Android mapping

#if defined(SUPPORT_GESTURES_SYSTEM)
    #define GESTURES_IMPLEMENTATION
    #include "gestures.h"       // Gestures detection functionality
#endif

#if defined(SUPPORT_CAMERA_SYSTEM) && !defined(PLATFORM_ANDROID)
    #define CAMERA_IMPLEMENTATION
    #include "camera.h"         // Camera system functionality
#endif

#if defined(SUPPORT_GIF_RECORDING)
    #define RGIF_IMPLEMENTATION
    #include "external/rgif.h"  // Support GIF recording
#endif

#include <stdio.h>          // Standard input / output lib
#include <stdlib.h>         // Required for: malloc(), free(), rand(), atexit()
#include <stdint.h>         // Required for: typedef unsigned long long int uint64_t, used by hi-res timer
#include <time.h>           // Required for: time() - Android/RPI hi-res timer (NOTE: Linux only!)
#include <math.h>           // Required for: tan() [Used in BeginMode3D() to set perspective]
#include <string.h>         // Required for: strrchr(), strcmp()
//#include <errno.h>          // Macros for reporting and retrieving error conditions through error codes
#include <ctype.h>          // Required for: tolower() [Used in IsFileExtension()]
#include <sys/stat.h>       // Required for stat() [Used in GetLastWriteTime()]

#if defined(PLATFORM_DESKTOP) && defined(_WIN32) && (defined(_MSC_VER) || defined(__TINYC__))
    #include "external/dirent.h"    // Required for: DIR, opendir(), closedir() [Used in GetDirectoryFiles()]
#else
    #include <dirent.h>             // Required for: DIR, opendir(), closedir() [Used in GetDirectoryFiles()]
#endif

#if defined(_WIN32)
    #include <direct.h>             // Required for: _getch(), _chdir()
    #define GETCWD _getcwd          // NOTE: MSDN recommends not to use getcwd(), chdir()
    #define CHDIR _chdir
    #include <io.h>                 // Required for _access() [Used in FileExists()]
#else
    #include "unistd.h"             // Required for: getch(), chdir() (POSIX), access()
    #define GETCWD getcwd
    #define CHDIR chdir
#endif

#if defined(PLATFORM_DESKTOP)
    #define GLFW_INCLUDE_NONE       // Disable the standard OpenGL header inclusion on GLFW3
                                    // NOTE: Already provided by rlgl implementation (on glad.h)
    #include <GLFW/glfw3.h>         // GLFW3 library: Windows, OpenGL context and Input management
                                    // NOTE: GLFW3 already includes gl.h (OpenGL) headers

    // Support retrieving native window handlers
    #if defined(_WIN32)
        #define GLFW_EXPOSE_NATIVE_WIN32
        #include <GLFW/glfw3native.h>       // WARNING: It requires customization to avoid windows.h inclusion!

        #if !defined(SUPPORT_BUSY_WAIT_LOOP)
            // NOTE: Those functions require linking with winmm library
            unsigned int __stdcall timeBeginPeriod(unsigned int uPeriod);
            unsigned int __stdcall timeEndPeriod(unsigned int uPeriod);
        #endif

    #elif defined(__linux__)
        #include <sys/time.h>           // Required for: timespec, nanosleep(), select() - POSIX

        //#define GLFW_EXPOSE_NATIVE_X11        // WARNING: Exposing Xlib.h > X.h results in dup symbols for Font type
        //#define GLFW_EXPOSE_NATIVE_WAYLAND
        //#define GLFW_EXPOSE_NATIVE_MIR
        #include <GLFW/glfw3native.h>   // Required for: glfwGetX11Window()
    #elif defined(__APPLE__)
        #include <unistd.h>             // Required for: usleep()

        //#define GLFW_EXPOSE_NATIVE_COCOA      // WARNING: Fails due to type redefinition
        #include <GLFW/glfw3native.h>   // Required for: glfwGetCocoaWindow()
    #endif
#endif

#if defined(__linux__)
    #include <linux/limits.h>               // for NAME_MAX and PATH_MAX defines
    #define MAX_FILEPATH_LENGTH PATH_MAX    // Use Linux define (4096)
#else
    #define MAX_FILEPATH_LENGTH     512     // Use common value
#endif

#if defined(PLATFORM_ANDROID)
    //#include <android/sensor.h>           // Android sensors functions (accelerometer, gyroscope, light...)
    #include <android/window.h>             // Defines AWINDOW_FLAG_FULLSCREEN and others
    #include <android_native_app_glue.h>    // Defines basic app state struct and manages activity

    #include <EGL/egl.h>        // Khronos EGL library - Native platform display device control functions
    #include <GLES2/gl2.h>      // Khronos OpenGL ES 2.0 library
#endif

#if defined(PLATFORM_RPI)
    #include <fcntl.h>          // POSIX file control definitions - open(), creat(), fcntl()
    #include <unistd.h>         // POSIX standard function definitions - read(), close(), STDIN_FILENO
    #include <termios.h>        // POSIX terminal control definitions - tcgetattr(), tcsetattr()
    #include <pthread.h>        // POSIX threads management (mouse input)
    #include <dirent.h>         // POSIX directory browsing

    #include <sys/ioctl.h>      // UNIX System call for device-specific input/output operations - ioctl()
    #include <linux/kd.h>       // Linux: KDSKBMODE, K_MEDIUMRAM constants definition
    #include <linux/input.h>    // Linux: Keycodes constants definition (KEY_A, ...)
    #include <linux/joystick.h> // Linux: Joystick support library

    #include "bcm_host.h"       // Raspberry Pi VideoCore IV access functions

    #include "EGL/egl.h"        // Khronos EGL library - Native platform display device control functions
    #include "EGL/eglext.h"     // Khronos EGL library - Extensions
    #include "GLES2/gl2.h"      // Khronos OpenGL ES 2.0 library
#endif

#if defined(PLATFORM_UWP)
    #include "EGL/egl.h"        // Khronos EGL library - Native platform display device control functions
    #include "EGL/eglext.h"     // Khronos EGL library - Extensions
    #include "GLES2/gl2.h"      // Khronos OpenGL ES 2.0 library
#endif

#if defined(PLATFORM_WEB)
    #define GLFW_INCLUDE_ES2        // GLFW3: Enable OpenGL ES 2.0 (translated to WebGL)
    #include <GLFW/glfw3.h>         // GLFW3 library: Windows, OpenGL context and Input management
    #include <sys/time.h>           // Required for: timespec, nanosleep(), select() - POSIX

    #include <emscripten/emscripten.h>  // Emscripten library - LLVM to JavaScript compiler
    #include <emscripten/html5.h>       // Emscripten HTML5 library
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#if defined(PLATFORM_RPI)
    #define USE_LAST_TOUCH_DEVICE       // When multiple touchscreens are connected, only use the one with the highest event<N> number

    // Old device inputs system
    #define DEFAULT_KEYBOARD_DEV      STDIN_FILENO              // Standard input
    #define DEFAULT_GAMEPAD_DEV       "/dev/input/js"           // Gamepad input (base dev for all gamepads: js0, js1, ...)
    #define DEFAULT_EVDEV_PATH        "/dev/input/"             // Path to the linux input events

    // New device input events (evdev) (must be detected)
    //#define DEFAULT_KEYBOARD_DEV    "/dev/input/eventN"
    //#define DEFAULT_MOUSE_DEV       "/dev/input/eventN"
    //#define DEFAULT_GAMEPAD_DEV     "/dev/input/eventN"

    #define MOUSE_SENSITIVITY         0.8f
#endif

#define MAX_GAMEPADS              4         // Max number of gamepads supported
#define MAX_GAMEPAD_BUTTONS       32        // Max bumber of buttons supported (per gamepad)
#define MAX_GAMEPAD_AXIS          8         // Max number of axis supported (per gamepad)

#define STORAGE_FILENAME        "storage.data"

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------

// Window/Graphics related variables
//-----------------------------------------------------------------------------------
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
static GLFWwindow *window;                      // Native window (graphic device)
#endif
static bool windowReady = false;                // Check if window has been initialized successfully
static bool windowMinimized = false;            // Check if window has been minimized
static bool windowResized = false;              // Check if window has been resized
static const char *windowTitle = NULL;          // Window text title...

static unsigned int displayWidth, displayHeight;// Display width and height (monitor, device-screen, LCD, ...)
static int screenWidth, screenHeight;           // Screen width and height (used render area)
static int renderWidth, renderHeight;           // Framebuffer width and height (render area, including black bars if required)
static int currentWidth, currentHeight;         // Current render width and height, it could change on BeginTextureMode()
static int renderOffsetX = 0;                   // Offset X from render area (must be divided by 2)
static int renderOffsetY = 0;                   // Offset Y from render area (must be divided by 2)
static bool fullscreen = false;                 // Fullscreen mode (useful only for PLATFORM_DESKTOP)
static Matrix downscaleView;                    // Matrix to downscale view (in case screen size bigger than display size)

#if defined(PLATFORM_RPI)
static EGL_DISPMANX_WINDOW_T nativeWindow;      // Native window (graphic device)
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI) || defined(PLATFORM_UWP)
static EGLDisplay display;                      // Native display device (physical screen connection)
static EGLSurface surface;                      // Surface to draw on, framebuffers (connected to context)
static EGLContext context;                      // Graphic context, mode in which drawing can be done
static EGLConfig config;                        // Graphic config
static uint64_t baseTime;                       // Base time measure for hi-res timer
static bool windowShouldClose = false;          // Flag to set window for closing
#endif

#if defined(PLATFORM_UWP)
extern EGLNativeWindowType uwpWindow;           // Native EGL window handler for UWP (external, defined in UWP App)
#endif
//-----------------------------------------------------------------------------------

#if defined(PLATFORM_ANDROID)
static struct android_app *androidApp;          // Android activity
static struct android_poll_source *source;      // Android events polling source
static int ident, events;                       // Android ALooper_pollAll() variables
static const char *internalDataPath = NULL;     // Android internal data path to write data (/data/data/<package>/files)

static bool appEnabled = true;                  // Used to detec if app is active
static bool contextRebindRequired = false;      // Used to know context rebind required
#endif

// Inputs related variables
//-----------------------------------------------------------------------------------
// Keyboard states
static char previousKeyState[512] = { 0 };      // Registers previous frame key state
static char currentKeyState[512] = { 0 };       // Registers current frame key state
static int lastKeyPressed = -1;                 // Register last key pressed
static int exitKey = KEY_ESCAPE;                // Default exit key (ESC)

#if defined(PLATFORM_RPI)
// NOTE: For keyboard we will use the standard input (but reconfigured...)
static struct termios defaultKeyboardSettings;  // Used to store default keyboard settings
static int defaultKeyboardMode;                 // Used to store default keyboard mode
#endif

// Mouse states
static Vector2 mousePosition = { 0.0f, 0.0f };  // Mouse position on screen
static Vector2 mouseScale = { 1.0f, 1.0f };     // Mouse scaling
static Vector2 mouseOffset = { 0.0f, 0.0f };    // Mouse offset
static bool cursorHidden = false;               // Track if cursor is hidden
static bool cursorOnScreen = false;             // Tracks if cursor is inside client area
static Vector2 touchPosition[MAX_TOUCH_POINTS]; // Touch position on screen

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI) || defined(PLATFORM_WEB) || defined(PLATFORM_UWP)
static char previousMouseState[3] = { 0 };      // Registers previous mouse button state
static char currentMouseState[3] = { 0 };       // Registers current mouse button state
static int previousMouseWheelY = 0;             // Registers previous mouse wheel variation
static int currentMouseWheelY = 0;              // Registers current mouse wheel variation
#endif

#if defined(PLATFORM_RPI)
static char currentMouseStateEvdev[3] = { 0 };  // Holds the new mouse state for the next polling event to grab (Can't be written directly due to multithreading, app could miss the update)

typedef struct {
    pthread_t threadId;                         // Event reading thread id
    int fd;                                     // File descriptor to the device it is assigned to
    int eventNum;                               // Number of 'event<N>' device
    Rectangle absRange;                         // Range of values for absolute pointing devices (touchscreens)
    int touchSlot;                              // Hold the touch slot number of the currently being sent multitouch block
    bool isMouse;                               // True if device supports relative X Y movements
    bool isTouch;                               // True if device supports absolute X Y movements and has BTN_TOUCH
    bool isMultitouch;                          // True if device supports multiple absolute movevents and has BTN_TOUCH
    bool isKeyboard;                            // True if device has letter keycodes
    bool isGamepad;                             // True if device has gamepad buttons
} InputEventWorker;

static InputEventWorker eventWorkers[10];       // List of worker threads for every monitored "/dev/input/event<N>"

typedef struct{
    int Contents[8];
    char Head;
    char Tail;
} KeyEventFifo;

static KeyEventFifo lastKeyPressedEvdev;        // Buffer for holding keydown events as they arrive (Needed due to multitreading of event workers)
static char currentKeyStateEvdev[512] = { 0 };  // Registers current frame key state from event based driver (Needs to be seperate because the legacy console based method clears keys on every frame)

#endif
#if defined(PLATFORM_WEB)
static bool toggleCursorLock = false;           // Ask for cursor pointer lock on next click
#endif

// Gamepads states
static int lastGamepadButtonPressed = -1;       // Register last gamepad button pressed
static int gamepadAxisCount = 0;                // Register number of available gamepad axis

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI) || defined(PLATFORM_WEB) || defined(PLATFORM_UWP)
static bool gamepadReady[MAX_GAMEPADS] = { false };             // Flag to know if gamepad is ready
static float gamepadAxisState[MAX_GAMEPADS][MAX_GAMEPAD_AXIS];  // Gamepad axis state
static char previousGamepadState[MAX_GAMEPADS][MAX_GAMEPAD_BUTTONS];    // Previous gamepad buttons state
static char currentGamepadState[MAX_GAMEPADS][MAX_GAMEPAD_BUTTONS];     // Current gamepad buttons state
#endif

#if defined(PLATFORM_RPI)
static int gamepadStream[MAX_GAMEPADS] = { -1 };// Gamepad device file descriptor
static pthread_t gamepadThreadId;               // Gamepad reading thread id
static char gamepadName[64];                    // Gamepad name holder
#endif
//-----------------------------------------------------------------------------------

// Timming system variables
//-----------------------------------------------------------------------------------
static double currentTime = 0.0;            // Current time measure
static double previousTime = 0.0;           // Previous time measure
static double updateTime = 0.0;             // Time measure for frame update
static double drawTime = 0.0;               // Time measure for frame draw
static double frameTime = 0.0;              // Time measure for one frame
static double targetTime = 0.0;             // Desired time for one frame, if 0 not applied
//-----------------------------------------------------------------------------------

// Config internal variables
//-----------------------------------------------------------------------------------
static unsigned char configFlags = 0;       // Configuration flags (bit based)
static bool showLogo = false;               // Track if showing logo at init is enabled

static char **dropFilesPath;                // Store dropped files paths as strings
static int dropFilesCount = 0;              // Count dropped files strings

static char **dirFilesPath;                 // Store directory files paths as strings
static int dirFilesCount = 0;               // Count directory files strings

#if defined(SUPPORT_SCREEN_CAPTURE)
static int screenshotCounter = 0;           // Screenshots counter
#endif

#if defined(SUPPORT_GIF_RECORDING)
static int gifFramesCounter = 0;            // GIF frames counter
static bool gifRecording = false;           // GIF recording state
#endif
//-----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Other Modules Functions Declaration (required by core)
//----------------------------------------------------------------------------------
#if defined(SUPPORT_DEFAULT_FONT)
extern void LoadDefaultFont(void);          // [Module: text] Loads default font on InitWindow()
extern void UnloadDefaultFont(void);        // [Module: text] Unloads default font from GPU memory
#endif

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static bool InitGraphicsDevice(int width, int height);  // Initialize graphics device
static void SetupFramebuffer(int width, int height);    // Setup main framebuffer
static void SetupViewport(void);                        // Set viewport parameters
static void SwapBuffers(void);                          // Copy back buffer to front buffers

static void InitTimer(void);                            // Initialize timer
static void Wait(float ms);                             // Wait for some milliseconds (stop program execution)

static bool GetKeyStatus(int key);                      // Returns if a key has been pressed
static bool GetMouseButtonStatus(int button);           // Returns if a mouse button has been pressed
static void PollInputEvents(void);                      // Register user events

static void LogoAnimation(void);                        // Plays raylib logo appearing animation

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
static void ErrorCallback(int error, const char *description);                             // GLFW3 Error Callback, runs on GLFW3 error
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);  // GLFW3 Keyboard Callback, runs on key pressed
static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);     // GLFW3 Mouse Button Callback, runs on mouse button pressed
static void MouseCursorPosCallback(GLFWwindow *window, double x, double y);                // GLFW3 Cursor Position Callback, runs on mouse move
static void CharCallback(GLFWwindow *window, unsigned int key);                            // GLFW3 Char Key Callback, runs on key pressed (get char value)
static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);            // GLFW3 Srolling Callback, runs on mouse wheel
static void CursorEnterCallback(GLFWwindow *window, int enter);                            // GLFW3 Cursor Enter Callback, cursor enters client area
static void WindowSizeCallback(GLFWwindow *window, int width, int height);                 // GLFW3 WindowSize Callback, runs when window is resized
static void WindowIconifyCallback(GLFWwindow *window, int iconified);                      // GLFW3 WindowIconify Callback, runs when window is minimized/restored
static void WindowDropCallback(GLFWwindow *window, int count, const char **paths);         // GLFW3 Window Drop Callback, runs when drop files into window
#endif

#if defined(PLATFORM_ANDROID)
static void AndroidCommandCallback(struct android_app *app, int32_t cmd);                  // Process Android activity lifecycle commands
static int32_t AndroidInputCallback(struct android_app *app, AInputEvent *event);          // Process Android inputs
#endif

#if defined(PLATFORM_WEB)
static EM_BOOL EmscriptenFullscreenChangeCallback(int eventType, const EmscriptenFullscreenChangeEvent *e, void *userData);
static EM_BOOL EmscriptenKeyboardCallback(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData);
static EM_BOOL EmscriptenMouseCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
static EM_BOOL EmscriptenTouchCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
static EM_BOOL EmscriptenGamepadCallback(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData);
#endif

#if defined(PLATFORM_RPI)
static void InitKeyboard(void);                         // Init raw keyboard system (standard input reading)
static void ProcessKeyboard(void);                      // Process keyboard events
static void RestoreKeyboard(void);                      // Restore keyboard system
static void InitEvdevInput(void);                            // Mouse initialization (including mouse thread)
static void EventThreadSpawn(char *device);             // Identifies a input device and spawns a thread to handle it if needed
static void *EventThread(void *arg);                    // Input device events reading thread
static void InitGamepad(void);                          // Init raw gamepad input
static void *GamepadThread(void *arg);                  // Mouse reading thread
#endif

#if defined(PLATFORM_UWP)
// TODO: Define functions required to manage inputs
#endif

#if defined(_WIN32)
    // NOTE: We include Sleep() function signature here to avoid windows.h inclusion
    void __stdcall Sleep(unsigned long msTimeout);      // Required for Wait()
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - Window and OpenGL Context Functions
//----------------------------------------------------------------------------------

#if defined(PLATFORM_ANDROID)
// To allow easier porting to android, we allow the user to define a
// main function which we call from android_main, defined by ourselves
extern int main(int argc, char *argv[]);

void android_main(struct android_app *app)
{
    char arg0[] = "raylib";     // NOTE: argv[] are mutable
    androidApp = app;

    // TODO: Should we maybe report != 0 return codes somewhere?
    (void)main(1, (char *[]) { arg0, NULL });
}

// TODO: Add this to header (if apps really need it)
struct android_app *GetAndroidApp(void)
{
    return androidApp;
}
#endif

// Initialize window and OpenGL context
// NOTE: data parameter could be used to pass any kind of required data to the initialization
void InitWindow(int width, int height, const char *title)
{
    TraceLog(LOG_INFO, "Initializing raylib %s", RAYLIB_VERSION);

    windowTitle = title;
#if defined(PLATFORM_ANDROID)
    screenWidth = width;
    screenHeight = height;
    currentWidth = width;
    currentHeight = height;

    // Input data is android app pointer
    internalDataPath = androidApp->activity->internalDataPath;

    // Set desired windows flags before initializing anything
    ANativeActivity_setWindowFlags(androidApp->activity, AWINDOW_FLAG_FULLSCREEN, 0);  //AWINDOW_FLAG_SCALED, AWINDOW_FLAG_DITHER
    //ANativeActivity_setWindowFlags(androidApp->activity, AWINDOW_FLAG_FORCE_NOT_FULLSCREEN, AWINDOW_FLAG_FULLSCREEN);

    int orientation = AConfiguration_getOrientation(androidApp->config);

    if (orientation == ACONFIGURATION_ORIENTATION_PORT) TraceLog(LOG_INFO, "PORTRAIT window orientation");
    else if (orientation == ACONFIGURATION_ORIENTATION_LAND) TraceLog(LOG_INFO, "LANDSCAPE window orientation");

    // TODO: Automatic orientation doesn't seem to work
    if (width <= height)
    {
        AConfiguration_setOrientation(androidApp->config, ACONFIGURATION_ORIENTATION_PORT);
        TraceLog(LOG_WARNING, "Window set to portraid mode");
    }
    else
    {
        AConfiguration_setOrientation(androidApp->config, ACONFIGURATION_ORIENTATION_LAND);
        TraceLog(LOG_WARNING, "Window set to landscape mode");
    }

    //AConfiguration_getDensity(androidApp->config);
    //AConfiguration_getKeyboard(androidApp->config);
    //AConfiguration_getScreenSize(androidApp->config);
    //AConfiguration_getScreenLong(androidApp->config);

    androidApp->onAppCmd = AndroidCommandCallback;
    androidApp->onInputEvent = AndroidInputCallback;

    InitAssetManager(androidApp->activity->assetManager);

    TraceLog(LOG_INFO, "Android app initialized successfully");

    // Wait for window to be initialized (display and context)
    while (!windowReady)
    {
        // Process events loop
        while ((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0)
        {
            // Process this event
            if (source != NULL) source->process(androidApp, source);

            // NOTE: Never close window, native activity is controlled by the system!
            //if (androidApp->destroyRequested != 0) windowShouldClose = true;
        }
    }
#else
    // Init graphics device (display device and OpenGL context)
    // NOTE: returns true if window and graphic device has been initialized successfully
    windowReady = InitGraphicsDevice(width, height);
    if (!windowReady) return;

    // Init hi-res timer
    InitTimer();

#if defined(SUPPORT_DEFAULT_FONT)
    // Load default font
    // NOTE: External function (defined in module: text)
    LoadDefaultFont();
#endif

#if defined(PLATFORM_RPI)
    // Init raw input system
    InitEvdevInput();        // Mouse init
    InitKeyboard();     // Keyboard init
    InitGamepad();      // Gamepad init
#endif

#if defined(PLATFORM_WEB)
    emscripten_set_fullscreenchange_callback(0, 0, 1, EmscriptenFullscreenChangeCallback);

    // Support keyboard events
    emscripten_set_keypress_callback("#canvas", NULL, 1, EmscriptenKeyboardCallback);

    // Support mouse events
    emscripten_set_click_callback("#canvas", NULL, 1, EmscriptenMouseCallback);

    // Support touch events
    emscripten_set_touchstart_callback("#canvas", NULL, 1, EmscriptenTouchCallback);
    emscripten_set_touchend_callback("#canvas", NULL, 1, EmscriptenTouchCallback);
    emscripten_set_touchmove_callback("#canvas", NULL, 1, EmscriptenTouchCallback);
    emscripten_set_touchcancel_callback("#canvas", NULL, 1, EmscriptenTouchCallback);
    //emscripten_set_touchstart_callback(0, NULL, 1, Emscripten_HandleTouch);
    //emscripten_set_touchend_callback("#canvas", data, 0, Emscripten_HandleTouch);

    // Support gamepad events (not provided by GLFW3 on emscripten)
    emscripten_set_gamepadconnected_callback(NULL, 1, EmscriptenGamepadCallback);
    emscripten_set_gamepaddisconnected_callback(NULL, 1, EmscriptenGamepadCallback);
#endif

    mousePosition.x = (float)screenWidth/2.0f;
    mousePosition.y = (float)screenHeight/2.0f;

    // raylib logo appearing animation (if enabled)
    if (showLogo)
    {
        SetTargetFPS(60);
        LogoAnimation();
    }
#endif        // defined(PLATFORM_ANDROID)
}

// Close window and unload OpenGL context
void CloseWindow(void)
{
#if defined(SUPPORT_GIF_RECORDING)
    if (gifRecording)
    {
        GifEnd();
        gifRecording = false;
    }
#endif

#if defined(SUPPORT_DEFAULT_FONT)
    UnloadDefaultFont();
#endif

    rlglClose();                // De-init rlgl

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    glfwDestroyWindow(window);
    glfwTerminate();
#endif

#if !defined(SUPPORT_BUSY_WAIT_LOOP) && defined(_WIN32)
    timeEndPeriod(1);           // Restore time period
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI) || defined(PLATFORM_UWP)
    // Close surface, context and display
    if (display != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (surface != EGL_NO_SURFACE)
        {
            eglDestroySurface(display, surface);
            surface = EGL_NO_SURFACE;
        }

        if (context != EGL_NO_CONTEXT)
        {
            eglDestroyContext(display, context);
            context = EGL_NO_CONTEXT;
        }

        eglTerminate(display);
        display = EGL_NO_DISPLAY;
    }
#endif

#if defined(PLATFORM_RPI)
    // Wait for mouse and gamepad threads to finish before closing
    // NOTE: Those threads should already have finished at this point
    // because they are controlled by windowShouldClose variable

    windowShouldClose = true;   // Added to force threads to exit when the close window is called

    for (int i = 0; i < sizeof(eventWorkers)/sizeof(InputEventWorker); ++i)
    {
        if (eventWorkers[i].threadId == 0)
        {
            pthread_join(eventWorkers[i].threadId, NULL);
        }
    }
    pthread_join(gamepadThreadId, NULL);
#endif

    TraceLog(LOG_INFO, "Window closed successfully");
}

// Check if window has been initialized successfully
bool IsWindowReady(void)
{
    return windowReady;
}

// Check if KEY_ESCAPE pressed or Close icon pressed
bool WindowShouldClose(void)
{
#if defined(PLATFORM_WEB)
    // Emterpreter-Async required to run sync code
    // https://github.com/emscripten-core/emscripten/wiki/Emterpreter#emterpreter-async-run-synchronous-code
    // By default, this function is never called on a web-ready raylib example because we encapsulate
    // frame code in a UpdateDrawFrame() function, to allow browser manage execution asynchronously
    // but now emscripten allows sync code to be executed in an interpreted way, using emterpreter!
    emscripten_sleep(16);
    return false;
#endif

#if defined(PLATFORM_DESKTOP)
    if (windowReady)
    {
        // While window minimized, stop loop execution
        while (windowMinimized) glfwWaitEvents();

        return (glfwWindowShouldClose(window));
    }
    else return true;
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI) || defined(PLATFORM_UWP)
    if (windowReady) return windowShouldClose;
    else return true;
#endif
}

// Check if window has been minimized (or lost focus)
bool IsWindowMinimized(void)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB) || defined(PLATFORM_UWP)
    return windowMinimized;
#else
    return false;
#endif
}

// Check if window has been resized
bool IsWindowResized(void)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB) || defined(PLATFORM_UWP)
    return windowResized;
#else
    return false;
#endif
}

// Check if window is currently hidden
bool IsWindowHidden(void)
{
#if defined(PLATFORM_DESKTOP)
    return (glfwGetWindowAttrib(window, GLFW_VISIBLE) == GL_FALSE);
#endif
    return false;
}

// Toggle fullscreen mode (only PLATFORM_DESKTOP)
void ToggleFullscreen(void)
{
#if defined(PLATFORM_DESKTOP)
    fullscreen = !fullscreen;          // Toggle fullscreen flag

    // NOTE: glfwSetWindowMonitor() doesn't work properly (bugs)
    if (fullscreen) glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, screenWidth, screenHeight, GLFW_DONT_CARE);
    else glfwSetWindowMonitor(window, NULL, 0, 0, screenWidth, screenHeight, GLFW_DONT_CARE);
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
    TraceLog(LOG_WARNING, "Could not toggle to windowed mode");
#endif
}

// Set icon for window (only PLATFORM_DESKTOP)
// NOTE: Image must be in RGBA format, 8bit per channel
void SetWindowIcon(Image image)
{
#if defined(PLATFORM_DESKTOP)
    if (image.format == UNCOMPRESSED_R8G8B8A8)
    {
        GLFWimage icon[1] = { 0 };

        icon[0].width = image.width;
        icon[0].height = image.height;
        icon[0].pixels = (unsigned char *)image.data;

        // NOTE 1: We only support one image icon
        // NOTE 2: The specified image data is copied before this function returns
        glfwSetWindowIcon(window, 1, icon);
    }
    else TraceLog(LOG_WARNING, "Window icon image must be in R8G8B8A8 pixel format");
#endif
}

// Set title for window (only PLATFORM_DESKTOP)
void SetWindowTitle(const char *title)
{
#if defined(PLATFORM_DESKTOP)
    glfwSetWindowTitle(window, title);
#endif
}

// Set window position on screen (windowed mode)
void SetWindowPosition(int x, int y)
{
#if defined(PLATFORM_DESKTOP)
    glfwSetWindowPos(window, x, y);
#endif
}

// Set monitor for the current window (fullscreen mode)
void SetWindowMonitor(int monitor)
{
#if defined(PLATFORM_DESKTOP)
    int monitorCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        //glfwSetWindowMonitor(window, monitors[monitor], 0, 0, screenWidth, screenHeight, GLFW_DONT_CARE);
        TraceLog(LOG_INFO, "Selected fullscreen monitor: [%i] %s", monitor, glfwGetMonitorName(monitors[monitor]));
    }
    else TraceLog(LOG_WARNING, "Selected monitor not found");
#endif
}

// Set window minimum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMinSize(int width, int height)
{
#if defined(PLATFORM_DESKTOP)
    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowSizeLimits(window, width, height, mode->width, mode->height);
#endif
}

// Set window dimensions
void SetWindowSize(int width, int height)
{
#if defined(PLATFORM_DESKTOP)
    glfwSetWindowSize(window, width, height);
#endif
}

// Show the window
void UnhideWindow(void)
{
#if defined(PLATFORM_DESKTOP)
    glfwShowWindow(window);
#endif
}

// Hide the window
void HideWindow(void)
{
#if defined(PLATFORM_DESKTOP)
    glfwHideWindow(window);
#endif
}

// Get current screen width
int GetScreenWidth(void)
{
    return screenWidth;
}

// Get current screen height
int GetScreenHeight(void)
{
    return screenHeight;
}

// Get native window handle
void *GetWindowHandle(void)
{
#if defined(_WIN32)
    // NOTE: Returned handle is: void *HWND (windows.h)
    return glfwGetWin32Window(window);
#elif defined(__linux__)
    // NOTE: Returned handle is: unsigned long Window (X.h)
    // typedef unsigned long XID;
    // typedef XID Window;
    //unsigned long id = (unsigned long)glfwGetX11Window(window);
    return NULL;    // TODO: Find a way to return value... cast to void *?
#elif defined(__APPLE__)
    // NOTE: Returned handle is: (objc_object *)
    return NULL;    // TODO: return (void *)glfwGetCocoaWindow(window);
#else
    return NULL;
#endif
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

// Get primary monitor width
int GetMonitorWidth(int monitor)
{
#if defined(PLATFORM_DESKTOP)
    int monitorCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);
        return mode->width;
    }
    else TraceLog(LOG_WARNING, "Selected monitor not found");
#endif
    return 0;
}

// Get primary monitor width
int GetMonitorHeight(int monitor)
{
#if defined(PLATFORM_DESKTOP)
    int monitorCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);
        return mode->height;
    }
    else TraceLog(LOG_WARNING, "Selected monitor not found");
#endif
    return 0;
}

// Get primary montior physical width in millimetres
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
    else TraceLog(LOG_WARNING, "Selected monitor not found");
#endif
    return 0;
}

// Get primary monitor physical height in millimetres
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
    else TraceLog(LOG_WARNING, "Selected monitor not found");
#endif
    return 0;
}

// Get the human-readable, UTF-8 encoded name of the primary monitor
const char *GetMonitorName(int monitor)
{
#if defined(PLATFORM_DESKTOP)
    int monitorCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        return glfwGetMonitorName(monitors[monitor]);
    }
    else TraceLog(LOG_WARNING, "Selected monitor not found");
#endif
    return "";
}

// Get clipboard text content
// NOTE: returned string is allocated and freed by GLFW
const char *GetClipboardText(void)
{
#if defined(PLATFORM_DESKTOP)
    return glfwGetClipboardString(window);
#else
    return NULL;
#endif
}

// Set clipboard text content
void SetClipboardText(const char *text)
{
#if defined(PLATFORM_DESKTOP)
    glfwSetClipboardString(window, text);
#endif
}

// Show mouse cursor
void ShowCursor(void)
{
#if defined(PLATFORM_DESKTOP)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
#endif
    cursorHidden = false;
}

// Hides mouse cursor
void HideCursor(void)
{
#if defined(PLATFORM_DESKTOP)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
#endif
    cursorHidden = true;
}

// Check if cursor is not visible
bool IsCursorHidden(void)
{
    return cursorHidden;
}

// Enables cursor (unlock cursor)
void EnableCursor(void)
{
#if defined(PLATFORM_DESKTOP)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
#endif
#if defined(PLATFORM_WEB)
    toggleCursorLock = true;
#endif
    cursorHidden = false;
}

// Disables cursor (lock cursor)
void DisableCursor(void)
{
#if defined(PLATFORM_DESKTOP)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif
#if defined(PLATFORM_WEB)
    toggleCursorLock = true;
#endif
    cursorHidden = true;
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
    currentTime = GetTime();            // Number of elapsed seconds since InitTimer()
    updateTime = currentTime - previousTime;
    previousTime = currentTime;

    rlLoadIdentity();                   // Reset current matrix (MODELVIEW)
    rlMultMatrixf(MatrixToFloat(downscaleView));       // If downscale required, apply it here

    //rlTranslatef(0.375, 0.375, 0);    // HACK to have 2D pixel-perfect drawing on OpenGL 1.1
                                        // NOTE: Not required with OpenGL 3.3+
}

// End canvas drawing and swap buffers (double buffering)
void EndDrawing(void)
{
    rlglDraw();                     // Draw Buffers (Only OpenGL 3+ and ES2)

#if defined(SUPPORT_GIF_RECORDING)

    #define GIF_RECORD_FRAMERATE    10

    if (gifRecording)
    {
        gifFramesCounter++;

        // NOTE: We record one gif frame every 10 game frames
        if ((gifFramesCounter%GIF_RECORD_FRAMERATE) == 0)
        {
            // Get image data for the current frame (from backbuffer)
            // NOTE: This process is very slow... :(
            unsigned char *screenData = rlReadScreenPixels(screenWidth, screenHeight);
            GifWriteFrame(screenData, screenWidth, screenHeight, 10, 8, false);

            free(screenData);   // Free image data
        }

        if (((gifFramesCounter/15)%2) == 1)
        {
            DrawCircle(30, screenHeight - 20, 10, RED);
            DrawText("RECORDING", 50, screenHeight - 25, 10, MAROON);
        }

        rlglDraw();                 // Draw RECORDING message
    }
#endif

    SwapBuffers();                  // Copy back buffer to front buffer
    PollInputEvents();              // Poll user events

    // Frame time control system
    currentTime = GetTime();
    drawTime = currentTime - previousTime;
    previousTime = currentTime;

    frameTime = updateTime + drawTime;

    // Wait for some milliseconds...
    if (frameTime < targetTime)
    {
        Wait((float)(targetTime - frameTime)*1000.0f);

        currentTime = GetTime();
        double extraTime = currentTime - previousTime;
        previousTime = currentTime;

        frameTime += extraTime;
    }
}

// Initialize 2D mode with custom camera (2D)
void BeginMode2D(Camera2D camera)
{
    rlglDraw();                         // Draw Buffers (Only OpenGL 3+ and ES2)

    rlLoadIdentity();                   // Reset current matrix (MODELVIEW)

    // Camera rotation and scaling is always relative to target
    Matrix matOrigin = MatrixTranslate(-camera.target.x, -camera.target.y, 0.0f);
    Matrix matRotation = MatrixRotate((Vector3){ 0.0f, 0.0f, 1.0f }, camera.rotation*DEG2RAD);
    Matrix matScale = MatrixScale(camera.zoom, camera.zoom, 1.0f);
    Matrix matTranslation = MatrixTranslate(camera.offset.x + camera.target.x, camera.offset.y + camera.target.y, 0.0f);

    Matrix matTransform = MatrixMultiply(MatrixMultiply(matOrigin, MatrixMultiply(matScale, matRotation)), matTranslation);

    rlMultMatrixf(MatrixToFloat(matTransform));  // Apply transformation to modelview
}

// Ends 2D mode with custom camera
void EndMode2D(void)
{
    rlglDraw();                         // Draw Buffers (Only OpenGL 3+ and ES2)

    rlLoadIdentity();                   // Reset current matrix (MODELVIEW)
}

// Initializes 3D mode with custom camera (3D)
void BeginMode3D(Camera3D camera)
{
    rlglDraw();                         // Draw Buffers (Only OpenGL 3+ and ES2)

    rlMatrixMode(RL_PROJECTION);        // Switch to projection matrix
    rlPushMatrix();                     // Save previous matrix, which contains the settings for the 2d ortho projection
    rlLoadIdentity();                   // Reset current matrix (PROJECTION)

    float aspect = (float)currentWidth/(float)currentHeight;

    if (camera.type == CAMERA_PERSPECTIVE)
    {
        // Setup perspective projection
        double top = 0.01*tan(camera.fovy*0.5*DEG2RAD);
        double right = top*aspect;

        rlFrustum(-right, right, -top, top, 0.01, 1000.0);
    }
    else if (camera.type == CAMERA_ORTHOGRAPHIC)
    {
        // Setup orthographic projection
        double top = camera.fovy/2.0;
        double right = top*aspect;

        rlOrtho(-right,right,-top,top, 0.01, 1000.0);
    }

    // NOTE: zNear and zFar values are important when computing depth buffer values

    rlMatrixMode(RL_MODELVIEW);         // Switch back to modelview matrix
    rlLoadIdentity();                   // Reset current matrix (MODELVIEW)

    // Setup Camera view
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
    rlMultMatrixf(MatrixToFloat(matView));      // Multiply MODELVIEW matrix by view matrix (camera)

    rlEnableDepthTest();                // Enable DEPTH_TEST for 3D
}

// Ends 3D mode and returns to default 2D orthographic mode
void EndMode3D(void)
{
    rlglDraw();                         // Process internal buffers (update + draw)

    rlMatrixMode(RL_PROJECTION);        // Switch to projection matrix
    rlPopMatrix();                      // Restore previous matrix (PROJECTION) from matrix stack

    rlMatrixMode(RL_MODELVIEW);         // Get back to modelview matrix
    rlLoadIdentity();                   // Reset current matrix (MODELVIEW)

    rlDisableDepthTest();               // Disable DEPTH_TEST for 2D
}

// Initializes render texture for drawing
void BeginTextureMode(RenderTexture2D target)
{
    rlglDraw();                         // Draw Buffers (Only OpenGL 3+ and ES2)

    rlEnableRenderTexture(target.id);   // Enable render target

    // Set viewport to framebuffer size
    rlViewport(0, 0, target.texture.width, target.texture.height);

    rlMatrixMode(RL_PROJECTION);        // Switch to PROJECTION matrix
    rlLoadIdentity();                   // Reset current matrix (PROJECTION)

    // Set orthographic projection to current framebuffer size
    // NOTE: Configured top-left corner as (0, 0)
    rlOrtho(0, target.texture.width, target.texture.height, 0, 0.0f, 1.0f);

    rlMatrixMode(RL_MODELVIEW);         // Switch back to MODELVIEW matrix
    rlLoadIdentity();                   // Reset current matrix (MODELVIEW)

    //rlScalef(0.0f, -1.0f, 0.0f);      // Flip Y-drawing (?)

    // Setup current width/height for proper aspect ratio
    // calculation when using BeginMode3D()
    currentWidth = target.texture.width;
    currentHeight = target.texture.height;
}

// Ends drawing to render texture
void EndTextureMode(void)
{
    rlglDraw();                         // Draw Buffers (Only OpenGL 3+ and ES2)

    rlDisableRenderTexture();           // Disable render target

    // Set viewport to default framebuffer size (screen size)
    SetupViewport();

    rlMatrixMode(RL_PROJECTION);        // Switch to PROJECTION matrix
    rlLoadIdentity();                   // Reset current matrix (PROJECTION)

    // Set orthographic projection to current framebuffer size
    // NOTE: Configured top-left corner as (0, 0)
    rlOrtho(0, GetScreenWidth(), GetScreenHeight(), 0, 0.0f, 1.0f);

    rlMatrixMode(RL_MODELVIEW);         // Switch back to MODELVIEW matrix
    rlLoadIdentity();                   // Reset current matrix (MODELVIEW)

    // Reset current screen size
    currentWidth = GetScreenWidth();
    currentHeight = GetScreenHeight();
}

// Returns a ray trace from mouse position
Ray GetMouseRay(Vector2 mousePosition, Camera camera)
{
    Ray ray;

    // Calculate normalized device coordinates
    // NOTE: y value is negative
    float x = (2.0f*mousePosition.x)/(float)GetScreenWidth() - 1.0f;
    float y = 1.0f - (2.0f*mousePosition.y)/(float)GetScreenHeight();
    float z = 1.0f;

    // Store values in a vector
    Vector3 deviceCoords = { x, y, z };

    // Calculate view matrix from camera look at
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);

    Matrix matProj = MatrixIdentity();

    if (camera.type == CAMERA_PERSPECTIVE)
    {
        // Calculate projection matrix from perspective
        matProj = MatrixPerspective(camera.fovy*DEG2RAD, ((double)GetScreenWidth()/(double)GetScreenHeight()), 0.01, 1000.0);
    }
    else if (camera.type == CAMERA_ORTHOGRAPHIC)
    {
        float aspect = (float)screenWidth/(float)screenHeight;
        double top = camera.fovy/2.0;
        double right = top*aspect;

        // Calculate projection matrix from orthographic
        matProj = MatrixOrtho(-right, right, -top, top, 0.01, 1000.0);
    }

    // Unproject far/near points
    Vector3 nearPoint = rlUnproject((Vector3){ deviceCoords.x, deviceCoords.y, 0.0f }, matProj, matView);
    Vector3 farPoint = rlUnproject((Vector3){ deviceCoords.x, deviceCoords.y, 1.0f }, matProj, matView);

    // Unproject the mouse cursor in the near plane.
    // We need this as the source position because orthographic projects, compared to perspect doesn't have a
    // convergence point, meaning that the "eye" of the camera is more like a plane than a point.
    Vector3 cameraPlanePointerPos = rlUnproject((Vector3){ deviceCoords.x, deviceCoords.y, -1.0f }, matProj, matView);

    // Calculate normalized direction vector
    Vector3 direction = Vector3Normalize(Vector3Subtract(farPoint, nearPoint));

    if (camera.type == CAMERA_PERSPECTIVE) ray.position = camera.position;
    else if (camera.type == CAMERA_ORTHOGRAPHIC) ray.position = cameraPlanePointerPos;

    // Apply calculated vectors to ray
    ray.direction = direction;

    return ray;
}

// Returns the screen space position from a 3d world space position
Vector2 GetWorldToScreen(Vector3 position, Camera camera)
{
    // Calculate projection matrix (from perspective instead of frustum
    Matrix matProj = MatrixIdentity();

    if (camera.type == CAMERA_PERSPECTIVE)
    {
        // Calculate projection matrix from perspective
        matProj = MatrixPerspective(camera.fovy*DEG2RAD, ((double)GetScreenWidth()/(double)GetScreenHeight()), 0.01, 1000.0);
    }
    else if (camera.type == CAMERA_ORTHOGRAPHIC)
    {
        float aspect = (float)screenWidth/(float)screenHeight;
        double top = camera.fovy/2.0;
        double right = top*aspect;

        // Calculate projection matrix from orthographic
        matProj = MatrixOrtho(-right, right, -top, top, 0.01, 1000.0);
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
    Vector2 screenPosition = { (ndcPos.x + 1.0f)/2.0f*(float)GetScreenWidth(), (ndcPos.y + 1.0f)/2.0f*(float)GetScreenHeight() };

    return screenPosition;
}

// Get transform matrix for camera
Matrix GetCameraMatrix(Camera camera)
{
    return MatrixLookAt(camera.position, camera.target, camera.up);
}

// Set target FPS (maximum)
void SetTargetFPS(int fps)
{
    if (fps < 1) targetTime = 0.0;
    else targetTime = 1.0/(double)fps;

    TraceLog(LOG_INFO, "Target time per frame: %02.03f milliseconds", (float)targetTime*1000);
}

// Returns current FPS
int GetFPS(void)
{
    return (int)(1.0f/GetFrameTime());
}

// Returns time in seconds for last frame drawn
float GetFrameTime(void)
{
    // NOTE: We round value to milliseconds
    return (float)frameTime;
}

// Get elapsed time measure in seconds since InitTimer()
// NOTE: On PLATFORM_DESKTOP InitTimer() is called on InitWindow()
// NOTE: On PLATFORM_DESKTOP, timer is initialized on glfwInit()
double GetTime(void)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    return glfwGetTime();                   // Elapsed time since glfwInit()
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t time = (uint64_t)ts.tv_sec*1000000000LLU + (uint64_t)ts.tv_nsec;

    return (double)(time - baseTime)*1e-9;  // Elapsed time since InitTimer()
#endif
}

// Returns hexadecimal value for a Color
int ColorToInt(Color color)
{
    return (((int)color.r << 24) | ((int)color.g << 16) | ((int)color.b << 8) | (int)color.a);
}

// Returns color normalized as float [0..1]
Vector4 ColorNormalize(Color color)
{
    Vector4 result;

    result.x = (float)color.r/255.0f;
    result.y = (float)color.g/255.0f;
    result.z = (float)color.b/255.0f;
    result.w = (float)color.a/255.0f;

    return result;
}

// Returns HSV values for a Color
// NOTE: Hue is returned as degrees [0..360]
Vector3 ColorToHSV(Color color)
{
    Vector3 rgb = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };
    Vector3 hsv = { 0.0f, 0.0f, 0.0f };
    float min, max, delta;

    min = rgb.x < rgb.y? rgb.x : rgb.y;
    min = min  < rgb.z? min  : rgb.z;

    max = rgb.x > rgb.y? rgb.x : rgb.y;
    max = max  > rgb.z? max  : rgb.z;

    hsv.z = max;            // Value
    delta = max - min;

    if (delta < 0.00001f)
    {
        hsv.y = 0.0f;
        hsv.x = 0.0f;       // Undefined, maybe NAN?
        return hsv;
    }

    if (max > 0.0f)
    {
        // NOTE: If max is 0, this divide would cause a crash
        hsv.y = (delta/max);    // Saturation
    }
    else
    {
        // NOTE: If max is 0, then r = g = b = 0, s = 0, h is undefined
        hsv.y = 0.0f;
        hsv.x = NAN;        // Undefined
        return hsv;
    }

    // NOTE: Comparing float values could not work properly
    if (rgb.x >= max) hsv.x = (rgb.y - rgb.z)/delta;    // Between yellow & magenta
    else
    {
        if (rgb.y >= max) hsv.x = 2.0f + (rgb.z - rgb.x)/delta;  // Between cyan & yellow
        else hsv.x = 4.0f + (rgb.x - rgb.y)/delta;      // Between magenta & cyan
    }

    hsv.x *= 60.0f;     // Convert to degrees

    if (hsv.x < 0.0f) hsv.x += 360.0f;

    return hsv;
}

// Returns a Color from HSV values
// Implementation reference: https://en.wikipedia.org/wiki/HSL_and_HSV#Alternative_HSV_conversion
// NOTE: Color->HSV->Color conversion will not yield exactly the same color due to rounding errors
Color ColorFromHSV(Vector3 hsv)
{
    Color color = { 0, 0, 0, 255 };
    float h = hsv.x, s = hsv.y, v = hsv.z;

    // Red channel
    float k = fmod((5.0f + h/60.0f), 6);
    float t = 4.0f - k;
    k = (t < k)? t : k;
    k = (k < 1)? k : 1;
    k = (k > 0)? k : 0;
    color.r = (v - v*s*k)*255;

    // Green channel
    k = fmod((3.0f + h/60.0f), 6);
    t = 4.0f - k;
    k = (t < k)? t : k;
    k = (k < 1)? k : 1;
    k = (k > 0)? k : 0;
    color.g = (v - v*s*k)*255;

    // Blue channel
    k = fmod((1.0f + h/60.0f), 6);
    t = 4.0f - k;
    k = (t < k)? t : k;
    k = (k < 1)? k : 1;
    k = (k > 0)? k : 0;
    color.b = (v - v*s*k)*255;

    return color;
}

// Returns a Color struct from hexadecimal value
Color GetColor(int hexValue)
{
    Color color;

    color.r = (unsigned char)(hexValue >> 24) & 0xFF;
    color.g = (unsigned char)(hexValue >> 16) & 0xFF;
    color.b = (unsigned char)(hexValue >> 8) & 0xFF;
    color.a = (unsigned char)hexValue & 0xFF;

    return color;
}

// Returns a random value between min and max (both included)
int GetRandomValue(int min, int max)
{
    if (min > max)
    {
        int tmp = max;
        max = min;
        min = tmp;
    }

    return (rand()%(abs(max - min) + 1) + min);
}

// Color fade-in or fade-out, alpha goes from 0.0f to 1.0f
Color Fade(Color color, float alpha)
{
    if (alpha < 0.0f) alpha = 0.0f;
    else if (alpha > 1.0f) alpha = 1.0f;

    return (Color){color.r, color.g, color.b, (unsigned char)(255.0f*alpha)};
}

// Setup window configuration flags (view FLAGS)
void SetConfigFlags(unsigned char flags)
{
    configFlags = flags;

    if (configFlags & FLAG_SHOW_LOGO) showLogo = true;
    if (configFlags & FLAG_FULLSCREEN_MODE) fullscreen = true;
}

// NOTE TraceLog() function is located in [utils.h]

// Takes a screenshot of current screen (saved a .png)
// NOTE: This function could work in any platform but some platforms: PLATFORM_ANDROID and PLATFORM_WEB
// have their own internal file-systems, to dowload image to user file-system some additional mechanism is required
void TakeScreenshot(const char *fileName)
{
    unsigned char *imgData = rlReadScreenPixels(renderWidth, renderHeight);
    Image image = { imgData, renderWidth, renderHeight, 1, UNCOMPRESSED_R8G8B8A8 };

    char path[512] = { 0 };
#if defined(PLATFORM_ANDROID)
    strcpy(path, internalDataPath);
    strcat(path, "/");
    strcat(path, fileName);
#else
    strcpy(path, fileName);
#endif

    ExportImage(image, path);
    free(imgData);

#if defined(PLATFORM_WEB)
    // Download file from MEMFS (emscripten memory filesystem)
    // SaveFileFromMEMFSToDisk() function is defined in raylib/templates/web_shel/shell.html
    emscripten_run_script(TextFormat("SaveFileFromMEMFSToDisk('%s','%s')", GetFileName(path), GetFileName(path)));
#endif

    TraceLog(LOG_INFO, "Screenshot taken: %s", path);
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

    return result;
}

// Check file extension
bool IsFileExtension(const char *fileName, const char *ext)
{
    bool result = false;
    const char *fileExt;

    if ((fileExt = strrchr(fileName, '.')) != NULL)
    {
#if defined(_WIN32)
        result = true;
        int extLen = strlen(ext);

        if (strlen(fileExt) == extLen)
        {
            for (int i = 0; i < extLen; i++)
            {
                if (tolower(fileExt[i]) != tolower(ext[i]))
                {
                    result = false;
                    break;
                }
            }
        }
        else result = false;
#else
        if (strcmp(fileExt, ext) == 0) result = true;
#endif
    }

    return result;
}

// Get pointer to extension for a filename string
const char *GetExtension(const char *fileName)
{
    const char *dot = strrchr(fileName, '.');

    if (!dot || dot == fileName) return NULL;

    return (dot + 1);
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
    const char *fileName = strprbrk(filePath, "\\/");

    if (!fileName || fileName == filePath) return filePath;

    return fileName + 1;
}

// Get filename string without extension (memory should be freed)
const char *GetFileNameWithoutExt(const char *filePath)
{
    #define MAX_FILENAMEWITHOUTEXT_LENGTH   64

    static char fileName[MAX_FILENAMEWITHOUTEXT_LENGTH];
    memset(fileName, 0, MAX_FILENAMEWITHOUTEXT_LENGTH);

    strcpy(fileName, GetFileName(filePath));   // Get filename with extension

    int len = strlen(fileName);

    for (int i = 0; (i < len) && (i < MAX_FILENAMEWITHOUTEXT_LENGTH); i++)
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

// Get directory for a given fileName (with path)
const char *GetDirectoryPath(const char *fileName)
{
    const char *lastSlash = NULL;
    static char filePath[MAX_FILEPATH_LENGTH];
    memset(filePath, 0, MAX_FILEPATH_LENGTH);

    lastSlash = strprbrk(fileName, "\\/");
    if (!lastSlash) return NULL;

    // NOTE: Be careful, strncpy() is not safe, it does not care about '\0'
    strncpy(filePath, fileName, strlen(fileName) - (strlen(lastSlash) - 1));
    filePath[strlen(fileName) - strlen(lastSlash)] = '\0';  // Add '\0' manually

    return filePath;
}

// Get current working directory
const char *GetWorkingDirectory(void)
{
    static char currentDir[MAX_FILEPATH_LENGTH];
    memset(currentDir, 0, MAX_FILEPATH_LENGTH);

    GETCWD(currentDir, MAX_FILEPATH_LENGTH - 1);

    return currentDir;
}

// Get filenames in a directory path (max 512 files)
// NOTE: Files count is returned by parameters pointer
char **GetDirectoryFiles(const char *dirPath, int *fileCount)
{
    #define MAX_DIRECTORY_FILES     512

    ClearDirectoryFiles();

    // Memory allocation for MAX_DIRECTORY_FILES
    dirFilesPath = (char **)malloc(sizeof(char *)*MAX_DIRECTORY_FILES);
    for (int i = 0; i < MAX_DIRECTORY_FILES; i++) dirFilesPath[i] = (char *)malloc(sizeof(char)*MAX_FILEPATH_LENGTH);

    int counter = 0;
    struct dirent *ent;
    DIR *dir = opendir(dirPath);

    if (dir != NULL)  // It's a directory
    {
        // TODO: Reading could be done in two passes,
        // first one to count files and second one to read names
        // That way we can allocate required memory, instead of a limited pool

        while ((ent = readdir(dir)) != NULL)
        {
            strcpy(dirFilesPath[counter], ent->d_name);
            counter++;
        }

        closedir(dir);
    }
    else TraceLog(LOG_WARNING, "Can not open directory...\n"); // Maybe it's a file...

    dirFilesCount = counter;
    *fileCount = dirFilesCount;

    return dirFilesPath;
}

// Clear directory files paths buffers
void ClearDirectoryFiles(void)
{
    if (dirFilesCount > 0)
    {
        for (int i = 0; i < dirFilesCount; i++) free(dirFilesPath[i]);

        free(dirFilesPath);
        dirFilesCount = 0;
    }
}

// Change working directory, returns true if success
bool ChangeDirectory(const char *dir)
{
    return (CHDIR(dir) == 0);
}

// Check if a file has been dropped into window
bool IsFileDropped(void)
{
    if (dropFilesCount > 0) return true;
    else return false;
}

// Get dropped files names
char **GetDroppedFiles(int *count)
{
    *count = dropFilesCount;
    return dropFilesPath;
}

// Clear dropped files paths buffer
void ClearDroppedFiles(void)
{
    if (dropFilesCount > 0)
    {
        for (int i = 0; i < dropFilesCount; i++) free(dropFilesPath[i]);

        free(dropFilesPath);

        dropFilesCount = 0;
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

// Save integer value to storage file (to defined position)
// NOTE: Storage positions is directly related to file memory layout (4 bytes each integer)
void StorageSaveValue(int position, int value)
{
    FILE *storageFile = NULL;

    char path[512] = { 0 };
#if defined(PLATFORM_ANDROID)
    strcpy(path, internalDataPath);
    strcat(path, "/");
    strcat(path, STORAGE_FILENAME);
#else
    strcpy(path, STORAGE_FILENAME);
#endif

    // Try open existing file to append data
    storageFile = fopen(path, "rb+");

    // If file doesn't exist, create a new storage data file
    if (!storageFile) storageFile = fopen(path, "wb");

    if (!storageFile) TraceLog(LOG_WARNING, "Storage data file could not be created");
    else
    {
        // Get file size
        fseek(storageFile, 0, SEEK_END);
        int fileSize = ftell(storageFile);  // Size in bytes
        fseek(storageFile, 0, SEEK_SET);

        if (fileSize < (position*sizeof(int))) TraceLog(LOG_WARNING, "Storage position could not be found");
        else
        {
            fseek(storageFile, (position*sizeof(int)), SEEK_SET);
            fwrite(&value, 1, sizeof(int), storageFile);
        }

        fclose(storageFile);
    }
}

// Load integer value from storage file (from defined position)
// NOTE: If requested position could not be found, value 0 is returned
int StorageLoadValue(int position)
{
    int value = 0;

    char path[512] = { 0 };
#if defined(PLATFORM_ANDROID)
    strcpy(path, internalDataPath);
    strcat(path, "/");
    strcat(path, STORAGE_FILENAME);
#else
    strcpy(path, STORAGE_FILENAME);
#endif

    // Try open existing file to append data
    FILE *storageFile = fopen(path, "rb");

    if (!storageFile) TraceLog(LOG_WARNING, "Storage data file could not be found");
    else
    {
        // Get file size
        fseek(storageFile, 0, SEEK_END);
        int fileSize = ftell(storageFile);      // Size in bytes
        rewind(storageFile);

        if (fileSize < (position*4)) TraceLog(LOG_WARNING, "Storage position could not be found");
        else
        {
            fseek(storageFile, (position*4), SEEK_SET);
            fread(&value, 4, 1, storageFile);   // Read 1 element of 4 bytes size
        }

        fclose(storageFile);
    }

    return value;
}

// Open URL with default system browser (if available)
// NOTE: This function is onlyl safe to use if you control the URL given.
// A user could craft a malicious string performing another action.
// Only call this function yourself not with user input or make sure to check the string yourself.
// CHECK: https://github.com/raysan5/raylib/issues/686
void OpenURL(const char *url)
{
    // Small security check trying to avoid (partially) malicious code...
    // sorry for the inconvenience when you hit this point...
    if (strchr(url, '\'') != NULL)
    {
        TraceLog(LOG_WARNING, "Provided URL does not seem to be valid.");
    }
    else
    {
        char *cmd = (char *)calloc(strlen(url) + 10, sizeof(char));

#if defined(_WIN32)
        sprintf(cmd, "explorer %s", url);
#elif defined(__linux__)
        sprintf(cmd, "xdg-open '%s'", url); // Alternatives: firefox, x-www-browser
#elif defined(__APPLE__)
        sprintf(cmd, "open '%s'", url);
#endif
        system(cmd);
        free(cmd);
    }
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Input (Keyboard, Mouse, Gamepad) Functions
//----------------------------------------------------------------------------------
// Detect if a key has been pressed once
bool IsKeyPressed(int key)
{
    bool pressed = false;

    if ((currentKeyState[key] != previousKeyState[key]) && (currentKeyState[key] == 1)) pressed = true;
    else pressed = false;

    return pressed;
}

// Detect if a key is being pressed (key held down)
bool IsKeyDown(int key)
{
    if (GetKeyStatus(key) == 1) return true;
    else return false;
}

// Detect if a key has been released once
bool IsKeyReleased(int key)
{
    bool released = false;

    if ((currentKeyState[key] != previousKeyState[key]) && (currentKeyState[key] == 0)) released = true;
    else released = false;

    return released;
}

// Detect if a key is NOT being pressed (key not held down)
bool IsKeyUp(int key)
{
    if (GetKeyStatus(key) == 0) return true;
    else return false;
}

// Get the last key pressed
int GetKeyPressed(void)
{
    return lastKeyPressed;
}

// Set a custom key to exit program
// NOTE: default exitKey is ESCAPE
void SetExitKey(int key)
{
#if !defined(PLATFORM_ANDROID)
    exitKey = key;
#endif
}

// NOTE: Gamepad support not implemented in emscripten GLFW3 (PLATFORM_WEB)

// Detect if a gamepad is available
bool IsGamepadAvailable(int gamepad)
{
    bool result = false;

#if !defined(PLATFORM_ANDROID)
    if ((gamepad < MAX_GAMEPADS) && gamepadReady[gamepad]) result = true;
#endif

    return result;
}

// Check gamepad name (if available)
bool IsGamepadName(int gamepad, const char *name)
{
    bool result = false;

#if !defined(PLATFORM_ANDROID)
    const char *gamepadName = NULL;

    if (gamepadReady[gamepad]) gamepadName = GetGamepadName(gamepad);
    if ((name != NULL) && (gamepadName != NULL)) result = (strcmp(name, gamepadName) == 0);
#endif

    return result;
}

// Return gamepad internal name id
const char *GetGamepadName(int gamepad)
{
#if defined(PLATFORM_DESKTOP)
    if (gamepadReady[gamepad]) return glfwGetJoystickName(gamepad);
    else return NULL;
#elif defined(PLATFORM_RPI)
    if (gamepadReady[gamepad]) ioctl(gamepadStream[gamepad], JSIOCGNAME(64), &gamepadName);

    return gamepadName;
#else
    return NULL;
#endif
}

// Return gamepad axis count
int GetGamepadAxisCount(int gamepad)
{
#if defined(PLATFORM_RPI)
    int axisCount = 0;
    if (gamepadReady[gamepad]) ioctl(gamepadStream[gamepad], JSIOCGAXES, &axisCount);
    gamepadAxisCount = axisCount;
#endif
    return gamepadAxisCount;
}

// Return axis movement vector for a gamepad
float GetGamepadAxisMovement(int gamepad, int axis)
{
    float value = 0;

#if !defined(PLATFORM_ANDROID)
    if ((gamepad < MAX_GAMEPADS) && gamepadReady[gamepad] && (axis < MAX_GAMEPAD_AXIS)) value = gamepadAxisState[gamepad][axis];
#endif

    return value;
}

// Detect if a gamepad button has been pressed once
bool IsGamepadButtonPressed(int gamepad, int button)
{
    bool pressed = false;

#if !defined(PLATFORM_ANDROID)
    if ((gamepad < MAX_GAMEPADS) && gamepadReady[gamepad] && (button < MAX_GAMEPAD_BUTTONS) &&
        (currentGamepadState[gamepad][button] != previousGamepadState[gamepad][button]) &&
        (currentGamepadState[gamepad][button] == 1)) pressed = true;
#endif

    return pressed;
}

// Detect if a gamepad button is being pressed
bool IsGamepadButtonDown(int gamepad, int button)
{
    bool result = false;

#if !defined(PLATFORM_ANDROID)
    if ((gamepad < MAX_GAMEPADS) && gamepadReady[gamepad] && (button < MAX_GAMEPAD_BUTTONS) &&
        (currentGamepadState[gamepad][button] == 1)) result = true;
#endif

    return result;
}

// Detect if a gamepad button has NOT been pressed once
bool IsGamepadButtonReleased(int gamepad, int button)
{
    bool released = false;

#if !defined(PLATFORM_ANDROID)
    if ((gamepad < MAX_GAMEPADS) && gamepadReady[gamepad] && (button < MAX_GAMEPAD_BUTTONS) &&
        (currentGamepadState[gamepad][button] != previousGamepadState[gamepad][button]) &&
        (currentGamepadState[gamepad][button] == 0)) released = true;
#endif

    return released;
}

// Detect if a mouse button is NOT being pressed
bool IsGamepadButtonUp(int gamepad, int button)
{
    bool result = false;

#if !defined(PLATFORM_ANDROID)
    if ((gamepad < MAX_GAMEPADS) && gamepadReady[gamepad] && (button < MAX_GAMEPAD_BUTTONS) &&
        (currentGamepadState[gamepad][button] == 0)) result = true;
#endif

    return result;
}

// Get the last gamepad button pressed
int GetGamepadButtonPressed(void)
{
    return lastGamepadButtonPressed;
}

// Detect if a mouse button has been pressed once
bool IsMouseButtonPressed(int button)
{
    bool pressed = false;

// TODO: Review, gestures could be not supported despite being on Android platform!
#if defined(PLATFORM_ANDROID)
    if (IsGestureDetected(GESTURE_TAP)) pressed = true;
#else
    if ((currentMouseState[button] != previousMouseState[button]) && (currentMouseState[button] == 1)) pressed = true;
#endif

    return pressed;
}

// Detect if a mouse button is being pressed
bool IsMouseButtonDown(int button)
{
    bool down = false;

#if defined(PLATFORM_ANDROID)
    if (IsGestureDetected(GESTURE_HOLD)) down = true;
#else
    if (GetMouseButtonStatus(button) == 1) down = true;
#endif

    return down;
}

// Detect if a mouse button has been released once
bool IsMouseButtonReleased(int button)
{
    bool released = false;

#if !defined(PLATFORM_ANDROID)
    if ((currentMouseState[button] != previousMouseState[button]) && (currentMouseState[button] == 0)) released = true;
#endif

    return released;
}

// Detect if a mouse button is NOT being pressed
bool IsMouseButtonUp(int button)
{
    bool up = false;

#if !defined(PLATFORM_ANDROID)
    if (GetMouseButtonStatus(button) == 0) up = true;
#endif

    return up;
}

// Returns mouse position X
int GetMouseX(void)
{
#if defined(PLATFORM_ANDROID)
    return (int)touchPosition[0].x;
#else
    return (int)((mousePosition.x + mouseOffset.x)*mouseScale.x);
#endif
}

// Returns mouse position Y
int GetMouseY(void)
{
#if defined(PLATFORM_ANDROID)
    return (int)touchPosition[0].x;
#else
    return (int)((mousePosition.y + mouseOffset.y)*mouseScale.y);
#endif
}

// Returns mouse position XY
Vector2 GetMousePosition(void)
{
#if defined(PLATFORM_ANDROID)
    return GetTouchPosition(0);
#else
    return (Vector2){ (mousePosition.x + mouseOffset.x)*mouseScale.x, (mousePosition.y + mouseOffset.y)*mouseScale.y };
#endif
}

// Set mouse position XY
void SetMousePosition(int x, int y)
{
    mousePosition = (Vector2){ (float)x, (float)y };
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    // NOTE: emscripten not implemented
    glfwSetCursorPos(window, mousePosition.x, mousePosition.y);
#endif
}

// Set mouse offset
// NOTE: Useful when rendering to different size targets
void SetMouseOffset(int offsetX, int offsetY)
{
    mouseOffset = (Vector2){ (float)offsetX, (float)offsetY };
}

// Set mouse scaling
// NOTE: Useful when rendering to different size targets
void SetMouseScale(float scaleX, float scaleY)
{
    mouseScale = (Vector2){ scaleX, scaleY };
}

// Returns mouse wheel movement Y
int GetMouseWheelMove(void)
{
#if defined(PLATFORM_ANDROID)
    return 0;
#elif defined(PLATFORM_WEB)
    return previousMouseWheelY/100;
#else
    return previousMouseWheelY;
#endif
}

// Returns touch position X for touch point 0 (relative to screen size)
int GetTouchX(void)
{
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
    return (int)touchPosition[0].x;
#else   // PLATFORM_DESKTOP, PLATFORM_RPI
    return GetMouseX();
#endif
}

// Returns touch position Y for touch point 0 (relative to screen size)
int GetTouchY(void)
{
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
    return (int)touchPosition[0].y;
#else   // PLATFORM_DESKTOP, PLATFORM_RPI
    return GetMouseY();
#endif
}

// Returns touch position XY for a touch point index (relative to screen size)
// TODO: Touch position should be scaled depending on display size and render size
Vector2 GetTouchPosition(int index)
{
    Vector2 position = { -1.0f, -1.0f };

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
    if (index < MAX_TOUCH_POINTS) position = touchPosition[index];
    else TraceLog(LOG_WARNING, "Required touch point out of range (Max touch points: %i)", MAX_TOUCH_POINTS);

    if ((screenWidth > displayWidth) || (screenHeight > displayHeight))
    {
        // TODO: Review touch position scaling for screenSize vs displaySize
        position.x = position.x*((float)screenWidth/(float)(displayWidth - renderOffsetX)) - renderOffsetX/2;
        position.y = position.y*((float)screenHeight/(float)(displayHeight - renderOffsetY)) - renderOffsetY/2;
    }
    else
    {
        position.x = position.x*((float)renderWidth/(float)displayWidth) - renderOffsetX/2;
        position.y = position.y*((float)renderHeight/(float)displayHeight) - renderOffsetY/2;
    }
#elif defined(PLATFORM_RPI)
    position = touchPosition[index];
#else   // PLATFORM_DESKTOP
    if (index == 0) position = GetMousePosition();
#endif

    return position;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Initialize display device and framebuffer
// NOTE: width and height represent the screen (framebuffer) desired size, not actual display size
// If width or height are 0, default display size will be used for framebuffer size
// NOTE: returns false in case graphic device could not be created
static bool InitGraphicsDevice(int width, int height)
{
    screenWidth = width;        // User desired width
    screenHeight = height;      // User desired height
    currentWidth = width;
    currentHeight = height;

    // NOTE: Framebuffer (render area - renderWidth, renderHeight) could include black bars...
    // ...in top-down or left-right to match display aspect ratio (no weird scalings)

    // Downscale matrix is required in case desired screen area is bigger than display area
    downscaleView = MatrixIdentity();

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    glfwSetErrorCallback(ErrorCallback);

#if defined(__APPLE__)
    glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif

    if (!glfwInit())
    {
        TraceLog(LOG_WARNING, "Failed to initialize GLFW");
        return false;
    }

    // NOTE: Getting video modes is not implemented in emscripten GLFW3 version
#if defined(PLATFORM_DESKTOP)
    // Find monitor resolution
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    if (!monitor)
    {
        TraceLog(LOG_WARNING, "Failed to get monitor");
        return false;
    }
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    displayWidth = mode->width;
    displayHeight = mode->height;

    // Screen size security check
    if (screenWidth <= 0) screenWidth = displayWidth;
    if (screenHeight <= 0) screenHeight = displayHeight;
#endif  // defined(PLATFORM_DESKTOP)

#if defined(PLATFORM_WEB)
    displayWidth = screenWidth;
    displayHeight = screenHeight;
#endif  // defined(PLATFORM_WEB)

    glfwDefaultWindowHints();                   // Set default windows hints:
    //glfwWindowHint(GLFW_RED_BITS, 8);             // Framebuffer red color component bits
    //glfwWindowHint(GLFW_GREEN_BITS, 8);           // Framebuffer green color component bits
    //glfwWindowHint(GLFW_BLUE_BITS, 8);            // Framebuffer blue color component bits
    //glfwWindowHint(GLFW_ALPHA_BITS, 8);           // Framebuffer alpha color component bits
    //glfwWindowHint(GLFW_DEPTH_BITS, 24);          // Depthbuffer bits
    //glfwWindowHint(GLFW_REFRESH_RATE, 0);         // Refresh rate for fullscreen window
    //glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API); // OpenGL API to use. Alternative: GLFW_OPENGL_ES_API
    //glfwWindowHint(GLFW_AUX_BUFFERS, 0);          // Number of auxiliar buffers

    // Check some Window creation flags
    if (configFlags & FLAG_WINDOW_HIDDEN) glfwWindowHint(GLFW_VISIBLE, GL_FALSE);           // Visible window
    else glfwWindowHint(GLFW_VISIBLE, GL_TRUE);     // Window initially hidden

    if (configFlags & FLAG_WINDOW_RESIZABLE) glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);       // Resizable window
    else glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);  // Avoid window being resizable

    if (configFlags & FLAG_WINDOW_UNDECORATED) glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);  // Border and buttons on Window
    else glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);   // Decorated window
    // FLAG_WINDOW_TRANSPARENT not supported on HTML5 and not included in any released GLFW version yet
#if defined(GLFW_TRANSPARENT_FRAMEBUFFER)
    if (configFlags & FLAG_WINDOW_TRANSPARENT) glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);     // Transparent framebuffer
    else glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);  // Opaque framebuffer
#endif

    if (configFlags & FLAG_MSAA_4X_HINT) glfwWindowHint(GLFW_SAMPLES, 4);   // Tries to enable multisampling x4 (MSAA), default is 0

    // NOTE: When asking for an OpenGL context version, most drivers provide highest supported version
    // with forward compatibility to older OpenGL versions.
    // For example, if using OpenGL 1.1, driver can provide a 4.3 context forward compatible.

    // Check selection OpenGL version
    if (rlGetVersion() == OPENGL_21)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);          // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);          // Choose OpenGL minor version (just hint)
    }
    else if (rlGetVersion() == OPENGL_33)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);          // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);          // Choose OpenGL minor version (just hint)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Profiles Hint: Only 3.3 and above!
                                                                       // Values: GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_ANY_PROFILE, GLFW_OPENGL_COMPAT_PROFILE
#if defined(__APPLE__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);  // OSX Requires fordward compatibility
#else
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE); // Fordward Compatibility Hint: Only 3.3 and above!
#endif
        //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); // Request OpenGL DEBUG context
    }
    else if (rlGetVersion() == OPENGL_ES_20)                    // Request OpenGL ES 2.0 context
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);     // Alternative: GLFW_EGL_CONTEXT_API (ANGLE)
    }

    if (fullscreen)
    {
        // Obtain recommended displayWidth/displayHeight from a valid videomode for the monitor
        int count;
        const GLFWvidmode *modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);

        // Get closest videomode to desired screenWidth/screenHeight
        for (int i = 0; i < count; i++)
        {
            if (modes[i].width >= screenWidth)
            {
                if (modes[i].height >= screenHeight)
                {
                    displayWidth = modes[i].width;
                    displayHeight = modes[i].height;
                    break;
                }
            }
        }

        TraceLog(LOG_WARNING, "Closest fullscreen videomode: %i x %i", displayWidth, displayHeight);

        // NOTE: ISSUE: Closest videomode could not match monitor aspect-ratio, for example,
        // for a desired screen size of 800x450 (16:9), closest supported videomode is 800x600 (4:3),
        // framebuffer is rendered correctly but once displayed on a 16:9 monitor, it gets stretched
        // by the sides to fit all monitor space...

        // At this point we need to manage render size vs screen size
        // NOTE: This function uses and modifies global module variables:
        //       screenWidth/screenHeight - renderWidth/renderHeight - downscaleView
        SetupFramebuffer(displayWidth, displayHeight);

        window = glfwCreateWindow(displayWidth, displayHeight, windowTitle, glfwGetPrimaryMonitor(), NULL);

        // NOTE: Full-screen change, not working properly...
        //glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, screenWidth, screenHeight, GLFW_DONT_CARE);
    }
    else
    {
        // No-fullscreen window creation
        window = glfwCreateWindow(screenWidth, screenHeight, windowTitle, NULL, NULL);

        if (window)
        {
#if defined(PLATFORM_DESKTOP)
            // Center window on screen
            int windowPosX = displayWidth/2 - screenWidth/2;
            int windowPosY = displayHeight/2 - screenHeight/2;

            if (windowPosX < 0) windowPosX = 0;
            if (windowPosY < 0) windowPosY = 0;

            glfwSetWindowPos(window, windowPosX, windowPosY);
#endif
            renderWidth = screenWidth;
            renderHeight = screenHeight;
        }
    }

    if (!window)
    {
        glfwTerminate();
        TraceLog(LOG_WARNING, "GLFW Failed to initialize Window");
        return false;
    }
    else
    {
        TraceLog(LOG_INFO, "Display device initialized successfully");
#if defined(PLATFORM_DESKTOP)
        TraceLog(LOG_INFO, "Display size: %i x %i", displayWidth, displayHeight);
#endif
        TraceLog(LOG_INFO, "Render size: %i x %i", renderWidth, renderHeight);
        TraceLog(LOG_INFO, "Screen size: %i x %i", screenWidth, screenHeight);
        TraceLog(LOG_INFO, "Viewport offsets: %i, %i", renderOffsetX, renderOffsetY);
    }

    glfwSetWindowSizeCallback(window, WindowSizeCallback);      // NOTE: Resizing not allowed by default!
    glfwSetCursorEnterCallback(window, CursorEnterCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, MouseCursorPosCallback);   // Track mouse position changes
    glfwSetCharCallback(window, CharCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetWindowIconifyCallback(window, WindowIconifyCallback);
    glfwSetDropCallback(window, WindowDropCallback);

    glfwMakeContextCurrent(window);

    // Try to disable GPU V-Sync by default, set framerate using SetTargetFPS()
    // NOTE: V-Sync can be enabled by graphic driver configuration
#if !defined(PLATFORM_WEB)
    glfwSwapInterval(0);
#endif

#if defined(PLATFORM_DESKTOP)
    // Load OpenGL 3.3 extensions
    // NOTE: GLFW loader function is passed as parameter
    rlLoadExtensions(glfwGetProcAddress);
#endif

    // Try to enable GPU V-Sync, so frames are limited to screen refresh rate (60Hz -> 60 FPS)
    // NOTE: V-Sync can be enabled by graphic driver configuration
    if (configFlags & FLAG_VSYNC_HINT)
    {
        // WARNING: It seems to hits a critical render path in Intel HD Graphics
        glfwSwapInterval(1);
        TraceLog(LOG_INFO, "Trying to enable VSYNC");
    }
#endif // defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI) || defined(PLATFORM_UWP)
    fullscreen = true;

    // Screen size security check
    if (screenWidth <= 0) screenWidth = displayWidth;
    if (screenHeight <= 0) screenHeight = displayHeight;

#if defined(PLATFORM_RPI)
    bcm_host_init();

    DISPMANX_ELEMENT_HANDLE_T dispmanElement;
    DISPMANX_DISPLAY_HANDLE_T dispmanDisplay;
    DISPMANX_UPDATE_HANDLE_T dispmanUpdate;

    VC_RECT_T dstRect;
    VC_RECT_T srcRect;
#endif

    EGLint samples = 0;
    EGLint sampleBuffer = 0;
    if (configFlags & FLAG_MSAA_4X_HINT)
    {
        samples = 4;
        sampleBuffer = 1;
        TraceLog(LOG_INFO, "Trying to enable MSAA x4");
    }

    const EGLint framebufferAttribs[] =
    {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,     // Type of context support -> Required on RPI?
        //EGL_SURFACE_TYPE, EGL_WINDOW_BIT,          // Don't use it on Android!
        EGL_RED_SIZE, 8,            // RED color bit depth (alternative: 5)
        EGL_GREEN_SIZE, 8,          // GREEN color bit depth (alternative: 6)
        EGL_BLUE_SIZE, 8,           // BLUE color bit depth (alternative: 5)
        //EGL_ALPHA_SIZE, 8,        // ALPHA bit depth (required for transparent framebuffer)
        //EGL_TRANSPARENT_TYPE, EGL_NONE, // Request transparent framebuffer (EGL_TRANSPARENT_RGB does not work on RPI)
        EGL_DEPTH_SIZE, 16,         // Depth buffer size (Required to use Depth testing!)
        //EGL_STENCIL_SIZE, 8,      // Stencil buffer size
        EGL_SAMPLE_BUFFERS, sampleBuffer,    // Activate MSAA
        EGL_SAMPLES, samples,       // 4x Antialiasing if activated (Free on MALI GPUs)
        EGL_NONE
    };

    const EGLint contextAttribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

#if defined(PLATFORM_UWP)
    const EGLint surfaceAttributes[] =
    {
        // EGL_ANGLE_SURFACE_RENDER_TO_BACK_BUFFER is part of the same optimization as EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER (see above).
        // If you have compilation issues with it then please update your Visual Studio templates.
        EGL_ANGLE_SURFACE_RENDER_TO_BACK_BUFFER, EGL_TRUE,
        EGL_NONE
    };

    const EGLint defaultDisplayAttributes[] =
    {
        // These are the default display attributes, used to request ANGLE's D3D11 renderer.
        // eglInitialize will only succeed with these attributes if the hardware supports D3D11 Feature Level 10_0+.
        EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,

        // EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER is an optimization that can have large performance benefits on mobile devices.
        // Its syntax is subject to change, though. Please update your Visual Studio templates if you experience compilation issues with it.
        EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,

        // EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE is an option that enables ANGLE to automatically call
        // the IDXGIDevice3::Trim method on behalf of the application when it gets suspended.
        // Calling IDXGIDevice3::Trim when an application is suspended is a Windows Store application certification requirement.
        EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
        EGL_NONE,
    };

    const EGLint fl9_3DisplayAttributes[] =
    {
        // These can be used to request ANGLE's D3D11 renderer, with D3D11 Feature Level 9_3.
        // These attributes are used if the call to eglInitialize fails with the default display attributes.
        EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, 9,
        EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, 3,
        EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
        EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
        EGL_NONE,
    };

    const EGLint warpDisplayAttributes[] =
    {
        // These attributes can be used to request D3D11 WARP.
        // They are used if eglInitialize fails with both the default display attributes and the 9_3 display attributes.
        EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE,
        EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
        EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
        EGL_NONE,
    };

    EGLConfig config = NULL;

    // eglGetPlatformDisplayEXT is an alternative to eglGetDisplay. It allows us to pass in display attributes, used to configure D3D11.
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)(eglGetProcAddress("eglGetPlatformDisplayEXT"));
    if (!eglGetPlatformDisplayEXT)
    {
        TraceLog(LOG_WARNING, "Failed to get function eglGetPlatformDisplayEXT");
        return false;
    }

    //
    // To initialize the display, we make three sets of calls to eglGetPlatformDisplayEXT and eglInitialize, with varying
    // parameters passed to eglGetPlatformDisplayEXT:
    // 1) The first calls uses "defaultDisplayAttributes" as a parameter. This corresponds to D3D11 Feature Level 10_0+.
    // 2) If eglInitialize fails for step 1 (e.g. because 10_0+ isn't supported by the default GPU), then we try again
    //    using "fl9_3DisplayAttributes". This corresponds to D3D11 Feature Level 9_3.
    // 3) If eglInitialize fails for step 2 (e.g. because 9_3+ isn't supported by the default GPU), then we try again
    //    using "warpDisplayAttributes".  This corresponds to D3D11 Feature Level 11_0 on WARP, a D3D11 software rasterizer.
    //

    // This tries to initialize EGL to D3D11 Feature Level 10_0+. See above comment for details.
    display = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, defaultDisplayAttributes);
    if (display == EGL_NO_DISPLAY)
    {
        TraceLog(LOG_WARNING, "Failed to initialize EGL display");
        return false;
    }

    if (eglInitialize(display, NULL, NULL) == EGL_FALSE)
    {
        // This tries to initialize EGL to D3D11 Feature Level 9_3, if 10_0+ is unavailable (e.g. on some mobile devices).
        display = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, fl9_3DisplayAttributes);
        if (display == EGL_NO_DISPLAY)
        {
            TraceLog(LOG_WARNING, "Failed to initialize EGL display");
            return false;
        }

        if (eglInitialize(display, NULL, NULL) == EGL_FALSE)
        {
            // This initializes EGL to D3D11 Feature Level 11_0 on WARP, if 9_3+ is unavailable on the default GPU.
            display = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, warpDisplayAttributes);
            if (display == EGL_NO_DISPLAY)
            {
                TraceLog(LOG_WARNING, "Failed to initialize EGL display");
                return false;
            }

            if (eglInitialize(display, NULL, NULL) == EGL_FALSE)
            {
                // If all of the calls to eglInitialize returned EGL_FALSE then an error has occurred.
                TraceLog(LOG_WARNING, "Failed to initialize EGL");
                return false;
            }
        }
    }

    //SetupFramebuffer(displayWidth, displayHeight);

    EGLint numConfigs = 0;
    if ((eglChooseConfig(display, framebufferAttribs, &config, 1, &numConfigs) == EGL_FALSE) || (numConfigs == 0))
    {
        TraceLog(LOG_WARNING, "Failed to choose first EGLConfig");
        return false;
    }

    // Create a PropertySet and initialize with the EGLNativeWindowType.
    //PropertySet^ surfaceCreationProperties = ref new PropertySet();
    //surfaceCreationProperties->Insert(ref new String(EGLNativeWindowTypeProperty), window);     // CoreWindow^ window

    // You can configure the surface to render at a lower resolution and be scaled up to
    // the full window size. The scaling is often free on mobile hardware.
    //
    // One way to configure the SwapChainPanel is to specify precisely which resolution it should render at.
    // Size customRenderSurfaceSize = Size(800, 600);
    // surfaceCreationProperties->Insert(ref new String(EGLRenderSurfaceSizeProperty), PropertyValue::CreateSize(customRenderSurfaceSize));
    //
    // Another way is to tell the SwapChainPanel to render at a certain scale factor compared to its size.
    // e.g. if the SwapChainPanel is 1920x1280 then setting a factor of 0.5f will make the app render at 960x640
    // float customResolutionScale = 0.5f;
    // surfaceCreationProperties->Insert(ref new String(EGLRenderResolutionScaleProperty), PropertyValue::CreateSingle(customResolutionScale));


    // eglCreateWindowSurface() requires a EGLNativeWindowType parameter,
    // In Windows platform: typedef HWND EGLNativeWindowType;


    // Property: EGLNativeWindowTypeProperty
    // Type: IInspectable
    // Description: Set this property to specify the window type to use for creating a surface.
    //              If this property is missing, surface creation will fail.
    //
    //const wchar_t EGLNativeWindowTypeProperty[] = L"EGLNativeWindowTypeProperty";

    //https://stackoverflow.com/questions/46550182/how-to-create-eglsurface-using-c-winrt-and-angle

    //surface = eglCreateWindowSurface(display, config, reinterpret_cast<IInspectable*>(surfaceCreationProperties), surfaceAttributes);
    surface = eglCreateWindowSurface(display, config, uwpWindow, surfaceAttributes);
    if (surface == EGL_NO_SURFACE)
    {
        TraceLog(LOG_WARNING, "Failed to create EGL fullscreen surface");
        return false;
    }

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT)
    {
        TraceLog(LOG_WARNING, "Failed to create EGL context");
        return false;
    }

    // Get EGL display window size
    eglQuerySurface(display, surface, EGL_WIDTH, &screenWidth);
    eglQuerySurface(display, surface, EGL_HEIGHT, &screenHeight);

#else   // PLATFORM_ANDROID, PLATFORM_RPI
    EGLint numConfigs;

    // Get an EGL display connection
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY)
    {
        TraceLog(LOG_WARNING, "Failed to initialize EGL display");
        return false;
    }

    // Initialize the EGL display connection
    if (eglInitialize(display, NULL, NULL) == EGL_FALSE)
    {
        // If all of the calls to eglInitialize returned EGL_FALSE then an error has occurred.
        TraceLog(LOG_WARNING, "Failed to initialize EGL");
        return false;
    }

    // Get an appropriate EGL framebuffer configuration
    eglChooseConfig(display, framebufferAttribs, &config, 1, &numConfigs);

    // Set rendering API
    eglBindAPI(EGL_OPENGL_ES_API);

    // Create an EGL rendering context
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT)
    {
        TraceLog(LOG_WARNING, "Failed to create EGL context");
        return false;
    }
#endif

    // Create an EGL window surface
    //---------------------------------------------------------------------------------
#if defined(PLATFORM_ANDROID)
    EGLint displayFormat;

    displayWidth = ANativeWindow_getWidth(androidApp->window);
    displayHeight = ANativeWindow_getHeight(androidApp->window);

    // EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is guaranteed to be accepted by ANativeWindow_setBuffersGeometry()
    // As soon as we picked a EGLConfig, we can safely reconfigure the ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &displayFormat);

    // At this point we need to manage render size vs screen size
    // NOTE: This function use and modify global module variables: screenWidth/screenHeight and renderWidth/renderHeight and downscaleView
    SetupFramebuffer(displayWidth, displayHeight);

    ANativeWindow_setBuffersGeometry(androidApp->window, renderWidth, renderHeight, displayFormat);
    //ANativeWindow_setBuffersGeometry(androidApp->window, 0, 0, displayFormat);       // Force use of native display size

    surface = eglCreateWindowSurface(display, config, androidApp->window, NULL);
#endif  // defined(PLATFORM_ANDROID)

#if defined(PLATFORM_RPI)
    graphics_get_display_size(0, &displayWidth, &displayHeight);

    // At this point we need to manage render size vs screen size
    // NOTE: This function use and modify global module variables: screenWidth/screenHeight and renderWidth/renderHeight and downscaleView
    SetupFramebuffer(displayWidth, displayHeight);

    dstRect.x = 0;
    dstRect.y = 0;
    dstRect.width = displayWidth;
    dstRect.height = displayHeight;

    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.width = renderWidth << 16;
    srcRect.height = renderHeight << 16;

    // NOTE: RPI dispmanx windowing system takes care of srcRec scaling to dstRec by hardware (no cost)
    // Take care that renderWidth/renderHeight fit on displayWidth/displayHeight aspect ratio

    VC_DISPMANX_ALPHA_T alpha;
    alpha.flags = DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS;
    alpha.opacity = 255;    // Set transparency level for framebuffer, requires EGLAttrib: EGL_TRANSPARENT_TYPE
    alpha.mask = 0;

    dispmanDisplay = vc_dispmanx_display_open(0);   // LCD
    dispmanUpdate = vc_dispmanx_update_start(0);

    dispmanElement = vc_dispmanx_element_add(dispmanUpdate, dispmanDisplay, 0/*layer*/, &dstRect, 0/*src*/,
                                            &srcRect, DISPMANX_PROTECTION_NONE, &alpha, 0/*clamp*/, DISPMANX_NO_ROTATE);

    nativeWindow.element = dispmanElement;
    nativeWindow.width = renderWidth;
    nativeWindow.height = renderHeight;
    vc_dispmanx_update_submit_sync(dispmanUpdate);

    surface = eglCreateWindowSurface(display, config, &nativeWindow, NULL);
    //---------------------------------------------------------------------------------
#endif  // defined(PLATFORM_RPI)
    // There must be at least one frame displayed before the buffers are swapped
    //eglSwapInterval(display, 1);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
    {
        TraceLog(LOG_WARNING, "Unable to attach EGL rendering context to EGL surface");
        return false;
    }
    else
    {
        // Grab the width and height of the surface
        //eglQuerySurface(display, surface, EGL_WIDTH, &renderWidth);
        //eglQuerySurface(display, surface, EGL_HEIGHT, &renderHeight);

        TraceLog(LOG_INFO, "Display device initialized successfully");
        TraceLog(LOG_INFO, "Display size: %i x %i", displayWidth, displayHeight);
        TraceLog(LOG_INFO, "Render size: %i x %i", renderWidth, renderHeight);
        TraceLog(LOG_INFO, "Screen size: %i x %i", screenWidth, screenHeight);
        TraceLog(LOG_INFO, "Viewport offsets: %i, %i", renderOffsetX, renderOffsetY);
    }
#endif // defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)

    renderWidth = screenWidth;
    renderHeight = screenHeight;

    // Initialize OpenGL context (states and resources)
    // NOTE: screenWidth and screenHeight not used, just stored as globals
    rlglInit(screenWidth, screenHeight);

    // Setup default viewport
    SetupViewport();

    // Initialize internal projection and modelview matrices
    // NOTE: Default to orthographic projection mode with top-left corner at (0,0)
    rlMatrixMode(RL_PROJECTION);                // Switch to PROJECTION matrix
    rlLoadIdentity();                           // Reset current matrix (PROJECTION)
    rlOrtho(0, renderWidth - renderOffsetX, renderHeight - renderOffsetY, 0, 0.0f, 1.0f);
    rlMatrixMode(RL_MODELVIEW);                 // Switch back to MODELVIEW matrix
    rlLoadIdentity();                           // Reset current matrix (MODELVIEW)

    ClearBackground(RAYWHITE);      // Default background color for raylib games :P

#if defined(PLATFORM_ANDROID)
    windowReady = true;             // IMPORTANT!
#endif
    return true;
}

// Set viewport parameters
static void SetupViewport(void)
{
#if defined(__APPLE__)
    // Get framebuffer size of current window
    // NOTE: Required to handle HighDPI display correctly on OSX because framebuffer
    // is automatically reasized to adapt to new DPI.
    // When OS does that, it can be detected using GLFW3 callback: glfwSetFramebufferSizeCallback()
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    rlViewport(renderOffsetX/2, renderOffsetY/2,  fbWidth - renderOffsetX, fbHeight - renderOffsetY);
#else
    // Initialize screen viewport (area of the screen that you will actually draw to)
    // NOTE: Viewport must be recalculated if screen is resized
    rlViewport(renderOffsetX/2, renderOffsetY/2, renderWidth - renderOffsetX, renderHeight - renderOffsetY);
#endif
}

// Compute framebuffer size relative to screen size and display size
// NOTE: Global variables renderWidth/renderHeight and renderOffsetX/renderOffsetY can be modified
static void SetupFramebuffer(int width, int height)
{
    // Calculate renderWidth and renderHeight, we have the display size (input params) and the desired screen size (global var)
    if ((screenWidth > displayWidth) || (screenHeight > displayHeight))
    {
        TraceLog(LOG_WARNING, "DOWNSCALING: Required screen size (%ix%i) is bigger than display size (%ix%i)", screenWidth, screenHeight, displayWidth, displayHeight);

        // Downscaling to fit display with border-bars
        float widthRatio = (float)displayWidth/(float)screenWidth;
        float heightRatio = (float)displayHeight/(float)screenHeight;

        if (widthRatio <= heightRatio)
        {
            renderWidth = displayWidth;
            renderHeight = (int)round((float)screenHeight*widthRatio);
            renderOffsetX = 0;
            renderOffsetY = (displayHeight - renderHeight);
        }
        else
        {
            renderWidth = (int)round((float)screenWidth*heightRatio);
            renderHeight = displayHeight;
            renderOffsetX = (displayWidth - renderWidth);
            renderOffsetY = 0;
        }

        // NOTE: downscale matrix required!
        float scaleRatio = (float)renderWidth/(float)screenWidth;

        downscaleView = MatrixScale(scaleRatio, scaleRatio, scaleRatio);

        // NOTE: We render to full display resolution!
        // We just need to calculate above parameters for downscale matrix and offsets
        renderWidth = displayWidth;
        renderHeight = displayHeight;

        TraceLog(LOG_WARNING, "Downscale matrix generated, content will be rendered at: %i x %i", renderWidth, renderHeight);
    }
    else if ((screenWidth < displayWidth) || (screenHeight < displayHeight))
    {
        // Required screen size is smaller than display size
        TraceLog(LOG_INFO, "UPSCALING: Required screen size: %i x %i -> Display size: %i x %i", screenWidth, screenHeight, displayWidth, displayHeight);

        // Upscaling to fit display with border-bars
        float displayRatio = (float)displayWidth/(float)displayHeight;
        float screenRatio = (float)screenWidth/(float)screenHeight;

        if (displayRatio <= screenRatio)
        {
            renderWidth = screenWidth;
            renderHeight = (int)round((float)screenWidth/displayRatio);
            renderOffsetX = 0;
            renderOffsetY = (renderHeight - screenHeight);
        }
        else
        {
            renderWidth = (int)round((float)screenHeight*displayRatio);
            renderHeight = screenHeight;
            renderOffsetX = (renderWidth - screenWidth);
            renderOffsetY = 0;
        }
    }
    else    // screen == display
    {
        renderWidth = screenWidth;
        renderHeight = screenHeight;
        renderOffsetX = 0;
        renderOffsetY = 0;
    }
}

// Initialize hi-resolution timer
static void InitTimer(void)
{
    srand((unsigned int)time(NULL));              // Initialize random seed

#if !defined(SUPPORT_BUSY_WAIT_LOOP) && defined(_WIN32)
    timeBeginPeriod(1);             // Setup high-resolution timer to 1ms (granularity of 1-2 ms)
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
    struct timespec now;

    if (clock_gettime(CLOCK_MONOTONIC, &now) == 0)  // Success
    {
        baseTime = (uint64_t)now.tv_sec*1000000000LLU + (uint64_t)now.tv_nsec;
    }
    else TraceLog(LOG_WARNING, "No hi-resolution timer available");
#endif

    previousTime = GetTime();       // Get time as double
}

// Wait for some milliseconds (stop program execution)
// NOTE: Sleep() granularity could be around 10 ms, it means, Sleep() could
// take longer than expected... for that reason we use the busy wait loop
// http://stackoverflow.com/questions/43057578/c-programming-win32-games-sleep-taking-longer-than-expected
static void Wait(float ms)
{
#if defined(SUPPORT_BUSY_WAIT_LOOP)
    double prevTime = GetTime();
    double nextTime = 0.0;

    // Busy wait loop
    while ((nextTime - prevTime) < ms/1000.0f) nextTime = GetTime();
#else
    #if defined(_WIN32)
        Sleep((unsigned int)ms);
    #elif defined(__linux__) || defined(PLATFORM_WEB)
        struct timespec req = { 0 };
        time_t sec = (int)(ms/1000.0f);
        ms -= (sec*1000);
        req.tv_sec = sec;
        req.tv_nsec = ms*1000000L;

        // NOTE: Use nanosleep() on Unix platforms... usleep() it's deprecated.
        while (nanosleep(&req, &req) == -1) continue;
    #elif defined(__APPLE__)
        usleep(ms*1000.0f);
    #endif
#endif
}

// Get one key state
static bool GetKeyStatus(int key)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    return glfwGetKey(window, key);
#elif defined(PLATFORM_ANDROID)
    // NOTE: Android supports up to 260 keys
    if (key < 0 || key > 260) return false;
    else return currentKeyState[key];
#elif defined(PLATFORM_RPI)
    // NOTE: Keys states are filled in PollInputEvents()
    if (key < 0 || key > 511) return false;
    else return currentKeyState[key];
#endif
}

// Get one mouse button state
static bool GetMouseButtonStatus(int button)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    return glfwGetMouseButton(window, button);
#elif defined(PLATFORM_ANDROID)
    // TODO: Check for virtual mouse?
    return false;
#elif defined(PLATFORM_RPI)
    // NOTE: Mouse buttons states are filled in PollInputEvents()
    return currentMouseState[button];
#endif
}

// Poll (store) all input events
static void PollInputEvents(void)
{
#if defined(SUPPORT_GESTURES_SYSTEM)
    // NOTE: Gestures update must be called every frame to reset gestures correctly
    // because ProcessGestureEvent() is just called on an event, not every frame
    UpdateGestures();
#endif

    // Reset last key pressed registered
    lastKeyPressed = -1;

#if !defined(PLATFORM_RPI)
    // Reset last gamepad button/axis registered state
    lastGamepadButtonPressed = -1;
    gamepadAxisCount = 0;
#endif

#if defined(PLATFORM_RPI)

    // Register previous keys states
    for (int i = 0; i < 512; i++)previousKeyState[i] = currentKeyState[i];

    // Grab a keypress from the evdev fifo if avalable
    if(lastKeyPressedEvdev.Head != lastKeyPressedEvdev.Tail)
    {
        lastKeyPressed = lastKeyPressedEvdev.Contents[lastKeyPressedEvdev.Tail];    // Read the key from the buffer
        lastKeyPressedEvdev.Tail = (lastKeyPressedEvdev.Tail + 1) & 0x07;           // Increment the tail pointer forwards and binary wraparound after 7 (fifo is 8 elements long)
    }

    // Register previous mouse states
    previousMouseWheelY = currentMouseWheelY;
    currentMouseWheelY = 0;
    for (int i = 0; i < 3; i++)
    {
        previousMouseState[i] = currentMouseState[i];
        currentMouseState[i] = currentMouseStateEvdev[i];
    }
#endif

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    // Mouse input polling
    double mouseX;
    double mouseY;

    glfwGetCursorPos(window, &mouseX, &mouseY);

    mousePosition.x = (float)mouseX;
    mousePosition.y = (float)mouseY;

    // Keyboard input polling (automatically managed by GLFW3 through callback)

    // Register previous keys states
    for (int i = 0; i < 512; i++) previousKeyState[i] = currentKeyState[i];

    // Register previous mouse states
    for (int i = 0; i < 3; i++) previousMouseState[i] = currentMouseState[i];

    previousMouseWheelY = currentMouseWheelY;
    currentMouseWheelY = 0;
#endif

#if defined(PLATFORM_DESKTOP)
    // Check if gamepads are ready
    // NOTE: We do it here in case of disconection
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        if (glfwJoystickPresent(i)) gamepadReady[i] = true;
        else gamepadReady[i] = false;
    }

    // Register gamepads buttons events
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        if (gamepadReady[i])     // Check if gamepad is available
        {
            // Register previous gamepad states
            for (int k = 0; k < MAX_GAMEPAD_BUTTONS; k++) previousGamepadState[i][k] = currentGamepadState[i][k];

            // Get current gamepad state
            // NOTE: There is no callback available, so we get it manually
            const unsigned char *buttons;
            int buttonsCount;

            buttons = glfwGetJoystickButtons(i, &buttonsCount);

            for (int k = 0; (buttons != NULL) && (k < buttonsCount) && (buttonsCount < MAX_GAMEPAD_BUTTONS); k++)
            {
                if (buttons[k] == GLFW_PRESS)
                {
                    currentGamepadState[i][k] = 1;
                    lastGamepadButtonPressed = k;
                }
                else currentGamepadState[i][k] = 0;
            }

            // Get current axis state
            const float *axes;
            int axisCount = 0;

            axes = glfwGetJoystickAxes(i, &axisCount);

            for (int k = 0; (axes != NULL) && (k < axisCount) && (k < MAX_GAMEPAD_AXIS); k++)
            {
                gamepadAxisState[i][k] = axes[k];
            }

            gamepadAxisCount = axisCount;
        }
    }

    windowResized = false;

#if defined(SUPPORT_EVENTS_WAITING)
    glfwWaitEvents();
#else
    glfwPollEvents();       // Register keyboard/mouse events (callbacks)... and window events!
#endif
#endif      //defined(PLATFORM_DESKTOP)

// Gamepad support using emscripten API
// NOTE: GLFW3 joystick functionality not available in web
#if defined(PLATFORM_WEB)
    // Get number of gamepads connected
    int numGamepads = emscripten_get_num_gamepads();

    for (int i = 0; (i < numGamepads) && (i < MAX_GAMEPADS); i++)
    {
        // Register previous gamepad button states
        for (int k = 0; k < MAX_GAMEPAD_BUTTONS; k++) previousGamepadState[i][k] = currentGamepadState[i][k];

        EmscriptenGamepadEvent gamepadState;

        int result = emscripten_get_gamepad_status(i, &gamepadState);

        if (result == EMSCRIPTEN_RESULT_SUCCESS)
        {
            // Register buttons data for every connected gamepad
            for (int j = 0; (j < gamepadState.numButtons) && (j < MAX_GAMEPAD_BUTTONS); j++)
            {
                if (gamepadState.digitalButton[j] == 1)
                {
                    currentGamepadState[i][j] = 1;
                    lastGamepadButtonPressed = j;
                }
                else currentGamepadState[i][j] = 0;

                //TraceLog(LOG_DEBUG, "Gamepad %d, button %d: Digital: %d, Analog: %g", gamepadState.index, j, gamepadState.digitalButton[j], gamepadState.analogButton[j]);
            }

            // Register axis data for every connected gamepad
            for (int j = 0; (j < gamepadState.numAxes) && (j < MAX_GAMEPAD_AXIS); j++)
            {
                gamepadAxisState[i][j] = gamepadState.axis[j];
            }

            gamepadAxisCount = gamepadState.numAxes;
        }
    }
#endif

#if defined(PLATFORM_ANDROID)
    // Register previous keys states
    // NOTE: Android supports up to 260 keys
    for (int i = 0; i < 260; i++) previousKeyState[i] = currentKeyState[i];

    // Poll Events (registered events)
    // NOTE: Activity is paused if not enabled (appEnabled)
    while ((ident = ALooper_pollAll(appEnabled? 0 : -1, NULL, &events,(void**)&source)) >= 0)
    {
        // Process this event
        if (source != NULL) source->process(androidApp, source);

        // NOTE: Never close window, native activity is controlled by the system!
        if (androidApp->destroyRequested != 0)
        {
            //TraceLog(LOG_INFO, "Closing Window...");
            //windowShouldClose = true;
            //ANativeActivity_finish(androidApp->activity);
        }
    }
#endif

#if defined(PLATFORM_RPI)
    // NOTE: Mouse input events polling is done asynchonously in another pthread - EventThread()

    // NOTE: Keyboard reading could be done using input_event(s) reading or just read from stdin,
    // we now use both methods inside here. 2nd method is still used for legacy purposes (Allows for input trough SSH console)
    ProcessKeyboard();

    // NOTE: Gamepad (Joystick) input events polling is done asynchonously in another pthread - GamepadThread()
#endif
}

// Copy back buffer to front buffers
static void SwapBuffers(void)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    glfwSwapBuffers(window);
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI) || defined(PLATFORM_UWP)
    eglSwapBuffers(display, surface);
#endif
}

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
// GLFW3 Error Callback, runs on GLFW3 error
static void ErrorCallback(int error, const char *description)
{
    TraceLog(LOG_WARNING, "[GLFW3 Error] Code: %i Decription: %s", error, description);
}

// GLFW3 Srolling Callback, runs on mouse wheel
static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    currentMouseWheelY = (int)yoffset;
}

// GLFW3 Keyboard Callback, runs on key pressed
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == exitKey && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);

        // NOTE: Before closing window, while loop must be left!
    }
    else if (key == GLFW_KEY_F12 && action == GLFW_PRESS)
    {
#if defined(SUPPORT_GIF_RECORDING)
        if (mods == GLFW_MOD_CONTROL)
        {
            if (gifRecording)
            {
                GifEnd();
                gifRecording = false;

            #if defined(PLATFORM_WEB)
                // Download file from MEMFS (emscripten memory filesystem)
                // SaveFileFromMEMFSToDisk() function is defined in raylib/templates/web_shel/shell.html
                emscripten_run_script(TextFormat("SaveFileFromMEMFSToDisk('%s','%s')", TextFormat("screenrec%03i.gif", screenshotCounter - 1), TextFormat("screenrec%03i.gif", screenshotCounter - 1)));
            #endif

                TraceLog(LOG_INFO, "End animated GIF recording");
            }
            else
            {
                gifRecording = true;
                gifFramesCounter = 0;

                char path[512] = { 0 };
            #if defined(PLATFORM_ANDROID)
                strcpy(path, internalDataPath);
                strcat(path, TextFormat("./screenrec%03i.gif", screenshotCounter));
            #else
                strcpy(path, TextFormat("./screenrec%03i.gif", screenshotCounter));
            #endif

                // NOTE: delay represents the time between frames in the gif, if we capture a gif frame every
                // 10 game frames and each frame trakes 16.6ms (60fps), delay between gif frames should be ~16.6*10.
                GifBegin(path, screenWidth, screenHeight, (int)(GetFrameTime()*10.0f), 8, false);
                screenshotCounter++;

                TraceLog(LOG_INFO, "Begin animated GIF recording: %s", TextFormat("screenrec%03i.gif", screenshotCounter));
            }
        }
        else
#endif  // SUPPORT_GIF_RECORDING
#if defined(SUPPORT_SCREEN_CAPTURE)
        {
            TakeScreenshot(TextFormat("screenshot%03i.png", screenshotCounter));
            screenshotCounter++;
        }
#endif  // SUPPORT_SCREEN_CAPTURE
    }
    else
    {
        currentKeyState[key] = action;

        // NOTE: lastKeyPressed already registered on CharCallback()
        //if (action == GLFW_PRESS) lastKeyPressed = key;
    }
}

// GLFW3 Mouse Button Callback, runs on mouse button pressed
static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    currentMouseState[button] = action;

#if defined(SUPPORT_GESTURES_SYSTEM) && defined(SUPPORT_MOUSE_GESTURES)
    // Process mouse events as touches to be able to use mouse-gestures
    GestureEvent gestureEvent;

    // Register touch actions
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) gestureEvent.touchAction = TOUCH_DOWN;
    else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) gestureEvent.touchAction = TOUCH_UP;

    // NOTE: TOUCH_MOVE event is registered in MouseCursorPosCallback()

    // Assign a pointer ID
    gestureEvent.pointerId[0] = 0;

    // Register touch points count
    gestureEvent.pointCount = 1;

    // Register touch points position, only one point registered
    gestureEvent.position[0] = GetMousePosition();

    // Normalize gestureEvent.position[0] for screenWidth and screenHeight
    gestureEvent.position[0].x /= (float)GetScreenWidth();
    gestureEvent.position[0].y /= (float)GetScreenHeight();

    // Gesture data is sent to gestures system for processing
    ProcessGestureEvent(gestureEvent);
#endif
}

// GLFW3 Cursor Position Callback, runs on mouse move
static void MouseCursorPosCallback(GLFWwindow *window, double x, double y)
{
#if defined(SUPPORT_GESTURES_SYSTEM) && defined(SUPPORT_MOUSE_GESTURES)
    // Process mouse events as touches to be able to use mouse-gestures
    GestureEvent gestureEvent;

    gestureEvent.touchAction = TOUCH_MOVE;

    // Assign a pointer ID
    gestureEvent.pointerId[0] = 0;

    // Register touch points count
    gestureEvent.pointCount = 1;

    // Register touch points position, only one point registered
    gestureEvent.position[0] = (Vector2){ (float)x, (float)y };

    touchPosition[0] = gestureEvent.position[0];

    // Normalize gestureEvent.position[0] for screenWidth and screenHeight
    gestureEvent.position[0].x /= (float)GetScreenWidth();
    gestureEvent.position[0].y /= (float)GetScreenHeight();

    // Gesture data is sent to gestures system for processing
    ProcessGestureEvent(gestureEvent);
#endif
}

// GLFW3 Char Key Callback, runs on key down (get unicode char value)
static void CharCallback(GLFWwindow *window, unsigned int key)
{
    // NOTE: Registers any key down considering OS keyboard layout but
    // do not detects action events, those should be managed by user...
    // https://github.com/glfw/glfw/issues/668#issuecomment-166794907
    // http://www.glfw.org/docs/latest/input_guide.html#input_char

    lastKeyPressed = key;
}

// GLFW3 CursorEnter Callback, when cursor enters the window
static void CursorEnterCallback(GLFWwindow *window, int enter)
{
    if (enter == true) cursorOnScreen = true;
    else cursorOnScreen = false;
}

// GLFW3 WindowSize Callback, runs when window is resized
// NOTE: Window resizing not allowed by default
static void WindowSizeCallback(GLFWwindow *window, int width, int height)
{
    // If window is resized, viewport and projection matrix needs to be re-calculated
    rlViewport(0, 0, width, height);            // Set viewport width and height
    rlMatrixMode(RL_PROJECTION);                // Switch to PROJECTION matrix
    rlLoadIdentity();                           // Reset current matrix (PROJECTION)
    rlOrtho(0, width, height, 0, 0.0f, 1.0f);   // Orthographic projection mode with top-left corner at (0,0)
    rlMatrixMode(RL_MODELVIEW);                 // Switch back to MODELVIEW matrix
    rlLoadIdentity();                           // Reset current matrix (MODELVIEW)
    rlClearScreenBuffers();                     // Clear screen buffers (color and depth)

    // Window size must be updated to be used on 3D mode to get new aspect ratio (BeginMode3D())
    // NOTE: Be careful! GLFW3 will choose the closest fullscreen resolution supported by current monitor,
    // for example, if reescaling back to 800x450 (desired), it could set 720x480 (closest fullscreen supported)
    screenWidth = width;
    screenHeight = height;
    renderWidth = width;
    renderHeight = height;
    currentWidth = width;
    currentHeight = height;

    // NOTE: Postprocessing texture is not scaled to new size

    windowResized = true;
}

// GLFW3 WindowIconify Callback, runs when window is minimized/restored
static void WindowIconifyCallback(GLFWwindow *window, int iconified)
{
    if (iconified) windowMinimized = true;  // The window was iconified
    else windowMinimized = false;           // The window was restored
}

// GLFW3 Window Drop Callback, runs when drop files into window
// NOTE: Paths are stored in dinamic memory for further retrieval
// Everytime new files are dropped, old ones are discarded
static void WindowDropCallback(GLFWwindow *window, int count, const char **paths)
{
    ClearDroppedFiles();

    dropFilesPath = (char **)malloc(sizeof(char *)*count);

    for (int i = 0; i < count; i++)
    {
        dropFilesPath[i] = (char *)malloc(sizeof(char)*MAX_FILEPATH_LENGTH);
        strcpy(dropFilesPath[i], paths[i]);
    }

    dropFilesCount = count;
}
#endif

#if defined(PLATFORM_ANDROID)
// Android: Process activity lifecycle commands
static void AndroidCommandCallback(struct android_app *app, int32_t cmd)
{
    switch (cmd)
    {
        case APP_CMD_START:
        {
            //rendering = true;
            TraceLog(LOG_INFO, "APP_CMD_START");
        } break;
        case APP_CMD_RESUME:
        {
            TraceLog(LOG_INFO, "APP_CMD_RESUME");
        } break;
        case APP_CMD_INIT_WINDOW:
        {
            TraceLog(LOG_INFO, "APP_CMD_INIT_WINDOW");

            if (app->window != NULL)
            {
                if (contextRebindRequired)
                {
                    // Reset screen scaling to full display size
                    EGLint displayFormat;
                    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &displayFormat);
                    ANativeWindow_setBuffersGeometry(app->window, renderWidth, renderHeight, displayFormat);

                    // Recreate display surface and re-attach OpenGL context
                    surface = eglCreateWindowSurface(display, config, app->window, NULL);
                    eglMakeCurrent(display, surface, surface, context);

                    contextRebindRequired = false;
                }
                else
                {
                    // Init graphics device (display device and OpenGL context)
                    InitGraphicsDevice(screenWidth, screenHeight);

                    // Init hi-res timer
                    InitTimer();

                #if defined(SUPPORT_DEFAULT_FONT)
                    // Load default font
                    // NOTE: External function (defined in module: text)
                    LoadDefaultFont();
                #endif

                    // TODO: GPU assets reload in case of lost focus (lost context)
                    // NOTE: This problem has been solved just unbinding and rebinding context from display
                    /*
                    if (assetsReloadRequired)
                    {
                        for (int i = 0; i < assetsCount; i++)
                        {
                            // TODO: Unload old asset if required

                            // Load texture again to pointed texture
                            (*textureAsset + i) = LoadTexture(assetPath[i]);
                        }
                    }
                    */

                    // raylib logo appearing animation (if enabled)
                    if (showLogo)
                    {
                        SetTargetFPS(60);   // Not required on Android
                        LogoAnimation();
                    }
                }
            }
        } break;
        case APP_CMD_GAINED_FOCUS:
        {
            TraceLog(LOG_INFO, "APP_CMD_GAINED_FOCUS");
            appEnabled = true;
            //ResumeMusicStream();
        } break;
        case APP_CMD_PAUSE:
        {
            TraceLog(LOG_INFO, "APP_CMD_PAUSE");
        } break;
        case APP_CMD_LOST_FOCUS:
        {
            //DrawFrame();
            TraceLog(LOG_INFO, "APP_CMD_LOST_FOCUS");
            appEnabled = false;
            //PauseMusicStream();
        } break;
        case APP_CMD_TERM_WINDOW:
        {
            // Dettach OpenGL context and destroy display surface
            // NOTE 1: Detaching context before destroying display surface avoids losing our resources (textures, shaders, VBOs...)
            // NOTE 2: In some cases (too many context loaded), OS could unload context automatically... :(
            eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            eglDestroySurface(display, surface);

            contextRebindRequired = true;

            TraceLog(LOG_INFO, "APP_CMD_TERM_WINDOW");
        } break;
        case APP_CMD_SAVE_STATE:
        {
            TraceLog(LOG_INFO, "APP_CMD_SAVE_STATE");
        } break;
        case APP_CMD_STOP:
        {
            TraceLog(LOG_INFO, "APP_CMD_STOP");
        } break;
        case APP_CMD_DESTROY:
        {
            // TODO: Finish activity?
            //ANativeActivity_finish(androidApp->activity);

            TraceLog(LOG_INFO, "APP_CMD_DESTROY");
        } break;
        case APP_CMD_CONFIG_CHANGED:
        {
            //AConfiguration_fromAssetManager(androidApp->config, androidApp->activity->assetManager);
            //print_cur_config(androidApp);

            // Check screen orientation here!

            TraceLog(LOG_INFO, "APP_CMD_CONFIG_CHANGED");
        } break;
        default: break;
    }
}

// Android: Get input events
static int32_t AndroidInputCallback(struct android_app *app, AInputEvent *event)
{
    // If additional inputs are required check:
    // https://developer.android.com/ndk/reference/group/input

    int type = AInputEvent_getType(event);

    if (type == AINPUT_EVENT_TYPE_MOTION)
    {
        // Get first touch position
        touchPosition[0].x = AMotionEvent_getX(event, 0);
        touchPosition[0].y = AMotionEvent_getY(event, 0);

        // Get second touch position
        touchPosition[1].x = AMotionEvent_getX(event, 1);
        touchPosition[1].y = AMotionEvent_getY(event, 1);

        // Useful functions for gamepad inputs:
        //AMotionEvent_getAction()
        //AMotionEvent_getAxisValue()
        //AMotionEvent_getButtonState()

        // Gamepad dpad button presses capturing
        // TODO: That's weird, key input (or button)
        // shouldn't come as a TYPE_MOTION event...
        int32_t keycode = AKeyEvent_getKeyCode(event);
        if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN)
        {
            currentKeyState[keycode] = 1;  // Key down
            lastKeyPressed = keycode;
        }
        else currentKeyState[keycode] = 0;  // Key up

    }
    else if (type == AINPUT_EVENT_TYPE_KEY)
    {
        int32_t keycode = AKeyEvent_getKeyCode(event);
        //int32_t AKeyEvent_getMetaState(event);

        // Save current button and its state
        // NOTE: Android key action is 0 for down and 1 for up
        if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN)
        {
            currentKeyState[keycode] = 1;   // Key down
            lastKeyPressed = keycode;
        }
        else currentKeyState[keycode] = 0;  // Key up

        if (keycode == AKEYCODE_POWER)
        {
            // Let the OS handle input to avoid app stuck. Behaviour: CMD_PAUSE -> CMD_SAVE_STATE -> CMD_STOP -> CMD_CONFIG_CHANGED -> CMD_LOST_FOCUS
            // Resuming Behaviour: CMD_START -> CMD_RESUME -> CMD_CONFIG_CHANGED -> CMD_CONFIG_CHANGED -> CMD_GAINED_FOCUS
            // It seems like locking mobile, screen size (CMD_CONFIG_CHANGED) is affected.
            // NOTE: AndroidManifest.xml must have <activity android:configChanges="orientation|keyboardHidden|screenSize" >
            // Before that change, activity was calling CMD_TERM_WINDOW and CMD_DESTROY when locking mobile, so that was not a normal behaviour
            return 0;
        }
        else if ((keycode == AKEYCODE_BACK) || (keycode == AKEYCODE_MENU))
        {
            // Eat BACK_BUTTON and AKEYCODE_MENU, just do nothing... and don't let to be handled by OS!
            return 1;
        }
        else if ((keycode == AKEYCODE_VOLUME_UP) || (keycode == AKEYCODE_VOLUME_DOWN))
        {
            // Set default OS behaviour
            return 0;
        }
    }

    int32_t action = AMotionEvent_getAction(event);
    unsigned int flags = action & AMOTION_EVENT_ACTION_MASK;

#if defined(SUPPORT_GESTURES_SYSTEM)
    GestureEvent gestureEvent;

    // Register touch actions
    if (flags == AMOTION_EVENT_ACTION_DOWN) gestureEvent.touchAction = TOUCH_DOWN;
    else if (flags == AMOTION_EVENT_ACTION_UP) gestureEvent.touchAction = TOUCH_UP;
    else if (flags == AMOTION_EVENT_ACTION_MOVE) gestureEvent.touchAction = TOUCH_MOVE;

    // Register touch points count
    // NOTE: Documentation says pointerCount is Always >= 1,
    // but in practice it can be 0 or over a million
    gestureEvent.pointCount = AMotionEvent_getPointerCount(event);

    // Only enable gestures for 1-3 touch points
    if ((gestureEvent.pointCount > 0) && (gestureEvent.pointCount < 4))
    {
        // Register touch points id
        // NOTE: Only two points registered
        gestureEvent.pointerId[0] = AMotionEvent_getPointerId(event, 0);
        gestureEvent.pointerId[1] = AMotionEvent_getPointerId(event, 1);

        // Register touch points position
        gestureEvent.position[0] = (Vector2){ AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0) };
        gestureEvent.position[1] = (Vector2){ AMotionEvent_getX(event, 1), AMotionEvent_getY(event, 1) };

        // Normalize gestureEvent.position[x] for screenWidth and screenHeight
        gestureEvent.position[0].x /= (float)GetScreenWidth();
        gestureEvent.position[0].y /= (float)GetScreenHeight();

        gestureEvent.position[1].x /= (float)GetScreenWidth();
        gestureEvent.position[1].y /= (float)GetScreenHeight();

        // Gesture data is sent to gestures system for processing
        ProcessGestureEvent(gestureEvent);
    }
#else
    // Support only simple touch position
    if (flags == AMOTION_EVENT_ACTION_DOWN)
    {
        // Get first touch position
        touchPosition[0].x = AMotionEvent_getX(event, 0);
        touchPosition[0].y = AMotionEvent_getY(event, 0);

        touchPosition[0].x /= (float)GetScreenWidth();
        touchPosition[0].y /= (float)GetScreenHeight();
    }
#endif

    return 0;
}
#endif

#if defined(PLATFORM_WEB)

// Register fullscreen change events
static EM_BOOL EmscriptenFullscreenChangeCallback(int eventType, const EmscriptenFullscreenChangeEvent *e, void *userData)
{
    //isFullscreen: int e->isFullscreen
    //fullscreenEnabled: int e->fullscreenEnabled
    //fs element nodeName: (char *) e->nodeName
    //fs element id: (char *) e->id
    //Current element size: (int) e->elementWidth, (int) e->elementHeight
    //Screen size:(int) e->screenWidth, (int) e->screenHeight

    if (e->isFullscreen)
    {
        TraceLog(LOG_INFO, "Canvas scaled to fullscreen. ElementSize: (%ix%i), ScreenSize(%ix%i)", e->elementWidth, e->elementHeight, e->screenWidth, e->screenHeight);
    }
    else
    {
        TraceLog(LOG_INFO, "Canvas scaled to windowed. ElementSize: (%ix%i), ScreenSize(%ix%i)", e->elementWidth, e->elementHeight, e->screenWidth, e->screenHeight);
    }

    // TODO: Depending on scaling factor (screen vs element), calculate factor to scale mouse/touch input

    return 0;
}

// Register keyboard input events
static EM_BOOL EmscriptenKeyboardCallback(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData)
{
    if ((eventType == EMSCRIPTEN_EVENT_KEYPRESS) && (strcmp(keyEvent->code, "Escape") == 0))
    {
        emscripten_exit_pointerlock();
    }

    return 0;
}

// Register mouse input events
static EM_BOOL EmscriptenMouseCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
    // Lock mouse pointer when click on screen
    if ((eventType == EMSCRIPTEN_EVENT_CLICK) && toggleCursorLock)
    {
        EmscriptenPointerlockChangeEvent plce;
        emscripten_get_pointerlock_status(&plce);

        if (!plce.isActive) emscripten_request_pointerlock(0, 1);
        else
        {
            emscripten_exit_pointerlock();
            emscripten_get_pointerlock_status(&plce);
            //if (plce.isActive) TraceLog(LOG_WARNING, "Pointer lock exit did not work!");
        }

        toggleCursorLock = false;
    }

    return 0;
}

// Register touch input events
static EM_BOOL EmscriptenTouchCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData)
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

    TraceLog(LOG_DEBUG, "%s, numTouches: %d %s%s%s%s", emscripten_event_type_to_string(eventType), event->numTouches,
           event->ctrlKey? " CTRL" : "", event->shiftKey? " SHIFT" : "", event->altKey? " ALT" : "", event->metaKey? " META" : "");

    for (int i = 0; i < event->numTouches; ++i)
    {
        const EmscriptenTouchPoint *t = &event->touches[i];

        TraceLog(LOG_DEBUG, "  %ld: screen: (%ld,%ld), client: (%ld,%ld), page: (%ld,%ld), isChanged: %d, onTarget: %d, canvas: (%ld, %ld)",
          t->identifier, t->screenX, t->screenY, t->clientX, t->clientY, t->pageX, t->pageY, t->isChanged, t->onTarget, t->canvasX, t->canvasY);
    }
    */

#if defined(SUPPORT_GESTURES_SYSTEM)
    GestureEvent gestureEvent;

    // Register touch actions
    if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART) gestureEvent.touchAction = TOUCH_DOWN;
    else if (eventType == EMSCRIPTEN_EVENT_TOUCHEND) gestureEvent.touchAction = TOUCH_UP;
    else if (eventType == EMSCRIPTEN_EVENT_TOUCHMOVE) gestureEvent.touchAction = TOUCH_MOVE;

    // Register touch points count
    gestureEvent.pointCount = touchEvent->numTouches;

    // Register touch points id
    gestureEvent.pointerId[0] = touchEvent->touches[0].identifier;
    gestureEvent.pointerId[1] = touchEvent->touches[1].identifier;

    // Register touch points position
    // NOTE: Only two points registered
    // TODO: Touch data should be scaled accordingly!
    //gestureEvent.position[0] = (Vector2){ touchEvent->touches[0].canvasX, touchEvent->touches[0].canvasY };
    //gestureEvent.position[1] = (Vector2){ touchEvent->touches[1].canvasX, touchEvent->touches[1].canvasY };
    gestureEvent.position[0] = (Vector2){ touchEvent->touches[0].targetX, touchEvent->touches[0].targetY };
    gestureEvent.position[1] = (Vector2){ touchEvent->touches[1].targetX, touchEvent->touches[1].targetY };

    touchPosition[0] = gestureEvent.position[0];
    touchPosition[1] = gestureEvent.position[1];

    // Normalize gestureEvent.position[x] for screenWidth and screenHeight
    gestureEvent.position[0].x /= (float)GetScreenWidth();
    gestureEvent.position[0].y /= (float)GetScreenHeight();

    gestureEvent.position[1].x /= (float)GetScreenWidth();
    gestureEvent.position[1].y /= (float)GetScreenHeight();

    // Gesture data is sent to gestures system for processing
    ProcessGestureEvent(gestureEvent);
#else
    // Support only simple touch position
    if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART)
    {
        // Get first touch position
        touchPosition[0] = (Vector2){ touchEvent->touches[0].targetX, touchEvent->touches[0].targetY };

        touchPosition[0].x /= (float)GetScreenWidth();
        touchPosition[0].y /= (float)GetScreenHeight();
    }
#endif

    return 1;
}

// Register connected/disconnected gamepads events
static EM_BOOL EmscriptenGamepadCallback(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData)
{
    /*
    TraceLog(LOG_DEBUG, "%s: timeStamp: %g, connected: %d, index: %ld, numAxes: %d, numButtons: %d, id: \"%s\", mapping: \"%s\"",
           eventType != 0? emscripten_event_type_to_string(eventType) : "Gamepad state",
           gamepadEvent->timestamp, gamepadEvent->connected, gamepadEvent->index, gamepadEvent->numAxes, gamepadEvent->numButtons, gamepadEvent->id, gamepadEvent->mapping);

    for(int i = 0; i < gamepadEvent->numAxes; ++i) TraceLog(LOG_DEBUG, "Axis %d: %g", i, gamepadEvent->axis[i]);
    for(int i = 0; i < gamepadEvent->numButtons; ++i) TraceLog(LOG_DEBUG, "Button %d: Digital: %d, Analog: %g", i, gamepadEvent->digitalButton[i], gamepadEvent->analogButton[i]);
    */

    if ((gamepadEvent->connected) && (gamepadEvent->index < MAX_GAMEPADS)) gamepadReady[gamepadEvent->index] = true;
    else gamepadReady[gamepadEvent->index] = false;

    // TODO: Test gamepadEvent->index

    return 0;
}
#endif

#if defined(PLATFORM_RPI)
// Initialize Keyboard system (using standard input)
static void InitKeyboard(void)
{
    // NOTE: We read directly from Standard Input (stdin) - STDIN_FILENO file descriptor

    // Make stdin non-blocking (not enough, need to configure to non-canonical mode)
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);          // F_GETFL: Get the file access mode and the file status flags
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);     // F_SETFL: Set the file status flags to the value specified

    // Save terminal keyboard settings and reconfigure terminal with new settings
    struct termios keyboardNewSettings;
    tcgetattr(STDIN_FILENO, &defaultKeyboardSettings);    // Get current keyboard settings
    keyboardNewSettings = defaultKeyboardSettings;

    // New terminal settings for keyboard: turn off buffering (non-canonical mode), echo and key processing
    // NOTE: ISIG controls if ^C and ^Z generate break signals or not
    keyboardNewSettings.c_lflag &= ~(ICANON | ECHO | ISIG);
    //keyboardNewSettings.c_iflag &= ~(ISTRIP | INLCR | ICRNL | IGNCR | IXON | IXOFF);
    keyboardNewSettings.c_cc[VMIN] = 1;
    keyboardNewSettings.c_cc[VTIME] = 0;

    // Set new keyboard settings (change occurs immediately)
    tcsetattr(STDIN_FILENO, TCSANOW, &keyboardNewSettings);

    // NOTE: Reading directly from stdin will give chars already key-mapped by kernel to ASCII or UNICODE

    // Save old keyboard mode to restore it at the end
    if (ioctl(STDIN_FILENO, KDGKBMODE, &defaultKeyboardMode) < 0)
    {
        // NOTE: It could mean we are using a remote keyboard through ssh!
        TraceLog(LOG_WARNING, "Could not change keyboard mode (SSH keyboard?)");
    }
    else
    {
        // We reconfigure keyboard mode to get:
        //    - scancodes (K_RAW)
        //    - keycodes (K_MEDIUMRAW)
        //    - ASCII chars (K_XLATE)
        //    - UNICODE chars (K_UNICODE)
        ioctl(STDIN_FILENO, KDSKBMODE, K_XLATE);
    }

    // Register keyboard restore when program finishes
    atexit(RestoreKeyboard);
}

// Process keyboard inputs
// TODO: Most probably input reading and processing should be in a separate thread
static void ProcessKeyboard(void)
{
    #define MAX_KEYBUFFER_SIZE      32      // Max size in bytes to read

    // Keyboard input polling (fill keys[256] array with status)
    int bufferByteCount = 0;                // Bytes available on the buffer
    char keysBuffer[MAX_KEYBUFFER_SIZE];    // Max keys to be read at a time

    // Read availables keycodes from stdin
    bufferByteCount = read(STDIN_FILENO, keysBuffer, MAX_KEYBUFFER_SIZE);     // POSIX system call

    // Reset pressed keys array (it will be filled below)
    for (int i = 0; i < 512; i++) currentKeyState[i] = 0;

    // Check keys from event input workers (This is the new keyboard reading method)
    for (int i = 0; i < 512; i++) currentKeyState[i] = currentKeyStateEvdev[i];

    // Fill all read bytes (looking for keys)
    for (int i = 0; i < bufferByteCount; i++)
    {
        TraceLog(LOG_DEBUG, "Bytes on keysBuffer: %i", bufferByteCount);

        // NOTE: If (key == 0x1b), depending on next key, it could be a special keymap code!
        // Up -> 1b 5b 41 / Left -> 1b 5b 44 / Right -> 1b 5b 43 / Down -> 1b 5b 42
        if (keysBuffer[i] == 0x1b)
        {
            // Detect ESC to stop program
            if (bufferByteCount == 1) currentKeyState[256] = 1; // raylib key: KEY_ESCAPE
            else
            {
                if (keysBuffer[i + 1] == 0x5b)    // Special function key
                {
                    if ((keysBuffer[i + 2] == 0x5b) || (keysBuffer[i + 2] == 0x31) || (keysBuffer[i + 2] == 0x32))
                    {
                        // Process special function keys (F1 - F12)
                        switch (keysBuffer[i + 3])
                        {
                            case 0x41: currentKeyState[290] = 1; break;    // raylib KEY_F1
                            case 0x42: currentKeyState[291] = 1; break;    // raylib KEY_F2
                            case 0x43: currentKeyState[292] = 1; break;    // raylib KEY_F3
                            case 0x44: currentKeyState[293] = 1; break;    // raylib KEY_F4
                            case 0x45: currentKeyState[294] = 1; break;    // raylib KEY_F5
                            case 0x37: currentKeyState[295] = 1; break;    // raylib KEY_F6
                            case 0x38: currentKeyState[296] = 1; break;    // raylib KEY_F7
                            case 0x39: currentKeyState[297] = 1; break;    // raylib KEY_F8
                            case 0x30: currentKeyState[298] = 1; break;    // raylib KEY_F9
                            case 0x31: currentKeyState[299] = 1; break;    // raylib KEY_F10
                            case 0x33: currentKeyState[300] = 1; break;    // raylib KEY_F11
                            case 0x34: currentKeyState[301] = 1; break;    // raylib KEY_F12
                            default: break;
                        }

                        if (keysBuffer[i + 2] == 0x5b) i += 4;
                        else if ((keysBuffer[i + 2] == 0x31) || (keysBuffer[i + 2] == 0x32)) i += 5;
                    }
                    else
                    {
                        switch (keysBuffer[i + 2])
                        {
                            case 0x41: currentKeyState[265] = 1; break;    // raylib KEY_UP
                            case 0x42: currentKeyState[264] = 1; break;    // raylib KEY_DOWN
                            case 0x43: currentKeyState[262] = 1; break;    // raylib KEY_RIGHT
                            case 0x44: currentKeyState[263] = 1; break;    // raylib KEY_LEFT
                            default: break;
                        }

                        i += 3;  // Jump to next key
                    }

                    // NOTE: Some keys are not directly keymapped (CTRL, ALT, SHIFT)
                }
            }
        }
        else if (keysBuffer[i] == 0x0a) { lastKeyPressed = 257; currentKeyState[257] = 1; }    // raylib KEY_ENTER (don't mix with <linux/input.h> KEY_*)
        else if (keysBuffer[i] == 0x7f) { lastKeyPressed = 259; currentKeyState[259] = 1; }    // raylib KEY_BACKSPACE
        else
        {
            TraceLog(LOG_DEBUG, "Pressed key (ASCII): 0x%02x", keysBuffer[i]);

            // Translate lowercase a-z letters to A-Z
            if ((keysBuffer[i] >= 97) && (keysBuffer[i] <= 122))
            {
                currentKeyState[(int)keysBuffer[i] - 32] = 1;
            }
            else currentKeyState[(int)keysBuffer[i]] = 1;

            lastKeyPressed = keysBuffer[i];     // Register last key pressed
        }
    }

    // Check exit key (same functionality as GLFW3 KeyCallback())
    if (currentKeyState[exitKey] == 1) windowShouldClose = true;

#if defined(SUPPORT_SCREEN_CAPTURE)
    // Check screen capture key (raylib key: KEY_F12)
    if (currentKeyState[301] == 1)
    {
        TakeScreenshot(FormatText("screenshot%03i.png", screenshotCounter));
        screenshotCounter++;
    }
#endif
}

// Restore default keyboard input
static void RestoreKeyboard(void)
{
    // Reset to default keyboard settings
    tcsetattr(STDIN_FILENO, TCSANOW, &defaultKeyboardSettings);

    // Reconfigure keyboard to default mode
    ioctl(STDIN_FILENO, KDSKBMODE, defaultKeyboardMode);
}

// Initialise user input from evdev(/dev/input/event<N>) this means mouse, keyboard or gamepad devices
static void InitEvdevInput(void)
{
    char path[MAX_FILEPATH_LENGTH];
    DIR *directory;
    struct dirent *entity;

    // Reset variables
    for (int i = 0; i < MAX_TOUCH_POINTS; ++i)
    {
        touchPosition[i].x = -1;
        touchPosition[i].y = -1;
    }
    // Reset keypress buffer
    lastKeyPressedEvdev.Head = 0;
    lastKeyPressedEvdev.Tail = 0;
    // Reset keyboard key state
    for (int i = 0; i < 512; i++) currentKeyStateEvdev[i] = 0;

    // Open the linux directory of "/dev/input"
    directory = opendir(DEFAULT_EVDEV_PATH);
    if (directory)
    {
        while ((entity = readdir(directory)) != NULL)
        {
            if (strncmp("event", entity->d_name, strlen("event")) == 0)         // Search for devices named "event*"
            {
                sprintf(path, "%s%s", DEFAULT_EVDEV_PATH, entity->d_name);
                EventThreadSpawn(path);                                         // Identify the device and spawn a thread for it
            }
        }

        closedir(directory);
    }
    else TraceLog(LOG_WARNING, "Unable to open linux event directory: %s", DEFAULT_EVDEV_PATH);
}

// Identifies a input device and spawns a thread to handle it if needed
static void EventThreadSpawn(char *device)
{
    #define BITS_PER_LONG   (sizeof(long)*8)
    #define NBITS(x)        ((((x) - 1)/BITS_PER_LONG) + 1)
    #define OFF(x)          ((x)%BITS_PER_LONG)
    #define BIT(x)          (1UL<<OFF(x))
    #define LONG(x)         ((x)/BITS_PER_LONG)
    #define TEST_BIT(array, bit) ((array[LONG(bit)] >> OFF(bit)) & 1)

    struct input_absinfo absinfo;
    unsigned long evBits[NBITS(EV_MAX)];
    unsigned long absBits[NBITS(ABS_MAX)];
    unsigned long relBits[NBITS(REL_MAX)];
    unsigned long keyBits[NBITS(KEY_MAX)];
    bool hasAbs = false;
    bool hasRel = false;
    bool hasAbsMulti = false;
    int freeWorkerId = -1;
    int fd = -1;

    InputEventWorker *worker;

    // Open the device and allocate worker
    //-------------------------------------------------------------------------------------------------------
    // Find a free spot in the workers array
    for (int i = 0; i < sizeof(eventWorkers)/sizeof(InputEventWorker); ++i)
    {
        if (eventWorkers[i].threadId == 0)
        {
            freeWorkerId = i;
            break;
        }
    }

    // Select the free worker from array
    if (freeWorkerId >= 0)
    {
        worker = &(eventWorkers[freeWorkerId]);       // Grab a pointer to the worker
        memset(worker, 0, sizeof(InputEventWorker));  // Clear the worker
    }
    else
    {
        TraceLog(LOG_WARNING, "Error creating input device thread for [%s]: Out of worker slots", device);
        return;
    }

    // Open the device
    fd = open(device, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        TraceLog(LOG_WARNING, "Error creating input device thread for [%s]: Can't open device (Error: %d)", device, worker->fd);
        return;
    }
    worker->fd = fd;

    // Grab number on the end of the devices name "event<N>"
    int devNum = 0;
    char *ptrDevName = strrchr(device, 't');
    worker->eventNum = -1;

    if (ptrDevName != NULL)
    {
        if (sscanf(ptrDevName, "t%d", &devNum) == 1)
            worker->eventNum = devNum;
    }

    // At this point we have a connection to the device, but we don't yet know what the device is.
    // It could be many things, even as simple as a power button...
    //-------------------------------------------------------------------------------------------------------

    // Identify the device
    //-------------------------------------------------------------------------------------------------------
    ioctl(fd, EVIOCGBIT(0, sizeof(evBits)), evBits);    // Read a bitfield of the avalable device properties

    // Check for absolute input devices
    if (TEST_BIT(evBits, EV_ABS))
    {
        ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absBits)), absBits);

        // Check for absolute movement support (usualy touchscreens, but also joysticks)
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

        // Check for multiple absolute movement support (usualy multitouch touchscreens)
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

    // Check for relative movement support (usualy mouse)
    if (TEST_BIT(evBits, EV_REL))
    {
        ioctl(fd, EVIOCGBIT(EV_REL, sizeof(relBits)), relBits);

        if (TEST_BIT(relBits, REL_X) && TEST_BIT(relBits, REL_Y)) hasRel = true;
    }

    // Check for button support to determine the device type(usualy on all input devices)
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
    if (worker->isTouch || worker->isMouse || worker->isKeyboard)
    {
        // Looks like a interesting device
        TraceLog(LOG_INFO, "Opening input device [%s] (%s%s%s%s%s)", device,
            worker->isMouse? "mouse " : "",
            worker->isMultitouch? "multitouch " : "",
            worker->isTouch? "touchscreen " : "",
            worker->isGamepad? "gamepad " : "",
            worker->isKeyboard? "keyboard " : "");

        // Create a thread for this device
        int error = pthread_create(&worker->threadId, NULL, &EventThread, (void *)worker);
        if (error != 0)
        {
            TraceLog(LOG_WARNING, "Error creating input device thread for [%s]: Can't create thread (Error: %d)", device, error);
            worker->threadId = 0;
            close(fd);
        }

#if defined(USE_LAST_TOUCH_DEVICE)
        // Find touchscreen with the highest index
        int maxTouchNumber = -1;

        for (int i = 0; i < sizeof(eventWorkers)/sizeof(InputEventWorker); ++i)
        {
            if (eventWorkers[i].isTouch && (eventWorkers[i].eventNum > maxTouchNumber)) maxTouchNumber = eventWorkers[i].eventNum;
        }

        // Find toucnscreens with lower indexes
        for (int i = 0; i < sizeof(eventWorkers)/sizeof(InputEventWorker); ++i)
        {
            if (eventWorkers[i].isTouch && (eventWorkers[i].eventNum < maxTouchNumber))
            {
                if (eventWorkers[i].threadId != 0)
                {
                    TraceLog(LOG_WARNING, "Duplicate touchscreen found, killing touchscreen on event: %d", i);
                    pthread_cancel(eventWorkers[i].threadId);
                    close(eventWorkers[i].fd);
                }
            }
        }
#endif
    }
    else close(fd);  // We are not interested in this device
    //-------------------------------------------------------------------------------------------------------
}

// Input device events reading thread
static void *EventThread(void *arg)
{
    // Scancode to keycode mapping for US keyboards
    // TODO: Proabobly replace this with a keymap from the X11 to get the correct regional map for the keyboard (Currently non US keyboards will have the wrong mapping for some keys)
    static const int keymap_US[] =
        {0,256,49,50,51,52,53,54,55,56,57,48,45,61,259,258,81,87,69,82,84,
        89,85,73,79,80,91,93,257,341,65,83,68,70,71,72,74,75,76,59,39,96,
        340,92,90,88,67,86,66,78,77,44,46,47,344,332,342,32,280,290,291,
        292,293,294,295,296,297,298,299,282,281,327,328,329,333,324,325,
        326,334,321,322,323,320,330,0,85,86,300,301,89,90,91,92,93,94,95,
        335,345,331,283,346,101,268,265,266,263,262,269,264,267,260,261,
        112,113,114,115,116,117,118,119,120,121,122,123,124,125,347,127,
        128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
        144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
        160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
        176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
        192,193,194,0,0,0,0,0,200,201,202,203,204,205,206,207,208,209,210,
        211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,
        227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,
        243,244,245,246,247,248,0,0,0,0,0,0,0,};

    struct input_event event;
    InputEventWorker *worker = (InputEventWorker *)arg;

    int touchAction = -1;
    bool gestureUpdate = false;
    int keycode;

    while (!windowShouldClose)
    {
        // Try to read data from the device and only continue if successful
        if (read(worker->fd, &event, sizeof(event)) == (int)sizeof(event))
        {
            // Relative movement parsing
            if (event.type == EV_REL)
            {
                if (event.code == REL_X)
                {
                    mousePosition.x += event.value;
                    touchPosition[0].x = mousePosition.x;

                    #if defined(SUPPORT_GESTURES_SYSTEM)
                        touchAction = TOUCH_MOVE;
                        gestureUpdate = true;
                    #endif
                }

                if (event.code == REL_Y)
                {
                    mousePosition.y += event.value;
                    touchPosition[0].y = mousePosition.y;

                    #if defined(SUPPORT_GESTURES_SYSTEM)
                        touchAction = TOUCH_MOVE;
                        gestureUpdate = true;
                    #endif
                }

                if (event.code == REL_WHEEL)
                {
                    currentMouseWheelY += event.value;
                }
            }

            // Absolute movement parsing
            if (event.type == EV_ABS)
            {
                // Basic movement
                if (event.code == ABS_X)
                {
                    mousePosition.x = (event.value - worker->absRange.x)*screenWidth/worker->absRange.width;   // Scale acording to absRange

                    #if defined(SUPPORT_GESTURES_SYSTEM)
                        touchAction = TOUCH_MOVE;
                        gestureUpdate = true;
                    #endif
                }

                if (event.code == ABS_Y)
                {
                    mousePosition.y = (event.value - worker->absRange.y)*screenHeight/worker->absRange.height; // Scale acording to absRange

                    #if defined(SUPPORT_GESTURES_SYSTEM)
                        touchAction = TOUCH_MOVE;
                        gestureUpdate = true;
                    #endif
                }

                // Multitouch movement
                if (event.code == ABS_MT_SLOT)
                {
                    worker->touchSlot = event.value;   // Remeber the slot number for the folowing events
                }

                if (event.code == ABS_MT_POSITION_X)
                {
                    if (worker->touchSlot < MAX_TOUCH_POINTS)
                        touchPosition[worker->touchSlot].x = (event.value - worker->absRange.x)*screenWidth/worker->absRange.width;    // Scale acording to absRange
                }

                if (event.code == ABS_MT_POSITION_Y)
                {
                    if (worker->touchSlot < MAX_TOUCH_POINTS)
                        touchPosition[worker->touchSlot].y = (event.value - worker->absRange.y)*screenHeight/worker->absRange.height;  // Scale acording to absRange
                }

                if (event.code == ABS_MT_TRACKING_ID)
                {
                    if ( (event.value < 0) && (worker->touchSlot < MAX_TOUCH_POINTS) )
                    {
                        // Touch has ended for this point
                        touchPosition[worker->touchSlot].x = -1;
                        touchPosition[worker->touchSlot].y = -1;
                    }
                }
            }

            // Button parsing
            if (event.type == EV_KEY)
            {

                // Mouse button parsing
                if ((event.code == BTN_TOUCH) || (event.code == BTN_LEFT))
                {
                    currentMouseStateEvdev[MOUSE_LEFT_BUTTON] = event.value;

                    #if defined(SUPPORT_GESTURES_SYSTEM)
                        if (event.value > 0) touchAction = TOUCH_DOWN;
                        else touchAction = TOUCH_UP;
                        gestureUpdate = true;
                    #endif
                }

                if (event.code == BTN_RIGHT) currentMouseStateEvdev[MOUSE_RIGHT_BUTTON] =  event.value;

                if (event.code == BTN_MIDDLE) currentMouseStateEvdev[MOUSE_MIDDLE_BUTTON] =  event.value;

                // Keyboard button parsing
                if((event.code >= 1) && (event.code <= 255))     //Keyboard keys appear for codes 1 to 255
                {
                    keycode = keymap_US[event.code & 0xFF];     // The code we get is a scancode so we look up the apropriate keycode
                    // Make sure we got a valid keycode
                    if((keycode > 0) && (keycode < sizeof(currentKeyState)))
                    {
                        // Store the key information for raylib to later use
                        currentKeyStateEvdev[keycode] = event.value;
                        if(event.value > 0)
                        {
                            // Add the key int the fifo
                            lastKeyPressedEvdev.Contents[lastKeyPressedEvdev.Head] = keycode;   // Put the data at the front of the fifo snake
                            lastKeyPressedEvdev.Head = (lastKeyPressedEvdev.Head + 1) & 0x07;   // Increment the head pointer forwards and binary wraparound after 7 (fifo is 8 elements long)
                            // TODO: This fifo is not fully threadsafe with multiple writers, so multiple keyboards hitting a key at the exact same time could miss a key (double write to head before it was incremented)
                        }
                        TraceLog(LOG_DEBUG, "KEY%s ScanCode: %4i KeyCode: %4i",event.value == 0 ? "UP":"DOWN", event.code, keycode);
                    }
                }

            }

            // Screen confinement
            if (mousePosition.x < 0) mousePosition.x = 0;
            if (mousePosition.x > screenWidth/mouseScale.x) mousePosition.x = screenWidth/mouseScale.x;

            if (mousePosition.y < 0) mousePosition.y = 0;
            if (mousePosition.y > screenHeight/mouseScale.y) mousePosition.y = screenHeight/mouseScale.y;

            // Gesture update
            if (gestureUpdate)
            {
#if defined(SUPPORT_GESTURES_SYSTEM)
                GestureEvent gestureEvent = { 0 };

                gestureEvent.pointCount = 0;
                gestureEvent.touchAction = touchAction;

                if (touchPosition[0].x >= 0) gestureEvent.pointCount++;
                if (touchPosition[1].x >= 0) gestureEvent.pointCount++;
                if (touchPosition[2].x >= 0) gestureEvent.pointCount++;
                if (touchPosition[3].x >= 0) gestureEvent.pointCount++;

                gestureEvent.pointerId[0] = 0;
                gestureEvent.pointerId[1] = 1;
                gestureEvent.pointerId[2] = 2;
                gestureEvent.pointerId[3] = 3;

                gestureEvent.position[0] = touchPosition[0];
                gestureEvent.position[1] = touchPosition[1];
                gestureEvent.position[2] = touchPosition[2];
                gestureEvent.position[3] = touchPosition[3];

                ProcessGestureEvent(gestureEvent);
#endif
            }
        }
        else
        {
            usleep(5000); // Sleep for 5ms to avoid hogging CPU time
        }
    }

    close(worker->fd);

    return NULL;
}

// Init gamepad system
static void InitGamepad(void)
{
    char gamepadDev[128] = "";

    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        sprintf(gamepadDev, "%s%i", DEFAULT_GAMEPAD_DEV, i);

        if ((gamepadStream[i] = open(gamepadDev, O_RDONLY|O_NONBLOCK)) < 0)
        {
            // NOTE: Only show message for first gamepad
            if (i == 0) TraceLog(LOG_WARNING, "Gamepad device could not be opened, no gamepad available");
        }
        else
        {
            gamepadReady[i] = true;

            // NOTE: Only create one thread
            if (i == 0)
            {
                int error = pthread_create(&gamepadThreadId, NULL, &GamepadThread, NULL);

                if (error != 0) TraceLog(LOG_WARNING, "Error creating gamepad input event thread");
                else  TraceLog(LOG_INFO, "Gamepad device initialized successfully");
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
    struct js_event gamepadEvent;

    while (!windowShouldClose)
    {
        for (int i = 0; i < MAX_GAMEPADS; i++)
        {
            if (read(gamepadStream[i], &gamepadEvent, sizeof(struct js_event)) == (int)sizeof(struct js_event))
            {
                gamepadEvent.type &= ~JS_EVENT_INIT;     // Ignore synthetic events

                // Process gamepad events by type
                if (gamepadEvent.type == JS_EVENT_BUTTON)
                {
                    TraceLog(LOG_DEBUG, "Gamepad button: %i, value: %i", gamepadEvent.number, gamepadEvent.value);

                    if (gamepadEvent.number < MAX_GAMEPAD_BUTTONS)
                    {
                        // 1 - button pressed, 0 - button released
                        currentGamepadState[i][gamepadEvent.number] = (int)gamepadEvent.value;

                        if ((int)gamepadEvent.value == 1) lastGamepadButtonPressed = gamepadEvent.number;
                        else lastGamepadButtonPressed = -1;
                    }
                }
                else if (gamepadEvent.type == JS_EVENT_AXIS)
                {
                    TraceLog(LOG_DEBUG, "Gamepad axis: %i, value: %i", gamepadEvent.number, gamepadEvent.value);

                    if (gamepadEvent.number < MAX_GAMEPAD_AXIS)
                    {
                        // NOTE: Scaling of gamepadEvent.value to get values between -1..1
                        gamepadAxisState[i][gamepadEvent.number] = (float)gamepadEvent.value/32768;
                    }
                }
            }
            else
            {
                usleep(1000); //Sleep for 1ms to avoid hogging CPU time
            }
        }
    }

    return NULL;
}
#endif      // PLATFORM_RPI

// Plays raylib logo appearing animation
static void LogoAnimation(void)
{
#if !defined(PLATFORM_WEB)
    int logoPositionX = screenWidth/2 - 128;
    int logoPositionY = screenHeight/2 - 128;

    int framesCounter = 0;
    int lettersCount = 0;

    int topSideRecWidth = 16;
    int leftSideRecHeight = 16;

    int bottomSideRecWidth = 16;
    int rightSideRecHeight = 16;

    int state = 0;                  // Tracking animation states (State Machine)
    float alpha = 1.0f;             // Useful for fading

    while (!WindowShouldClose() && (state != 4))    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (state == 0)                 // State 0: Small box blinking
        {
            framesCounter++;

            if (framesCounter == 84)
            {
                state = 1;
                framesCounter = 0;      // Reset counter... will be used later...
            }
        }
        else if (state == 1)            // State 1: Top and left bars growing
        {
            topSideRecWidth += 4;
            leftSideRecHeight += 4;

            if (topSideRecWidth == 256) state = 2;
        }
        else if (state == 2)            // State 2: Bottom and right bars growing
        {
            bottomSideRecWidth += 4;
            rightSideRecHeight += 4;

            if (bottomSideRecWidth == 256) state = 3;
        }
        else if (state == 3)            // State 3: Letters appearing (one by one)
        {
            framesCounter++;

            if (framesCounter/12)       // Every 12 frames, one more letter!
            {
                lettersCount++;
                framesCounter = 0;
            }

            if (lettersCount >= 10)     // When all letters have appeared, just fade out everything
            {
                alpha -= 0.02f;

                if (alpha <= 0.0f)
                {
                    alpha = 0.0f;
                    state = 4;
                }
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            if (state == 0)
            {
                if ((framesCounter/12)%2) DrawRectangle(logoPositionX, logoPositionY, 16, 16, BLACK);
            }
            else if (state == 1)
            {
                DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, BLACK);
                DrawRectangle(logoPositionX, logoPositionY, 16, leftSideRecHeight, BLACK);
            }
            else if (state == 2)
            {
                DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, BLACK);
                DrawRectangle(logoPositionX, logoPositionY, 16, leftSideRecHeight, BLACK);

                DrawRectangle(logoPositionX + 240, logoPositionY, 16, rightSideRecHeight, BLACK);
                DrawRectangle(logoPositionX, logoPositionY + 240, bottomSideRecWidth, 16, BLACK);
            }
            else if (state == 3)
            {
                DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, Fade(BLACK, alpha));
                DrawRectangle(logoPositionX, logoPositionY + 16, 16, leftSideRecHeight - 32, Fade(BLACK, alpha));

                DrawRectangle(logoPositionX + 240, logoPositionY + 16, 16, rightSideRecHeight - 32, Fade(BLACK, alpha));
                DrawRectangle(logoPositionX, logoPositionY + 240, bottomSideRecWidth, 16, Fade(BLACK, alpha));

                DrawRectangle(screenWidth/2 - 112, screenHeight/2 - 112, 224, 224, Fade(RAYWHITE, alpha));

                DrawText(TextSubtext("raylib", 0, lettersCount), screenWidth/2 - 44, screenHeight/2 + 48, 50, Fade(BLACK, alpha));
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }
#endif

    showLogo = false;  // Prevent for repeating when reloading window (Android)
}
