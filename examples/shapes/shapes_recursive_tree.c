/*******************************************************************************************
*
*   raylib [shapes] example - recursive tree
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
*   Copyright (c) 2025 Jopestpe (@jopestpe)
*
********************************************************************************************/

#include "raylib.h"
#include <math.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"                 // Required for GUI controls

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    Vector2 start;
    Vector2 end;
    float angle;
    float length;
} Branch;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - recursive tree");

    Vector2 start = { (screenWidth/2.0f) - 125.0f, (float)screenHeight };
    float angle = 40.0f;
    float thick = 1.0f;
    float treeDepth = 10.0f;
    float branchDecay = 0.66f;
    float length = 120.0f;
    bool bezier = false;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        float theta = angle*DEG2RAD;
        int maxBranches = (int)(powf(2, floorf(treeDepth)));
        Branch branches[1030] = { 0 };
        int count = 0;

        Vector2 initialEnd = { start.x + length*sinf(0.0f), start.y - length*cosf(0.0f) };
        branches[count++] = (Branch){start, initialEnd, 0.0f, length};

        for (int i = 0; i < count; i++)
        {
            Branch branch = branches[i];
            if (branch.length < 2) continue;

            float nextLength = branch.length*branchDecay;

            if (count < maxBranches && nextLength >= 2)
            {
                Vector2 branchStart = branch.end;

                float angle1 = branch.angle + theta;
                Vector2 branchEnd1 = { branchStart.x + nextLength*sinf(angle1), branchStart.y - nextLength*cosf(angle1) };
                branches[count++] = (Branch){branchStart, branchEnd1, angle1, nextLength};

                float angle2 = branch.angle - theta;
                Vector2 branchEnd2 = { branchStart.x + nextLength*sinf(angle2), branchStart.y - nextLength*cosf(angle2) };
                branches[count++] = (Branch){branchStart, branchEnd2, angle2, nextLength};
            }
        }
        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            for (int i = 0; i < count; i++)
            {
                Branch branch = branches[i];
                if (branch.length >= 2)
                {
                    if (bezier) DrawLineBezier(branch.start, branch.end, thick, RED);
                    else DrawLineEx(branch.start, branch.end, thick, RED);
                }
            }

            DrawLine(580, 0, 580, GetScreenHeight(), (Color){ 218, 218, 218, 255 });
            DrawRectangle(580, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 232, 232, 232, 255 });

            // Draw GUI controls
            //------------------------------------------------------------------------------
            GuiSliderBar((Rectangle){ 640, 40, 120, 20}, "Angle", TextFormat("%.0f", angle), &angle, 0, 180);
            GuiSliderBar((Rectangle){ 640, 70, 120, 20 }, "Length", TextFormat("%.0f", length), &length, 12.0f, 240.0f);
            GuiSliderBar((Rectangle){ 640, 100, 120, 20}, "Decay", TextFormat("%.2f", branchDecay), &branchDecay, 0.1f, 0.78f);
            GuiSliderBar((Rectangle){ 640, 130, 120, 20 }, "Depth", TextFormat("%.0f", treeDepth), &treeDepth, 1.0f, 10.0f);
            GuiSliderBar((Rectangle){ 640, 160, 120, 20}, "Thick", TextFormat("%.0f", thick), &thick, 1, 8);
            GuiCheckBox((Rectangle){ 640, 190, 20, 20 }, "Bezier", &bezier);
            //------------------------------------------------------------------------------

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