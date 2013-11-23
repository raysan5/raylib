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

#include <GLFW/glfw3.h>     // GLFW3 lib: Windows, OpenGL context and Input management
//#include <GL/gl.h>        // OpenGL functions (GLFW3 already includes gl.h)
#include <stdio.h>          // Standard input / output lib
#include <stdlib.h>         // Declares malloc() and free() for memory management
#include <math.h>           // Math related functions, tan() on SetPerspective
#include "vector3.h"        // Basic Vector3 functions

//#define GLFW_DLL          // Using GLFW DLL on Windows -> No, we use static version!

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Nop...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef Color pixel;

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
static char *windowTitle;                   // Required to switch between windowed/fullscren mode (F11)

//----------------------------------------------------------------------------------
// Other Modules Functions Declaration (required by core)
//----------------------------------------------------------------------------------
extern void LoadDefaultFont();               // [Module: text] Loads default font on InitWindow()
extern void UnloadDefaultFont();             // [Module: text] Unloads default font from GPU memory
extern void WriteBitmap(const char *fileName, const pixel *imgDataPixel, int width, int height);    // [Module: textures] Writes a bitmap (BMP) file

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void InitGraphicsDevice();                                                          // Initialize Graphics Device (OpenGL stuff)
static void ErrorCallback(int error, const char *description);                             // GLFW3 Error Callback, runs on GLFW3 error
static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);  // GLFW3 Keyboard Callback, runs on key pressed
static void WindowSizeCallback(GLFWwindow* window, int width, int height);                 // GLFW3 WindowSize Callback, runs when window is resized
static void CameraLookAt(Vector3 position, Vector3 target, Vector3 up);                    // Setup camera view (updates MODELVIEW matrix)
static void SetPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar); // Setup view projection (updates PROJECTION matrix)
static void TakeScreenshot();                                                              // Takes a bitmap (BMP) screenshot and saves it in the same folder as executable

//----------------------------------------------------------------------------------
// Module Functions Definition - Window and OpenGL Context Functions
//----------------------------------------------------------------------------------

// Initialize Window and Graphics Context (OpenGL)
void InitWindow(int width, int height, char* title)
{
    glfwSetErrorCallback(ErrorCallback);
    
    if (!glfwInit()) exit(1);
    
    //glfwWindowHint(GLFW_SAMPLES, 4);    // If called before windows creation, enables multisampling x4 (MSAA), default is 0
        
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
    
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSwapInterval(0);            // Disables GPU v-sync (if set), so frames are not limited to screen refresh rate (60Hz -> 60 FPS)
                                    // If not set, swap interval uses GPU v-sync configuration
                                    // Framerate can be setup using SetTargetFPS()
    InitGraphicsDevice();
    
    previousTime = glfwGetTime();

    LoadDefaultFont();
}

// Close Window and Terminate Context
void CloseWindow()
{
    UnloadDefaultFont();

    glfwDestroyWindow(window);
    glfwTerminate();
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

        glfwDestroyWindow(window);         // Destroy the current window (we will recreate it!)

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

        InitGraphicsDevice();
    }
}

// Sets Background Color
void ClearBackground(Color color)
{
    // Color values clamp to 0.0f(0) and 1.0f(255)
    float r = (float)color.r / 255;
    float g = (float)color.g / 255;
    float b = (float)color.b / 255;
    float a = (float)color.a / 255;
    
    glClearColor(r, g, b, a);
}

// Setup drawing canvas to start drawing
void BeginDrawing()
{
    currentTime = glfwGetTime();        // glfwGetTime() returns a 'double' containing the number of elapsed seconds since glfwInit() was called
    updateTime = currentTime - previousTime;
    previousTime = currentTime;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear used buffers, Depth Buffer is used for 3D
    
    glLoadIdentity();                   // Reset current matrix (MODELVIEW)
    
    glTranslatef(0.375, 0.375, 0);      // HACK to have 2D pixel-perfect drawing on OpenGL
}

