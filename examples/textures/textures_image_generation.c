/*******************************************************************************************
*
*   raylib [textures] example - Procedural images generation
*
*   This example has been created using raylib 1.7 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2O17 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define TEXTURES_NUM 5 // for now we have 5 generation algorithms

int main()
{
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - procedural images generation");

    Image verticalGradient = GenImageGradientV(screenWidth, screenHeight, RED, BLUE);
    Image horizontalGradient = GenImageGradientH(screenWidth, screenHeight, RED, BLUE);
    Image checked = GenImageChecked(screenWidth, screenHeight, 32, 32, RED, BLUE);
    Image whiteNoise = GenImageWhiteNoise(screenWidth, screenHeight, 0.5f);
    Image cellular = GenImageCellular(screenWidth, screenHeight, 32);

    Texture2D textures[TEXTURES_NUM];
    textures[0] = LoadTextureFromImage(verticalGradient);
    textures[1] = LoadTextureFromImage(horizontalGradient);
    textures[2] = LoadTextureFromImage(checked);
    textures[3] = LoadTextureFromImage(whiteNoise);
    textures[4] = LoadTextureFromImage(cellular);

    int currentTexture = 0;

    while (!WindowShouldClose())
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            currentTexture = (currentTexture + 1) % TEXTURES_NUM; // cycle between the 5 textures
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawTexture(textures[currentTexture], 0, 0, WHITE);
        EndDrawing();
    }

    for (int i = 0; i < TEXTURES_NUM; i++) // unload the textures
    {
        UnloadTexture(textures[i]);
    }

    CloseWindow();
}
