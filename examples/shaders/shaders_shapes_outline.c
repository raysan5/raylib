/*******************************************************************************************
*
*   raylib [shaders] example - Apply an shdrOutline to a texture
*
*   NOTE: This example requires raylib OpenGL 3.3 or ES2 versions for shaders support,
*         OpenGL 1.1 does not support shaders, recompile raylib to OpenGL 3.3 version.
*
*   NOTE: Shaders used in this example are #version 330 (OpenGL 3.3).
*
*   This example has been created using raylib 3.8 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Samuel Skiff (@GoldenThumbs)
*
*   Copyright (c) 2021 Samuel SKiff (@GoldenThumbs) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - Apply an outline to a texture");

    Texture2D egg = LoadTexture("resources/egg.png");
    Texture2D torus = LoadTexture("resources/torus.png");
    Shader shdrOutline = LoadShader(0, TextFormat("resources/shaders/glsl%i/outline.fs", GLSL_VERSION));

    float outlineScale = 16.0f;
    float textureScale[2] = { 16.0f*4, 16.0f*4 };
    SetShaderValue(shdrOutline, GetShaderLocation(shdrOutline, "texScale"), textureScale, SHADER_UNIFORM_VEC2);

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

            BeginShaderMode(shdrOutline);
                DrawTextureEx(egg, (Vector2){ 0, 230 }, 0.0, outlineScale, WHITE);
                DrawTextureEx(torus, (Vector2){ 544, 230 }, 0.0, outlineScale, WHITE);
            EndShaderMode();

            DrawText("Shader-based outlines for textures", 190, 200, 20, LIGHTGRAY);

            DrawFPS(710, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(egg);
    UnloadTexture(torus);
    UnloadShader(shdrOutline);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}