/*******************************************************************************************
*
*   raylib [shapes] example - draw ring (with gui options)
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 2.5, last time updated with raylib 2.5
*
*   Example contributed by Vlad Adrian (@demizdor) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2018-2025 Vlad Adrian (@demizdor) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"                 // Required for GUI controls

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - draw ring");

    Vector2 center = {(GetScreenWidth() - 300)/2.0f, GetScreenHeight()/2.0f };

    float innerRadius = 80.0f;
    float outerRadius = 190.0f;

    float startAngle = 0.0f;
    float endAngle = 360.0f;
    float segments = 0.0f;

    bool drawRing = true;
    bool drawRingLines = false;
    bool drawCircleLines = false;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // NOTE: All variables update happens inside GUI control functions
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawLine(500, 0, 500, GetScreenHeight(), Fade(LIGHTGRAY, 0.6f));
            DrawRectangle(500, 0, GetScreenWidth() - 500, GetScreenHeight(), Fade(LIGHTGRAY, 0.3f));

            if (drawRing) DrawRing(center, innerRadius, outerRadius, startAngle, endAngle, (int)segments, Fade(MAROON, 0.3f));
            if (drawRingLines) DrawRingLines(center, innerRadius, outerRadius, startAngle, endAngle, (int)segments, Fade(BLACK, 0.4f));
            if (drawCircleLines) DrawCircleSectorLines(center, outerRadius, startAngle, endAngle, (int)segments, Fade(BLACK, 0.4f));

            // Draw GUI controls
            //------------------------------------------------------------------------------
            GuiSliderBar((Rectangle){ 600, 40, 120, 20 }, "StartAngle", TextFormat("%.2f", startAngle), &startAngle, -450, 450);
            GuiSliderBar((Rectangle){ 600, 70, 120, 20 }, "EndAngle", TextFormat("%.2f", endAngle), &endAngle, -450, 450);

            GuiSliderBar((Rectangle){ 600, 140, 120, 20 }, "InnerRadius", TextFormat("%.2f", innerRadius), &innerRadius, 0, 100);
            GuiSliderBar((Rectangle){ 600, 170, 120, 20 }, "OuterRadius", TextFormat("%.2f", outerRadius), &outerRadius, 0, 200);

            GuiSliderBar((Rectangle){ 600, 240, 120, 20 }, "Segments", TextFormat("%.2f", segments), &segments, 0, 100);

            GuiCheckBox((Rectangle){ 600, 320, 20, 20 }, "Draw Ring", &drawRing);
            GuiCheckBox((Rectangle){ 600, 350, 20, 20 }, "Draw RingLines", &drawRingLines);
            GuiCheckBox((Rectangle){ 600, 380, 20, 20 }, "Draw CircleLines", &drawCircleLines);
            //------------------------------------------------------------------------------

            int minSegments = (int)ceilf((endAngle - startAngle)/90);
            DrawText(TextFormat("MODE: %s", (segments >= minSegments)? "MANUAL" : "AUTO"), 600, 270, 10, (segments >= minSegments)? MAROON : DARKGRAY);

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