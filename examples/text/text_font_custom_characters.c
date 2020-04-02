#include "raylib.h"

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [text] example - font custom characters");
    SetTargetFPS(60);

    int chars[126 - 33];
    for (int i = 0; i < 126 - 33; i++) chars[i] = 33 + i;

    Font font = LoadFontEx("resources/KAISG.ttf", 64, chars, 126 - 33);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(WHITE);

        DrawTexture(font.texture, 0, 0, BLACK);
        EndDrawing();
    }
}