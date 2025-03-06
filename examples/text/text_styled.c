/*******************************************************************************************
*
*   raylib [text] example - Text styled
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Wagner Barongello (@SultansOfCode)
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

    InitWindow(screenWidth, screenHeight, "raylib [text] example - text styled");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    Color colors[3] = { RED, GREEN, BLUE };
    int colorCount = sizeof(colors) / sizeof(Color);
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

            ClearBackground(SKYBLUE);

            DrawTextStyled("This changes the \0032foreground color", 200, 80, 20, colors, colorCount);

            DrawTextStyled("This changes the \0030,2background color", 200, 120, 20, colors, colorCount);

            DrawTextStyled("This changes the \0031,2foreground and background colors", 200, 160, 20, colors, colorCount);

            DrawTextStyled("\0031,2This \015restores the colors to the default ones", 200, 200, 20, colors, colorCount);

            DrawTextStyled("\0031,2This \022inverts\022 the colors", 200, 240, 20, colors, colorCount);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
