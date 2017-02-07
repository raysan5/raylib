/*******************************************************************************************
*
*   raylib [core] example - Gamepad input (adapted for HTML5 platform)
*
*   NOTE: This example requires a Gamepad connected to the system
*         raylib is configured to work with Xbox 360 gamepad, check raylib.h for buttons configuration
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
int screenWidth = 800;
int screenHeight = 450;

Vector2 ballPosition;
Vector2 gamepadMovement = { 0.0f, 0.0f };


//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void);     // Update and Draw one frame

//----------------------------------------------------------------------------------
// Main Enry Point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib [core] example - gamepad input");
    
    ballPosition = (Vector2){ (float)screenWidth/2, (float)screenHeight/2 };
    
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void)
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