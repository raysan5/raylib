/*******************************************************************************************
*
*   raylib [textures] example - Procedural images generation
*
*   Example originally created with raylib 1.8, last time updated with raylib 1.8
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2O17-2023 Wilhem Barbier (@nounoursheureux) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define NUM_TEXTURES  6      // Currently we have 7 generation algorithms

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - procedural images generation");

    Image verticalGradient = GenImageGradientV(screenWidth, screenHeight, RED, BLUE);
    Image horizontalGradient = GenImageGradientH(screenWidth, screenHeight, RED, BLUE);
    Image radialGradient = GenImageGradientRadial(screenWidth, screenHeight, 0.0f, WHITE, BLACK);
    Image checked = GenImageChecked(screenWidth, screenHeight, 32, 32, RED, BLUE);
    Image whiteNoise = GenImageWhiteNoise(screenWidth, screenHeight, 0.5f);
    Image cellular = GenImageCellular(screenWidth, screenHeight, 32);

    Texture2D textures[NUM_TEXTURES] = { 0 };

    textures[0] = LoadTextureFromImage(verticalGradient);
    textures[1] = LoadTextureFromImage(horizontalGradient);
    textures[2] = LoadTextureFromImage(radialGradient);
    textures[3] = LoadTextureFromImage(checked);
    textures[4] = LoadTextureFromImage(whiteNoise);
    textures[5] = LoadTextureFromImage(cellular);

    // Unload image data (CPU RAM)
    UnloadImage(verticalGradient);
    UnloadImage(horizontalGradient);
    UnloadImage(radialGradient);
    UnloadImage(checked);
    UnloadImage(whiteNoise);
    UnloadImage(cellular);

    int currentTexture = 0;

    SetTargetFPS(60);
    //---------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_RIGHT))
        {
            currentTexture = (currentTexture + 1)%NUM_TEXTURES; // Cycle between the textures
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawTexture(textures[currentTexture], 0, 0, WHITE);

            DrawRectangle(30, 400, 325, 30, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(30, 400, 325, 30, Fade(WHITE, 0.5f));
            DrawText("MOUSE LEFT BUTTON to CYCLE PROCEDURAL TEXTURES", 40, 410, 10, WHITE);

            switch(currentTexture)
            {
                case 0: DrawText("VERTICAL GRADIENT", 560, 10, 20, RAYWHITE); break;
                case 1: DrawText("HORIZONTAL GRADIENT", 540, 10, 20, RAYWHITE); break;
                case 2: DrawText("RADIAL GRADIENT", 580, 10, 20, LIGHTGRAY); break;
                case 3: DrawText("CHECKED", 680, 10, 20, RAYWHITE); break;
                case 4: DrawText("WHITE NOISE", 640, 10, 20, RED); break;
                case 5: DrawText("CELLULAR", 670, 10, 20, RAYWHITE); break;
                default: break;
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    // Unload textures data (GPU VRAM)
    for (int i = 0; i < NUM_TEXTURES; i++) UnloadTexture(textures[i]);

    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
