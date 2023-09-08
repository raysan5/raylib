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

#define MAX_CONTROL_POINTS      32

typedef struct {
    Vector2 start;
    Vector2 end;
} ControlPoint;

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

    Vector2 points[MAX_CONTROL_POINTS] = {
        { 100.0f, 200.0f },
        { 300.0f, 400.0f },
        { 500.0f, 300.0f },
        { 700.0f, 100.0f },
        { 200.0f, 100.0f },
    };
    
    int pointCount = 5;
    int selectedPoint = -1;
    
    int splineType = 0;             // 0-Linear, 1-BSpline, 2-CatmullRom, 3-Bezier
    
    // Cubic Bezier control points
    ControlPoint control[MAX_CONTROL_POINTS] = { 0 };
    for (int i = 0; i < pointCount - 1; i++)
    {
        control[i].start = points[i];
        control[i].end = points[i + 1];
    }
    
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Points movement logic
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && (pointCount < MAX_CONTROL_POINTS))
        {
            points[pointCount] = GetMousePosition();
            pointCount++;
        }

        for (int i = 0; i < pointCount; i++)
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && CheckCollisionPointCircle(GetMousePosition(), points[i], 6.0f))
            {
                selectedPoint = i;
                break;
            }
        }

        if (selectedPoint >= 0)
        {
            points[selectedPoint] = GetMousePosition();
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) selectedPoint = -1;
        }
        
        // TODO: Cubic Bezier spline control points logic
        
        
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
        
            if (splineType == 0)        // Linear
            {
                // Draw linear spline
                for (int i = 0; i < pointCount - 1; i++)
                {
                    DrawLineEx(points[i], points[i + 1], 2.0f, RED);
                }
            }
            else if (splineType == 1)   // B-Spline
            {
                // Draw b-spline
                DrawLineBSpline(points, pointCount, 2.0f, RED);
                //for (int i = 0; i < (pointCount - 3); i++) DrawLineBSplineSegment(points[i], points[i + 1], points[i + 2], points[i + 3], 24.0f, BLUE);
            }
            else if (splineType == 2)   // CatmullRom Spline
            {
                // Draw spline: catmull-rom
                DrawLineCatmullRom(points, pointCount, 2.0f, RED);
                //for (int i = 0; i < (pointCount - 3); i++) DrawLineCatmullRomSegment(points[i], points[i + 1], points[i + 2], points[i + 3], 24.0f, Fade(BLUE, 0.4f));
            }
            else if (splineType == 3)   // Cubic Bezier
            {
                // Draw line bezier cubic (with control points)
                for (int i = 0; i < pointCount - 1; i++)
                {
                    DrawLineBezierCubic(points[i], points[i + 1], control[i].start, control[i + 1].end, 2.0f, RED);

                    // TODO: Every cubic bezier point should have two control points
                    DrawCircleV(control[i].start, 4, GOLD);
                    DrawCircleV(control[i].end, 4, GOLD);
                    DrawLineEx(points[i], control[i].start, 1.0, LIGHTGRAY);
                    DrawLineEx(points[i + 1], control[i].end, 1.0, LIGHTGRAY);
                }
            }

            // Draw control points
            for (int i = 0; i < pointCount; i++)
            {
                DrawCircleV(points[i], 6.0f, RED);
                if ((splineType != 0) && (i < pointCount - 1)) DrawLineV(points[i], points[i + 1], GRAY);
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}