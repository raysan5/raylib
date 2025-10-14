/*******************************************************************************************
*
*   raylib [shapes] example - dashed line
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   Example contributed by Luís Almeida (@luis605)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Luís Almeida (@luis605)
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

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - dashed line");

    // Line Properties
    Vector2 lineStartPosition = { 20.0f, 50.0f };
    Vector2 lineEndPosition = { 780.0f, 400.0f };
    float dashLength = 25.0f;
    float blankLength = 15.0f;

    // Color selection
    Color lineColors[] = { RED, ORANGE, GOLD, GREEN, BLUE, VIOLET, PINK, BLACK };
    int colorIndex = 0;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        lineEndPosition = GetMousePosition(); // Line endpoint follows the mouse

        // Change Dash Length (UP/DOWN arrows)
        if (IsKeyDown(KEY_UP)) dashLength += 1.0f;
        if (IsKeyDown(KEY_DOWN) && dashLength > 1.0f) dashLength -= 1.0f;

        // Change Space Length (LEFT/RIGHT arrows)
        if (IsKeyDown(KEY_RIGHT)) blankLength += 1.0f;
        if (IsKeyDown(KEY_LEFT) && blankLength > 1.0f) blankLength -= 1.0f;

        // Cycle through colors ('C' key)
        if (IsKeyPressed(KEY_C)) colorIndex = (colorIndex + 1)%(sizeof(lineColors)/sizeof(Color));
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Draw the dashed line with the current properties
            DrawLineDashed(lineStartPosition, lineEndPosition, (int)dashLength, (int)blankLength, lineColors[colorIndex]);

            // Draw UI and Instructions
            DrawRectangle(5, 5, 265, 95, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(5, 5, 265, 95, BLUE);

            DrawText("CONTROLS:", 15, 15, 10, BLACK);
            DrawText("UP/DOWN: Change Dash Length", 15, 35, 10, BLACK);
            DrawText("LEFT/RIGHT: Change Space Length", 15, 55, 10, BLACK);
            DrawText("C: Cycle Color", 15, 75, 10, BLACK);

            DrawText(TextFormat("Dash: %.0f | Space: %.0f", dashLength, blankLength), 15, 115, 10, DARKGRAY);

            DrawFPS(screenWidth - 80, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}