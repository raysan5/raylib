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
*   gcc -o $(NAME_PART).exe $(FILE_NAME) rlgl.o glad.o -lglfw3 -lopengl32 -lgdi32 -std=c99
*
*   This example has been created using raylib 1.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "glad.h"
#include <GLFW/glfw3.h>

#define RLGL_STANDALONE
#include "rlgl.h"

#include <stdlib.h>
#include <stdio.h>

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void ErrorCallback(int error, const char* description)
{
    fputs(description, stderr);
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void DrawRectangleV(Vector2 position, Vector2 size, Color color);

//----------------------------------------------------------------------------------
// Main Entry point
//----------------------------------------------------------------------------------
int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;
    
    GLFWwindow *window;
    
    glfwSetErrorCallback(ErrorCallback);
    
    if (!glfwInit()) exit(EXIT_FAILURE);
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   
    window = glfwCreateWindow(screenWidth, screenHeight, "rlgl standalone", NULL, NULL);
    
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    
    glfwSetKeyCallback(window, KeyCallback);
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Cannot load GL extensions.\n");
        exit(1);
    }
    
    rlglInit();
    rlglInitGraphics(0, 0, screenWidth, screenHeight);
    rlClearColor(245, 245, 245, 255); // Define clear color
    
    Vector2 position = { screenWidth/2 - 100, screenHeight/2 - 100 };
    Vector2 size = { 200, 200 };
    Color color = { 180, 20, 20, 255 };
    
    while (!glfwWindowShouldClose(window))
    {
        rlClearScreenBuffers();
        
        DrawRectangleV(position, size, color);
        
        rlglDraw();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    rlglClose();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definitions
//----------------------------------------------------------------------------------
void DrawRectangleV(Vector2 position, Vector2 size, Color color)
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