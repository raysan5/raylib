/*******************************************************************************************
*
*   raylib [core] example - monitor change
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.6
*
*   Example contributed by Maicon Santana (@maiconpintoabreu) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025-2025 Maicon Santana (@maiconpintoabreu)
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

    InitWindow(screenWidth, screenHeight, "raylib [core] example - monitor change");

    int currentMonitor = GetCurrentMonitor();

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        if (IsKeyPressed(KEY_ENTER)) {
            currentMonitor += 1;
            if(currentMonitor == GetMonitorCount()) {
                currentMonitor = 0;
            }
            SetWindowMonitor(currentMonitor);
        } else {
            currentMonitor = GetCurrentMonitor();
        }
        
        const Vector2 resolution = (Vector2){(float)GetMonitorWidth(currentMonitor), (float)GetMonitorHeight(currentMonitor)};
        const int refreshRate = GetMonitorRefreshRate(currentMonitor);

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("[Enter] Move to the next monitor available", 20, 20, 20, DARKGRAY);

            DrawText(
                TextFormat("%3.0f x %3.0f [%ihz]", 
                    resolution.x, 
                    resolution.y,
                    refreshRate
                ), 20, 70, 20, GRAY);
            DrawText(TextFormat("%i", currentMonitor), screenWidth * 0.5, screenHeight * 0.5, 70, GRAY);
            DrawRectangleLines(20, 100, screenWidth - 40, screenHeight - 120, DARKGRAY);


        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}