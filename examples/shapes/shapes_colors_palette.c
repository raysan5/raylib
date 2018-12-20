/*******************************************************************************************
*
*   raylib [shapes] example - Draw raylib custom color palette
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

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - raylib color palette");
    
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

            DrawText("raylib color palette", 28, 42, 20, RL_BLACK);

            DrawRectangle(26, 80, 100, 100, RL_DARKGRAY);
            DrawRectangle(26, 188, 100, 100, RL_GRAY);
            DrawRectangle(26, 296, 100, 100, RL_LIGHTGRAY);
            DrawRectangle(134, 80, 100, 100, RL_MAROON);
            DrawRectangle(134, 188, 100, 100, RL_RED);
            DrawRectangle(134, 296, 100, 100, RL_PINK);
            DrawRectangle(242, 80, 100, 100, RL_ORANGE);
            DrawRectangle(242, 188, 100, 100, RL_GOLD);
            DrawRectangle(242, 296, 100, 100, RL_YELLOW);
            DrawRectangle(350, 80, 100, 100, RL_DARKGREEN);
            DrawRectangle(350, 188, 100, 100, RL_LIME);
            DrawRectangle(350, 296, 100, 100, RL_GREEN);
            DrawRectangle(458, 80, 100, 100, RL_DARKBLUE);
            DrawRectangle(458, 188, 100, 100, RL_BLUE);
            DrawRectangle(458, 296, 100, 100, RL_SKYBLUE);
            DrawRectangle(566, 80, 100, 100, RL_DARKPURPLE);
            DrawRectangle(566, 188, 100, 100, RL_VIOLET);
            DrawRectangle(566, 296, 100, 100, RL_PURPLE);
            DrawRectangle(674, 80, 100, 100, RL_DARKBROWN);
            DrawRectangle(674, 188, 100, 100, RL_BROWN);
            DrawRectangle(674, 296, 100, 100, RL_BEIGE);


            DrawText("RL_DARKGRAY", 65, 166, 10, RL_BLACK);
            DrawText("RL_GRAY", 93, 274, 10, RL_BLACK);
            DrawText("RL_LIGHTGRAY", 61, 382, 10, RL_BLACK);
            DrawText("RL_MAROON", 186, 166, 10, RL_BLACK);
            DrawText("RL_RED", 208, 274, 10, RL_BLACK);
            DrawText("RL_PINK", 204, 382, 10, RL_BLACK);
            DrawText("RL_ORANGE", 295, 166, 10, RL_BLACK);
            DrawText("RL_GOLD", 310, 274, 10, RL_BLACK);
            DrawText("RL_YELLOW", 300, 382, 10, RL_BLACK);
            DrawText("RL_DARKGREEN", 382, 166, 10, RL_BLACK);
            DrawText("RL_LIME", 420, 274, 10, RL_BLACK);
            DrawText("RL_GREEN", 410, 382, 10, RL_BLACK);
            DrawText("RL_DARKBLUE", 498, 166, 10, RL_BLACK);
            DrawText("RL_BLUE", 526, 274, 10, RL_BLACK);
            DrawText("RL_SKYBLUE", 505, 382, 10, RL_BLACK);
            DrawText("RL_DARKPURPLE", 592, 166, 10, RL_BLACK);
            DrawText("RL_VIOLET", 621, 274, 10, RL_BLACK);
            DrawText("RL_PURPLE", 620, 382, 10, RL_BLACK);
            DrawText("RL_DARKBROWN", 705, 166, 10, RL_BLACK);
            DrawText("RL_BROWN", 733, 274, 10, RL_BLACK);
            DrawText("RL_BEIGE", 737, 382, 10, RL_BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}