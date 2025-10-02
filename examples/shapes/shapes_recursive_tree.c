/*******************************************************************************************
*
*   raylib [shapes] example - shapes recursive tree
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 5.6-dev, last time updated with raylib 5.6-dev
*
*   Example contributed by Jopestpe (@jopestpe)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2018-2025 Jopestpe (@jopestpe)
*
********************************************************************************************/

#include "raylib.h"
#include <math.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"                 // Required for GUI controls

float theta;
float thick = 1.0f;
float branchDecay = 0.66f;
bool bezier = false;

void Branch(Vector2 start, float length, float angle) {
    length *= branchDecay;

    if (length > 2) {
        Vector2 end = {
            start.x + length * sinf(angle),
            start.y - length * cosf(angle)
        };

        if(!bezier) DrawLineEx(start, end, thick, RED);  
        else DrawLineBezier(start, end, thick, RED);  

        Branch(end, length, angle + theta);
        Branch(end, length, angle - theta);
    }
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - shapes recursive tree");

    Vector2 start = { (screenWidth / 2.0f) - 125, screenHeight };
    Vector2 end = { (screenWidth / 2.0f) - 125, screenHeight - 120.0f };
    float angle = 0.0f;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        float a = (angle / screenWidth) * 90.0f;
        theta = a * (PI / 180.0f);
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawLine(560, 0, 560, GetScreenHeight(), Fade(LIGHTGRAY, 0.6f));
            DrawRectangle(560, 0, GetScreenWidth() - 500, GetScreenHeight(), Fade(LIGHTGRAY, 0.3f));
            
            // Draw GUI controls
            //------------------------------------------------------------------------------
            GuiSliderBar((Rectangle){ 640, 40, 120, 20}, "Angle", TextFormat("%.2f", angle), &angle, 0, 1600);
            GuiSliderBar((Rectangle){ 640, 70, 120, 20}, "Thick", TextFormat("%.2f", thick), &thick, 1, 8);
            GuiSliderBar((Rectangle){ 640, 100, 120, 20}, "Branch Decay", TextFormat("%.2f", branchDecay), &branchDecay, 0.1f, 0.78f);
            GuiCheckBox((Rectangle){ 640, 130, 20, 20 }, "Bezier", &bezier);
            //------------------------------------------------------------------------------
            
            DrawLineEx(start, end, thick, RED);
            Branch(end, 120.0f, 0.0f);

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}