// End canvas drawing and Swap Buffers (Double Buffering)
void EndDrawing()
{
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
    //glEnable(GL_LIGHTING);            // TODO: Setup proper lighting system (raylib 1.x)
    
    glMatrixMode(GL_PROJECTION);        // Switch to projection matrix
    
    glPushMatrix();                     // Save previous matrix, which contains the settings for the 2d ortho projection
    glLoadIdentity();                   // Reset current matrix (PROJECTION)
    
    SetPerspective(45.0f, (GLfloat)windowWidth/(GLfloat)windowHeight, 0.1f, 100.0f);    // Setup perspective projection

    glMatrixMode(GL_MODELVIEW);         // Switch back to modelview matrix
    glLoadIdentity();                   // Reset current matrix (MODELVIEW)
    
    CameraLookAt(camera.position, camera.target, camera.up);        // Setup Camera view
}

// Ends 3D mode and returns to default 2D orthographic mode
void End3dMode()
{
    glMatrixMode(GL_PROJECTION);        // Switch to projection matrix
    glPopMatrix();                      // Restore previous matrix (PROJECTION) from matrix stack
    
    glMatrixMode(GL_MODELVIEW);         // Get back to modelview matrix
    glLoadIdentity();                   // Reset current matrix (MODELVIEW)
    
    glTranslatef(0.375, 0.375, 0);      // HACK to ensure pixel-perfect drawing on OpenGL (after exiting 3D mode)
        
    //glDisable(GL_LIGHTING);           // TODO: Setup proper lighting system (raylib 1.x)
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

//----------------------------------------------------------------------------------
// Module Functions Definition - Input (Keyboard, Mouse, Gamepad) Functions
//----------------------------------------------------------------------------------

// Detect if a key is being pressed (key held down)
bool IsKeyPressed(int key)
{
    if (glfwGetKey(window, key) == GLFW_PRESS) return true;
    else return false;
}

// Detect if a key is NOT being pressed (key not held down)
bool IsKeyReleased(int key)
{
    if (glfwGetKey(window, key) == GLFW_RELEASE) return true;
    else return false;
}

// Detect if a mouse button is being pressed
bool IsMouseButtonPressed(int button)
{
    if (glfwGetMouseButton(window, button) == GLFW_PRESS) return true;
    else return false;
}

// Detect if a mouse button is NOT being pressed
bool IsMouseButtonReleased(int button)
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
    const unsigned char* buttons;
    int buttonsCount;
    
    buttons = glfwGetJoystickButtons(gamepad, &buttonsCount);
    
    if (buttons[button] == GLFW_PRESS)
    {
        return true;
    }
    else return false;
}

// Detect if a gamepad button is NOT being pressed
bool IsGamepadButtonReleased(int gamepad, int button)
{
    const unsigned char* buttons;
    int buttonsCount;
    
    buttons = glfwGetJoystickButtons(gamepad, &buttonsCount);
    
    if (buttons[button] == GLFW_RELEASE)
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

// GLFW3 Keyboard Callback, runs on key pressed
static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
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

// GLFW3 WindowSize Callback, runs when window is resized
static void WindowSizeCallback(GLFWwindow* window, int width, int height)
{
    InitGraphicsDevice();   // If window is resized, graphics device is re-initialized
                            // NOTE: Aspect ratio does not change, so, image can be deformed
}

// Initialize Graphics Device (OpenGL stuff)
static void InitGraphicsDevice()
{
    int fbWidth, fbHeight;
    
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);    // Get framebuffer size of current window

    glViewport(0, 0, fbWidth, fbHeight);                    // Set viewport width and height

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear used buffers, depth buffer is used for 3D
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);                   // Set background color (black)
    glClearDepth(1.0f);                                     // Clear depth buffer
    
    glEnable(GL_DEPTH_TEST);                                // Enables depth testing (required for 3D)
    glDepthFunc(GL_LEQUAL);                                 // Type of depth testing to apply
    
    glEnable(GL_BLEND);                                     // Enable color blending (required to work with transparencies)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);      // Color blending function (how colors are mixed)
    
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);      // Improve quality of color and texture coordinate interpolation (Deprecated in OGL 3.0)
                                                            // Other options: GL_FASTEST, GL_DONT_CARE (default)
    
    glMatrixMode(GL_PROJECTION);                // Switch to PROJECTION matrix
    glLoadIdentity();                           // Reset current matrix (PROJECTION)
    glOrtho(0, fbWidth, fbHeight, 0, 0, 1);     // Config orthographic mode: top-left corner --> (0,0)
    glMatrixMode(GL_MODELVIEW);                 // Switch back to MODELVIEW matrix
    glLoadIdentity();                           // Reset current matrix (MODELVIEW)
    
    glDisable(GL_LIGHTING);                     // Lighting Disabled...
    
    // TODO: Create an efficient Lighting System with proper functions (raylib 1.x)
