/*******************************************************************************************
 *
 *   raylib [shapes] example - draw ellipse
 *
 *   Example originally created with raylib 4.2, last time updated with raylib 5.5
 *
 *   Example contributed by Vlad Adrian (@demizdor) and reviewed by Ramon Santamaria (@raysan5)
 *
 *   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
 *   BSD-like license that allows static linking with closed source software
 *
 *   Copyright (c) 2022-2024 Lu Hong (@CManLH)
 *
 ********************************************************************************************/

#include "raylib.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - draw ellipse");

    float radiusH = 70.0f;
    float radiusV = 100.0f;

    Vector2 ellipseCenter = (Vector2){200, 100};
    Color ellipseColor = SKYBLUE;

    Vector2 ellipseLineCenter = (Vector2){200, 325};
    Color ellipseLineColor = DARKGREEN;

    Vector2 rotationEllipseCenter = (Vector2){460, 100};
    Color rotationEllipseColor = GREEN;

    Vector2 rotationEllipseLineCenter = (Vector2){460, 325};
    Color rotationEllipseLineColor = RED;

    float angle = PI / 6.0f;

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        // Draw ellipse
        DrawEllipse(ellipseCenter.x, ellipseCenter.y, radiusH, radiusV, ellipseColor);
        DrawText("Ellipse!", ellipseCenter.x - 35, ellipseCenter.y, 20, BLACK);

        // Draw ellipse outline
        DrawEllipseLines(ellipseLineCenter.x, ellipseLineCenter.y, radiusH, radiusV, ellipseLineColor);
        DrawText("Ellipse Outline", ellipseLineCenter.x - radiusH, ellipseLineCenter.y, 20, BLACK);

        // Draw ellipse with rotation
        DrawEllipseRotation(rotationEllipseCenter.x, rotationEllipseCenter.y, radiusH, radiusV, rotationEllipseColor, angle);
        DrawText("Rotation Ellipse", rotationEllipseCenter.x - radiusH - 35, rotationEllipseCenter.y, 20, BLACK);

        // Draw ellipse outline with rotation
        DrawEllipseLinesRotation(rotationEllipseLineCenter.x, rotationEllipseLineCenter.y, radiusH, radiusV, rotationEllipseLineColor, angle);
        DrawText("Rotation Ellipse outline", rotationEllipseLineCenter.x - radiusH - 35, rotationEllipseLineCenter.y, 20, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}