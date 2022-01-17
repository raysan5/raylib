/*******************************************************************************************
*
*   raylib [core] example - window shows placeholder while resizing
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
        DrawText("Resizing",  GetScreenWidth()/2.5, GetScreenHeight()/2.5, 30, LIGHTGRAY);
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
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Congrats! You created your first window!",  GetScreenWidth()/2.5-100, GetScreenHeight()/2, 20, LIGHTGRAY);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
