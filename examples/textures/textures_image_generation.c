/*******************************************************************************************
*
*   raylib [textures] example - Procedural images generation
*
*   This example has been created using raylib 1.8 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2O17 Wilhem Barbier (@nounoursheureux)
*
********************************************************************************************/

#include "raylib.h"

#define TEXTURES_NUM 7      // for now we have 7 generation algorithms

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - procedural images generation");

    Image verticalGradient = GenImageGradientV(screenWidth, screenHeight, RED, BLUE);
    Image horizontalGradient = GenImageGradientH(screenWidth, screenHeight, RED, BLUE);
    Image radialGradient = GenImageGradientRadial(screenWidth, screenHeight, 0.f, WHITE, BLACK);
    Image checked = GenImageChecked(screenWidth, screenHeight, 32, 32, RED, BLUE);
    Image whiteNoise = GenImageWhiteNoise(screenWidth, screenHeight, 0.5f);
    Image perlinNoise = GenImagePerlinNoise(screenWidth, screenHeight, 8.f);
    Image cellular = GenImageCellular(screenWidth, screenHeight, 32);

    Texture2D textures[TEXTURES_NUM];
    textures[0] = LoadTextureFromImage(verticalGradient);
    textures[1] = LoadTextureFromImage(horizontalGradient);
    textures[2] = LoadTextureFromImage(radialGradient);
    textures[3] = LoadTextureFromImage(checked);
    textures[4] = LoadTextureFromImage(whiteNoise);
    textures[5] = LoadTextureFromImage(perlinNoise);
    textures[6] = LoadTextureFromImage(cellular);

    int currentTexture = 0;
    
    SetTargetFPS(60);
    //---------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            currentTexture = (currentTexture + 1) % TEXTURES_NUM; // cycle between the 5 textures
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            DrawTexture(textures[currentTexture], 0, 0, WHITE);
            
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    
    // Unload image data (CPU RAM)
    UnloadImage(verticalGradient);
    UnloadImage(horizontalGradient);
    UnloadImage(radialGradient);
    UnloadImage(checked);
    UnloadImage(whiteNoise);
    UnloadImage(perlinNoise);
    UnloadImage(cellular);
    
    // Unload textures data (GPU VRAM)
    for (int i = 0; i < TEXTURES_NUM; i++) UnloadTexture(textures[i]);
    
    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}
