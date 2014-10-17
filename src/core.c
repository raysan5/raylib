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
*
*   On PLATFORM_DESKTOP, the external lib GLFW3 (www.glfw.com) is used to manage graphic
*   device, OpenGL context and input on multiple operating systems (Windows, Linux, OSX).
*
*   On PLATFORM_ANDROID, graphic device is managed by EGL and input system by Android activity.
*
*   On PLATFORM_RPI, graphic device is managed by EGL and input system is coded in raw mode.
*
*   Copyright (c) 2014 Ramon Santamaria (Ray San - raysan@raysanweb.com)
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
#include "raymath.h"        // Required for data type Matrix and Matrix functions
#include "utils.h"          // TraceLog() function
                            // NOTE: Includes Android fopen map, InitAssetManager()

#include <stdio.h>          // Standard input / output lib
#include <stdlib.h>         // Declares malloc() and free() for memory management, rand(), atexit()
#include <stdint.h>         // Required for typedef unsigned long long int uint64_t, used by hi-res timer
#include <time.h>           // Useful to initialize random seed - Android/RPI hi-res timer
#include <math.h>           // Math related functions, tan() used to set perspective
#include <string.h>         // String function definitions, memset()
#include <errno.h>          // Macros for reporting and retrieving error conditions through error codes

#if defined(PLATFORM_DESKTOP)
    #include <GLFW/glfw3.h>     // GLFW3 library: Windows, OpenGL context and Input management
    //#include <GL/gl.h>        // OpenGL functions (GLFW3 already includes gl.h)
    //#define GLFW_DLL          // Using GLFW DLL on Windows -> No, we use static version!
#endif

#if defined(PLATFORM_ANDROID)
    #include <jni.h>                        // Java native interface
    #include <android/sensor.h>             // Android sensors functions
    #include <android/window.h>             // Defines AWINDOW_FLAG_FULLSCREEN and others
    //#include <android_native_app_glue.h>    // Defines basic app state struct and manages activity

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

    #define DEFAULT_KEYBOARD_DEV    "/dev/input/event0"     // Not used, keyboard inputs are read raw from stdin
    #define DEFAULT_MOUSE_DEV       "/dev/input/event1"
    //#define DEFAULT_MOUSE_DEV     "/dev/input/mouse0"
    #define DEFAULT_GAMEPAD_DEV     "/dev/input/js0"
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
#if defined(PLATFORM_DESKTOP)
static GLFWwindow *window;                      // Native window (graphic device)
#elif defined(PLATFORM_ANDROID)
static struct android_app *app;                 // Android activity
static struct android_poll_source *source;      // Android events polling source
static int ident, events;
static bool windowReady = false;                // Used to detect display initialization

// Gestures detection variables
static float tapTouchX, tapTouchY;
static bool touchTap = false;
static int32_t touchId;
const int32_t DOUBLE_TAP_TIMEOUT = 300*1000000;
const int32_t DOUBLE_TAP_SLOP = 100;
const int32_t TAP_TIMEOUT = 180*1000000;
const int32_t TOUCH_SLOP = 8;

#elif defined(PLATFORM_RPI)
static EGL_DISPMANX_WINDOW_T nativeWindow;      // Native window (graphic device)

// Input variables (mouse/keyboard)
static int mouseStream = -1;                    // Mouse device file descriptor
static bool mouseReady = false;                 // Flag to know if mouse is ready
pthread_t mouseThreadId;                        // Mouse reading thread id

// NOTE: For keyboard we will use the standard input (but reconfigured...)
static int defaultKeyboardMode;                 // Used to store default keyboard mode
static struct termios defaultKeyboardSettings;  // Used to staore default keyboard settings

static int keyboardMode = 0;    // Keyboard mode: 1 (KEYCODES), 2 (ASCII)

// This array maps Unix keycodes to ASCII equivalent and to GLFW3 equivalent for special function keys (>256)
const short UnixKeycodeToASCII[128] = { 256, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 45, 61, 259, 9, 81, 87, 69, 82, 84, 89, 85, 73, 79, 80, 91, 93, 257, 341, 65, 83, 68,
                            70, 71, 72, 74, 75, 76, 59, 39, 96, 340, 92, 90, 88, 67, 86, 66, 78, 77, 44, 46, 47, 344, -1, 342, 32, -1, 290, 291, 292, 293, 294, 295, 296,
                            297, 298, 299, -1, -1, -1, -1, -1, 45, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 257, 345, 47, -1,
                            346, -1, -1, 265, -1, 263, 262, -1, 264, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

static int gamepadStream = -1;                  // Gamepad device file descriptor
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
static EGLDisplay display;          // Native display device (physical screen connection)
static EGLSurface surface;          // Surface to draw on, framebuffers (connected to context)
static EGLContext context;          // Graphic context, mode in which drawing can be done 

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

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI)
static const char *windowTitle;             // Window text title...
static char configFlags = 0;

