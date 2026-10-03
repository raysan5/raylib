/*******************************************************************************************
*
*   raylib [shapes] example - outlines thickness
*
*   Example complexity rating: [★☆☆☆] 1/4
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

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"         // Required for GUI controls

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - outlines thickness");

    float thick = 5.0f;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update variables / Implement example logic at this point
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            GuiSliderBar((Rectangle){ 290, 50, 220, 24 }, "Thickness", TextFormat("%.2f", thick), &thick, -30.0f, 30.0f);

            DrawRectangle(35, 180, 220, 220, LIGHTGRAY);
            DrawRectangleLinesEx((Rectangle){ 35, 180, 220, 220 }, thick, BLUE);
            DrawText("DrawRectangleLinesEx()", 35, 160, 10, BLACK);

            DrawRectangleRounded((Rectangle){ 290, 180, 220, 220 }, 0.2f, 9, LIGHTGRAY);
            DrawRectangleRoundedLinesEx((Rectangle){ 290, 180, 220, 220 }, 0.2f, 9, thick, BLUE);
            DrawText("DrawRectangleRoundedLinesEx()", 290, 160, 10, BLACK);

            DrawCircle(655, 290, 110, LIGHTGRAY);
            DrawCircleLinesEx((Vector2){ 655, 290 }, 110, thick, BLUE);
            DrawText("DrawCircleLinesEx()", 545, 160, 10, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
