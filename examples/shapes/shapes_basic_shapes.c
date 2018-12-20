/*******************************************************************************************
*
*   raylib [shapes] example - Draw basic shapes 2d (rectangle, circle, line...)
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

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - basic shapes drawing");
    
    SetTargetFPS(60);
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

            ClearBackground(RL_RAYWHITE);

            DrawText("some basic shapes available on raylib", 20, 20, 20, RL_DARKGRAY);

            DrawCircle(screenWidth/4, 120, 35, RL_DARKBLUE);

            DrawRectangle(screenWidth/4*2 - 60, 100, 120, 60, RL_RED);
            DrawRectangleLines(screenWidth/4*2 - 40, 320, 80, 60, RL_ORANGE);  // NOTE: Uses QUADS internally, not lines
            DrawRectangleGradientH(screenWidth/4*2 - 90, 170, 180, 130, RL_MAROON, RL_GOLD);

            DrawTriangle((Vector2){screenWidth/4*3, 80},
                         (Vector2){screenWidth/4*3 - 60, 150},
                         (Vector2){screenWidth/4*3 + 60, 150}, RL_VIOLET);

            DrawPoly((Vector2){screenWidth/4*3, 320}, 6, 80, 0, RL_BROWN);

            DrawCircleGradient(screenWidth/4, 220, 60, RL_GREEN, RL_SKYBLUE);

            // NOTE: We draw all LINES based shapes together to optimize internal drawing,
            // this way, all LINES are rendered in a single draw pass
            DrawLine(18, 42, screenWidth - 18, 42, RL_BLACK);
            DrawCircleLines(screenWidth/4, 340, 80, RL_DARKBLUE);
            DrawTriangleLines((Vector2){screenWidth/4*3, 160},
                              (Vector2){screenWidth/4*3 - 20, 230},
                              (Vector2){screenWidth/4*3 + 20, 230}, RL_DARKBLUE);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}