/*******************************************************************************************
*
*   raylib [textures] example - Shader texture drawing
*
*   This example illustrates how to draw on a blank texture using a shader
*
*   This example has been created using raylib 2.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2019 Michał Ciesielski and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - shader texture drawing");

    Image imBlank = GenImageColor(1024, 1024, BLANK);
    Texture2D texture = LoadTextureFromImage(imBlank);  // Load blank texture to fill on shader
    UnloadImage(imBlank);

    // NOTE: Using GLSL 330 shader version, on OpenGL ES 2.0 use GLSL 100 shader version
    Shader shader = LoadShader(0, "resources/shaders/glsl330/cubes_panning.fs");

    float time = 0.0f;
    int timeLoc = GetShaderLocation(shader, "uTime");
    SetShaderValue(shader, timeLoc, &time, UNIFORM_FLOAT);

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        time = GetTime();
        SetShaderValue(shader, timeLoc, &time, UNIFORM_FLOAT);
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginShaderMode(shader);    // Enable our custom shader for next shapes/textures drawings
                DrawTexture(texture, 0, 0, WHITE);  // Drawing BLANK texture, all magic happens on shader
            EndShaderMode();            // Disable our custom shader, return to default shader
            
            DrawText("BACKGROUND is PAINTED and ANIMATED on SHADER!", 10, 10, 20, MAROON);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    
    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(shader);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
