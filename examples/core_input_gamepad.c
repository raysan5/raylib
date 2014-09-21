/*******************************************************************************************
*
*   raylib example 03c - Gamepad input
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013 Ramon Santamaria (Ray San - raysan@raysanweb.com)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;
    
    Vector2 ballPosition = { screenWidth/2, screenHeight/2 };
    Vector2 gamepadMove = { 0, 0 };
    
    InitWindow(screenWidth, screenHeight, "raylib example 01 - gamepad input");
    
    SetTargetFPS(60);       // Set target frames-per-second
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsGamepadAvailable(GAMEPAD_PLAYER1))
        {
            gamepadMove = GetGamepadMovement(GAMEPAD_PLAYER1);
            
            ballPosition.x += gamepadMove.x;
            ballPosition.y -= gamepadMove.y;
        
            if (IsGamepadButtonPressed(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_A))
            {
                ballPosition.x = screenWidth/2;
                ballPosition.y = screenHeight/2;
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
