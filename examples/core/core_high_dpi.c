/*******************************************************************************************
*
*   raylib [core] example - HighDPI
*
*   Example complexity rating: [★☆☆☆] e/4
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2013-2025 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

static void DrawTextCenter(const char *text, int x, int y, int fontSize, Color color)
{
    Vector2 size = MeasureTextEx(GetFontDefault(), text, (float)fontSize, 3);
    Vector2 pos = (Vector2){x - size.x/2, y - size.y/2 };
    DrawTextEx(GetFontDefault(), text, pos, (float)fontSize, 3, color);
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

    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);

    InitWindow(screenWidth, screenHeight, "raylib [core] example - highdpi");
    SetWindowMinSize(450, 450);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        int monitorCount = GetMonitorCount();
        if (monitorCount > 1 && IsKeyPressed(KEY_N)) {
            SetWindowMonitor((GetCurrentMonitor() + 1) % monitorCount);
        }
        int currentMonitor = GetCurrentMonitor();

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            Vector2 dpiScale = GetWindowScaleDPI();
            ClearBackground(RAYWHITE);

            int windowCenter = GetScreenWidth() / 2;
            DrawTextCenter(TextFormat("Dpi Scale: %f", dpiScale.x), windowCenter, 30, 40, DARKGRAY);
            DrawTextCenter(TextFormat("Monitor: %d/%d ([N] next monitor)", currentMonitor+1, monitorCount), windowCenter, 70, 16, LIGHTGRAY);

            const int logicalGridDescY = 120;
            const int logicalGridLabelY = logicalGridDescY + 30;
            const int logicalGridTop = logicalGridLabelY + 30;
            const int logicalGridBottom = logicalGridTop + 80;
            const int pixelGridTop = logicalGridBottom - 20;
            const int pixelGridBottom = pixelGridTop + 80;
            const int pixelGridLabelY = pixelGridBottom + 30;
            const int pixelGridDescY = pixelGridLabelY + 30;

            const int cellSize = 50;
            const float cellSizePx = ((float)cellSize) / dpiScale.x;

            DrawTextCenter(TextFormat("Window is %d \"logical points\" wide", GetScreenWidth()), windowCenter, logicalGridDescY, 20, ORANGE);
            bool odd = true;
            for (int i = cellSize; i < GetScreenWidth(); i += cellSize, odd = !odd) {
                if (odd) {
                    DrawRectangle(i, logicalGridTop, cellSize, logicalGridBottom-logicalGridTop, ORANGE);
                }
                DrawTextCenter(TextFormat("%d", i), i, logicalGridLabelY, 12, LIGHTGRAY);
                DrawLine(i, logicalGridLabelY + 10, i, logicalGridBottom, GRAY);
            }

            odd = true;
            const int minTextSpace = 30;
            int last_text_x = -minTextSpace;
            for (int i = cellSize; i < GetRenderWidth(); i += cellSize, odd = !odd) {
                int x = (int)(((float)i) / dpiScale.x);
                if (odd) {
                    DrawRectangle(x, pixelGridTop, (int)cellSizePx, pixelGridBottom-pixelGridTop, CLITERAL(Color){ 0, 121, 241, 100 });
                }
                DrawLine(x, pixelGridTop, (int)(((float)i) / dpiScale.x), pixelGridLabelY - 10, GRAY);
                if (x - last_text_x >= minTextSpace) {
                    DrawTextCenter(TextFormat("%d", i), x, pixelGridLabelY, 12, LIGHTGRAY);
                    last_text_x = x;
                }
            }

            DrawTextCenter(TextFormat("Window is %d \"physical pixels\" wide", GetRenderWidth()), windowCenter, pixelGridDescY, 20, BLUE);

            {
                const char *text = "Can you see this?";
                Vector2 size = MeasureTextEx(GetFontDefault(), text, 16, 3);
                Vector2 pos = (Vector2){GetScreenWidth() - size.x - 5, GetScreenHeight() - size.y - 5};
                DrawTextEx(GetFontDefault(), text, pos, 16, 3, LIGHTGRAY);
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
