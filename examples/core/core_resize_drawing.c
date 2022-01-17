/*******************************************************************************************
*
*   raylib [core] example - window redraws while resizing
*
*   This example has been created using raylib 4.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Efejjota (@efejjota) and reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2022 Efejjota (@efejjota) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

void draw()
{
    BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Congrats! Your window redraws while resizing!",  GetScreenWidth()/2.5-150, GetScreenHeight()/2, 20, LIGHTGRAY);
    EndDrawing();
}

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE); // Allow resizing the windows
    SetResizeCallback(draw);               // Set the function to call while resizing
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        draw(); // Write drawing code in a function so we can use as callback while resizing
    }
    CloseWindow();
    return 0;
}
