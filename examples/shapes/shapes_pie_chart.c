/*******************************************************************************************
*
*   raylib [shapes] example - pie chart
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.6
*
*   Example contributed by Gideon Serfontein (@GideonSerf) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Gideon Serfontein (@GideonSerf)
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

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - pie chart");

    // Pie slice angles (must sum to 360)
    const float angles[] = { 30.0f, 10.0f, 45.0f, 35.0f, 60.0f, 38.0f, 75.0f, 67.0f };
    const int anglesCount = (int)(sizeof(angles)/sizeof(angles[0]));

    const float radius = 150.0f;
    const Vector2 center = { screenWidth/2.0f, screenHeight/2.0f };

    // Set our game to run at 60 frames-per-second
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            float lastAngleDeg = 0.0f;

            // Number of radial segments for sector drawing
            const int segments = 100;

            for (int i = 0; i < anglesCount; i++)
            {
                float hue = (i/(float)anglesCount)*360.0f;
                float sat = 0.75f;
                float val = 0.95f;

                // Convert HSV to RGBA
                Color col = ColorFromHSV(hue, sat, val);

                // Draw pie piece using DrawCircleSector()
                DrawCircleSector(center, radius, lastAngleDeg, lastAngleDeg + angles[i],
                    segments, col);

                lastAngleDeg += angles[i];
            }

            DrawText("Pie chart drawing example", 260, 400, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
