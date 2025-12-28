/*******************************************************************************************
*
*   raylib [textures] example - sprite stacking
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.6-dev, last time updated with raylib 6.0
*
*   Example contributed by Robin (@RobinsAviary) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Redbooth model (c) 2017-2025 @kluchek under https://creativecommons.org/licenses/by/4.0/ https://github.com/kluchek/vox-models/
*   Copyright (c) 2025 Robin (@RobinsAviary)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"    // Required for: Clamp()

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - sprite stacking");

    Texture2D booth = LoadTexture("resources/booth.png");

    float stackScale = 3.0f; // Overall scale of the stacked sprite
    float stackSpacing = 2.0f; // Vertical spacing between each layer
    unsigned int stackCount = 122; // Number of layers, used for calculating the size of a single slice
    float rotationSpeed = 30.0f; // Stacked sprites rotation speed
    float rotation = 0.0f; // Current rotation of the stacked sprite
    const float speedChange = 0.25f; // Amount speed will change by when the user presses A/D

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Use mouse wheel to affect stack separation
        stackSpacing += GetMouseWheelMove()*0.1f;
        stackSpacing = Clamp(stackSpacing, 0.0f, 5.0f);

        // Add a positive/negative offset to spin right/left at different speeds
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) rotationSpeed -= speedChange;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) rotationSpeed += speedChange;

        rotation += rotationSpeed*GetFrameTime();
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Get the size of a single slice
            float frameWidth = (float)booth.width;
            float frameHeight = (float)booth.height/(float)stackCount;

            // Get the scaled resolution to draw at
            float scaledWidth = frameWidth*stackScale;
            float scaledHeight = frameHeight*stackScale;

            // Draw the stacked sprite, rotated to the correct angle, with an vertical offset applied based on its y location
            for (int i = stackCount - 1; i >= 0; i--)
            {
                // Center vertically
                Rectangle source = { 0.0f, (float)i*frameHeight, frameWidth, frameHeight };
                Rectangle dest = { screenWidth/2.0f, (screenHeight/2.0f) + (i*stackSpacing) - (stackSpacing*stackCount/2.0f), scaledWidth, scaledHeight };
                Vector2 origin = { scaledWidth/2.0f, scaledHeight/2.0f };

                DrawTexturePro(booth, source, dest, origin, rotation, WHITE);
            }

            DrawText("A/D to spin\nmouse wheel to change separation (aka 'angle')", 10, 10, 20, DARKGRAY);
            DrawText(TextFormat("current spacing: %.01f", stackSpacing), 10, 50, 20, DARKGRAY);
            DrawText(TextFormat("current speed: %.02f", rotationSpeed), 10, 70, 20, DARKGRAY);
            DrawText("redbooth model (c) kluchek under cc 4.0", 10, 420, 20, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(booth);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}