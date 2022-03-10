/*******************************************************************************************
*
*   raylib [core] example - Input multitouch
*
*   This example has been created using raylib 2.1 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Berni (@Berni8k) and reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2019 Berni (@Berni8k) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define MAX_TOUCH_POINTS 10

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - input multitouch");

    Vector2 touchPositions[MAX_TOUCH_POINTS] = { 0 };

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //---------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Get multiple touchpoints
        for (int i = 0; i < MAX_TOUCH_POINTS; ++i) touchPositions[i] = GetTouchPosition(i);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            for (int i = 0; i < MAX_TOUCH_POINTS; ++i)
            {
                // Make sure point is not (0, 0) as this means there is no touch for it
                if ((touchPositions[i].x > 0) && (touchPositions[i].y > 0))
                {
                    // Draw circle and touch index number
                    DrawCircleV(touchPositions[i], 34, ORANGE);
                    DrawText(TextFormat("%d", i), (int)touchPositions[i].x - 10, (int)touchPositions[i].y - 70, 40, BLACK);
                }
            }

            DrawText("touch the screen at multiple locations to get multiple balls", 10, 10, 20, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}