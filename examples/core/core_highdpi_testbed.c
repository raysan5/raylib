/*******************************************************************************************
*
*   raylib [core] example - highdpi testbed
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.6-dev, last time updated with raylib 5.6-dev
*
*   Example contributed by Ramon Santamaria (@raysan5) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Ramon Santamaria (@raysan5)
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

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "raylib [core] example - highdpi testbed");

    Vector2 scaleDpi = GetWindowScaleDPI();
    Vector2 mousePos = GetMousePosition();
    int currentMonitor = GetCurrentMonitor();
    Vector2 windowPos = GetWindowPosition();

    int gridSpacing = 40;   // Grid spacing in pixels

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        mousePos = GetMousePosition();
        currentMonitor = GetCurrentMonitor();
        scaleDpi = GetWindowScaleDPI();
        windowPos = GetWindowPosition();

        if (IsKeyPressed(KEY_SPACE)) ToggleBorderlessWindowed();
        if (IsKeyPressed(KEY_F)) ToggleFullscreen();
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Draw grid
            for (int h = 0; h < GetScreenHeight()/gridSpacing + 1; h++)
            {
                DrawText(TextFormat("%02i", h*gridSpacing), 4, h*gridSpacing - 4, 10, GRAY);
                DrawLine(24, h*gridSpacing, GetScreenWidth(), h*gridSpacing, LIGHTGRAY);
            }
            for (int v = 0; v < GetScreenWidth()/gridSpacing + 1; v++)
            {
                DrawText(TextFormat("%02i", v*gridSpacing), v*gridSpacing - 10, 4, 10, GRAY);
                DrawLine(v*gridSpacing, 20, v*gridSpacing, GetScreenHeight(), LIGHTGRAY);
            }

            // Draw UI info
            DrawText(TextFormat("CURRENT MONITOR: %i/%i (%ix%i)", currentMonitor + 1, GetMonitorCount(), 
                GetMonitorWidth(currentMonitor), GetMonitorHeight(currentMonitor)), 50, 50, 20, DARKGRAY);
            DrawText(TextFormat("WINDOW POSITION: %ix%i", (int)windowPos.x, (int)windowPos.y), 50, 90, 20, DARKGRAY);
            DrawText(TextFormat("SCREEN SIZE: %ix%i", GetScreenWidth(), GetScreenHeight()), 50, 130, 20, DARKGRAY);
            DrawText(TextFormat("RENDER SIZE: %ix%i", GetRenderWidth(), GetRenderHeight()), 50, 170, 20, DARKGRAY);
            DrawText(TextFormat("SCALE FACTOR: %.1fx%.1f", scaleDpi.x, scaleDpi.y), 50, 210, 20, GRAY);

            // Draw reference rectangles, top-left and bottom-right corners
            DrawRectangle(0, 0, 30, 60, RED);
            DrawRectangle(GetScreenWidth() - 30, GetScreenHeight() - 60, 30, 60, BLUE);

            // Draw mouse position
            DrawCircleV(GetMousePosition(), 20, MAROON);
            DrawRectangle(mousePos.x - 25, mousePos.y, 50, 2, BLACK);
            DrawRectangle(mousePos.x, mousePos.y - 25, 2, 50, BLACK);
            DrawText(TextFormat("[%i,%i]", GetMouseX(), GetMouseY()), mousePos.x - 44,
                (mousePos.y > GetScreenHeight() - 60)? mousePos.y - 46 : mousePos.y + 30, 20, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    // TODO: Unload all loaded resources at this point

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
