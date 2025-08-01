/*******************************************************************************************
*
*   raylib [shapes] example - bouncing ball
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 2.5, last time updated with raylib 2.5
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2013-2025 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //---------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - bouncing ball");

    Vector2 ballPosition = { screenWidth/2.0f, screenHeight/2.0f };
    Vector2 ballSpeed = { 5.0f, 4.0f };
    int ballRadius = 20;

    bool pause = 0;
    int framesCounter = 0;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //----------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //-----------------------------------------------------
	if (IsWindowResized())
	{
		screenWidth  = GetScreenWidth();
		screenHeight = GetScreenHeight();
		ballPosition.x = screenWidth/2.0f;
		ballPosition.y = screenHeight/2.0f;
	}
        if (IsKeyPressed(KEY_SPACE)) pause = !pause;

        if (!pause)
        {
            ballPosition.x += ballSpeed.x;
            ballPosition.y += ballSpeed.y;

            // Check walls collision for bouncing
            if ((ballPosition.x >= (screenWidth - ballRadius)) || (ballPosition.x <= ballRadius)) ballSpeed.x *= -1.0f;
            if ((ballPosition.y >= (screenHeight - ballRadius)) || (ballPosition.y <= ballRadius)) ballSpeed.y *= -1.0f;
        }
        else framesCounter++;
        //-----------------------------------------------------

        // Draw
        //-----------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawCircleV(ballPosition, (float)ballRadius, MAROON);
            DrawText("PRESS SPACE to PAUSE BALL MOVEMENT", 10, screenHeight - 25, 20, LIGHTGRAY);

            // On pause, we draw a blinking message
            if (pause && ((framesCounter/30)%2)) DrawText("PAUSED", (screenWidth - 100)/2, (screenHeight- 50)/2, 30, GRAY);

            DrawFPS(10, 10);

        EndDrawing();
        //-----------------------------------------------------
    }

    // De-Initialization
    //---------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //----------------------------------------------------------

    return 0;
}
