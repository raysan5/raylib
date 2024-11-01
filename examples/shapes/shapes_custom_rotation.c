/*******************************************************************************************
 *
 *   raylib [shapes] example - draw rotation shapes
 *
 *   Example originally created with raylib 5.5
 *
 *   Example contributed by Vlad Adrian (@demizdor) and reviewed by Ramon Santamaria (@raysan5)
 *
 *   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
 *   BSD-like license that allows static linking with closed source software
 *
 *   Copyright (c) 2022-2024 Lu Hong (@CManLH)
 *
 ********************************************************************************************/
#include <math.h>

#include "raylib.h"
#include "rlgl.h"

// Fetch new coordinates of a point after it rotates around a center point by a certain angle
Vector2 RotatePointAroundPoint(Vector2 point, Vector2 center, float angle);

// Draw ellipse with rotation
void DrawEllipseRotation(Vector2 center, float radiusH, float radiusV, Color color, float angle);

// Draw ellipse outline with rotation
void DrawEllipseLinesRotation(Vector2 center, float radiusH, float radiusV, Color color, float angle);

// Draw rectangle with rotation
void DrawRectangleRotation(Vector2 point, float width, float height, Color color, float angle);

// Draw rectangle outline with rotation
void DrawRectangleLinesRotation(Vector2 point, float width, float height, Color color, float angle);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1024;
    const int screenHeight = 480;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - draw rotation shapes");

    float angle = PI / 6.0f;

    float ellipseRadiusH = 32.0f;
    float ellipseRadiusV = 72.0f;

    float rectangleWidth = 64.0f;
    float rectangleHeight = 128.0f;

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        // Draw rectangle
        DrawRectangleRotation((Vector2){128, 20}, rectangleWidth, rectangleHeight, SKYBLUE, angle);
        DrawText("Rotation Rectangle", 64, 84, 20, BLACK);

        // Draw rectangle outline
        DrawRectangleLinesRotation((Vector2){128, 256}, rectangleWidth, rectangleHeight, DARKGREEN, angle);
        DrawText("Rotation Rectangle Outline", 16, 320, 20, BLACK);

        // Draw ellipse with rotation
        DrawEllipseRotation((Vector2){432, 84}, ellipseRadiusH, ellipseRadiusV, SKYBLUE, angle);
        DrawText("Rotation Ellipse", 336, 84, 20, BLACK);

        // Draw ellipse outline with rotation
        DrawEllipseLinesRotation((Vector2){432, 320}, ellipseRadiusH, ellipseRadiusV, DARKGREEN, angle);
        DrawText("Rotation Ellipse outline", 320, 320, 20, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

void DrawEllipseRotation(Vector2 center, float radiusH, float radiusV, Color color, float angle)
{
    rlBegin(RL_TRIANGLES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        Vector2 startPoint = RotatePointAroundPoint((Vector2){center.x + radiusH, center.y}, center, angle);
        for (int i = 0; i < 360; i += 10)
        {
            Vector2 endPoint = RotatePointAroundPoint((Vector2){center.x + cosf(DEG2RAD * (i + 10)) * radiusH, center.y + sinf(DEG2RAD * (i + 10)) * radiusV}, center, angle);
            rlVertex2f(center.x, center.y);
            rlVertex2f(endPoint.x, endPoint.y);
            rlVertex2f(startPoint.x, startPoint.y);

            startPoint = endPoint;
        }
    rlEnd();
}

void DrawEllipseLinesRotation(Vector2 center, float radiusH, float radiusV, Color color, float angle)
{
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        Vector2 startPoint = RotatePointAroundPoint((Vector2){center.x + radiusH, center.y}, center, angle);
        for (int i = 0; i < 360; i += 10)
        {
            Vector2 endPoint = RotatePointAroundPoint((Vector2){center.x + cosf(DEG2RAD * (i + 10)) * radiusH, center.y + sinf(DEG2RAD * (i + 10)) * radiusV}, center, angle);
            rlVertex2f(startPoint.x, startPoint.y);
            rlVertex2f(endPoint.x, endPoint.y);

            startPoint = endPoint;
        }
    rlEnd();
}

void DrawRectangleRotation(Vector2 point, float width, float height, Color color, float angle)
{
    rlBegin(RL_TRIANGLES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        Vector2 center = (Vector2){point.x + width / 2.0f, point.y + height / 2.0f};
        Vector2 newTopLeft = RotatePointAroundPoint(point, center, angle);
        Vector2 newTopRight = RotatePointAroundPoint((Vector2){point.x + width, point.y}, center, angle);
        Vector2 newBottomLeft = RotatePointAroundPoint((Vector2){point.x, point.y + height}, center, angle);
        Vector2 newBottomRight = RotatePointAroundPoint((Vector2){point.x + width, point.y + height}, center, angle);

        rlVertex2f(newTopLeft.x, newTopLeft.y);
        rlVertex2f(newBottomLeft.x, newBottomLeft.y);
        rlVertex2f(newTopRight.x, newTopRight.y);

        rlVertex2f(newTopRight.x, newTopRight.y);
        rlVertex2f(newBottomLeft.x, newBottomLeft.y);
        rlVertex2f(newBottomRight.x, newBottomRight.y);
    rlEnd();
}

void DrawRectangleLinesRotation(Vector2 point, float width, float height, Color color, float angle)
{
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        Vector2 center = (Vector2){point.x + width / 2.0f, point.y + height / 2.0f};
        Vector2 newLeftTop = RotatePointAroundPoint(point, center, angle);
        Vector2 newRightTop = RotatePointAroundPoint((Vector2){point.x + width, point.y}, center, angle);
        Vector2 newLeftBottom = RotatePointAroundPoint((Vector2){point.x, point.y + height}, center, angle);
        Vector2 newRightBottom = RotatePointAroundPoint((Vector2){point.x + width, point.y + height}, center, angle);

        rlVertex2f(newLeftTop.x, newLeftTop.y);
        rlVertex2f(newRightTop.x, newRightTop.y);

        rlVertex2f(newRightTop.x, newRightTop.y);
        rlVertex2f(newRightBottom.x, newRightBottom.y);

        rlVertex2f(newRightBottom.x, newRightBottom.y);
        rlVertex2f(newLeftBottom.x, newLeftBottom.y);

        rlVertex2f(newLeftBottom.x, newLeftBottom.y);
        rlVertex2f(newLeftTop.x, newLeftTop.y);
    rlEnd();
}

Vector2 RotatePointAroundPoint(Vector2 point, Vector2 center, float angle)
{
    // Vector center->point
    float x = point.x - center.x;
    float y = point.y - center.y;

    // new vector after rotation
    float cosres = cosf(angle);
    float sinres = sinf(angle);
    float newX = x * cosres - y * sinres;
    float newY = x * sinres + y * cosres;

    // new point
    return (Vector2){newX + center.x, newY + center.y};
}