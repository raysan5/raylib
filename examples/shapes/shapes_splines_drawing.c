/*******************************************************************************************
*
*   raylib [shapes] example - splines drawing
*
*   Example originally created with raylib 5.0, last time updated with raylib 5.0
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2023 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"     // Required for UI controls

#include <stdlib.h>     // Required for: NULL

#define MAX_SPLINE_POINTS      32

// Cubic Bezier spline control points
// NOTE: Every segment has two control points 
typedef struct {
    Vector2 start;
    Vector2 end;
} ControlPoint;

// Spline types
typedef enum {
    SPLINE_LINEAR = 0,      // Linear
    SPLINE_BASIS,           // B-Spline
    SPLINE_CATMULLROM,      // Catmull-Rom
    SPLINE_BEZIER           // Cubic Bezier
} SplineType;

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
    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - splines drawing");

    Vector2 points[MAX_SPLINE_POINTS] = {
        {  50.0f, 400.0f },
        { 160.0f, 220.0f },
        { 340.0f, 380.0f },
        { 520.0f, 60.0f },
        { 710.0f, 260.0f },
    };
    
    int pointCount = 5;
    int selectedPoint = -1;
    int focusedPoint = -1;
    Vector2 *selectedControlPoint = NULL;
    Vector2 *focusedControlPoint = NULL;
    
    // Cubic Bezier control points initialization
    ControlPoint control[MAX_SPLINE_POINTS] = { 0 };
    for (int i = 0; i < pointCount - 1; i++)
    {
        control[i].start = (Vector2){ points[i].x + 50, points[i].y };
        control[i].end = (Vector2){ points[i + 1].x - 50, points[i + 1].y };
    }

    // Spline config variables
    float splineThickness = 8.0f;
    int splineTypeActive = SPLINE_LINEAR; // 0-Linear, 1-BSpline, 2-CatmullRom, 3-Bezier
    bool splineTypeEditMode = false; 
    bool splineHelpersActive = true;
    
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Spline points creation logic (at the end of spline)
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && (pointCount < MAX_SPLINE_POINTS))
        {
            points[pointCount] = GetMousePosition();
            pointCount++;
        }

        // Spline point focus and selection logic
        for (int i = 0; i < pointCount; i++)
        {
            if (CheckCollisionPointCircle(GetMousePosition(), points[i], 8.0f))
            {
                focusedPoint = i;
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) selectedPoint = i; 
                break;
            }
            else focusedPoint = -1;
        }
        
        // Spline point movement logic
        if (selectedPoint >= 0)
        {
            points[selectedPoint] = GetMousePosition();
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) selectedPoint = -1;
        }
        
        // Cubic Bezier spline control points logic
        if ((splineTypeActive == SPLINE_BEZIER) && (focusedPoint == -1))
        {
            // Spline control point focus and selection logic
            for (int i = 0; i < pointCount; i++)
            {
                if (CheckCollisionPointCircle(GetMousePosition(), control[i].start, 6.0f))
                {
                    focusedControlPoint = &control[i].start;
                    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) selectedControlPoint = &control[i].start; 
                    break;
                }
                else if (CheckCollisionPointCircle(GetMousePosition(), control[i].end, 6.0f))
                {
                    focusedControlPoint = &control[i].end;
                    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) selectedControlPoint = &control[i].end; 
                    break;
                }
                else focusedControlPoint = NULL;
            }
            
            // Spline control point movement logic
            if (selectedControlPoint != NULL)
            {
                *selectedControlPoint = GetMousePosition();
                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) selectedControlPoint = NULL;
            }
        }
        
        // Spline selection logic
        if (IsKeyPressed(KEY_ONE)) splineTypeActive = 0;
        else if (IsKeyPressed(KEY_TWO)) splineTypeActive = 1;
        else if (IsKeyPressed(KEY_THREE)) splineTypeActive = 2;
        else if (IsKeyPressed(KEY_FOUR)) splineTypeActive = 3;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
        
            if (splineTypeActive == SPLINE_LINEAR)
            {
                // Draw spline: linear
                DrawSplineLinear(points, pointCount, splineThickness, RED);
            }
            else if (splineTypeActive == SPLINE_BASIS)
            {
                // Draw spline: basis
                DrawSplineBasis(points, pointCount, splineThickness, RED);  // Provide connected points array

                /*
                for (int i = 0; i < (pointCount - 3); i++)
                {
                    // Drawing individual segments, not considering thickness connection compensation
                    DrawSplineSegmentBasis(points[i], points[i + 1], points[i + 2], points[i + 3], splineThickness, MAROON);
                }
                */
            }
            else if (splineTypeActive == SPLINE_CATMULLROM)
            {
                // Draw spline: catmull-rom
                DrawSplineCatmullRom(points, pointCount, splineThickness, RED); // Provide connected points array
                
                /*
                for (int i = 0; i < (pointCount - 3); i++)
                {
                    // Drawing individual segments, not considering thickness connection compensation
                    DrawSplineSegmentCatmullRom(points[i], points[i + 1], points[i + 2], points[i + 3], splineThickness, MAROON);
                }
                */
            }
            else if (splineTypeActive == SPLINE_BEZIER)
            {
                // Draw spline: cubic-bezier (with control points)
                for (int i = 0; i < pointCount - 1; i++)
                {
                    // Drawing individual segments, not considering thickness connection compensation
                    DrawSplineSegmentBezierCubic(points[i], control[i].start, control[i].end, points[i + 1], splineThickness, RED);

                    // Every cubic bezier point should have two control points
                    DrawCircleV(control[i].start, 6, GOLD);
                    DrawCircleV(control[i].end, 6, GOLD);
                    if (focusedControlPoint == &control[i].start) DrawCircleV(control[i].start, 8, GREEN);
                    else if (focusedControlPoint == &control[i].end) DrawCircleV(control[i].end, 8, GREEN);
                    DrawLineEx(points[i], control[i].start, 1.0f, LIGHTGRAY);
                    DrawLineEx(points[i + 1], control[i].end, 1.0f, LIGHTGRAY);
                
                    // Draw spline control lines
                    DrawLineV(points[i], control[i].start, GRAY);
                    //DrawLineV(control[i].start, control[i].end, LIGHTGRAY);
                    DrawLineV(control[i].end, points[i + 1], GRAY);
                }
            }

            if (splineHelpersActive)
            {
                // Draw spline point helpers
                for (int i = 0; i < pointCount; i++)
                {
                    DrawCircleLinesV(points[i], (focusedPoint == i)? 12.0f : 8.0f, (focusedPoint == i)? BLUE: DARKBLUE);
                    if ((splineTypeActive != SPLINE_LINEAR) &&
                        (splineTypeActive != SPLINE_BEZIER) &&
                        (i < pointCount - 1)) DrawLineV(points[i], points[i + 1], GRAY);

                    DrawText(TextFormat("[%.0f, %.0f]", points[i].x, points[i].y), points[i].x, points[i].y + 10, 10, BLACK);
                }
            }

            // Check all possible UI states that require controls lock
            if (splineTypeEditMode) GuiLock();
            
            // Draw spline config
            GuiLabel((Rectangle){ 12, 62, 140, 24 }, TextFormat("Spline thickness: %i", (int)splineThickness));
            GuiSliderBar((Rectangle){ 12, 60 + 24, 140, 16 }, NULL, NULL, &splineThickness, 1.0f, 40.0f);

            GuiCheckBox((Rectangle){ 12, 110, 20, 20 }, "Show point helpers", &splineHelpersActive);

            GuiUnlock();

            GuiLabel((Rectangle){ 12, 10, 140, 24 }, "Spline type:");
            if (GuiDropdownBox((Rectangle){ 12, 8 + 24, 140, 28 }, "LINEAR;BSPLINE;CATMULLROM;BEZIER", &splineTypeActive, splineTypeEditMode)) splineTypeEditMode = !splineTypeEditMode;

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}