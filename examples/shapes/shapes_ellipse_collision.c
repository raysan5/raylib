/*******************************************************************************************
*
*   raylib [shapes] example - ellipse collision
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   Example contributed by Ziya (@Monjaris)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Ziya (@Monjaris)
*
********************************************************************************************/

#include "raylib.h"
#include <math.h>

// Check if point is inside ellipse
static bool CheckCollisionPointEllipse(Vector2 point, Vector2 center, float rx, float ry)
{
    float dx = (point.x - center.x)/rx;
    float dy = (point.y - center.y)/ry;
    return (dx*dx + dy*dy) <= 1.0f;
}

// Check if two ellipses collide
// Uses radial boundary distance in the direction between centers — scales correctly with radii
static bool CheckCollisionEllipses(Vector2 c1, float rx1, float ry1, Vector2 c2, float rx2, float ry2)
{
    float dx = c2.x - c1.x;
    float dy = c2.y - c1.y;
    float dist = sqrtf(dx*dx + dy*dy);

    // Ellipses are on top of each other
    if (dist == 0.0f) return true;

    float theta = atan2f(dy, dx);
    float cosT = cosf(theta);
    float sinT = sinf(theta);

    // Radial distance from center to ellipse boundary in direction theta
    // r(theta) = (rx * ry) / sqrt((ry*cos)^2 + (rx*sin)^2)
    float r1 = (rx1*ry1)/sqrtf((ry1*cosT)*(ry1*cosT) + (rx1*sinT)*(rx1*sinT));
    float r2 = (rx2*ry2)/sqrtf((ry2*cosT)*(ry2*cosT) + (rx2*sinT)*(rx2*sinT));

    return dist <= (r1 + r2);
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

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - collision ellipses");
    SetTargetFPS(60);

    Vector2 ellipseACenter = { (float)screenWidth/4, (float)screenHeight/2 };
    float ellipseARx = 120.0f;
    float ellipseARy = 70.0f;

    Vector2 ellipseBCenter = { (float)screenWidth*3/4, (float)screenHeight/2 };
    float ellipseBRx = 90.0f;
    float ellipseBRy = 140.0f;

    // 0 = controlling A, 1 = controlling B
    int controlled = 0;

    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_A)) controlled = 0;
        if (IsKeyPressed(KEY_B)) controlled = 1;

        if (controlled == 0) ellipseACenter = GetMousePosition();
        else ellipseBCenter = GetMousePosition();

        bool ellipsesCollide = CheckCollisionEllipses(
            ellipseACenter, ellipseARx, ellipseARy,
            ellipseBCenter, ellipseBRx, ellipseBRy
        );

        bool mouseInA = CheckCollisionPointEllipse(GetMousePosition(), ellipseACenter, ellipseARx, ellipseARy);
        bool mouseInB = CheckCollisionPointEllipse(GetMousePosition(), ellipseBCenter, ellipseBRx, ellipseBRy);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawEllipse((int)ellipseACenter.x, (int)ellipseACenter.y, ellipseARx, ellipseARy, ellipsesCollide ? RED : BLUE);

            DrawEllipse((int)ellipseBCenter.x, (int)ellipseBCenter.y, ellipseBRx, ellipseBRy, ellipsesCollide ? RED : GREEN);

            DrawEllipseLines((int)ellipseACenter.x, (int)ellipseACenter.y, ellipseARx, ellipseARy, WHITE);

            DrawEllipseLines((int)ellipseBCenter.x, (int)ellipseBCenter.y, ellipseBRx, ellipseBRy, WHITE);

            DrawCircleV(ellipseACenter, 4, WHITE);
            DrawCircleV(ellipseBCenter, 4, WHITE);

            if (ellipsesCollide) DrawText("ELLIPSES COLLIDE", screenWidth/2 - 120, 40, 28, RED);
            else DrawText("NO COLLISION", screenWidth/2 - 80, 40, 28, DARKGRAY);

            DrawText(controlled == 0 ? "Controlling: A" : "Controlling: B", 20, screenHeight - 40, 20, YELLOW);

            if (mouseInA && controlled != 0) DrawText("Mouse inside ellipse A", 20, screenHeight - 70, 20, BLUE);
            if (mouseInB && controlled != 1) DrawText("Mouse inside ellipse B", 20, screenHeight - 70, 20, GREEN);

            DrawText("Press [A] or [B] to switch control", 20, 20, 20, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    CloseWindow();

    return 0;
}
