// Standalone GLFW regression: link with raylib and run at normal and high-DPI desktop scaling.
#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static int CheckSize(const char *step)
{
    for (int i = 0; i < 10; i++)
    {
        BeginDrawing();
        EndDrawing();
    }
    Vector2 dpi = IsWindowState(FLAG_WINDOW_HIGHDPI)? GetWindowScaleDPI() : (Vector2){ 1.0f, 1.0f };
    int expectedWidth = (int)roundf((float)GetRenderWidth()/dpi.x);
    int expectedHeight = (int)roundf((float)GetRenderHeight()/dpi.y);
    printf("%s: screen %dx%d, render %dx%d, dpi %.2fx%.2f\n", step,
        GetScreenWidth(), GetScreenHeight(), GetRenderWidth(), GetRenderHeight(), dpi.x, dpi.y);
    return (GetScreenWidth() == expectedWidth) && (GetScreenHeight() == expectedHeight);
}

int main(int argc, char **argv)
{
    bool highDpi = true;
    bool resizable = true;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--no-highdpi") == 0) highDpi = false;
        else if (strcmp(argv[i], "--fixed") == 0) resizable = false;
        else return 2;
    }
    SetConfigFlags(FLAG_WINDOW_HIDDEN | (resizable? FLAG_WINDOW_RESIZABLE : 0) |
        (highDpi? FLAG_WINDOW_HIGHDPI : 0));
    InitWindow(800, 450, "Window size regression");
    if (!IsWindowReady()) return 2;
    SetTargetFPS(60);
    int success = CheckSize("initial");
    SetWindowSize(2000, 1100);
    if (!highDpi)
    {
        // A real resize must still update the reported size before polling events.
        printf("immediate resize: screen %dx%d\n", GetScreenWidth(), GetScreenHeight());
        success &= (GetScreenWidth() == 2000) && (GetScreenHeight() == 1100);
    }
    success &= CheckSize("resize");
    SetWindowSize(2000, 1100);
    success &= CheckSize("same-size resize");
    SetWindowSize(1200, 800);
    success &= CheckSize("second resize");
    SetWindowSize(1200, 800);
    success &= CheckSize("same-size second resize");
    SetWindowSize(1201, 800);
    success &= CheckSize("width-only resize");
    SetWindowSize(1201, 800);
    success &= CheckSize("same-size width-only resize");
    SetWindowSize(1201, 801);
    success &= CheckSize("height-only resize");
    SetWindowSize(1201, 801);
    success &= CheckSize("same-size height-only resize");
    CloseWindow();
    return success ? 0 : 1;
}
