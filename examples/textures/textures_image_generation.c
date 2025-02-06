/*******************************************************************************************
*
*   raylib [textures] example - Procedural images generation
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 1.8, last time updated with raylib 1.8
*
*   Example contributed by Wilhem Barbier (@nounoursheureux) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2017-2025 Wilhem Barbier (@nounoursheureux) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define NUM_TEXTURES  9      // Currently we have 8 generation algorithms but some have multiple purposes (Linear and Square Gradients)

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

    Image verticalGradient = GenImageGradientLinear(screenWidth, screenHeight, 0, RED, BLUE);
    Image horizontalGradient = GenImageGradientLinear(screenWidth, screenHeight, 90, RED, BLUE);
    Image diagonalGradient = GenImageGradientLinear(screenWidth, screenHeight, 45, RED, BLUE);
    Image radialGradient = GenImageGradientRadial(screenWidth, screenHeight, 0.0f, WHITE, BLACK);
    Image squareGradient = GenImageGradientSquare(screenWidth, screenHeight, 0.0f, WHITE, BLACK);
    Image checked = GenImageChecked(screenWidth, screenHeight, 32, 32, RED, BLUE);
    Image whiteNoise = GenImageWhiteNoise(screenWidth, screenHeight, 0.5f);
    Image perlinNoise = GenImagePerlinNoise(screenWidth, screenHeight, 50, 50, 4.0f);
    Image cellular = GenImageCellular(screenWidth, screenHeight, 32);

    Texture2D textures[NUM_TEXTURES] = { 0 };

    textures[0] = LoadTextureFromImage(verticalGradient);
    textures[1] = LoadTextureFromImage(horizontalGradient);
    textures[2] = LoadTextureFromImage(diagonalGradient);
    textures[3] = LoadTextureFromImage(radialGradient);
    textures[4] = LoadTextureFromImage(squareGradient);
    textures[5] = LoadTextureFromImage(checked);
    textures[6] = LoadTextureFromImage(whiteNoise);
    textures[7] = LoadTextureFromImage(perlinNoise);
    textures[8] = LoadTextureFromImage(cellular);

    // Unload image data (CPU RAM)
    UnloadImage(verticalGradient);
    UnloadImage(horizontalGradient);
    UnloadImage(diagonalGradient);
    UnloadImage(radialGradient);
    UnloadImage(squareGradient);
    UnloadImage(checked);
    UnloadImage(whiteNoise);
    UnloadImage(perlinNoise);
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

            switch (currentTexture)
            {
                case 0: DrawText("VERTICAL GRADIENT", 560, 10, 20, RAYWHITE); break;
                case 1: DrawText("HORIZONTAL GRADIENT", 540, 10, 20, RAYWHITE); break;
                case 2: DrawText("DIAGONAL GRADIENT", 540, 10, 20, RAYWHITE); break;
                case 3: DrawText("RADIAL GRADIENT", 580, 10, 20, LIGHTGRAY); break;
                case 4: DrawText("SQUARE GRADIENT", 580, 10, 20, LIGHTGRAY); break;
                case 5: DrawText("CHECKED", 680, 10, 20, RAYWHITE); break;
                case 6: DrawText("WHITE NOISE", 640, 10, 20, RED); break;
                case 7: DrawText("PERLIN NOISE", 640, 10, 20, RED); break;
                case 8: DrawText("CELLULAR", 670, 10, 20, RAYWHITE); break;
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
