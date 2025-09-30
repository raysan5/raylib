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

#define MAX_MONITORS 10

// Monitor Details
typedef struct Monitor {
    Vector2 position;
    char *name;
    int width;
    int height;
    int physicalWidth;
    int physicalHeight;
    int refreshRate;
} Monitor;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    Monitor monitors[MAX_MONITORS] = { 0 };

    InitWindow(screenWidth, screenHeight, "raylib [core] example - monitor change");

    int currentMonitorIndex = GetCurrentMonitor();
    int monitorCount = 0;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        // Rebuild monitors array with the new monitor count
        if (monitorCount != GetMonitorCount())
        {
            monitorCount = GetMonitorCount();
            for (int i = 0; i < monitorCount; i++)
            {
                monitors[i] = (Monitor){
                    GetMonitorPosition(i), 
                    GetMonitorName(i), 
                    GetMonitorWidth(i),
                    GetMonitorHeight(i),
                    GetMonitorPhysicalWidth(i),
                    GetMonitorPhysicalHeight(i),
                    GetMonitorRefreshRate(i)
                };
            }
        }

        if (IsKeyPressed(KEY_ENTER) && monitorCount > 1) 
        {
            currentMonitorIndex += 1;

            // Set index to 0 if the last one
            if(currentMonitorIndex == GetMonitorCount()) 
            {
                currentMonitorIndex = 0;
            }
            SetWindowMonitor(currentMonitorIndex); // Move window to currentMonitorIndex
        }
        else 
        {
            // Get currentMonitorIndex if manually moved
            currentMonitorIndex = GetCurrentMonitor();
        }
        const Monitor currentMonitor = monitors[currentMonitorIndex];

        const float monitorScale = 0.2 / monitorCount; 

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("Press [Enter] to move window to next monitor available", 20, 20, 20, DARKGRAY);

            DrawText(
                TextFormat("Resolution: [%ipx x %ipx]\nRefreshRate: [%ihz]\nPhysical Size: [%imm x %imm]\nPosition: %3.2f x %3.2f", 
                    currentMonitor.width, 
                    currentMonitor.height, 
                    currentMonitor.refreshRate,
                    currentMonitor.physicalWidth,
                    currentMonitor.physicalHeight,
                    currentMonitor.position.x,
                    currentMonitor.position.y
                ), 30, 80, 20, GRAY);

            // List available Monitors
            for (int i = 0; i < monitorCount; i++)
            {
                DrawText(TextFormat("%s", monitors[i].name), 40, 180 + 20*i, 20, GRAY);
                if (i == currentMonitorIndex)
                {
                    DrawCircle(30, 190 + 20*i, 5, RED);
                }
            }
            DrawRectangleLines(20, 60, screenWidth - 40, screenHeight - 100, DARKGRAY);

            // Draw Monitors
            for (int i = 0; i < monitorCount; i++)
            {
                const Rectangle rec = (Rectangle){
                    monitors[i].position.x * monitorScale + 140,
                    monitors[i].position.y * monitorScale + 180,
                    monitors[i].width * monitorScale,
                    monitors[i].height * monitorScale
                };
                if (i == currentMonitorIndex)
                {
                    DrawRectangleLinesEx(rec, 5, RED);
                }
                else
                {
                    DrawRectangleLinesEx(rec, 5, GRAY);
                }
                DrawText(TextFormat("%i", i), rec.x + rec.width * 0.5 - 10, rec.y + rec.height * 0.5 - 25, 50, GRAY);

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