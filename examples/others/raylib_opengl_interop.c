/*******************************************************************************************
*
*   raylib [others] example - OpenGL interoperatibility
*
*   Example complexity rating: [★★★★] 4/4
*
*   Example originally created with raylib 3.8, last time updated with raylib 4.0
*
*   Example contributed by Stephan Soller (@arkanis) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2021-2025 Stephan Soller (@arkanis) and Ramon Santamaria (@raysan5)
*
********************************************************************************************
*
*   Mixes raylib and plain OpenGL code to draw a GL_POINTS based particle system. The
*   primary point is to demonstrate raylib and OpenGL interop
*
*   rlgl batched draw operations internally so we have to flush the current batch before
*   doing our own OpenGL work (rlDrawRenderBatchActive())
*
*   The example also demonstrates how to get the current model view projection matrix of
*   raylib. That way raylib cameras and so on work as expected
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_DESKTOP_SDL)
    #if defined(GRAPHICS_API_OPENGL_ES2)
        #define GLAD_GLES2_IMPLEMENTATION
        #include "glad_gles2.h"       // Required for: OpenGL functionality
        #define glGenVertexArrays glGenVertexArraysOES
        #define glBindVertexArray glBindVertexArrayOES
        #define glDeleteVertexArrays glDeleteVertexArraysOES
        #define GLSL_VERSION            100
    #else
        #if defined(__APPLE__)
            #define GL_SILENCE_DEPRECATION // Silence Opengl API deprecation warnings
            #include <OpenGL/gl3.h>     // OpenGL 3 library for OSX
            #include <OpenGL/gl3ext.h>  // OpenGL 3 extensions library for OSX
        #else
            #include "glad.h"       // Required for: OpenGL functionality
        #endif
        #define GLSL_VERSION            330
    #endif
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#include "rlgl.h"           // Required for: rlDrawRenderBatchActive(), rlGetMatrixModelview(), rlGetMatrixProjection()
#include "raymath.h"        // Required for: MatrixMultiply(), MatrixToFloat()

#define MAX_PARTICLES       1000

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
// Particle type
typedef struct Particle {
    float x;
    float y;
    float period;
} Particle;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [others] example - OpenGL interoperatibility");

    Shader shader = LoadShader(TextFormat("resources/shaders/glsl%i/point_particle.vs", GLSL_VERSION),
                               TextFormat("resources/shaders/glsl%i/point_particle.fs", GLSL_VERSION));

    int currentTimeLoc = GetShaderLocation(shader, "currentTime");
    int colorLoc = GetShaderLocation(shader, "color");

    // Initialize the vertex buffer for the particles and assign each particle random values
    Particle particles[MAX_PARTICLES] = { 0 };

    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        particles[i].x = (float)GetRandomValue(20, screenWidth - 20);
        particles[i].y = (float)GetRandomValue(50, screenHeight - 20);

        // Give each particle a slightly different period. But don't spread it to much
        // This way the particles line up every so often and you get a glimps of what is going on
        particles[i].period = (float)GetRandomValue(10, 30)/10.0f;
    }

    // Create a plain OpenGL vertex buffer with the data and an vertex array object
    // that feeds the data from the buffer into the vertexPosition shader attribute
    GLuint vao = 0;
    GLuint vbo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES*sizeof(Particle), particles, GL_STATIC_DRAW);
        // Note: LoadShader() automatically fetches the attribute index of "vertexPosition" and saves it in shader.locs[SHADER_LOC_VERTEX_POSITION]
        glVertexAttribPointer(shader.locs[SHADER_LOC_VERTEX_POSITION], 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Allows the vertex shader to set the point size of each particle individually
    #ifndef GRAPHICS_API_OPENGL_ES2
    glEnable(GL_PROGRAM_POINT_SIZE);
    #endif

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(WHITE);

            DrawRectangle(10, 10, 210, 30, MAROON);
            DrawText(TextFormat("%zu particles in one vertex buffer", MAX_PARTICLES), 20, 20, 10, RAYWHITE);

            rlDrawRenderBatchActive();      // Draw iternal buffers data (previous draw calls)

            // Switch to plain OpenGL
            //------------------------------------------------------------------------------
            glUseProgram(shader.id);

                glUniform1f(currentTimeLoc, GetTime());

                Vector4 color = ColorNormalize((Color){ 255, 0, 0, 128 });
                glUniform4fv(colorLoc, 1, (float *)&color);

                // Get the current modelview and projection matrix so the particle system is displayed and transformed
                Matrix modelViewProjection = MatrixMultiply(rlGetMatrixModelview(), rlGetMatrixProjection());

                glUniformMatrix4fv(shader.locs[SHADER_LOC_MATRIX_MVP], 1, false, MatrixToFloat(modelViewProjection));

                glBindVertexArray(vao);
                    glDrawArrays(GL_POINTS, 0, MAX_PARTICLES);
                glBindVertexArray(0);

            glUseProgram(0);
            //------------------------------------------------------------------------------

            DrawFPS(screenWidth - 100, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    UnloadShader(shader);   // Unload shader

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