static bool customCursor = false;           // Tracks if custom cursor has been set
static bool cursorOnScreen = true;          // Tracks if cursor is inside client area
static Texture2D cursor;                    // Cursor texture

static Vector2 mousePosition;

static char previousKeyState[512] = { 0 };  // Required to check if key pressed/released once
static char currentKeyState[512] = { 0 };   // Required to check if key pressed/released once

static char previousMouseState[3] = { 0 };  // Required to check if mouse btn pressed/released once
static char currentMouseState[3] = { 0 };   // Required to check if mouse btn pressed/released once

static char previousGamepadState[32] = {0}; // Required to check if gamepad btn pressed/released once
static char currentGamepadState[32] = {0};  // Required to check if gamepad btn pressed/released once

static int previousMouseWheelY = 0;         // Required to track mouse wheel variation
static int currentMouseWheelY = 0;          // Required to track mouse wheel variation

static int exitKey = KEY_ESCAPE;            // Default exit key (ESC)
#endif

#if defined(PLATFORM_ANDROID)
static float touchX;                        // Touch position X
static float touchY;                        // Touch position Y
#endif

static double currentTime, previousTime;    // Used to track timmings
static double updateTime, drawTime;         // Time measures for update and draw
static double frameTime;                    // Time measure for one frame
static double targetTime = 0.0;             // Desired time for one frame, if 0 not applied

static bool showLogo = false;

//----------------------------------------------------------------------------------
// Other Modules Functions Declaration (required by core)
//----------------------------------------------------------------------------------
extern void LoadDefaultFont(void);              // [Module: text] Loads default font on InitWindow()
extern void UnloadDefaultFont(void);            // [Module: text] Unloads default font from GPU memory

extern void UpdateMusicStream(void);            // [Module: audio] Updates buffers for music streaming

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
static void InitMouse(void);                            // Mouse initialization (including mouse thread)
static void *MouseThread(void *arg);                    // Mouse reading thread
static void InitKeyboard(void);                         // Init raw keyboard system (standard input reading)
static void RestoreKeyboard(void);                      // Restore keyboard system
static void InitGamepad(void);                          // Init raw gamepad input
#endif

#if defined(PLATFORM_DESKTOP)
static void ErrorCallback(int error, const char *description);                             // GLFW3 Error Callback, runs on GLFW3 error
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);  // GLFW3 Keyboard Callback, runs on key pressed
static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);            // GLFW3 Srolling Callback, runs on mouse wheel
static void CursorEnterCallback(GLFWwindow *window, int enter);                            // GLFW3 Cursor Enter Callback, cursor enters client area
static void WindowSizeCallback(GLFWwindow *window, int width, int height);                 // GLFW3 WindowSize Callback, runs when window is resized
static void TakeScreenshot(void);                                                          // Takes a screenshot and saves it in the same folder as executable
#endif

#if defined(PLATFORM_ANDROID)
static int32_t InputCallback(struct android_app *app, AInputEvent *event);   // Process Android activity input events
static void CommandCallback(struct android_app *app, int32_t cmd);           // Process Android activity lifecycle commands
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - Window and OpenGL Context Functions
//----------------------------------------------------------------------------------
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI)
// Initialize Window and Graphics Context (OpenGL)
void InitWindow(int width, int height, const char *title)
{
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
    mousePosition.x = screenWidth/2;
    mousePosition.y = screenHeight/2;

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

    // TODO: Review, it doesn't work...
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
    app->onAppCmd = CommandCallback;
    app->onInputEvent = InputCallback;

    InitAssetManager(app->activity->assetManager);

    TraceLog(INFO, "Android app initialized successfully");

    while (!windowReady)
    {
        // Wait for window to be initialized (display and context)
        // Process events loop
        while ((ident = ALooper_pollAll(0, NULL, &events,(void**)&source)) >= 0)
        {
            // Process this event
            if (source != NULL) source->process(app, source);

            // Check if we are exiting
            if (app->destroyRequested != 0) windowShouldClose = true;
        }
    }
}
#endif

// Close Window and Terminate Context
void CloseWindow(void)
{
    UnloadDefaultFont();

    rlglClose();                // De-init rlgl

#if defined(PLATFORM_DESKTOP)
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
#if defined(PLATFORM_DESKTOP)
    return (glfwWindowShouldClose(window));
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
    return windowShouldClose;
#endif
}

