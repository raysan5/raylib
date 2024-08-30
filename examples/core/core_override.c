/*******************************************************************************************
*
*   raylib [core] example - Ascii rendering
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   Example contributed by Gavin Parker (@IoIxD) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2023 Gavin Parker (@IoIxD)
*
********************************************************************************************/

#include "raylib.h"
#include "rcore.h"

#include "GLFW/glfw3.h"         // Windows/Context and inputs management

#include <stdio.h>              // Required for: printf()
#include "rlgl.h"
#include "raymath.h"

const int screenWidth = 800;
const int screenHeight = 450;
GLFWwindow *window;


void CustomSetWindowTitle(const char *title)
{
    GetCore()->Window.title = title;
    glfwSetWindowTitle(window, title);
}

double CustomGetTime(void) {
    return glfwGetTime();
}

bool CustomWindowShouldClose(void) {
    if (GetCore()->Window.ready) return GetCore()->Window.shouldClose;
    else return true;
}

int CustomInitPlatform(void) {
    if (!glfwInit())
    {
        printf("GLFW3: Can not initialize GLFW\n");
        return 1;
    }
    else printf("GLFW3: GLFW initialized successfully\n");

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    
    // WARNING: OpenGL 3.3 Core profile only
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if defined(__APPLE__)
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
#endif

    window = glfwCreateWindow(screenWidth, screenHeight, "rlgl standalone", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return 2;
    }
    else printf("GLFW3: Window created successfully\n");

    //glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwSetWindowSize(window, GetCore()->Window.screen.width, GetCore()->Window.screen.height);
    glfwSetWindowPos(window, 200, 200);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    // Load OpenGL 3.3 supported extensions
    rlLoadExtensions(glfwGetProcAddress);
    //--------------------------------------------------------

    // Initialize OpenGL context (states and resources)
    rlglInit(screenWidth, screenHeight);

    // Initialize viewport and internal projection/modelview matrices
    rlViewport(0, 0, screenWidth, screenHeight);
    rlMatrixMode(RL_PROJECTION);                        // Switch to PROJECTION matrix
    rlLoadIdentity();                                   // Reset current matrix (PROJECTION)
    rlOrtho(0, screenWidth, screenHeight, 0, 0.0f, 1.0f); // Orthographic projection with top-left corner at (0,0)
    rlMatrixMode(RL_MODELVIEW);                         // Switch back to MODELVIEW matrix
    rlLoadIdentity();                                   // Reset current matrix (MODELVIEW)

    GetCore()->Window.ready = true;
    return 0;
}

void CustomClosePlatform(void) {
    glfwDestroyWindow(window);      // Close window
    glfwTerminate();                // Free GLFW3 resources
}



//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)

{

    union OverridableFunctionPointer funcs[4];

    funcs[0].InitPlatform = CustomInitPlatform;
    OverrideInternalFunction("InitPlatform",&funcs[0]);

    funcs[1].ClosePlatform = CustomClosePlatform;
    OverrideInternalFunction("ClosePlatform",&funcs[1]);

    funcs[2].WindowShouldClose = CustomWindowShouldClose;
    OverrideInternalFunction("WindowShouldClose",&funcs[2]);

    funcs[3].GetTime = CustomGetTime;
    OverrideInternalFunction("GetTime",&funcs[3]);

    funcs[4].SetWindowTitle = CustomSetWindowTitle;
    OverrideInternalFunction("SetWindowTitle",&funcs[4]);


    InitWindow(screenWidth,screenHeight,"Test");

    Color col = WHITE;

    SetTargetFPS(5);

    //--------------------------------------------------------------------------------------

    // Main game loop
    while(!WindowShouldClose()) {        
        BeginDrawing();
            ClearBackground(col);
        EndDrawing();

        col.r = GetRandomValue(0,255);
        col.g = GetRandomValue(0,255);
        col.b = GetRandomValue(0,255);

        glfwSwapBuffers(window);
        glfwPollEvents();
        GetCore()->Window.shouldClose = glfwWindowShouldClose(window);
    }

    //CustomClosePlatform();
    return 0;
}
