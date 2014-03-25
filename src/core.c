/*********************************************************************************************
*
*   raylib.core
*
*   Basic functions to manage Windows, OpenGL context and Input
*    
*   Uses external lib:    
*       GLFW3 - Window, context and Input management (static lib version)
*
*   Copyright (c) 2013 Ramon Santamaria (Ray San - raysan@raysanweb.com)
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

#include "raylib.h"

#include "rlgl.h"           // raylib OpenGL abstraction layer to OpenGL 1.1, 3.3+ or ES2

#include <GLFW/glfw3.h>     // GLFW3 lib: Windows, OpenGL context and Input management
//#include <GL/gl.h>        // OpenGL functions (GLFW3 already includes gl.h)
#include <stdio.h>          // Standard input / output lib
#include <stdlib.h>         // Declares malloc() and free() for memory management, rand()
#include <time.h>           // Useful to initialize random seed
#include <math.h>           // Math related functions, tan() used to set perspective
//#include "vector3.h"      // Basic Vector3 functions, not required any more, replaced by raymath
#include "utils.h"          // WritePNG() function

#include "raymath.h"        // Required for data type Matrix and Matrix functions

//#define GLFW_DLL          // Using GLFW DLL on Windows -> No, we use static version!

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Nop...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static GLFWwindow* window;                  // Main window
static bool fullscreen;                     // Fullscreen mode track

static double currentTime, previousTime;    // Used to track timmings
static double updateTime, drawTime;         // Time measures for update and draw
static double frameTime;                    // Time measure for one frame
static double targetTime = 0;               // Desired time for one frame, if 0 not applied

static int windowWidth, windowHeight;       // Required to switch between windowed/fullscren mode (F11)
static const char *windowTitle;             // Required to switch between windowed/fullscren mode (F11)
static int exitKey = GLFW_KEY_ESCAPE;       // Default exit key (ESC)

static bool customCursor = false;           // Tracks if custom cursor has been set
static bool cursorOnScreen = false;         // Tracks if cursor is inside client area
static Texture2D cursor;                    // Cursor texture

static char previousKeyState[512] = { 0 };  // Required to check if key pressed/released once
static char currentKeyState[512] = { 0 };   // Required to check if key pressed/released once

static char previousMouseState[3] = { 0 };  // Required to check if mouse btn pressed/released once
static char currentMouseState[3] = { 0 };   // Required to check if mouse btn pressed/released once

static char previousGamepadState[32] = {0}; // Required to check if gamepad btn pressed/released once
static char currentGamepadState[32] = {0};  // Required to check if gamepad btn pressed/released once

static int previousMouseWheelY = 0;         // Required to track mouse wheel variation
static int currentMouseWheelY = 0;          // Required to track mouse wheel variation

static Color background = { 0, 0, 0, 0 };   // Screen background color

//----------------------------------------------------------------------------------
// Other Modules Functions Declaration (required by core)
//----------------------------------------------------------------------------------
extern void LoadDefaultFont();               // [Module: text] Loads default font on InitWindow()
extern void UnloadDefaultFont();             // [Module: text] Unloads default font from GPU memory

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void ErrorCallback(int error, const char *description);                             // GLFW3 Error Callback, runs on GLFW3 error
static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);  // GLFW3 Keyboard Callback, runs on key pressed
static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);            // GLFW3 Srolling Callback, runs on mouse wheel
static void CursorEnterCallback(GLFWwindow* window, int enter);                            // GLFW3 Cursor Enter Callback, cursor enters client area
static void WindowSizeCallback(GLFWwindow* window, int width, int height);                 // GLFW3 WindowSize Callback, runs when window is resized
static void TakeScreenshot();                                                              // Takes a bitmap (BMP) screenshot and saves it in the same folder as executable

//----------------------------------------------------------------------------------
// Module Functions Definition - Window and OpenGL Context Functions
//----------------------------------------------------------------------------------

// Initialize Window and Graphics Context (OpenGL)
void InitWindow(int width, int height, const char *title)
{
    InitWindowEx(width, height, title, true, NULL);
}

// Initialize Window and Graphics Context (OpenGL) with extended parameters
void InitWindowEx(int width, int height, const char* title, bool resizable, const char *cursorImage)
{
    glfwSetErrorCallback(ErrorCallback);
    
    if (!glfwInit()) exit(1);
    
    //glfwDefaultWindowHints()                  // Set default windows hints
    
    if (!resizable) glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // Avoid window being resizable

#ifdef USE_OPENGL_33
    //glfwWindowHint(GLFW_SAMPLES, 4);          // Enables multisampling x4 (MSAA), default is 0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
#endif

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    
    windowWidth = width;
    windowHeight = height;
    windowTitle = title;
    
    if (!window)
    {
        glfwTerminate();
        exit(1);
    }
    
    glfwSetWindowSizeCallback(window, WindowSizeCallback);
    glfwSetCursorEnterCallback(window, CursorEnterCallback);
    
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSwapInterval(0);            // Disables GPU v-sync (if set), so frames are not limited to screen refresh rate (60Hz -> 60 FPS)
                                    // If not set, swap interval uses GPU v-sync configuration
                                    // Framerate can be setup using SetTargetFPS()

    //------------------------------------------------------ 
#ifdef USE_OPENGL_33
    rlglInit();                     // Init rlgl
#endif
    //------------------------------------------------------
    
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);    // Get framebuffer size of current window

    //------------------------------------------------------ 
    rlglInitGraphicsDevice(fbWidth, fbHeight);
    //------------------------------------------------------
    
    previousTime = glfwGetTime();

    LoadDefaultFont();              // NOTE: External function (defined in module: text)
    
    if (cursorImage != NULL) SetCustomCursor(cursorImage);
    
    srand(time(NULL));              // Initialize random seed
    
    ClearBackground(RAYWHITE);      // Default background color for raylib games :P
}

// Close Window and Terminate Context
void CloseWindow()
{
    UnloadDefaultFont();
    
    //------------------------------------------------------
#ifdef USE_OPENGL_33
    rlglClose();                    // De-init rlgl
#endif
    //------------------------------------------------------

    glfwDestroyWindow(window);
    glfwTerminate();
}

// Set a custom cursor icon/image
void SetCustomCursor(const char *cursorImage)
{
    if (customCursor) UnloadTexture(cursor);
    
    cursor = LoadTexture(cursorImage);
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    customCursor = true;
}

// Set a custom key to exit program
// NOTE: default exitKey is ESCAPE
void SetExitKey(int key)
{
    exitKey = key;
}

// Detect if KEY_ESCAPE pressed or Close icon pressed
bool WindowShouldClose()
{
    return (glfwWindowShouldClose(window));
}

// Fullscreen toggle (by default F11)
void ToggleFullscreen()
{
    if (glfwGetKey(window, GLFW_KEY_F11)) 
    {
        fullscreen = !fullscreen;          // Toggle fullscreen flag

        UnloadDefaultFont();
        
        glfwDestroyWindow(window);         // Destroy the current window (we will recreate it!)
        
        // TODO: WARNING! All loaded resources are lost, we loose Context!

        // NOTE: Window aspect ratio is always windowWidth / windowHeight
        if (fullscreen) window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, glfwGetPrimaryMonitor(), NULL);    // Fullscreen mode
        else window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
    
        if (!window)
        {
            glfwTerminate();
            exit(1);
        }
        
        glfwMakeContextCurrent(window);
        glfwSetKeyCallback(window, KeyCallback);

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);    // Get framebuffer size of current window

        rlglInitGraphicsDevice(fbWidth, fbHeight);
        
        LoadDefaultFont();
    }
}

// Sets Background Color
void ClearBackground(Color color)
{
    if ((color.r != background.r) || (color.g != background.g) || (color.b != background.b) || (color.a != background.a))
    {       
        rlClearColor(color.r, color.g, color.b, color.a);
        
        background = color;
    }
}

// Setup drawing canvas to start drawing
void BeginDrawing()
{
    currentTime = glfwGetTime();        // glfwGetTime() returns a 'double' containing the number of elapsed seconds since glfwInit() was called
    updateTime = currentTime - previousTime;
    previousTime = currentTime;

    rlClearScreenBuffers();
    
    rlLoadIdentity();                   // Reset current matrix (MODELVIEW)

//#ifdef USE_OPENGL_11
//  rlTranslatef(0.375, 0.375, 0);      // HACK to have 2D pixel-perfect drawing on OpenGL
                                        // NOTE: Not required with OpenGL 3.3+
//#endif
}

// End canvas drawing and Swap Buffers (Double Buffering)
void EndDrawing()
{
    if (customCursor && cursorOnScreen) DrawTexture(cursor, GetMouseX(), GetMouseY(), WHITE);

    //------------------------------------------------------
#ifdef USE_OPENGL_33
    rlglDraw();                         //  Draw Buffers
#endif
    //------------------------------------------------------
    
    glfwSwapBuffers(window);            // Swap back and front buffers
    glfwPollEvents();                   // Register keyboard/mouse events
    
    currentTime = glfwGetTime();
    drawTime = currentTime - previousTime;
    previousTime = currentTime;
    
    frameTime = updateTime + drawTime;
    
    double extraTime = 0;

    while (frameTime < targetTime)
    {
        // Implement a delay
        currentTime = glfwGetTime();
        extraTime = currentTime - previousTime;
        previousTime = currentTime;
        frameTime += extraTime;
    }
}

// Initializes 3D mode for drawing (Camera setup)
void Begin3dMode(Camera camera)
{
    //------------------------------------------------------
#ifdef USE_OPENGL_33
    rlglDraw();                         //  Draw Buffers
#endif
    //------------------------------------------------------

    rlMatrixMode(RL_PROJECTION);        // Switch to projection matrix
    
    rlPushMatrix();                     // Save previous matrix, which contains the settings for the 2d ortho projection
    rlLoadIdentity();                   // Reset current matrix (PROJECTION)
    
    // Setup perspective projection
    float aspect = (GLfloat)windowWidth/(GLfloat)windowHeight;
    double top = 0.1f*tan(45.0f*PI / 360.0);
    double right = top*aspect;

    rlFrustum(-right, right, -top, top, 0.1f, 100.0f);
    
    rlMatrixMode(RL_MODELVIEW);         // Switch back to modelview matrix
    rlLoadIdentity();                   // Reset current matrix (MODELVIEW)
    
    // Setup Camera view
    Matrix matLookAt = MatrixLookAt(camera.position, camera.target, camera.up);
    rlMultMatrixf(GetMatrixVector(matLookAt));      // Multiply MODELVIEW matrix by view matrix (camera)
}

// Ends 3D mode and returns to default 2D orthographic mode
void End3dMode()
{
    //------------------------------------------------------
#ifdef USE_OPENGL_33
    rlglDraw();                         //  Draw Buffers
#endif
    //------------------------------------------------------

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
    
    printf("TargetTime per Frame: %f seconds\n", (float)targetTime);
}

// Returns current FPS
float GetFPS()
{
    return (1/(float)frameTime);
}

// Returns time in seconds for one frame
float GetFrameTime()
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
    if (alpha < 0.0) alpha = 0.0;
    else if (alpha > 1.0) alpha = 1.0;

    return (Color){color.r, color.g, color.b, color.a*alpha};
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Input (Keyboard, Mouse, Gamepad) Functions
//----------------------------------------------------------------------------------

// Detect if a key has been pressed once
bool IsKeyPressed(int key)
{   
    bool ret = false;

    currentKeyState[key] = IsKeyDown(key);

    if (currentKeyState[key] != previousKeyState[key])
    {
        if (currentKeyState[key]) ret = true;
        previousKeyState[key] = currentKeyState[key];
    }
    else ret = false;
    
    return ret;
}

// Detect if a key is being pressed (key held down)
bool IsKeyDown(int key)
{
    if (glfwGetKey(window, key) == GLFW_PRESS) return true;
    else return false;
}

// Detect if a key has been released once
bool IsKeyReleased(int key)
{   
    bool ret = false;
    
    currentKeyState[key] = IsKeyUp(key);

    if (currentKeyState[key] != previousKeyState[key])
    {
        if (currentKeyState[key]) ret = true;
        previousKeyState[key] = currentKeyState[key];
    }
    else ret = false;
    
    return ret;
}

// Detect if a key is NOT being pressed (key not held down)
bool IsKeyUp(int key)
{
    if (glfwGetKey(window, key) == GLFW_RELEASE) return true;
    else return false;
}

// Detect if a mouse button has been pressed once
bool IsMouseButtonPressed(int button)
{
    bool ret = false;

    currentMouseState[button] = IsMouseButtonDown(button);

    if (currentMouseState[button] != previousMouseState[button])
    {
        if (currentMouseState[button]) ret = true;
        previousMouseState[button] = currentMouseState[button];
    }
    else ret = false;
    
    return ret;
}

// Detect if a mouse button is being pressed
bool IsMouseButtonDown(int button)
{
    if (glfwGetMouseButton(window, button) == GLFW_PRESS) return true;
    else return false;
}

// Detect if a mouse button has been released once
bool IsMouseButtonReleased(int button)
{
    bool ret = false;

    currentMouseState[button] = IsMouseButtonUp(button);

    if (currentMouseState[button] != previousMouseState[button])
    {
        if (currentMouseState[button]) ret = true;
        previousMouseState[button] = currentMouseState[button];
    }
    else ret = false;
    
    return ret;
}

// Detect if a mouse button is NOT being pressed
bool IsMouseButtonUp(int button)
{
    if (glfwGetMouseButton(window, button) == GLFW_RELEASE) return true;
    else return false;
}

// Returns mouse position X
int GetMouseX()
{
    double mouseX;
    double mouseY;
    
    glfwGetCursorPos(window, &mouseX, &mouseY);

    return (int)mouseX;
}

// Returns mouse position Y
int GetMouseY()
{
    double mouseX;
    double mouseY;
    
    glfwGetCursorPos(window, &mouseX, &mouseY);

    return (int)mouseY;
}

// Returns mouse position XY
Vector2 GetMousePosition()
{
    double mouseX;
    double mouseY;
    
    glfwGetCursorPos(window, &mouseX, &mouseY);
    
    Vector2 position = { (float)mouseX, (float)mouseY };

    return position;
}

// Returns mouse wheel movement Y
int GetMouseWheelMove()
{
    previousMouseWheelY = currentMouseWheelY;

    currentMouseWheelY = 0;
    
    return previousMouseWheelY;
}

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

// Detect if a gamepad button is being pressed
bool IsGamepadButtonPressed(int gamepad, int button)
{
    bool ret = false;

    currentGamepadState[button] = IsGamepadButtonDown(gamepad, button);

    if (currentGamepadState[button] != previousGamepadState[button])
    {
        if (currentGamepadState[button]) ret = true;
        previousGamepadState[button] = currentGamepadState[button];
    }
    else ret = false;
    
    return ret;
}

bool IsGamepadButtonDown(int gamepad, int button)
{
    const unsigned char* buttons;
    int buttonsCount;
    
    buttons = glfwGetJoystickButtons(gamepad, &buttonsCount);
    
    if ((buttons != NULL) && (buttons[button] == GLFW_PRESS))
    {
        return true;
    }
    else return false;
}

// Detect if a gamepad button is NOT being pressed
bool IsGamepadButtonReleased(int gamepad, int button)
{
    bool ret = false;

    currentGamepadState[button] = IsGamepadButtonUp(gamepad, button);

    if (currentGamepadState[button] != previousGamepadState[button])
    {
        if (currentGamepadState[button]) ret = true;
        previousGamepadState[button] = currentGamepadState[button];
    }
    else ret = false;
    
    return ret;
}

bool IsGamepadButtonUp(int gamepad, int button)
{
    const unsigned char* buttons;
    int buttonsCount;
    
    buttons = glfwGetJoystickButtons(gamepad, &buttonsCount);
    
    if ((buttons != NULL) && (buttons[button] == GLFW_RELEASE))
    {
        return true;
    }
    else return false;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// GLFW3 Error Callback, runs on GLFW3 error
static void ErrorCallback(int error, const char *description)
{
    printf(description);
    //fprintf(stderr, description);
}

// GLFW3 Srolling Callback, runs on mouse wheel
static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    currentMouseWheelY = (int)yoffset;
}

// GLFW3 Keyboard Callback, runs on key pressed
static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == exitKey && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
        
        // NOTE: Before closing window, while loop must be left!
    }
    else if (key == GLFW_KEY_F11 && action == GLFW_PRESS)
    {
        ToggleFullscreen();
    }
    else if (key == GLFW_KEY_F12 && action == GLFW_PRESS)
    {
        TakeScreenshot();
    }
}

static void CursorEnterCallback(GLFWwindow* window, int enter)
{
    if (enter == GL_TRUE) cursorOnScreen = true;
    else cursorOnScreen = false;
}

// GLFW3 WindowSize Callback, runs when window is resized
static void WindowSizeCallback(GLFWwindow* window, int width, int height)
{
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);    // Get framebuffer size of current window

    // If window is resized, graphics device is re-initialized
    // NOTE: Aspect ratio does not change, so, image can be deformed
    rlglInitGraphicsDevice(fbWidth, fbHeight);
}

// Takes a bitmap (BMP) screenshot and saves it in the same folder as executable
static void TakeScreenshot()
{
    static int shotNum = 0;     // Screenshot number, increments every screenshot take during program execution

    char buffer[20];            // Buffer to store file name
    int fbWidth, fbHeight;      // Frame buffer width and height

    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);    // Get framebuffer size of current window

    unsigned char *imgData = rlglReadScreenPixels(fbWidth, fbHeight);

    sprintf(buffer, "screenshot%03i.png", shotNum);

    WritePNG(buffer, imgData, fbWidth, fbHeight);

    free(imgData);

    shotNum++;
}