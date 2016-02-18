/**********************************************************************************************
*
*   raylib.core
*
*   Basic functions to manage windows, OpenGL context and input on multiple platforms
*
*   The following platforms are supported:
*       PLATFORM_DESKTOP - Windows, Linux, Mac (OSX)
*       PLATFORM_ANDROID - Only OpenGL ES 2.0 devices
*       PLATFORM_RPI - Rapsberry Pi (tested on Raspbian)
*       PLATFORM_WEB - Emscripten, HTML5
*
*   On PLATFORM_DESKTOP, the external lib GLFW3 (www.glfw.com) is used to manage graphic
*   device, OpenGL context and input on multiple operating systems (Windows, Linux, OSX).
*
*   On PLATFORM_ANDROID, graphic device is managed by EGL and input system by Android activity.
*
*   On PLATFORM_RPI, graphic device is managed by EGL and input system is coded in raw mode.
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
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

#include "raylib.h"         // raylib main header
#include "rlgl.h"           // raylib OpenGL abstraction layer to OpenGL 1.1, 3.3+ or ES2
#include "utils.h"          // TraceLog() function
                            // NOTE: Includes Android fopen map, InitAssetManager()
                            
#define RAYMATH_IMPLEMENTATION  // Use raymath as a header-only library (includes implementation)
#define RAYMATH_EXTERN_INLINE   // Compile raymath functions as static inline (remember, it's a compiler hint)
#include "raymath.h"            // Required for Vector3 and Matrix functions

#include <stdio.h>          // Standard input / output lib
#include <stdlib.h>         // Declares malloc() and free() for memory management, rand(), atexit()
#include <stdint.h>         // Required for typedef unsigned long long int uint64_t, used by hi-res timer
#include <time.h>           // Useful to initialize random seed - Android/RPI hi-res timer (NOTE: Linux only!)
#include <math.h>           // Math related functions, tan() used to set perspective
#include <string.h>         // String function definitions, memset()
#include <errno.h>          // Macros for reporting and retrieving error conditions through error codes

#if defined(PLATFORM_DESKTOP)
    #define GLAD_EXTENSIONS_LOADER
    #if defined(GLEW_EXTENSIONS_LOADER)
        #define GLEW_STATIC
        #include <GL/glew.h>        // GLEW extensions loading lib
    #elif defined(GLAD_EXTENSIONS_LOADER)
        #include "glad.h"           // GLAD library: Manage OpenGL headers and extensions
    #endif
#endif

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    //#define GLFW_INCLUDE_NONE   // Disable the standard OpenGL header inclusion on GLFW3
    #include <GLFW/glfw3.h>       // GLFW3 library: Windows, OpenGL context and Input management

    #ifdef __linux
        #define GLFW_EXPOSE_NATIVE_X11   // Linux specific definitions for getting
        #define GLFW_EXPOSE_NATIVE_GLX   // native functions like glfwGetX11Window
        #include <GLFW/glfw3native.h>    // which are required for hiding mouse
    #endif
    //#include <GL/gl.h>        // OpenGL functions (GLFW3 already includes gl.h)
    //#define GLFW_DLL          // Using GLFW DLL on Windows -> No, we use static version!
#endif

#if defined(PLATFORM_ANDROID)
    #include <jni.h>                        // Java native interface
    #include <android/sensor.h>             // Android sensors functions
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

    #include <sys/ioctl.h>      // UNIX System call for device-specific input/output operations - ioctl()
    #include <linux/kd.h>       // Linux: KDSKBMODE, K_MEDIUMRAM constants definition
    #include <linux/input.h>    // Linux: Keycodes constants definition (KEY_A, ...)
    #include <linux/joystick.h>

    #include "bcm_host.h"       // Raspberry Pi VideoCore IV access functions

    #include "EGL/egl.h"        // Khronos EGL library - Native platform display device control functions
    #include "EGL/eglext.h"     // Khronos EGL library - Extensions
    #include "GLES2/gl2.h"      // Khronos OpenGL ES 2.0 library
    
    // Old device inputs system
    #define DEFAULT_KEYBOARD_DEV      STDIN_FILENO            // Standard input
    #define DEFAULT_MOUSE_DEV         "/dev/input/mouse0"
    #define DEFAULT_GAMEPAD_DEV       "/dev/input/js0"

    // New device input events (evdev) (must be detected)
    //#define DEFAULT_KEYBOARD_DEV    "/dev/input/eventN"
    //#define DEFAULT_MOUSE_DEV       "/dev/input/eventN"
    //#define DEFAULT_GAMEPAD_DEV     "/dev/input/eventN"
#endif

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
    #include <emscripten/html5.h>
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define STORAGE_FILENAME     "storage.data"

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
static GLFWwindow *window;                      // Native window (graphic device)
static bool windowMinimized = false;
#elif defined(PLATFORM_ANDROID)
static struct android_app *app;                 // Android activity
static struct android_poll_source *source;      // Android events polling source
static int ident, events;
static bool windowReady = false;                // Used to detect display initialization
static bool appEnabled = true;                  // Used to detec if app is active
static bool contextRebindRequired = false;      // Used to know context rebind required
static int previousButtonState[128] = { 1 };    // Required to check if button pressed/released once
static int currentButtonState[128] = { 1 };     // Required to check if button pressed/released once
#elif defined(PLATFORM_RPI)
static EGL_DISPMANX_WINDOW_T nativeWindow;      // Native window (graphic device)

// Keyboard input variables
// NOTE: For keyboard we will use the standard input (but reconfigured...)
static struct termios defaultKeyboardSettings;  // Used to store default keyboard settings
static int defaultKeyboardMode;                 // Used to store default keyboard mode
static int keyboardMode = 0;                    // Register Keyboard mode: 1 - KEYCODES, 2 - ASCII

// This array maps Unix keycodes to ASCII equivalent and to GLFW3 equivalent for special function keys (>256)
const short UnixKeycodeToASCII[128] = { 256, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 45, 61, 259, 9, 81, 87, 69, 82, 84, 89, 85, 73, 79, 80, 91, 93, 257, 341, 65, 83, 68,
                            70, 71, 72, 74, 75, 76, 59, 39, 96, 340, 92, 90, 88, 67, 86, 66, 78, 77, 44, 46, 47, 344, -1, 342, 32, -1, 290, 291, 292, 293, 294, 295, 296,
                            297, 298, 299, -1, -1, -1, -1, -1, 45, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 257, 345, 47, -1,
                            346, -1, -1, 265, -1, 263, 262, -1, 264, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

// Mouse input variables
static int mouseStream = -1;                    // Mouse device file descriptor
static bool mouseReady = false;                 // Flag to know if mouse is ready
pthread_t mouseThreadId;                        // Mouse reading thread id

// Gamepad input variables
static int gamepadStream = -1;                  // Gamepad device file descriptor
static bool gamepadReady = false;               // Flag to know if gamepad is ready
pthread_t gamepadThreadId;                      // Gamepad reading thread id
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
static EGLDisplay display;          // Native display device (physical screen connection)
static EGLSurface surface;          // Surface to draw on, framebuffers (connected to context)
static EGLContext context;          // Graphic context, mode in which drawing can be done
static EGLConfig config;            // Graphic config
static uint64_t baseTime;                   // Base time measure for hi-res timer
static bool windowShouldClose = false;      // Flag to set window for closing
#endif

static unsigned int displayWidth, displayHeight;     // Display width and height (monitor, device-screen, LCD, ...)
static int screenWidth, screenHeight;       // Screen width and height (used render area)
static int renderWidth, renderHeight;       // Framebuffer width and height (render area)
                                            // NOTE: Framebuffer could include black bars

static int renderOffsetX = 0;               // Offset X from render area (must be divided by 2)
static int renderOffsetY = 0;               // Offset Y from render area (must be divided by 2)
static bool fullscreen = false;             // Fullscreen mode (useful only for PLATFORM_DESKTOP)
static Matrix downscaleView;                // Matrix to downscale view (in case screen size bigger than display size)

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
static Vector2 touchPosition[MAX_TOUCH_POINTS];     // Touch position on screen
#endif

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI) || defined(PLATFORM_WEB)
static const char *windowTitle;             // Window text title...

static bool customCursor = false;           // Tracks if custom cursor has been set
static bool cursorOnScreen = false;         // Tracks if cursor is inside client area
static Texture2D cursor;                    // Cursor texture

static Vector2 mousePosition;               // Mouse position on screen

static char previousKeyState[512] = { 0 };  // Required to check if key pressed/released once
static char currentKeyState[512] = { 0 };   // Required to check if key pressed/released once

static char previousMouseState[3] = { 0 };  // Required to check if mouse btn pressed/released once
static char currentMouseState[3] = { 0 };   // Required to check if mouse btn pressed/released once

static char previousGamepadState[32] = {0}; // Required to check if gamepad btn pressed/released once
static char currentGamepadState[32] = {0};  // Required to check if gamepad btn pressed/released once

static int previousMouseWheelY = 0;         // Required to track mouse wheel variation
static int currentMouseWheelY = 0;          // Required to track mouse wheel variation

static int exitKey = KEY_ESCAPE;            // Default exit key (ESC)
static int lastKeyPressed = -1;             // Register last key pressed

static bool cursorHidden;                   // Track if cursor is hidden

static char **dropFilesPath;                // Store dropped files paths as strings
static int dropFilesCount = 0;              // Count stored strings
#endif

static double currentTime, previousTime;    // Used to track timmings
static double updateTime, drawTime;         // Time measures for update and draw
static double frameTime;                    // Time measure for one frame
static double targetTime = 0.0;             // Desired time for one frame, if 0 not applied

static char configFlags = 0;                // Configuration flags (bit  based)
static bool showLogo = false;               // Track if showing logo at init is enabled

//----------------------------------------------------------------------------------
// Other Modules Functions Declaration (required by core)
//----------------------------------------------------------------------------------
extern void LoadDefaultFont(void);              // [Module: text] Loads default font on InitWindow()
extern void UnloadDefaultFont(void);            // [Module: text] Unloads default font from GPU memory

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void InitDisplay(int width, int height);         // Initialize display device and framebuffer
static void InitGraphics(void);                         // Initialize OpenGL graphics
static void InitTimer(void);                            // Initialize timer
static double GetTime(void);                            // Returns time since InitTimer() was run
static bool GetKeyStatus(int key);                      // Returns if a key has been pressed
static bool GetMouseButtonStatus(int button);           // Returns if a mouse button has been pressed
static void SwapBuffers(void);                          // Copy back buffer to front buffers
static void PollInputEvents(void);                      // Register user events
static void LogoAnimation(void);                        // Plays raylib logo appearing animation
static void SetupFramebufferSize(int displayWidth, int displayHeight);

#if defined(PLATFORM_RPI)
static void InitKeyboard(void);                         // Init raw keyboard system (standard input reading)
static void ProcessKeyboard(void);                      // Process keyboard events
static void RestoreKeyboard(void);                      // Restore keyboard system
static void InitMouse(void);                            // Mouse initialization (including mouse thread)
static void *MouseThread(void *arg);                    // Mouse reading thread
static void InitGamepad(void);                          // Init raw gamepad input
static void *GamepadThread(void *arg);                  // Mouse reading thread
#endif

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
#endif

#if defined(PLATFORM_DESKTOP)
static void WindowDropCallback(GLFWwindow *window, int count, const char **paths);         // GLFW3 Window Drop Callback, runs when drop files into window
#endif

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI)
static void TakeScreenshot(void);                                                          // Takes a screenshot and saves it in the same folder as executable
#endif

#if defined(PLATFORM_ANDROID)
static void AndroidCommandCallback(struct android_app *app, int32_t cmd);                  // Process Android activity lifecycle commands
static int32_t AndroidInputCallback(struct android_app *app, AInputEvent *event);          // Process Android inputs
#endif

#if defined(PLATFORM_WEB)
static EM_BOOL EmscriptenFullscreenChangeCallback(int eventType, const EmscriptenFullscreenChangeEvent *e, void *userData);
static EM_BOOL EmscriptenInputCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - Window and OpenGL Context Functions
//----------------------------------------------------------------------------------
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI) || defined(PLATFORM_WEB)
// Initialize Window and Graphics Context (OpenGL)
void InitWindow(int width, int height, const char *title)
{
    TraceLog(INFO, "Initializing raylib (v1.4.0)");

    // Store window title (could be useful...)
    windowTitle = title;

    // Init device display (monitor, LCD, ...)
    InitDisplay(width, height);

    // Init OpenGL graphics
    InitGraphics();

    // Load default font for convenience
    // NOTE: External function (defined in module: text)
    LoadDefaultFont();

    // Init hi-res timer
    InitTimer();

#if defined(PLATFORM_RPI)
    // Init raw input system
    InitMouse();        // Mouse init
    InitKeyboard();     // Keyboard init
    InitGamepad();      // Gamepad init
#endif

#if defined(PLATFORM_WEB)
    emscripten_set_fullscreenchange_callback(0, 0, 1, EmscriptenFullscreenChangeCallback);

    // NOTE: Some code examples
    //emscripten_set_touchstart_callback(0, NULL, 1, Emscripten_HandleTouch);
    //emscripten_set_touchend_callback("#canvas", data, 0, Emscripten_HandleTouch);
    emscripten_set_touchstart_callback("#canvas", NULL, 1, EmscriptenInputCallback);
    emscripten_set_touchend_callback("#canvas", NULL, 1, EmscriptenInputCallback);
    emscripten_set_touchmove_callback("#canvas", NULL, 1, EmscriptenInputCallback);
    emscripten_set_touchcancel_callback("#canvas", NULL, 1, EmscriptenInputCallback);
    
    // TODO: Add gamepad support (not provided by GLFW3 on emscripten)
    //emscripten_set_gamepadconnected_callback(NULL, 1, EmscriptenInputCallback);
    //emscripten_set_gamepaddisconnected_callback(NULL, 1, EmscriptenInputCallback);
#endif

    mousePosition.x = (float)screenWidth/2.0f;
    mousePosition.y = (float)screenHeight/2.0f;

    // raylib logo appearing animation (if enabled)
    if (showLogo)
    {
        SetTargetFPS(60);
        LogoAnimation();
    }
}

#elif defined(PLATFORM_ANDROID)
// Android activity initialization
void InitWindow(int width, int height, struct android_app *state)
{
    TraceLog(INFO, "Initializing raylib (v1.4.0)");

    app_dummy();

    screenWidth = width;
    screenHeight = height;

    app = state;

    // Set desired windows flags before initializing anything
    ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_FULLSCREEN, 0);  //AWINDOW_FLAG_SCALED, AWINDOW_FLAG_DITHER
    //ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_FORCE_NOT_FULLSCREEN, AWINDOW_FLAG_FULLSCREEN);

    int orientation = AConfiguration_getOrientation(app->config);

    if (orientation == ACONFIGURATION_ORIENTATION_PORT) TraceLog(INFO, "PORTRAIT window orientation");
    else if (orientation == ACONFIGURATION_ORIENTATION_LAND) TraceLog(INFO, "LANDSCAPE window orientation");

    // TODO: Automatic orientation doesn't seem to work
    if (width <= height)
    {
        AConfiguration_setOrientation(app->config, ACONFIGURATION_ORIENTATION_PORT);
        TraceLog(WARNING, "Window set to portraid mode");
    }
    else
    {
        AConfiguration_setOrientation(app->config, ACONFIGURATION_ORIENTATION_LAND);
        TraceLog(WARNING, "Window set to landscape mode");
    }

    //AConfiguration_getDensity(app->config);
    //AConfiguration_getKeyboard(app->config);
    //AConfiguration_getScreenSize(app->config);
    //AConfiguration_getScreenLong(app->config);

    //state->userData = &engine;
    app->onAppCmd = AndroidCommandCallback;
    app->onInputEvent = AndroidInputCallback;
    
    InitAssetManager(app->activity->assetManager);

    TraceLog(INFO, "Android app initialized successfully");

    // Init button states values (default up)
    for(int i = 0; i < 128; i++)
    {
        currentButtonState[i] = 1;
        previousButtonState[i] = 1;
    }

    // Wait for window to be initialized (display and context)
    while (!windowReady)
    {
        // Process events loop
        while ((ident = ALooper_pollAll(0, NULL, &events,(void**)&source)) >= 0)
        {
            // Process this event
            if (source != NULL) source->process(app, source);

            // NOTE: Never close window, native activity is controlled by the system!
            //if (app->destroyRequested != 0) windowShouldClose = true;
        }
    }
}
#endif

// Close Window and Terminate Context
void CloseWindow(void)
{
    UnloadDefaultFont();

    rlglClose();                // De-init rlgl

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    glfwDestroyWindow(window);
    glfwTerminate();
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
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

    TraceLog(INFO, "Window closed successfully");
}

// Detect if KEY_ESCAPE pressed or Close icon pressed
bool WindowShouldClose(void)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    // While window minimized, stop loop execution
    while (windowMinimized) glfwPollEvents();

    return (glfwWindowShouldClose(window));
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
    return windowShouldClose;
#endif
}

// Detect if window has been minimized (or lost focus)
bool IsWindowMinimized(void)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    return windowMinimized;
#else
    return false;
#endif
}

// Fullscreen toggle
// TODO: When destroying window context is lost and resources too, take care!
void ToggleFullscreen(void)
{
#if defined(PLATFORM_DESKTOP)
    fullscreen = !fullscreen;          // Toggle fullscreen flag

    rlglClose();                       // De-init rlgl
    glfwDestroyWindow(window);         // Destroy the current window (we will recreate it!)

    InitWindow(screenWidth, screenHeight, windowTitle);
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
    TraceLog(WARNING, "Could not toggle to windowed mode");
#endif
}

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI) || defined(PLATFORM_WEB)
// Set a custom cursor icon/image
void SetCustomCursor(const char *cursorImage)
{
    if (customCursor) UnloadTexture(cursor);

    cursor = LoadTexture(cursorImage);

#if defined(PLATFORM_DESKTOP)
    // NOTE: emscripten not implemented
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
#endif
    customCursor = true;
}

// Set a custom key to exit program
// NOTE: default exitKey is ESCAPE
void SetExitKey(int key)
{
    exitKey = key;
}
#endif

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

// Sets Background Color
void ClearBackground(Color color)
{
    // Clear full framebuffer (not only render area) to color
    rlClearColor(color.r, color.g, color.b, color.a);
}

// Setup drawing canvas to start drawing
void BeginDrawing(void)
{
    currentTime = GetTime();            // Number of elapsed seconds since InitTimer() was called
    updateTime = currentTime - previousTime;
    previousTime = currentTime;

    if (IsPosproShaderEnabled()) rlEnablePostproFBO();

    rlClearScreenBuffers();

    rlLoadIdentity();                   // Reset current matrix (MODELVIEW)

    rlMultMatrixf(MatrixToFloat(downscaleView));       // If downscale required, apply it here

    //rlTranslatef(0.375, 0.375, 0);    // HACK to have 2D pixel-perfect drawing on OpenGL 1.1
                                        // NOTE: Not required with OpenGL 3.3+
}

// Setup drawing canvas with extended parameters
void BeginDrawingEx(int blendMode, Shader shader, Matrix transform)
{
    BeginDrawing();
    
    SetBlendMode(blendMode);
    SetPostproShader(shader);
    
    rlMultMatrixf(MatrixToFloat(transform));
}

// End canvas drawing and Swap Buffers (Double Buffering)
void EndDrawing(void)
{
    rlglDraw();                     // Draw Buffers (Only OpenGL 3+ and ES2)

    if (IsPosproShaderEnabled()) rlglDrawPostpro(); // Draw postprocessing effect (shader)

    SwapBuffers();                  // Copy back buffer to front buffer

    PollInputEvents();              // Poll user events
    
    currentTime = GetTime();
    drawTime = currentTime - previousTime;
    previousTime = currentTime;

    frameTime = updateTime + drawTime;

    double extraTime = 0.0;

    while (frameTime < targetTime)
    {
        // Implement a delay
        currentTime = GetTime();
        extraTime = currentTime - previousTime;
        previousTime = currentTime;
        frameTime += extraTime;
    }
}

// Initializes 3D mode for drawing (Camera setup)
void Begin3dMode(Camera camera)
{
    rlglDraw();                         // Draw Buffers (Only OpenGL 3+ and ES2)

    rlMatrixMode(RL_PROJECTION);        // Switch to projection matrix

    rlPushMatrix();                     // Save previous matrix, which contains the settings for the 2d ortho projection
    rlLoadIdentity();                   // Reset current matrix (PROJECTION)

    // Setup perspective projection
    float aspect = (float)screenWidth/(float)screenHeight;
    double top = 0.1*tan(45.0*PI/360.0);
    double right = top*aspect;

    // NOTE: zNear and zFar values are important when computing depth buffer values
    rlFrustum(-right, right, -top, top, 0.1f, 1000.0f);

    rlMatrixMode(RL_MODELVIEW);         // Switch back to modelview matrix
    rlLoadIdentity();                   // Reset current matrix (MODELVIEW)

    // Setup Camera view
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
    rlMultMatrixf(MatrixToFloat(matView));      // Multiply MODELVIEW matrix by view matrix (camera)
}

// Ends 3D mode and returns to default 2D orthographic mode
void End3dMode(void)
{
    rlglDraw();                         // Draw Buffers (Only OpenGL 3+ and ES2)

    rlMatrixMode(RL_PROJECTION);        // Switch to projection matrix
    rlPopMatrix();                      // Restore previous matrix (PROJECTION) from matrix stack

    rlMatrixMode(RL_MODELVIEW);         // Get back to modelview matrix
    rlLoadIdentity();                   // Reset current matrix (MODELVIEW)

    //rlTranslatef(0.375, 0.375, 0);      // HACK to ensure pixel-perfect drawing on OpenGL (after exiting 3D mode)
}

// Set target FPS for the game
void SetTargetFPS(int fps)
{
    targetTime = 1.0/(double)fps;

    TraceLog(INFO, "Target time per frame: %02.03f milliseconds", (float)targetTime*1000);
}

// Returns current FPS
float GetFPS(void)
{
    return (float)(1.0/frameTime);
}

// Returns time in seconds for one frame
float GetFrameTime(void)
{
    // As we are operate quite a lot with frameTime, 
    // it could be no stable, so we round it before passing it around
    // NOTE: There are still problems with high framerates (>500fps)
    double roundedFrameTime =  round(frameTime*10000)/10000.0;

    return (float)roundedFrameTime;    // Time in seconds to run a frame
}

// Converts Color to float array and normalizes
float *ColorToFloat(Color color)
{
    static float buffer[4];

    buffer[0] = (float)color.r/255;
    buffer[1] = (float)color.g/255;
    buffer[2] = (float)color.b/255;
    buffer[3] = (float)color.a/255;

    return buffer;
}

// Converts Vector3 to float array
float *VectorToFloat(Vector3 vec)
{
    static float buffer[3];

    buffer[0] = vec.x;
    buffer[1] = vec.y;
    buffer[2] = vec.z;

    return buffer;
}

// Converts Matrix to float array
// NOTE: Returned vector is a transposed version of the Matrix struct, 
// it should be this way because, despite raymath use OpenGL column-major convention,
// Matrix struct memory alignment and variables naming are not coherent
float *MatrixToFloat(Matrix mat)
{
    static float buffer[16];

    buffer[0] = mat.m0;
    buffer[1] = mat.m4;
    buffer[2] = mat.m8;
    buffer[3] = mat.m12;
    buffer[4] = mat.m1;
    buffer[5] = mat.m5;
    buffer[6] = mat.m9;
    buffer[7] = mat.m13;
    buffer[8] = mat.m2;
    buffer[9] = mat.m6;
    buffer[10] = mat.m10;
    buffer[11] = mat.m14;
    buffer[12] = mat.m3;
    buffer[13] = mat.m7;
    buffer[14] = mat.m11;
    buffer[15] = mat.m15;

    return buffer;
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

// Returns hexadecimal value for a Color
int GetHexValue(Color color)
{
    return (((int)color.r << 24) | ((int)color.g << 16) | ((int)color.b << 8) | (int)color.a);
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

    return (rand()%(abs(max-min)+1) + min);
}

// Fades color by a percentadge
Color Fade(Color color, float alpha)
{
    if (alpha < 0.0f) alpha = 0.0f;
    else if (alpha > 1.0f) alpha = 1.0f;
    
    float colorAlpha = (float)color.a*alpha;

    return (Color){color.r, color.g, color.b, (unsigned char)colorAlpha};
}

// Enable some window/system configurations
void SetConfigFlags(char flags)
{
    configFlags = flags;

    if (configFlags & FLAG_SHOW_LOGO) showLogo = true;
    if (configFlags & FLAG_FULLSCREEN_MODE) fullscreen = true;
}

// Activates raylib logo at startup
void ShowLogo(void)
{
    showLogo = true;
}

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
// Check if a file have been dropped into window
bool IsFileDropped(void)
{
    if (dropFilesCount > 0) return true;
    else return false;
}

// Retrieve dropped files into window
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
#endif

// Storage save integer value (to defined position)
// NOTE: Storage positions is directly related to file memory layout (4 bytes each integer)
void StorageSaveValue(int position, int value)
{
    FILE *storageFile = NULL;

    // Try open existing file to append data
    storageFile = fopen(STORAGE_FILENAME, "rb+");      

    // If file doesn't exist, create a new storage data file
    if (!storageFile) storageFile = fopen(STORAGE_FILENAME, "wb");

    if (!storageFile) TraceLog(WARNING, "Storage data file could not be created");
    else
    {
        // Get file size
        fseek(storageFile, 0, SEEK_END);
        int fileSize = ftell(storageFile);  // Size in bytes
        fseek(storageFile, 0, SEEK_SET);
        
        if (fileSize < (position*4)) TraceLog(WARNING, "Storage position could not be found");
        else
        {
            fseek(storageFile, (position*4), SEEK_SET);
            fwrite(&value, 1, 4, storageFile);
        }
        
        fclose(storageFile);
    }
}

// Storage load integer value (from defined position)
// NOTE: If requested position could not be found, value 0 is returned
int StorageLoadValue(int position)
{
    int value = 0;
    
    // Try open existing file to append data
    FILE *storageFile = fopen(STORAGE_FILENAME, "rb");      

    if (!storageFile) TraceLog(WARNING, "Storage data file could not be found");
    else
    {
        // Get file size
        fseek(storageFile, 0, SEEK_END);
        int fileSize = ftell(storageFile);  // Size in bytes
        rewind(storageFile);
        
        if (fileSize < (position*4)) TraceLog(WARNING, "Storage position could not be found");
        else
        {
            fseek(storageFile, (position*4), SEEK_SET);
            fread(&value, 1, 4, storageFile);
        }
        
        fclose(storageFile);
    }
    
    return value;
}

// Returns a ray trace from mouse position
//http://www.songho.ca/opengl/gl_transform.html
//http://www.songho.ca/opengl/gl_matrix.html
//http://www.sjbaker.org/steve/omniv/matrices_can_be_your_friends.html
//https://www.opengl.org/archives/resources/faq/technical/transformations.htm
Ray GetMouseRay(Vector2 mousePosition, Camera camera)
{
    // Tutorial used: https://mkonrad.net/2014/08/07/simple-opengl-object-picking-in-3d.html
    // Similar to http://antongerdelan.net, the problem is maybe in MatrixPerspective vs MatrixFrustum
    // or matrix order (transpose it or not... that's the question)
    
    Ray ray;
    
    // Calculate normalized device coordinates
    // NOTE: y value is negative
    float x = (2.0f*mousePosition.x)/(float)GetScreenWidth() - 1.0f;
    float y = 1.0f - (2.0f*mousePosition.y)/(float)GetScreenHeight();
    float z = 1.0f;
    
    // Store values in a vector
    Vector3 deviceCoords = {x, y, z};
    
    // Device debug message
    TraceLog(INFO, "device(%f, %f, %f)", deviceCoords.x, deviceCoords.y, deviceCoords.z);
    
    // Calculate projection matrix (from perspective instead of frustum
    Matrix matProj = MatrixPerspective(45.0f, (float)((float)GetScreenWidth() / (float)GetScreenHeight()), 0.01f, 1000.0f);
    
    // Calculate view matrix from camera look at
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
    
    // Do I need to transpose it? It seems that yes...
    // NOTE: matrix order is maybe incorrect... In OpenGL to get world position from
    // camera view it just needs to get inverted, but here we need to transpose it too.
    // For example, if you get view matrix, transpose and inverted and you transform it
    // to a vector, you will get its 3d world position coordinates (camera.position).
    // If you don't transpose, final position will be wrong.
    MatrixTranspose(&matView);
    
    // Calculate unproject matrix (multiply projection matrix and view matrix) and invert it
    Matrix matProjView = MatrixMultiply(matProj, matView);
    MatrixInvert(&matProjView);
    
    // Calculate far and near points
    Quaternion near = { deviceCoords.x, deviceCoords.y, 0.0f, 1.0f};
    Quaternion far = { deviceCoords.x, deviceCoords.y, 1.0f, 1.0f};
    
    // Multiply points by unproject matrix
    QuaternionTransform(&near, matProjView);
    QuaternionTransform(&far, matProjView);
    
    // Calculate normalized world points in vectors
    Vector3 nearPoint = {near.x / near.w, near.y / near.w, near.z / near.w};
    Vector3 farPoint = {far.x / far.w, far.y / far.w, far.z / far.w};
    
    // Calculate normalized direction vector
    Vector3 direction = VectorSubtract(farPoint, nearPoint);
    VectorNormalize(&direction);
    
    // Apply calculated vectors to ray
    ray.position = camera.position;
    ray.direction = direction;
    
    return ray;
}

// Returns the screen space position from a 3d world space position
Vector2 WorldToScreen(Vector3 position, Camera camera)
{    
    // Calculate projection matrix (from perspective instead of frustum
    Matrix matProj = MatrixPerspective(45.0f, (float)((float)GetScreenWidth() / (float)GetScreenHeight()), 0.01f, 1000.0f);
    
    // Calculate view matrix from camera look at (and transpose it)
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
    MatrixTranspose(&matView);
    
    // Convert world position vector to quaternion
    Quaternion worldPos = { position.x, position.y, position.z, 1.0f };
    
    // Transform world position to view
    QuaternionTransform(&worldPos, matView);
    
    // Transform result to projection (clip space position)
    QuaternionTransform(&worldPos, matProj);
    
    // Calculate normalized device coordinates (inverted y)
    Vector3 ndcPos = { worldPos.x / worldPos.w, -worldPos.y / worldPos.w, worldPos.z / worldPos.z };
    
    // Calculate 2d screen position vector
    Vector2 screenPosition = { (ndcPos.x + 1.0f)/2.0f*(float)GetScreenWidth(), (ndcPos.y + 1.0f)/2.0f*(float)GetScreenHeight() };
    
    return screenPosition;
}

// Get transform matrix for camera
Matrix GetCameraMatrix(Camera camera)
{
    return MatrixLookAt(camera.position, camera.target, camera.up);
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Input (Keyboard, Mouse, Gamepad) Functions
//----------------------------------------------------------------------------------
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI) || defined(PLATFORM_WEB)
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

// Detect if a mouse button has been pressed once
bool IsMouseButtonPressed(int button)
{
    bool pressed = false;

    if ((currentMouseState[button] != previousMouseState[button]) && (currentMouseState[button] == 1)) pressed = true;
    else pressed = false;

    return pressed;
}

// Detect if a mouse button is being pressed
bool IsMouseButtonDown(int button)
{
    if (GetMouseButtonStatus(button) == 1) return true;
    else return false;
}

// Detect if a mouse button has been released once
bool IsMouseButtonReleased(int button)
{
    bool released = false;

    if ((currentMouseState[button] != previousMouseState[button]) && (currentMouseState[button] == 0)) released = true;
    else released = false;

    return released;
}

// Detect if a mouse button is NOT being pressed
bool IsMouseButtonUp(int button)
{
    if (GetMouseButtonStatus(button) == 0) return true;
    else return false;
}

// Returns mouse position X
int GetMouseX(void)
{
    return (int)mousePosition.x;
}

// Returns mouse position Y
int GetMouseY(void)
{
    return (int)mousePosition.y;
}

// Returns mouse position XY
Vector2 GetMousePosition(void)
{
    return mousePosition;
}

// Set mouse position XY
void SetMousePosition(Vector2 position)
{
    mousePosition = position;
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    // NOTE: emscripten not implemented
    glfwSetCursorPos(window, position.x, position.y);
#endif
}

// Returns mouse wheel movement Y
int GetMouseWheelMove(void)
{
#if defined(PLATFORM_WEB)
    return previousMouseWheelY/100;
#else
    return previousMouseWheelY;
#endif
}

// Hide mouse cursor
void HideCursor()
{
#if defined(PLATFORM_DESKTOP)
    #ifdef __linux
        XColor Col;
        const char Nil[] = {0};

        Pixmap Pix = XCreateBitmapFromData(glfwGetX11Display(), glfwGetX11Window(window), Nil, 1, 1);
        Cursor Cur = XCreatePixmapCursor(glfwGetX11Display(), Pix, Pix, &Col, &Col, 0, 0);

        XDefineCursor(glfwGetX11Display(), glfwGetX11Window(window), Cur);
        XFreeCursor(glfwGetX11Display(), Cur);
    #else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    #endif
#endif
    cursorHidden = true;
}

// Show mouse cursor
void ShowCursor()
{
#if defined(PLATFORM_DESKTOP)
    #ifdef __linux
        XUndefineCursor(glfwGetX11Display(), glfwGetX11Window(window));
    #else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    #endif
#endif
    cursorHidden = false;
}

// Disable mouse cursor
void DisableCursor()
{
#if defined(PLATFORM_DESKTOP)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif
    cursorHidden = true;
}

// Enable mouse cursor
void EnableCursor()
{
#if defined(PLATFORM_DESKTOP)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
#endif
    cursorHidden = false;
}

// Check if mouse cursor is hidden
bool IsCursorHidden()
{
    return cursorHidden;
}

// NOTE: Gamepad support not implemented in emscripten GLFW3 (PLATFORM_WEB)

// Detect if a gamepad is available
bool IsGamepadAvailable(int gamepad)
{
    bool result = false;
    
#if defined(PLATFORM_RPI)
    if (gamepadReady && (gamepad == 0)) result = true;
#else
    if (glfwJoystickPresent(gamepad) == 1) result = true;
#endif

    return result;
}

// Return axis movement vector for a gamepad
Vector2 GetGamepadMovement(int gamepad)
{
    Vector2 vec = { 0, 0 };

    const float *axes;
    int axisCount = 0;
    
#if defined(PLATFORM_RPI)
    // TODO: Get gamepad axis information
#else
    axes = glfwGetJoystickAxes(gamepad, &axisCount);
#endif
    
    if (axisCount >= 2)
    {
        vec.x = axes[0];    // Left joystick X
        vec.y = axes[1];    // Left joystick Y

        //vec.x = axes[2];    // Right joystick X
        //vec.x = axes[3];    // Right joystick Y
    }

    return vec;
}

// Detect if a gamepad button has been pressed once
bool IsGamepadButtonPressed(int gamepad, int button)
{
    bool pressed = false;

    currentGamepadState[button] = IsGamepadButtonDown(gamepad, button);

    if (currentGamepadState[button] != previousGamepadState[button])
    {
        if (currentGamepadState[button]) pressed = true;
        previousGamepadState[button] = currentGamepadState[button];
    }
    else pressed = false;

    return pressed;
}

// Detect if a gamepad button is being pressed
bool IsGamepadButtonDown(int gamepad, int button)
{
    bool result = false;
    const unsigned char *buttons;
    int buttonsCount;
    
#if defined(PLATFORM_RPI)
    // TODO: Get gamepad buttons information
#else
    buttons = glfwGetJoystickButtons(gamepad, &buttonsCount);

    if ((buttons != NULL) && (buttons[button] == GLFW_PRESS)) result = true;
    else result = false;
#endif
    
    return result;
}

// Detect if a gamepad button has NOT been pressed once
bool IsGamepadButtonReleased(int gamepad, int button)
{
    bool released = false;

    currentGamepadState[button] = IsGamepadButtonUp(gamepad, button);

    if (currentGamepadState[button] != previousGamepadState[button])
    {
        if (currentGamepadState[button]) released = true;
        previousGamepadState[button] = currentGamepadState[button];
    }
    else released = false;

    return released;
}

// Detect if a mouse button is NOT being pressed
bool IsGamepadButtonUp(int gamepad, int button)
{
    bool result = false;
    const unsigned char *buttons;
    int buttonsCount;

#if defined(PLATFORM_RPI)
    // TODO: Get gamepad buttons information
#else
    buttons = glfwGetJoystickButtons(gamepad, &buttonsCount);

    if ((buttons != NULL) && (buttons[button] == GLFW_RELEASE)) result = true;
    else result = false;
#endif

    return result;
}
#endif  //defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI) || defined(PLATFORM_WEB)

// Returns touch position X
int GetTouchX(void)
{
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
    return (int)touchPosition[0].x;
#else   // PLATFORM_DESKTOP, PLATFORM_RPI
    return GetMouseX();
#endif
}

// Returns touch position Y
int GetTouchY(void)
{
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
    return (int)touchPosition[0].y;
#else   // PLATFORM_DESKTOP, PLATFORM_RPI
    return GetMouseY();
#endif
}

// Returns touch position XY
// TODO: Touch position should be scaled depending on display size and render size
Vector2 GetTouchPosition(int index)
{
    Vector2 position = { -1.0f, -1.0f };
    
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
    if (index < MAX_TOUCH_POINTS) position = touchPosition[index];
    else TraceLog(WARNING, "Required touch point out of range (Max touch points: %i)", MAX_TOUCH_POINTS);

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
#else   // PLATFORM_DESKTOP, PLATFORM_RPI
    if (index == 0) position = GetMousePosition();
#endif

    return position;
}

#if defined(PLATFORM_ANDROID)
// Detect if a button has been pressed once
bool IsButtonPressed(int button)
{
    bool pressed = false;

    if ((currentButtonState[button] != previousButtonState[button]) && (currentButtonState[button] == 0)) pressed = true;
    else pressed = false;

    return pressed;
}

// Detect if a button is being pressed (button held down)
bool IsButtonDown(int button)
{
    if (currentButtonState[button] == 0) return true;
    else return false;
}

// Detect if a button has been released once
bool IsButtonReleased(int button)
{
    bool released = false;

    if ((currentButtonState[button] != previousButtonState[button]) && (currentButtonState[button] == 1)) released = true;
    else released = false;

    return released;
}
#endif

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Initialize display device and framebuffer
// NOTE: width and height represent the screen (framebuffer) desired size, not actual display size
// If width or height are 0, default display size will be used for framebuffer size
static void InitDisplay(int width, int height)
{
    screenWidth = width;        // User desired width
    screenHeight = height;      // User desired height

    // NOTE: Framebuffer (render area - renderWidth, renderHeight) could include black bars...
    // ...in top-down or left-right to match display aspect ratio (no weird scalings)

    // Downscale matrix is required in case desired screen area is bigger than display area
    downscaleView = MatrixIdentity();

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    glfwSetErrorCallback(ErrorCallback);

    if (!glfwInit()) TraceLog(ERROR, "Failed to initialize GLFW");

    // NOTE: Getting video modes is not implemented in emscripten GLFW3 version
#if defined(PLATFORM_DESKTOP)
    // Find monitor resolution
    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    displayWidth = mode->width;
    displayHeight = mode->height;

    // Screen size security check
    if (screenWidth <= 0) screenWidth = displayWidth;
    if (screenHeight <= 0) screenHeight = displayHeight;
#elif defined(PLATFORM_WEB)
    displayWidth = screenWidth;
    displayHeight = screenHeight;
#endif

    glfwDefaultWindowHints();                     // Set default windows hints

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);     // Avoid window being resizable
    //glfwWindowHint(GLFW_DECORATED, GL_TRUE);    // Border and buttons on Window
    //glfwWindowHint(GLFW_RED_BITS, 8);           // Framebuffer red color component bits
    //glfwWindowHint(GLFW_DEPTH_BITS, 16);        // Depthbuffer bits (24 by default)
    //glfwWindowHint(GLFW_REFRESH_RATE, 0);       // Refresh rate for fullscreen window
    //glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);    // Default OpenGL API to use. Alternative: GLFW_OPENGL_ES_API
    //glfwWindowHint(GLFW_AUX_BUFFERS, 0);        // Number of auxiliar buffers

    // NOTE: When asking for an OpenGL context version, most drivers provide highest supported version
    // with forward compatibility to older OpenGL versions.
    // For example, if using OpenGL 1.1, driver can provide a 3.3 context fordward compatible.

    // Check selection OpenGL version (not initialized yet!)
    if (rlGetVersion() == OPENGL_33)
    {
        if (configFlags & FLAG_MSAA_4X_HINT)
        {
            glfwWindowHint(GLFW_SAMPLES, 4);       // Enables multisampling x4 (MSAA), default is 0
            TraceLog(INFO, "Trying to enable MSAA x4");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);        // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);        // Choose OpenGL minor version (just hint)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Profiles Hint: Only 3.3 and above!
                                                                       // Other values: GLFW_OPENGL_ANY_PROFILE, GLFW_OPENGL_COMPAT_PROFILE
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE); // Fordward Compatibility Hint: Only 3.3 and above!
    }

    if (fullscreen)
    {
        // At this point we need to manage render size vs screen size
        // NOTE: This function uses and modifies global module variables: 
        //       screenWidth/screenHeight - renderWidth/renderHeight - downscaleView
        SetupFramebufferSize(displayWidth, displayHeight);
        
        // TODO: SetupFramebufferSize() does not consider properly display video modes.
        // It setups a renderWidth/renderHeight with black bars that could not match a valid video mode,
        // and so, framebuffer is not scaled properly to some monitors.
        
        int count; 
        const GLFWvidmode *modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);
        
        for (int i = 0; i < count; i++)
        {
            // TODO: Check modes[i]->width;
            // TODO: Check modes[i]->height;
        }

        window = glfwCreateWindow(screenWidth, screenHeight, windowTitle, glfwGetPrimaryMonitor(), NULL);
    }
    else
    {
        // No-fullscreen window creation
        window = glfwCreateWindow(screenWidth, screenHeight, windowTitle, NULL, NULL);
        
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

    if (!window)
    {
        glfwTerminate();
        TraceLog(ERROR, "GLFW Failed to initialize Window");
    }
    else
    {
        TraceLog(INFO, "Display device initialized successfully");
#if defined(PLATFORM_DESKTOP)
        TraceLog(INFO, "Display size: %i x %i", displayWidth, displayHeight);
#endif
        TraceLog(INFO, "Render size: %i x %i", renderWidth, renderHeight);
        TraceLog(INFO, "Screen size: %i x %i", screenWidth, screenHeight);
        TraceLog(INFO, "Viewport offsets: %i, %i", renderOffsetX, renderOffsetY);
    }

    glfwSetWindowSizeCallback(window, WindowSizeCallback);
    glfwSetCursorEnterCallback(window, CursorEnterCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, MouseCursorPosCallback);    // Track mouse position changes
    glfwSetCharCallback(window, CharCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetWindowIconifyCallback(window, WindowIconifyCallback);
#if defined(PLATFORM_DESKTOP)
    glfwSetDropCallback(window, WindowDropCallback);
#endif

    glfwMakeContextCurrent(window);

#if defined(PLATFORM_DESKTOP)
    // Extensions initialization for OpenGL 3.3
    if (rlGetVersion() == OPENGL_33)
    {
        #if defined(GLEW_EXTENSIONS_LOADER)
            // Initialize extensions using GLEW
            glewExperimental = 1;       // Needed for core profile
            GLenum error = glewInit();
            
            if (error != GLEW_OK) TraceLog(ERROR, "Failed to initialize GLEW - Error Code: %s\n", glewGetErrorString(error));
            
            if (glewIsSupported("GL_VERSION_3_3")) TraceLog(INFO, "OpenGL 3.3 Core profile supported");
            else TraceLog(ERROR, "OpenGL 3.3 Core profile not supported");

            // With GLEW, we can check if an extension has been loaded in two ways:
            //if (GLEW_ARB_vertex_array_object) { } 
            //if (glewIsSupported("GL_ARB_vertex_array_object")) { }

            // NOTE: GLEW is a big library that loads ALL extensions, we can use some alternative to load only required ones
            // Alternatives: glLoadGen, glad, libepoxy
        #elif defined(GLAD_EXTENSIONS_LOADER)
            // NOTE: glad is generated and contains only required OpenGL version and Core extensions
            //if (!gladLoadGL()) TraceLog(ERROR, "Failed to initialize glad\n");
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) TraceLog(ERROR, "Failed to initialize glad\n"); // No GLFW3 in this module...

            if (GLAD_GL_VERSION_3_3) TraceLog(INFO, "OpenGL 3.3 Core profile supported");
            else TraceLog(ERROR, "OpenGL 3.3 Core profile not supported");
            
            // With GLAD, we can check if an extension is supported using the GLAD_GL_xxx booleans
            //if (GLAD_GL_ARB_vertex_array_object) // Use GL_ARB_vertex_array_object
        #endif
    }
#endif
    
    // Enables GPU v-sync, so frames are not limited to screen refresh rate (60Hz -> 60 FPS)
    // If not set, swap interval uses GPU v-sync configuration
    // Framerate can be setup using SetTargetFPS()
    if (configFlags & FLAG_VSYNC_HINT)
    {
        glfwSwapInterval(1);
        TraceLog(INFO, "Trying to enable VSYNC");
    }

    //glfwGetFramebufferSize(window, &renderWidth, &renderHeight);    // Get framebuffer size of current window

#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
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
        TraceLog(INFO, "Trying to enable MSAA x4");
    }
    
    const EGLint framebufferAttribs[] =
    {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,    // Type of context support -> Required on RPI?
        //EGL_SURFACE_TYPE, EGL_WINDOW_BIT,         // Don't use it on Android!
        EGL_RED_SIZE, 8,            // RED color bit depth (alternative: 5)
        EGL_GREEN_SIZE, 8,          // GREEN color bit depth (alternative: 6)
        EGL_BLUE_SIZE, 8,           // BLUE color bit depth (alternative: 5)
        //EGL_ALPHA_SIZE, 8,        // ALPHA bit depth
        EGL_DEPTH_SIZE, 16,         // Depth buffer size (Required to use Depth testing!)
        //EGL_STENCIL_SIZE, 8,      // Stencil buffer size
        EGL_SAMPLE_BUFFERS, sampleBuffer,    // Activate MSAA
        EGL_SAMPLES, samples,       // 4x Antialiasing if activated (Free on MALI GPUs)
        EGL_NONE
    };

    EGLint contextAttribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    EGLint numConfigs;

    // Get an EGL display connection
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    // Initialize the EGL display connection
    eglInitialize(display, NULL, NULL);

    // Get an appropriate EGL framebuffer configuration
    eglChooseConfig(display, framebufferAttribs, &config, 1, &numConfigs);

    // Set rendering API
    eglBindAPI(EGL_OPENGL_ES_API);

    // Create an EGL rendering context
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);

    // Create an EGL window surface
    //---------------------------------------------------------------------------------
#if defined(PLATFORM_ANDROID)
    EGLint displayFormat;

    displayWidth = ANativeWindow_getWidth(app->window);
    displayHeight = ANativeWindow_getHeight(app->window);

    // EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is guaranteed to be accepted by ANativeWindow_setBuffersGeometry()
    // As soon as we picked a EGLConfig, we can safely reconfigure the ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &displayFormat);

    // At this point we need to manage render size vs screen size
    // NOTE: This function use and modify global module variables: screenWidth/screenHeight and renderWidth/renderHeight and downscaleView
    SetupFramebufferSize(displayWidth, displayHeight);

    ANativeWindow_setBuffersGeometry(app->window, renderWidth, renderHeight, displayFormat);
    //ANativeWindow_setBuffersGeometry(app->window, 0, 0, displayFormat);       // Force use of native display size

    surface = eglCreateWindowSurface(display, config, app->window, NULL);

#elif defined(PLATFORM_RPI)
    graphics_get_display_size(0, &displayWidth, &displayHeight);

    // At this point we need to manage render size vs screen size
    // NOTE: This function use and modify global module variables: screenWidth/screenHeight and renderWidth/renderHeight and downscaleView
    SetupFramebufferSize(displayWidth, displayHeight);

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
    alpha.opacity = 255;
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
#endif
    // There must be at least one frame displayed before the buffers are swapped
    //eglSwapInterval(display, 1);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
    {
        TraceLog(ERROR, "Unable to attach EGL rendering context to EGL surface");
    }
    else
    {
        // Grab the width and height of the surface
        //eglQuerySurface(display, surface, EGL_WIDTH, &renderWidth);
        //eglQuerySurface(display, surface, EGL_HEIGHT, &renderHeight);

        TraceLog(INFO, "Display device initialized successfully");
        TraceLog(INFO, "Display size: %i x %i", displayWidth, displayHeight);
        TraceLog(INFO, "Render size: %i x %i", renderWidth, renderHeight);
        TraceLog(INFO, "Screen size: %i x %i", screenWidth, screenHeight);
        TraceLog(INFO, "Viewport offsets: %i, %i", renderOffsetX, renderOffsetY);
    }
#endif
}

// Initialize OpenGL graphics
static void InitGraphics(void)
{
    rlglInit();                     // Init rlgl

    rlglInitGraphics(renderOffsetX, renderOffsetY, renderWidth, renderHeight);  // Init graphics (OpenGL stuff)

    ClearBackground(RAYWHITE);      // Default background color for raylib games :P

#if defined(PLATFORM_ANDROID)
    windowReady = true;     // IMPORTANT!
#endif
}

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
// GLFW3 Error Callback, runs on GLFW3 error
static void ErrorCallback(int error, const char *description)
{
    TraceLog(WARNING, "[GLFW3 Error] Code: %i Decription: %s", error, description);
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
        glfwSetWindowShouldClose(window, GL_TRUE);

        // NOTE: Before closing window, while loop must be left!
    }
#if defined(PLATFORM_DESKTOP)
    else if (key == GLFW_KEY_F12 && action == GLFW_PRESS)
    {
        TakeScreenshot();
    }
#endif
    else currentKeyState[key] = action;

    // TODO: Review (and remove) this HACK for GuiTextBox, to deteck back key
    if ((key == 259) && (action == GLFW_PRESS)) lastKeyPressed = 3;
}

// GLFW3 Mouse Button Callback, runs on mouse button pressed
static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    currentMouseState[button] = action;
    
#define ENABLE_MOUSE_GESTURES
#if defined(ENABLE_MOUSE_GESTURES)
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
#define ENABLE_MOUSE_GESTURES
#if defined(ENABLE_MOUSE_GESTURES)
    // Process mouse events as touches to be able to use mouse-gestures
    GestureEvent gestureEvent;

    gestureEvent.touchAction = TOUCH_MOVE;

    // Register touch points count
    gestureEvent.pointCount = 1;
    
    // Register touch points position, only one point registered
    gestureEvent.position[0] = (Vector2){ (float)x, (float)y };
    
    // Normalize gestureEvent.position[0] for screenWidth and screenHeight
    gestureEvent.position[0].x /= (float)GetScreenWidth(); 
    gestureEvent.position[0].y /= (float)GetScreenHeight();

    // Gesture data is sent to gestures system for processing
    ProcessGestureEvent(gestureEvent);
#endif
}

// GLFW3 Char Key Callback, runs on key pressed (get char value)
static void CharCallback(GLFWwindow *window, unsigned int key)
{
    lastKeyPressed = key;

    //TraceLog(INFO, "Char Callback Key pressed: %i\n", key);
}

// GLFW3 CursorEnter Callback, when cursor enters the window
static void CursorEnterCallback(GLFWwindow *window, int enter)
{
    if (enter == true) cursorOnScreen = true;
    else cursorOnScreen = false;
}

// GLFW3 WindowSize Callback, runs when window is resized
static void WindowSizeCallback(GLFWwindow *window, int width, int height)
{
    // If window is resized, graphics device is re-initialized (but only ortho mode)
    rlglInitGraphics(renderOffsetX, renderOffsetY, renderWidth, renderHeight);

    // Window size must be updated to be used on 3D mode to get new aspect ratio (Begin3dMode())
    //screenWidth = width;
    //screenHeight = height;

    // TODO: Update render size?

    // Background must be also re-cleared
    ClearBackground(RAYWHITE);
}

// GLFW3 WindowIconify Callback, runs when window is minimized/restored
static void WindowIconifyCallback(GLFWwindow* window, int iconified)
{
    if (iconified) windowMinimized = true;  // The window was iconified
    else windowMinimized = false;           // The window was restored
}
#endif

#if defined(PLATFORM_DESKTOP)
// GLFW3 Window Drop Callback, runs when drop files into window
// NOTE: Paths are stored in dinamic memory for further retrieval
// Everytime new files are dropped, old ones are discarded
static void WindowDropCallback(GLFWwindow *window, int count, const char **paths)
{
    ClearDroppedFiles();
    
    dropFilesPath = (char **)malloc(sizeof(char *)*count);
    
    for (int i = 0; i < count; i++)
    {
        dropFilesPath[i] = (char *)malloc(sizeof(char)*256);     // Max path length set to 256 char
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
            TraceLog(INFO, "APP_CMD_START");
        } break;
        case APP_CMD_RESUME:
        {
            TraceLog(INFO, "APP_CMD_RESUME");
        } break;
        case APP_CMD_INIT_WINDOW:
        {
            TraceLog(INFO, "APP_CMD_INIT_WINDOW");

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
                    // Init device display (monitor, LCD, ...)
                    InitDisplay(screenWidth, screenHeight);

                    // Init OpenGL graphics
                    InitGraphics();

                    // Load default font for convenience
                    // NOTE: External function (defined in module: text)
                    LoadDefaultFont();
                    
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

                    // Init hi-res timer
                    InitTimer();

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
            TraceLog(INFO, "APP_CMD_GAINED_FOCUS");
            appEnabled = true;
            //ResumeMusicStream();
        } break;
        case APP_CMD_PAUSE:
        {
            TraceLog(INFO, "APP_CMD_PAUSE");
        } break;
        case APP_CMD_LOST_FOCUS:
        {
            //DrawFrame();
            TraceLog(INFO, "APP_CMD_LOST_FOCUS");
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

            TraceLog(INFO, "APP_CMD_TERM_WINDOW");
        } break;
        case APP_CMD_SAVE_STATE:
        {
            TraceLog(INFO, "APP_CMD_SAVE_STATE");
        } break;
        case APP_CMD_STOP:
        {
            TraceLog(INFO, "APP_CMD_STOP");
        } break;
        case APP_CMD_DESTROY:
        {
            // TODO: Finish activity?
            //ANativeActivity_finish(app->activity);

            TraceLog(INFO, "APP_CMD_DESTROY");
        } break;
        case APP_CMD_CONFIG_CHANGED:
        {
            //AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
            //print_cur_config(app);

            // Check screen orientation here!

            TraceLog(INFO, "APP_CMD_CONFIG_CHANGED");
        } break;
        default: break;
    }
}

// Android: Get input events
static int32_t AndroidInputCallback(struct android_app *app, AInputEvent *event)
{
    //http://developer.android.com/ndk/reference/index.html
    
    int type = AInputEvent_getType(event);

    if (type == AINPUT_EVENT_TYPE_MOTION)
    {
        // Get first touch position
        touchPosition[0].x = AMotionEvent_getX(event, 0);
        touchPosition[0].y = AMotionEvent_getY(event, 0);
        
        // Get second touch position
        touchPosition[1].x = AMotionEvent_getX(event, 1);
        touchPosition[1].y = AMotionEvent_getY(event, 1);
    }
    else if (type == AINPUT_EVENT_TYPE_KEY)
    {
        int32_t keycode = AKeyEvent_getKeyCode(event);
        //int32_t AKeyEvent_getMetaState(event);
        
        // Save current button and its state
        currentButtonState[keycode] = AKeyEvent_getAction(event);  // Down = 0, Up = 1

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
    
    GestureEvent gestureEvent;
    
    // Register touch actions
    if (flags == AMOTION_EVENT_ACTION_DOWN) gestureEvent.touchAction = TOUCH_DOWN;
    else if (flags == AMOTION_EVENT_ACTION_UP) gestureEvent.touchAction = TOUCH_UP;
    else if (flags == AMOTION_EVENT_ACTION_MOVE) gestureEvent.touchAction = TOUCH_MOVE;
    
    // Register touch points count
    gestureEvent.pointCount = AMotionEvent_getPointerCount(event);
    
    // Register touch points id
    gestureEvent.pointerId[0] = AMotionEvent_getPointerId(event, 0);
    gestureEvent.pointerId[1] = AMotionEvent_getPointerId(event, 1);
    
    // Register touch points position
    // NOTE: Only two points registered
    gestureEvent.position[0] = (Vector2){ AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0) };
    gestureEvent.position[1] = (Vector2){ AMotionEvent_getX(event, 1), AMotionEvent_getY(event, 1) };
    
    // Normalize gestureEvent.position[x] for screenWidth and screenHeight
    gestureEvent.position[0].x /= (float)GetScreenWidth(); 
    gestureEvent.position[0].y /= (float)GetScreenHeight();
    
    gestureEvent.position[1].x /= (float)GetScreenWidth(); 
    gestureEvent.position[1].y /= (float)GetScreenHeight();
    
    // Gesture data is sent to gestures system for processing
    ProcessGestureEvent(gestureEvent);

    return 0;   // return 1;
}
#endif

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI)
// Takes a screenshot and saves it in the same folder as executable
static void TakeScreenshot(void)
{
    static int shotNum = 0;     // Screenshot number, increments every screenshot take during program execution
    char buffer[20];            // Buffer to store file name

    unsigned char *imgData = rlglReadScreenPixels(renderWidth, renderHeight);

    sprintf(buffer, "screenshot%03i.png", shotNum);
    
    // Save image as PNG
    WritePNG(buffer, imgData, renderWidth, renderHeight, 4);

    free(imgData);

    shotNum++;

    TraceLog(INFO, "[%s] Screenshot taken!", buffer);
}
#endif

// Initialize hi-resolution timer
static void InitTimer(void)
{
    srand(time(NULL));              // Initialize random seed

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
    struct timespec now;

    if (clock_gettime(CLOCK_MONOTONIC, &now) == 0)  // Success
    {
        baseTime = (uint64_t)now.tv_sec*1000000000LLU + (uint64_t)now.tv_nsec;
    }
    else TraceLog(WARNING, "No hi-resolution timer available");
#endif

    previousTime = GetTime();       // Get time as double
}

// Get current time measure (in seconds) since InitTimer()
static double GetTime(void)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    return glfwGetTime();
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t time = (uint64_t)ts.tv_sec*1000000000LLU + (uint64_t)ts.tv_nsec;

    return (double)(time - baseTime)*1e-9;
#endif
}

// Get one key state
static bool GetKeyStatus(int key)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    return glfwGetKey(window, key);
#elif defined(PLATFORM_ANDROID)
    // TODO: Check for virtual keyboard
    return false;
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
    // TODO: Check for virtual keyboard
    return false;
#elif defined(PLATFORM_RPI)
    // NOTE: Mouse buttons states are filled in PollInputEvents()
    return currentMouseState[button];
#endif
}

// Poll (store) all input events
static void PollInputEvents(void)
{
    // NOTE: Gestures update must be called every frame to reset gestures correctly
    // because ProcessGestureEvent() is just called on an event, not every frame
    UpdateGestures();
    
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    // Mouse input polling
    double mouseX;
    double mouseY;

    glfwGetCursorPos(window, &mouseX, &mouseY);

    mousePosition.x = (float)mouseX;
    mousePosition.y = (float)mouseY;

    // Keyboard input polling (automatically managed by GLFW3 through callback)
    lastKeyPressed = -1;

    // Register previous keys states
    for (int i = 0; i < 512; i++) previousKeyState[i] = currentKeyState[i];

    // Register previous mouse states
    for (int i = 0; i < 3; i++) previousMouseState[i] = currentMouseState[i];

    previousMouseWheelY = currentMouseWheelY;
    currentMouseWheelY = 0;

    glfwPollEvents();       // Register keyboard/mouse events... and window events!
#elif defined(PLATFORM_ANDROID)

    // Register previous keys states
    for (int i = 0; i < 128; i++) previousButtonState[i] = currentButtonState[i];

    // Poll Events (registered events)
    // NOTE: Activity is paused if not enabled (appEnabled)
    while ((ident = ALooper_pollAll(appEnabled ? 0 : -1, NULL, &events,(void**)&source)) >= 0)
    {
        // Process this event
        if (source != NULL) source->process(app, source);

        // NOTE: Never close window, native activity is controlled by the system!
        if (app->destroyRequested != 0)
        {
            //TraceLog(INFO, "Closing Window...");
            //windowShouldClose = true;
            //ANativeActivity_finish(app->activity);
        }
    }
#elif defined(PLATFORM_RPI)

    // NOTE: Mouse input events polling is done asynchonously in another pthread - MouseThread()

    // NOTE: Keyboard reading could be done using input_event(s) reading or just read from stdin,
    // we use method 2 (stdin) but maybe in a future we should change to method 1...
    ProcessKeyboard();

    // NOTE: Gamepad (Joystick) input events polling is done asynchonously in another pthread - GamepadThread()

#endif
}

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

    // NOTE: Reading directly from stdin will give chars already key-mapped by kernel to ASCII or UNICODE, we change that! -> WHY???

    // Save old keyboard mode to restore it at the end
    if (ioctl(STDIN_FILENO, KDGKBMODE, &defaultKeyboardMode) < 0)
    {
        // NOTE: It could mean we are using a remote keyboard through ssh!
        TraceLog(WARNING, "Could not change keyboard mode (SSH keyboard?)");

        keyboardMode = 2;   // ASCII
    }
    else
    {
        // We reconfigure keyboard mode to get:
        //    - scancodes (K_RAW) 
        //    - keycodes (K_MEDIUMRAW)
        //    - ASCII chars (K_XLATE)
        //    - UNICODE chars (K_UNICODE)
        ioctl(STDIN_FILENO, KDSKBMODE, K_XLATE);
        
        //http://lct.sourceforge.net/lct/x60.html

        keyboardMode = 2;   // keycodes
    }

    // Register keyboard restore when program finishes
    atexit(RestoreKeyboard);
}

static void ProcessKeyboard(void)
{
    // Keyboard input polling (fill keys[256] array with status)
    int numKeysBuffer = 0;      // Keys available on buffer
    char keysBuffer[32];        // Max keys to be read at a time

    // Reset pressed keys array
    for (int i = 0; i < 512; i++) currentKeyState[i] = 0;

    // Read availables keycodes from stdin
    numKeysBuffer = read(STDIN_FILENO, keysBuffer, 32);     // POSIX system call

    // Fill array with pressed keys
    for (int i = 0; i < numKeysBuffer; i++)
    {
        //TraceLog(INFO, "Bytes on keysBuffer: %i", numKeysBuffer);

        int key = keysBuffer[i];

        if (keyboardMode == 2)  // ASCII chars (K_XLATE mode)
        {
            // NOTE: If (key == 0x1b), depending on next key, it could be a special keymap code!
            // Up -> 1b 5b 41 / Left -> 1b 5b 44 / Right -> 1b 5b 43 / Down -> 1b 5b 42
            if (key == 0x1b)
            {
                if (keysBuffer[i+1] == 0x5b)    // Special function key
                {
                    switch (keysBuffer[i+2])
                    {
                        case 0x41: currentKeyState[265] = 1; break;
                        case 0x42: currentKeyState[264] = 1; break;
                        case 0x43: currentKeyState[262] = 1; break;
                        case 0x44: currentKeyState[263] = 1; break;
                        default: break;
                    }

                    i += 2;  // Jump to next key

                    // NOTE: Other special function keys (F1, F2...) are not contempled for this keyboardMode...
                    // ...or they are just not directly keymapped (CTRL, ALT, SHIFT)
                }
            }
            else if (key == 0x0a) currentKeyState[257] = 1;     // raylib KEY_ENTER (don't mix with <linux/input.h> KEY_*)
            else if (key == 0x7f) currentKeyState[259] = 1;
            else
            {
                TraceLog(DEBUG, "Pressed key (ASCII): 0x%02x", key);

                currentKeyState[key] = 1;
            }

            // Detect ESC to stop program
            if ((key == 0x1b) && (numKeysBuffer == 1)) windowShouldClose = true;
        }
        else if (keyboardMode == 1)     // keycodes (K_MEDIUMRAW mode)
        {
            TraceLog(DEBUG, "Pressed key (keycode): 0x%02x", key);
            
            // NOTE: Each key is 7-bits (high bit in the byte is 0 for down, 1 for up)
            
            // TODO: Review (or rewrite) this code... not clear... replace by events!

            int asciiKey = -1;

            // Convert keycode to some recognized key (ASCII or GLFW3 equivalent)
            if (key < 128) asciiKey = (int)UnixKeycodeToASCII[key];

            // Record equivalent key state
            if ((asciiKey >= 0) && (asciiKey < 512)) currentKeyState[asciiKey] = 1;

            // In case of letter, we also activate lower case version
            if ((asciiKey >= 65) && (asciiKey <=90)) currentKeyState[asciiKey + 32] = 1;

            // Detect KEY_ESC to stop program
            if (key == 0x01) windowShouldClose = true;
        }

        // Same functionality as GLFW3 KeyCallback()
        /*
        if (asciiKey == exitKey) windowShouldClose = true;
        else if (key == GLFW_KEY_F12 && action == GLFW_PRESS)
        {
            TakeScreenshot();
        }
        */
    }
}