// Fullscreen toggle
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

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI)
// Set a custom cursor icon/image
void SetCustomCursor(const char *cursorImage)
{
    if (customCursor) UnloadTexture(cursor);

    cursor = LoadTexture(cursorImage);

#if defined(PLATFORM_DESKTOP)
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
    // TODO: Review "clearing area", full framebuffer vs render area
    rlClearColor(color.r, color.g, color.b, color.a);
}

// Setup drawing canvas to start drawing
void BeginDrawing(void)
{
    currentTime = GetTime();            // Number of elapsed seconds since InitTimer() was called
    updateTime = currentTime - previousTime;
    previousTime = currentTime;

    rlClearScreenBuffers();

    rlLoadIdentity();                   // Reset current matrix (MODELVIEW)

    rlMultMatrixf(GetMatrixVector(downscaleView));       // If downscale required, apply it here

//  rlTranslatef(0.375, 0.375, 0);      // HACK to have 2D pixel-perfect drawing on OpenGL 1.1
                                        // NOTE: Not required with OpenGL 3.3+
}

// End canvas drawing and Swap Buffers (Double Buffering)
void EndDrawing(void)
{
    rlglDraw();                     //  Draw Buffers (Only OpenGL 3+ and ES2)

    SwapBuffers();                  // Copy back buffer to front buffer
    PollInputEvents();              // Poll user events

    UpdateMusicStream();            // NOTE: Function checks if music is enabled

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
    rlglDraw();                         //  Draw Buffers (Only OpenGL 3+ and ES2)

    rlMatrixMode(RL_PROJECTION);        // Switch to projection matrix

    rlPushMatrix();                     // Save previous matrix, which contains the settings for the 2d ortho projection
    rlLoadIdentity();                   // Reset current matrix (PROJECTION)

    // Setup perspective projection
    float aspect = (GLfloat)screenWidth/(GLfloat)screenHeight;
    double top = 0.1f*tan(45.0f*PI / 360.0f);
    double right = top*aspect;

    rlFrustum(-right, right, -top, top, 0.1f, 1000.0f);

    rlMatrixMode(RL_MODELVIEW);         // Switch back to modelview matrix
    rlLoadIdentity();                   // Reset current matrix (MODELVIEW)

    // Setup Camera view
    Matrix matLookAt = MatrixLookAt(camera.position, camera.target, camera.up);
    rlMultMatrixf(GetMatrixVector(matLookAt));      // Multiply MODELVIEW matrix by view matrix (camera)
}

// Ends 3D mode and returns to default 2D orthographic mode
void End3dMode(void)
{
    rlglDraw();                         //  Draw Buffers (Only OpenGL 3+ and ES2)

    rlMatrixMode(RL_PROJECTION);        // Switch to projection matrix
    rlPopMatrix();                      // Restore previous matrix (PROJECTION) from matrix stack

    rlMatrixMode(RL_MODELVIEW);         // Get back to modelview matrix
    rlLoadIdentity();                   // Reset current matrix (MODELVIEW)

    //rlTranslatef(0.375, 0.375, 0);      // HACK to ensure pixel-perfect drawing on OpenGL (after exiting 3D mode)
}

// Set target FPS for the game
void SetTargetFPS(int fps)
{
    targetTime = 1 / (float)fps;

    TraceLog(INFO, "Target time per frame: %02.03f milliseconds", (float)targetTime*1000);
}

// Returns current FPS
float GetFPS(void)
{
    return (1/(float)frameTime);
}

// Returns time in seconds for one frame
float GetFrameTime(void)
{
    // As we are operating quite a lot with frameTime, it could be no stable
    // so we round it before before passing around to be used
    // NOTE: There are still problems with high framerates (>500fps)
    double roundedFrameTime =  round(frameTime*10000) / 10000;

    return (float)roundedFrameTime;    // Time in seconds to run a frame
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
    return ((color.a << 24) + (color.r << 16) + (color.g << 8) + color.b);
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

    return (Color){color.r, color.g, color.b, color.a*alpha};
}

// Enable some window configurations (SetWindowFlags()?)
// TODO: Review function name and usage
void SetupFlags(char flags)
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

