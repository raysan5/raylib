/*******************************************************************************************
*
*   raylib [shapes] example - splines drawing
*
*   Example originally created with raylib 4.6-dev, last time updated with raylib 4.6-dev
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2023 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>     // Required for: NULL

#define MAX_SPLINE_POINTS      32

// Bezier spline control points
// NOTE: Every segment has two control points 
typedef struct {
    Vector2 start;
    Vector2 end;
} ControlPoint;

// Spline types
typedef enum {
    SPLINE_LINEAR = 0,
    SPLINE_BASIS,       // B-Spline
    SPLINE_CATMULLROM,
    SPLINE_BEZIER
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
        { 100.0f, 200.0f },
        { 300.0f, 400.0f },
        { 500.0f, 300.0f },
        { 700.0f, 100.0f },
        { 200.0f, 100.0f },
    };
    
    int pointCount = 5;
    int selectedPoint = -1;
    int focusedPoint = -1;
    Vector2 *selectedControlPoint = NULL;
    Vector2 *focusedControlPoint = NULL;
    
    int splineType = SPLINE_LINEAR; // 0-Linear, 1-BSpline, 2-CatmullRom, 3-Bezier
    
    // Cubic Bezier control points initialization
    ControlPoint control[MAX_SPLINE_POINTS] = { 0 };
    for (int i = 0; i < pointCount - 1; i++)
    {
        control[i].start = (Vector2){ points[i].x - 20, points[i].y - 20 };
        control[i].end = (Vector2){ points[i + 1].x + 20, points[i + 1].y + 20 };
    }
    
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
        if ((splineType == SPLINE_BEZIER) && (focusedPoint == -1))
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
        if (IsKeyPressed(KEY_ONE)) splineType = 0;
        else if (IsKeyPressed(KEY_TWO)) splineType = 1;
        else if (IsKeyPressed(KEY_THREE)) splineType = 2;
        else if (IsKeyPressed(KEY_FOUR)) splineType = 3;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
        
            if (splineType == SPLINE_LINEAR)
            {
                // Draw spline: linear
                DrawSplineLinear(points, pointCount, 2.0f, RED);
            }
            else if (splineType == SPLINE_BASIS)
            {
                // Draw spline: basis
                DrawSplineBasis(points, pointCount, 2.0f, RED);
                //for (int i = 0; i < (pointCount - 3); i++) DrawSplineBasisSegment(points[i], points[i + 1], points[i + 2], points[i + 3], 24.0f, BLUE);
            }
            else if (splineType == SPLINE_CATMULLROM)
            {
                // Draw spline: catmull-rom
                DrawSplineCatmullRom(points, pointCount, 2.0f, RED);
                //for (int i = 0; i < (pointCount - 3); i++) DrawSplineCatmullRomSegment(points[i], points[i + 1], points[i + 2], points[i + 3], 24.0f, Fade(BLUE, 0.4f));
            }
            else if (splineType == SPLINE_BEZIER)
            {
                // Draw spline: cubic-bezier (with control points)
                for (int i = 0; i < pointCount - 1; i++)
                {
                    DrawSplineBezierCubic(points[i], control[i].start, control[i].end, points[i + 1], 2.0f, RED);

                    // Every cubic bezier point should have two control points
                    DrawCircleV(control[i].start, 4, GOLD);
                    DrawCircleV(control[i].end, 4, GOLD);
                    if (focusedControlPoint == &control[i].start) DrawCircleV(control[i].start, 6, GREEN);
                    else if (focusedControlPoint == &control[i].end) DrawCircleV(control[i].end, 6, GREEN);
                    DrawLineEx(points[i], control[i].start, 1.0, LIGHTGRAY);
                    DrawLineEx(points[i + 1], control[i].end, 1.0, LIGHTGRAY);
                
                    // Draw spline control lines
                    DrawLineV(points[i], control[i].start, LIGHTGRAY);
                    DrawLineV(control[i].start, control[i].end, LIGHTGRAY);
                    DrawLineV(control[i].end, points[i + 1], LIGHTGRAY);
                }
            }

            // Draw spline key-points
            for (int i = 0; i < pointCount; i++)
            {
                DrawCircleV(points[i], (focusedPoint == i)? 8.0f : 5.0f, (focusedPoint == i)? BLUE: RED);
                if ((splineType != SPLINE_LINEAR) && 
                    (splineType != SPLINE_BEZIER) && 
                    (i < pointCount - 1)) DrawLineV(points[i], points[i + 1], LIGHTGRAY);
            }
            
            // TODO: Draw help

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}