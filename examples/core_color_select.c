/*******************************************************************************************
*
*   raylib [core] example - Color selection by mouse (collision detection)
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
    int screenHeight = 400;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - color selection (collision detection)");

    Color colors[21] = { DARKGRAY, MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE, DARKBROWN,
                         GRAY, RED, GOLD, LIME, BLUE, VIOLET, BROWN, LIGHTGRAY, PINK, YELLOW,
                         GREEN, SKYBLUE, PURPLE, BEIGE };

    Rectangle colorsRecs[21];             // Rectangles array

    // Fills colorsRecs data (for every rectangle)
    for (int i = 0; i < 21; i++)
    {
        colorsRecs[i].x = 20 + 100*(i%7) + 10*(i%7);
        colorsRecs[i].y = 40 + 100*(i/7) + 10*(i/7);
        colorsRecs[i].width = 100;
        colorsRecs[i].height = 100;
    }

    bool selected[21] = { false };  // Selected rectangles indicator

    Vector2 mousePoint;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        mousePoint = GetMousePosition();

        for (int i = 0; i < 21; i++)    // Iterate along all the rectangles
        {
            if (CheckCollisionPointRec(mousePoint, colorsRecs[i]))
            {
                colors[i].a = 120;

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selected[i] = !selected[i];
            }
            else colors[i].a = 255;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            for (int i = 0; i < 21; i++)    // Draw all rectangles
            {
                DrawRectangleRec(colorsRecs[i], colors[i]);

                // Draw four rectangles around selected rectangle
                if (selected[i])
                {
                    DrawRectangle(colorsRecs[i].x, colorsRecs[i].y, 100, 10, RAYWHITE);        // Square top rectangle
                    DrawRectangle(colorsRecs[i].x, colorsRecs[i].y, 10, 100, RAYWHITE);        // Square left rectangle
                    DrawRectangle(colorsRecs[i].x + 90, colorsRecs[i].y, 10, 100, RAYWHITE);   // Square right rectangle
                    DrawRectangle(colorsRecs[i].x, colorsRecs[i].y + 90, 100, 10, RAYWHITE);   // Square bottom rectangle
                }
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}