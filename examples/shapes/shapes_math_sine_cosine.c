/*******************************************************************************************
*
*   raylib [shapes] example - math sine cosine
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.6-dev, last time updated with raylib 5.6-dev
*
*   Example contributed by Jopestpe (@jopestpe) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Jopestpe (@jopestpe)
*
********************************************************************************************/

#include "raylib.h"
#include <math.h>
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"                 // Required for GUI controls

// Wave points for sine/cosine visualization
#define WAVE_POINTS 36

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
    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - math sine cosine");

    Vector2 sinePoints[WAVE_POINTS];
    Vector2 cosPoints[WAVE_POINTS];
    Vector2 center = { (screenWidth/2.0f) - 30.0f, screenHeight/2.0f };
    Rectangle start = { 20.0f, screenHeight - 120.f , 200.0f, 100.0f};
    float radius = 130.0f;
    float angle = 0.0f;
    bool pause = false;

    for (int i = 0; i < WAVE_POINTS; i++)
    {
        float t = i/(float)(WAVE_POINTS - 1);
        float currentAngle = t*360.0f*DEG2RAD;
        sinePoints[i] = (Vector2){ start.x + t*start.width, start.y + start.height/2.0f - sinf(currentAngle)*(start.height/2.0f) };
        cosPoints[i] = (Vector2){ start.x + t*start.width, start.y + start.height/2.0f - cosf(currentAngle)*(start.height/2.0f) };
    }

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        float angleRad = angle*DEG2RAD;
        float cosRad = cosf(angleRad);
        float sinRad = sinf(angleRad);

        Vector2 point = { center.x + cosRad*radius, center.y - sinRad*radius };
        Vector2 limitMin = { center.x - radius, center.y - radius };
        Vector2 limitMax = { center.x + radius, center.y + radius };

        float complementary = 90.0f - angle;
        float supplementary = 180.0f - angle;
        float explementary = 360.0f - angle;

        float tangent = Clamp(tanf(angleRad), -10.0f, 10.0f);
        float cotangent = (fabsf(tangent) > 0.001f) ? Clamp(1.0f/tangent, -radius, radius) : 0.0f;
        Vector2 tangentPoint = { center.x + radius, center.y - tangent*radius };
        Vector2 cotangentPoint = { center.x + cotangent*radius, center.y - radius };

        angle = Wrap(angle + (!pause ? 1.0f : 0.0f), 0.0f, 360.0f);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Cotangent (orange)
            DrawLineEx((Vector2){ center.x , limitMin.y }, (Vector2){ cotangentPoint.x, limitMin.y }, 2.0f, ORANGE);
            DrawLineDashed(center, cotangentPoint, 10.0f, 4.0f, ORANGE);

            // Side background
            DrawLine(580, 0, 580, GetScreenHeight(), (Color){ 218, 218, 218, 255 });
            DrawRectangle(580, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 232, 232, 232, 255 });

            // Base circle and axes
            DrawCircleLinesV(center, radius, GRAY);
            DrawLineEx((Vector2){ center.x, limitMin.y }, (Vector2){ center.x, limitMax.y }, 1.0f, GRAY);
            DrawLineEx((Vector2){ limitMin.x, center.y }, (Vector2){ limitMax.x, center.y }, 1.0f, GRAY);

            // Wave graph axes
            DrawLineEx((Vector2){ start.x , start.y }, (Vector2){ start.x , start.y + start.height }, 2.0f, GRAY);
            DrawLineEx((Vector2){ start.x + start.width, start.y }, (Vector2){ start.x + start.width, start.y + start.height }, 2.0f, GRAY);
            DrawLineEx((Vector2){ start.x, start.y + start.height/2 }, (Vector2){ start.x + start.width, start.y + start.height/2 }, 2.0f, GRAY);

            // Wave graph axis labels
            DrawText("1", start.x - 8,  start.y, 6, GRAY);
            DrawText("0", start.x - 8,  start.y + start.height/2 - 6, 6, GRAY);
            DrawText("-1", start.x - 12, start.y + start.height - 8, 6, GRAY);
            DrawText("0", start.x - 2,  start.y + start.height + 4, 6, GRAY);
            DrawText("360", start.x + start.width - 8,  start.y + start.height + 4, 6, GRAY);

            // Sine (red - vertical)
            DrawLineEx((Vector2){ center.x, center.y }, (Vector2){ center.x, point.y }, 2.0f, RED);
            DrawLineDashed((Vector2){ point.x, center.y }, (Vector2){ point.x, point.y }, 10.0f, 4.0f, RED);
            DrawText(TextFormat("Sine %.2f", sinRad), 640, 190, 6, RED);
            DrawCircleV((Vector2){ start.x + (angle/360.0f)*start.width, start.y + ((-sinRad + 1)*start.height/2.0f) }, 4.0f, RED);
            DrawSplineLinear(sinePoints, WAVE_POINTS, 1.0f, RED);

            // Cosine (blue - horizontal)
            DrawLineEx((Vector2){ center.x, center.y }, (Vector2){ point.x, center.y }, 2.0f, BLUE);
            DrawLineDashed((Vector2){ center.x , point.y }, (Vector2){ point.x, point.y }, 10.0f, 4.0f, BLUE);
            DrawText(TextFormat("Cosine %.2f", cosRad), 640, 210, 6, BLUE);
            DrawCircleV((Vector2){ start.x + (angle/360.0f)*start.width, start.y + ((-cosRad + 1)*start.height/2.0f) }, 4.0f, BLUE);
            DrawSplineLinear(cosPoints, WAVE_POINTS, 1.0f, BLUE);

            // Tangent (purple)
            DrawLineEx((Vector2){ limitMax.x , center.y }, (Vector2){ limitMax.x, tangentPoint.y }, 2.0f, PURPLE);
            DrawLineDashed(center, tangentPoint, 10.0f, 4.0f, PURPLE);
            DrawText(TextFormat("Tangent %.2f", tangent), 640, 230, 6, PURPLE);

            // Cotangent (orange)
            DrawText(TextFormat("Cotangent %.2f", cotangent), 640, 250, 6, ORANGE);

            // Complementary angle (beige)
            DrawCircleSectorLines(center, radius*0.6f , -angle, -90.0f , 36.0f, BEIGE);
            DrawText(TextFormat("Complementary  %0.f°",complementary), 640, 150, 6, BEIGE);

            // Supplementary angle (darkblue)
            DrawCircleSectorLines(center, radius*0.5f , -angle, -180.0f , 36.0f, DARKBLUE);
            DrawText(TextFormat("Supplementary  %0.f°",supplementary), 640, 130, 6, DARKBLUE);

            // Explementary angle (pink)
            DrawCircleSectorLines(center, radius*0.4f , -angle, -360.0f , 36.0f, PINK);
            DrawText(TextFormat("Explementary  %0.f°",explementary), 640, 170, 6, PINK);

            // Current angle - arc (lime), radius (black), endpoint (black)
            DrawCircleSectorLines(center, radius*0.7f , -angle, 0.0f, 36.0f, LIME);
            DrawLineEx((Vector2){ center.x , center.y }, point, 2.0f, BLACK);
            DrawCircleV(point, 4.0f, BLACK);

            // Draw GUI controls
            //------------------------------------------------------------------------------
            GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(GRAY));
            GuiToggle((Rectangle){ 640, 70, 120, 20}, TextFormat("Pause"), &pause);
            GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(LIME));
            GuiSliderBar((Rectangle){ 640, 40, 120, 20}, "Angle", TextFormat("%.0f°", angle), &angle, 0.0f, 360.0f);

            // Angle values panel
            GuiGroupBox((Rectangle){ 620, 110, 140, 170}, "Angle Values");
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