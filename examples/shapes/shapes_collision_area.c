/*******************************************************************************************
*
*   raylib [shapes] example - collision area
*
*   Example originally created with raylib 2.5, last time updated with raylib 2.5
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2013-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>     // Required for: abs()

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //---------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - collision area");


    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //----------------------------------------------------------


    Vector2 p0 = { 0,0 };
    Vector2 p1 = { 300,300 };

 
    float r = 20.0f;

    Color col = RED;



    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        Vector2 c = { GetMouseX(),GetMouseY() };
        //-----------------------------------------------------
        if (CheckCollisionCircleLine(c, r, p0, p1))
        {
            col = BLUE;
        }
        else
        {
            col = RED;
        }

        // Draw
        //-----------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            DrawLine(p0.x, p0.y, p1.x, p1.y, BLACK);
            DrawCircle(c.x, c.y, r, col);
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
