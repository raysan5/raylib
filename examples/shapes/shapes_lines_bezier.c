/*******************************************************************************************
*
*   raylib [shapes] example - Cubic-bezier lines
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 1.7, last time updated with raylib 1.7
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2017-2025 Ramon Santamaria (@raysan5)
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

    Vector2 startPoint = { 30, 30 };
    Vector2 endPoint = { (float)screenWidth - 30, (float)screenHeight - 30 };
    bool moveStartPoint = false;
    bool moveEndPoint = false;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        Vector2 mouse = GetMousePosition();

        if (CheckCollisionPointCircle(mouse, startPoint, 10.0f) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) moveStartPoint = true;
        else if (CheckCollisionPointCircle(mouse, endPoint, 10.0f) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) moveEndPoint = true;

        if (moveStartPoint)
        {
            startPoint = mouse;
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) moveStartPoint = false;
        }

        if (moveEndPoint)
        {
            endPoint = mouse;
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) moveEndPoint = false;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("MOVE START-END POINTS WITH MOUSE", 15, 20, 20, GRAY);

            // Draw line Cubic Bezier, in-out interpolation (easing), no control points
            DrawLineBezier(startPoint, endPoint, 4.0f, BLUE);
            
            // Draw start-end spline circles with some details
            DrawCircleV(startPoint, CheckCollisionPointCircle(mouse, startPoint, 10.0f)? 14.0f : 8.0f, moveStartPoint? RED : BLUE);
            DrawCircleV(endPoint, CheckCollisionPointCircle(mouse, endPoint, 10.0f)? 14.0f : 8.0f, moveEndPoint? RED : BLUE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
