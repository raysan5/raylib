/*******************************************************************************************
*
*   raylib [core] example - Gamepad information
*
*   NOTE: This example requires a Gamepad connected to the system
*         Check raylib.h for buttons configuration
*
*   Example originally created with raylib 4.6, last time updated with raylib 4.6
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2013-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);  // Set MSAA 4X hint before windows creation

    InitWindow(screenWidth, screenHeight, "raylib [core] example - gamepad information");

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
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

            for (int i = 0, y = 10; i < 4; i++)     // MAX_GAMEPADS = 4
            {
                if (IsGamepadAvailable(i))
                {
                    DrawText(TextFormat("Gamepad name: %s", GetGamepadName(i)), 10, y, 20, BLACK);
                    y += 30;
                    DrawText(TextFormat("\tAxis count:   %d", GetGamepadAxisCount(i)), 10, y, 20, BLACK);
                    y += 30;

                    for (int axis = 0; axis < GetGamepadAxisCount(i); axis++)
                    {
                        DrawText(TextFormat("\tAxis %d = %f", axis, GetGamepadAxisMovement(i, axis)), 10, y, 20, BLACK);
                        y += 30;
                    }

                    for (int button = 0; button < 32; button++)
                    {
                        DrawText(TextFormat("\tButton %d = %d", button, IsGamepadButtonDown(i, button)), 10, y, 20, BLACK);
                        y += 30;
                    }
                }
            }

            DrawFPS(GetScreenWidth() - 100, 100);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
}