//----------------------------------------------------------------------------------
// Module Functions Definition - Input (Keyboard, Mouse, Gamepad) Functions
//----------------------------------------------------------------------------------
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI)
// Detect if a key has been pressed once
bool IsKeyPressed(int key)
{
    bool pressed = false;

    currentKeyState[key] = IsKeyDown(key);

    if (currentKeyState[key] != previousKeyState[key])
    {
        if (currentKeyState[key]) pressed = true;
        previousKeyState[key] = currentKeyState[key];
    }
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

    currentKeyState[key] = IsKeyUp(key);

    if (currentKeyState[key] != previousKeyState[key])
    {
        if (currentKeyState[key]) released = true;
        previousKeyState[key] = currentKeyState[key];
    }
    else released = false;

    return released;
}

// Detect if a key is NOT being pressed (key not held down)
bool IsKeyUp(int key)
{
    if (GetKeyStatus(key) == 0) return true;
    else return false;
}

// Detect if a mouse button has been pressed once
bool IsMouseButtonPressed(int button)
{
    bool pressed = false;

    currentMouseState[button] = IsMouseButtonDown(button);

    if (currentMouseState[button] != previousMouseState[button])
    {
        if (currentMouseState[button]) pressed = true;
        previousMouseState[button] = currentMouseState[button];
    }
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

    currentMouseState[button] = IsMouseButtonUp(button);

    if (currentMouseState[button] != previousMouseState[button])
    {
        if (currentMouseState[button]) released = true;
        previousMouseState[button] = currentMouseState[button];
    }
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

// Returns mouse wheel movement Y
int GetMouseWheelMove(void)
{
    previousMouseWheelY = currentMouseWheelY;

    currentMouseWheelY = 0;

    return previousMouseWheelY;
}
#endif

// TODO: Enable gamepad usage on Rapsberr Pi
#if defined(PLATFORM_DESKTOP)
// Detect if a gamepad is available
bool IsGamepadAvailable(int gamepad)
{
    int result = glfwJoystickPresent(gamepad);

    if (result == 1) return true;
    else return false;
}

// Return axis movement vector for a gamepad
Vector2 GetGamepadMovement(int gamepad)
{
    Vector2 vec = { 0, 0 };

    const float *axes;
    int axisCount;

    axes = glfwGetJoystickAxes(gamepad, &axisCount);

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
    const unsigned char *buttons;
    int buttonsCount;

    buttons = glfwGetJoystickButtons(gamepad, &buttonsCount);

    if ((buttons != NULL) && (buttons[button] == GLFW_PRESS))
    {
        return true;
    }
    else return false;
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
    const unsigned char *buttons;
    int buttonsCount;

    buttons = glfwGetJoystickButtons(gamepad, &buttonsCount);

    if ((buttons != NULL) && (buttons[button] == GLFW_RELEASE))
    {
        return true;
    }
    else return false;
}
#endif

#if defined(PLATFORM_ANDROID)
bool IsScreenTouched(void)
{
    return touchTap;
}

// Returns touch position X
int GetTouchX(void)
{
    return (int)touchX;
}

// Returns touch position Y
int GetTouchY(void)
{
    return (int)touchY;
}

// Returns touch position XY
Vector2 GetTouchPosition(void)
{
    Vector2 position = { touchX, touchY };

    return position;
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

#if defined(PLATFORM_DESKTOP)
    glfwSetErrorCallback(ErrorCallback);

    if (!glfwInit()) TraceLog(ERROR, "Failed to initialize GLFW");

    // Find monitor resolution
    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    displayWidth = mode->width;
    displayHeight = mode->height;

    // Screen size security check
    if (screenWidth <= 0) screenWidth = displayWidth;
    if (screenHeight <= 0) screenHeight = displayHeight;

    glfwDefaultWindowHints();                     // Set default windows hints

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);     // Avoid window being resizable
    //glfwWindowHint(GLFW_DECORATED, GL_TRUE);    // Border and buttons on Window
    //glfwWindowHint(GLFW_RED_BITS, 8);           // Bit depths of color components for default framebuffer
    //glfwWindowHint(GLFW_REFRESH_RATE, 0);       // Refresh rate for fullscreen window
    //glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);    // Default OpenGL API to use. Alternative: GLFW_OPENGL_ES_API
    //glfwWindowHint(GLFW_AUX_BUFFERS, 0);        // Number of auxiliar buffers

    // NOTE: When asking for an OpenGL context version, most drivers provide highest supported version 
    // with forward compatibility to older OpenGL versions.
    // For example, if using OpenGL 1.1, driver can provide a 3.3 context fordward compatible.

    if (rlGetVersion() == OPENGL_33)
    {
        //glfwWindowHint(GLFW_SAMPLES, 4);                    // Enables multisampling x4 (MSAA), default is 0
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);        // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);        // Choose OpenGL minor version (just hint)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Profiles Hint: Only 3.2 and above!
                                                                       // Other values: GLFW_OPENGL_ANY_PROFILE, GLFW_OPENGL_COMPAT_PROFILE
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE); // Fordward Compatibility Hint: Only 3.0 and above!
    }

    if (fullscreen)
    {
        // At this point we need to manage render size vs screen size
        // NOTE: This function use and modify global module variables: screenWidth/screenHeight and renderWidth/renderHeight and downscaleView
        SetupFramebufferSize(displayWidth, displayHeight);

        window = glfwCreateWindow(renderWidth, renderHeight, windowTitle, glfwGetPrimaryMonitor(), NULL);
    }
    else
    {
        // No-fullscreen window creation
        window = glfwCreateWindow(screenWidth, screenHeight, windowTitle, NULL, NULL);

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
        TraceLog(INFO, "Display size: %i x %i", displayWidth, displayHeight);
        TraceLog(INFO, "Render size: %i x %i", renderWidth, renderHeight);
        TraceLog(INFO, "Screen size: %i x %i", screenWidth, screenHeight);
        TraceLog(INFO, "Viewport offsets: %i, %i", renderOffsetX, renderOffsetY);
    }

    glfwSetWindowSizeCallback(window, WindowSizeCallback);
    glfwSetCursorEnterCallback(window, CursorEnterCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetScrollCallback(window, ScrollCallback);

    glfwMakeContextCurrent(window);

    //glfwSwapInterval(0);          // Disables GPU v-sync (if set), so frames are not limited to screen refresh rate (60Hz -> 60 FPS)
                                    // If not set, swap interval uses GPU v-sync configuration
                                    // Framerate can be setup using SetTargetFPS()

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

    const EGLint framebufferAttribs[] =
    {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,    // Type of context support -> Required on RPI?
        //EGL_SURFACE_TYPE, EGL_WINDOW_BIT,         // Don't use it on Android!
        EGL_RED_SIZE, 8,            // RED color bit depth (alternative: 5)
        EGL_GREEN_SIZE, 8,          // GREEN color bit depth (alternative: 6)
        EGL_BLUE_SIZE, 8,           // BLUE color bit depth (alternative: 5)
        //EGL_ALPHA_SIZE, 8,        // ALPHA bit depth
        EGL_DEPTH_SIZE, 8,          // Depth buffer size (Required to use Depth testing!)
        //EGL_STENCIL_SIZE, 8,      // Stencil buffer size
        //EGL_SAMPLE_BUFFERS, 1,    // Activate MSAA
        //EGL_SAMPLES, 4,           // 4x Antialiasing (Free on MALI GPUs)
        EGL_NONE
    };

    EGLint contextAttribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    EGLint numConfigs;
    EGLConfig config;

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
void InitGraphics(void)
{
    rlglInit();                     // Init rlgl

    rlglInitGraphics(renderOffsetX, renderOffsetY, renderWidth, renderHeight);  // Init graphics (OpenGL stuff)

    ClearBackground(RAYWHITE);      // Default background color for raylib games :P

#if defined(PLATFORM_ANDROID)
    windowReady = true;     // IMPORTANT!
#endif
}

#if defined(PLATFORM_DESKTOP)
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
    else if (key == GLFW_KEY_F12 && action == GLFW_PRESS)
    {
        TakeScreenshot();
    }
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
#endif

#if defined(PLATFORM_ANDROID)
// Android: Process activity input events
static int32_t InputCallback(struct android_app *app, AInputEvent *event)
{
    int type = AInputEvent_getType(event);
    //int32_t key = 0;

    if (type == AINPUT_EVENT_TYPE_MOTION)
    {
        if ((screenWidth > displayWidth) || (screenHeight > displayHeight))
        {
            // TODO: Seems to work ok but... review!
            touchX = AMotionEvent_getX(event, 0) * ((float)screenWidth / (float)(displayWidth - renderOffsetX)) - renderOffsetX/2;
            touchY = AMotionEvent_getY(event, 0) * ((float)screenHeight / (float)(displayHeight - renderOffsetY)) - renderOffsetY/2;
        }
        else
        {
            touchX = AMotionEvent_getX(event, 0) * ((float)renderWidth / (float)displayWidth) - renderOffsetX/2;
            touchY = AMotionEvent_getY(event, 0) * ((float)renderHeight / (float)displayHeight) - renderOffsetY/2;
        }

        // Detect TAP event
/*
        if (AMotionEvent_getPointerCount(event) > 1 )
        {
            // Only support single touch
            return false;
        }
*/
        int32_t action = AMotionEvent_getAction(event);
        unsigned int flags = action & AMOTION_EVENT_ACTION_MASK;

        switch (flags)
        {
            case AMOTION_EVENT_ACTION_DOWN:
            {
                touchId = AMotionEvent_getPointerId(event, 0);
                tapTouchX = AMotionEvent_getX(event, 0);
                tapTouchY = AMotionEvent_getY(event, 0);

            } break;
            case AMOTION_EVENT_ACTION_UP:
            {
                int64_t eventTime = AMotionEvent_getEventTime(event);
                int64_t downTime = AMotionEvent_getDownTime(event);

                if (eventTime - downTime <= TAP_TIMEOUT)
                {
                    if (touchId == AMotionEvent_getPointerId(event, 0))
                    {
                        float x = AMotionEvent_getX(event, 0) - tapTouchX;
                        float y = AMotionEvent_getY(event, 0) - tapTouchY;

                        float densityFactor = 1.0f;

                        if ( x*x + y*y < TOUCH_SLOP*TOUCH_SLOP * densityFactor)
                        {
                            // TAP Detected
                            touchTap = true;
                        }
                    }
                }
                break;
            }
        }

        //float AMotionEvent_getX(event, size_t pointer_index);
        //int32_t AMotionEvent_getButtonState(event); // Pressed buttons
        //int32_t AMotionEvent_getPointerId(event, size_t pointer_index);
        //size_t pointerCount =  AMotionEvent_getPointerCount(event);
        //float AMotionEvent_getPressure(const AInputEvent *motion_event, size_t pointer_index); // 0 to 1
        //float AMotionEvent_getSize(const AInputEvent *motion_event, size_t pointer_index); // Pressed area

        return 1;
    }
    else if (type == AINPUT_EVENT_TYPE_KEY)
    {
        //key = AKeyEvent_getKeyCode(event);
        //int32_t AKeyEvent_getMetaState(event);
    }

    return 0;
}

// Android: Process activity lifecycle commands
static void CommandCallback(struct android_app *app, int32_t cmd)
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
                // Init device display (monitor, LCD, ...)
                InitDisplay(screenWidth, screenHeight);

                // Init OpenGL graphics
                InitGraphics();

                // Load default font for convenience
                // NOTE: External function (defined in module: text)
                LoadDefaultFont();

                // Init hi-res timer
                InitTimer();

                // raylib logo appearing animation (if enabled)
                if (showLogo)
                {
                    SetTargetFPS(60);
                    LogoAnimation();
                }
            }
        } break;
        case APP_CMD_GAINED_FOCUS:
        {
            TraceLog(INFO, "APP_CMD_GAINED_FOCUS");
            ResumeMusicStream();
        } break;
        case APP_CMD_PAUSE:
        {
            TraceLog(INFO, "APP_CMD_PAUSE");
        } break;
        case APP_CMD_LOST_FOCUS:
        {
            //DrawFrame();
            TraceLog(INFO, "APP_CMD_LOST_FOCUS");
            PauseMusicStream();
        } break;
        case APP_CMD_TERM_WINDOW:
        {
            // TODO: Do display destruction here? -> Yes but only display, don't free buffers!

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
    WritePNG(buffer, imgData, renderWidth, renderHeight);

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

// Get current time measure since InitTimer()
static double GetTime(void)
{
#if defined(PLATFORM_DESKTOP)
    return glfwGetTime();
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t time = ts.tv_sec*1000000000LLU + (uint64_t)ts.tv_nsec;

    return (double)(time - baseTime) * 1e-9;
#endif
}

// Get one key state
static bool GetKeyStatus(int key)
{
#if defined(PLATFORM_DESKTOP)
    return glfwGetKey(window, key);
#elif defined(PLATFORM_ANDROID)
    // TODO: Check virtual keyboard (?)
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
#if defined(PLATFORM_DESKTOP)
    return glfwGetMouseButton(window, button);
#elif defined(PLATFORM_ANDROID)
    // TODO: Check virtual keyboard (?)
    return false;
#elif defined(PLATFORM_RPI)
    // NOTE: mouse buttons array is filled on PollInputEvents()
    return currentMouseState[button];
#endif
}

// Poll (store) all input events
static void PollInputEvents(void)
{
#if defined(PLATFORM_DESKTOP)
    // Mouse input polling
    double mouseX;
    double mouseY;

    glfwGetCursorPos(window, &mouseX, &mouseY);

    mousePosition.x = (float)mouseX;
    mousePosition.y = (float)mouseY;

    // Keyboard polling
    // Automatically managed by GLFW3 through callback

    glfwPollEvents();       // Register keyboard/mouse events
#elif defined(PLATFORM_ANDROID)

    // TODO: Check virtual keyboard (?)

    // Reset touchTap event
    touchTap = false;

    // Poll Events (registered events)
    while ((ident = ALooper_pollAll(0, NULL, &events,(void**)&source)) >= 0)
    {
        // Process this event
        if (source != NULL) source->process(app, source);

        // Check if we are exiting
        if (app->destroyRequested != 0)
        {
            TraceLog(INFO, "Closing Window...");
            //CloseWindow();
            windowShouldClose = true;
            //ANativeActivity_finish(app->activity);
        }
    }
#elif defined(PLATFORM_RPI)

    // NOTE: Mouse input events polling is done asynchonously in another pthread - MouseThread()

    // NOTE: Keyboard reading could be done using input_event(s) reading or just read from stdin,
    // we use method 2 (stdin) but maybe in a future we should change to method 1...

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

        if (keyboardMode == 2)
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
                TraceLog(INFO, "Pressed key (ASCII): 0x%02x", key);

                currentKeyState[key] = 1;
            }

            // Detect ESC to stop program
            if ((key == 0x1b) && (numKeysBuffer == 1)) windowShouldClose = true;
        }
        else if (keyboardMode == 1)
        {
            TraceLog(INFO, "Pressed key (keycode): 0x%02x", key);

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


        // Same fucnionality as GLFW3 KeyCallback()
        /*
        if (asciiKey == exitKey) windowShouldClose = true;
        else if (key == GLFW_KEY_F12 && action == GLFW_PRESS)
        {
            TakeScreenshot();
        }
        */
    }

    // TODO: Gamepad support (use events, easy!)
