/*******************************************************************************************
*
*   raylib [shaders] example - Multiple sample2D with default batch system
*
*   NOTE: This example requires raylib OpenGL 3.3 or ES2 versions for shaders support,
*         OpenGL 1.1 does not support shaders, recompile raylib to OpenGL 3.3 version.
*
*   NOTE: Shaders used in this example are #version 330 (OpenGL 3.3), to test this example
*         on OpenGL ES 2.0 platforms (Android, Raspberry Pi, HTML5), use #version 100 shaders
*         raylib comes with shaders ready for both versions, check raylib/shaders install folder
*
*   This example has been created using raylib 3.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2020 Ramon Santamaria (@raysan5)
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

    InitWindow(screenWidth, screenHeight, "raylib - multiple sample2D");

    Image imRed = GenImageColor(800, 450, (Color){ 255, 0, 0, 255 });
    Texture texRed = LoadTextureFromImage(imRed);
    UnloadImage(imRed);

    Image imBlue = GenImageColor(800, 450, (Color){ 0, 0, 255, 255 });
    Texture texBlue = LoadTextureFromImage(imBlue);
    UnloadImage(imBlue);

    Shader shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/color_mix.fs", GLSL_VERSION));
    
    // Set an additional sampler2D, using another texture1
    // NOTE: Additional samplers are enabled for all batch calls
    SetShaderValueTexture(shader, GetShaderLocation(shader, "texture1"), texBlue);

    SetTargetFPS(60);                           // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())                // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // ...
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);

            BeginShaderMode(shader);
            
                // We are drawing texture using default sampler2D but
                // but additional texture units will be also enabled
                DrawTexture(texRed, 0, 0, WHITE);
                
            EndShaderMode();
            
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(shader);       // Unload shader
    UnloadTexture(texRed);      // Unload texture
    UnloadTexture(texBlue);     // Unload texture

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}