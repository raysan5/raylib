/*******************************************************************************************
*
*   raylib [shapes] example - triangle strip
*
*   Example complexity rating: [★★☆☆] 2/4
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

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - triangle strip");

    Vector2 points[122] = { 0 };
    Vector2 center = { (screenWidth/2.0f) - 125.0f, screenHeight/2.0f };
    float segments = 6.0f;
    float insideRadius = 100.0f;
    float outsideRadius = 150.0f;
    bool outline = true;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        int pointCount = (int)(segments);
        float angleStep = (360.0f/pointCount)*DEG2RAD;

        for (int i = 0, i2 = 0; i < pointCount; i++, i2 += 2)
        {
            float angle1 = i*angleStep;
            points[i2] = (Vector2){ center.x + cosf(angle1)*insideRadius, center.y + sinf(angle1)*insideRadius };
            float angle2 = angle1 + angleStep/2.0f;
            points[i2 + 1] = (Vector2){ center.x + cosf(angle2)*outsideRadius, center.y + sinf(angle2)*outsideRadius };
        }

        points[pointCount*2] = points[0];
        points[pointCount*2 + 1] = points[1];
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            for (int i = 0; i < pointCount; i++)
            {
                Vector2 a = points[i*2];
                Vector2 b = points[i*2 + 1];
                Vector2 c = points[i*2 + 2];
                Vector2 d = points[i*2 + 3];

                float angle1 = i*angleStep;
                DrawTriangle(c, b, a, ColorFromHSV(angle1*RAD2DEG, 1.0f, 1.0f));
                DrawTriangle(d, b, c, ColorFromHSV((angle1 + angleStep/2)*RAD2DEG, 1.0f, 1.0f));

                if (outline)
                {
                    DrawTriangleLines(a, b, c, BLACK);
                    DrawTriangleLines(c, b, d, BLACK);
                }
            }

            DrawLine(580, 0, 580, GetScreenHeight(), (Color){ 218, 218, 218, 255 });
            DrawRectangle(580, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 232, 232, 232, 255 });

            // Draw GUI controls
            //------------------------------------------------------------------------------
            GuiSliderBar((Rectangle){ 640, 40, 120, 20}, "Segments", TextFormat("%.0f", segments), &segments, 6.0f, 60.0f);
            GuiCheckBox((Rectangle){ 640, 70, 20, 20 }, "Outline", &outline);
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