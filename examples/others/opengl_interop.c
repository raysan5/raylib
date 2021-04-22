/*******************************************************************************************
*
*   raylib [shaders] example - OpenGL point particle system
*
*   This example has been created using raylib 2ad3eb1 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Stephan Soller (@arkanis -  http://arkanis.de/)
*   and reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2021 Stephan Soller (@arkanis) and Ramon Santamaria (@raysan5)
*
********************************************************************************************
*
*   Mixes raylib and plain OpenGL code to draw a GL_POINTS based particle system. The
*   primary point is to demonstrate raylib and OpenGL interop.
*
*   rlgl batched draw operations internally so we have to flush the current batch before
*   doing our own OpenGL work (rlDrawRenderBatchActive()).
*
*   The example also demonstrates how to get the current model view projection matrix of
*   raylib. That way raylib cameras and so on work as expected.
*
********************************************************************************************/

#include "raylib.h"
#include "rlgl.h"
#include "glad.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib - point particles");

    Shader shader = LoadShader(
        TextFormat("resources/shaders/glsl%i/point_particle.vs", GLSL_VERSION),
        TextFormat("resources/shaders/glsl%i/point_particle.fs", GLSL_VERSION));
    int currentTimeLoc = GetShaderLocation(shader, "currentTime");
    int colorLoc = GetShaderLocation(shader, "color");

    // Initialize the vertex buffer for the particles and assign each particle random values
    struct { float x, y, period; } particles[10000];
    const size_t particleCount = sizeof(particles) / sizeof(particles[0]);
    for (size_t i = 0; i < particleCount; i++)
    {
        particles[i].x = GetRandomValue(20, screenWidth - 20);
        particles[i].y = GetRandomValue(50, screenHeight - 20);
        // Give each particle a slightly different period. But don't spread it to much. This way the particles line up
        // every so often and you get a glimps of what is going on.
        particles[i].period = GetRandomValue(10, 30) / 10.0f;
    }

    // Create a plain OpenGL vertex buffer with the data and an vertex array object that feeds the data from the buffer
    // into the vertexPosition shader attribute.
    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(particles), particles, GL_STATIC_DRAW);
        // Note: LoadShader() automatically fetches the attribute index of "vertexPosition" and saves it in shader.locs[SHADER_LOC_VERTEX_POSITION]
        glVertexAttribPointer(shader.locs[SHADER_LOC_VERTEX_POSITION], 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Allows the vertex shader to set the point size of each particle individually
    glEnable(GL_PROGRAM_POINT_SIZE);

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
            DrawText(TextFormat("%zu particles in one vertex buffer", particleCount), 20, 20, 10, RAYWHITE);

            // Switch to plain OpenGL
            //------------------------------------------------------------------------------
            // rlglDraw() in raylib 3.5
            rlDrawRenderBatchActive();
            glUseProgram(shader.id);
                glUniform1f(currentTimeLoc, GetTime());

                Vector4 color = ColorNormalize((Color){ 255, 0, 0, 128 });
                glUniform4fv(colorLoc, 1, (float*)&color);

                // The the current model view projection matrix so the particle system is displayed and transformed
                // (e.g. by cameras) just like everything else.
                // GetMatrixModelview() and GetMatrixProjection() in raylib 3.5
                Matrix modelViewProjection = MatrixMultiply(rlGetMatrixModelview(), rlGetMatrixProjection());
                glUniformMatrix4fv(shader.locs[SHADER_LOC_MATRIX_MVP], 1, false, MatrixToFloat(modelViewProjection));

                glBindVertexArray(vao);
                    glDrawArrays(GL_POINTS, 0, particleCount);
                glBindVertexArray(0);
            glUseProgram(0);

            // And back to raylib again
            //------------------------------------------------------------------------------
            DrawFPS(screenWidth - 100, 10);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    UnloadShader(shader);
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}