/*    
    glEnable(GL_COLOR_MATERIAL);                        // Enable materials, causes some glMaterial atributes to track the current color (glColor)...
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);  // Material types and where to apply them
                                                        // NOTE: ONLY works with lighting; defines how light interacts with material
                                                        
    glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient);     // Define ambient light color property
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);     // Define diffuse light color property
    glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);   // Define light position
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT1);                                // Enable light one (8 lights available at the same time)
*/    
    // TODO: Review all shapes/models are drawn CCW and enable backface culling

    //glEnable(GL_CULL_FACE);       // Enable backface culling (Disabled by default)
    //glCullFace(GL_BACK);          // Cull the Back face (default)
    //glFrontFace(GL_CCW);          // Front face are defined counter clockwise (default)
    
    glShadeModel(GL_SMOOTH);        // Smooth shading between vertex (vertex colors interpolation)
                                    // Possible options: GL_SMOOTH (Color interpolation) or GL_FLAT (no interpolation)
}

// Setup camera view (updates MODELVIEW matrix)
static void CameraLookAt(Vector3 position, Vector3 target, Vector3 up)
{
    float rotMatrix[16];            // Matrix to store camera rotation
    
    Vector3 rotX, rotY, rotZ;                // Vectors to calculate camera rotations X, Y, Z (Euler)
    
    // Construct rotation matrix from vectors
    rotZ = VectorSubtract(position, target);
    VectorNormalize(&rotZ);
    rotY = up;                                // Y rotation vector
    rotX = VectorCrossProduct(rotY, rotZ);    // X rotation vector = Y cross Z
    rotY = VectorCrossProduct(rotZ, rotX);    // Recompute Y rotation = Z cross X
    VectorNormalize(&rotX);                   // X rotation vector normalization
    VectorNormalize(&rotY);                   // Y rotation vector normalization
    
    rotMatrix[0] = rotX.x;
    rotMatrix[1] = rotY.x;
    rotMatrix[2] = rotZ.x;    
    rotMatrix[3] = 0.0f;    
    rotMatrix[4] = rotX.y;
    rotMatrix[5] = rotY.y;    
    rotMatrix[6] = rotZ.y;
    rotMatrix[7] = 0.0f;    
    rotMatrix[8] = rotX.z;
    rotMatrix[9] = rotY.z;
    rotMatrix[10] = rotZ.z;
    rotMatrix[11] = 0.0f;
    rotMatrix[12] = 0.0f;
    rotMatrix[13] = 0.0f;
    rotMatrix[14] = 0.0f;
    rotMatrix[15] = 1.0f;

    glMultMatrixf(rotMatrix);    // Multiply MODELVIEW matrix by rotation matrix
    
    glTranslatef(-position.x, -position.y, -position.z);    // Translate eye to position
}

// Setup view projection (updates PROJECTION matrix)
static void SetPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
   double xmin, xmax, ymin, ymax;

   ymax = zNear * tan(fovy * PI / 360.0);
   ymin = -ymax;
   xmin = ymin * aspect;
   xmax = ymax * aspect;

   glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

// Takes a bitmap (BMP) screenshot and saves it in the same folder as executable
static void TakeScreenshot()
{
    static int shotNum = 0;     // Screenshot number, increments every screenshot take during program execution
        
    char buffer[20];            // Buffer to store file name
    int fbWidth, fbHeight;
    
    Color *imgDataPixel;        // Pixel image data array

    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);    // Get framebuffer size of current window

    imgDataPixel = (Color *)malloc(fbWidth * fbHeight * sizeof(Color));

    // NOTE: glReadPixels returns image flipped vertically -> (0,0) is the bottom left corner of the framebuffer
    glReadPixels(0, 0, fbWidth, fbHeight, GL_RGBA, GL_UNSIGNED_BYTE, imgDataPixel);
    
    sprintf(buffer, "screenshot%03i.bmp", shotNum);

    // NOTE: BMP directly stores data flipped vertically
    WriteBitmap(buffer, imgDataPixel, fbWidth, fbHeight);    // Writes pixel data array into a bitmap (BMP) file
    
    free(imgDataPixel);
    
    shotNum++;
}