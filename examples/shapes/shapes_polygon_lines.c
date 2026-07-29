/*******************************************************************************************
*
*   raylib [shapes] example - polygon lines
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 6.1, last time updated with raylib 6.1
*
*   Example contributed by Matthew Roush (@MatthewRoush) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026 Matthew Roush (@MatthewRoush)
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

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - polygon lines");

    int thick = 2;
    float rotation = 0.0f;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_UP))   thick = thick + 1;
        if (IsKeyPressed(KEY_DOWN)) thick = thick - 1;

        thick = (thick < 1) ? 1 : (thick > 60) ? 60 : thick;

        rotation += (360.0f/10.0f)*GetFrameTime();
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText(TextFormat("thick = %d", thick), 10, 280, 20, LIME);

            // The square and circle outlines in the top left should match
            // The fist pair of outlines are drawn using `DrawRectangleLinesEx()` and `DrawCircleLinesEx()`
            // The second pair are drawn using `DrawPolyLinesEx()` with parameters that should visually match the first pair

            DrawText("These should look identical!", 10, 10, 20, MAROON);

            // Outline pair 1
            DrawRectangle(10, 40, 50, 50, LIGHTGRAY);
            DrawRectangleLinesEx((Rectangle){ 10, 40, 50, 50 }, (float)thick, RED);

            DrawCircle(95, 65, 25, LIGHTGRAY);
            DrawCircleLinesEx((Vector2){ 95, 65 }, 25, (float)thick, RED);

            DrawText("DrawRectangleLinesEx() and DrawCircleLinesEx()", 130, 60, 10, BLACK);

            // Outline pair 2
            DrawPoly((Vector2){ 35, 125 }, 4, 35.355f, 45, LIGHTGRAY);
            DrawPolyLinesEx((Vector2){ 35, 125 }, 4, 35.355f, 45, (float)thick, RED);

            DrawPoly((Vector2){ 95, 125 }, 36, 25, 0, LIGHTGRAY);
            DrawPolyLinesEx((Vector2){ 95, 125 }, 36, 25, 0, (float)thick, RED);

            DrawText("DrawPolyLinesEx()", 130, 120, 10, BLACK);

            // Some other shapes, all of these outlines should have the same looking thickness
            DrawPoly((Vector2){ 290, 220 }, 3, 60, rotation, LIGHTGRAY);
            DrawPolyLinesEx((Vector2){ 290, 220 }, 3, 60, rotation, (float)thick, BLUE);

            DrawPoly((Vector2){ 430, 220 }, 4, 60, rotation, LIGHTGRAY);
            DrawPolyLinesEx((Vector2){ 430, 220 }, 4, 60, rotation, (float)thick, BLUE);

            DrawPoly((Vector2){ 570, 220 }, 5, 60, rotation, LIGHTGRAY);
            DrawPolyLinesEx((Vector2){ 570, 220 }, 5, 60, rotation, (float)thick, BLUE);

            DrawPoly((Vector2){ 710, 220 }, 6, 60, rotation, LIGHTGRAY);
            DrawPolyLinesEx((Vector2){ 710, 220 }, 6, 60, rotation, (float)thick, BLUE);

            DrawPoly((Vector2){ 290, 360 }, 7, 60, rotation, LIGHTGRAY);
            DrawPolyLinesEx((Vector2){ 290, 360 }, 7, 60, rotation, (float)thick, BLUE);

            DrawPoly((Vector2){ 430, 360 }, 8, 60, rotation, LIGHTGRAY);
            DrawPolyLinesEx((Vector2){ 430, 360 }, 8, 60, rotation, (float)thick, BLUE);

            DrawPoly((Vector2){ 570, 360 }, 9, 60, rotation, LIGHTGRAY);
            DrawPolyLinesEx((Vector2){ 570, 360 }, 9, 60, rotation, (float)thick, BLUE);

            DrawPoly((Vector2){ 710, 360 }, 10, 60, rotation, LIGHTGRAY);
            DrawPolyLinesEx((Vector2){ 710, 360 }, 10, 60, rotation, (float)thick, BLUE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
