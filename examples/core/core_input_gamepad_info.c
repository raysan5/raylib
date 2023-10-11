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
*   Copyright (c) 2013-2023 Ramon Santamaria (@raysan5)
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

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        int y = 10;

        BeginDrawing();

        ClearBackground(RAYWHITE);

        for (int i = 0; i < 4; i++)     // by default rcore.h has a MAX_GAMEPADS of 4 so mimmic that here.
        {
            if (IsGamepadAvailable(i))
            {
                DrawText(TextFormat("Gamepad:\n\tName: %s\n\tAxes: %d", GetGamepadName(i), GetGamepadAxisCount(i)), 10, y, 20, BLACK);
                y += 40;
            }
        }

        EndDrawing();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
}