// Restore default keyboard input
static void RestoreKeyboard(void)
{
    // Reset to default keyboard settings
    tcsetattr(STDIN_FILENO, TCSANOW, &defaultKeyboardSettings);
    
    // Reconfigure keyboard to default mode
    ioctl(STDIN_FILENO, KDSKBMODE, defaultKeyboardMode);
}

// Mouse initialization (including mouse thread)
static void InitMouse(void)
{
    // NOTE: We can use /dev/input/mice to read from all available mice
    if ((mouseStream = open(DEFAULT_MOUSE_DEV, O_RDONLY|O_NONBLOCK)) < 0)
    {
        TraceLog(WARNING, "Mouse device could not be opened, no mouse available");
    }
    else
    {
        mouseReady = true;

        int error = pthread_create(&mouseThreadId, NULL, &MouseThread, NULL);

        if (error != 0) TraceLog(WARNING, "Error creating mouse input event thread");
        else TraceLog(INFO, "Mouse device initialized successfully");
    }
}

// Mouse reading thread
// NOTE: We need a separate thread to avoid loosing mouse events,
// if too much time passes between reads, queue gets full and new events override older ones...
static void *MouseThread(void *arg)
{
    const unsigned char XSIGN = 1<<4, YSIGN = 1<<5;
    
    typedef struct { 
        char buttons;
        char dx, dy; 
    } MouseEvent;
    
    MouseEvent mouse;
    
    int mouseRelX = 0;
    int mouseRelY = 0;

    while(1)
    {
        if (read(mouseStream, &mouse, sizeof(MouseEvent)) == (int)sizeof(MouseEvent))
        {
            if ((mouse.buttons & 0x08) == 0) break;   // This bit should always be set
            
            // Check Left button pressed
            if ((mouse.buttons & 0x01) > 0) currentMouseState[0] = 1;
            else currentMouseState[0] = 0;

            // Check Right button pressed
            if ((mouse.buttons & 0x02) > 0) currentMouseState[1] = 1;
            else currentMouseState[1] = 0;
            
            // Check Middle button pressed
            if ((mouse.buttons & 0x04) > 0) currentMouseState[2] = 1;
            else currentMouseState[2] = 0;
            
            mouseRelX = (int)mouse.dx;
            mouseRelY = (int)mouse.dy;
            
            if ((mouse.buttons & XSIGN) > 0) mouseRelX = -1*(255 - mouseRelX);
            if ((mouse.buttons & YSIGN) > 0) mouseRelY = -1*(255 - mouseRelY);
            
            mousePosition.x += (float)mouseRelX;
            mousePosition.y -= (float)mouseRelY;
            
            if (mousePosition.x < 0) mousePosition.x = 0;
            if (mousePosition.y < 0) mousePosition.y = 0;
            
            if (mousePosition.x > screenWidth) mousePosition.x = screenWidth;
            if (mousePosition.y > screenHeight) mousePosition.y = screenHeight;
       }
       //else read(mouseStream, &mouse, 1);   // Try to sync up again
    }

    return NULL;
}

