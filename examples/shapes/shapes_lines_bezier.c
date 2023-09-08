/*******************************************************************************************
*
*   raylib [shapes] example - Cubic-bezier lines
*
*   Example originally created with raylib 1.7, last time updated with raylib 1.7
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2017-2023 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - cubic-bezier lines");

    Vector2 start = { 0, 0 };
    Vector2 end = { (float)screenWidth, (float)screenHeight };
    
    Vector2 startControl = { 100, 0 };
    Vector2 endControl = { GetScreenWidth() - 100, GetScreenHeight() };

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_LEFT_CONTROL))
        {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) startControl = GetMousePosition();
            else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) endControl = GetMousePosition();
        }
        else
        {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) start = GetMousePosition();
            else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) end = GetMousePosition();
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("USE MOUSE LEFT-RIGHT CLICK to DEFINE LINE START and END POINTS", 15, 20, 20, GRAY);

            //DrawLineBezier(start, end, 2.0f, RED);
            
            DrawLineBezierCubic(start, end, startControl, endControl, 2.0f, RED);
            
            DrawLineEx(start, startControl, 1.0, LIGHTGRAY);
            DrawLineEx(end, endControl, 1.0, LIGHTGRAY);
            DrawCircleV(startControl, 10, RED);
            DrawCircleV(endControl, 10, RED);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
