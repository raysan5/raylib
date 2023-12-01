#include "raylib.h"

static char BUF[1024];

int main()
{
  double width = 850;
  double height = 450;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow((int) width, (int) height, "raylib custom platform 1 (web)");

  SetTargetFPS(60);

  while (!WindowShouldClose())
  {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Congrats! You created your first window!", 10, 40, 20, LIGHTGRAY);
    DrawLine(0, 0, GetScreenWidth(), GetScreenHeight(), RED);
    DrawFPS(10, 70);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
