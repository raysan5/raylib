/*******************************************************************************************
*
*   raylib [core] example - Color selection by mouse (collision detection) (adapted for HTML5 platform)
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

Color *colors;

Rectangle colorsRecs[21];             // Rectangles array

bool selected[21] = { false };  // Selected rectangles indicator

Vector2 mousePoint;

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
    InitWindow(screenWidth, screenHeight, "raylib [core] example - color selection (collision detection)");

    Color tempColors[21] = { DARKGRAY, MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE, DARKBROWN,
                         GRAY, RED, GOLD, LIME, BLUE, VIOLET, BROWN, LIGHTGRAY, PINK, YELLOW,
                         GREEN, SKYBLUE, PURPLE, BEIGE };
                         
    colors = tempColors;
    
    // Fills colorsRecs data (for every rectangle)
    for (int i = 0; i < 21; i++)
    {
        colorsRecs[i].x = 20 + 100*(i%7) + 10*(i%7);
        colorsRecs[i].y = 40 + 100*(i/7) + 10*(i/7);
        colorsRecs[i].width = 100;
        colorsRecs[i].height = 100;
    }
    
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