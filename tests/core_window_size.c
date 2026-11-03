// Standalone GLFW regression: link with raylib and run at normal and high-DPI desktop scaling.
#include "raylib.h"
#include <math.h>
#include <stdio.h>

static int CheckSize(const char *step)
{
    for (int i = 0; i < 10; i++)
    {
        BeginDrawing();
        EndDrawing();
    }
    Vector2 dpi = GetWindowScaleDPI();
    int expectedWidth = (int)roundf((float)GetRenderWidth()/dpi.x);
    int expectedHeight = (int)roundf((float)GetRenderHeight()/dpi.y);
    printf("%s: screen %dx%d, render %dx%d, dpi %.2fx%.2f\n", step,
        GetScreenWidth(), GetScreenHeight(), GetRenderWidth(), GetRenderHeight(), dpi.x, dpi.y);
    return (GetScreenWidth() == expectedWidth) && (GetScreenHeight() == expectedHeight);
}

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_HIDDEN);
    InitWindow(800, 450, "Window size regression");
    if (!IsWindowReady()) return 2;
    SetTargetFPS(60);
    int success = CheckSize("initial");
    SetWindowSize(2000, 1100);
    success &= CheckSize("resize");
    SetWindowSize(2000, 1100);
    success &= CheckSize("same-size resize");
    SetWindowSize(1200, 800);
    success &= CheckSize("second resize");
    SetWindowSize(1200, 800);
    success &= CheckSize("same-size second resize");
    CloseWindow();
    return success ? 0 : 1;
}