// Init gamepad system
static void InitGamepad(void)
{
    if ((gamepadStream = open(DEFAULT_GAMEPAD_DEV, O_RDONLY|O_NONBLOCK)) < 0) 
    {
        TraceLog(WARNING, "Gamepad device could not be opened, no gamepad available");
    }
    else
    {
        gamepadReady = true;

        int error = pthread_create(&gamepadThreadId, NULL, &GamepadThread, NULL);

        if (error != 0) TraceLog(WARNING, "Error creating gamepad input event thread");
        else  TraceLog(INFO, "Gamepad device initialized successfully");
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

    // These values are sensible on Logitech Dual Action Rumble and Xbox360 controller
    const int joystickAxisX = 0;
    const int joystickAxisY = 1;

    // Read gamepad event
	struct js_event gamepadEvent;
    int bytes;
    
    int buttons[11];
    int stickX;
    int stickY;
    
	while (1) 
    {
        if (read(gamepadStream, &gamepadEvent, sizeof(struct js_event)) == (int)sizeof(struct js_event))
        {
            gamepadEvent.type &= ~JS_EVENT_INIT;     // Ignore synthetic events
            
            // Process gamepad events by type
            if (gamepadEvent.type == JS_EVENT_BUTTON) 
            {
                if (gamepadEvent.number < 11) 
                {
                    switch (gamepadEvent.value) 
                    {
                        case 0:
                        case 1: buttons[gamepadEvent.number] = gamepadEvent.value; break;
                        default: break;
                    }
                }
                /*
                switch (gamepadEvent.number)
                {
                    case 0: // 1st Axis X
                    case 1: // 1st Axis Y
                    case 2: // 2st Axis X
                    case 3: // 2st Axis Y
                    case 4:
                    {
                        if (gamepadEvent.value == 1) // Button pressed, 0 release
                        {
                            
                        }

                    } break;
                    // Buttons is similar, variable for every joystick
                }
                */
            }
            else if (gamepadEvent.type == JS_EVENT_AXIS) 
            {
                if (gamepadEvent.number == joystickAxisX) stickX = gamepadEvent.value;
                if (gamepadEvent.number == joystickAxisY) stickY = gamepadEvent.value;
                /*
                switch (gamepadEvent.number)
                {
                    case 0: // 1st Axis X
                    case 1: // 1st Axis Y
                    case 2: // 2st Axis X
                    case 3: // 2st Axis Y
                    // Buttons is similar, variable for every joystick
                }
                */
            }
        }
        else read(gamepadStream, &gamepadEvent, 1);   // Try to sync up again
	}
}
#endif

// Copy back buffer to front buffers
static void SwapBuffers(void)
{
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    glfwSwapBuffers(window);
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
    eglSwapBuffers(display, surface);
#endif
}

// Compute framebuffer size relative to screen size and display size
// NOTE: Global variables renderWidth/renderHeight can be modified
static void SetupFramebufferSize(int displayWidth, int displayHeight)
{
    // TODO: SetupFramebufferSize() does not consider properly display video modes.
    // It setups a renderWidth/renderHeight with black bars that could not match a valid video mode,
    // and so, framebuffer is not scaled properly to some monitors.
    
    // Calculate renderWidth and renderHeight, we have the display size (input params) and the desired screen size (global var)
    if ((screenWidth > displayWidth) || (screenHeight > displayHeight))
    {
        TraceLog(WARNING, "DOWNSCALING: Required screen size (%ix%i) is bigger than display size (%ix%i)", screenWidth, screenHeight, displayWidth, displayHeight);

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

        TraceLog(WARNING, "Downscale matrix generated, content will be rendered at: %i x %i", renderWidth, renderHeight);
    }
    else if ((screenWidth < displayWidth) || (screenHeight < displayHeight))
    {
        // Required screen size is smaller than display size
        TraceLog(INFO, "UPSCALING: Required screen size: %i x %i -> Display size: %i x %i", screenWidth, screenHeight, displayWidth, displayHeight);

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

#if defined(PLATFORM_WEB)
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
        TraceLog(INFO, "Canvas scaled to fullscreen. ElementSize: (%ix%i), ScreenSize(%ix%i)", e->elementWidth, e->elementHeight, e->screenWidth, e->screenHeight);
    }
    else
    {
        TraceLog(INFO, "Canvas scaled to windowed. ElementSize: (%ix%i), ScreenSize(%ix%i)", e->elementWidth, e->elementHeight, e->screenWidth, e->screenHeight);
    }
    
    // TODO: Depending on scaling factor (screen vs element), calculate factor to scale mouse/touch input

    return 0;
}

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
    ProcessGestureEvent(gestureEvent);   // Process obtained gestures data

    return 1;
}
#endif

// Plays raylib logo appearing animation
static void LogoAnimation(void)
{
#ifndef PLATFORM_WEB
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

                DrawText(SubText("raylib", 0, lettersCount), screenWidth/2 - 44, screenHeight/2 + 48, 50, Fade(BLACK, alpha));
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }
#endif

    showLogo = false;  // Prevent for repeating when reloading window (Android)
}
