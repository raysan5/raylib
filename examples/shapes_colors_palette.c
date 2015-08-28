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

            DrawText("raylib color palette", 28, 42, 20, BLACK);

            DrawRectangle(26, 80, 100, 100, DARKGRAY);
            DrawRectangle(26, 188, 100, 100, GRAY);
            DrawRectangle(26, 296, 100, 100, LIGHTGRAY);
            DrawRectangle(134, 80, 100, 100, MAROON);
            DrawRectangle(134, 188, 100, 100, RED);
            DrawRectangle(134, 296, 100, 100, PINK);
            DrawRectangle(242, 80, 100, 100, ORANGE);
            DrawRectangle(242, 188, 100, 100, GOLD);
            DrawRectangle(242, 296, 100, 100, YELLOW);
            DrawRectangle(350, 80, 100, 100, DARKGREEN);
            DrawRectangle(350, 188, 100, 100, LIME);
            DrawRectangle(350, 296, 100, 100, GREEN);
            DrawRectangle(458, 80, 100, 100, DARKBLUE);
            DrawRectangle(458, 188, 100, 100, BLUE);
            DrawRectangle(458, 296, 100, 100, SKYBLUE);
            DrawRectangle(566, 80, 100, 100, DARKPURPLE);
            DrawRectangle(566, 188, 100, 100, VIOLET);
            DrawRectangle(566, 296, 100, 100, PURPLE);
            DrawRectangle(674, 80, 100, 100, DARKBROWN);
            DrawRectangle(674, 188, 100, 100, BROWN);
            DrawRectangle(674, 296, 100, 100, BEIGE);


            DrawText("DARKGRAY", 65, 166, 10, BLACK);
            DrawText("GRAY", 93, 274, 10, BLACK);
            DrawText("LIGHTGRAY", 61, 382, 10, BLACK);
            DrawText("MAROON", 186, 166, 10, BLACK);
            DrawText("RED", 208, 274, 10, BLACK);
            DrawText("PINK", 204, 382, 10, BLACK);
            DrawText("ORANGE", 295, 166, 10, BLACK);
            DrawText("GOLD", 310, 274, 10, BLACK);
            DrawText("YELLOW", 300, 382, 10, BLACK);
            DrawText("DARKGREEN", 382, 166, 10, BLACK);
            DrawText("LIME", 420, 274, 10, BLACK);
            DrawText("GREEN", 410, 382, 10, BLACK);
            DrawText("DARKBLUE", 498, 166, 10, BLACK);
            DrawText("BLUE", 526, 274, 10, BLACK);
            DrawText("SKYBLUE", 505, 382, 10, BLACK);
            DrawText("DARKPURPLE", 592, 166, 10, BLACK);
            DrawText("VIOLET", 621, 274, 10, BLACK);
            DrawText("PURPLE", 620, 382, 10, BLACK);
            DrawText("DARKBROWN", 705, 166, 10, BLACK);
            DrawText("BROWN", 733, 274, 10, BLACK);
            DrawText("BEIGE", 737, 382, 10, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}