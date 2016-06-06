/*******************************************************************************************
*
*   raylib [rlgl] example - Using rlgl module as standalone module
*
*   NOTE: This example requires OpenGL 3.3 or ES2 versions for shaders support,
*         OpenGL 1.1 does not support shaders but it can also be used.
*
*   Compile rlgl module using:
*   gcc -c rlgl.c -Wall -std=c99 -DRLGL_STANDALONE -DRAYMATH_IMPLEMENTATION -DGRAPHICS_API_OPENGL_33
*
*   Compile example using:
*   gcc -o $(NAME_PART).exe $(FILE_NAME) rlgl.o -lglfw3 -lopengl32 -lgdi32 -std=c99
*
*   This example has been created using raylib 1.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#define GLAD_IMPLEMENTATION
#include "glad.h"               // Extensions loading library
#include <GLFW/glfw3.h>         // Windows/Context and inputs management

#define RLGL_STANDALONE
#include "rlgl.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define RED        (Color){ 230, 41, 55, 255 }     // Red
#define MAROON     (Color){ 190, 33, 55, 255 }     // Maroon
#define RAYWHITE   (Color){ 245, 245, 245, 255 }   // My own White (raylib logo)
#define DARKGRAY   (Color){ 80, 80, 80, 255 }      // Dark Gray
//----------------------------------------------------------------------------------
typedef enum { LOG_INFO = 0, LOG_ERROR, LOG_WARNING, LOG_DEBUG, LOG_OTHER } TraceLogType;

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void ErrorCallback(int error, const char* description);
static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void TraceLog(int msgType, const char *text, ...);

// Drawing functions (uses rlgl functionality)
static void DrawGrid(int slices, float spacing);
static void DrawCube(Vector3 position, float width, float height, float length, Color color);
static void DrawCubeWires(Vector3 position, float width, float height, float length, Color color);
static void DrawRectangleV(Vector2 position, Vector2 size, Color color);

//----------------------------------------------------------------------------------
// Main Entry point
//----------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    
    
    // GLFW3 Initialization + OpenGL 3.3 Context + Extensions
    //--------------------------------------------------------
    glfwSetErrorCallback(ErrorCallback);
    
    if (!glfwInit())
    {
        TraceLog(LOG_WARNING, "GLFW3: Can not initialize GLFW");
        exit(EXIT_FAILURE);
    }
    else TraceLog(LOG_INFO, "GLFW3: GLFW initialized successfully");
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
   
    GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, "rlgl standalone", NULL, NULL);
    
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    else TraceLog(LOG_INFO, "GLFW3: Window created successfully");
    
    glfwSetKeyCallback(window, KeyCallback);
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Load OpenGL 3.3 extensions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        TraceLog(LOG_WARNING, "GLAD: Cannot load OpenGL extensions");
        exit(1);
    }
    else TraceLog(LOG_INFO, "GLAD: OpenGL extensions loaded successfully");
    //--------------------------------------------------------
    
    // Initialize rlgl internal buffers and OpenGL state
    rlglInit();
    rlglInitGraphics(0, 0, screenWidth, screenHeight);
    rlClearColor(245, 245, 245, 255);   // Define clear color
    rlEnableDepthTest();                // Enable DEPTH_TEST for 3D
    
    Vector2 size = { 200, 200 };
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
    
    Camera camera;
    camera.position = (Vector3){ 5.0f, 5.0f, 5.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    //--------------------------------------------------------------------------------------    

    // Main game loop    
    while (!glfwWindowShouldClose(window)) 
    {
        // Update
        //----------------------------------------------------------------------------------
        // ...
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        rlClearScreenBuffers();             // Clear current framebuffer
            // Calculate projection matrix (from perspective) and view matrix from camera look at
            Matrix matProj = MatrixPerspective(camera.fovy, (double)screenWidth/(double)screenHeight, 0.01, 1000.0);
            MatrixTranspose(&matProj);
            Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
            Matrix mvp = MatrixMultiply(matView, matProj);

            DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
            DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, RAYWHITE);
            DrawGrid(10, 1.0f);

            // NOTE: Internal buffers drawing (3D data)
            rlglDraw(mvp);
            
            matProj = MatrixOrtho(0.0, screenWidth, screenHeight, 0.0, 0.0, 1.0);
            MatrixTranspose(&matProj);
            matView = MatrixIdentity();
            mvp = MatrixMultiply(matView, matProj);
            
            // TODO: 2D drawing on Oculus Rift: requires an ovrLayerQuad layer
            DrawRectangleV((Vector2){ 10.0f, 10.0f }, (Vector2){ 300.0f, 20.0f }, DARKGRAY);

            // NOTE: Internal buffers drawing (2D data)
            rlglDraw(mvp);
            
        glfwSwapBuffers(window);
        glfwPollEvents();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    rlglClose();                            // Unload rlgl internal buffers and default shader/texture
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    //--------------------------------------------------------------------------------------
    
    return 0;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definitions
//----------------------------------------------------------------------------------

// GLFW3: Error callback
static void ErrorCallback(int error, const char* description)
{
    TraceLog(LOG_ERROR, description);
}

// GLFW3: Keyboard callback
static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

// Output a trace log message
static void TraceLog(int msgType, const char *text, ...)
{
    va_list args;
    va_start(args, text);

    switch(msgType)
    {
        case LOG_INFO: fprintf(stdout, "INFO: "); break;
        case LOG_ERROR: fprintf(stdout, "ERROR: "); break;
        case LOG_WARNING: fprintf(stdout, "WARNING: "); break;
        case LOG_DEBUG: fprintf(stdout, "DEBUG: "); break;
        default: break;
    }

    vfprintf(stdout, text, args);
    fprintf(stdout, "\n");

    va_end(args);

    //if (msgType == LOG_ERROR) exit(1);
}

// Draw rectangle using rlgl OpenGL 1.1 style coding (translated to OpenGL 3.3 internally)
static void DrawRectangleV(Vector2 position, Vector2 size, Color color)
{
    rlBegin(RL_TRIANGLES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        rlVertex2i(position.x, position.y);
        rlVertex2i(position.x, position.y + size.y);
        rlVertex2i(position.x + size.x, position.y + size.y);

        rlVertex2i(position.x, position.y);
        rlVertex2i(position.x + size.x, position.y + size.y);
        rlVertex2i(position.x + size.x, position.y);
    rlEnd();
}

// Draw a grid centered at (0, 0, 0)
static void DrawGrid(int slices, float spacing)
{
    int halfSlices = slices / 2;

    rlBegin(RL_LINES);
        for(int i = -halfSlices; i <= halfSlices; i++)
        {
            if (i == 0)
            {
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
            }
            else
            {
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
            }

            rlVertex3f((float)i*spacing, 0.0f, (float)-halfSlices*spacing);
            rlVertex3f((float)i*spacing, 0.0f, (float)halfSlices*spacing);

            rlVertex3f((float)-halfSlices*spacing, 0.0f, (float)i*spacing);
            rlVertex3f((float)halfSlices*spacing, 0.0f, (float)i*spacing);
        }
    rlEnd();
}

// Draw cube
// NOTE: Cube position is the center position
void DrawCube(Vector3 position, float width, float height, float length, Color color)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    rlPushMatrix();

        // NOTE: Be careful! Function order matters (rotate -> scale -> translate)
        rlTranslatef(position.x, position.y, position.z);
        //rlScalef(2.0f, 2.0f, 2.0f);
        //rlRotatef(45, 0, 1, 0);

        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            // Front Face -----------------------------------------------------
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left

            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right

            // Back Face ------------------------------------------------------
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Left
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right

            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left

            // Top Face -------------------------------------------------------
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Bottom Left
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Bottom Right

            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Bottom Right

            // Bottom Face ----------------------------------------------------
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Top Left
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left

            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Top Right
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Top Left

            // Right face -----------------------------------------------------
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Left

            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Left

            // Left Face ------------------------------------------------------
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Right

            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Right
        rlEnd();
    rlPopMatrix();
}

// Draw cube wires
void DrawCubeWires(Vector3 position, float width, float height, float length, Color color)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    rlPushMatrix();

        rlTranslatef(position.x, position.y, position.z);
        //rlRotatef(45, 0, 1, 0);

        rlBegin(RL_LINES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            // Front Face -----------------------------------------------------
            // Bottom Line
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right

            // Left Line
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right

            // Top Line
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left

            // Right Line
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left

            // Back Face ------------------------------------------------------
            // Bottom Line
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right

            // Left Line
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right

            // Top Line
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left

            // Right Line
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Left

            // Top Face -------------------------------------------------------
            // Left Line
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left Front
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left Back

            // Right Line
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right Front
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right Back

            // Bottom Face  ---------------------------------------------------
            // Left Line
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Top Left Front
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Top Left Back

            // Right Line
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Top Right Front
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Top Right Back
        rlEnd();
    rlPopMatrix();
}
