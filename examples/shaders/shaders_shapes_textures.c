/*******************************************************************************************
*
*   raylib [shaders] example - Apply a shader to some shape or texture
*
*   NOTE: This example requires raylib OpenGL 3.3 or ES2 versions for shaders support,
*         OpenGL 1.1 does not support shaders, recompile raylib to OpenGL 3.3 version.
*
*   NOTE: Shaders used in this example are #version 330 (OpenGL 3.3), to test this example
*         on OpenGL ES 2.0 platforms (Android, Raspberry Pi, HTML5), use #version 100 shaders
*         raylib comes with shaders ready for both versions, check raylib/shaders install folder
*
*   Example originally created with raylib 1.7, last time updated with raylib 3.7
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2015-2023 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - shapes and texture shaders");

    Texture2D fudesumi = LoadTexture("resources/fudesumi.png");

    // Load shader to be used on some parts drawing
    // NOTE 1: Using GLSL 330 shader version, on OpenGL ES 2.0 use GLSL 100 shader version
    // NOTE 2: Defining 0 (NULL) for vertex shader forces usage of internal default vertex shader
    Shader shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/grayscale.fs", GLSL_VERSION));

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Start drawing with default shader

            DrawText("USING DEFAULT SHADER", 20, 40, 10, RED);

            DrawCircle(80, 120, 35, DARKBLUE);
            DrawCircleGradient(80, 220, 60, GREEN, SKYBLUE);
            DrawCircleLines(80, 340, 80, DARKBLUE);


            // Activate our custom shader to be applied on next shapes/textures drawings
            BeginShaderMode(shader);

                DrawText("USING CUSTOM SHADER", 190, 40, 10, RED);

                DrawRectangle(250 - 60, 90, 120, 60, RED);
                DrawRectangleGradientH(250 - 90, 170, 180, 130, MAROON, GOLD);
                DrawRectangleLines(250 - 40, 320, 80, 60, ORANGE);

            // Activate our default shader for next drawings
            EndShaderMode();

            DrawText("USING DEFAULT SHADER", 370, 40, 10, RED);

            DrawTriangle((Vector2){430, 80},
                         (Vector2){430 - 60, 150},
                         (Vector2){430 + 60, 150}, VIOLET);

            DrawTriangleLines((Vector2){430, 160},
                              (Vector2){430 - 20, 230},
                              (Vector2){430 + 20, 230}, DARKBLUE);

            DrawPoly((Vector2){430, 320}, 6, 80, 0, BROWN);

            // Activate our custom shader to be applied on next shapes/textures drawings
            BeginShaderMode(shader);

                DrawTexture(fudesumi, 500, -30, WHITE);    // Using custom shader

            // Activate our default shader for next drawings
            EndShaderMode();

            DrawText("(c) Fudesumi sprite by Eiden Marsal", 380, screenHeight - 20, 10, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(shader);       // Unload shader
    UnloadTexture(fudesumi);    // Unload texture

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}