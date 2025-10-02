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
    const int angles[] = { 30, 10, 45, 35, 60, 38, 75, 67 };
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
                // Gradient gray color per slice
                unsigned char gray = (unsigned char)((i/(float)anglesCount)*255.0f);

                // Draw pie piece using DrawCircleSector()
                DrawCircleSector(center, radius, lastAngleDeg, lastAngleDeg + (float)angles[i],
                    segments, (Color){ gray, gray, gray, 255 });

                lastAngleDeg += (float)angles[i];
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