/*
    struct js_event gamepadEvent;

    read(gamepadStream, &gamepadEvent, sizeof(struct js_event));

    if (gamepadEvent.type == JS_EVENT_BUTTON)
    {
        switch (gamepadEvent.number)
        {
            case 0: // 1st Axis X
            case 1: // 1st Axis Y
            case 2: // 2st Axis X
            case 3: // 2st Axis Y
            case 4:
            {
                if (gamepadEvent.value == 1) // Button pressed, 0 release

            } break;
            // Buttons is similar, variable for every joystick
        }
    }
    else if (gamepadEvent.type == JS_EVENT_AXIS)
    {
        switch (gamepadEvent.number)
        {
            case 0: // 1st Axis X
            case 1: // 1st Axis Y
            case 2: // 2st Axis X
            case 3: // 2st Axis Y
            // Buttons is similar, variable for every joystick
        }
    }
*/
#endif
}

#if defined(PLATFORM_RPI)
// Mouse initialization (including mouse thread)
static void InitMouse(void)
{
    // NOTE: We can use /dev/input/mice to read from all available mice
    if ((mouseStream = open(DEFAULT_MOUSE_DEV, O_RDONLY|O_NONBLOCK)) < 0) TraceLog(WARNING, "Could not open mouse device, no mouse available");
    else
    {
        mouseReady = true;

        int err = pthread_create(&mouseThreadId, NULL, &MouseThread, NULL);

        if (err != 0) TraceLog(WARNING, "Error creating mouse input event thread");
        else TraceLog(INFO, "Mouse device initialized successfully");
    }
}

