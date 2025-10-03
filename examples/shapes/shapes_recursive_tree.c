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

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    Vector2 start;
    float angle;
    float length;
} Branch;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static Vector2 CalculateBranchEnd(Vector2 start, float angle, float length);
static void DrawBranch(Vector2 start, float angle, float length, float thick, bool bezier);

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
    float angle = 45.0f;
    float thick = 1.0f;   
    float treeDepth = 1.0f;
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
        float theta = angle * DEG2RAD;
        int maxBranches = (int)(powf(2, treeDepth) - 1); 
        Branch branches[4096];
        int count = 0;
        
        branches[count++] = (Branch){start, 0.0f, length};
        
        for (int i = 0; i < count; i++) {
            Branch branch = branches[i];
            if (branch.length < 2) continue;
            
            Vector2 end = CalculateBranchEnd(branch.start, branch.angle, branch.length);
            float nextLength = branch.length * branchDecay;
            
            if (count < maxBranches && nextLength >= 2) {
                branches[count++] = (Branch){end, branch.angle + theta, nextLength};
                branches[count++] = (Branch){end, branch.angle - theta, nextLength};
            }
        }
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            for (int i = 0; i < count; i++) {
                Branch branch = branches[i];
                if (branch.length >= 2) {
                    DrawBranch(branch.start, branch.angle, branch.length, thick, bezier);
                }
            }

            DrawLine(560, 0, 560, GetScreenHeight(), (Color){ 218, 218, 218, 255 });
            DrawRectangle(560, 0, GetScreenWidth() - 500, GetScreenHeight(), (Color){ 232, 232, 232, 255 });
            
            // Draw GUI controls
            //------------------------------------------------------------------------------
            GuiSliderBar((Rectangle){ 640, 40, 120, 20}, "Angle", TextFormat("%.0f", angle), &angle, 0, 180);
            GuiSliderBar((Rectangle){ 640, 70, 120, 20 }, "Length", TextFormat("%.0f", length), &length, 12.0f, 240.0f);
            GuiSliderBar((Rectangle){ 640, 100, 120, 20}, "Branch Decay", TextFormat("%.2f", branchDecay), &branchDecay, 0.1f, 0.78f);
            GuiSliderBar((Rectangle){ 640, 130, 120, 20 }, "Tree Depth", TextFormat("%.0f", treeDepth), &treeDepth, 1.0f, 12.f);
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

static Vector2 CalculateBranchEnd(Vector2 start, float angle, float length) {
    return (Vector2){
        start.x + length * sinf(angle),
        start.y - length * cosf(angle)
    };
}

static void DrawBranch(Vector2 start, float angle, float length, float thick, bool bezier) {
    Vector2 end = CalculateBranchEnd(start, angle, length);
    if (!bezier) DrawLineEx(start, end, thick, RED);
    else DrawLineBezier(start, end, thick, RED);
}