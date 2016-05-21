/*******************************************************************************************
*
*   raylib [core] example - Gamepad input
*
*   NOTE: This example requires a Gamepad connected to the system
*         raylib is configured to work with Xbox 360 gamepad, check raylib.h for buttons configuration
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - gamepad input");

    Vector2 ballPosition = { (float)screenWidth/2, (float)screenHeight/2 };
    Vector2 gamepadMovement = { 0.0f, 0.0f };

    SetTargetFPS(60);               // Set target frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsGamepadAvailable(GAMEPAD_PLAYER1))
        {
            gamepadMovement.x = GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_XBOX_AXIS_LEFT_X);
            gamepadMovement.y = GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_XBOX_AXIS_LEFT_Y);

            ballPosition.x += gamepadMovement.x;
            ballPosition.y -= gamepadMovement.y;

            if (IsGamepadButtonPressed(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_A))
            {
                ballPosition.x = (float)screenWidth/2;
                ballPosition.y = (float)screenHeight/2;
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("move the ball with gamepad", 10, 10, 20, DARKGRAY);

            DrawCircleV(ballPosition, 50, MAROON);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}