// Mouse reading thread
// NOTE: We need a separate thread to avoid loosing mouse events,
// if too much time passes between reads, queue gets full and new events override older wants...
static void *MouseThread(void *arg)
{
    struct input_event mouseEvent;

    while(1)
    {
        // NOTE: read() will return -1 if the events queue is empty
        read(mouseStream, &mouseEvent, sizeof(struct input_event));

        // Check event types
        if (mouseEvent.type == EV_REL) // Relative motion event
        {
            if (mouseEvent.code == REL_X)
            {
                mousePosition.x += (float)mouseEvent.value;

                // Screen limits X check
                if (mousePosition.x < 0) mousePosition.x = 0;
                if (mousePosition.x > screenWidth) mousePosition.x = screenWidth;
            }

            if (mouseEvent.code == REL_Y)
            {
                mousePosition.y += (float)mouseEvent.value;

                // Screen limits Y check
                if (mousePosition.y < 0) mousePosition.y = 0;
                if (mousePosition.y > screenHeight) mousePosition.y = screenHeight;
            }

            if (mouseEvent.code == REL_WHEEL)
            {
                // mouseEvent.value give 1 or -1 (direction)
            }
        }
        else if (mouseEvent.type == EV_KEY) // Mouse button event
        {
            if (mouseEvent.code == BTN_LEFT) currentMouseState[0] = mouseEvent.value;
            if (mouseEvent.code == BTN_RIGHT) currentMouseState[1] = mouseEvent.value;
            if (mouseEvent.code == BTN_MIDDLE) currentMouseState[2] = mouseEvent.value;
        }
    }

    return NULL;
}

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

    // NOTE: Reading directly from stdin will give chars already key-mapped by kernel to ASCII or UNICODE, we change that!

    // Save old keyboard mode to restore it at the end
    if (ioctl(STDIN_FILENO, KDGKBMODE, &defaultKeyboardMode) < 0)
    {
        // NOTE: It could mean we are using a remote keyboard through ssh!
        TraceLog(WARNING, "Could not change keyboard mode (SSH keyboard?)");

        keyboardMode = 2;   // ASCII
    }
    else
    {
        // We reconfigure keyboard mode to get scancodes (K_RAW) or keycodes (K_MEDIUMRAW)
        ioctl(STDIN_FILENO, KDSKBMODE, K_MEDIUMRAW);    // ASCII chars (K_XLATE), UNICODE chars (K_UNICODE)

        keyboardMode = 1;   // keycodes
    }

    // Register keyboard restore when program finishes
    atexit(RestoreKeyboard);
}

