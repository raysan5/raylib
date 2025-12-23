/*******************************************************************************************
*
*   raylib [core] example - highdpi demo
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.0, last time updated with raylib 5.5
*
*   Example contributed by Jonathan Marler (@marler8997) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Jonathan Marler (@marler8997)
*
********************************************************************************************/

#include "raylib.h"

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
static void DrawTextCenter(const char *text, int x, int y, int fontSize, Color color);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "raylib [core] example - highdpi demo");
    SetWindowMinSize(450, 450);

    int logicalGridDescY = 120;
    int logicalGridLabelY = logicalGridDescY + 30;
    int logicalGridTop = logicalGridLabelY + 30;
    int logicalGridBottom = logicalGridTop + 80;
    int pixelGridTop = logicalGridBottom - 20;
    int pixelGridBottom = pixelGridTop + 80;
    int pixelGridLabelY = pixelGridBottom + 30;
    int pixelGridDescY = pixelGridLabelY + 30;
    int cellSize = 50;
    float cellSizePx = (float)cellSize;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        int monitorCount = GetMonitorCount();

        if ((monitorCount > 1) && IsKeyPressed(KEY_N))
        {
            SetWindowMonitor((GetCurrentMonitor() + 1)%monitorCount);
        }

        int currentMonitor = GetCurrentMonitor();
        Vector2 dpiScale = GetWindowScaleDPI();
        cellSizePx = ((float)cellSize)/dpiScale.x;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            int windowCenter = GetScreenWidth()/2;
            DrawTextCenter(TextFormat("Dpi Scale: %f", dpiScale.x), windowCenter, 30, 40, DARKGRAY);
            DrawTextCenter(TextFormat("Monitor: %d/%d ([N] next monitor)", currentMonitor+1, monitorCount), windowCenter, 70, 20, LIGHTGRAY);
            DrawTextCenter(TextFormat("Window is %d \"logical points\" wide", GetScreenWidth()), windowCenter, logicalGridDescY, 20, ORANGE);

            bool odd = true;
            for (int i = cellSize; i < GetScreenWidth(); i += cellSize, odd = !odd)
            {
                if (odd) DrawRectangle(i, logicalGridTop, cellSize, logicalGridBottom-logicalGridTop, ORANGE);

                DrawTextCenter(TextFormat("%d", i), i, logicalGridLabelY, 10, LIGHTGRAY);
                DrawLine(i, logicalGridLabelY + 10, i, logicalGridBottom, GRAY);
            }

            odd = true;
            const int minTextSpace = 30;
            int lastTextX = -minTextSpace;
            for (int i = cellSize; i < GetRenderWidth(); i += cellSize, odd = !odd)
            {
                int x = (int)(((float)i)/dpiScale.x);
                if (odd) DrawRectangle(x, pixelGridTop, (int)cellSizePx, pixelGridBottom - pixelGridTop, CLITERAL(Color){ 0, 121, 241, 100 });

                DrawLine(x, pixelGridTop, (int)(((float)i)/dpiScale.x), pixelGridLabelY - 10, GRAY);

                if ((x - lastTextX) >= minTextSpace)
                {
                    DrawTextCenter(TextFormat("%d", i), x, pixelGridLabelY, 10, LIGHTGRAY);
                    lastTextX = x;
                }
            }

            DrawTextCenter(TextFormat("Window is %d \"physical pixels\" wide", GetRenderWidth()), windowCenter, pixelGridDescY, 20, BLUE);

            const char *text = "Can you see this?";
            Vector2 size = MeasureTextEx(GetFontDefault(), text, 20, 3);
            Vector2 pos = (Vector2){ GetScreenWidth() - size.x - 5, GetScreenHeight() - size.y - 5 };
            DrawTextEx(GetFontDefault(), text, pos, 20, 3, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definition
//------------------------------------------------------------------------------------
static void DrawTextCenter(const char *text, int x, int y, int fontSize, Color color)
{
    Vector2 size = MeasureTextEx(GetFontDefault(), text, (float)fontSize, 3);
    Vector2 pos = (Vector2){ x - size.x/2, y - size.y/2 };
    DrawTextEx(GetFontDefault(), text, pos, (float)fontSize, 3, color);
}