// Restore default keyboard input
static void RestoreKeyboard(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &defaultKeyboardSettings);
    ioctl(STDIN_FILENO, KDSKBMODE, defaultKeyboardMode);
}

// Init gamepad system
static void InitGamepad(void)
{
    // TODO: Gamepad support
    if ((gamepadStream = open(DEFAULT_GAMEPAD_DEV, O_RDONLY|O_NONBLOCK)) < 0) TraceLog(WARNING, "Could not open gamepad device, no gamepad available");
    else TraceLog(INFO, "Gamepad device initialized successfully");
}
#endif

// Copy back buffer to front buffers
static void SwapBuffers(void)
{
#if defined(PLATFORM_DESKTOP)
    glfwSwapBuffers(window);
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
    eglSwapBuffers(display, surface);
#endif
}

// Compute framebuffer size relative to screen size and display size
// NOTE: Global variables renderWidth/renderHeight can be modified
static void SetupFramebufferSize(int displayWidth, int displayHeight)
{
    // Calculate renderWidth and renderHeight, we have the display size (input params) and the desired screen size (global var)
    if ((screenWidth > displayWidth) || (screenHeight > displayHeight))
    {
        TraceLog(WARNING, "DOWNSCALING: Required screen size (%i x %i) is bigger than display size (%i x %i)", screenWidth, screenHeight, displayWidth, displayHeight);

        // Downscaling to fit display with border-bars
        float widthRatio = (float)displayWidth/(float)screenWidth;
        float heightRatio = (float)displayHeight/(float)screenHeight;

        if (widthRatio <= heightRatio)
        {
            renderWidth = displayWidth;
            renderHeight = (int)((float)screenHeight*widthRatio);
            renderOffsetX = 0;
            renderOffsetY = (displayHeight - renderHeight);
        }
        else
        {
            renderWidth = (int)((float)screenWidth*heightRatio);
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
            renderHeight = (int)((float)screenWidth/displayRatio);
            renderOffsetX = 0;
            renderOffsetY = (renderHeight - screenHeight);
        }
        else
        {
            renderWidth = (int)((float)screenHeight*displayRatio);
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

// Plays raylib logo appearing animation
static void LogoAnimation(void)
{
    int logoPositionX = screenWidth/2 - 128;
    int logoPositionY = screenHeight/2 - 128;

    int framesCounter = 0;
    int lettersCount = 0;

    int topSideRecWidth = 16;
    int leftSideRecHeight = 16;

    int bottomSideRecWidth = 16;
    int rightSideRecHeight = 16;

    char raylib[8] = "       ";     // raylib text array, max 8 letters
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

            switch (lettersCount)
            {
                case 1: raylib[0] = 'r'; break;
                case 2: raylib[1] = 'a'; break;
                case 3: raylib[2] = 'y'; break;
                case 4: raylib[3] = 'l'; break;
                case 5: raylib[4] = 'i'; break;
                case 6: raylib[5] = 'b'; break;
                default: break;
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

                DrawText(raylib, screenWidth/2 - 44, screenHeight/2 + 48, 50, Fade(BLACK, alpha));
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    showLogo = false;  // Prevent for repeating when reloading window (Android)
}
